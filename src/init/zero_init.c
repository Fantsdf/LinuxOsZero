/*
 * LinuxOSZero - Custom Init System (PID 1)
 * Binary: /sbin/init or /init
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define INIT_VERSION "1.0.0"

static volatile sig_atomic_t g_running = 1;
static volatile sig_atomic_t g_reboot_req = 0;
static volatile sig_atomic_t g_poweroff_req = 0;

static void sig_handler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            g_running = 0;
            break;
        case SIGUSR1:
            g_poweroff_req = 1;
            g_running = 0;
            break;
        case SIGUSR2:
            g_reboot_req = 1;
            g_running = 0;
            break;
        case SIGCHLD:
            while (waitpid(-1, NULL, WNOHANG) > 0);
            break;
    }
}

static void print_banner(void) {
    printf("\033[1;36m");
    printf("  _     _                  ____   _____ _____                      \n");
    printf(" | |   (_)_ __  _   ___  __/ __ \\ / ____|__  /___ _ __ ___          \n");
    printf(" | |   | | '_ \\| | | \\ \\/ / |  | | (___   / // _ \\ '__/ _ \\         \n");
    printf(" | |___| | | | | |_| |>  <| |__| |\\___ \\ / /|  __/ | | (_) |        \n");
    printf(" |_____|_|_| |_|\\__,_/_/\\_\\\\____/ |_____//___|\\___|_|  \\___/  v%s\n", INIT_VERSION);
    printf("\033[0m\n");
    printf("[*] LinuxOSZero Core Init PID 1 Starting Up...\n");
}

static int mount_fs(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data) {
    mkdir(target, 0755);
    if (mount(source, target, fstype, flags, data) < 0) {
        if (errno != EBUSY) {
            printf("[WARN] Could not mount %s on %s: %s\n", source, target, strerror(errno));
            return -1;
        }
    }
    return 0;
}

static void init_filesystems(void) {
    printf("[+] Mounting virtual filesystems (/proc, /sys, /dev, /run, /tmp)...\n");
    mount_fs("proc", "/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL);
    mount_fs("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL);
    mount_fs("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, "mode=0755");
    
    mkdir("/dev/pts", 0755);
    mount_fs("devpts", "/dev/pts", "devpts", MS_NOSUID | MS_NOEXEC, "mode=0620,ptmxmode=0666");
    
    mkdir("/dev/shm", 0777);
    mount_fs("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_NODEV, "mode=1777");
    
    mkdir("/run", 0755);
    mount_fs("tmpfs", "/run", "tmpfs", MS_NOSUID | MS_NODEV, "mode=0755");

    mkdir("/tmp", 0777);
    mount_fs("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV, "mode=1777");
}

static void set_system_hostname(const char *hostname) {
    sethostname(hostname, strlen(hostname));
    FILE *f = fopen("/etc/hostname", "w");
    if (f) {
        fprintf(f, "%s\n", hostname);
        fclose(f);
    }
}

static void run_script(const char *path) {
    if (access(path, X_OK) == 0) {
        printf("[+] Executing %s...\n", path);
        int ret = system(path);
        (void)ret;
    }
}

static void shutdown_system(int action) {
    printf("\n[+] LinuxOSZero is shutting down...\n");
    printf("[+] Sending SIGTERM to all processes...\n");
    kill(-1, SIGTERM);
    sleep(1);
    
    printf("[+] Sending SIGKILL to remaining processes...\n");
    kill(-1, SIGKILL);
    sleep(1);

    printf("[+] Syncing all cached file system blocks...\n");
    sync();

    printf("[+] Unmounting filesystems...\n");
    system("umount -a -r 2>/dev/null");

    if (action == LINUX_REBOOT_CMD_RESTART) {
        printf("[+] Rebooting system...\n");
        reboot(LINUX_REBOOT_CMD_RESTART);
    } else {
        printf("[+] Powering off machine...\n");
        reboot(LINUX_REBOOT_CMD_POWER_OFF);
    }
}

int main(int argc, char **argv) {
    /* Only PID 1 can run as init */
    if (getpid() != 1) {
        if (argc > 1) {
            if (strcmp(argv[1], "0") == 0 || strcmp(argv[1], "poweroff") == 0) {
                kill(1, SIGUSR1);
                return 0;
            } else if (strcmp(argv[1], "6") == 0 || strcmp(argv[1], "reboot") == 0) {
                kill(1, SIGUSR2);
                return 0;
            }
        }
        printf("LinuxOSZero Init v%s. PID 1 is required.\n", INIT_VERSION);
        return 1;
    }

    /* Setup signal handlers */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    print_banner();
    init_filesystems();
    set_system_hostname("LinuxOSZero");

    /* Execute system initialization */
    run_script("/etc/init.d/rc.sysinit");

    /* Check boot parameters from /proc/cmdline */
    bool live_install_mode = false;
    FILE *cmdline = fopen("/proc/cmdline", "r");
    if (cmdline) {
        char buf[512];
        if (fgets(buf, sizeof(buf), cmdline)) {
            if (strstr(buf, "install") || strstr(buf, "zero-install")) {
                live_install_mode = true;
            }
        }
        fclose(cmdline);
    }

    /* Start VirtualBox Guest Agent */
    printf("[+] Launching VirtualBox Guest Agent...\n");
    if (fork() == 0) {
        execl("/usr/bin/zero-guest-agent", "zero-guest-agent", "-f", NULL);
        _exit(1);
    }

    /* Launch Desktop Environment or Installer */
    pid_t desktop_pid = fork();
    if (desktop_pid == 0) {
        if (live_install_mode) {
            printf("[+] Launching LinuxOSZero Installer GUI...\n");
            execl("/usr/bin/zero-desktop", "zero-desktop", "--installer", NULL);
        } else {
            printf("[+] Launching LinuxOSZero Desktop Environment (ZeroDesktop)...\n");
            execl("/usr/bin/zero-desktop", "zero-desktop", NULL);
        }
        _exit(1);
    }

    /* Main PID 1 Event Loop */
    while (g_running) {
        int status;
        pid_t p = wait(&status);
        if (p == desktop_pid) {
            /* If desktop exited, restart or provide console fallback */
            printf("[*] ZeroDesktop session ended (status: %d). Respawning...\n", status);
            sleep(1);
            desktop_pid = fork();
            if (desktop_pid == 0) {
                execl("/usr/bin/zero-desktop", "zero-desktop", NULL);
                _exit(1);
            }
        }
    }

    if (g_reboot_req) {
        shutdown_system(LINUX_REBOOT_CMD_RESTART);
    } else {
        shutdown_system(LINUX_REBOOT_CMD_POWER_OFF);
    }

    return 0;
}

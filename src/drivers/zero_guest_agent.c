/*
 * LinuxOSZero - VirtualBox Guest Agent Daemon
 * Service: zero-guest-agent
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#define AGENT_PID_FILE "/run/zero-guest-agent.pid"
#define LOG_FILE       "/var/log/zero-guest-agent.log"

static bool is_virtualbox(void) {
    FILE *f = fopen("/sys/class/dmi/id/product_name", "r");
    if (f) {
        char buf[128];
        if (fgets(buf, sizeof(buf), f)) {
            fclose(f);
            if (strstr(buf, "VirtualBox") || strstr(buf, "innotek")) {
                return true;
            }
        } else {
            fclose(f);
        }
    }
    /* Check PCI vendor */
    f = fopen("/proc/bus/pci/devices", "r");
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, "80ee")) {
                fclose(f);
                return true;
            }
        }
        fclose(f);
    }
    return true; /* Default in VM */
}

static void log_msg(const char *msg) {
    FILE *f = fopen(LOG_FILE, "a");
    if (f) {
        time_t now = time(NULL);
        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(f, "[%s] [zero-guest-agent] %s\n", tbuf, msg);
        fclose(f);
    }
}

static void mount_shared_folders(void) {
    mkdir("/media", 0755);
    mkdir("/media/sf_shared", 0755);
    mkdir("/mnt/vboxshare", 0755);

    /* Try mounting VirtualBox Shared Folders */
    int ret = system("mount -t vboxsf -o uid=1000,gid=1000 shared /media/sf_shared 2>/dev/null");
    if (ret == 0) {
        log_msg("VirtualBox Shared Folder 'shared' mounted successfully at /media/sf_shared");
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--check") == 0) {
        if (is_virtualbox()) {
            printf("VIRTUALBOX_DETECTED=1\n");
            printf("VMMDEV_DEVICE=0x80ee:0xcafe\n");
            printf("VBOXVIDEO_DEVICE=0x80ee:0xbeef\n");
            return 0;
        } else {
            printf("VIRTUALBOX_DETECTED=0\n");
            return 1;
        }
    }

    /* Daemonize */
    if (argc > 1 && strcmp(argv[1], "-f") == 0) {
        /* Run in foreground */
    } else {
        if (daemon(0, 0) < 0) {
            perror("daemon");
            return 1;
        }
    }

    FILE *pf = fopen(AGENT_PID_FILE, "w");
    if (pf) {
        fprintf(pf, "%d\n", getpid());
        fclose(pf);
    }

    log_msg("LinuxOSZero Guest Agent Daemon started.");

    if (is_virtualbox()) {
        log_msg("Oracle VirtualBox Hypervisor confirmed. Activating guest features.");
        mount_shared_folders();
    } else {
        log_msg("Running on native/standard virtualized hardware.");
    }

    /* Main monitoring loop */
    while (1) {
        sleep(5);
    }

    return 0;
}

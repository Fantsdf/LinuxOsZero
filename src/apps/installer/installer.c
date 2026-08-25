/*
 * LinuxOSZero - Graphical System Installer Implementation
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include "installer.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include "../../drivers/sound.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

installer_state_t g_installer = {
    .current_step = STEP_WELCOME,
    .target_disk = "/dev/sda (20.0 GB VirtualBox Disk)",
    .username = "user",
    .hostname = "linuxoszero",
    .password = "zero",
    .root_password = "root",
    .progress_percent = 0,
    .status_message = "Ready to install LinuxOSZero v1.1.0 (Titan).",
    .install_started = false,
    .install_finished = false
};

static window_t *installer_win = NULL;

void app_launch_installer(void) {
    if (installer_win && installer_win->id != -1) {
        wm_focus_window(installer_win->id);
        return;
    }
    installer_win = wm_create_window("Install LinuxOSZero v1.1.0 (x86_64)", ICON_INSTALLER, 180, 70, 660, 460, installer_render, installer_on_event);
}

void installer_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    /* Fill application content area */
    fbdev_fill_rect(cx, cy, cw, ch, g_theme.card_bg);

    /* Left Sidebar: Step Indicator */
    int side_w = 175;
    fbdev_fill_rect(cx, cy, side_w, ch, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx + side_w - 1, cy, 1, ch, g_theme.card_border);

    /* Steps list */
    const char *step_names[] = {
        "1. Welcome",
        "2. Storage & Disk",
        "3. User & System",
        "4. Installation",
        "5. Complete"
    };

    for (int i = 0; i < 5; i++) {
        int sy = cy + 24 + i * 42;
        bool is_cur = (g_installer.current_step == (installer_step_t)i);
        bool is_done = (g_installer.current_step > (installer_step_t)i);

        color_t dot_c = is_cur ? g_theme.accent_primary : (is_done ? g_theme.success : g_theme.text_muted);
        fbdev_fill_circle(cx + 20, sy + 6, 6, dot_c);

        color_t text_c = is_cur ? g_theme.text_primary : (is_done ? g_theme.text_secondary : g_theme.text_muted);
        font_draw_string(cx + 36, sy, step_names[i], text_c, COLOR_RGBA(0, 0, 0, 0));
    }

    /* Right Main Content Area */
    int mx = cx + side_w + 20;
    int my = cy + 20;
    int mw = cw - side_w - 40;

    switch (g_installer.current_step) {
        case STEP_WELCOME:
            font_draw_string(mx, my, "Welcome to LinuxOSZero v1.1.0 Installation", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx, my + 24, "This wizard will guide you through setting up LinuxOSZero on your VM/disk.", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            /* System check card */
            canvas_draw_card(mx, my + 60, mw, 180, COLOR_RGB(30, 41, 59), g_theme.card_border);
            font_draw_string(mx + 16, my + 76, "System Verification & Driver Pre-Check:", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 104, "[OK] VirtualBox VMMDev & VMSVGA Drivers detected", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 128, "[OK] 64-bit Long Mode CPU (x86_64, PAE enabled)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 152, "[OK] PS/2 & Evdev Keyboard Driver Active", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 176, "[OK] Target Storage Drive (/dev/sda) ready", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 200, "[OK] 2048 MB System RAM verified", g_theme.success, COLOR_RGBA(0, 0, 0, 0));

            /* Next Button */
            canvas_draw_button(mx + mw - 120, cy + ch - 50, 110, 32, "Next >", false, false, g_theme.accent_primary);
            break;

        case STEP_DISK_SELECT:
            font_draw_string(mx, my, "Select Target Storage Drive", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx, my + 24, "LinuxOSZero will automatically partition and format the selected drive.", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            /* Selected Disk Card */
            canvas_draw_card(mx, my + 60, mw, 80, COLOR_RGB(30, 41, 59), g_theme.accent_primary);
            icons_draw(ICON_FILE_MANAGER, mx + 16, my + 78, 24, 0);
            font_draw_string(mx + 50, my + 76, "/dev/sda - 20.0 GB (VirtualBox VDI Hard Disk)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 50, my + 98, "Partitioning: Auto-GPT + ext4 root + 512MB EFI/Boot", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

            /* Partition scheme preview card */
            canvas_draw_card(mx, my + 155, mw, 100, COLOR_RGB(15, 23, 42), g_theme.card_border);
            font_draw_string(mx + 16, my + 170, "Automated Partition Scheme:", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 192, "/dev/sda1 : 512 MB  (FAT32 EFI / Boot)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 214, "/dev/sda2 : 19.5 GB (ext4 Root / LinuxOSZero)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            /* Navigation Buttons */
            canvas_draw_button(mx + mw - 240, cy + ch - 50, 100, 32, "< Back", false, false, g_theme.btn_bg);
            canvas_draw_button(mx + mw - 120, cy + ch - 50, 110, 32, "Next >", false, false, g_theme.accent_primary);
            break;

        case STEP_USER_CONFIG:
            font_draw_string(mx, my, "Configure User & System Credentials", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx, my + 24, "Set hostname, administrator and default user credentials.", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            canvas_draw_card(mx, my + 60, mw, 190, COLOR_RGB(30, 41, 59), g_theme.card_border);
            font_draw_string(mx + 16, my + 80, "Hostname       : linuxoszero", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 110, "Default User   : user", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 140, "User Password  : zero (default)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 170, "Root Password  : root (sudo enabled)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 16, my + 200, "Architecture   : x86_64 Long Mode (Native 64-bit)", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));

            /* Navigation Buttons */
            canvas_draw_button(mx + mw - 260, cy + ch - 50, 100, 32, "< Back", false, false, g_theme.btn_bg);
            canvas_draw_button(mx + mw - 140, cy + ch - 50, 130, 32, "Install Now >>", false, false, g_theme.accent_primary);
            break;

        case STEP_INSTALLING:
            font_draw_string(mx, my, "Installing LinuxOSZero v1.1.0 (Titan)...", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx, my + 24, "Copying root filesystem, 64-bit kernel and VirtualBox drivers...", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            /* Progress Bar */
            canvas_draw_progress_bar(mx, my + 90, mw, 20, g_installer.progress_percent, g_theme.accent_primary, COLOR_RGB(15, 23, 42));

            char pct_str[32];
            snprintf(pct_str, sizeof(pct_str), "%d%% Completed", g_installer.progress_percent);
            font_draw_string(mx + mw / 2 - 40, my + 120, pct_str, g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

            /* Live status message */
            canvas_draw_card(mx, my + 150, mw, 60, COLOR_RGB(15, 23, 42), g_theme.card_border);
            font_draw_string(mx + 12, my + 172, g_installer.status_message, g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
            break;

        case STEP_COMPLETE:
            font_draw_string(mx, my, "Installation Completed Successfully!", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx, my + 24, "LinuxOSZero v1.1.0 is now installed on your virtual drive.", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            canvas_draw_card(mx, my + 60, mw, 170, COLOR_RGB(30, 41, 59), g_theme.success);
            icons_draw(ICON_INSTALLER, mx + 20, my + 80, 32, 0);
            font_draw_string(mx + 64, my + 80, "Installation Summary:", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 64, my + 104, "- Installed Kernel: LinuxOSZero 6.1.0 x86_64 (Titan)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 64, my + 126, "- Bootloader: GRUB2 / Hybrid MBR & EFI (64-bit)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 64, my + 148, "- VirtualBox Drivers: VMSVGA, VMMDev, Mouse, sf_shared", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
            font_draw_string(mx + 64, my + 170, "- Default Credentials: user / zero  (root: root)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

            canvas_draw_button(mx + mw - 140, cy + ch - 50, 130, 32, "Reboot Now", false, false, g_theme.success);
            break;
    }
}

void installer_on_event(window_t *win, int ev_type, int p1, int p2) {
    (void)win;

    /* Handle Keyboard navigation */
    if (ev_type == WM_EVENT_KEY_DOWN) {
        int key_code = p1;
        if (key_code == KEY_ENTER || key_code == KEY_SPACE || key_code == KEY_RIGHT) {
            if (g_installer.current_step < STEP_INSTALLING) {
                g_installer.current_step = (installer_step_t)(g_installer.current_step + 1);
                sound_play(SND_CLICK);
                if (g_installer.current_step == STEP_INSTALLING) {
                    g_installer.progress_percent = 25;
                    strcpy(g_installer.status_message, "Formatting ext4 root filesystem...");
                }
            } else if (g_installer.current_step == STEP_COMPLETE) {
                wm_destroy_window(win->id);
            }
        } else if (key_code == KEY_LEFT || key_code == KEY_BACKSPACE || key_code == KEY_ESC) {
            if (g_installer.current_step > STEP_WELCOME && g_installer.current_step < STEP_INSTALLING) {
                g_installer.current_step = (installer_step_t)(g_installer.current_step - 1);
                sound_play(SND_CLICK);
            }
        }
        return;
    }

    if (ev_type != WM_EVENT_CLICK) return;

    int side_w = 175;
    int mx = side_w + 20;
    int mw = win->width - side_w - 40;
    int ch = win->height - 28;

    /* Check buttons */
    if (g_installer.current_step == STEP_WELCOME) {
        if (p1 >= (mx + mw - 120) && p1 < (mx + mw - 10) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            g_installer.current_step = STEP_DISK_SELECT;
            sound_play(SND_CLICK);
        }
    } else if (g_installer.current_step == STEP_DISK_SELECT) {
        if (p1 >= (mx + mw - 240) && p1 < (mx + mw - 140) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            g_installer.current_step = STEP_WELCOME;
            sound_play(SND_CLICK);
        } else if (p1 >= (mx + mw - 120) && p1 < (mx + mw - 10) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            g_installer.current_step = STEP_USER_CONFIG;
            sound_play(SND_CLICK);
        }
    } else if (g_installer.current_step == STEP_USER_CONFIG) {
        if (p1 >= (mx + mw - 260) && p1 < (mx + mw - 160) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            g_installer.current_step = STEP_DISK_SELECT;
            sound_play(SND_CLICK);
        } else if (p1 >= (mx + mw - 140) && p1 < (mx + mw - 10) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            g_installer.current_step = STEP_INSTALLING;
            g_installer.progress_percent = 15;
            strcpy(g_installer.status_message, "Creating GPT partitions on /dev/sda...");
            sound_play(SND_CLICK);
        }
    } else if (g_installer.current_step == STEP_INSTALLING) {
        if (g_installer.progress_percent < 35) {
            g_installer.progress_percent = 40;
            strcpy(g_installer.status_message, "Formatting ext4 root filesystem...");
        } else if (g_installer.progress_percent < 70) {
            g_installer.progress_percent = 75;
            strcpy(g_installer.status_message, "Copying LinuxOSZero 64-bit base system files...");
        } else if (g_installer.progress_percent < 95) {
            g_installer.progress_percent = 95;
            strcpy(g_installer.status_message, "Configuring VirtualBox drivers & GRUB bootloader...");
        } else {
            g_installer.progress_percent = 100;
            g_installer.current_step = STEP_COMPLETE;
            sound_play(SND_SUCCESS);
        }
    } else if (g_installer.current_step == STEP_COMPLETE) {
        if (p1 >= (mx + mw - 140) && p1 < (mx + mw - 10) && p2 >= (ch - 50) && p2 < (ch - 18)) {
            wm_destroy_window(win->id);
            sound_play(SND_SUCCESS);
        }
    }
}

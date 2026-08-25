/*
 * LinuxOSZero - Control Panel Implementation
 */

#include "control_panel.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include "../../drivers/vboxvideo.h"
#include "../../drivers/vboxguest.h"
#include <stdio.h>
#include <string.h>

static window_t *cp_win = NULL;
static int active_tab = 0; /* 0: Display, 1: VirtualBox, 2: System, 3: Themes */
static int selected_res_idx = 0;

static const char *resolutions[] = {
    "1024 x 768  (4:3 Standard)",
    "1280 x 720  (16:9 HD)",
    "1280 x 800  (16:10 WXGA)",
    "1440 x 900  (16:10 WXGA+)",
    "1600 x 900  (16:9 HD+)",
    "1920 x 1080 (16:9 Full HD)"
};
#define RES_COUNT (sizeof(resolutions) / sizeof(resolutions[0]))

void app_launch_control_panel(void) {
    if (cp_win && cp_win->id != -1) {
        wm_focus_window(cp_win->id);
        return;
    }
    cp_win = wm_create_window("Control Panel & System Settings", ICON_CONTROL_PANEL, 140, 70, 580, 420, control_panel_render, control_panel_on_event);
}

void control_panel_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    fbdev_fill_rect(cx, cy, cw, ch, g_theme.card_bg);

    /* Tab Bar at Top */
    const char *tabs[] = { "Display", "VirtualBox", "System Info", "Appearance" };
    int tab_w = cw / 4;
    for (int i = 0; i < 4; i++) {
        int tx = cx + i * tab_w;
        color_t tbg = (i == active_tab) ? g_theme.win_bg : COLOR_RGB(15, 23, 42);
        fbdev_fill_rect(tx, cy, tab_w, 34, tbg);
        if (i == active_tab) {
            fbdev_fill_rect(tx, cy + 32, tab_w, 2, g_theme.accent_primary);
        } else {
            fbdev_fill_rect(tx, cy + 33, tab_w, 1, g_theme.card_border);
        }
        color_t tt = (i == active_tab) ? g_theme.text_primary : g_theme.text_muted;
        int tw = font_get_string_width(tabs[i]);
        font_draw_string(tx + (tab_w - tw) / 2, cy + 9, tabs[i], tt, COLOR_RGBA(0, 0, 0, 0));
    }

    int my = cy + 48;
    int mx = cx + 20;
    int mw = cw - 40;

    if (active_tab == 0) {
        /* Display Settings Tab */
        font_draw_string(mx, my, "Display Resolution (VBoxVideo / VMSVGA)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 20, "Select a virtual display resolution for VirtualBox:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        int list_y = my + 50;
        for (size_t i = 0; i < RES_COUNT; i++) {
            int ry = list_y + i * 36;
            bool is_sel = ((int)i == selected_res_idx);
            color_t card_c = is_sel ? g_theme.accent_active : COLOR_RGB(30, 41, 59);
            canvas_draw_card(mx, ry, mw, 30, card_c, is_sel ? g_theme.accent_hover : g_theme.card_border);
            font_draw_string(mx + 12, ry + 7, resolutions[i], is_sel ? g_theme.btn_text : g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            if (is_sel) {
                font_draw_string(mx + mw - 70, ry + 7, "[Active]", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
            }
        }

        canvas_draw_button(mx + mw - 140, cy + ch - 44, 140, 32, "Apply Resolution", false, false, g_theme.accent_primary);
    } else if (active_tab == 1) {
        /* VirtualBox Integration Tab */
        font_draw_string(mx, my, "Oracle VM VirtualBox Integration", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 20, "VirtualBox Guest Additions protocol & service configuration:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 50, mw, 180, COLOR_RGB(30, 41, 59), g_theme.card_border);
        font_draw_string(mx + 16, my + 66, "[x] Mouse Pointer Integration (Seamless mode)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 94, "[x] Automatic Dynamic Display Resizing", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 122, "[x] Shared Folders Support (/media/sf_shared)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 150, "[x] Host-to-Guest Time Synchronization (RTC)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 178, "[x] VMMDev PCI Communication Channel (Port 0xD020)", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 206, "[x] 2D / 3D Graphics Acceleration Pipeline", g_theme.success, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_button(mx + mw - 160, cy + ch - 44, 160, 32, "Restart Guest Agent", false, false, g_theme.btn_bg);
    } else if (active_tab == 2) {
        /* System Information Tab */
        font_draw_string(mx, my, "System & Hardware Overview", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 36, mw, 220, COLOR_RGB(30, 41, 59), g_theme.card_border);
        font_draw_string(mx + 16, my + 50, "Operating System : LinuxOSZero v1.0.0 (Genesis)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 74, "Architecture     : x86_64 (64-bit Long Mode)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 98, "Hypervisor       : Oracle VM VirtualBox", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 122, "Display Adapter  : VirtualBox VMSVGA (BEEF/CAFE)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 146, "Network Adapter  : Intel 82540EM Gigabit Ethernet", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 170, "Audio Controller : Intel AC'97 / High Definition Audio", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 194, "Package Manager  : zpkg (Zero Package Manager)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 218, "Init System      : ZeroInit (PID 1)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
    } else if (active_tab == 3) {
        /* Appearance Tab */
        font_draw_string(mx, my, "Desktop Theme & Appearance", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 20, "Customize window borders, color scheme and styling:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 50, mw / 2 - 10, 100, COLOR_RGB(15, 23, 42), g_theme.accent_primary);
        font_draw_string(mx + 14, my + 70, "Dark Cyber (Active)", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 14, my + 94, "Cyan accents & slate cards", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx + mw / 2 + 10, my + 50, mw / 2 - 10, 100, COLOR_RGB(241, 245, 249), g_theme.card_border);
        font_draw_string(mx + mw / 2 + 24, my + 70, "Light Clean", COLOR_RGB(15, 23, 42), COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + mw / 2 + 24, my + 94, "Bright slate & blue accents", COLOR_RGB(100, 116, 139), COLOR_RGBA(0, 0, 0, 0));
    }
}

void control_panel_on_event(window_t *win, int ev_type, int p1, int p2) {
    if (ev_type != 1) return;

    /* Tab bar clicks */
    if (p2 < 34) {
        int tab_w = win->width / 4;
        active_tab = p1 / tab_w;
        if (active_tab > 3) active_tab = 3;
        return;
    }

    if (active_tab == 0) {
        /* Resolution selection */
        int list_y = 98;
        for (size_t i = 0; i < RES_COUNT; i++) {
            int ry = list_y + i * 36;
            if (p2 >= ry && p2 < (ry + 30)) {
                selected_res_idx = (int)i;
                return;
            }
        }
    }
}

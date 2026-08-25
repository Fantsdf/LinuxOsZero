/*
 * LinuxOSZero - Taskbar & Start Menu Implementation
 */

#include "zeropanel.h"
#include "../gui/theme.h"
#include "../gui/canvas.h"
#include "../gui/font.h"
#include "../gui/icons.h"
#include "../kernel/kernel.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

zeropanel_t g_panel = {0};

/* External app launcher declarations */
extern void app_launch_installer(void);
extern void app_launch_terminal(void);
extern void app_launch_file_manager(void);
extern void app_launch_control_panel(void);
extern void app_launch_editor(void);
extern void app_launch_fetch(void);

typedef struct {
    const char *title;
    icon_type_t icon;
    void (*launch)(void);
} menu_item_t;

static const menu_item_t menu_items[] = {
    { "Install LinuxOSZero (GUI)",     ICON_INSTALLER,     app_launch_installer },
    { "Zero Terminal",                 ICON_TERMINAL,      app_launch_terminal },
    { "File Manager",                  ICON_FILE_MANAGER,  app_launch_file_manager },
    { "Control Panel & Drivers",       ICON_CONTROL_PANEL, app_launch_control_panel },
    { "Zero Text Editor",              ICON_EDITOR,        app_launch_editor },
    { "System Info (ZeroFetch)",       ICON_SYSTEM_INFO,   app_launch_fetch },
};
#define MENU_ITEM_COUNT (sizeof(menu_items) / sizeof(menu_items[0]))

void panel_init(int screen_w, int screen_h) {
    g_panel.screen_w = screen_w;
    g_panel.screen_h = screen_h;
    g_panel.start_menu_open = false;
    g_panel.calendar_open = false;
    g_panel.hovered_item = -1;
}

void panel_handle_input(mouse_state_t *mouse) {
    int panel_y = g_panel.screen_h - PANEL_HEIGHT;

    /* Start Menu Popup Hit Test */
    if (g_panel.start_menu_open) {
        int menu_w = 260;
        int menu_h = MENU_ITEM_COUNT * 36 + 70;
        int menu_x = 6;
        int menu_y = panel_y - menu_h - 6;

        if (mouse->left_clicked) {
            if (mouse->x >= menu_x && mouse->x < (menu_x + menu_w) &&
                mouse->y >= menu_y && mouse->y < (menu_y + menu_h)) {
                
                /* Check which item was clicked */
                int list_y = menu_y + 50;
                for (size_t i = 0; i < MENU_ITEM_COUNT; i++) {
                    int item_top = list_y + i * 36;
                    if (mouse->y >= item_top && mouse->y < (item_top + 34)) {
                        g_panel.start_menu_open = false;
                        if (menu_items[i].launch) {
                            menu_items[i].launch();
                        }
                        return;
                    }
                }
            } else if (mouse->y < panel_y || mouse->x > 120) {
                /* Clicked outside start menu */
                g_panel.start_menu_open = false;
            }
        }
    }

    /* Panel Clicks */
    if (mouse->left_clicked && mouse->y >= panel_y) {
        /* Start Button Hit Test */
        if (mouse->x >= 6 && mouse->x < 110) {
            g_panel.start_menu_open = !g_panel.start_menu_open;
            return;
        }

        /* Taskbar Items Hit Test */
        int task_x = 120;
        for (int i = 0; i < g_wm.window_count; i++) {
            int wid = g_wm.z_order[i];
            window_t *win = &g_wm.windows[wid];
            if (win->id == -1) continue;

            int task_w = 140;
            if (mouse->x >= task_x && mouse->x < (task_x + task_w)) {
                if (win->state == WIN_STATE_MINIMIZED || !win->is_visible) {
                    wm_restore_window(wid);
                } else if (win->is_active) {
                    wm_minimize_window(wid);
                } else {
                    wm_focus_window(wid);
                }
                return;
            }
            task_x += task_w + 4;
        }
    }
}

void panel_render(void) {
    int panel_y = g_panel.screen_h - PANEL_HEIGHT;

    /* 1. Panel Background */
    fbdev_fill_rect(0, panel_y, g_panel.screen_w, PANEL_HEIGHT, g_theme.panel_bg);
    fbdev_fill_rect(0, panel_y, g_panel.screen_w, 1, g_theme.panel_border);

    /* 2. Start Button */
    bool start_active = g_panel.start_menu_open;
    color_t start_bg = start_active ? g_theme.accent_active : g_theme.card_bg;
    canvas_fill_rounded_rect(6, panel_y + 4, 100, 32, 6, start_bg);
    canvas_draw_rounded_rect(6, panel_y + 4, 100, 32, 6, g_theme.card_border);
    icons_draw_logo(20, panel_y + 20, 10);
    font_draw_string_utf8(36, panel_y + 12, "ZERO OS", g_theme.btn_text, COLOR_RGBA(0, 0, 0, 0));

    /* 3. Taskbar Window Buttons */
    int task_x = 120;
    for (int i = 0; i < g_wm.window_count; i++) {
        int wid = g_wm.z_order[i];
        window_t *win = &g_wm.windows[wid];
        if (win->id == -1) continue;

        int task_w = 140;
        color_t tab_bg = win->is_active ? g_theme.accent_primary : g_theme.card_bg;
        canvas_fill_rounded_rect(task_x, panel_y + 4, task_w, 32, 4, tab_bg);
        canvas_draw_rounded_rect(task_x, panel_y + 4, task_w, 32, 4, g_theme.card_border);

        icons_draw(win->icon, task_x + 6, panel_y + 12, 16, 0);

        char trunc_title[16];
        strncpy(trunc_title, win->title, 12);
        trunc_title[12] = '\0';
        font_draw_string_utf8(task_x + 26, panel_y + 12, trunc_title, g_theme.btn_text, COLOR_RGBA(0, 0, 0, 0));

        task_x += task_w + 4;
    }

    /* 4. System Tray (Right-aligned) */
    int tray_x = g_panel.screen_w - 220;

    /* VirtualBox Badge */
    canvas_fill_rounded_rect(tray_x, panel_y + 8, 64, 24, 4, COLOR_RGB(30, 41, 59));
    canvas_draw_rounded_rect(tray_x, panel_y + 8, 64, 24, 4, COLOR_RGB(51, 65, 85));
    font_draw_string(tray_x + 6, panel_y + 12, "VBox", COLOR_RGB(34, 197, 94), COLOR_RGBA(0, 0, 0, 0));

    /* Network Icon */
    icons_draw(ICON_NETWORK, tray_x + 72, panel_y + 12, 16, COLOR_RGB(34, 197, 94));

    /* Volume Icon */
    icons_draw(ICON_VOLUME, tray_x + 96, panel_y + 12, 16, COLOR_RGB(248, 250, 252));

    /* Clock */
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);
    char time_str[16];
    if (timeinfo) {
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    } else {
        strcpy(time_str, "12:00:00");
    }
    font_draw_string(tray_x + 124, panel_y + 12, time_str, g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

    /* 5. Start Menu Popup Rendering */
    if (g_panel.start_menu_open) {
        int menu_w = 260;
        int menu_h = MENU_ITEM_COUNT * 36 + 70;
        int menu_x = 6;
        int menu_y = panel_y - menu_h - 6;

        /* Shadow & Menu Background */
        canvas_draw_shadow(menu_x, menu_y, menu_w, menu_h, 10, g_theme.win_shadow);
        canvas_fill_rounded_rect(menu_x, menu_y, menu_w, menu_h, 8, g_theme.win_bg);
        canvas_draw_rounded_rect(menu_x, menu_y, menu_w, menu_h, 8, g_theme.card_border);

        /* Menu Header */
        canvas_fill_rounded_rect(menu_x + 2, menu_y + 2, menu_w - 4, 40, 6, g_theme.win_titlebar_active);
        icons_draw_logo(menu_x + 22, menu_y + 22, 12);
        font_draw_string(menu_x + 42, menu_y + 10, "LinuxOSZero v1.0", g_theme.btn_text, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(menu_x + 42, menu_y + 24, "Genesis Desktop", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));

        /* Menu Items */
        int list_y = menu_y + 48;
        for (size_t i = 0; i < MENU_ITEM_COUNT; i++) {
            int item_y = list_y + i * 36;
            canvas_fill_rounded_rect(menu_x + 6, item_y, menu_w - 12, 32, 4, g_theme.card_bg);
            icons_draw(menu_items[i].icon, menu_x + 12, item_y + 8, 16, 0);
            font_draw_string_utf8(menu_x + 36, item_y + 8, menu_items[i].title, g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        }

        /* Power Actions */
        int bot_y = menu_y + menu_h - 22;
        font_draw_string(menu_x + 12, bot_y, "VirtualBox Guest Active", COLOR_RGB(34, 197, 94), COLOR_RGBA(0, 0, 0, 0));
    }
}

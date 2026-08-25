/*
 * LinuxOSZero - File Manager Implementation
 */

#include "file_manager.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include <stdio.h>
#include <string.h>

static window_t *fm_win = NULL;

typedef struct {
    const char *name;
    const char *size;
    const char *type;
    bool is_dir;
} file_item_t;

static const file_item_t file_list[] = {
    { "bin",      "DIR",    "System Binaries", true },
    { "boot",     "DIR",    "Kernel & Bootloader", true },
    { "dev",      "DIR",    "Device Nodes", true },
    { "etc",      "DIR",    "Configuration", true },
    { "home",     "DIR",    "User Directories", true },
    { "lib",      "DIR",    "Shared Libraries", true },
    { "media",    "DIR",    "Mount Points (vboxsf)", true },
    { "proc",     "DIR",    "Process FS", true },
    { "sys",      "DIR",    "Sysfs", true },
    { "usr",      "DIR",    "Userland Programs", true },
    { "var",      "DIR",    "Variable Data", true },
    { "init",     "128 KB", "ZeroInit (PID 1)", false },
};
#define FILE_COUNT (sizeof(file_list) / sizeof(file_list[0]))

void app_launch_file_manager(void) {
    if (fm_win && fm_win->id != -1) {
        wm_focus_window(fm_win->id);
        return;
    }
    fm_win = wm_create_window("Zero File Manager - /", ICON_FILE_MANAGER, 200, 100, 560, 380, file_manager_render, file_manager_on_event);
}

void file_manager_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    fbdev_fill_rect(cx, cy, cw, ch, g_theme.card_bg);

    /* Address / Path Bar */
    int bar_h = 32;
    fbdev_fill_rect(cx, cy, cw, bar_h, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx, cy + bar_h - 1, cw, 1, g_theme.card_border);
    canvas_draw_button(cx + 8, cy + 4, 30, 24, "<", false, false, g_theme.btn_bg);
    canvas_draw_button(cx + 42, cy + 4, 30, 24, ">", false, false, g_theme.btn_bg);

    canvas_fill_rounded_rect(cx + 80, cy + 4, cw - 90, 24, 4, COLOR_RGB(30, 41, 59));
    canvas_draw_rounded_rect(cx + 80, cy + 4, cw - 90, 24, 4, g_theme.card_border);
    font_draw_string(cx + 90, cy + 8, "Location: / (Root Filesystem)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

    /* File List Table */
    int table_y = cy + bar_h + 8;
    int row_h = 24;

    /* Header */
    font_draw_string(cx + 36, table_y, "Name", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(cx + 200, table_y, "Type", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(cx + 400, table_y, "Size", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
    fbdev_fill_rect(cx + 10, table_y + 18, cw - 20, 1, g_theme.card_border);

    int cur_y = table_y + 24;
    for (size_t i = 0; i < FILE_COUNT; i++) {
        if (cur_y + row_h > cy + ch) break;

        if (file_list[i].is_dir) {
            icons_draw(ICON_FILE_MANAGER, cx + 14, cur_y + 4, 16, 0);
            font_draw_string(cx + 36, cur_y + 4, file_list[i].name, g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        } else {
            icons_draw(ICON_EDITOR, cx + 14, cur_y + 4, 16, 0);
            font_draw_string(cx + 36, cur_y + 4, file_list[i].name, g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        }

        font_draw_string(cx + 200, cur_y + 4, file_list[i].type, g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(cx + 400, cur_y + 4, file_list[i].size, g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

        cur_y += row_h;
    }
}

void file_manager_on_event(window_t *win, int ev_type, int p1, int p2) {
    (void)win;
    (void)ev_type;
    (void)p1;
    (void)p2;
}

/*
 * LinuxOSZero - Text Editor Implementation
 */

#include "editor.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include <stdio.h>
#include <string.h>

static window_t *editor_win = NULL;

static const char *code_lines[] = {
    "/* LinuxOSZero - Custom OS Kernel & Desktop */",
    "#include <zero/os.h>",
    "#include <zero/vbox.h>",
    "",
    "int main(void) {",
    "    printf(\"Hello from LinuxOSZero!\\n\");",
    "    vbox_enable_mouse_integration(true);",
    "    zerowm_start_session();",
    "    return 0;",
    "}"
};
#define CODE_LINE_COUNT (sizeof(code_lines) / sizeof(code_lines[0]))

void app_launch_editor(void) {
    if (editor_win && editor_win->id != -1) {
        wm_focus_window(editor_win->id);
        return;
    }
    editor_win = wm_create_window("Zero Editor - main.c", ICON_EDITOR, 220, 80, 560, 360, editor_render, editor_on_event);
}

void editor_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    /* Toolbar */
    int tb_h = 28;
    fbdev_fill_rect(cx, cy, cw, tb_h, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx, cy + tb_h - 1, cw, 1, g_theme.card_border);
    canvas_draw_button(cx + 6, cy + 3, 50, 22, "Save", false, false, g_theme.btn_bg);
    canvas_draw_button(cx + 60, cy + 3, 50, 22, "Open", false, false, g_theme.btn_bg);
    font_draw_string(cx + 120, cy + 6, "main.c  (UTF-8, C Source)", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

    /* Editor Code View */
    int text_y = cy + tb_h;
    int text_h = ch - tb_h - 22;
    fbdev_fill_rect(cx, text_y, cw, text_h, COLOR_RGB(10, 15, 26));

    /* Line Number Gutter */
    int gutter_w = 36;
    fbdev_fill_rect(cx, text_y, gutter_w, text_h, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx + gutter_w - 1, text_y, 1, text_h, g_theme.card_border);

    for (size_t i = 0; i < CODE_LINE_COUNT; i++) {
        int ly = text_y + 6 + i * FONT_HEIGHT;
        if (ly + FONT_HEIGHT > text_y + text_h) break;

        char lnum[8];
        snprintf(lnum, sizeof(lnum), "%2zu", i + 1);
        font_draw_string(cx + 8, ly, lnum, g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

        color_t code_c = g_theme.text_primary;
        if (strstr(code_lines[i], "/*") || strstr(code_lines[i], "//")) {
            code_c = COLOR_RGB(100, 116, 139); /* Comment */
        } else if (strstr(code_lines[i], "#include")) {
            code_c = COLOR_RGB(236, 72, 153);  /* Preprocessor pink */
        } else if (strstr(code_lines[i], "int ") || strstr(code_lines[i], "return ")) {
            code_c = COLOR_RGB(56, 189, 248);  /* Keyword blue */
        } else if (strstr(code_lines[i], "\"")) {
            code_c = COLOR_RGB(34, 197, 94);   /* String green */
        }
        font_draw_string(cx + gutter_w + 10, ly, code_lines[i], code_c, COLOR_RGBA(0, 0, 0, 0));
    }

    /* Status bar at bottom */
    int sb_y = cy + ch - 22;
    fbdev_fill_rect(cx, sb_y, cw, 22, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx, sb_y, cw, 1, g_theme.card_border);
    font_draw_string(cx + 10, sb_y + 3, "Line 1, Col 1 | Spaces: 4 | C (GCC 12)", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
}

void editor_on_event(window_t *win, int ev_type, int p1, int p2) {
    (void)win;
    (void)ev_type;
    (void)p1;
    (void)p2;
}

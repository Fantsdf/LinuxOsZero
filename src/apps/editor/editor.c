/*
 * LinuxOSZero - Interactive Text Editor Implementation
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include "editor.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include "../../drivers/sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EDITOR_LINES 64
#define MAX_LINE_LEN     96

static window_t *editor_win = NULL;

static char text_lines[MAX_EDITOR_LINES][MAX_LINE_LEN] = {
    "/* LinuxOSZero v1.1.0 (Titan) - Custom 64-bit OS */",
    "#include <zero/os.h>",
    "#include <zero/vbox.h>",
    "#include <zero/input.h>",
    "",
    "int main(void) {",
    "    printf(\"Welcome to LinuxOSZero 64-bit!\\n\");",
    "    vbox_enable_mouse_integration(true);",
    "    vbox_set_resolution(1920, 1080, 32);",
    "    zerowm_start_session();",
    "    return 0;",
    "}"
};
static int line_count = 12;
static int cur_line = 6;
static int cur_col = 10;
static char current_filename[64] = "main.c";
static char status_msg[128] = "Ready | UTF-8 | C (GCC 12 x86_64)";
static int ed_blink = 0;

void app_launch_editor(void) {
    if (editor_win && editor_win->id != -1) {
        wm_focus_window(editor_win->id);
        return;
    }
    editor_win = wm_create_window("Zero Editor - main.c (x86_64)", ICON_EDITOR, 220, 80, 600, 420, editor_render, editor_on_event);
}

void editor_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;

    /* Toolbar */
    int tb_h = 28;
    fbdev_fill_rect(cx, cy, cw, tb_h, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx, cy + tb_h - 1, cw, 1, g_theme.card_border);

    canvas_draw_button(cx + 6, cy + 3, 54, 22, "Save", false, false, g_theme.accent_primary);
    canvas_draw_button(cx + 66, cy + 3, 54, 22, "Open", false, false, g_theme.btn_bg);
    canvas_draw_button(cx + 126, cy + 3, 54, 22, "Clear", false, false, g_theme.btn_bg);

    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "%s (UTF-8, %d lines)", current_filename, line_count);
    font_draw_string(cx + 190, cy + 6, title_buf, g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

    /* Editor Code View */
    int text_y = cy + tb_h;
    int text_h = ch - tb_h - 24;
    fbdev_fill_rect(cx, text_y, cw, text_h, COLOR_RGB(10, 15, 26));

    /* Line Number Gutter */
    int gutter_w = 40;
    fbdev_fill_rect(cx, text_y, gutter_w, text_h, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx + gutter_w - 1, text_y, 1, text_h, g_theme.card_border);

    int visible_lines = (text_h - 12) / FONT_HEIGHT;
    if (visible_lines <= 0) visible_lines = 1;

    int top_line = 0;
    if (cur_line >= visible_lines) {
        top_line = cur_line - visible_lines + 1;
    }

    for (int i = top_line; i < line_count && i < top_line + visible_lines; i++) {
        int ly = text_y + 6 + (i - top_line) * FONT_HEIGHT;

        /* Highlight active line */
        if (i == cur_line) {
            fbdev_fill_rect(cx + gutter_w, ly - 1, cw - gutter_w, FONT_HEIGHT, COLOR_RGBA(255, 255, 255, 12));
        }

        /* Gutter number */
        char lnum[16];
        snprintf(lnum, sizeof(lnum), "%2d", i + 1);
        font_draw_string(cx + 8, ly, lnum, (i == cur_line) ? g_theme.accent_primary : g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

        /* Code syntax highlighting */
        color_t code_c = g_theme.text_primary;
        if (strstr(text_lines[i], "/*") || strstr(text_lines[i], "//") || strstr(text_lines[i], " *")) {
            code_c = COLOR_RGB(100, 116, 139); /* Comment */
        } else if (strstr(text_lines[i], "#include") || strstr(text_lines[i], "#define")) {
            code_c = COLOR_RGB(236, 72, 153);  /* Preprocessor pink */
        } else if (strstr(text_lines[i], "int ") || strstr(text_lines[i], "return ") || strstr(text_lines[i], "void ")) {
            code_c = COLOR_RGB(56, 189, 248);  /* Keyword blue */
        } else if (strstr(text_lines[i], "\"")) {
            code_c = COLOR_RGB(34, 197, 94);   /* String green */
        }
        font_draw_string(cx + gutter_w + 10, ly, text_lines[i], code_c, COLOR_RGBA(0, 0, 0, 0));

        /* Cursor rendering on active line */
        if (i == cur_line) {
            ed_blink = (ed_blink + 1) % 60;
            if (ed_blink < 35) {
                char pre[MAX_LINE_LEN];
                int cur_len = (int)strlen(text_lines[i]);
                int col = (cur_col < cur_len) ? cur_col : cur_len;
                if (col >= MAX_LINE_LEN) col = MAX_LINE_LEN - 1;
                memcpy(pre, text_lines[i], col);
                pre[col] = '\0';
                int cur_x = cx + gutter_w + 10 + font_get_string_width(pre);
                fbdev_fill_rect(cur_x, ly, 2, FONT_HEIGHT - 2, COLOR_RGB(56, 189, 248));
            }
        }
    }

    /* Status bar at bottom */
    int sb_y = cy + ch - 24;
    fbdev_fill_rect(cx, sb_y, cw, 24, COLOR_RGB(15, 23, 42));
    fbdev_fill_rect(cx, sb_y, cw, 1, g_theme.card_border);

    char sb_text[256];
    snprintf(sb_text, sizeof(sb_text), "Ln %d, Col %d | %s", cur_line + 1, cur_col + 1, status_msg);
    font_draw_string(cx + 10, sb_y + 4, sb_text, g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
}

void editor_on_event(window_t *win, int ev_type, int p1, int p2) {
    (void)win;

    if (ev_type == WM_EVENT_CLICK) {
        int mx = p1;
        int my = p2;

        if (my < 28) {
            if (mx >= 6 && mx < 60) {
                snprintf(status_msg, sizeof(status_msg), "[Saved] %s written.", current_filename);
                sound_play(SND_SUCCESS);
            } else if (mx >= 66 && mx < 120) {
                snprintf(status_msg, sizeof(status_msg), "[Opened] %s loaded.", current_filename);
                sound_play(SND_CLICK);
            } else if (mx >= 126 && mx < 180) {
                line_count = 1;
                text_lines[0][0] = '\0';
                cur_line = 0;
                cur_col = 0;
                snprintf(status_msg, sizeof(status_msg), "[New] Blank document ready.");
                sound_play(SND_CLICK);
            }
        }
        return;
    }

    if (ev_type == WM_EVENT_KEY_DOWN) {
        int key_code = p1;
        char ascii = (char)p2;

        if (key_code == KEY_UP) {
            if (cur_line > 0) cur_line--;
            int len = (int)strlen(text_lines[cur_line]);
            if (cur_col > len) cur_col = len;
        } else if (key_code == KEY_DOWN) {
            if (cur_line < line_count - 1) cur_line++;
            int len = (int)strlen(text_lines[cur_line]);
            if (cur_col > len) cur_col = len;
        } else if (key_code == KEY_LEFT) {
            if (cur_col > 0) {
                cur_col--;
            } else if (cur_line > 0) {
                cur_line--;
                cur_col = (int)strlen(text_lines[cur_line]);
            }
        } else if (key_code == KEY_RIGHT) {
            int len = (int)strlen(text_lines[cur_line]);
            if (cur_col < len) {
                cur_col++;
            } else if (cur_line < line_count - 1) {
                cur_line++;
                cur_col = 0;
            }
        } else if (key_code == KEY_HOME) {
            cur_col = 0;
        } else if (key_code == KEY_END) {
            cur_col = (int)strlen(text_lines[cur_line]);
        } else if (key_code == KEY_ENTER || ascii == '\n' || ascii == '\r') {
            if (line_count < MAX_EDITOR_LINES - 1) {
                char temp[MAX_LINE_LEN];
                int col = (cur_col < (int)strlen(text_lines[cur_line])) ? cur_col : (int)strlen(text_lines[cur_line]);
                snprintf(temp, sizeof(temp), "%s", &text_lines[cur_line][col]);
                text_lines[cur_line][col] = '\0';

                for (int i = line_count; i > cur_line + 1; i--) {
                    memcpy(text_lines[i], text_lines[i - 1], MAX_LINE_LEN);
                }
                snprintf(text_lines[cur_line + 1], MAX_LINE_LEN, "%s", temp);
                line_count++;
                cur_line++;
                cur_col = 0;
            }
        } else if (key_code == KEY_BACKSPACE || ascii == '\b') {
            if (cur_col > 0) {
                char *line = text_lines[cur_line];
                int len = (int)strlen(line);
                if (cur_col <= len) {
                    memmove(line + cur_col - 1, line + cur_col, (size_t)(len - cur_col + 1));
                    cur_col--;
                }
            } else if (cur_line > 0) {
                int prev_len = (int)strlen(text_lines[cur_line - 1]);
                int cur_len = (int)strlen(text_lines[cur_line]);
                if (prev_len + cur_len < MAX_LINE_LEN - 1) {
                    memcpy(&text_lines[cur_line - 1][prev_len], text_lines[cur_line], (size_t)(cur_len + 1));
                    for (int i = cur_line; i < line_count - 1; i++) {
                        memcpy(text_lines[i], text_lines[i + 1], MAX_LINE_LEN);
                    }
                    line_count--;
                    cur_line--;
                    cur_col = prev_len;
                }
            }
        } else if (key_code == KEY_TAB || ascii == '\t') {
            char *line = text_lines[cur_line];
            int len = (int)strlen(line);
            if (len + 4 < MAX_LINE_LEN - 1 && cur_col >= 0 && cur_col <= len) {
                memmove(line + cur_col + 4, line + cur_col, (size_t)(len - cur_col + 1));
                line[cur_col] = ' ';
                line[cur_col + 1] = ' ';
                line[cur_col + 2] = ' ';
                line[cur_col + 3] = ' ';
                cur_col += 4;
            }
        } else if (ascii >= 32 && ascii <= 126) {
            char *line = text_lines[cur_line];
            int len = (int)strlen(line);
            if (len + 1 < MAX_LINE_LEN - 1 && cur_col >= 0 && cur_col <= len) {
                memmove(line + cur_col + 1, line + cur_col, (size_t)(len - cur_col + 1));
                line[cur_col] = ascii;
                cur_col++;
            }
        }
    }
}

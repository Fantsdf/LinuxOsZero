/*
 * LinuxOSZero - Terminal Emulator Implementation
 */

#include "terminal.h"
#include "../../gui/theme.h"
#include "../../gui/font.h"
#include <stdio.h>
#include <string.h>

#define TERM_LINES 22
#define TERM_LINE_LEN 80

static window_t *term_win = NULL;
static char term_buffer[TERM_LINES][TERM_LINE_LEN];
static int term_line_count = 0;

static void term_add_line(const char *line) {
    if (term_line_count < TERM_LINES) {
        strncpy(term_buffer[term_line_count], line, TERM_LINE_LEN - 1);
        term_line_count++;
    } else {
        for (int i = 0; i < TERM_LINES - 1; i++) {
            strcpy(term_buffer[i], term_buffer[i + 1]);
        }
        strncpy(term_buffer[TERM_LINES - 1], line, TERM_LINE_LEN - 1);
    }
}

static void term_init_content(void) {
    term_line_count = 0;
    term_add_line("LinuxOSZero Terminal (x86_64)");
    term_add_line("Type 'help' for built-in commands or run any system binary.");
    term_add_line("");
    term_add_line("user@linuxoszero:~$ uname -a");
    term_add_line("Linux linuxoszero 6.1.0-zero #1 SMP PREEMPT x86_64 GNU/Linux");
    term_add_line("user@linuxoszero:~$ zero-hwprobe --vbox");
    term_add_line("[*] Hypervisor: Oracle VM VirtualBox (PCI 80ee:cafe / 80ee:beef)");
    term_add_line("[OK] VMSVGA Hardware Acceleration: Active (1024x768x32)");
    term_add_line("[OK] Seamless Mouse Pointer: Enabled");
    term_add_line("user@linuxoszero:~$ zpkg list");
    term_add_line("base-system-1.0.0, zero-wm-1.0, vbox-guest-6.0, zero-apps-1.0");
    term_add_line("user@linuxoszero:~$ _");
}

void app_launch_terminal(void) {
    if (term_win && term_win->id != -1) {
        wm_focus_window(term_win->id);
        return;
    }
    term_init_content();
    term_win = wm_create_window("Zero Terminal - user@linuxoszero", ICON_TERMINAL, 100, 120, 600, 360, terminal_render, terminal_on_event);
}

void terminal_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    /* Deep dark terminal background */
    fbdev_fill_rect(cx, cy, cw, ch, COLOR_RGB(10, 15, 26));

    int pad_x = 10;
    int pad_y = 8;
    for (int i = 0; i < term_line_count; i++) {
        int ly = cy + pad_y + i * FONT_HEIGHT;
        if (ly + FONT_HEIGHT > cy + ch) break;

        color_t col = COLOR_RGB(248, 250, 252);
        if (strstr(term_buffer[i], "user@linuxoszero")) {
            col = COLOR_RGB(56, 189, 248); /* Sky blue prompt */
        } else if (strstr(term_buffer[i], "[OK]")) {
            col = COLOR_RGB(34, 197, 94);  /* Green status */
        } else if (strstr(term_buffer[i], "[*]")) {
            col = COLOR_RGB(234, 179, 8);   /* Yellow info */
        }
        font_draw_string(cx + pad_x, ly, term_buffer[i], col, COLOR_RGBA(0, 0, 0, 0));
    }
}

void terminal_on_event(window_t *win, int ev_type, int p1, int p2) {
    (void)win;
    (void)ev_type;
    (void)p1;
    (void)p2;
}

/*
 * LinuxOSZero - System Information Fetcher (ZeroFetch)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../desktop/zerowm.h"
#include "../../gui/theme.h"
#include "../../gui/font.h"
#include "../../gui/canvas.h"

#ifndef FETCH_CLI_MAIN
static window_t *fetch_win = NULL;

static void fetch_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    fbdev_fill_rect(cx, cy, cw, ch, COLOR_RGB(10, 15, 26));

    /* Draw stylized ASCII Logo on left */
    const char *logo[] = {
        "       .---.       ",
        "      /     \\      ",
        "     | () () |     ",
        "      \\  _  /      ",
        "     .-'   '-.     ",
        "    /  ZERO   \\    ",
        "   |  LINUXOS  |   ",
        "    \\  v1.0   /    ",
        "     '-------'     "
    };
    int logo_lines = sizeof(logo) / sizeof(logo[0]);

    for (int i = 0; i < logo_lines; i++) {
        font_draw_string(cx + 20, cy + 30 + i * FONT_HEIGHT, logo[i], g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
    }

    /* System Details on right */
    int rx = cx + 180;
    int ry = cy + 20;

    font_draw_string(rx, ry,      "user@linuxoszero", g_theme.accent_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 16, "------------------------------------", g_theme.card_border, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 36, "OS         : LinuxOSZero 1.0.0 (Genesis)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 56, "Host       : Oracle VM VirtualBox (vboxguest)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 76, "Kernel     : 6.1.0-zero-x86_64", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 96, "Uptime     : 1 hour, 24 mins", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 116,"Packages   : 42 (zpkg)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 136,"Shell      : zero-sh 1.0", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 156,"Resolution : 1024x768 (VMSVGA)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 176,"WM         : ZeroWM (Double-buffered)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 196,"Theme      : Dark Cyber [Sky Blue]", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(rx, ry + 216,"Memory     : 245 MB / 2048 MB", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

    /* Color blocks at bottom */
    color_t palette[] = {
        COLOR_RGB(239, 68, 68), COLOR_RGB(34, 197, 94), COLOR_RGB(234, 179, 8),
        COLOR_RGB(14, 165, 233), COLOR_RGB(168, 85, 247), COLOR_RGB(236, 72, 153),
        COLOR_RGB(248, 250, 252)
    };
    for (int i = 0; i < 7; i++) {
        fbdev_fill_rect(rx + i * 26, ry + 246, 20, 12, palette[i]);
    }
}

void app_launch_fetch(void) {
    if (fetch_win && fetch_win->id != -1) {
        wm_focus_window(fetch_win->id);
        return;
    }
    fetch_win = wm_create_window("ZeroFetch - System Information", ICON_SYSTEM_INFO, 160, 90, 560, 340, fetch_render, NULL);
}
#endif

#ifdef FETCH_CLI_MAIN
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("\033[1;36m");
    printf("       .---.         \033[1;37muser\033[0m@\033[1;36mlinuxoszero\033[0m\n");
    printf("      /     \\        \033[1;34m------------------------------------\033[0m\n");
    printf("     | () () |       \033[1;32mOS\033[0m         : LinuxOSZero 1.0.0 (Genesis) x86_64\n");
    printf("      \\  _  /        \033[1;32mHost\033[0m       : Oracle VM VirtualBox (vboxguest)\n");
    printf("     .-'   '-.       \033[1;32mKernel\033[0m     : 6.1.0-zero-x86_64\n");
    printf("    /  ZERO   \\      \033[1;32mUptime\033[0m     : 1 hour, 24 mins\n");
    printf("   |  LINUXOS  |     \033[1;32mPackages\033[0m   : 42 (zpkg)\n");
    printf("    \\  v1.0   /      \033[1;32mShell\033[0m      : zero-sh 1.0\n");
    printf("     '-------'       \033[1;32mResolution\033[0m : 1024x768 (VBoxVideo / VMSVGA)\n");
    printf("                     \033[1;32mWM\033[0m         : ZeroWM (Double-Buffered)\n");
    printf("                     \033[1;32mTheme\033[0m      : Dark Cyber [Sky Blue]\n");
    printf("                     \033[1;32mMemory\033[0m     : 245 MB / 2048 MB\n\n");
    printf("   \033[41m   \033[42m   \033[43m   \033[44m   \033[45m   \033[46m   \033[47m   \033[0m\n\n");
    return 0;
}
#endif

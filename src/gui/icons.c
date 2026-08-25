/*
 * LinuxOSZero - Vector Icons Drawing Engine
 */

#include "icons.h"
#include "theme.h"
#include "font.h"

void icons_draw_logo(int cx, int cy, int radius) {
    /* Draw glowing cyan stylized "0" / "Z" logo */
    fbdev_draw_circle(cx, cy, radius, g_theme.accent_primary);
    fbdev_draw_circle(cx, cy, radius - 1, g_theme.accent_hover);
    fbdev_draw_circle(cx, cy, radius - 2, g_theme.accent_active);

    /* Stylized 'Z' inside circle */
    int arm = radius / 2;
    for (int t = -1; t <= 1; t++) {
        fbdev_draw_line(cx - arm, cy - arm + t, cx + arm, cy - arm + t, g_theme.btn_text);
        fbdev_draw_line(cx + arm, cy - arm + t, cx - arm, cy + arm + t, g_theme.accent_hover);
        fbdev_draw_line(cx - arm, cy + arm + t, cx + arm, cy + arm + t, g_theme.btn_text);
    }
}

void icons_draw(icon_type_t type, int x, int y, int size, color_t tint) {
    int pad = size / 6;
    int inner_w = size - pad * 2;
    int inner_h = size - pad * 2;

    switch (type) {
        case ICON_INSTALLER:
            /* CD / Disk with Arrow pointing down */
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w, inner_h, 4, COLOR_RGB(16, 185, 129));
            fbdev_draw_circle(x + size / 2, y + size / 2, size / 4, COLOR_RGB(255, 255, 255));
            fbdev_draw_circle(x + size / 2, y + size / 2, size / 8, COLOR_RGB(16, 185, 129));
            /* Down arrow */
            fbdev_draw_line(x + size / 2, y + pad + 2, x + size / 2, y + size - pad - 2, COLOR_RGB(255, 255, 255));
            fbdev_draw_line(x + size / 2 - 3, y + size - pad - 5, x + size / 2, y + size - pad - 2, COLOR_RGB(255, 255, 255));
            fbdev_draw_line(x + size / 2 + 3, y + size - pad - 5, x + size / 2, y + size - pad - 2, COLOR_RGB(255, 255, 255));
            break;

        case ICON_TERMINAL:
            /* Black console box with `>_` */
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w, inner_h, 3, COLOR_RGB(15, 23, 42));
            canvas_draw_rounded_rect(x + pad, y + pad, inner_w, inner_h, 3, COLOR_RGB(100, 116, 139));
            font_draw_string(x + pad + 3, y + pad + (inner_h - 16) / 2, ">_", COLOR_RGB(34, 197, 94), COLOR_RGBA(0, 0, 0, 0));
            break;

        case ICON_FILE_MANAGER:
            /* Blue Folder */
            canvas_fill_rounded_rect(x + pad, y + pad + 3, inner_w, inner_h - 3, 3, COLOR_RGB(2, 132, 199));
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w / 2, 4, 2, COLOR_RGB(56, 189, 248));
            break;

        case ICON_CONTROL_PANEL:
            /* Gear / Sliders */
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w, inner_h, 4, COLOR_RGB(100, 116, 139));
            fbdev_fill_circle(x + size / 2, y + size / 2, size / 4, COLOR_RGB(248, 250, 252));
            fbdev_fill_circle(x + size / 2, y + size / 2, size / 8, COLOR_RGB(100, 116, 139));
            break;

        case ICON_EDITOR:
            /* Notepad / Document */
            canvas_fill_rounded_rect(x + pad + 2, y + pad, inner_w - 4, inner_h, 2, COLOR_RGB(248, 250, 252));
            fbdev_fill_rect(x + pad + 5, y + pad + 4, inner_w - 10, 2, COLOR_RGB(100, 116, 139));
            fbdev_fill_rect(x + pad + 5, y + pad + 8, inner_w - 10, 2, COLOR_RGB(100, 116, 139));
            fbdev_fill_rect(x + pad + 5, y + pad + 12, inner_w - 14, 2, COLOR_RGB(100, 116, 139));
            break;

        case ICON_SYSTEM_INFO:
            /* Blue Info Circle */
            fbdev_fill_circle(x + size / 2, y + size / 2, size / 2 - 2, COLOR_RGB(14, 165, 233));
            font_draw_string(x + size / 2 - 3, y + (size - 16) / 2, "i", COLOR_RGB(255, 255, 255), COLOR_RGBA(0, 0, 0, 0));
            break;

        case ICON_VIRTUALBOX:
            /* VirtualBox 3D Cube Icon */
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w, inner_h, 3, COLOR_RGB(37, 99, 235));
            fbdev_draw_rect(x + pad + 2, y + pad + 2, inner_w - 4, inner_h - 4, COLOR_RGB(147, 197, 253));
            font_draw_string(x + pad + (inner_w - 8) / 2, y + pad + (inner_h - 16) / 2, "V", COLOR_RGB(255, 255, 255), COLOR_RGBA(0, 0, 0, 0));
            break;

        case ICON_POWER:
            /* Red Power Button */
            fbdev_fill_circle(x + size / 2, y + size / 2, size / 2 - 2, COLOR_RGB(239, 68, 68));
            fbdev_draw_circle(x + size / 2, y + size / 2, size / 4, COLOR_RGB(255, 255, 255));
            fbdev_fill_rect(x + size / 2, y + size / 4, 1, size / 3, COLOR_RGB(255, 255, 255));
            break;

        case ICON_NETWORK:
            /* Network Bars */
            for (int b = 0; b < 4; b++) {
                int bh = 3 + b * 3;
                fbdev_fill_rect(x + pad + b * 4, y + size - pad - bh, 3, bh, tint ? tint : COLOR_RGB(34, 197, 94));
            }
            break;

        case ICON_VOLUME:
            /* Speaker */
            fbdev_fill_rect(x + pad, y + size / 2 - 3, 3, 6, tint ? tint : COLOR_RGB(248, 250, 252));
            fbdev_draw_line(x + pad + 3, y + size / 2 - 3, x + pad + 7, y + pad + 2, tint ? tint : COLOR_RGB(248, 250, 252));
            fbdev_draw_line(x + pad + 3, y + size / 2 + 3, x + pad + 7, y + size - pad - 2, tint ? tint : COLOR_RGB(248, 250, 252));
            fbdev_draw_line(x + pad + 7, y + pad + 2, x + pad + 7, y + size - pad - 2, tint ? tint : COLOR_RGB(248, 250, 252));
            break;

        default:
            canvas_fill_rounded_rect(x + pad, y + pad, inner_w, inner_h, 3, g_theme.card_bg);
            break;
    }
}

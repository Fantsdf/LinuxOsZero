/*
 * LinuxOSZero - Wallpaper Engine
 */

#include "wallpaper.h"
#include "theme.h"
#include "canvas.h"
#include "font.h"
#include "icons.h"

void wallpaper_render(int width, int height) {
    /* 1. Deep Space Dark Gradient: #0b0f19 at top to #1e1b4b / #0f172a at bottom */
    color_t top_c = COLOR_RGB(11, 15, 25);
    color_t bot_c = COLOR_RGB(15, 23, 42);
    canvas_draw_gradient_v(0, 0, width, height, top_c, bot_c);

    /* 2. Cyberpunk subtle grid pattern */
    color_t grid_c = COLOR_RGBA(56, 189, 248, 12);
    int grid_step = 40;
    for (int x = 0; x < width; x += grid_step) {
        fbdev_fill_rect(x, 0, 1, height, grid_c);
    }
    for (int y = 0; y < height; y += grid_step) {
        fbdev_fill_rect(0, y, width, 1, grid_c);
    }

    /* 3. Glowing geometric accent lines */
    color_t glow1 = COLOR_RGBA(14, 165, 233, 40);
    color_t glow2 = COLOR_RGBA(99, 102, 241, 35);
    fbdev_draw_line(0, height / 3, width, height / 2, glow1);
    fbdev_draw_line(0, (2 * height) / 3, width, height / 2, glow2);

    /* 4. Centered LinuxOSZero Emblem & Branding */
    int cx = width / 2;
    int cy = height / 2 - 40;

    /* Outer glowing circles */
    for (int r = 50; r <= 56; r++) {
        uint8_t a = (56 - r) * 15;
        fbdev_draw_circle(cx, cy, r, COLOR_RGBA(14, 165, 233, a));
    }
    icons_draw_logo(cx, cy, 48);

    /* Brand typography */
    const char *title = "LinuxOSZero";
    const char *subtitle = "Genesis Edition v1.0.0 (x86_64)";
    const char *tagline = "Minimalist . Modular . Blazingly Fast";

    int tw1 = font_get_string_width(title);
    int tw2 = font_get_string_width(subtitle);
    int tw3 = font_get_string_width(tagline);

    font_draw_string(cx - tw1 / 2, cy + 65, title, COLOR_RGB(248, 250, 252), COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(cx - tw2 / 2, cy + 85, subtitle, g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
    font_draw_string(cx - tw3 / 2, cy + 105, tagline, g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));
}

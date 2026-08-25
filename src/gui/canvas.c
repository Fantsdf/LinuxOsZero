/*
 * LinuxOSZero - Canvas Implementation
 */

#include "canvas.h"
#include <string.h>
#include <stdlib.h>

void canvas_fill_rounded_rect(int x, int y, int w, int h, int radius, color_t color) {
    if (w <= 0 || h <= 0) return;
    if (radius <= 0) {
        fbdev_fill_rect(x, y, w, h, color);
        return;
    }
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;

    /* Center rectangle */
    fbdev_fill_rect(x + radius, y, w - 2 * radius, h, color);
    /* Left and right side rectangles */
    fbdev_fill_rect(x, y + radius, radius, h - 2 * radius, color);
    fbdev_fill_rect(x + w - radius, y + radius, radius, h - 2 * radius, color);

    /* 4 Rounded corners */
    for (int cy = -radius; cy <= radius; cy++) {
        for (int cx = -radius; cx <= radius; cx++) {
            if (cx * cx + cy * cy <= radius * radius) {
                if (cx < 0 && cy < 0) fbdev_set_pixel(x + radius + cx, y + radius + cy, color);
                if (cx >= 0 && cy < 0) fbdev_set_pixel(x + w - radius + cx - 1, y + radius + cy, color);
                if (cx < 0 && cy >= 0) fbdev_set_pixel(x + radius + cx, y + h - radius + cy - 1, color);
                if (cx >= 0 && cy >= 0) fbdev_set_pixel(x + w - radius + cx - 1, y + h - radius + cy - 1, color);
            }
        }
    }
}

void canvas_draw_rounded_rect(int x, int y, int w, int h, int radius, color_t color) {
    if (w <= 0 || h <= 0) return;
    if (radius <= 0) {
        fbdev_draw_rect(x, y, w, h, color);
        return;
    }
    /* Top & bottom edges */
    fbdev_fill_rect(x + radius, y, w - 2 * radius, 1, color);
    fbdev_fill_rect(x + radius, y + h - 1, w - 2 * radius, 1, color);
    /* Left & right edges */
    fbdev_fill_rect(x, y + radius, 1, h - 2 * radius, color);
    fbdev_fill_rect(x + w - 1, y + radius, 1, h - 2 * radius, color);
}

void canvas_draw_shadow(int x, int y, int w, int h, int blur, color_t shadow_color) {
    if (blur <= 0) return;
    uint8_t base_alpha = COLOR_GET_A(shadow_color);
    for (int i = 1; i <= blur; i++) {
        uint8_t a = (base_alpha * (blur - i + 1)) / (blur * 2);
        color_t col = COLOR_RGBA(0, 0, 0, a);
        fbdev_draw_rect(x - i, y - i + 2, w + 2 * i, h + 2 * i, col);
    }
}

void canvas_draw_card(int x, int y, int w, int h, color_t bg_color, color_t border_color) {
    canvas_fill_rounded_rect(x, y, w, h, 6, bg_color);
    canvas_draw_rounded_rect(x, y, w, h, 6, border_color);
}

void canvas_draw_gradient_v(int x, int y, int w, int h, color_t top_col, color_t bot_col) {
    if (w <= 0 || h <= 0) return;
    uint32_t r1 = COLOR_GET_R(top_col), g1 = COLOR_GET_G(top_col), b1 = COLOR_GET_B(top_col);
    uint32_t r2 = COLOR_GET_R(bot_col), g2 = COLOR_GET_G(bot_col), b2 = COLOR_GET_B(bot_col);

    for (int cy = 0; cy < h; cy++) {
        uint32_t r = r1 + ((r2 - r1) * cy) / h;
        uint32_t g = g1 + ((g2 - g1) * cy) / h;
        uint32_t b = b1 + ((b2 - b1) * cy) / h;
        color_t line_col = COLOR_RGB(r, g, b);
        fbdev_fill_rect(x, y + cy, w, 1, line_col);
    }
}

void canvas_draw_gradient_h(int x, int y, int w, int h, color_t left_col, color_t right_col) {
    if (w <= 0 || h <= 0) return;
    uint32_t r1 = COLOR_GET_R(left_col), g1 = COLOR_GET_G(left_col), b1 = COLOR_GET_B(left_col);
    uint32_t r2 = COLOR_GET_R(right_col), g2 = COLOR_GET_G(right_col), b2 = COLOR_GET_B(right_col);

    for (int cx = 0; cx < w; cx++) {
        uint32_t r = r1 + ((r2 - r1) * cx) / w;
        uint32_t g = g1 + ((g2 - g1) * cx) / w;
        uint32_t b = b1 + ((b2 - b1) * cx) / w;
        color_t line_col = COLOR_RGB(r, g, b);
        fbdev_fill_rect(x + cx, y, 1, h, line_col);
    }
}

void canvas_draw_button(int x, int y, int w, int h, const char *text, bool is_hovered, bool is_active, color_t base_color) {
    color_t bg = base_color;
    if (is_active) {
        bg = g_theme.accent_active;
    } else if (is_hovered) {
        bg = g_theme.accent_hover;
    }

    canvas_fill_rounded_rect(x, y, w, h, 4, bg);
    canvas_draw_rounded_rect(x, y, w, h, 4, is_hovered ? g_theme.accent_primary : g_theme.card_border);

    if (text) {
        int text_w = font_get_string_width(text);
        int tx = x + (w - text_w) / 2;
        int ty = y + (h - FONT_HEIGHT) / 2;
        font_draw_string_utf8(tx, ty, text, g_theme.btn_text, COLOR_RGBA(0, 0, 0, 0));
    }
}

void canvas_draw_progress_bar(int x, int y, int w, int h, int percent, color_t fg_col, color_t bg_col) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    canvas_fill_rounded_rect(x, y, w, h, h / 2, bg_col);
    canvas_draw_rounded_rect(x, y, w, h, h / 2, g_theme.card_border);

    int fill_w = (w * percent) / 100;
    if (fill_w > 0) {
        canvas_fill_rounded_rect(x, y, fill_w, h, h / 2, fg_col);
    }
}

void canvas_draw_badge(int x, int y, const char *text, color_t bg_col, color_t text_col) {
    if (!text) return;
    int tw = font_get_string_width(text);
    int pad_x = 6;
    int pad_y = 2;
    int bw = tw + pad_x * 2;
    int bh = FONT_HEIGHT + pad_y * 2;

    canvas_fill_rounded_rect(x, y, bw, bh, 4, bg_col);
    font_draw_string_utf8(x + pad_x, y + pad_y, text, text_col, COLOR_RGBA(0, 0, 0, 0));
}

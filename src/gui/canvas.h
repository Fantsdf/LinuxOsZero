/*
 * LinuxOSZero - 2D Canvas & Advanced Drawing Primitives
 */

#ifndef CANVAS_H
#define CANVAS_H

#include "../drivers/fbdev.h"
#include "theme.h"
#include "font.h"

void canvas_draw_rounded_rect(int x, int y, int w, int h, int radius, color_t color);
void canvas_fill_rounded_rect(int x, int y, int w, int h, int radius, color_t color);
void canvas_draw_shadow(int x, int y, int w, int h, int blur, color_t shadow_color);
void canvas_draw_card(int x, int y, int w, int h, color_t bg_color, color_t border_color);
void canvas_draw_gradient_v(int x, int y, int w, int h, color_t top_col, color_t bot_col);
void canvas_draw_gradient_h(int x, int y, int w, int h, color_t left_col, color_t right_col);
void canvas_draw_button(int x, int y, int w, int h, const char *text, bool is_hovered, bool is_active, color_t base_color);
void canvas_draw_progress_bar(int x, int y, int w, int h, int percent, color_t fg_col, color_t bg_col);
void canvas_draw_badge(int x, int y, const char *text, color_t bg_col, color_t text_col);

#endif /* CANVAS_H */

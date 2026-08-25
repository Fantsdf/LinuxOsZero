/*
 * LinuxOSZero - Font Subsystem & Cyrillic/Latin Glyph Engine
 */

#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stddef.h>
#include "../drivers/fbdev.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

/* 8x16 glyph raster table */
extern const uint8_t g_font_data[256][16];

void font_draw_char(int x, int y, unsigned char c, color_t fg, color_t bg);
void font_draw_string(int x, int y, const char *str, color_t fg, color_t bg);
void font_draw_string_utf8(int x, int y, const char *utf8_str, color_t fg, color_t bg);
int font_get_string_width(const char *utf8_str);

#endif /* FONT_H */

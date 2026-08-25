/*
 * LinuxOSZero - Font Engine Implementation
 */

#include "font.h"
#include <string.h>

/* Standard 8x16 VGA / BIOS bitmap font with Cyrillic & Latin support */
#include "font_data.inl"

void font_draw_char(int x, int y, unsigned char c, color_t fg, color_t bg) {
    const uint8_t *glyph = g_font_data[c];
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (bits & (0x80 >> col)) {
                fbdev_set_pixel(x + col, y + row, fg);
            } else if (COLOR_GET_A(bg) > 0) {
                fbdev_set_pixel(x + col, y + row, bg);
            }
        }
    }
}

void font_draw_string(int x, int y, const char *str, color_t fg, color_t bg) {
    if (!str) return;
    int cur_x = x;
    int cur_y = y;

    while (*str) {
        if (*str == '\n') {
            cur_x = x;
            cur_y += FONT_HEIGHT;
        } else if (*str == '\t') {
            cur_x += FONT_WIDTH * 4;
        } else {
            font_draw_char(cur_x, cur_y, (unsigned char)*str, fg, bg);
            cur_x += FONT_WIDTH;
        }
        str++;
    }
}

/* Decode UTF-8 Cyrillic & Latin characters */
void font_draw_string_utf8(int x, int y, const char *utf8_str, color_t fg, color_t bg) {
    if (!utf8_str) return;
    const unsigned char *s = (const unsigned char *)utf8_str;
    int cur_x = x;
    int cur_y = y;

    while (*s) {
        if (*s == '\n') {
            cur_x = x;
            cur_y += FONT_HEIGHT;
            s++;
        } else if (*s == '\t') {
            cur_x += FONT_WIDTH * 4;
            s++;
        } else if (*s < 0x80) {
            /* ASCII */
            font_draw_char(cur_x, cur_y, *s, fg, bg);
            cur_x += FONT_WIDTH;
            s++;
        } else if ((*s == 0xD0 || *s == 0xD1) && *(s + 1)) {
            /* UTF-8 2-byte Cyrillic: D0 90..BF (А..п), D0 81 (Ё), D1 80..8F (р..я), D1 91 (ё) */
            uint8_t byte1 = *s;
            uint8_t byte2 = *(s + 1);
            unsigned char cp_char = '?';

            if (byte1 == 0xD0) {
                if (byte2 == 0x81) cp_char = 0xF0; /* Ё */
                else if (byte2 >= 0x90 && byte2 <= 0xBF) cp_char = (byte2 - 0x90) + 0x80; /* А..п in CP866 */
            } else if (byte1 == 0xD1) {
                if (byte2 == 0x91) cp_char = 0xF1; /* ё */
                else if (byte2 >= 0x80 && byte2 <= 0x8F) cp_char = (byte2 - 0x80) + 0xE0; /* р..я in CP866 */
            }

            font_draw_char(cur_x, cur_y, cp_char, fg, bg);
            cur_x += FONT_WIDTH;
            s += 2;
        } else {
            /* Skip unknown multibyte */
            s++;
        }
    }
}

int font_get_string_width(const char *utf8_str) {
    if (!utf8_str) return 0;
    int len = 0;
    const unsigned char *s = (const unsigned char *)utf8_str;
    while (*s) {
        if (*s < 0x80) {
            len++;
            s++;
        } else if ((*s & 0xE0) == 0xC0) {
            len++;
            s += 2;
        } else if ((*s & 0xF0) == 0xE0) {
            len++;
            s += 3;
        } else {
            s++;
        }
    }
    return len * FONT_WIDTH;
}

/* Provide font drawing aliases for fbdev */
void fbdev_draw_char(int x, int y, char c, color_t fg, color_t bg) {
    font_draw_char(x, y, (unsigned char)c, fg, bg);
}

void fbdev_draw_string(int x, int y, const char *str, color_t fg, color_t bg) {
    font_draw_string(x, y, str, fg, bg);
}

void fbdev_draw_string_utf8(int x, int y, const char *utf8_str, color_t fg, color_t bg) {
    font_draw_string_utf8(x, y, utf8_str, fg, bg);
}

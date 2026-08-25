/*
 * LinuxOSZero - Framebuffer Graphics Driver (FBDEV / Direct Render)
 */

#ifndef FBDEV_H
#define FBDEV_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* 32-bit ARGB Color */
typedef uint32_t color_t;

#define COLOR_RGBA(r, g, b, a)  (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))
#define COLOR_RGB(r, g, b)      COLOR_RGBA(r, g, b, 255)
#define COLOR_GET_A(c)          (((c) >> 24) & 0xFF)
#define COLOR_GET_R(c)          (((c) >> 16) & 0xFF)
#define COLOR_GET_G(c)          (((c) >> 8) & 0xFF)
#define COLOR_GET_B(c)          ((c) & 0xFF)

/* Rectangle structure */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} rect_t;

/* Framebuffer device context */
typedef struct {
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    size_t size;
    uint32_t *fb_mem;        /* Direct mapped memory */
    uint32_t *back_buffer;   /* Double buffering for tear-free rendering */
    bool is_simulated;
} fbdev_t;

extern fbdev_t g_fbdev;

int fbdev_init(const char *dev_path, uint32_t default_w, uint32_t default_h);
void fbdev_close(void);
void fbdev_clear(color_t color);
void fbdev_set_pixel(int x, int y, color_t color);
color_t fbdev_get_pixel(int x, int y);
void fbdev_fill_rect(int x, int y, int width, int height, color_t color);
void fbdev_draw_rect(int x, int y, int width, int height, color_t color);
void fbdev_draw_line(int x0, int y0, int x1, int y1, color_t color);
void fbdev_draw_circle(int cx, int cy, int radius, color_t color);
void fbdev_fill_circle(int cx, int cy, int radius, color_t color);
void fbdev_draw_char(int x, int y, char c, color_t fg, color_t bg);
void fbdev_draw_string(int x, int y, const char *str, color_t fg, color_t bg);
void fbdev_draw_string_utf8(int x, int y, const char *utf8_str, color_t fg, color_t bg);
void fbdev_blit_buffer(int dx, int dy, int dw, int dh, const uint32_t *src, int sw, int sh);
void fbdev_swap_buffers(void);

#endif /* FBDEV_H */

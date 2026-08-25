/*
 * LinuxOSZero - Framebuffer Graphics Driver Implementation
 */

#include "fbdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

fbdev_t g_fbdev = {0};

/* 8x16 Bitmap Font */
extern const uint8_t font_8x16_data[256][16];

int fbdev_init(const char *dev_path, uint32_t default_w, uint32_t default_h) {
    g_fbdev.fd = open(dev_path ? dev_path : "/dev/fb0", O_RDWR);
    if (g_fbdev.fd >= 0) {
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;

        if (ioctl(g_fbdev.fd, FBIOGET_FSCREENINFO, &finfo) == 0 &&
            ioctl(g_fbdev.fd, FBIOGET_VSCREENINFO, &vinfo) == 0) {
            g_fbdev.width = vinfo.xres;
            g_fbdev.height = vinfo.yres;
            g_fbdev.bpp = vinfo.bits_per_pixel;
            g_fbdev.pitch = finfo.line_length;
            g_fbdev.size = finfo.smem_len;
            g_fbdev.fb_mem = (uint32_t *)mmap(0, g_fbdev.size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fbdev.fd, 0);
            g_fbdev.is_simulated = false;
        }
    }

    /* Fallback to in-memory allocated back buffer if fb0 is unavailable */
    if (!g_fbdev.fb_mem) {
        g_fbdev.width = default_w ? default_w : 1024;
        g_fbdev.height = default_h ? default_h : 768;
        g_fbdev.bpp = 32;
        g_fbdev.pitch = g_fbdev.width * 4;
        g_fbdev.size = g_fbdev.pitch * g_fbdev.height;
        g_fbdev.fb_mem = (uint32_t *)calloc(1, g_fbdev.size);
        g_fbdev.is_simulated = true;
    }

    g_fbdev.back_buffer = (uint32_t *)calloc(1, g_fbdev.size);
    return 0;
}

void fbdev_close(void) {
    if (!g_fbdev.is_simulated && g_fbdev.fb_mem) {
        munmap(g_fbdev.fb_mem, g_fbdev.size);
    } else if (g_fbdev.fb_mem) {
        free(g_fbdev.fb_mem);
    }
    if (g_fbdev.back_buffer) {
        free(g_fbdev.back_buffer);
    }
    if (g_fbdev.fd >= 0) {
        close(g_fbdev.fd);
    }
    memset(&g_fbdev, 0, sizeof(g_fbdev));
}

void fbdev_clear(color_t color) {
    uint32_t *dst = g_fbdev.back_buffer;
    size_t count = (size_t)g_fbdev.width * g_fbdev.height;
    for (size_t i = 0; i < count; i++) {
        dst[i] = color;
    }
}

void fbdev_set_pixel(int x, int y, color_t color) {
    if (x < 0 || x >= (int)g_fbdev.width || y < 0 || y >= (int)g_fbdev.height) return;

    uint8_t a = COLOR_GET_A(color);
    if (a == 255) {
        g_fbdev.back_buffer[y * g_fbdev.width + x] = color;
    } else if (a > 0) {
        /* Alpha Blending */
        color_t bg = g_fbdev.back_buffer[y * g_fbdev.width + x];
        uint32_t inv_a = 255 - a;
        uint32_t r = (COLOR_GET_R(color) * a + COLOR_GET_R(bg) * inv_a) / 255;
        uint32_t g = (COLOR_GET_G(color) * a + COLOR_GET_G(bg) * inv_a) / 255;
        uint32_t b = (COLOR_GET_B(color) * a + COLOR_GET_B(bg) * inv_a) / 255;
        g_fbdev.back_buffer[y * g_fbdev.width + x] = COLOR_RGB(r, g, b);
    }
}

color_t fbdev_get_pixel(int x, int y) {
    if (x < 0 || x >= (int)g_fbdev.width || y < 0 || y >= (int)g_fbdev.height) return 0;
    return g_fbdev.back_buffer[y * g_fbdev.width + x];
}

void fbdev_fill_rect(int x, int y, int width, int height, color_t color) {
    if (width <= 0 || height <= 0) return;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + width) > (int)g_fbdev.width ? (int)g_fbdev.width : (x + width);
    int y1 = (y + height) > (int)g_fbdev.height ? (int)g_fbdev.height : (y + height);

    uint8_t a = COLOR_GET_A(color);
    if (a == 255) {
        for (int cy = y0; cy < y1; cy++) {
            uint32_t *row = &g_fbdev.back_buffer[cy * g_fbdev.width + x0];
            int span = x1 - x0;
            for (int cx = 0; cx < span; cx++) {
                row[cx] = color;
            }
        }
    } else {
        for (int cy = y0; cy < y1; cy++) {
            for (int cx = x0; cx < x1; cx++) {
                fbdev_set_pixel(cx, cy, color);
            }
        }
    }
}

void fbdev_draw_rect(int x, int y, int width, int height, color_t color) {
    if (width <= 0 || height <= 0) return;
    fbdev_fill_rect(x, y, width, 1, color);
    fbdev_fill_rect(x, y + height - 1, width, 1, color);
    fbdev_fill_rect(x, y, 1, height, color);
    fbdev_fill_rect(x + width - 1, y, 1, height, color);
}

void fbdev_draw_line(int x0, int y0, int x1, int y1, color_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        fbdev_set_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void fbdev_fill_circle(int cx, int cy, int radius, color_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                fbdev_set_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void fbdev_draw_circle(int cx, int cy, int radius, color_t color) {
    int x = radius, y = 0;
    int err = 0;

    while (x >= y) {
        fbdev_set_pixel(cx + x, cy + y, color);
        fbdev_set_pixel(cx + y, cy + x, color);
        fbdev_set_pixel(cx - y, cy + x, color);
        fbdev_set_pixel(cx - x, cy + y, color);
        fbdev_set_pixel(cx - x, cy - y, color);
        fbdev_set_pixel(cx - y, cy - x, color);
        fbdev_set_pixel(cx + y, cy - x, color);
        fbdev_set_pixel(cx + x, cy - y, color);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void fbdev_blit_buffer(int dx, int dy, int dw, int dh, const uint32_t *src, int sw, int sh) {
    for (int y = 0; y < dh && (dy + y) < (int)g_fbdev.height; y++) {
        if (dy + y < 0) continue;
        int sy = (y * sh) / dh;
        for (int x = 0; x < dw && (dx + x) < (int)g_fbdev.width; x++) {
            if (dx + x < 0) continue;
            int sx = (x * sw) / dw;
            color_t c = src[sy * sw + sx];
            fbdev_set_pixel(dx + x, dy + y, c);
        }
    }
}

void fbdev_swap_buffers(void) {
    if (g_fbdev.fb_mem && g_fbdev.back_buffer) {
        memcpy(g_fbdev.fb_mem, g_fbdev.back_buffer, g_fbdev.size);
    }
}

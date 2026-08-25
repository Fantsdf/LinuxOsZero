/*
 * LinuxOSZero - VGA / Console Output Driver
 */

#include "kernel.h"
#include <stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((volatile uint16_t *)0xB8000)

static size_t vga_row = 0;
static size_t vga_column = 0;
static uint8_t vga_color = 0x07; /* Light grey on black */

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = 0x0F; /* White on black */
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', vga_color);
        }
    }
}

static void vga_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = VGA_BUFFER[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

static void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    if (c == '\r') {
        vga_column = 0;
        return;
    }

    const size_t index = vga_row * VGA_WIDTH + vga_column;
    VGA_BUFFER[index] = vga_entry(c, vga_color);
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

void vga_puts(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

static void print_number(uint64_t n, int base) {
    char buf[32];
    int i = 0;
    static const char digits[] = "0123456789ABCDEF";

    if (n == 0) {
        vga_putchar('0');
        return;
    }

    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }

    while (i > 0) {
        vga_putchar(buf[--i]);
    }
}

void vga_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            vga_putchar(fmt[i]);
            continue;
        }

        i++;
        switch (fmt[i]) {
            case 's': {
                const char *s = va_arg(args, const char *);
                vga_puts(s ? s : "(null)");
                break;
            }
            case 'd':
            case 'i': {
                int d = va_arg(args, int);
                if (d < 0) {
                    vga_putchar('-');
                    d = -d;
                }
                print_number((uint64_t)d, 10);
                break;
            }
            case 'u': {
                unsigned int u = va_arg(args, unsigned int);
                print_number((uint64_t)u, 10);
                break;
            }
            case 'x':
            case 'p': {
                uint64_t x = va_arg(args, uint64_t);
                vga_puts("0x");
                print_number(x, 16);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                vga_putchar(c);
                break;
            }
            case '%': {
                vga_putchar('%');
                break;
            }
            default:
                vga_putchar('%');
                vga_putchar(fmt[i]);
                break;
        }
    }

    va_end(args);
}

/*
 * LinuxOSZero - Kernel Core Headers
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define OS_NAME        "LinuxOSZero"
#define OS_VERSION     "1.0.0"
#define OS_CODENAME    "Genesis"
#define OS_ARCH        "x86_64"

/* System info structure */
typedef struct {
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t screen_pitch;
    uint32_t screen_bpp;
    uint32_t *framebuffer;
    uint64_t total_memory_kb;
    uint64_t free_memory_kb;
    bool is_virtualbox;
    bool is_qemu;
    bool is_vmware;
    char cpu_vendor[16];
    char cpu_brand[64];
    uint32_t cpu_cores;
} system_info_t;

extern system_info_t g_sysinfo;

/* I/O Port functions */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

/* Interrupt handling */
static inline void cli(void) { __asm__ volatile ("cli"); }
static inline void sti(void) { __asm__ volatile ("sti"); }
static inline void hlt(void) { __asm__ volatile ("hlt"); }

/* Kernel subsystems */
void gdt_init(void);
void idt_init(void);
void pci_init(void);
void vga_init(void);
void vga_puts(const char *str);
void vga_printf(const char *fmt, ...);
void kernel_main(void);

#endif /* KERNEL_H */

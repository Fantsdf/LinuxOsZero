/*
 * LinuxOSZero - GDT (Global Descriptor Table)
 * Architecture: x86_64
 *
 * Configures the 64-bit Long Mode Global Descriptor Table:
 *   Selector 0x00: Null Descriptor
 *   Selector 0x08: 32-bit Compatibility Code Segment (Ring 0)
 *   Selector 0x10: 64-bit Kernel Data Segment (Ring 0)
 *   Selector 0x18: 64-bit Kernel Code Segment (Ring 0, L=1, D=0)
 *   Selector 0x20: 64-bit User Code Segment (Ring 3, L=1, D=0)
 *   Selector 0x28: 64-bit User Data Segment (Ring 3)
 */

#include "kernel.h"

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static uint64_t gdt[6] __attribute__((aligned(16))) = {
    0x0000000000000000ULL, /* 0x00: Null */
    0x00CF9A000000FFFFULL, /* 0x08: 32-bit Code (Ring 0) */
    0x00CF92000000FFFFULL, /* 0x10: 64-bit/32-bit Data (Ring 0) */
    0x00209A0000000000ULL, /* 0x18: 64-bit Kernel Code (Ring 0, L=1, D=0) */
    0x0020FA0000000000ULL, /* 0x20: 64-bit User Code (Ring 3, L=1, D=0) */
    0x00CFF2000000FFFFULL  /* 0x28: 64-bit User Data (Ring 3) */
};

static struct gdt_ptr gp __attribute__((aligned(16)));

void gdt_init(void) {
    gp.limit = (uint16_t)(sizeof(gdt) - 1);
    gp.base = (uint64_t)&gdt;

    __asm__ volatile (
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        : : "r"(&gp) : "rax", "memory"
    );
}

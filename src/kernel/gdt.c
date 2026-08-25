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

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr gp;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    gp.limit = (uint16_t)(sizeof(struct gdt_entry) * 6 - 1);
    gp.base = (uint64_t)&gdt;

    /* Selector 0x00: Null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Selector 0x08: 32-bit Compatibility Code (Ring 0) */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Selector 0x10: 64-bit Kernel Data (Ring 0) */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Selector 0x18: 64-bit Kernel Code (Ring 0, L=1, D=0) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0x9A, 0xAF);

    /* Selector 0x20: 64-bit User Code (Ring 3, L=1, D=0) */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xAF);

    /* Selector 0x28: 64-bit User Data (Ring 3) */
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xF2, 0xCF);

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

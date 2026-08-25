/*
 * LinuxOSZero - IDT (Interrupt Descriptor Table) & CPU Exceptions
 */

#include "kernel.h"

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_hi;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr ip;

static void default_isr(void) {
    /* Generic interrupt acknowledge */
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

static void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_mid = ((base >> 16) & 0xFFFF);
    idt[num].base_hi = ((base >> 32) & 0xFFFFFFFF);
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

void idt_init(void) {
    ip.limit = (sizeof(struct idt_entry) * 256) - 1;
    ip.base = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, (uint64_t)default_isr, 0x08, 0x8E);
    }

    /* Remap PIC */
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    outb(0x21, 0x20); /* Master PIC vector offset 32 */
    io_wait();
    outb(0xA1, 0x28); /* Slave PIC vector offset 40 */
    io_wait();
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    __asm__ volatile ("lidt (%0)" : : "r"(&ip));
}

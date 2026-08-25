/*
 * LinuxOSZero - IDT (Interrupt Descriptor Table) & CPU Exceptions
 * Architecture: x86_64
 */

#include "kernel.h"
#include "keyboard.h"

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  ist;        /* Interrupt Stack Table index & reserved */
    uint8_t  flags;      /* Type and attributes (0x8E = 64-bit Interrupt Gate) */
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

static void irq1_keyboard_isr(void) {
    keyboard_isr();
}

static void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (uint16_t)(base & 0xFFFF);
    idt[num].base_mid = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].base_hi = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    idt[num].sel = sel;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

void idt_init(void) {
    ip.limit = (uint16_t)(sizeof(struct idt_entry) * 256 - 1);
    ip.base = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, (uint64_t)default_isr, 0x18, 0x8E);
    }

    /* Remap Master and Slave 8259 PIC */
    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    outb(0x21, 0x20); /* Master PIC vector offset 32 (IRQ 0..7 -> INT 32..39) */
    io_wait();
    outb(0xA1, 0x28); /* Slave PIC vector offset 40 (IRQ 8..15 -> INT 40..47) */
    io_wait();
    outb(0x21, 0x04); /* Master PIC: Slave at IRQ2 */
    io_wait();
    outb(0xA1, 0x02); /* Slave PIC: Cascade identity */
    io_wait();
    outb(0x21, 0x01); /* 8086/88 mode */
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    /* Unmask IRQ 1 (Keyboard) and IRQ 2 (Cascade) on Master PIC */
    outb(0x21, 0xFD); /* 11111101b - IRQ1 unmasked */
    outb(0xA1, 0xFF); /* Mask all on Slave PIC */

    /* Set IRQ1 Keyboard Handler (INT 33 = 0x21) */
    idt_set_gate(0x21, (uint64_t)irq1_keyboard_isr, 0x18, 0x8E);

    __asm__ volatile ("lidt (%0)" : : "r"(&ip));
}

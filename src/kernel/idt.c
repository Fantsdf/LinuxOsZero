/*
 * LinuxOSZero - IDT (Interrupt Descriptor Table) & CPU Exceptions
 * Architecture: x86_64
 *
 * Implements 64-bit IDT configuration and 8259 PIC remapping.
 */

#include "kernel.h"
#include "keyboard.h"

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  ist;        /* Interrupt Stack Table index & reserved */
    uint8_t  flags;      /* Type and attributes (0x8E = 64-bit Interrupt Gate, DPL 0) */
    uint16_t base_mid;
    uint32_t base_hi;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

typedef struct {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t int_no;
    uint64_t err_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) interrupt_frame_t;

static struct idt_entry idt[256] __attribute__((aligned(16)));
static struct idt_ptr ip __attribute__((aligned(16)));

extern void *isr_stub_table[256];

void isr_handler(interrupt_frame_t *frame) {
    if (frame->int_no == 0x21) {
        /* IRQ 1 - PS/2 Keyboard */
        keyboard_isr();
    } else if (frame->int_no >= 0x20 && frame->int_no <= 0x2F) {
        /* Generic Hardware IRQ acknowledgment to 8259 PIC */
        if (frame->int_no >= 0x28) {
            outb(0xA0, 0x20); /* Slave PIC EOI */
        }
        outb(0x20, 0x20);     /* Master PIC EOI */
    }
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
    ip.limit = (uint16_t)(sizeof(idt) - 1);
    ip.base = (uint64_t)&idt;

    /* Populate all 256 gates FIRST so every vector has a valid 64-bit handler */
    for (int i = 0; i < 256; i++) {
        idt_set_gate((uint8_t)i, (uint64_t)isr_stub_table[i], 0x18, 0x8E);
    }

    /* Load 64-bit IDTR immediately so IDT is active before PIC remapping */
    __asm__ volatile ("lidt (%0)" : : "r"(&ip) : "memory");

    /* Remap Master and Slave 8259 PIC to vectors 0x20..0x2F */
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

    /* Unmask IRQ 1 (Keyboard) and IRQ 2 (Cascade) on Master PIC, mask all on Slave */
    outb(0x21, 0xFD); /* 11111101b - IRQ1 unmasked */
    outb(0xA1, 0xFF); /* Mask all on Slave PIC */
}

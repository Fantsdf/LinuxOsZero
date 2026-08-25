/*
 * LinuxOSZero - 64-bit Interrupt Service Routine (ISR) Stubs
 * Architecture: x86_64
 *
 * Implements 64-bit IDT gate ISR stubs for all 256 interrupt vectors,
 * properly preserving general-purpose registers and returning via iretq.
 */

.intel_syntax noprefix
.altmacro
.text

.macro ISR_NOERRCODE num
.global isr_stub_\num
isr_stub_\num:
    push 0                  /* Dummy error code */
    push \num               /* Interrupt vector number */
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
.global isr_stub_\num
isr_stub_\num:
    push \num               /* Interrupt vector number */
    jmp isr_common_stub
.endm

/* CPU Exceptions (Vectors 0..31) */
ISR_NOERRCODE 0   /* #DE Divide-by-zero Error */
ISR_NOERRCODE 1   /* #DB Debug */
ISR_NOERRCODE 2   /* NMI Non-Maskable Interrupt */
ISR_NOERRCODE 3   /* #BP Breakpoint */
ISR_NOERRCODE 4   /* #OF Overflow */
ISR_NOERRCODE 5   /* #BR BOUND Range Exceeded */
ISR_NOERRCODE 6   /* #UD Invalid Opcode (Undefined Opcode) */
ISR_NOERRCODE 7   /* #NM Device Not Available (No Math Coprocessor) */
ISR_ERRCODE   8   /* #DF Double Fault (with error code) */
ISR_NOERRCODE 9   /* Coprocessor Segment Overrun */
ISR_ERRCODE   10  /* #TS Invalid TSS (with error code) */
ISR_ERRCODE   11  /* #NP Segment Not Present (with error code) */
ISR_ERRCODE   12  /* #SS Stack-Segment Fault (with error code) */
ISR_ERRCODE   13  /* #GP General Protection Fault (with error code) */
ISR_ERRCODE   14  /* #PF Page Fault (with error code) */
ISR_NOERRCODE 15  /* Reserved */
ISR_NOERRCODE 16  /* #MF x87 FPU Floating-Point Error (Math Fault) */
ISR_ERRCODE   17  /* #AC Alignment Check (with error code) */
ISR_NOERRCODE 18  /* #MC Machine Check */
ISR_NOERRCODE 19  /* #XM SIMD Floating-Point Exception */
ISR_NOERRCODE 20  /* #VE Virtualization Exception */
ISR_ERRCODE   21  /* #CP Control Protection Exception */
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29  /* #VC VMM Communication Exception */
ISR_ERRCODE   30  /* #SX Security Exception */
ISR_NOERRCODE 31

/* Vectors 32..255 (Hardware IRQs & Software Interrupts) */
.set i, 32
.rept 224
    ISR_NOERRCODE %i
    .set i, i + 1
.endr

.extern isr_handler

isr_common_stub:
    /* Save all 64-bit general purpose registers */
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    /* 1st parameter to isr_handler(interrupt_frame_t *frame) in RDI */
    mov rdi, rsp
    cld
    call isr_handler

    /* Restore all 64-bit general purpose registers */
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    /* Pop int_no and error code from stack */
    add rsp, 16

    /* Return from 64-bit interrupt */
    iretq

/* 256-entry table of ISR stub function pointers */
.section .rodata
.align 16
.global isr_stub_table
.macro STUB_PTR num
    .quad isr_stub_\num
.endm

isr_stub_table:
.set j, 0
.rept 256
    STUB_PTR %j
    .set j, j + 1
.endr

.section .note.GNU-stack,"",@progbits

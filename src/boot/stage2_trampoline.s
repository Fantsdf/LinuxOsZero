/*
 * LinuxOSZero - Stage 2 Trampoline (16 -> 32 -> 64 bit)
 *
 * Loaded by boot.s at physical 0x10000 (real mode, CS=0x1000, IP=0).
 *
 * Because the whole kernel is linked at 0x10000, the 16-bit portion reaches
 * its own data through DS=0x1000 (base 0x10000). All displacement arithmetic
 * uses (label - _start) which fits in 16 bits, and far jumps use absolute
 * physical addresses (0x10000 + offset) so EIP is correct once in flat mode.
 *
 * Steps:
 *   1. Build 4 GB identity page tables (1 GB pages) at 0x9000
 *   2. Load a minimal GDT (hand-encoded LGDT with 16-bit displacement)
 *   3. Protected mode -> PAE + EFER.LME + Paging -> long mode
 *   4. call stage2_entry (kernel C entry)
 */

.intel_syntax noprefix
.global _start
.global stage2_entry

.code16
_start:
    cli

    /* DS = 0x1000 so data inside the kernel (physical 0x10000+) is reachable
       with 16-bit offsets = (label - _start). */
    mov ax, 0x1000
    mov ds, ax

    /* ---- Hand-encoded LGDT [gdt_desc - _start]  (0x0F 0x01 0x16 disp16) ---- */
    .byte 0x0F
    .byte 0x01
    .byte 0x16
    .word gdt_desc - _start

    /* ---- Enter Protected Mode ---- */
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    /* Far jump to 32-bit code: EIP = 0x10000 + (pm32 - _start).
       Encoded as EA imm32 seg16 with a 32-bit operand override. */
    .byte 0x66
    .byte 0xEA
    .long 0x10000 + (pm32 - _start)
    .word 0x08

.code32
pm32:
    /* Flat 32-bit segment reload. */
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    /* ---- Build 4 GB identity page tables at 0x9000 (flat addressing) ---- */
    mov edi, 0x9000
    xor eax, eax
    mov ecx, 0x800          /* 2048 dwords = 8 KB */
    cld
    rep stosd

    /* PML4[0] -> PDPT at 0x9100 (present, rw) */
    mov dword ptr [0x9000], 0x00009103

    /* PDPT[0..3] = 1 GB pages (PS=1), identity-map 0..4 GB.
       The framebuffer at 0xE0000000 is thus reachable. */
    mov dword ptr [0x9100], 0x00000083
    mov dword ptr [0x9108], 0x40000083
    mov dword ptr [0x9110], 0x80000083
    mov dword ptr [0x9118], 0xC0000083

    /* ---- Enable PAE ---- */
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    /* ---- Load PML4 ---- */
    mov eax, 0x9000
    mov cr3, eax

    /* ---- Enable Long Mode (EFER.LME = bit 8) ---- */
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    /* ---- Enable Paging ---- */
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    /* Far jump to 64-bit code: EIP = 0x10000 + (pm64 - _start). */
    .byte 0xEA
    .long 0x10000 + (pm64 - _start)
    .word 0x18

.code64
pm64:
    mov rsp, 0x100000
    call stage2_entry

.halt_loop:
    hlt
    jmp .halt_loop

/* ---- Minimal GDT (aligned to 8) ---- */
.align 8
gdt:
    .quad 0x0000000000000000   /* NULL          */
    .quad 0x00CF9A000000FFFF   /* code32 (0x08) */
    .quad 0x00CF92000000FFFF   /* data   (0x10) */
    .quad 0x00209A0000000000   /* code64 (0x18), L=1 */
gdt_end:

gdt_desc:
    .word gdt_end - gdt - 1
    .long 0x10000 + (gdt - _start)   /* linear base = physical address of GDT */

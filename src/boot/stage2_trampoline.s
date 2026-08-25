/*
 * LinuxOSZero - Stage 2 Trampoline (16 -> 32 -> 64 bit)
 *
 * Loaded by boot.s at physical 0x10000 (real mode, CS=0x1000, IP=0).
 *
 * This version uses 2 MiB pages (PSE) instead of 1 GiB pages. 1 GiB pages
 * (PDPTE with PS=1) are NOT supported by every VirtualBox guest CPUID
 * (e.g. when the VM is configured without the Page1GB feature), which caused
 * a page-fault -> double fault -> triple fault ("Guru Meditation") right at
 * the far jump into long mode, i.e. a black screen. 2 MiB pages are supported
 * on every x86-64 CPU, so this boots reliably in VirtualBox/QEMU/bare metal.
 *
 * Page table layout (physical, at 0x9000):
 *   PML4 @ 0x9000   [0] -> PDPT @ 0x9100
 *   PDPT @ 0x9100   [0..3] -> PD0..PD3
 *   PD0  @ 0x9200   : 512 x 2 MiB pages = 0x00000000 - 0x3FFFFFFF
 *   PD1  @ 0x9400   : maps 0x40000000 - 0x7FFFFFFF
 *   PD2  @ 0x9600   : maps 0x80000000 - 0xBFFFFFFF
 *   PD3  @ 0x9800   : maps 0xC0000000 - 0xFFFFFFFF   (framebuffer 0xE0000000)
 *
 * Because the whole kernel is linked at 0x10000, the 16-bit portion reaches
 * its own data through DS=0x1000 (base 0x10000). All displacement arithmetic
 * uses (label - _start) which fits in 16 bits, and far jumps use absolute
 * physical addresses (0x10000 + offset).
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

    /* Far jump to 32-bit code: EIP = 0x10000 + (pm32 - _start). */
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

    /* ---- Build 2 MiB-page identity tables at 0x9000 ----
       Zero PML4 + PDPT + 4 PDs = 24 KB = 0x9000..0xF000 (6144 dwords). */
    mov edi, 0x9000
    xor eax, eax
    mov ecx, 0x1800
    cld
    rep stosd

    /* PML4[0] -> PDPT at 0x9100 (present, rw) */
    mov dword ptr [0x9000], 0x00009103

    /* PDPT[0..3] -> PD0..PD3 (present, rw) */
    mov dword ptr [0x9100], 0x00009203
    mov dword ptr [0x9108], 0x00009403
    mov dword ptr [0x9110], 0x00009603
    mov dword ptr [0x9118], 0x00009803

    /* Fill all four PDs (2048 entries) with 2 MiB pages (PS=1, present, rw).
       Entry value = (index << 21) | 0x83. Covers the full 4 GiB so the
       framebuffer at 0xE0000000 is identity-mapped. */
    mov edi, 0x9200          /* start of PD0 */
    mov eax, 0x00000083      /* first 2 MiB page (0x00000000) */
    mov ecx, 2048
.fill_pd:
    mov dword ptr [edi], eax
    add eax, 0x200000        /* advance 2 MiB */
    add edi, 4
    loop .fill_pd

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

    /* Far jump to 64-bit code: EIP = 0x10000 + (pm64 - _start), CS = 0x18. */
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

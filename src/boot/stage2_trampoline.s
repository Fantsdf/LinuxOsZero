/*
 * LinuxOSZero - Stage 2 Trampoline (16 -> 32 -> 64 bit)
 *
 * Loaded by boot.s at physical 0x10000 (real mode, CS=0x1000, IP=0).
 *
 * HISTORY / ROOT CAUSE (VirtualBox "Guru Meditation 1155 triple fault",
 * eip=0x100b1, cr2=0x100b1, "Guest GDT read error VERR_PAGE_TABLE_NOT_PRESENT"):
 *   Earlier versions placed the page tables at 0x9000, 0x9100, 0x9200, 0x9400...
 *   i.e. only 0x100 bytes apart -- NOT 4 KiB aligned. In a PML4E/PDPTE the
 *   physical base field is bits 12-51 (frame number = addr>>12), so e.g. the
 *   value 0x9103 yielded frame 0x9000 instead of 0x9100. The CPU therefore
 *   walked tables at addresses where they were not actually written -> page
 *   fault -> double fault -> triple fault -> black screen.
 *
 * FIX: every table sits on its own 4 KiB boundary:
 *     PML4  @ 0x9000
 *     PDPT  @ 0xA000
 *     PD0   @ 0xB000   (512 x 2 MiB pages = 0x00000000 - 0x3FFFFFFF)
 *     PD1   @ 0xC000   (0x40000000 - 0x7FFFFFFF)
 *     PD2   @ 0xD000   (0x80000000 - 0xBFFFFFFF)
 *     PD3   @ 0xE000   (0xC0000000 - 0xFFFFFFFF, framebuffer 0xE0000000)
 * 2 MiB pages (PSE) are supported on every x86-64 CPU (unlike 1 GiB pages).
 *
 * Because the whole kernel is linked at 0x10000, the 16-bit portion reaches
 * its own data through DS=0x1000 (base 0x10000). All displacement arithmetic
 * uses (label - _start), and far jumps use absolute physical offsets.
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

    /* ---- Zero 0x9000..0xF000 (24 KB = 6 x 4 KiB tables) ---- */
    mov edi, 0x9000
    xor eax, eax
    mov ecx, 0x1800          /* 6144 dwords = 24 KB */
    cld
    rep stosd

    /* ---- PML4 @ 0x9000 : [0] -> PDPT @ 0xA000 (frame 0xA000, present,rw) ---- */
    mov dword ptr [0x9000], 0x0000A003

    /* ---- PDPT @ 0xA000 : [0..3] -> PD0..PD3 ---- */
    mov dword ptr [0xA000], 0x0000B003   /* PD0 @ 0xB000 */
    mov dword ptr [0xA008], 0x0000C003   /* PD1 @ 0xC000 */
    mov dword ptr [0xA010], 0x0000D003   /* PD2 @ 0xD000 */
    mov dword ptr [0xA018], 0x0000E003   /* PD3 @ 0xE000 */

    /* ---- Fill PD0 @ 0xB000 with 512 x 2 MiB pages (PS=1, present, rw) ----
       Each PD covers 1 GiB; entry = (index << 21) | 0x83. */
    mov edi, 0xB000          /* start of PD0 */
    mov eax, 0x00000083      /* first 2 MiB page (0x00000000) */
    mov ecx, 512
.fill_pd0:
    mov dword ptr [edi], eax
    add eax, 0x200000
    add edi, 4
    loop .fill_pd0

    /* ---- Fill PD1 @ 0xC000 (base 0x40000000) ---- */
    mov edi, 0xC000
    mov eax, 0x40000083
    mov ecx, 512
.fill_pd1:
    mov dword ptr [edi], eax
    add eax, 0x200000
    add edi, 4
    loop .fill_pd1

    /* ---- Fill PD2 @ 0xD000 (base 0x80000000) ---- */
    mov edi, 0xD000
    mov eax, 0x80000083
    mov ecx, 512
.fill_pd2:
    mov dword ptr [edi], eax
    add eax, 0x200000
    add edi, 4
    loop .fill_pd2

    /* ---- Fill PD3 @ 0xE000 (base 0xC0000000) ---- */
    mov edi, 0xE000
    mov eax, 0xC0000083
    mov ecx, 512
.fill_pd3:
    mov dword ptr [edi], eax
    add eax, 0x200000
    add edi, 4
    loop .fill_pd3

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

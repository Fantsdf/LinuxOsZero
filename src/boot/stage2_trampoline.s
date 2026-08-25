/*
 * LinuxOSZero - Stage 2 Trampoline (16 -> 32 -> 64 bit Long Mode)
 *
 * Loaded by boot.s at physical 0x10000 (real mode, CS=0x1000, IP=0).
 *
 * ============================================================================
 * CRITICAL VIRTUALBOX & X86_64 FIX:
 * In x86_64 Long Mode (4-level paging / PAE), every entry in PML4, PDPT,
 * Page Directory (PD), and Page Table (PT) is 64-BIT (8 BYTES).
 *
 * Earlier iterations had `add edi, 4` in the PD fill loop, which corrupted
 * the top 32 bits of every Page Directory Entry with the lower 32 bits of
 * the subsequent entry! When paging was enabled, the CPU walked invalid
 * page frames (address > 500 TB), causing immediate Guru Meditation /
 * Page Fault / Triple Fault on boot. VirtualBox's GUI frontend DisplayWrap
 * then received E_UNEXPECTED (0x8000ffff) / -52 (VERR_NOT_SUPPORTED) because
 * the VM died before video modesetting completed.
 *
 * FIX:
 * Every table sits on its own 4 KiB boundary:
 *     PML4  @ 0x9000 (8 bytes per entry)
 *     PDPT  @ 0xA000 (8 bytes per entry)
 *     PD0   @ 0xB000 (512 x 8-byte PDEs = 0x00000000 - 0x3FFFFFFF, 1 GiB)
 *     PD1   @ 0xC000 (512 x 8-byte PDEs = 0x40000000 - 0x7FFFFFFF, 1 GiB)
 *     PD2   @ 0xD000 (512 x 8-byte PDEs = 0x80000000 - 0xBFFFFFFF, 1 GiB)
 *     PD3   @ 0xE000 (512 x 8-byte PDEs = 0xC0000000 - 0xFFFFFFFF, 1 GiB)
 *
 * Total 4 GiB identity mapped using 2 MiB large pages (PS=1, bit 7).
 * Framebuffer at 0xE0000000 and all hardware MMIO / RAM in 0..4GB are
 * cleanly reachable.
 * ============================================================================
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

    /* ---- Detect 64-bit (long mode) support via CPUID ----
       If the guest is configured as 32-bit only (VirtualBox: Enable64bit=0,
       Long Mode=0), enabling PAE below raises #GP -> double/triple fault.
       Check CPUID 0x80000001 bit 29 (LM) first and print a clear message
       instead of crashing. */
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_longmode
    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000      /* bit 29 = LM (long mode) */
    jnz .longmode_ok

.no_longmode:
    /* Write "REQUIRES 64-BIT CPU / enable 64-bit in VirtualBox" to VGA text
       buffer at 0xB8000 (row 10, columns 0..68), cyan on black. */
    mov edi, 0xB8000 + 10 * 160
    mov eax, 0x0B000000
    mov byte ptr [edi], 'L'
    mov byte ptr [edi+1], 0x0B
    lea esi, [msg_no64]        /* DS=0x10 flat, so esi = physical addr of msg */
    cld
.next_c:
    lodsb
    test al, al
    jz .halt64
    mov ah, 0x0B
    mov word ptr [edi], ax
    add edi, 2
    jmp .next_c
.halt64:
    hlt
    jmp .halt64

.longmode_ok:
    /* ---- Zero 0x9000..0xF000 (24 KB = 6 x 4 KiB tables) ---- */
    mov edi, 0x9000
    xor eax, eax
    mov ecx, 0x1800          /* 6144 dwords = 24 KB */
    cld
    rep stosd

    /* ---- PML4 @ 0x9000 : [0] -> PDPT @ 0xA000 (frame 0xA000, present,rw) ----
       PML4E is 8 bytes: low dword = 0x0000A003, high dword = 0x00000000 */
    mov dword ptr [0x9000], 0x0000A003
    mov dword ptr [0x9004], 0x00000000

    /* ---- PDPT @ 0xA000 : [0..3] -> PD0..PD3 (8 bytes each) ---- */
    mov dword ptr [0xA000], 0x0000B003   /* PD0 @ 0xB000 */
    mov dword ptr [0xA004], 0x00000000
    mov dword ptr [0xA008], 0x0000C003   /* PD1 @ 0xC000 */
    mov dword ptr [0xA00C], 0x00000000
    mov dword ptr [0xA010], 0x0000D003   /* PD2 @ 0xD000 */
    mov dword ptr [0xA014], 0x00000000
    mov dword ptr [0xA018], 0x0000E003   /* PD3 @ 0xE000 */
    mov dword ptr [0xA01C], 0x00000000

    /* ---- Fill PD0 @ 0xB000 with 512 x 2 MiB pages (PS=1, present, rw) ----
       Each 64-bit PDE is 8 bytes:
       [edi]   = (index * 2MB) | 0x83 (Present, Writable, HugePage 2MB)
       [edi+4] = 0x00000000
       edi is incremented by 8! */
    mov edi, 0xB000          /* start of PD0 */
    mov eax, 0x00000083      /* first 2 MiB page (0x00000000) */
    mov ecx, 512
.fill_pd0:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd0

    /* ---- Fill PD1 @ 0xC000 (base 0x40000000) ---- */
    mov edi, 0xC000
    mov eax, 0x40000083
    mov ecx, 512
.fill_pd1:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd1

    /* ---- Fill PD2 @ 0xD000 (base 0x80000000) ---- */
    mov edi, 0xD000
    mov eax, 0x80000083
    mov ecx, 512
.fill_pd2:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd2

    /* ---- Fill PD3 @ 0xE000 (base 0xC0000000) ---- */
    mov edi, 0xE000
    mov eax, 0xC0000083
    mov ecx, 512
.fill_pd3:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd3

    /* ---- Enable PAE in CR4 (bit 5) and PGE (bit 7) ---- */
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    /* ---- Load PML4 address into CR3 ---- */
    mov eax, 0x9000
    mov cr3, eax

    /* ---- Enable Long Mode in IA32_EFER MSR (0xC0000080, bit 8 = LME) ---- */
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    /* ---- Enable Paging (CR0.PG = bit 31, CR0.PE = bit 0) ---- */
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    /* Far jump to 64-bit code: EIP = 0x10000 + (pm64 - _start), CS = 0x18. */
    .byte 0xEA
    .long 0x10000 + (pm64 - _start)
    .word 0x18

.code64
pm64:
    /* Reload data segment selectors in 64-bit Long Mode */
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    /* Set up 64-bit kernel stack (1MB physical, 16-byte aligned) */
    mov rsp, 0x100000
    and rsp, -16

    /* Jump to Stage 2 C kernel entry */
    call stage2_entry

.halt_loop:
    hlt
    jmp .halt_loop

/* ---- Minimal GDT (aligned to 8) ---- */
.align 8
gdt:
    .quad 0x0000000000000000   /* NULL selector (0x00) */
    .quad 0x00CF9A000000FFFF   /* code32 (0x08)        */
    .quad 0x00CF92000000FFFF   /* data   (0x10)        */
    .quad 0x00209A0000000000   /* code64 (0x18), L=1   */
gdt_end:

gdt_desc:
    .word gdt_end - gdt - 1
    .long 0x10000 + (gdt - _start)   /* linear base = physical address of GDT */

/* Clear on-screen message shown when the guest CPU is not 64-bit capable
   (e.g. VirtualBox VM created as 32-bit "Other", Enable64bit=0, Long Mode=0).
   Reached via lea in pm32; DS=0x10 flat so the VMA equals the physical
   address where the kernel is loaded (0x10000+). */
msg_no64:
    .asciz "LinuxOSZero needs a 64-bit CPU. Enable 64-bit in VirtualBox: create the VM as 'Other Linux (64-bit)'. Booting failed."

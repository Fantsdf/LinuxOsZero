/*
 * LinuxOSZero - Stage 1 Boot Sector (512 Bytes)
 *
 * This is the El Torito no-emulation boot image. The BIOS loads the whole
 * boot image (boot.bin + kernel64.bin) at 0x7C00. boot.bin occupies 0x7C00
 * and the kernel follows immediately at 0x7E00.
 *
 * Backward copy (std; rep movsb) is used to copy the 64 KB kernel from 0x7E00
 * to 0x10000 with ZERO overlap corruption.
 */

.code16
.intel_syntax noprefix
.global _start

.text
_start:
    jmp short boot_code
    nop

    /* ---- BPB (kept for disk-boot compatibility) ---- */
    .ascii "ZEROOS  "
    .word  512
    .byte  1
    .word  1
    .byte  2
    .word  224
    .word  2880
    .byte  0xF0
    .word  9
    .word  18
    .word  2
    .long  0
    .long  0
    .byte  0
    .byte  0
    .byte  0x29
    .long  0x12345678
    .ascii "LINUXOSZERO"
    .ascii "FAT12   "

boot_code:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, offset msg_booting
    call print_str

    /* ---- Try to set a graphics mode. Store info at 0x6000 on success. ---- */
    mov byte ptr [0x6000], 0      /* default: text mode */

    /* Query Mode 0x118 info into 0x5000 */
    mov ax, 0x4F01
    mov cx, 0x0118
    mov di, 0x5000
    int 0x10

    /* Try VBE Mode 0x118 (1024x768 with Linear Frame Buffer 0x4000) */
    mov ax, 0x4F02
    mov bx, 0x4118                /* 1024x768 LFB */
    int 0x10
    cmp ax, 0x004F
    jne .try_800
    mov byte ptr [0x6000], 1
    /* Save width, height, bpp, pitch, fb base */
    mov ax, [0x5012]              /* XResolution */
    mov [0x6002], ax
    mov ax, [0x5014]              /* YResolution */
    mov [0x6004], ax
    mov al, [0x5019]              /* BitsPerPixel */
    mov [0x6006], al
    mov ax, [0x5010]              /* BytesPerScanLine (pitch) */
    mov [0x6008], ax
    mov eax, [0x5028]             /* PhysBasePtr */
    mov [0x600C], eax
    jmp .vbe_done

.try_800:
    /* Query Mode 0x115 info into 0x5000 */
    mov ax, 0x4F01
    mov cx, 0x0115
    mov di, 0x5000
    int 0x10

    mov ax, 0x4F02
    mov bx, 0x4115                /* 800x600 LFB */
    int 0x10
    cmp ax, 0x004F
    jne .vbe_done
    mov byte ptr [0x6000], 1
    mov ax, [0x5012]
    mov [0x6002], ax
    mov ax, [0x5014]
    mov [0x6004], ax
    mov al, [0x5019]
    mov [0x6006], al
    mov ax, [0x5010]
    mov [0x6008], ax
    mov eax, [0x5028]
    mov [0x600C], eax

.vbe_done:

    /* ---- Enable Fast A20 Gate ---- */
    in al, 0x92
    or al, 2
    out 0x92, al

    /* ---- Copy kernel from 0x7E00 to 0x10000 (up to 64 KB) using backward copy ----
       ds:si = 0x07E0:0xFFFF , es:di = 0x1000:0xFFFF */
    mov ax, 0x07E0
    mov ds, ax
    mov ax, 0x1000
    mov es, ax
    mov si, 0xFFFF
    mov di, 0xFFFF
    xor cx, cx              /* 65536 bytes */
    std                     /* backward copy prevents overlap corruption */
    rep movsb
    cld                     /* restore forward direction */

    /* ---- Restore DS=0 for messaging and jump to the trampoline ---- */
    xor ax, ax
    mov ds, ax
    mov si, offset msg_ok
    call print_str

    ljmp 0x1000:0x0000

print_str:
    pusha
.next:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 7
    int 0x10
    jmp .next
.done:
    popa
    ret

boot_drive:
    .byte 0x80

msg_booting:
    .asciz "LinuxOSZero v1.1.0 Booting...\r\n"
msg_ok:
    .asciz "Kernel x64 OK.\r\n"

.org 510
.word 0xAA55

/*
 * LinuxOSZero - Stage 1 Boot Sector (512 Bytes)
 *
 * This is the El Torito no-emulation boot image. The BIOS loads the whole
 * boot image (boot.bin + kernel64.bin) at 0x7C00. boot.bin occupies 0x7C00
 * and the kernel follows immediately at 0x7E00, so there are NO raw disk
 * reads needed -- this makes booting in VirtualBox/QEMU reliable (fixes the
 * black screen caused by reading the wrong ISO sectors).
 *
 * Flow: set a video mode -> enable A20 -> copy kernel 0x7E00->0x10000 ->
 *       far jump to 0x10000 (trampoline switches to 64-bit long mode).
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

    /* ---- Try to set a graphics mode. Store 1 at 0x6000 on success so the
          64-bit kernel knows it can draw to the framebuffer (0xE0000000). ---- */
    mov byte ptr [0x6000], 0      /* default: text mode */

    /* Try VBE Mode 0x118 (1024x768x32 with Linear Frame Buffer 0x4000) */
    mov ax, 0x4F02
    mov bx, 0x4118                /* 1024x768x32 LFB */
    int 0x10
    cmp ax, 0x004F
    jne .try_800
    mov byte ptr [0x6000], 1
    jmp .vbe_done

.try_800:
    mov ax, 0x4F02
    mov bx, 0x4115                /* 800x600x32 LFB */
    int 0x10
    cmp ax, 0x004F
    jne .try_640
    mov byte ptr [0x6000], 1
    jmp .vbe_done

.try_640:
    mov ax, 0x4F02
    mov bx, 0x4112                /* 640x480x32 LFB */
    int 0x10
    cmp ax, 0x004F
    jne .vbe_done
    mov byte ptr [0x6000], 1

.vbe_done:

    /* ---- Enable Fast A20 Gate ---- */
    in al, 0x92
    or al, 2
    out 0x92, al

    /* ---- Copy kernel from 0x7E00 to 0x10000 (32 KB) ----
       ds:si = 0x07E0:0x0000 , es:di = 0x1000:0x0000 */
    mov ax, 0x07E0
    mov ds, ax
    xor si, si
    mov ax, 0x1000
    mov es, ax
    xor di, di
    mov cx, 0x8000          /* 32768 bytes = max kernel image size */
    cld
    rep movsb

    /* ---- Restore DS for messaging and jump to the trampoline ---- */
    mov ax, 0x1000
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

/*
 * LinuxOSZero - Master Boot Record / Stage 1 Boot Sector (512 Bytes)
 * Architecture: x86 16-bit Real Mode
 */

.code16
.intel_syntax noprefix
.global _start

.text
_start:
    jmp short boot_code
    nop

    /* BPB Header */
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

    /* Print welcome message */
    mov si, offset msg_booting
    call print_str

    /* Try VESA VBE linear framebuffer (Mode 0x118: 1024x768x32 or Mode 0x115: 800x600x32) */
    mov ax, 0x4F02
    mov bx, 0x4118
    int 0x10
    cmp ax, 0x004F
    je .vbe_done

    mov ax, 0x4F02
    mov bx, 0x4115
    int 0x10
.vbe_done:

    /* Enable Fast A20 */
    in al, 0x92
    or al, 2
    out 0x92, al

    /* Read Stage 2 from disk (sectors 2..64) into 0x1000:0x0000 (0x10000) */
    mov si, offset dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc .read_err

    mov si, offset msg_ok
    call print_str

    /* Jump to Stage 2 at 0x1000:0x0000 */
    jmp 0x1000:0x0000

.read_err:
    /* Fallback standard INT 13h read */
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 64
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jnc .jump_stage2

    mov si, offset msg_err
    call print_str
.hang:
    hlt
    jmp .hang

.jump_stage2:
    jmp 0x1000:0x0000

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

.align 4
dap:
    .byte 0x10
    .byte 0x00
    .word 64
    .word 0x0000
    .word 0x1000
    .quad 1

boot_drive:
    .byte 0x80

msg_booting:
    .asciz "\r\n[+] LinuxOSZero v1.0 Booting...\r\n"
msg_ok:
    .asciz "[OK] OS Loaded.\r\n"
msg_err:
    .asciz "[ERR] Disk Read Error.\r\n"

.org 510
.word 0xAA55

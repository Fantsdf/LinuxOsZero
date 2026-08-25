/*
 * LinuxOSZero - Stage 2 Trampoline & Kernel Launcher
 */

.intel_syntax noprefix
.global _start
.global stage2_trampoline

.text
.code64
_start:
stage2_trampoline:
    /* Set up stack */
    mov rsp, 0x100000

    /* Call C entry point */
    call stage2_entry

.halt_loop:
    hlt
    jmp .halt_loop

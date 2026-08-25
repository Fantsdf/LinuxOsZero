# LinuxOSZero Release 1.1.0 "Titan" (x86_64) Documentation

Please refer to the main release document at [RELEASE_1.1.md](../RELEASE_1.1.md) for full technical documentation, VirtualBox error resolution notes, keyboard driver specifications, and architecture diagrams.

## Summary of Changes
- **DisplayWrap Error Fix**: Corrected 64-bit PAE page table construction (8-byte PDEs) in `src/boot/stage2_trampoline.s`, preventing VirtualBox `E_UNEXPECTED (0x8000ffff) / -52`.
- **Keyboard Subsystem**: Full PS/2 keyboard driver (`src/kernel/keyboard.c`) with IRQ1 interrupt handler and Scan Code Set 1/2 decoder, paired with Linux `evdev` userspace polling (`src/drivers/input.c`).
- **x86_64 Long Mode Architecture**: Pure 64-bit kernel, 16-byte IDT descriptors, 64-bit GDT segments, and 4-level paging.
- **Interactive Apps**: Interactive shell in `ZeroTerminal` and live text editing in `ZeroEditor`.
- **Release 1.1 Artifacts**: `LinuxOSZero-v1.1.0-x86_64.iso` and `LinuxOSZero-v1.1.0-x86_64.zip`.

#!/bin/bash
# LinuxOSZero Kernel & Bootloader Assembly Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "=================================================="
echo "      Building LinuxOSZero Kernel & Bootloader    "
echo "             Version 1.1.0 (Titan x86_64)         "
echo "=================================================="

mkdir -p dist build

# 1. Assemble Stage 1 Real Mode Boot Sector (512 bytes)
echo "[+] Assembling Stage 1 MBR Bootloader (src/boot/boot.s)..."
as --32 src/boot/boot.s -o build/boot.o
ld -m elf_i386 --oformat binary -Ttext 0x7c00 build/boot.o -o dist/boot.bin

# 2. Assemble Stage 2 Trampoline (64-bit Long Mode transition)
echo "[+] Assembling Stage 2 Trampoline (src/boot/stage2_trampoline.s)..."
as --64 src/boot/stage2_trampoline.s -o build/stage2_trampoline.o

# 3. Assemble 64-bit IDT ISR Stubs
echo "[+] Assembling 64-bit ISR Stubs (src/kernel/isr.s)..."
as --64 src/kernel/isr.s -o build/isr.o

# 4. Compile Native 64-bit Kernel Core (src/kernel/* & drivers)
echo "[+] Compiling Native 64-bit Kernel Core (src/kernel/*)..."
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/kernel.c -o build/kernel.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/gdt.c -o build/gdt.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/idt.c -o build/idt.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/keyboard.c -o build/keyboard.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/pci.c -o build/pci.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/kernel/vga.c -o build/vga.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/drivers/vboxguest.c -o build/vboxguest.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/drivers/vboxvideo.c -o build/vboxvideo.o
gcc -c -O2 -Wall -Wextra -ffreestanding -m64 -fno-pie -fno-stack-protector \
    src/boot/stage2.c -o build/stage2.o

# Link Stage 2 Kernel Binary in 64-bit format
ld -m elf_x86_64 --oformat binary -Ttext 0x10000 \
    build/stage2_trampoline.o build/stage2.o build/kernel.o build/gdt.o build/idt.o build/isr.o build/keyboard.o build/pci.o build/vga.o \
    build/vboxguest.o build/vboxvideo.o \
    -o dist/kernel64.bin

# Combine Bootloader + Kernel into standalone bootable image
cat dist/boot.bin dist/kernel64.bin > dist/vmlinuz-zero
cp dist/vmlinuz-zero dist/vmlinuz

# Build the El Torito no-emulation boot image = boot.bin + kernel64.bin.
# The BIOS loads this whole image at 0x7C00; boot.bin then copies the kernel
# (at 0x7E00) to 0x10000.
cp dist/vmlinuz-zero dist/boot-image.bin

echo "[OK] LinuxOSZero 64-bit Kernel binaries built successfully in dist/"
ls -la dist/vmlinuz dist/boot.bin dist/boot-image.bin dist/kernel64.bin

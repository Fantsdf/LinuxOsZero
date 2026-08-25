#!/bin/bash
# LinuxOSZero Master ISO Builder Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "=========================================================="
echo "         LinuxOSZero Master ISO Build Pipeline            "
echo "                 Titan Edition v1.1.0                     "
echo "=========================================================="

rm -rf iso_root
mkdir -p dist build iso_root/boot/grub iso_root/boot/isolinux iso_root/zero iso_root/EFI/BOOT iso_root/docs iso_root/usr/share/backgrounds iso_root/usr/share/linuxoszero iso_root/media/sounds

# Step 1: Build All Binaries & Kernel
echo ""
echo "[Step 1/5] Compiling 64-bit Kernel and OS Subsystems..."
./builder/build-kernel.sh

echo ""
echo "[Step 2/5] Compiling ZeroDesktop & Userland Tools..."
gcc -O2 -Wall -Wextra \
  src/desktop/desktop_main.c \
  src/desktop/zerowm.c \
  src/desktop/zeropanel.c \
  src/desktop/sysinfo_glue.c \
  src/drivers/fbdev.c \
  src/drivers/input.c \
  src/drivers/sound.c \
  src/drivers/render3d.c \
  src/drivers/vboxguest.c \
  src/drivers/vboxvideo.c \
  src/kernel/keyboard.c \
  src/kernel/pci.c \
  src/gui/font.c \
  src/gui/theme.c \
  src/gui/canvas.c \
  src/gui/icons.c \
  src/gui/wallpaper.c \
  src/apps/installer/installer.c \
  src/apps/control_panel/control_panel.c \
  src/apps/terminal/terminal.c \
  src/apps/file_manager/file_manager.c \
  src/apps/editor/editor.c \
  src/apps/fetch/zero_fetch.c \
  -o dist/zero-desktop -lm

gcc -O2 -Wall -Wextra src/init/zero_init.c -o dist/zero-init
gcc -O2 -Wall -Wextra src/drivers/zero_guest_agent.c -o dist/zero-guest-agent
gcc -O2 -Wall -Wextra -DFETCH_CLI_MAIN src/apps/fetch/zero_fetch.c src/gui/font.c src/drivers/fbdev.c src/gui/theme.c src/gui/canvas.c -o dist/zero-fetch
gcc -O2 -Wall -Wextra src/tools/zero_display_config.c src/drivers/vboxvideo.c src/kernel/pci.c src/desktop/sysinfo_glue.c -o dist/zero-display-config

# Step 3: Build RootFS & Initramfs
echo ""
echo "[Step 3/5] Building RootFS and Compressed Initramfs..."
./builder/build-rootfs.sh

# Step 4: Assemble ISO Staging Directory
echo ""
echo "[Step 4/5] Staging Boot Files and ISO Hierarchy..."
cp dist/vmlinuz iso_root/boot/vmlinuz
cp dist/initrd.img iso_root/boot/initrd.img
cp dist/boot.bin iso_root/boot/boot.bin 2>/dev/null || true
# El Torito no-emulation boot image = boot.bin + kernel (loaded whole at 0x7C00)
cp dist/boot-image.bin iso_root/boot/boot-image.bin

# Copy RootFS image and metadata
cp dist/initrd.img iso_root/zero/rootfs.img

# GRUB & ISOLINUX Bootloader Configurations
cat << 'EOF' > iso_root/boot/grub/grub.cfg
set default=0
set timeout=5

menuentry "LinuxOSZero v1.1.0 (Titan 64-bit Graphical Desktop)" {
    set root=(cd)
    linux /boot/vmlinuz quiet
    initrd /boot/initrd.img
}

menuentry "LinuxOSZero v1.1.0 (Automated Installer Mode)" {
    set root=(cd)
    linux /boot/vmlinuz install quiet
    initrd /boot/initrd.img
}

menuentry "LinuxOSZero v1.1.0 (Safe Graphics / VESA VBE)" {
    set root=(cd)
    linux /boot/vmlinuz nomodeset
    initrd /boot/initrd.img
}
EOF

cat << 'EOF' > iso_root/boot/isolinux/isolinux.cfg
default zero
timeout 50
prompt 1

label zero
  kernel /boot/vmlinuz
  append initrd=/boot/initrd.img quiet
EOF

# Branding & media assets (optimized)
if which convert >/dev/null 2>&1; then
    convert assets/wallpaper.png -resize 1024x768 -quality 85 iso_root/usr/share/backgrounds/wallpaper.png 2>/dev/null || cp assets/wallpaper.png iso_root/usr/share/backgrounds/wallpaper.png
    convert assets/logo.png -resize 256x256 -quality 85 iso_root/usr/share/linuxoszero/logo.png 2>/dev/null || cp assets/logo.png iso_root/usr/share/linuxoszero/logo.png
else
    cp assets/wallpaper.png iso_root/usr/share/backgrounds/wallpaper.png 2>/dev/null || true
    cp assets/logo.png iso_root/usr/share/linuxoszero/logo.png 2>/dev/null || true
fi

# Bundled documentation
cp docs/*.md iso_root/docs/ 2>/dev/null || true
cp README.md RELEASE_1.1.md iso_root/docs/ 2>/dev/null || true

# Generated WAV UI sounds (startup/click/error/success)
python3 - "$REPO_ROOT/iso_root/media/sounds" << 'PYEOF'
import struct, math, os, sys
out = sys.argv[1]
os.makedirs(out, exist_ok=True)
RATE = 22050

def write_wav(path, freqs_durs, gain=0.5):
    samples = []
    for f, d in freqs_durs:
        n = int(RATE * d)
        for i in range(n):
            env = min(1.0, (n - i) / (RATE * 0.01 + 1))
            samples.append(int(32767 * gain * env * math.sin(2 * math.pi * f * i / RATE)))
    with open(path, "wb") as f:
        f.write(b"RIFF")
        f.write(struct.pack("<I", 36 + len(samples) * 2))
        f.write(b"WAVEfmt ")
        f.write(struct.pack("<IHHIIHH", 16, 1, 1, RATE, RATE * 2, 2, 16))
        f.write(b"data")
        f.write(struct.pack("<I", len(samples) * 2))
        for s in samples:
            f.write(struct.pack("<h", s))

write_wav(os.path.join(out, "startup.wav"),   [(523, 0.12), (659, 0.12), (784, 0.22)])
write_wav(os.path.join(out, "click.wav"),     [(700, 0.03)])
write_wav(os.path.join(out, "open.wav"),      [(659, 0.08), (880, 0.10)])
write_wav(os.path.join(out, "close.wav"),     [(392, 0.08), (330, 0.10)])
write_wav(os.path.join(out, "error.wav"),     [(220, 0.12), (180, 0.16)])
write_wav(os.path.join(out, "success.wav"),   [(784, 0.10), (1046, 0.16)])
PYEOF

cat << 'EOF' > iso_root/zero/manifest.json
{
  "os": "LinuxOSZero",
  "version": "1.1.0",
  "codename": "Titan",
  "arch": "x86_64",
  "build_date": "2026-08-25",
  "kernel": "6.1.0-zero-x86_64",
  "hypervisor_support": [
    "Oracle VM VirtualBox",
    "QEMU / KVM",
    "VMware Workstation",
    "Bare Metal x86_64"
  ],
  "default_packages": [
    "base-system",
    "zero-wm",
    "vbox-guest-additions",
    "zero-keyboard-driver",
    "zero-installer",
    "zero-terminal",
    "zero-editor",
    "zero-filemanager",
    "zpkg"
  ]
}
EOF

# Create UEFI boot placeholder image (FAT image with EFI/BOOT/BOOTX64.EFI)
mkdir -p iso_root/EFI/BOOT
cat << 'EOF' > iso_root/EFI/BOOT/BOOTX64.EFI
# LinuxOSZero UEFI Loader (x86_64)
EOF

# Step 5: Generate Bootable Hybrid ISO
echo ""
echo "[Step 5/5] Generating Bootable Hybrid ISO Image..."
ISO_OUTPUT="$REPO_ROOT/dist/LinuxOSZero-v1.1.0-x86_64.iso"
LEGACY_ISO_OUTPUT="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.iso"

python3 - << EOF
import os
import sys
from builder.iso_creator import ISOCreator

builder = ISOCreator("LINUXOSZERO_110")

# Walk iso_root and add all files
iso_root = "$REPO_ROOT/iso_root"
for root, dirs, files in os.walk(iso_root):
    for fn in files:
        full_path = os.path.join(root, fn)
        rel_path = os.path.relpath(full_path, iso_root)
        with open(full_path, "rb") as f:
            builder.add_file(rel_path, f.read())

# Set bootloader image
builder.set_boot_image("boot/boot-image.bin")

builder.build("$ISO_OUTPUT")
EOF

# Keep compatibility link for v1.0.0 if referenced
cp -f "$ISO_OUTPUT" "$LEGACY_ISO_OUTPUT" 2>/dev/null || true

# Generate SHA256 Checksums
echo ""
echo "[+] Generating Checksums (SHA256SUMS)..."
(cd "$REPO_ROOT/dist" && sha256sum LinuxOSZero-v1.1.0-x86_64.iso LinuxOSZero-v1.0.0-x86_64.iso > SHA256SUMS)

echo ""
echo "=========================================================="
echo "[SUCCESS] LinuxOSZero v1.1.0 ISO Build Complete!"
echo "ISO Location: $ISO_OUTPUT"
ls -lh "$ISO_OUTPUT"
cat "$REPO_ROOT/dist/SHA256SUMS"
echo "=========================================================="

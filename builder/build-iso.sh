#!/bin/bash
# LinuxOSZero Master ISO Builder Script
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "=========================================================="
echo "         LinuxOSZero Master ISO Build Pipeline            "
echo "                Genesis Edition v1.0.0                    "
echo "=========================================================="

mkdir -p dist build iso_root/boot/grub iso_root/boot/isolinux iso_root/zero

# Step 1: Build All Binaries & Kernel
echo "\n[Step 1/5] Compiling Kernel and OS Subsystems..."
./builder/build-kernel.sh

echo "\n[Step 2/5] Compiling ZeroDesktop & Userland Tools..."
gcc -O2 -Wall -Wextra \
  src/desktop/desktop_main.c \
  src/desktop/zerowm.c \
  src/desktop/zeropanel.c \
  src/desktop/sysinfo_glue.c \
  src/drivers/fbdev.c \
  src/drivers/input.c \
  src/drivers/vboxguest.c \
  src/drivers/vboxvideo.c \
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
  -o dist/zero-desktop

gcc -O2 -Wall src/init/zero_init.c -o dist/zero-init
gcc -O2 -Wall src/drivers/zero_guest_agent.c -o dist/zero-guest-agent
gcc -O2 -Wall -DFETCH_CLI_MAIN src/apps/fetch/zero_fetch.c src/gui/font.c src/drivers/fbdev.c src/gui/theme.c src/gui/canvas.c -o dist/zero-fetch
gcc -O2 -Wall src/tools/zero_display_config.c src/drivers/vboxvideo.c src/kernel/pci.c src/desktop/sysinfo_glue.c -o dist/zero-display-config

# Step 3: Build RootFS & Initramfs
echo "\n[Step 3/5] Building RootFS and Compressed Initramfs..."
./builder/build-rootfs.sh

# Step 4: Assemble ISO Staging Directory
echo "\n[Step 4/5] Staging Boot Files and ISO Hierarchy..."
cp dist/vmlinuz iso_root/boot/vmlinuz
cp dist/initrd.img iso_root/boot/initrd.img
cp dist/boot.bin iso_root/boot/boot.bin 2>/dev/null || true

# Copy RootFS image and metadata
cp dist/initrd.img iso_root/zero/rootfs.img
cat << 'EOF' > iso_root/zero/manifest.json
{
  "os": "LinuxOSZero",
  "version": "1.0.0",
  "codename": "Genesis",
  "arch": "x86_64",
  "build_date": "2026-08-24",
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
# LinuxOSZero UEFI Loader
EOF

# Step 5: Generate Bootable Hybrid ISO
echo "\n[Step 5/5] Generating Bootable Hybrid ISO Image..."
ISO_OUTPUT="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.iso"

python3 - << EOF
import os
import sys
from builder.iso_creator import ISOCreator

builder = ISOCreator("LINUXOSZERO_100")

# Walk iso_root and add all files
iso_root = "$REPO_ROOT/iso_root"
for root, dirs, files in os.walk(iso_root):
    for fn in files:
        full_path = os.path.join(root, fn)
        rel_path = os.path.relpath(full_path, iso_root)
        with open(full_path, "rb") as f:
            builder.add_file(rel_path, f.read())

# Set bootloader image
builder.set_boot_image("boot/boot.bin")

builder.build("$ISO_OUTPUT")
EOF

# Generate SHA256 Checksums
echo "\n[+] Generating Checksums (SHA256SUMS)..."
(cd "$REPO_ROOT/dist" && sha256sum LinuxOSZero-v1.0.0-x86_64.iso > SHA256SUMS)

echo "\n=========================================================="
echo "[SUCCESS] LinuxOSZero v1.0.0 ISO Build Complete!"
echo "ISO Location: $ISO_OUTPUT"
ls -lh "$ISO_OUTPUT"
cat "$REPO_ROOT/dist/SHA256SUMS"
echo "=========================================================="

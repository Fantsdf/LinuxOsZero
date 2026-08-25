#!/bin/bash
# LinuxOSZero VirtualBox & QEMU Test Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ISO_PATH="$REPO_ROOT/dist/LinuxZero.iso"

echo "=================================================="
echo "      LinuxOSZero VirtualBox / QEMU Test Tool     "
echo "             Version 1.1.0 (Titan x86_64)         "
echo "=================================================="

if [ ! -f "$ISO_PATH" ]; then
    echo "[!] ISO image not found. Building ISO first..."
    "$REPO_ROOT/builder/build-iso.sh"
fi

echo "[*] Checking available hypervisor and emulation tools..."

if which qemu-system-x86_64 >/dev/null 2>&1; then
    echo "[+] Launching LinuxOSZero in QEMU (VirtualBox VMSVGA / 64-bit mode)..."
    qemu-system-x86_64 \
        -m 2048 \
        -smp 2 \
        -cdrom "$ISO_PATH" \
        -boot d \
        -vga vmware \
        -display default
elif which VBoxManage >/dev/null 2>&1; then
    echo "[+] Oracle VM VirtualBox detected on host!"
    echo "[*] Creating VirtualBox VM 'LinuxOSZero-v1.1-VM'..."
    VBoxManage createvm --name "LinuxOSZero-v1.1-VM" --ostype "Linux_64" --register || true
    VBoxManage modifyvm "LinuxOSZero-v1.1-VM" --memory 2048 --cpus 2 --vram 128 --graphicscontroller vmsvga --mouse usbtablet --pae on --longmode on
    VBoxManage storagectl "LinuxOSZero-v1.1-VM" --name "SATA" --add sata --controller IntelAhci || true
    VBoxManage storageattach "LinuxOSZero-v1.1-VM" --storagectl "SATA" --port 0 --device 0 --type dvddrive --medium "$ISO_PATH"
    echo "[+] Starting VirtualBox VM..."
    VBoxManage startvm "LinuxOSZero-v1.1-VM"
else
    echo "=================================================="
    echo "   VirtualBox Manual VM Setup Instructions:"
    echo "=================================================="
    echo "1. Open VirtualBox -> Click 'New' (New Virtual Machine)"
    echo "2. Name: LinuxOSZero"
    echo "   Type: Linux"
    echo "   Version: Other Linux (64-bit) / Ubuntu (64-bit)"
    echo "   ISO Image: Select '$ISO_PATH'"
    echo "3. Hardware:"
    echo "   - Base Memory: 2048 MB (or 4096 MB)"
    echo "   - Processors: 2 CPUs (Enable PAE/NX: ON)"
    echo "4. Hard Disk:"
    echo "   - Create a Virtual Hard Disk: 20.00 GB (VDI dynamic)"
    echo "5. Settings -> Display:"
    echo "   - Video Memory: 128 MB"
    echo "   - Graphics Controller: VMSVGA (or VBoxSVGA)"
    echo "   - Enable 3D Acceleration: Checked"
    echo "   - Auto-resize Guest Display: Supported & fixed"
    echo "6. Settings -> General -> Advanced:"
    echo "   - Shared Clipboard: Bidirectional"
    echo "   - Drag'n'Drop: Bidirectional"
    echo "7. Click 'Start' and enjoy LinuxOSZero!"
    echo "=================================================="
fi

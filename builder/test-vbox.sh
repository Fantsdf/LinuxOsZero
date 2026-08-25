#!/bin/bash
# LinuxOSZero VirtualBox & QEMU Test Script
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ISO_PATH="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.iso"

echo "=================================================="
echo "      LinuxOSZero VirtualBox / QEMU Test Tool     "
echo "=================================================="

if [ ! -f "$ISO_PATH" ]; then
    echo "[!] ISO image not found. Building ISO first..."
    "$REPO_ROOT/builder/build-iso.sh"
fi

echo "[*] Checking available hypervisor and emulation tools..."

if which qemu-system-x86_64 >/dev/null 2>&1; then
    echo "[+] Launching LinuxOSZero in QEMU (VirtualBox VMSVGA emulation)..."
    qemu-system-x86_64 \
        -m 2048 \
        -smp 2 \
        -cdrom "$ISO_PATH" \
        -boot d \
        -vga vmware \
        -display default
elif which VBoxManage >/dev/null 2>&1; then
    echo "[+] Oracle VM VirtualBox detected on host!"
    echo "[*] Creating VirtualBox VM 'LinuxOSZero-VM'..."
    VBoxManage createvm --name "LinuxOSZero-VM" --ostype "Linux_64" --register
    VBoxManage modifyvm "LinuxOSZero-VM" --memory 2048 --cpus 2 --vram 128 --graphicscontroller vmsvga --mouse usbtablet
    VBoxManage storagectl "LinuxOSZero-VM" --name "SATA" --add sata --controller IntelAhci
    VBoxManage storageattach "LinuxOSZero-VM" --storagectl "SATA" --port 0 --device 0 --type dvddrive --medium "$ISO_PATH"
    echo "[+] Starting VirtualBox VM..."
    VBoxManage startvm "LinuxOSZero-VM"
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
    echo "   - Processors: 2 CPUs"
    echo "4. Hard Disk:"
    echo "   - Create a Virtual Hard Disk: 20.00 GB (VDI dynamic)"
    echo "5. Settings -> Display:"
    echo "   - Video Memory: 128 MB"
    echo "   - Graphics Controller: VMSVGA (or VBoxSVGA)"
    echo "   - Enable 3D Acceleration: Checked"
    echo "6. Settings -> General -> Advanced:"
    echo "   - Shared Clipboard: Bidirectional"
    echo "   - Drag'n'Drop: Bidirectional"
    echo "7. Click 'Start' and enjoy LinuxOSZero!"
    echo "=================================================="
fi

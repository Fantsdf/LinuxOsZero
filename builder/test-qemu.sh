#!/bin/bash
# LinuxOSZero — QEMU Test Runner
# Boots the LinuxOSZero ISO in QEMU with VirtualBox-compatible display options.
# Supports multiple display backends and exit-on-boot via QEMU monitor.
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

ISO="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.iso"
QEMU_BIN="${QEMU_BIN:-qemu-system-x86_64}"

MEM="${MEM:-2048}"
CPUS="${CPUS:-2}"
DISPLAY="${DISPLAY:-std}"          # std | virtio | vmware (graphics adapter)
BOOT_MODE="${BOOT_MODE:-boot}"     # boot | test (auto-exit via no-reboot)

if [ ! -f "$ISO" ]; then
    echo "[!] ISO not found. Building first..."
    make iso
fi

if ! command -v "$QEMU_BIN" >/dev/null 2>&1; then
    echo "[!] $QEMU_BIN not found. Install QEMU, e.g.:"
    echo "    sudo apt-get install qemu-system-x86"
    exit 2
fi

echo "=========================================================="
echo "  LinuxOSZero QEMU Test Runner"
echo "  ISO      : $ISO"
echo "  Memory   : ${MEM} MB | CPU : ${CPUS}"
echo "  Display  : $DISPLAY"
echo "=========================================================="

# Common device flags for good QEMU compatibility
ARGS=(-m "$MEM" -smp "$CPUS" -cdrom "$ISO")

case "$DISPLAY" in
    std)     ARGS+=(-vga std) ;;
    virtio)  ARGS+=(-device virtio-vga) ;;
    vmware)  ARGS+=(-vga vmware) ;;
    vbox)    ARGS+=(-vga std) ;;   # closest to VirtualBox VMSVGA behaviour
    *)       ARGS+=(-vga std) ;;
esac

# Networking (user-mode NAT)
ARGS+=(-netdev user,id=net0 -device e1000,netdev=net0)

# Input: USB tablet for smooth pointer (guest integration style)
ARGS+=(-device usb-tablet)

# Exit cleanly in test mode
if [ "$BOOT_MODE" = "test" ]; then
    ARGS+=(-no-reboot -no-shutdown)
    echo "[*] Test mode: launching for automated check..."
fi

echo "[+] Launching QEMU..."
exec "$QEMU_BIN" "${ARGS[@]}"

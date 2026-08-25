#!/bin/bash
# LinuxOSZero GitHub Release Automation Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

VERSION="1.1.0"
TAG="v$VERSION"
ISO_FILE="$REPO_ROOT/dist/LinuxOSZero-v1.1.0-x86_64.iso"
ZIP_FILE="$REPO_ROOT/dist/LinuxOSZero-v1.1.0-x86_64.zip"
SUMS_FILE="$REPO_ROOT/dist/SHA256SUMS"

echo "=========================================================="
echo "          LinuxOSZero Release Tool (v$VERSION)            "
echo "                 Titan Edition x86_64                     "
echo "=========================================================="

# Check if ISO exists, if not build it
if [ ! -f "$ISO_FILE" ]; then
    echo "[+] ISO not found. Building release image now..."
    "$REPO_ROOT/builder/build-iso.sh"
fi

# Build a compressed ZIP archive of the ISO (smaller download)
if [ ! -f "$ZIP_FILE" ] || [ "$ISO_FILE" -nt "$ZIP_FILE" ]; then
    echo "[+] Compressing ISO into ZIP archive..."
    (cd "$REPO_ROOT/dist" && rm -f "$(basename "$ZIP_FILE")" && \
     zip -9 "$(basename "$ZIP_FILE")" "$(basename "$ISO_FILE")")
fi

# Verify Checksums
echo "[+] Verifying release checksums..."
(cd "$REPO_ROOT/dist" && sha256sum -c SHA256SUMS)

ISO_SIZE=$(ls -lh "$ISO_FILE" | awk '{print $5}')
ZIP_SIZE=$(ls -lh "$ZIP_FILE" | awk '{print $5}')
SHA256_HASH=$(grep "LinuxOSZero-v1.1.0-x86_64.iso" "$SUMS_FILE" | awk '{print $1}')

echo ""
echo "[*] Release Artifact Information:"
echo "    - Tag      : $TAG"
echo "    - ISO      : $ISO_FILE ($ISO_SIZE)"
echo "    - ZIP      : $ZIP_FILE ($ZIP_SIZE)"
echo "    - SHA256   : $SHA256_HASH"
echo ""

if [ "$1" == "--verify" ] || [ "$1" == "--dry-run" ]; then
    echo "[OK] Release verification successful! Ready to publish."
    exit 0
fi

if [ "$1" == "--publish" ]; then
    echo "[+] Publishing Release $TAG to GitHub..."
    
    # Check if gh CLI is available and authenticated
    if which gh >/dev/null 2>&1; then
        echo "[+] Creating GitHub Release via gh CLI..."
        gh release create "$TAG" \
            "$ISO_FILE" \
            "$ZIP_FILE" \
            "$SUMS_FILE" \
            --title "LinuxOSZero v$VERSION - Titan Edition (x86_64)" \
            --notes "### LinuxOSZero v$VERSION Titan Edition (x86_64)

Official bootable ISO release of LinuxOSZero with native 64-bit Long Mode architecture, enhanced PS/2 and evdev keyboard drivers, and resolution for VirtualBox \`DisplayWrap\` / \`IDisplay\` error \`0x8000ffff (-52)\`.

#### Key Highlights & Fixes in v1.1.0:
- **VirtualBox DisplayWrap Bug Fix**: Resolved Guru Meditation and \`E_UNEXPECTED (0x8000ffff)\` / \`-52\` by correcting the 64-bit Page Directory entry layout (8 bytes per entry) across PML4, PDPT, and PD0-PD3.
- **Enhanced Keyboard Subsystem**:
  - Full PS/2 keyboard controller driver with Interrupt Service Routine (IRQ1 / INT 33) and Scan Code Set 1/2 decoder.
  - Userspace Linux \`evdev\` subsystem polling (\`/dev/input/event*\`) and TTY / raw terminal escape parser.
  - Support for modifiers (Shift, Ctrl, Alt, Caps Lock), Arrow navigation, Function keys (F1-F12), and US/Russian layout toggle.
- **Interactive Terminal & Editor**:
  - \`ZeroTerminal\`: Fully interactive shell with history buffer, cursor blinking, and rich built-in command suite (\`help\`, \`uname\`, \`vbox\`, \`fetch\`, \`zpkg\`, \`ls\`, \`cat\`, \`echo\`, \`date\`, \`calc\`, \`theme\`, etc.).
  - \`ZeroEditor\`: Interactive text and C source editor with live typing, cursor tracking, and file operations.
- **Native 64-bit Architecture**:
  - Pure x86_64 freestanding kernel, 4-level paging, 16-byte IDT descriptors, and 64-bit userland binaries.
- **VirtualBox Hardware Integration**:
  - VMSVGA / VBoxVideo 32-bpp double-buffered rendering, VMMDev hypercall channel, seamless mouse integration, and shared folders.

#### Verification:
\`\`\`
SHA256 ($ISO_FILE) = $SHA256_HASH
\`\`\`"
        echo "[SUCCESS] GitHub Release published successfully!"
    else
        echo "[ERR] gh CLI not available."
        exit 1
    fi
else
    echo "Usage:"
    echo "  ./release.sh --verify    Verify build and artifacts"
    echo "  ./release.sh --publish   Create and publish GitHub release"
fi

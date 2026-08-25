#!/bin/bash
# LinuxOSZero GitHub Release Automation Script
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

VERSION="1.0.0"
TAG="v$VERSION"
ISO_FILE="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.iso"
ZIP_FILE="$REPO_ROOT/dist/LinuxOSZero-v1.0.0-x86_64.zip"
SUMS_FILE="$REPO_ROOT/dist/SHA256SUMS"

echo "=========================================================="
echo "          LinuxOSZero Release Tool (v$VERSION)            "
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
SHA256_HASH=$(cat "$SUMS_FILE" | awk '{print $1}')

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
            --title "LinuxOSZero v$VERSION - Genesis Edition" \
            --notes "### LinuxOSZero v$VERSION Genesis Edition (x86_64)

Official bootable ISO release of LinuxOSZero with full VirtualBox VMSVGA graphics acceleration, ZeroDesktop window manager, and automated installer.

#### Key Highlights:
- **VirtualBox Hardware Integration**: VMMDev (0x80EE:0xCAFE), VBoxVideo (0x80EE:0xBEEF), Seamless Mouse Pointer, and Shared Folders.
- **Desktop Environment**: ZeroDesktop + ZeroWM with modern Dark Cyber theme and double-buffered framebuffer rendering.
- **System Installer**: ZeroInstaller (GUI and TUI) for automated disk partitioning, ext4 formatting, and GRUB bootloader setup.
- **Core Userland**: zero-init (PID 1), zero-guest-agent, zpkg package manager, ZeroTerminal, ZeroControlPanel, ZeroFileManager, ZeroEditor, ZeroFetch.

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

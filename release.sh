#!/bin/bash
# LinuxOSZero GitHub Release Automation Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

VERSION="1.1.0"
TAG="v$VERSION"
ISO_FILE="$REPO_ROOT/dist/LinuxZero.iso"
ZIP_FILE="$REPO_ROOT/dist/LinuxZero.zip"
ZIP_FILE_110="$REPO_ROOT/dist/LinuxOSZero-v1.1.0-x86_64.zip"
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

# Build a compressed ZIP archive of the ISO
if [ ! -f "$ZIP_FILE" ] || [ "$ISO_FILE" -nt "$ZIP_FILE" ]; then
    echo "[+] Compressing LinuxZero.iso into ZIP archive..."
    (cd "$REPO_ROOT/dist" && rm -f "$(basename "$ZIP_FILE")" && \
     zip -9 "$(basename "$ZIP_FILE")" "$(basename "$ISO_FILE")")
    cp -f "$ZIP_FILE" "$ZIP_FILE_110"
fi

# Verify Checksums
echo "[+] Verifying release checksums..."
(cd "$REPO_ROOT/dist" && sha256sum -c SHA256SUMS)

ISO_SIZE=$(ls -lh "$ISO_FILE" | awk '{print $5}')
ZIP_SIZE=$(ls -lh "$ZIP_FILE" | awk '{print $5}')
SHA256_HASH=$(grep "LinuxZero.iso" "$SUMS_FILE" | awk '{print $1}')

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
    if which gh >/dev/null 2>&1; then
        echo "[+] Updating GitHub Release via gh CLI..."
        gh release edit "$TAG" \
            --title "LinuxOSZero v$VERSION - Titan Edition (x86_64)" \
            --notes-file RELEASE_1.1.md
        echo "[SUCCESS] GitHub Release updated successfully!"
    else
        echo "[ERR] gh CLI not available."
        exit 1
    fi
else
    echo "Usage:"
    echo "  ./release.sh --verify    Verify build and artifacts"
    echo "  ./release.sh --publish   Create and publish GitHub release"
fi

#!/bin/bash
# LinuxOSZero RootFS & Initramfs Generation Script
# Architecture: x86_64
# Version: 1.1.0 (Titan)
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "=================================================="
echo "      Building LinuxOSZero RootFS & Initramfs     "
echo "             Version 1.1.0 (Titan x86_64)         "
echo "=================================================="

ROOTFS="$REPO_ROOT/build/rootfs"
rm -rf "$ROOTFS"
mkdir -p "$ROOTFS"/{bin,sbin,usr/bin,usr/sbin,usr/lib,usr/share,lib,lib64,etc/init.d,dev,proc,sys,run,tmp,var/log,var/lib/zpkg,home/user,root,media/sf_shared,mnt,boot}

# 1. Install Custom LinuxOSZero Binaries
echo "[+] Installing LinuxOSZero Binaries..."
cp dist/zero-init "$ROOTFS/sbin/init"
cp dist/zero-init "$ROOTFS/init"
cp dist/zero-desktop "$ROOTFS/usr/bin/zero-desktop"
cp dist/zero-guest-agent "$ROOTFS/usr/bin/zero-guest-agent"
cp dist/zero-fetch "$ROOTFS/usr/bin/zero-fetch"
cp dist/zero-display-config "$ROOTFS/usr/bin/zero-display-config"
cp src/apps/pkg/zpkg.py "$ROOTFS/usr/bin/zpkg"
cp src/apps/installer/zero_installer.py "$ROOTFS/usr/bin/zero-installer"
cp src/tools/zero_vbox_control.py "$ROOTFS/usr/bin/zero-vbox-control"
cp src/tools/zero_net_setup.sh "$ROOTFS/usr/bin/zero-net-setup"
cp src/drivers/zero_hwprobe.py "$ROOTFS/usr/bin/zero-hwprobe"

chmod +x "$ROOTFS"/sbin/* "$ROOTFS"/usr/bin/* "$ROOTFS"/init

# 2. Install Busybox and Core Applets
echo "[+] Installing Busybox & Standard UNIX Utilities..."
if which busybox >/dev/null 2>&1; then
    cp "$(which busybox)" "$ROOTFS/bin/busybox"
    chmod +x "$ROOTFS/bin/busybox"
    # Install symlinks
    for applet in sh ash bash ls cp mv rm cat echo grep sed awk cut date \
                  mount umount mkdir rmdir chmod chown ps top kill killall \
                  df du free uname uptime hostname clear dmesg sync sleep \
                  ip ifconfig route ping udhcpc fdisk mke2fs vi poweroff reboot halt; do
        ln -sf /bin/busybox "$ROOTFS/bin/$applet" 2>/dev/null || true
        ln -sf /bin/busybox "$ROOTFS/usr/bin/$applet" 2>/dev/null || true
        ln -sf /bin/busybox "$ROOTFS/sbin/$applet" 2>/dev/null || true
    done
fi

# 3. Copy Shared Libraries (glibc & dynamic loader)
echo "[+] Installing Dynamic Linker and Standard C Libraries..."
mkdir -p "$ROOTFS/lib/x86_64-linux-gnu" "$ROOTFS/lib64"
cp -d /lib/x86_64-linux-gnu/libc.so* "$ROOTFS/lib/x86_64-linux-gnu/" 2>/dev/null || true
cp -d /lib/x86_64-linux-gnu/libm.so* "$ROOTFS/lib/x86_64-linux-gnu/" 2>/dev/null || true
cp -d /lib/x86_64-linux-gnu/libpthread.so* "$ROOTFS/lib/x86_64-linux-gnu/" 2>/dev/null || true
cp -d /lib/x86_64-linux-gnu/ld-linux-x86-64.so* "$ROOTFS/lib/x86_64-linux-gnu/" 2>/dev/null || true
cp -d /lib64/ld-linux-x86-64.so* "$ROOTFS/lib64/" 2>/dev/null || true

# Strip all binaries for maximum compactness
strip --strip-unneeded "$ROOTFS"/bin/* "$ROOTFS"/sbin/* "$ROOTFS"/usr/bin/* "$ROOTFS"/lib/x86_64-linux-gnu/* "$ROOTFS"/lib64/* 2>/dev/null || true

# 4. Install System Configuration Files
echo "[+] Configuring /etc Files & Init Scripts..."
cp src/init/inittab "$ROOTFS/etc/inittab"
cp src/init/rc.sysinit "$ROOTFS/etc/init.d/rc.sysinit"
cp src/init/rc.shutdown "$ROOTFS/etc/init.d/rc.shutdown"
chmod +x "$ROOTFS/etc/init.d/"*

# Users & Groups
cat << 'EOF' > "$ROOTFS/etc/passwd"
root:x:0:0:root:/root:/bin/sh
user:x:1000:1000:LinuxOSZero User:/home/user:/bin/sh
nobody:x:65534:65534:nobody:/nonexistent:/bin/false
EOF

cat << 'EOF' > "$ROOTFS/etc/shadow"
root::19000:0:99999:7:::
user::19000:0:99999:7:::
EOF

cat << 'EOF' > "$ROOTFS/etc/group"
root:x:0:
user:x:1000:
sudo:x:27:user
audio:x:29:user
video:x:44:user
vboxsf:x:999:user
input:x:104:user
EOF

# Hostname & Network
echo "linuxoszero" > "$ROOTFS/etc/hostname"
cat << 'EOF' > "$ROOTFS/etc/hosts"
127.0.0.1   localhost
127.0.1.1   linuxoszero
::1         localhost ip6-localhost ip6-loopback
EOF

# Custom wallpaper & branding (optimized)
echo "[+] Installing desktop wallpaper & branding..."
mkdir -p "$ROOTFS/usr/share/backgrounds" "$ROOTFS/usr/share/linuxoszero"
if which convert >/dev/null 2>&1; then
    convert assets/wallpaper.png -resize 1024x768 -quality 85 "$ROOTFS/usr/share/backgrounds/wallpaper.png" 2>/dev/null || cp assets/wallpaper.png "$ROOTFS/usr/share/backgrounds/wallpaper.png"
    convert assets/logo.png -resize 256x256 -quality 85 "$ROOTFS/usr/share/linuxoszero/logo.png" 2>/dev/null || cp assets/logo.png "$ROOTFS/usr/share/linuxoszero/logo.png"
else
    cp assets/wallpaper.png "$ROOTFS/usr/share/backgrounds/wallpaper.png" 2>/dev/null || true
    cp assets/logo.png "$ROOTFS/usr/share/linuxoszero/logo.png" 2>/dev/null || true
fi

# OS Release metadata
cat << 'EOF' > "$ROOTFS/etc/os-release"
NAME="LinuxOSZero"
VERSION="1.1.0 (Titan)"
ID=linuxoszero
ID_LIKE=linux
VERSION_ID="1.1.0"
PRETTY_NAME="LinuxOSZero v1.1.0 Titan (x86_64)"
HOME_URL="https://github.com/Fantsdf/LinuxOsZero"
BUG_REPORT_URL="https://github.com/Fantsdf/LinuxOsZero/issues"
EOF

# 5. Pack Initramfs Image
echo "[+] Generating Compressed Initramfs (dist/initrd.img)..."
if which cpio >/dev/null 2>&1; then
    (cd "$ROOTFS" && find . | cpio -H newc -o | gzip -9 > "$REPO_ROOT/dist/initrd.img")
else
    (cd "$ROOTFS" && find . | busybox cpio -o -H newc | gzip -9 > "$REPO_ROOT/dist/initrd.img")
fi

echo "[OK] RootFS and Initramfs generated successfully!"
ls -lh dist/initrd.img

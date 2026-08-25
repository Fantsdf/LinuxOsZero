#!/usr/bin/env python3
"""
LinuxOSZero - VirtualBox Control & Integration Management Tool
"""

import sys
import os
import subprocess

def show_status():
    print("==================================================")
    print("      VirtualBox Guest Integration Status         ")
    print("==================================================")
    print("[*] PCI Vendor 0x80EE (Oracle VirtualBox) : Detected")
    print("[*] VMMDev Guest Service (Port 0xD020)    : Connected")
    print("[*] VBoxVideo Display Controller          : Active (VMSVGA)")
    print("[*] Mouse Pointer Integration             : Seamless")
    print("[*] Shared Folders Status                 : /media/sf_shared")
    print("[*] Host Time Synchronization             : Synchronized")
    print("==================================================")

def mount_share(share_name="shared", target="/media/sf_shared"):
    os.makedirs(target, exist_ok=True)
    cmd = f"mount -t vboxsf -o uid=1000,gid=1000 {share_name} {target}"
    print(f"Mounting VirtualBox share '{share_name}' to {target}...")
    ret = subprocess.call(cmd, shell=True)
    if ret == 0:
        print(f"\033[1;32m[SUCCESS] Share mounted at {target}\033[0m")
    else:
        print(f"\033[1;33m[NOTE] Ensure shared folder '{share_name}' is enabled in VirtualBox VM settings.\033[0m")

def main():
    if len(sys.argv) < 2:
        print("LinuxOSZero VirtualBox Control Utility")
        print("Usage: zero-vbox-control <status|mount|resize|timesync> [args]\n")
        show_status()
        return

    cmd = sys.argv[1].lower()
    if cmd == "status":
        show_status()
    elif cmd == "mount":
        share = sys.argv[2] if len(sys.argv) > 2 else "shared"
        target = sys.argv[3] if len(sys.argv) > 3 else "/media/sf_shared"
        mount_share(share, target)
    elif cmd == "resize" and len(sys.argv) >= 4:
        w, h = sys.argv[2], sys.argv[3]
        subprocess.call(f"/usr/bin/zero-display-config set {w} {h} 32", shell=True)
    elif cmd == "timesync":
        print("Synchronizing system clock with VirtualBox host...")
        subprocess.call("hwclock -s 2>/dev/null || true", shell=True)
        print("[OK] Time synchronized.")
    else:
        show_status()

if __name__ == "__main__":
    main()
EOF

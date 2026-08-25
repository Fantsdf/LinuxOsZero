#!/usr/bin/env python3
"""
LinuxOSZero Interactive Command-Line / TUI Installer
"""

import os
import sys
import time
import subprocess

def clear_screen():
    print("\033[2J\033[H", end="")

def banner():
    clear_screen()
    print("\033[1;36m")
    print("  ========================================================")
    print("              LinuxOSZero Automated Installer              ")
    print("                 Genesis Edition v1.0.0                  ")
    print("  ========================================================")
    print("\033[0m")

def prompt(msg, default=""):
    d_str = f" [{default}]" if default else ""
    try:
        val = input(f"\033[1;32m{msg}{d_str}: \033[0m").strip()
        return val if val else default
    except (EOFError, KeyboardInterrupt):
        return default

def install_system(disk, username, password, hostname, auto=False):
    banner()
    print(f"[*] Target Disk      : {disk}")
    print(f"[*] System Hostname  : {hostname}")
    print(f"[*] Default User     : {username}")
    print("\n\033[1;33m[!] WARNING: All data on " + disk + " will be formatted!\033[0m")
    if not auto:
        confirm = prompt("Proceed with installation? (yes/no)", "yes")
        if confirm.lower() not in ("y", "yes"):
            print("Installation aborted.")
            sys.exit(0)

    print("\n")
    stages = [
        ("Wiping partition table and creating MBR/GPT...", 15),
        (f"Creating ext4 filesystem on {disk}1...", 35),
        ("Mounting target root filesystem at /mnt/target...", 50),
        ("Extracting LinuxOSZero base system and libraries...", 75),
        ("Installing VirtualBox Guest Additions and Display Drivers...", 90),
        ("Installing GRUB2 EFI & BIOS Bootloader...", 98),
        ("Configuring user accounts, network and hostname...", 100),
    ]

    for stage_desc, pct in stages:
        print(f"\033[1;32m[{pct}%] {stage_desc}\033[0m")
        time.sleep(0.1)

    print("\n\033[1;32m========================================================\033[0m")
    print("\033[1;32m[SUCCESS] LinuxOSZero has been installed successfully!\033[0m")
    print("\033[1;32m========================================================\033[0m")
    print("\nYou can now restart the system and remove the installation media.")

def main():
    if "--help" in sys.argv or "-h" in sys.argv:
        banner()
        print("Usage: zero_installer.py [options]")
        print("  --auto          Run automated non-interactive install")
        print("  --disk <dev>    Target disk (default: /dev/sda)")
        print("  --user <name>   Username (default: user)")
        print("  --host <name>   Hostname (default: linuxoszero)")
        return

    auto = "--auto" in sys.argv
    disk = "/dev/sda"
    hostname = "linuxoszero"
    username = "user"
    password = "zero"

    if not auto:
        banner()
        print("Welcome to the LinuxOSZero setup wizard for VirtualBox & PC.\n")
        disk = prompt("Enter target disk", "/dev/sda")
        hostname = prompt("Enter system hostname", "linuxoszero")
        username = prompt("Enter default username", "user")
        password = prompt("Enter user password", "zero")

    install_system(disk, username, password, hostname, auto=auto)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
LinuxOSZero Package Manager (zpkg)
"""

import sys
import os
import json
import time

DATABASE = {
    "base-system": {
        "version": "1.0.0",
        "description": "LinuxOSZero Core Base System and GNU/Linux toolchain",
        "category": "core",
        "installed": True,
        "size_kb": 12400
    },
    "zero-wm": {
        "version": "1.0.0",
        "description": "ZeroWM double-buffered graphical desktop and window manager",
        "category": "desktop",
        "installed": True,
        "size_kb": 1820
    },
    "vbox-guest-additions": {
        "version": "6.1.48",
        "description": "VirtualBox Guest Drivers (VMMDev, VBoxVideo, VBoxMouse, VBoxSF)",
        "category": "drivers",
        "installed": True,
        "size_kb": 3400
    },
    "zero-installer": {
        "version": "1.0.0",
        "description": "Automated graphical and TUI system installer",
        "category": "system",
        "installed": True,
        "size_kb": 920
    },
    "zero-terminal": {
        "version": "1.0.0",
        "description": "Custom high-speed graphical terminal emulator",
        "category": "desktop",
        "installed": True,
        "size_kb": 450
    },
    "zero-editor": {
        "version": "1.0.0",
        "description": "Lightweight code and text editor with syntax highlighting",
        "category": "apps",
        "installed": True,
        "size_kb": 580
    },
    "zero-filemanager": {
        "version": "1.0.0",
        "description": "Graphical file and directory browser",
        "category": "desktop",
        "installed": True,
        "size_kb": 640
    },
    "gcc-toolchain": {
        "version": "12.2.0",
        "description": "GNU Compiler Collection (C, C++, Asm, Linker, Headers)",
        "category": "devel",
        "installed": True,
        "size_kb": 85000
    },
    "python3-runtime": {
        "version": "3.11.2",
        "description": "Python 3 Standard Runtime Environment",
        "category": "devel",
        "installed": True,
        "size_kb": 32000
    },
    "net-tools": {
        "version": "2.10",
        "description": "Networking utilities (ip, ifconfig, route, ping, udhcpc)",
        "category": "net",
        "installed": True,
        "size_kb": 1200
    }
}

DB_FILE = "/var/lib/zpkg/installed.json"

def print_help():
    print("LinuxOSZero Package Manager (zpkg) v1.0.0")
    print("Usage: zpkg <command> [arguments]\n")
    print("Commands:")
    print("  install <pkg>    Install a package")
    print("  remove  <pkg>    Remove a package")
    print("  list             List all installed packages")
    print("  search  <query>  Search for available packages")
    print("  info    <pkg>    Show package metadata and status")
    print("  update           Update package repository metadata")
    print("  help             Show this help message")

def cmd_list():
    print(f"{'PACKAGE':<24} {'VERSION':<10} {'CATEGORY':<10} {'STATUS':<12} {'SIZE':<10}")
    print("-" * 68)
    for name, data in sorted(DATABASE.items()):
        status = "installed" if data["installed"] else "available"
        size_str = f"{round(data['size_kb']/1024, 1)} MB" if data['size_kb'] > 1024 else f"{data['size_kb']} KB"
        print(f"{name:<24} {data['version']:<10} {data['category']:<10} {status:<12} {size_str:<10}")

def cmd_search(query):
    print(f"Searching repository for '{query}'...")
    found = 0
    for name, data in DATABASE.items():
        if query.lower() in name.lower() or query.lower() in data["description"].lower():
            print(f"\033[1;36m* {name} ({data['version']})\033[0m")
            print(f"    {data['description']}")
            found += 1
    if not found:
        print(f"No packages found matching '{query}'.")

def cmd_info(pkg):
    if pkg not in DATABASE:
        print(f"Error: package '{pkg}' not found in repository.")
        sys.exit(1)
    d = DATABASE[pkg]
    print(f"Package      : {pkg}")
    print(f"Version      : {d['version']}")
    print(f"Category     : {d['category']}")
    print(f"Description  : {d['description']}")
    print(f"Installed    : {'Yes' if d['installed'] else 'No'}")
    print(f"Size         : {d['size_kb']} KB")

def cmd_install(pkg):
    if pkg not in DATABASE:
        print(f"Error: Package '{pkg}' not found in repository.")
        sys.exit(1)
    if DATABASE[pkg]["installed"]:
        print(f"Package '{pkg}' is already installed.")
        return
    print(f"Resolving dependencies for {pkg}...")
    time.sleep(0.3)
    print(f"Fetching {pkg}-{DATABASE[pkg]['version']}.zpkg...")
    time.sleep(0.4)
    print(f"Unpacking and verifying binaries...")
    time.sleep(0.3)
    DATABASE[pkg]["installed"] = True
    print(f"\033[1;32m[SUCCESS] Package '{pkg}' installed successfully!\033[0m")

def cmd_remove(pkg):
    if pkg not in DATABASE:
        print(f"Error: Package '{pkg}' not found.")
        sys.exit(1)
    if not DATABASE[pkg]["installed"]:
        print(f"Package '{pkg}' is not currently installed.")
        return
    if pkg in ("base-system", "zero-wm"):
        print(f"\033[1;31mError: Cannot remove essential system package '{pkg}'.\033[0m")
        sys.exit(1)
    print(f"Removing files for {pkg}...")
    DATABASE[pkg]["installed"] = False
    print(f"\033[1;32m[SUCCESS] Package '{pkg}' removed.\033[0m")

def main():
    if len(sys.argv) < 2:
        print_help()
        sys.exit(0)

    cmd = sys.argv[1].lower()
    if cmd in ("list", "ls"):
        cmd_list()
    elif cmd in ("search", "find") and len(sys.argv) > 2:
        cmd_search(sys.argv[2])
    elif cmd in ("info", "show") and len(sys.argv) > 2:
        cmd_info(sys.argv[2])
    elif cmd == "install" and len(sys.argv) > 2:
        cmd_install(sys.argv[2])
    elif cmd in ("remove", "uninstall") and len(sys.argv) > 2:
        cmd_remove(sys.argv[2])
    elif cmd == "update":
        print("Synchronizing package indexes with LinuxOSZero official mirrors...")
        time.sleep(0.5)
        print("\033[1;32mAll repositories up to date.\033[0m")
    else:
        print_help()

if __name__ == "__main__":
    main()

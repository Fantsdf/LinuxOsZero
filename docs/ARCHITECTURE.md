# LinuxOSZero Operating System Architecture Specification

## 1. System Overview

**LinuxOSZero** is a modern, lightweight, modular 64-bit Linux operating system engineered from scratch with dedicated support for **Oracle VM VirtualBox**, **QEMU**, **VMware**, and physical **x86_64** hardware.

```
+-------------------------------------------------------------------------+
|                          LinuxOSZero Userland                           |
|  +------------------+  +------------------+  +-----------------------+  |
|  |  ZeroInstaller   |  | ZeroControlPanel |  | ZeroTerminal / Editor |  |
|  +------------------+  +------------------+  +-----------------------+  |
|  +-------------------------------------------------------------------+  |
|  |                 ZeroDesktop & ZeroWM Window Manager               |  |
|  +-------------------------------------------------------------------+  |
|  |     ZeroGUI Canvas / Double-buffered Framebuffer Engine (fbdev)   |  |
|  +-------------------------------------------------------------------+  |
|  |  zero-init (PID 1)  |  zero-guest-agent  |  zpkg Package Manager  |  |
+--+---------------------+--------------------+------------------------+--+
|                           Kernel & Drivers Layer                        |
|  +---------------------+--------------------+------------------------+  |
|  | VBoxGuest Driver    | VBoxVideo (VMSVGA) | Intel e1000 / VirtIO   |  |
|  | (PCI 0x80EE:0xCAFE) | (PCI 0x80EE:0xBEEF)| Audio (AC97 / HDA)     |  |
|  +---------------------+--------------------+------------------------+  |
|  |                      Linux x86_64 Kernel Core                     |  |
+--+-------------------------------------------------------------------+--+
|                            Hardware / Hypervisor                        |
|             Oracle VM VirtualBox / QEMU / VMware / PC BIOS & UEFI       |
+-------------------------------------------------------------------------+
```

---

## 2. Boot Sequence & Initialization

1. **Stage 1 (MBR / El Torito 16-bit Boot Sector)**:
   - Resides at LBA 0 (512 bytes).
   - Initializes BIOS segments, stack, verifies INT 13h LBA extensions.
   - Activates VESA VBE linear graphics mode (Mode 0x118: 1024x768x32 or Mode 0x115: 800x600x32).
   - Enables the Fast A20 gate.
   - Loads Stage 2 / Kernel image into physical memory at `0x10000`.

2. **Stage 2 (Protected Mode -> Long Mode Trampoline)**:
   - Loads Global Descriptor Table (GDT).
   - Sets CR0.PE (enters 32-bit Protected Mode).
   - Configures PML4, PDPT, Page Directory and Page Tables (identity maps 0-8MB).
   - Enables PAE in CR4 and Long Mode Enable (LME) in EFER MSR.
   - Sets CR0.PG and performs far jump to 64-bit Long Mode code segment (`0x18`).

3. **Kernel Core & Hardware Probe**:
   - Initializes IDT, GDT, interrupt vectors, and PIC remapping.
   - Scans PCI bus:
     - Detects `0x80EE:0xCAFE` (VirtualBox VMMDev)
     - Detects `0x80EE:0xBEEF` (VirtualBox VBoxVideo / VMSVGA)
     - Detects `0x8086:0x100E` (Intel 82540EM Gigabit NIC)
     - Detects `0x8086:0x2415` / `0x8086:0x2668` (Intel AC'97 / HDA Audio)

4. **Userland Init (`zero-init` PID 1)**:
   - Mounts virtual filesystems: `/proc`, `/sys`, `/dev`, `/dev/pts`, `/dev/shm`, `/run`, `/tmp`.
   - Sets system hostname (`linuxoszero`).
   - Runs `/etc/init.d/rc.sysinit` to start hardware daemon (`mdev`) and network DHCP client (`udhcpc`).
   - Spawns `zero-guest-agent` for VirtualBox host communication.
   - Launches `zero-desktop` (or directly opens `zero-installer` in live install mode).

---

## 3. Graphics & Window Manager (ZeroWM)

- **Direct Framebuffer Rendering**: Direct double-buffered 32-bit ARGB surface blitting onto `/dev/fb0`.
- **ZeroWM Features**:
  - Movable, resizable, minimizable, and maximizable windows.
  - Active focus management, z-ordering, drop shadows, and anti-aliased font rendering.
  - Multi-language font engine with full Latin and Cyrillic (Russian) character support.
- **Desktop Shell (ZeroPanel)**:
  - Applications Start Menu ("ZERO OS").
  - Dynamic taskbar with active window switcher.
  - System tray with live clock, network monitor, volume, and VirtualBox status badge.

# LinuxOSZero Operating System Architecture Specification
# Architecture: x86_64
# Version: 1.1.0 (Titan)

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
|  | VBoxGuest Driver    | VBoxVideo (VMSVGA) | PS/2 & Evdev Keyboard  |  |
|  | (PCI 0x80EE:0xCAFE) | (PCI 0x80EE:0xBEEF)| Driver (IRQ1 / 0x21)   |  |
|  +---------------------+--------------------+------------------------+  |
|  |                      Linux x86_64 Kernel Core                     |  |
+--+-------------------------------------------------------------------+--+
|                            Hardware / Hypervisor                        |
|             Oracle VM VirtualBox / QEMU / VMware / PC BIOS & UEFI       |
+-------------------------------------------------------------------------+
```

---

## 2. Boot Sequence & Initialization (64-bit Long Mode)

1. **Stage 1 (MBR / El Torito 16-bit Boot Sector)**:
   - Resides at LBA 0 (512 bytes).
   - Initializes BIOS segments, stack, verifies INT 13h LBA extensions.
   - Activates VESA VBE linear graphics mode (Mode 0x118: 1024x768x32 with LFB `0x4000` or fallback).
   - Enables the Fast A20 gate.
   - Loads Stage 2 / Kernel image into physical memory at `0x10000` (copies up to 32 KB).

2. **Stage 2 (Protected Mode -> Long Mode Trampoline)**:
   - Loads Global Descriptor Table (GDT).
   - Sets CR0.PE (enters 32-bit Protected Mode).
   - Checks for 64-bit Long Mode CPU support via CPUID `0x80000001` (bit 29).
   - Configures 4-level paging:
     - PML4 @ `0x9000` (8 bytes per entry) -> points to PDPT @ `0xA000`.
     - PDPT @ `0xA000` (8 bytes per entry) -> points to PD0..PD3.
     - PD0..PD3 @ `0xB000..0xE000` (512 x 8-byte PDEs each, mapping 4 GiB using 2 MiB large pages).
   - Enables PAE (CR4 bit 5) and Long Mode Enable (LME in IA32_EFER MSR `0xC0000080` bit 8).
   - Sets CR0.PG and performs far jump to 64-bit Long Mode code segment (`CS=0x18`).

3. **Kernel Core & Hardware Probe**:
   - Initializes 16-byte IDT descriptors and PIC remapping (vectors 32..47).
   - Initializes PS/2 Keyboard Driver on IRQ1 (INT 33 / `0x21`).
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

## 3. Graphics, Window Manager & Keyboard Dispatcher (ZeroWM)

- **Direct Framebuffer Rendering**: Direct double-buffered 32-bit ARGB surface blitting onto `/dev/fb0`.
- **Keyboard Event Pipeline**:
  - Hardware PS/2 Controller (IRQ1 / 0x21) -> Ring Buffer -> ZeroWM -> Active Window (`on_event`).
  - Linux `evdev` (/dev/input/event*) + Terminal TTY fallback.
  - Global desktop shortcuts (Alt+F4, Super/Windows key, Alt+Tab, Ctrl+Alt+T, Ctrl+Alt+E, etc.).
- **ZeroWM Features**:
  - Movable, resizable, minimizable, and maximizable windows.
  - Active focus management, z-ordering, drop shadows, and anti-aliased font rendering.
  - Multi-language font engine with full Latin and Cyrillic (Russian) character support.

# LinuxOSZero Installation Manual (GUI & TUI Modes)

## 1. Graphical Installation (ZeroInstaller GUI)

When booting from `LinuxOSZero-v1.0.0-x86_64.iso`, the live desktop will launch with **ZeroInstaller** automatically.

### Wizard Steps:
1. **Welcome & Checklist**: Checks that VirtualBox drivers, RAM (>2GB), and storage are available.
2. **Disk Selection**:
   - Detects all block devices (`/dev/sda`, `/dev/vda`, `/dev/nvme0n1`).
   - Automatically prepares:
     - `/dev/sda1`: 512 MB FAT32 EFI/Boot partition.
     - `/dev/sda2`: 19.5 GB ext4 root partition.
3. **User Accounts**:
   - Hostname: `linuxoszero`
   - Default User: `user`
   - Default Password: `zero` (with `sudo` privileges)
   - Root Password: `root`
4. **Installation**:
   - Creates partition table (MBR/GPT).
   - Formats `ext4` filesystem with `mke2fs`.
   - Copies LinuxOSZero binaries, libraries, and desktop environment.
   - Configures VirtualBox guest modules and GRUB bootloader.
5. **Reboot**:
   - Completes installation and prompts for safe reboot into the installed system.

---

## 2. Command-Line Installation (ZeroInstaller TUI)

To run the installation via console or SSH:

```bash
# Interactive mode
zero-installer

# Automated / non-interactive mode
zero-installer --auto --disk /dev/sda --user user --host linuxoszero
```

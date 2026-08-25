#!/usr/bin/env python3
"""
LinuxOSZero ISO-9660 & El Torito Bootable ISO Image Generator
Creates hybrid bootable ISO images (.iso) compatible with VirtualBox, QEMU, VMware and PC BIOS/UEFI.
"""

import os
import sys
import struct
import time
import hashlib

SECTOR_SIZE = 2048

class ISOFileEntry:
    def __init__(self, iso_path, local_data, is_dir=False):
        self.iso_path = iso_path.strip("/")
        self.local_data = local_data  # bytes
        self.is_dir = is_dir
        self.lba = 0
        self.size = len(local_data) if not is_dir else 0

def pad_sector(data):
    rem = len(data) % SECTOR_SIZE
    if rem != 0:
        return data + b"\x00" * (SECTOR_SIZE - rem)
    return data

def make_both_endian_16(val):
    return struct.pack("<H", val) + struct.pack(">H", val)

def make_both_endian_32(val):
    return struct.pack("<I", val) + struct.pack(">I", val)

def format_iso_date(t=None):
    if t is None:
        t = time.gmtime()
    s = time.strftime("%Y%m%d%H%M%S00", t)
    # 16 bytes ASCII + 1 byte timezone offset (0 = GMT)
    return s.encode("ascii")[:16] + b"\x00"

def format_dir_date(t=None):
    if t is None:
        t = time.gmtime()
    # 7 bytes: year (since 1900), month, day, hour, min, sec, tz
    return struct.pack("BBBBBBb", t.tm_year - 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, 0)

class ISOCreator:
    def __init__(self, volume_id="LINUXOSZERO"):
        self.volume_id = volume_id[:32].ljust(32)
        self.files = {}  # iso_path -> ISOFileEntry
        self.boot_image_path = None
        self.efi_image_path = None

    def add_file(self, iso_path, data):
        iso_path = iso_path.strip("/")
        # Add parent directories
        parts = iso_path.split("/")
        for i in range(1, len(parts)):
            parent = "/".join(parts[:i])
            if parent not in self.files:
                self.files[parent] = ISOFileEntry(parent, b"", is_dir=True)
        self.files[iso_path] = ISOFileEntry(iso_path, data, is_dir=False)

    def add_local_file(self, iso_path, host_path):
        if os.path.exists(host_path):
            with open(host_path, "rb") as f:
                self.add_file(iso_path, f.read())
        else:
            print(f"[WARN] Local file {host_path} not found.")

    def set_boot_image(self, iso_path):
        self.boot_image_path = iso_path.strip("/")

    def set_efi_image(self, iso_path):
        self.efi_image_path = iso_path.strip("/")

    def build(self, output_iso_path):
        print(f"[*] Building LinuxOSZero Bootable ISO: {output_iso_path}")

        # Ensure root directory entry
        self.files[""] = ISOFileEntry("", b"", is_dir=True)

        # 1. Reserved System Area (Sectors 0 to 15 = 32 KB)
        system_area = bytearray(16 * SECTOR_SIZE)
        
        # Embed MBR / Bootloader code in first sector if available
        if self.boot_image_path and self.boot_image_path in self.files:
            boot_data = self.files[self.boot_image_path].local_data
            mbr_len = min(len(boot_data), 512)
            system_area[0:mbr_len] = boot_data[0:mbr_len]

        # Layout calculation
        # Sector 16: Primary Volume Descriptor (PVD)
        # Sector 17: Boot Record Volume Descriptor (El Torito)
        # Sector 18: Volume Descriptor Set Terminator (VDST)
        # Sector 19: Path Tables & Root Directory
        # Sector 20: Boot Catalog
        # Sector 21+: Data files
        
        current_lba = 21

        # Place Data Files
        for path, entry in sorted(self.files.items()):
            if not entry.is_dir and len(entry.local_data) > 0:
                entry.lba = current_lba
                sectors = (len(entry.local_data) + SECTOR_SIZE - 1) // SECTOR_SIZE
                current_lba += sectors

        # Place Directories
        root_dir_lba = 19
        boot_catalog_lba = 20

        # Construct Boot Catalog (El Torito)
        boot_catalog = bytearray(SECTOR_SIZE)
        # Validation Entry (32 bytes)
        boot_catalog[0] = 0x01          # Header ID
        boot_catalog[1] = 0x00          # Platform: 80x86
        boot_catalog[2:4] = b"\x00\x00" # Reserved
        boot_catalog[4:28] = b"LinuxOSZero Bootloader  " # Developer ID (24 bytes)
        boot_catalog[28:30] = b"\x55\xAA" # Checksum placeholder
        boot_catalog[30] = 0x55
        boot_catalog[31] = 0xAA

        # Fix checksum: sum of 16-bit words in 32-byte header must be 0
        chk_sum = 0
        for i in range(0, 32, 2):
            if i != 28:
                w = boot_catalog[i] | (boot_catalog[i+1] << 8)
                chk_sum = (chk_sum + w) & 0xFFFF
        neg_chk = (-chk_sum) & 0xFFFF
        boot_catalog[28] = neg_chk & 0xFF
        boot_catalog[29] = (neg_chk >> 8) & 0xFF

        # Initial / Default Boot Entry (32 bytes at offset 32)
        boot_entry_lba = self.files[self.boot_image_path].lba if (self.boot_image_path and self.boot_image_path in self.files) else current_lba
        boot_catalog[32] = 0x88         # Bootable (0x88 = bootable, 0x00 = non-bootable)
        boot_catalog[33] = 0x00         # Media: No Emulation
        boot_catalog[34:36] = b"\x00\x00" # Load segment (0x0000 = default 0x07C0)
        boot_catalog[36] = 0x00         # System type
        boot_catalog[37] = 0x00         # Unused
        # Sector count = real boot image size in 2048-byte sectors (not hardcoded).
        # This is critical for VirtualBox/QEMU to load the whole boot image
        # (boot.bin + kernel) into memory at 0x7C00.
        boot_data_len = len(self.files[self.boot_image_path].local_data) if (self.boot_image_path and self.boot_image_path in self.files) else 2048
        boot_sectors = max(1, (boot_data_len + SECTOR_SIZE - 1) // SECTOR_SIZE)
        boot_catalog[38:40] = struct.pack("<H", boot_sectors) # Sector count
        boot_catalog[40:44] = struct.pack("<I", boot_entry_lba) # Load RBA (LBA of boot image)

        # EFI Boot Section if present
        if self.efi_image_path and self.efi_image_path in self.files:
            efi_entry_lba = self.files[self.efi_image_path].lba
            # Section Header (32 bytes at offset 64)
            boot_catalog[64] = 0x91     # Section Header: Final section, Platform EFI (0xEF)
            boot_catalog[65] = 0xEF     # Platform ID: EFI
            boot_catalog[66:68] = struct.pack("<H", 1) # 1 section entry follows
            boot_catalog[68:96] = b"LinuxOSZero UEFI Boot   ".ljust(28, b"\x00")
            # Section Entry (32 bytes at offset 96)
            boot_catalog[96] = 0x88     # Bootable
            boot_catalog[97] = 0x00     # No Emulation
            boot_catalog[98:100] = b"\x00\x00"
            boot_catalog[100] = 0x00
            boot_catalog[101] = 0x00
            boot_catalog[102:104] = struct.pack("<H", 1)
            boot_catalog[104:108] = struct.pack("<I", efi_entry_lba)

        # Construct Root Directory Record
        root_dir_data = bytearray()
        
        # '.' record
        dot_rec = bytearray(34)
        dot_rec[0] = 34
        dot_rec[1] = 0
        dot_rec[2:10] = make_both_endian_32(root_dir_lba)
        dot_rec[10:18] = make_both_endian_32(SECTOR_SIZE)
        dot_rec[18:25] = format_dir_date()
        dot_rec[25] = 0x02 # Directory flag
        dot_rec[26] = 0
        dot_rec[27] = 0
        dot_rec[28:32] = make_both_endian_16(1)
        dot_rec[32] = 1 # Identifier length
        dot_rec[33] = 0x00 # '.'
        root_dir_data.extend(dot_rec)

        # '..' record
        dotdot_rec = bytearray(34)
        dotdot_rec[0] = 34
        dotdot_rec[1] = 0
        dotdot_rec[2:10] = make_both_endian_32(root_dir_lba)
        dotdot_rec[10:18] = make_both_endian_32(SECTOR_SIZE)
        dotdot_rec[18:25] = format_dir_date()
        dotdot_rec[25] = 0x02 # Directory flag
        dotdot_rec[26] = 0
        dotdot_rec[27] = 0
        dotdot_rec[28:32] = make_both_endian_16(1)
        dotdot_rec[32] = 1
        dotdot_rec[33] = 0x01 # '..'
        root_dir_data.extend(dotdot_rec)

        # Add top-level file and directory records
        for path, entry in sorted(self.files.items()):
            if not path or "/" in path:
                continue
            name_bytes = path.upper().encode("ascii")
            if not entry.is_dir:
                name_bytes += b";1"
            rec_len = 33 + len(name_bytes)
            if rec_len % 2 != 0:
                rec_len += 1

            rec = bytearray(rec_len)
            rec[0] = rec_len
            rec[1] = 0
            rec[2:10] = make_both_endian_32(entry.lba)
            rec[10:18] = make_both_endian_32(len(entry.local_data) if not entry.is_dir else SECTOR_SIZE)
            rec[18:25] = format_dir_date()
            rec[25] = 0x02 if entry.is_dir else 0x00
            rec[26] = 0
            rec[27] = 0
            rec[28:32] = make_both_endian_16(1)
            rec[32] = len(name_bytes)
            rec[33:33+len(name_bytes)] = name_bytes
            root_dir_data.extend(rec)

        root_dir_sector = pad_sector(bytes(root_dir_data))

        # Total sectors calculation
        total_sectors = current_lba

        # Sector 16: Primary Volume Descriptor (PVD)
        pvd = bytearray(SECTOR_SIZE)
        pvd[0] = 0x01                      # Type: 1 (Primary Volume Descriptor)
        pvd[1:6] = b"CD001"                # Standard Identifier
        pvd[6] = 0x01                      # Version: 1
        pvd[7] = 0x00                      # Unused
        pvd[8:40] = b"LINUXOSZERO".ljust(32) # System Identifier (32 bytes)
        pvd[40:72] = self.volume_id.encode("ascii") # Volume Identifier (32 bytes)
        pvd[72:80] = b"\x00" * 8           # Unused
        pvd[80:88] = make_both_endian_32(total_sectors) # Volume Space Size
        pvd[88:120] = b"\x00" * 32         # Unused
        pvd[120:124] = make_both_endian_16(1) # Volume Set Size
        pvd[124:128] = make_both_endian_16(1) # Volume Sequence Number
        pvd[128:132] = make_both_endian_16(SECTOR_SIZE) # Logical Block Size
        pvd[132:140] = make_both_endian_32(10) # Path Table Size
        pvd[140:144] = struct.pack("<I", 19)   # Type L Path Table Location
        pvd[148:152] = struct.pack(">I", 19)   # Type M Path Table Location
        # Root Directory Record in PVD (34 bytes at offset 156)
        pvd[156:190] = dot_rec
        pvd[190:318] = self.volume_id.encode("ascii").ljust(128) # Volume Set Identifier
        pvd[318:446] = b"LINUXOSZERO_CORP".ljust(128) # Publisher Identifier
        pvd[446:574] = b"LINUXOSZERO_BUILDER".ljust(128) # Data Preparer Identifier
        pvd[574:702] = b"LINUXOSZERO_OS".ljust(128) # Application Identifier
        pvd[813:830] = format_iso_date()       # Volume Creation Date and Time
        pvd[830:847] = format_iso_date()       # Volume Modification Date and Time
        pvd[881] = 0x01                        # File Structure Version

        # Sector 17: Boot Record Volume Descriptor (El Torito BRVD)
        brvd = bytearray(SECTOR_SIZE)
        brvd[0] = 0x00                         # Type: 0 (Boot Record)
        brvd[1:6] = b"CD001"                   # Standard Identifier
        brvd[6] = 0x01                         # Version: 1
        brvd[7:39] = b"EL TORITO SPECIFICATION".ljust(32, b"\x00") # Boot System ID
        brvd[39:71] = b"\x00" * 32             # Unused
        brvd[71:75] = struct.pack("<I", boot_catalog_lba) # Boot Catalog Pointer (LBA 20)

        # Sector 18: Volume Descriptor Set Terminator (VDST)
        vdst = bytearray(SECTOR_SIZE)
        vdst[0] = 0xFF                         # Type: 255 (Terminator)
        vdst[1:6] = b"CD001"
        vdst[6] = 0x01

        # Write output ISO
        os.makedirs(os.path.dirname(output_iso_path) or ".", exist_ok=True)
        with open(output_iso_path, "wb") as f:
            f.write(system_area)               # Sectors 0-15
            f.write(pvd)                       # Sector 16
            f.write(brvd)                      # Sector 17
            f.write(vdst)                      # Sector 18
            f.write(root_dir_sector)           # Sector 19
            f.write(boot_catalog)              # Sector 20

            # Write file contents
            for path, entry in sorted(self.files.items()):
                if not entry.is_dir and len(entry.local_data) > 0:
                    f.write(pad_sector(entry.local_data))

        iso_size = os.path.getsize(output_iso_path)
        sha256 = hashlib.sha256(open(output_iso_path, 'rb').read()).hexdigest()

        print(f"[+] Successfully created ISO: {output_iso_path}")
        print(f"    - Size   : {iso_size} bytes ({round(iso_size / (1024*1024), 2)} MB)")
        print(f"    - SHA256 : {sha256}")
        return True

def main():
    if len(sys.argv) < 2:
        print("Usage: iso_creator.py <output.iso> [files_dir]")
        sys.exit(1)

    out_iso = sys.argv[1]
    builder = ISOCreator("LINUXOSZERO_100")
    
    if len(sys.argv) > 2 and os.path.isdir(sys.argv[2]):
        for root, dirs, files in os.walk(sys.argv[2]):
            for fn in files:
                full_path = os.path.join(root, fn)
                rel_path = os.path.relpath(full_path, sys.argv[2])
                with open(full_path, "rb") as f:
                    builder.add_file(rel_path, f.read())

    builder.build(out_iso)

if __name__ == "__main__":
    main()

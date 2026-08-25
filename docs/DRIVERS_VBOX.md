# VirtualBox Guest Drivers & Hardware Integration Guide

## Overview

LinuxOSZero contains specialized built-in drivers for Oracle VM VirtualBox hypervisor communication:

| Component | PCI ID / Port | Protocol & Functionality |
| :--- | :--- | :--- |
| **VBoxGuest (VMMDev)** | `0x80EE:0xCAFE` / I/O `0xD020` | Host communication channel, hypercalls, host version probe |
| **VBoxVideo (VMSVGA)** | `0x80EE:0xBEEF` / I/O `0x01CE` | Display modesetting, dynamic resolution resize, hardware blits |
| **VBoxMouse** | VMMDev mouse integration | Absolute pointing device (no mouse capture required in VM) |
| **VBoxSF (Shared Folders)**| VMMDev HGCM service | Auto-mounts host shared folder to `/media/sf_shared` |
| **Time Synchronization** | VMMDev Host Time | Synchronizes guest RTC clock with host clock |

---

## 1. VMMDev Protocol (`src/drivers/vboxguest.c`)

The VMMDev driver communicates via I/O Port `0xD020` (or BAR0 I/O space):

```c
typedef struct {
    uint32_t size;
    uint32_t version;
    uint32_t request_type;
    int32_t  rc;
    uint32_t reserved1;
    uint32_t reserved2;
} vbox_header_t;
```

Capabilities reported to host:
- `VBOX_GUEST_CAP_MOUSE_INTEGRATION (0x04)`
- `VBOX_GUEST_CAP_AUTORESIZE (0x20)`
- `VBOX_GUEST_CAP_SHARED_FOLDERS (0x10)`
- `VBOX_GUEST_CAP_SHARED_CLIPBOARD (0x08)`

---

## 2. VBoxVideo Modesetting (`src/drivers/vboxvideo.c`)

Display configuration is performed using Bochs/VBE Dispi I/O ports `0x01CE` (Index) and `0x01CF` (Data):

```c
/* Setting mode 1024x768x32 with Linear Frame Buffer */
vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
vbe_write(VBE_DISPI_INDEX_XRES, 1024);
vbe_write(VBE_DISPI_INDEX_YRES, 768);
vbe_write(VBE_DISPI_INDEX_BPP, 32);
vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
```

Supported resolutions in LinuxOSZero:
- `1024 x 768` (4:3 Standard)
- `1280 x 720` (16:9 HD)
- `1280 x 800` (16:10 WXGA)
- `1440 x 900` (16:10 WXGA+)
- `1600 x 900` (16:9 HD+)
- `1920 x 1080` (16:9 Full HD)

---

## 3. CLI Management Tools

- `zero-vbox-control status`: Show VirtualBox guest additions status.
- `zero-vbox-control mount [share] [path]`: Mount host shared folder.
- `zero-vbox-control resize <width> <height>`: Dynamically adjust display resolution.
- `zero-vbox-control timesync`: Synchronize guest clock with host RTC.

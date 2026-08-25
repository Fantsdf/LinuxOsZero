/*
 * LinuxOSZero - VirtualBox Display Driver Implementation
 * Architecture: x86_64
 *
 * Handles Bochs/VBE Dispi I/O (0x01CE/0x01CF) and VirtualBox VMSVGA / VBoxVideo
 * display adapter modesetting with strict boundary verification to eliminate
 * VirtualBox DisplayWrap -52 (0x8000ffff) errors.
 */

#include "vboxvideo.h"
#include "../kernel/kernel.h"
#include "../kernel/pci.h"

static vbox_display_mode_t current_mode = {
    .width = 1024,
    .height = 768,
    .bpp = 32,
    .pitch = 1024 * 4,
    .framebuffer = (uint32_t *)0xE0000000,
    .is_hardware_accelerated = true
};

static uint32_t vbox_vram_size = 128 * 1024 * 1024; /* Default 128 MB */
static bool vbe_available = false;

static void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

static uint16_t vbe_read(uint16_t index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

int vboxvideo_init(void) {
    /* Check VBE / VirtualBox display adapter (Bochs/VBE ID range 0xB0C0..0xB0C6) */
    uint16_t id = vbe_read(VBE_DISPI_INDEX_ID);
    if (id >= 0xB0C0 && id <= 0xB0C6) {
        vbe_available = true;
    }

    /* Probe PCI for physical framebuffer address (VBox VGA 0x80EE:0xBEEF or Class 0x0300) */
    pci_device_t *vga = pci_find_device(PCI_VENDOR_VBOX, PCI_DEVICE_VBOX_VIDEO);
    if (!vga) {
        vga = pci_find_class(0x03, 0x00);
    }
    if (!vga) {
        vga = pci_find_device(0x15AD, 0x0405); /* VMware / VirtualBox VMSVGA */
    }

    if (vga) {
        if (vga->bar[0] & ~0x0F) {
            uint32_t fb_addr = (vga->bar[0] & ~0x0F);
            current_mode.framebuffer = (uint32_t *)(uintptr_t)fb_addr;
            g_sysinfo.framebuffer = (uint32_t *)(uintptr_t)fb_addr;
        }
        if (vga->bar[1] & ~0x0F) {
            vbox_vram_size = (vga->bar[1] & ~0x0F);
        }
    }

    if (!current_mode.framebuffer) {
        current_mode.framebuffer = (uint32_t *)0xE0000000;
        g_sysinfo.framebuffer = (uint32_t *)0xE0000000;
    }

    return 0;
}

/*
 * Validate that the adapter supports the requested mode without mutating
 * hardware register states. Strict bounds checking prevents VirtualBox
 * DisplayWrap -52 / 0x8000ffff errors.
 */
int vboxvideo_mode_supported(uint32_t width, uint32_t height, uint32_t bpp) {
    if (width == 0 || width > 3840 || height == 0 || height > 2160) {
        return -1;
    }
    if (bpp != 16 && bpp != 24 && bpp != 32) {
        return -1;
    }

    uint64_t req_bytes = (uint64_t)width * height * (bpp / 8);
    if (vbox_vram_size > 0 && req_bytes > vbox_vram_size) {
        return -1;
    }

    return 0;
}

int vboxvideo_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
    if (width == 0 || width > 3840 || height == 0 || height > 2160) {
        return -1;
    }
    if (bpp != 16 && bpp != 24 && bpp != 32) {
        return -1;
    }

    uint64_t req_bytes = (uint64_t)width * height * (bpp / 8);
    if (vbox_vram_size > 0 && req_bytes > vbox_vram_size) {
        return -1;
    }

    if (vbe_available) {
        /* Disable display during reconfiguration to prevent race conditions */
        vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
        vbe_write(VBE_DISPI_INDEX_XRES, (uint16_t)width);
        vbe_write(VBE_DISPI_INDEX_YRES, (uint16_t)height);
        vbe_write(VBE_DISPI_INDEX_BPP, (uint16_t)bpp);
        vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, (uint16_t)width);
        vbe_write(VBE_DISPI_INDEX_VIRT_HEIGHT, (uint16_t)height);
        vbe_write(VBE_DISPI_INDEX_X_OFFSET, 0);
        vbe_write(VBE_DISPI_INDEX_Y_OFFSET, 0);
        vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    }

    current_mode.width = width;
    current_mode.height = height;
    current_mode.bpp = bpp;
    current_mode.pitch = width * (bpp / 8);

    g_sysinfo.screen_width = width;
    g_sysinfo.screen_height = height;
    g_sysinfo.screen_pitch = current_mode.pitch;
    g_sysinfo.screen_bpp = bpp;

    return 0;
}

void vboxvideo_get_current_mode(vbox_display_mode_t *mode) {
    if (mode) {
        *mode = current_mode;
    }
}

void vboxvideo_get_vram_size(uint32_t *vram_mb) {
    if (vram_mb) {
        *vram_mb = (vbox_vram_size > 0) ? (vbox_vram_size / (1024 * 1024)) : 0;
    }
}

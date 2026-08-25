/*
 * LinuxOSZero - VirtualBox Display Driver Implementation
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

static void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

static uint16_t vbe_read(uint16_t index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static uint32_t vbox_vram_size = 0;

int vboxvideo_init(void) {
    /* Check VBE / VirtualBox display adapter (Bochs/VBE ID range 0xB0C0..0xB0C6) */
    uint16_t id = vbe_read(VBE_DISPI_INDEX_ID);
    if (id < 0xB0C0 || id > 0xB0C6) {
        return -1;
    }

    /* Probe PCI BAR0 for the physical framebuffer address if available */
    pci_device_t *vbox_vga = pci_find_device(PCI_VENDOR_VBOX, PCI_DEVICE_VBOX_VIDEO);
    if (vbox_vga) {
        current_mode.framebuffer = (uint32_t *)(uintptr_t)(vbox_vga->bar[0] & ~0x0F);
        /* BAR1/2 region size usually corresponds to video memory */
        vbox_vram_size = (vbox_vga->bar[1] & ~0x0F);
    }

    /* Fall back to a reasonable default if no BAR was found */
    if (!current_mode.framebuffer) {
        current_mode.framebuffer = (uint32_t *)0xE0000000;
    }

    return vboxvideo_set_mode(1024, 768, 32);
}

/* Validate that the adapter can actually switch to the requested mode.
 * Returns 0 on success, -1 if the adapter rejects the configuration. */
int vboxvideo_mode_supported(uint32_t width, uint32_t height, uint32_t bpp) {
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    vbe_write(VBE_DISPI_INDEX_XRES, (uint16_t)width);
    vbe_write(VBE_DISPI_INDEX_YRES, (uint16_t)height);
    vbe_write(VBE_DISPI_INDEX_BPP, (uint16_t)bpp);
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    uint16_t actual_x = vbe_read(VBE_DISPI_INDEX_XRES);
    uint16_t actual_y = vbe_read(VBE_DISPI_INDEX_YRES);
    uint16_t actual_bpp = vbe_read(VBE_DISPI_INDEX_BPP);

    /* Re-apply the current mode so nothing is left in a broken state */
    vboxvideo_set_mode(current_mode.width, current_mode.height, current_mode.bpp);

    if (actual_x == (uint16_t)width && actual_y == (uint16_t)height && actual_bpp == (uint16_t)bpp) {
        return 0;
    }
    return -1;
}

int vboxvideo_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
    if (bpp != 16 && bpp != 24 && bpp != 32) {
        return -1;
    }

    /* Disable display during reconfiguration */
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    vbe_write(VBE_DISPI_INDEX_XRES, (uint16_t)width);
    vbe_write(VBE_DISPI_INDEX_YRES, (uint16_t)height);
    vbe_write(VBE_DISPI_INDEX_BPP, (uint16_t)bpp);
    vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, (uint16_t)width);
    vbe_write(VBE_DISPI_INDEX_VIRT_HEIGHT, (uint16_t)height);
    vbe_write(VBE_DISPI_INDEX_X_OFFSET, 0);
    vbe_write(VBE_DISPI_INDEX_Y_OFFSET, 0);
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

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

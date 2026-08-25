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

int vboxvideo_init(void) {
    /* Check VBE / VirtualBox display adapter */
    uint16_t id = vbe_read(VBE_DISPI_INDEX_ID);
    if (id < 0xB0C0 || id > 0xB0C6) {
        return -1;
    }

    /* Probe PCI BAR0 for physical framebuffer address if available */
    pci_device_t *vbox_vga = pci_find_device(PCI_VENDOR_VBOX, PCI_DEVICE_VBOX_VIDEO);
    if (vbox_vga) {
        current_mode.framebuffer = (uint32_t *)(uintptr_t)(vbox_vga->bar[0] & ~0x0F);
    }

    return vboxvideo_set_mode(1024, 768, 32);
}

int vboxvideo_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
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

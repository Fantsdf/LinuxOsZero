/*
 * LinuxOSZero - VirtualBox / Bochs Display Driver (VBoxVideo / VMSVGA)
 */

#ifndef VBOXVIDEO_H
#define VBOXVIDEO_H

#include <stdint.h>
#include <stdbool.h>

#define VBE_DISPI_IOPORT_INDEX          0x01CE
#define VBE_DISPI_IOPORT_DATA           0x01CF

#define VBE_DISPI_INDEX_ID              0x00
#define VBE_DISPI_INDEX_XRES            0x01
#define VBE_DISPI_INDEX_YRES            0x02
#define VBE_DISPI_INDEX_BPP             0x03
#define VBE_DISPI_INDEX_ENABLE          0x04
#define VBE_DISPI_INDEX_BANK            0x05
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x06
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x07
#define VBE_DISPI_INDEX_X_OFFSET        0x08
#define VBE_DISPI_INDEX_Y_OFFSET        0x09

#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_LFB_ENABLED           0x40
#define VBE_DISPI_NOCLEARMEM            0x80

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t *framebuffer;
    bool is_hardware_accelerated;
} vbox_display_mode_t;

int vboxvideo_init(void);
int vboxvideo_set_mode(uint32_t width, uint32_t height, uint32_t bpp);
int vboxvideo_mode_supported(uint32_t width, uint32_t height, uint32_t bpp);
void vboxvideo_get_current_mode(vbox_display_mode_t *mode);
void vboxvideo_get_vram_size(uint32_t *vram_mb);

#endif /* VBOXVIDEO_H */

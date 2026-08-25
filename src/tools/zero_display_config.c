/*
 * LinuxOSZero - Display Configuration Tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../drivers/vboxvideo.h"
#include "../kernel/kernel.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("LinuxOSZero Display Configuration Utility\n");
        printf("Usage: zero-display-config <set|get|list> [width] [height] [bpp]\n\n");
        printf("Examples:\n");
        printf("  zero-display-config set 1920 1080 32\n");
        printf("  zero-display-config set 1280 800 32\n");
        printf("  zero-display-config get\n");
        return 0;
    }

    if (strcmp(argv[1], "get") == 0) {
        vbox_display_mode_t mode;
        vboxvideo_get_current_mode(&mode);
        printf("Current Resolution: %dx%d (%d bpp, pitch: %d bytes)\n", mode.width, mode.height, mode.bpp, mode.pitch);
        return 0;
    }

    if (strcmp(argv[1], "set") == 0 && argc >= 4) {
        uint32_t w = atoi(argv[2]);
        uint32_t h = atoi(argv[3]);
        uint32_t bpp = (argc >= 5) ? atoi(argv[4]) : 32;

        printf("Configuring VBoxVideo / VMSVGA display to %dx%dx%d...\n", w, h, bpp);
        if (vboxvideo_set_mode(w, h, bpp) == 0) {
            printf("[OK] Display mode switched successfully!\n");
            return 0;
        } else {
            printf("[ERR] Failed to switch display mode.\n");
            return 1;
        }
    }

    if (strcmp(argv[1], "list") == 0) {
        printf("Supported VirtualBox Display Modes:\n");
        printf("  - 1024x768x32  (Standard 4:3)\n");
        printf("  - 1280x720x32  (HD 16:9)\n");
        printf("  - 1280x800x32  (WXGA 16:10)\n");
        printf("  - 1440x900x32  (WXGA+ 16:10)\n");
        printf("  - 1600x900x32  (HD+ 16:9)\n");
        printf("  - 1920x1080x32 (Full HD 16:9)\n");
        return 0;
    }

    return 0;
}

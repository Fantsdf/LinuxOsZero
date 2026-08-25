/*
 * LinuxOSZero - System Icons & Graphics
 */

#ifndef ICONS_H
#define ICONS_H

#include "../drivers/fbdev.h"
#include "canvas.h"

typedef enum {
    ICON_INSTALLER,
    ICON_TERMINAL,
    ICON_FILE_MANAGER,
    ICON_CONTROL_PANEL,
    ICON_EDITOR,
    ICON_SYSTEM_INFO,
    ICON_VIRTUALBOX,
    ICON_POWER,
    ICON_NETWORK,
    ICON_VOLUME,
    ICON_FOLDER,
    ICON_FILE,
    ICON_HARD_DISK
} icon_type_t;

void icons_draw(icon_type_t type, int x, int y, int size, color_t tint);
void icons_draw_logo(int cx, int cy, int radius);

#endif /* ICONS_H */

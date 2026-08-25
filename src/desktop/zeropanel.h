/*
 * LinuxOSZero - Desktop Panel & Taskbar (ZeroPanel)
 */

#ifndef ZEROPANEL_H
#define ZEROPANEL_H

#include "../drivers/fbdev.h"
#include "../drivers/input.h"
#include "zerowm.h"
#include <stdbool.h>

#define PANEL_HEIGHT 40

typedef struct {
    int screen_w;
    int screen_h;
    bool start_menu_open;
    bool calendar_open;
    int hovered_item;
} zeropanel_t;

extern zeropanel_t g_panel;

void panel_init(int screen_w, int screen_h);
void panel_handle_input(mouse_state_t *mouse);
void panel_render(void);

#endif /* ZEROPANEL_H */

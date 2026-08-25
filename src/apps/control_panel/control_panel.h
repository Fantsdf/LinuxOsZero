/*
 * LinuxOSZero - Control Panel & Settings (ZeroControlPanel)
 */

#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include "../../desktop/zerowm.h"

void app_launch_control_panel(void);
void control_panel_render(window_t *win, int cx, int cy, int cw, int ch);
void control_panel_on_event(window_t *win, int ev_type, int p1, int p2);

#endif /* CONTROL_PANEL_H */

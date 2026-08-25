/*
 * LinuxOSZero - Terminal Emulator Header
 * Architecture: x86_64
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "../../desktop/zerowm.h"

void app_launch_terminal(void);
void terminal_render(window_t *win, int cx, int cy, int cw, int ch);
void terminal_on_event(window_t *win, int ev_type, int p1, int p2);

#endif /* TERMINAL_H */

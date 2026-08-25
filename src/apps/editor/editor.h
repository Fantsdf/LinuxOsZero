/*
 * LinuxOSZero - Text & Code Editor (ZeroEditor)
 */

#ifndef EDITOR_H
#define EDITOR_H

#include "../../desktop/zerowm.h"

void app_launch_editor(void);
void editor_render(window_t *win, int cx, int cy, int cw, int ch);
void editor_on_event(window_t *win, int ev_type, int p1, int p2);

#endif /* EDITOR_H */

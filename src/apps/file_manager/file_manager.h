/*
 * LinuxOSZero - File Manager (ZeroFileManager)
 */

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "../../desktop/zerowm.h"

void app_launch_file_manager(void);
void file_manager_render(window_t *win, int cx, int cy, int cw, int ch);
void file_manager_on_event(window_t *win, int ev_type, int p1, int p2);

#endif /* FILE_MANAGER_H */

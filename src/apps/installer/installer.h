/*
 * LinuxOSZero - System Installer (ZeroInstaller)
 */

#ifndef INSTALLER_H
#define INSTALLER_H

#include "../../desktop/zerowm.h"
#include <stdbool.h>

typedef enum {
    STEP_WELCOME,
    STEP_DISK_SELECT,
    STEP_USER_CONFIG,
    STEP_INSTALLING,
    STEP_COMPLETE
} installer_step_t;

typedef struct {
    installer_step_t current_step;
    char target_disk[64];
    char username[32];
    char hostname[32];
    char password[32];
    char root_password[32];
    int progress_percent;
    char status_message[128];
    bool install_started;
    bool install_finished;
} installer_state_t;

extern installer_state_t g_installer;

void app_launch_installer(void);
void installer_render(window_t *win, int cx, int cy, int cw, int ch);
void installer_on_event(window_t *win, int ev_type, int p1, int p2);
int installer_execute_install(void);

#endif /* INSTALLER_H */

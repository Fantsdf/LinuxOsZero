/*
 * LinuxOSZero - Graphical Theme & Styling
 */

#ifndef THEME_H
#define THEME_H

#include "../drivers/fbdev.h"

typedef struct {
    color_t bg_desktop;
    color_t panel_bg;
    color_t panel_border;
    color_t panel_text;
    color_t accent_primary;
    color_t accent_hover;
    color_t accent_active;
    color_t win_bg;
    color_t win_titlebar_active;
    color_t win_titlebar_inactive;
    color_t win_titlebar_text_active;
    color_t win_titlebar_text_inactive;
    color_t win_border;
    color_t win_shadow;
    color_t btn_bg;
    color_t btn_hover;
    color_t btn_text;
    color_t btn_close;
    color_t btn_min;
    color_t btn_max;
    color_t text_primary;
    color_t text_secondary;
    color_t text_muted;
    color_t card_bg;
    color_t card_border;
    color_t success;
    color_t warning;
    color_t danger;
    color_t info;
} theme_t;

extern theme_t g_theme;

void theme_init_dark(void);
void theme_init_light(void);

#endif /* THEME_H */

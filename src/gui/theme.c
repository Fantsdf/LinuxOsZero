/*
 * LinuxOSZero - Theme Implementations
 */

#include "theme.h"

theme_t g_theme;

void theme_init_dark(void) {
    g_theme.bg_desktop              = COLOR_RGB(15, 23, 42);      /* #0f172a Deep Slate */
    g_theme.panel_bg                = COLOR_RGBA(15, 23, 42, 235); /* Translucent bar */
    g_theme.panel_border            = COLOR_RGB(30, 41, 59);      /* #1e293b */
    g_theme.panel_text              = COLOR_RGB(241, 245, 249);   /* #f1f5f9 */
    g_theme.accent_primary          = COLOR_RGB(14, 165, 233);    /* #0ea5e9 Sky Blue */
    g_theme.accent_hover            = COLOR_RGB(56, 189, 248);    /* #38bdf8 */
    g_theme.accent_active           = COLOR_RGB(2, 132, 199);     /* #0284c7 */
    g_theme.win_bg                  = COLOR_RGB(30, 41, 59);      /* #1e293b */
    g_theme.win_titlebar_active     = COLOR_RGB(15, 23, 42);      /* #0f172a */
    g_theme.win_titlebar_inactive   = COLOR_RGB(30, 41, 59);      /* #1e293b */
    g_theme.win_titlebar_text_active= COLOR_RGB(248, 250, 252);   /* #f8fafc */
    g_theme.win_titlebar_text_inactive = COLOR_RGB(148, 163, 184);/* #94a3b8 */
    g_theme.win_border              = COLOR_RGB(51, 65, 85);      /* #334155 */
    g_theme.win_shadow              = COLOR_RGBA(0, 0, 0, 90);
    g_theme.btn_bg                  = COLOR_RGB(51, 65, 85);      /* #334155 */
    g_theme.btn_hover               = COLOR_RGB(71, 85, 105);     /* #475569 */
    g_theme.btn_text                = COLOR_RGB(248, 250, 252);   /* #f8fafc */
    g_theme.btn_close               = COLOR_RGB(239, 68, 68);     /* #ef4444 Red */
    g_theme.btn_min                 = COLOR_RGB(234, 179, 8);     /* #eab308 Yellow */
    g_theme.btn_max                 = COLOR_RGB(34, 197, 94);     /* #22c55e Green */
    g_theme.text_primary            = COLOR_RGB(248, 250, 252);   /* #f8fafc */
    g_theme.text_secondary          = COLOR_RGB(203, 213, 225);   /* #cbd5e1 */
    g_theme.text_muted              = COLOR_RGB(148, 163, 184);   /* #94a3b8 */
    g_theme.card_bg                 = COLOR_RGB(15, 23, 42);      /* #0f172a */
    g_theme.card_border             = COLOR_RGB(51, 65, 85);      /* #334155 */
    g_theme.success                 = COLOR_RGB(34, 197, 94);     /* #22c55e */
    g_theme.warning                 = COLOR_RGB(234, 179, 8);     /* #eab308 */
    g_theme.danger                  = COLOR_RGB(239, 68, 68);     /* #ef4444 */
    g_theme.info                    = COLOR_RGB(56, 189, 248);    /* #38bdf8 */
}

void theme_init_light(void) {
    g_theme.bg_desktop              = COLOR_RGB(241, 245, 249);
    g_theme.panel_bg                = COLOR_RGBA(255, 255, 255, 240);
    g_theme.panel_border            = COLOR_RGB(203, 213, 225);
    g_theme.panel_text              = COLOR_RGB(15, 23, 42);
    g_theme.accent_primary          = COLOR_RGB(2, 132, 199);
    g_theme.accent_hover            = COLOR_RGB(14, 165, 233);
    g_theme.accent_active           = COLOR_RGB(3, 105, 161);
    g_theme.win_bg                  = COLOR_RGB(255, 255, 255);
    g_theme.win_titlebar_active     = COLOR_RGB(226, 232, 240);
    g_theme.win_titlebar_inactive   = COLOR_RGB(241, 245, 249);
    g_theme.win_titlebar_text_active= COLOR_RGB(15, 23, 42);
    g_theme.win_titlebar_text_inactive = COLOR_RGB(100, 116, 139);
    g_theme.win_border              = COLOR_RGB(203, 213, 225);
    g_theme.win_shadow              = COLOR_RGBA(0, 0, 0, 40);
    g_theme.btn_bg                  = COLOR_RGB(226, 232, 240);
    g_theme.btn_hover               = COLOR_RGB(203, 213, 225);
    g_theme.btn_text                = COLOR_RGB(15, 23, 42);
    g_theme.btn_close               = COLOR_RGB(239, 68, 68);
    g_theme.btn_min                 = COLOR_RGB(234, 179, 8);
    g_theme.btn_max                 = COLOR_RGB(34, 197, 94);
    g_theme.text_primary            = COLOR_RGB(15, 23, 42);
    g_theme.text_secondary          = COLOR_RGB(51, 65, 85);
    g_theme.text_muted              = COLOR_RGB(100, 116, 139);
    g_theme.card_bg                 = COLOR_RGB(248, 250, 252);
    g_theme.card_border             = COLOR_RGB(226, 232, 240);
    g_theme.success                 = COLOR_RGB(22, 163, 74);
    g_theme.warning                 = COLOR_RGB(202, 138, 4);
    g_theme.danger                  = COLOR_RGB(220, 38, 38);
    g_theme.info                    = COLOR_RGB(2, 132, 199);
}

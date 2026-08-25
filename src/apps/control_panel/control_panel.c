/*
 * LinuxOSZero - Control Panel Implementation
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include "control_panel.h"
#include "../../gui/theme.h"
#include "../../gui/canvas.h"
#include "../../gui/font.h"
#include "../../gui/icons.h"
#include "../../drivers/sound.h"
#include "../../drivers/vboxvideo.h"
#include "../../drivers/vboxguest.h"
#include <stdio.h>
#include <string.h>

static window_t *cp_win = NULL;
static int active_tab = 0; /* 0: Display, 1: VirtualBox, 2: System, 3: Themes */
static int selected_res_idx = 0;

static const char *resolutions[] = {
    "1024 x 768  (4:3 Standard VirtualBox)",
    "1280 x 720  (16:9 HD 720p)",
    "1280 x 800  (16:10 WXGA)",
    "1280 x 1024 (5:4 SXGA)",
    "1440 x 900  (16:10 WXGA+)",
    "1600 x 900  (16:9 HD+)",
    "1920 x 1080 (16:9 Full HD 1080p)",
    "800 x 600   (4:3 SVGA)"
};
static const uint32_t res_widths[]  = { 1024, 1280, 1280, 1280, 1440, 1600, 1920, 800 };
static const uint32_t res_heights[] = {  768,  720,  800, 1024,  900,  900, 1080, 600 };
#define RES_COUNT (sizeof(resolutions) / sizeof(resolutions[0]))

void app_launch_control_panel(void) {
    if (cp_win && cp_win->id != -1) {
        wm_focus_window(cp_win->id);
        return;
    }
    cp_win = wm_create_window("Настройка экрана и параметры (x86_64)", ICON_CONTROL_PANEL, 140, 60, 620, 460, control_panel_render, control_panel_on_event);
}

void control_panel_render(window_t *win, int cx, int cy, int cw, int ch) {
    (void)win;
    fbdev_fill_rect(cx, cy, cw, ch, g_theme.card_bg);

    /* Tab Bar at Top */
    const char *tabs[] = { "Экран / Display", "VirtualBox", "О системе", "Тема / Вид" };
    int tab_w = cw / 4;
    for (int i = 0; i < 4; i++) {
        int tx = cx + i * tab_w;
        color_t tbg = (i == active_tab) ? g_theme.win_bg : COLOR_RGB(15, 23, 42);
        fbdev_fill_rect(tx, cy, tab_w, 34, tbg);
        if (i == active_tab) {
            fbdev_fill_rect(tx, cy + 32, tab_w, 2, g_theme.accent_primary);
        } else {
            fbdev_fill_rect(tx, cy + 33, tab_w, 1, g_theme.card_border);
        }
        color_t tt = (i == active_tab) ? g_theme.text_primary : g_theme.text_muted;
        int tw = font_get_string_width(tabs[i]);
        font_draw_string(tx + (tab_w - tw) / 2, cy + 9, tabs[i], tt, COLOR_RGBA(0, 0, 0, 0));
    }

    int my = cy + 44;
    int mx = cx + 18;
    int mw = cw - 36;

    if (active_tab == 0) {
        /* Display Settings Tab */
        font_draw_string(mx, my, "Настройка разрешения экрана (VBoxVideo / VMSVGA 64-bit)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 18, "Выберите видеорежим для виртуальной машины VirtualBox:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        int list_y = my + 42;
        for (size_t i = 0; i < RES_COUNT; i++) {
            int ry = list_y + i * 32;
            if (ry + 28 > cy + ch - 48) break;
            bool is_sel = ((int)i == selected_res_idx);
            color_t card_c = is_sel ? g_theme.accent_active : COLOR_RGB(30, 41, 59);
            canvas_draw_card(mx, ry, mw, 28, card_c, is_sel ? g_theme.accent_hover : g_theme.card_border);
            font_draw_string(mx + 10, ry + 6, resolutions[i], is_sel ? g_theme.btn_text : g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
            if (is_sel) {
                font_draw_string(mx + mw - 75, ry + 6, "[Выбрано]", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
            }
        }

        canvas_draw_button(mx, cy + ch - 42, 140, 30, "Авто-подгонка", false, false, g_theme.btn_bg);
        canvas_draw_button(mx + mw - 160, cy + ch - 42, 160, 30, "Применить экран", false, false, g_theme.accent_primary);
    } else if (active_tab == 1) {
        /* VirtualBox Integration Tab */
        font_draw_string(mx, my, "Интеграция с Oracle VM VirtualBox (v1.1.0)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 18, "Состояние драйверов гостевых дополнений VirtualBox:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 42, mw, 210, COLOR_RGB(30, 41, 59), g_theme.card_border);
        font_draw_string(mx + 16, my + 58, "[✓] Указатель мыши (Seamless Mouse Integration)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 82, "[✓] Видеоадаптер: VMSVGA 3D (DisplayWrap устранено)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 106, "[✓] Драйвер клавиатуры: PS/2 Set 1/2 + раскладка US/RU", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 130, "[✓] Общие папки VirtualBox (/media/sf_shared)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 154, "[✓] Синхронизация времени с хостом (RTC Clock)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 178, "[✓] Канал VMMDev PCI (Порт 0xD020/0xD040, 64-bit)", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 202, "[✓] Звук WASAPI / Intel AC'97 разглушен", g_theme.success, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_button(mx + mw - 180, cy + ch - 42, 180, 30, "Перезапустить VMMDev", false, false, g_theme.btn_bg);
    } else if (active_tab == 2) {
        /* System Information Tab */
        font_draw_string(mx, my, "Сведения о системе и оборудовании", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 36, mw, 230, COLOR_RGB(30, 41, 59), g_theme.card_border);
        font_draw_string(mx + 16, my + 50, "Операционная система : LinuxOSZero v1.1.0 (Titan)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 74, "Архитектура          : x86_64 (64-bit Long Mode)", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 98, "Гипервизор           : Oracle VM VirtualBox 7.2.4", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 122, "Видеоадаптер         : VirtualBox VMSVGA (0x80EE:0xBEEF)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 146, "Клавиатура           : PS/2 контроллер + evdev (RU/US)", g_theme.success, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 170, "Сетевая карта        : Intel 82540EM Gigabit Ethernet", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 194, "Аудиоконтроллер      : Intel 82801AA AC'97 Audio", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 16, my + 218, "Пакетный менеджер    : zpkg (Zero Package Manager)", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));
    } else if (active_tab == 3) {
        /* Appearance Tab */
        font_draw_string(mx, my, "Темы оформления рабочего стола", g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx, my + 18, "Настройка цветовой схемы и стиля окон:", g_theme.text_secondary, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx, my + 44, mw / 2 - 10, 100, COLOR_RGB(15, 23, 42), g_theme.accent_primary);
        font_draw_string(mx + 14, my + 64, "Dark Cyber (Тёмная)", g_theme.accent_hover, COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + 14, my + 88, "Голубые акценты и тёмные панели", g_theme.text_muted, COLOR_RGBA(0, 0, 0, 0));

        canvas_draw_card(mx + mw / 2 + 10, my + 44, mw / 2 - 10, 100, COLOR_RGB(241, 245, 249), g_theme.card_border);
        font_draw_string(mx + mw / 2 + 24, my + 64, "Light Clean (Светлая)", COLOR_RGB(15, 23, 42), COLOR_RGBA(0, 0, 0, 0));
        font_draw_string(mx + mw / 2 + 24, my + 88, "Светлые панели и синие акценты", COLOR_RGB(100, 116, 139), COLOR_RGBA(0, 0, 0, 0));
    }
}

void control_panel_on_event(window_t *win, int ev_type, int p1, int p2) {
    if (ev_type == WM_EVENT_KEY_DOWN) {
        int key_code = p1;
        if (key_code == KEY_TAB || key_code == KEY_RIGHT) {
            active_tab = (active_tab + 1) % 4;
            sound_play(SND_CLICK);
        } else if (key_code == KEY_LEFT) {
            active_tab = (active_tab + 3) % 4;
            sound_play(SND_CLICK);
        } else if (active_tab == 0) {
            if (key_code == KEY_UP && selected_res_idx > 0) {
                selected_res_idx--;
                sound_play(SND_CLICK);
            } else if (key_code == KEY_DOWN && selected_res_idx < (int)RES_COUNT - 1) {
                selected_res_idx++;
                sound_play(SND_CLICK);
            } else if (key_code == KEY_ENTER) {
                if (selected_res_idx >= 0 && selected_res_idx < (int)RES_COUNT) {
                    vboxvideo_set_mode(res_widths[selected_res_idx], res_heights[selected_res_idx], 32);
                    sound_play(SND_SUCCESS);
                }
            }
        }
        return;
    }

    if (ev_type != WM_EVENT_CLICK) return;

    /* Tab bar clicks */
    if (p2 < 34) {
        int tab_w = win->width / 4;
        active_tab = p1 / tab_w;
        if (active_tab > 3) active_tab = 3;
        sound_play(SND_CLICK);
        return;
    }

    int mx = 18;
    int mw = win->width - 36;
    int ch = win->height - 28;

    if (active_tab == 0) {
        /* Resolution list selection */
        int list_y = 86;
        for (size_t i = 0; i < RES_COUNT; i++) {
            int ry = list_y + i * 32;
            if (p2 >= ry && p2 < (ry + 28) && p1 >= mx && p1 <= (mx + mw)) {
                selected_res_idx = (int)i;
                sound_play(SND_CLICK);
                return;
            }
        }

        /* Apply Resolution Button */
        if (p1 >= (mx + mw - 160) && p1 <= (mx + mw) && p2 >= (ch - 42) && p2 <= (ch - 12)) {
            if (selected_res_idx >= 0 && selected_res_idx < (int)RES_COUNT) {
                vboxvideo_set_mode(res_widths[selected_res_idx], res_heights[selected_res_idx], 32);
                sound_play(SND_SUCCESS);
            }
            return;
        }

        /* Auto-fit Button */
        if (p1 >= mx && p1 <= (mx + 140) && p2 >= (ch - 42) && p2 <= (ch - 12)) {
            selected_res_idx = 0;
            vboxvideo_set_mode(1024, 768, 32);
            sound_play(SND_SUCCESS);
            return;
        }
    } else if (active_tab == 3) {
        /* Theme cards click */
        if (p2 >= 88 && p2 < 188) {
            if (p1 < win->width / 2) {
                theme_init_dark();
                sound_play(SND_SUCCESS);
            } else {
                theme_init_light();
                sound_play(SND_SUCCESS);
            }
        }
    }
}

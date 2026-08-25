/*
 * LinuxOSZero - Graphical Desktop Environment (ZeroDesktop Main)
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "../drivers/fbdev.h"
#include "../drivers/input.h"
#include "../drivers/sound.h"
#include "../gui/theme.h"
#include "../gui/canvas.h"
#include "../gui/font.h"
#include "../gui/icons.h"
#include "../gui/wallpaper.h"
#include "zerowm.h"
#include "zeropanel.h"
#include "../apps/installer/installer.h"

/* Desktop Icon Structure */
typedef struct {
    const char *title;
    icon_type_t icon;
    int x;
    int y;
    void (*launch)(void);
} desktop_icon_t;

extern void app_launch_installer(void);
extern void app_launch_terminal(void);
extern void app_launch_file_manager(void);
extern void app_launch_control_panel(void);
extern void app_launch_editor(void);
extern void app_launch_fetch(void);

static desktop_icon_t desktop_icons[] = {
    { "Установка ОС",   ICON_INSTALLER,     24,  24, app_launch_installer },
    { "Терминал",       ICON_TERMINAL,      24, 110, app_launch_terminal },
    { "Файлы",          ICON_FILE_MANAGER,  24, 196, app_launch_file_manager },
    { "Экран и опции",  ICON_CONTROL_PANEL, 24, 282, app_launch_control_panel },
    { "Редактор",       ICON_EDITOR,        24, 368, app_launch_editor },
    { "О системе",      ICON_SYSTEM_INFO,   24, 454, app_launch_fetch },
};
#define DESKTOP_ICON_COUNT (sizeof(desktop_icons) / sizeof(desktop_icons[0]))

static volatile bool g_running = true;

static void sig_exit(int sig) {
    (void)sig;
    g_running = false;
}

static void draw_desktop_icons(mouse_state_t *mouse) {
    for (size_t i = 0; i < DESKTOP_ICON_COUNT; i++) {
        desktop_icon_t *ico = &desktop_icons[i];
        int ix = ico->x;
        int iy = ico->y;
        int isz = 48;

        bool hovered = (mouse->x >= ix && mouse->x < (ix + isz + 20) &&
                        mouse->y >= iy && mouse->y < (iy + isz + 24));

        if (hovered) {
            canvas_fill_rounded_rect(ix - 4, iy - 4, isz + 28, isz + 32, 6, COLOR_RGBA(255, 255, 255, 25));
            canvas_draw_rounded_rect(ix - 4, iy - 4, isz + 28, isz + 32, 6, g_theme.accent_primary);
        }

        /* Draw Icon */
        icons_draw(ico->icon, ix + 10, iy, 40, 0);

        /* Draw Icon Label */
        int tw = font_get_string_width(ico->title);
        int tx = ix + 10 + (40 - tw) / 2;
        font_draw_string_utf8(tx, iy + 46, ico->title, g_theme.text_primary, COLOR_RGBA(0, 0, 0, 0));
    }
}

static void handle_desktop_icon_clicks(mouse_state_t *mouse) {
    if (!mouse->left_clicked) return;

    /* If clicked on desktop background */
    for (size_t i = 0; i < DESKTOP_ICON_COUNT; i++) {
        desktop_icon_t *ico = &desktop_icons[i];
        int ix = ico->x;
        int iy = ico->y;
        int isz = 48;

        if (mouse->x >= ix && mouse->x < (ix + isz + 20) &&
            mouse->y >= iy && mouse->y < (iy + isz + 24)) {
            if (ico->launch) {
                sound_play(SND_CLICK);
                ico->launch();
                sound_play(SND_WINDOW_OPEN);
            }
            return;
        }
    }
}

static void draw_mouse_cursor(int x, int y) {
    /* Draw smooth modern cursor with shadow */
    static const uint8_t cursor_shape[16][16] = {
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0},
        {1,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0},
        {1,2,2,1,2,1,0,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,1,2,1,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,1,2,1,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
    };

    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 16; c++) {
            uint8_t pixel = cursor_shape[r][c];
            if (pixel == 1) {
                fbdev_set_pixel(x + c, y + r, COLOR_RGB(0, 0, 0)); /* Black border */
            } else if (pixel == 2) {
                fbdev_set_pixel(x + c, y + r, COLOR_RGB(255, 255, 255)); /* White fill */
            }
        }
    }
}

int main(int argc, char **argv) {
    bool auto_installer = false;
    bool single_frame = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--installer") == 0) {
            auto_installer = true;
        } else if (strcmp(argv[i], "--single-frame") == 0 || strcmp(argv[i], "-1") == 0) {
            single_frame = true;
        }
    }

    signal(SIGINT, sig_exit);
    signal(SIGTERM, sig_exit);

    /* Initialize Subsystems */
    printf("[+] Инициализация графики и рабочего стола LinuxOSZero v1.1.0 (x86_64)...\n");
    if (fbdev_init("/dev/fb0", 1024, 768) < 0) {
        fprintf(stderr, "Не удалось инициализировать fbdev.\n");
        return 1;
    }

    input_init();
    input_set_mouse_bounds(g_fbdev.width, g_fbdev.height);
    theme_init_dark();
    wm_init(g_fbdev.width, g_fbdev.height);
    panel_init(g_fbdev.width, g_fbdev.height);

    /* Initialize sound driver and play a startup jingle */
    sound_init();
    sound_play(SND_STARTUP);

    if (auto_installer) {
        app_launch_installer();
    } else {
        /* Открыть терминал по умолчанию */
        app_launch_terminal();
        sound_play(SND_WINDOW_OPEN);
    }

    printf("[+] ZeroDesktop v1.1.0 работает: %dx%d (32 bpp)\n", g_fbdev.width, g_fbdev.height);

    /* Main Render & Event Loop */
    while (g_running) {
        /* Poll Mouse and Keyboard Devices */
        input_poll();

        /* Process all queued keyboard events */
        key_event_t key_ev;
        while (input_get_key(&key_ev)) {
            wm_handle_keyboard(&key_ev);
        }

        /* Process Mouse Inputs */
        wm_handle_input(&g_mouse);
        panel_handle_input(&g_mouse);
        handle_desktop_icon_clicks(&g_mouse);

        /* 1. Clear & Render Wallpaper */
        wallpaper_render(g_fbdev.width, g_fbdev.height);

        /* 2. Desktop Icons */
        draw_desktop_icons(&g_mouse);

        /* 3. Window Manager & Windows */
        wm_render_all();

        /* 4. Taskbar Panel & Start Menu */
        panel_render();

        /* 5. Mouse Pointer */
        draw_mouse_cursor(g_mouse.x, g_mouse.y);

        /* 6. Swap back buffer to display */
        fbdev_swap_buffers();

        if (single_frame) {
            break;
        }

        usleep(16000); /* ~60 FPS */
    }

    input_close();
    fbdev_close();
    printf("[+] ZeroDesktop завершил работу корректно.\n");
    return 0;
}

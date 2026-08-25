/*
 * LinuxOSZero - Window Manager Implementation (ZeroWM)
 * Architecture: x86_64
 */

#include "zerowm.h"
#include "zeropanel.h"
#include "../drivers/sound.h"
#include <string.h>
#include <stdio.h>

wm_t g_wm = {0};

/* External app launchers for global shortcuts */
extern void app_launch_terminal(void);
extern void app_launch_editor(void);
extern void app_launch_file_manager(void);
extern void app_launch_control_panel(void);
extern void app_launch_installer(void);
extern void app_launch_fetch(void);

void wm_init(int screen_w, int screen_h) {
    g_wm.screen_w = screen_w;
    g_wm.screen_h = screen_h;
    g_wm.window_count = 0;
    g_wm.active_win_id = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        g_wm.windows[i].id = -1;
        g_wm.z_order[i] = -1;
    }
}

window_t *wm_create_window(const char *title, icon_type_t icon, int x, int y, int w, int h, win_render_fn render, win_event_fn event) {
    int slot = -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_wm.windows[i].id == -1) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return NULL;

    window_t *win = &g_wm.windows[slot];
    win->id = slot;
    strncpy(win->title, title ? title : "Window", sizeof(win->title) - 1);
    win->icon = icon;
    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->prev_x = x;
    win->prev_y = y;
    win->prev_w = w;
    win->prev_h = h;
    win->is_active = true;
    win->is_visible = true;
    win->is_resizable = true;
    win->is_dragging = false;
    win->is_resizing = false;
    win->drag_off_x = 0;
    win->drag_off_y = 0;
    win->state = WIN_STATE_NORMAL;
    win->render_content = render;
    win->on_event = event;
    win->user_data = NULL;

    /* Add to top of Z-order */
    g_wm.z_order[g_wm.window_count] = slot;
    g_wm.window_count++;

    wm_focus_window(slot);
    return win;
}

void wm_destroy_window(int win_id) {
    if (win_id < 0 || win_id >= MAX_WINDOWS) return;
    g_wm.windows[win_id].id = -1;
    g_wm.windows[win_id].is_visible = false;

    /* Remove from Z-order */
    int idx = -1;
    for (int i = 0; i < g_wm.window_count; i++) {
        if (g_wm.z_order[i] == win_id) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = idx; i < g_wm.window_count - 1; i++) {
            g_wm.z_order[i] = g_wm.z_order[i + 1];
        }
        g_wm.window_count--;
    }

    if (g_wm.active_win_id == win_id) {
        g_wm.active_win_id = g_wm.window_count > 0 ? g_wm.z_order[g_wm.window_count - 1] : -1;
        if (g_wm.active_win_id >= 0) {
            g_wm.windows[g_wm.active_win_id].is_active = true;
        }
    }
    sound_play(SND_WINDOW_CLOSE);
}

void wm_focus_window(int win_id) {
    if (win_id < 0 || win_id >= MAX_WINDOWS || g_wm.windows[win_id].id == -1) return;

    for (int i = 0; i < MAX_WINDOWS; i++) {
        g_wm.windows[i].is_active = (i == win_id);
    }
    g_wm.active_win_id = win_id;

    /* Move to top of Z-order */
    int idx = -1;
    for (int i = 0; i < g_wm.window_count; i++) {
        if (g_wm.z_order[i] == win_id) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = idx; i < g_wm.window_count - 1; i++) {
            g_wm.z_order[i] = g_wm.z_order[i + 1];
        }
        g_wm.z_order[g_wm.window_count - 1] = win_id;
    }
}

void wm_minimize_window(int win_id) {
    if (win_id < 0 || win_id >= MAX_WINDOWS) return;
    g_wm.windows[win_id].is_visible = false;
    g_wm.windows[win_id].state = WIN_STATE_MINIMIZED;
    g_wm.windows[win_id].is_active = false;
    if (g_wm.active_win_id == win_id) {
        g_wm.active_win_id = -1;
        for (int i = g_wm.window_count - 1; i >= 0; i--) {
            int wid = g_wm.z_order[i];
            if (g_wm.windows[wid].is_visible) {
                wm_focus_window(wid);
                break;
            }
        }
    }
}

void wm_maximize_window(int win_id) {
    if (win_id < 0 || win_id >= MAX_WINDOWS) return;
    window_t *win = &g_wm.windows[win_id];
    if (win->state == WIN_STATE_MAXIMIZED) {
        win->x = win->prev_x;
        win->y = win->prev_y;
        win->width = win->prev_w;
        win->height = win->prev_h;
        win->state = WIN_STATE_NORMAL;
    } else {
        win->prev_x = win->x;
        win->prev_y = win->y;
        win->prev_w = win->width;
        win->prev_h = win->height;
        win->x = 0;
        win->y = 0;
        win->width = g_wm.screen_w;
        win->height = g_wm.screen_h - 40;
        win->state = WIN_STATE_MAXIMIZED;
    }
    wm_focus_window(win_id);
}

void wm_restore_window(int win_id) {
    if (win_id < 0 || win_id >= MAX_WINDOWS) return;
    g_wm.windows[win_id].is_visible = true;
    g_wm.windows[win_id].state = WIN_STATE_NORMAL;
    wm_focus_window(win_id);
}

window_t *wm_get_window(int win_id) {
    if (win_id >= 0 && win_id < MAX_WINDOWS && g_wm.windows[win_id].id != -1) {
        return &g_wm.windows[win_id];
    }
    return NULL;
}

void wm_handle_input(mouse_state_t *mouse) {
    /* Handle drag/resize active window */
    if (g_wm.active_win_id >= 0) {
        window_t *win = &g_wm.windows[g_wm.active_win_id];
        if (win->is_dragging) {
            if (mouse->buttons & MOUSE_BTN_LEFT) {
                win->x = mouse->x - win->drag_off_x;
                win->y = mouse->y - win->drag_off_y;
                if (win->y < 0) win->y = 0;
                if (win->y > g_wm.screen_h - 60) win->y = g_wm.screen_h - 60;
                return;
            } else {
                win->is_dragging = false;
            }
        }
        if (win->is_resizing) {
            if (mouse->buttons & MOUSE_BTN_LEFT) {
                win->width = mouse->x - win->x;
                win->height = mouse->y - win->y;
                if (win->width < 200) win->width = 200;
                if (win->height < 120) win->height = 120;
                return;
            } else {
                win->is_resizing = false;
            }
        }
    }

    /* On click: hit test windows from top to bottom */
    if (mouse->left_clicked) {
        for (int i = g_wm.window_count - 1; i >= 0; i--) {
            int wid = g_wm.z_order[i];
            window_t *win = &g_wm.windows[wid];
            if (!win->is_visible) continue;

            if (mouse->x >= win->x && mouse->x < (win->x + win->width) &&
                mouse->y >= win->y && mouse->y < (win->y + win->height)) {
                
                wm_focus_window(wid);

                /* Titlebar Hit Test */
                if (mouse->y < (win->y + TITLEBAR_HEIGHT)) {
                    int btn_w = 16;
                    int btn_y = win->y + (TITLEBAR_HEIGHT - btn_w) / 2;
                    int close_x = win->x + win->width - 24;
                    int max_x = close_x - 22;
                    int min_x = max_x - 22;

                    if (mouse->x >= close_x && mouse->x < (close_x + btn_w) &&
                        mouse->y >= btn_y && mouse->y < (btn_y + btn_w)) {
                        wm_destroy_window(wid);
                        return;
                    }
                    if (mouse->x >= max_x && mouse->x < (max_x + btn_w) &&
                        mouse->y >= btn_y && mouse->y < (btn_y + btn_w)) {
                        wm_maximize_window(wid);
                        return;
                    }
                    if (mouse->x >= min_x && mouse->x < (min_x + btn_w) &&
                        mouse->y >= btn_y && mouse->y < (btn_y + btn_w)) {
                        wm_minimize_window(wid);
                        return;
                    }

                    /* Start dragging */
                    if (win->state != WIN_STATE_MAXIMIZED) {
                        win->is_dragging = true;
                        win->drag_off_x = mouse->x - win->x;
                        win->drag_off_y = mouse->y - win->y;
                    }
                    return;
                }

                /* Bottom-Right corner resize hit test */
                if (win->is_resizable && win->state != WIN_STATE_MAXIMIZED) {
                    if (mouse->x >= (win->x + win->width - 16) && mouse->y >= (win->y + win->height - 16)) {
                        win->is_resizing = true;
                        return;
                    }
                }

                /* Content area event forwarding */
                if (win->on_event) {
                    int cx = mouse->x - win->x;
                    int cy = mouse->y - (win->y + TITLEBAR_HEIGHT);
                    win->on_event(win, WM_EVENT_CLICK, cx, cy);
                }
                return;
            }
        }
    }
}

void wm_handle_keyboard(const key_event_t *ev) {
    if (!ev) return;

    /* Global Shortcuts (checked when pressed) */
    if (ev->pressed) {
        /* Super key / Windows key: Toggle Start Menu */
        if (ev->key_code == KEY_SUPER || (ev->ctrl && ev->key_code == KEY_ESC)) {
            g_panel.start_menu_open = !g_panel.start_menu_open;
            return;
        }

        /* Alt + F4: Close active window */
        if (ev->alt && ev->key_code == KEY_F4 && g_wm.active_win_id >= 0) {
            wm_destroy_window(g_wm.active_win_id);
            return;
        }

        /* Alt + Tab: Cycle through open windows */
        if (ev->alt && ev->key_code == KEY_TAB && g_wm.window_count > 1) {
            int current_idx = -1;
            for (int i = 0; i < g_wm.window_count; i++) {
                if (g_wm.z_order[i] == g_wm.active_win_id) {
                    current_idx = i;
                    break;
                }
            }
            int next_idx = (current_idx > 0) ? (current_idx - 1) : (g_wm.window_count - 1);
            int next_wid = g_wm.z_order[next_idx];
            if (next_wid >= 0 && g_wm.windows[next_wid].id != -1) {
                if (!g_wm.windows[next_wid].is_visible) {
                    wm_restore_window(next_wid);
                } else {
                    wm_focus_window(next_wid);
                }
            }
            return;
        }

        /* Ctrl + Alt + T: Open Terminal */
        if (ev->ctrl && ev->alt && (ev->key_code == 't' || ev->key_code == 'T')) {
            app_launch_terminal();
            return;
        }

        /* Ctrl + Alt + E: Open Editor */
        if (ev->ctrl && ev->alt && (ev->key_code == 'e' || ev->key_code == 'E')) {
            app_launch_editor();
            return;
        }

        /* Ctrl + Alt + F: Open File Manager */
        if (ev->ctrl && ev->alt && (ev->key_code == 'f' || ev->key_code == 'F')) {
            app_launch_file_manager();
            return;
        }

        /* Ctrl + Alt + C: Open Control Panel */
        if (ev->ctrl && ev->alt && (ev->key_code == 'c' || ev->key_code == 'C')) {
            app_launch_control_panel();
            return;
        }

        /* Ctrl + Alt + I: Open Installer */
        if (ev->ctrl && ev->alt && (ev->key_code == 'i' || ev->key_code == 'I')) {
            app_launch_installer();
            return;
        }
    }

    /* Forward keyboard event to focused active window */
    if (g_wm.active_win_id >= 0) {
        window_t *win = &g_wm.windows[g_wm.active_win_id];
        if (win->is_visible && win->on_event) {
            int ev_type = ev->pressed ? WM_EVENT_KEY_DOWN : WM_EVENT_KEY_UP;
            win->on_event(win, ev_type, ev->key_code, (int)ev->ascii);
        }
    }
}

void wm_render_all(void) {
    for (int i = 0; i < g_wm.window_count; i++) {
        int wid = g_wm.z_order[i];
        window_t *win = &g_wm.windows[wid];
        if (!win->is_visible) continue;

        /* Window Drop Shadow */
        canvas_draw_shadow(win->x, win->y, win->width, win->height, 8, g_theme.win_shadow);

        /* Window Background */
        canvas_fill_rounded_rect(win->x, win->y, win->width, win->height, 8, g_theme.win_bg);
        canvas_draw_rounded_rect(win->x, win->y, win->width, win->height, 8, win->is_active ? g_theme.accent_primary : g_theme.win_border);

        /* Titlebar */
        color_t tb_bg = win->is_active ? g_theme.win_titlebar_active : g_theme.win_titlebar_inactive;
        canvas_fill_rounded_rect(win->x + 1, win->y + 1, win->width - 2, TITLEBAR_HEIGHT, 7, tb_bg);
        fbdev_fill_rect(win->x + 1, win->y + TITLEBAR_HEIGHT - 1, win->width - 2, 1, g_theme.win_border);

        /* Window Icon */
        icons_draw(win->icon, win->x + 8, win->y + (TITLEBAR_HEIGHT - 16) / 2, 16, 0);

        /* Window Title */
        color_t title_c = win->is_active ? g_theme.win_titlebar_text_active : g_theme.win_titlebar_text_inactive;
        font_draw_string_utf8(win->x + 30, win->y + 6, win->title, title_c, COLOR_RGBA(0, 0, 0, 0));

        /* Window Controls: Close (Red), Maximize (Green), Minimize (Yellow) */
        int btn_w = 12;
        int btn_y = win->y + (TITLEBAR_HEIGHT - btn_w) / 2;
        int close_x = win->x + win->width - 20;
        int max_x = close_x - 18;
        int min_x = max_x - 18;

        fbdev_fill_circle(close_x + btn_w / 2, btn_y + btn_w / 2, btn_w / 2, g_theme.btn_close);
        fbdev_fill_circle(max_x + btn_w / 2, btn_y + btn_w / 2, btn_w / 2, g_theme.btn_max);
        fbdev_fill_circle(min_x + btn_w / 2, btn_y + btn_w / 2, btn_w / 2, g_theme.btn_min);

        /* Content Area */
        int cx = win->x + 2;
        int cy = win->y + TITLEBAR_HEIGHT;
        int cw = win->width - 4;
        int ch = win->height - TITLEBAR_HEIGHT - 2;

        if (win->render_content) {
            win->render_content(win, cx, cy, cw, ch);
        }

        /* Resize grip handle in bottom right corner */
        if (win->is_resizable && win->state != WIN_STATE_MAXIMIZED) {
            color_t grip_c = g_theme.text_muted;
            fbdev_draw_line(win->x + win->width - 6, win->y + win->height - 2, win->x + win->width - 2, win->y + win->height - 6, grip_c);
            fbdev_draw_line(win->x + win->width - 10, win->y + win->height - 2, win->x + win->width - 2, win->y + win->height - 10, grip_c);
        }
    }
}

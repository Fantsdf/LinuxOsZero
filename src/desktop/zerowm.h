/*
 * LinuxOSZero - Window Manager (ZeroWM)
 */

#ifndef ZEROWM_H
#define ZEROWM_H

#include "../drivers/fbdev.h"
#include "../drivers/input.h"
#include "../gui/theme.h"
#include "../gui/canvas.h"
#include "../gui/icons.h"
#include <stdbool.h>

#define MAX_WINDOWS         16
#define TITLEBAR_HEIGHT     28
#define WIN_RESIZE_BORDER   6

typedef enum {
    WIN_STATE_NORMAL,
    WIN_STATE_MINIMIZED,
    WIN_STATE_MAXIMIZED
} win_state_t;

struct window;

typedef void (*win_render_fn)(struct window *win, int cx, int cy, int cw, int ch);
typedef void (*win_event_fn)(struct window *win, int ev_type, int p1, int p2);

typedef struct window {
    int id;
    char title[64];
    icon_type_t icon;
    int x;
    int y;
    int width;
    int height;
    int prev_x;
    int prev_y;
    int prev_w;
    int prev_h;
    bool is_active;
    bool is_visible;
    bool is_resizable;
    bool is_dragging;
    bool is_resizing;
    int drag_off_x;
    int drag_off_y;
    win_state_t state;
    win_render_fn render_content;
    win_event_fn on_event;
    void *user_data;
} window_t;

typedef struct {
    window_t windows[MAX_WINDOWS];
    int window_count;
    int z_order[MAX_WINDOWS];
    int active_win_id;
    int screen_w;
    int screen_h;
} wm_t;

extern wm_t g_wm;

void wm_init(int screen_w, int screen_h);
window_t *wm_create_window(const char *title, icon_type_t icon, int x, int y, int w, int h, win_render_fn render, win_event_fn event);
void wm_destroy_window(int win_id);
void wm_focus_window(int win_id);
void wm_minimize_window(int win_id);
void wm_maximize_window(int win_id);
void wm_restore_window(int win_id);
void wm_handle_input(mouse_state_t *mouse);
void wm_render_all(void);
window_t *wm_get_window(int win_id);

#endif /* ZEROWM_H */

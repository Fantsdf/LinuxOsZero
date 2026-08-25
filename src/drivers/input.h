/*
 * LinuxOSZero - Input Subsystem (Keyboard & Mouse Handling)
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

#define KEY_ESC         27
#define KEY_ENTER       13
#define KEY_BACKSPACE   8
#define KEY_TAB         9
#define KEY_UP          1001
#define KEY_DOWN        1002
#define KEY_LEFT        1003
#define KEY_RIGHT       1004
#define KEY_F1          1011
#define KEY_F2          1012
#define KEY_F3          1013
#define KEY_F4          1014
#define KEY_F5          1015
#define KEY_F6          1016
#define KEY_F7          1017
#define KEY_F8          1018
#define KEY_F9          1019
#define KEY_F10         1020

#define MOUSE_BTN_LEFT    (1 << 0)
#define MOUSE_BTN_RIGHT   (1 << 1)
#define MOUSE_BTN_MIDDLE  (1 << 2)

typedef struct {
    int x;
    int y;
    int prev_x;
    int prev_y;
    int dx;
    int dy;
    uint32_t buttons;
    uint32_t prev_buttons;
    bool moved;
    bool left_clicked;
    bool right_clicked;
    bool left_released;
    bool dragging;
} mouse_state_t;

typedef struct {
    int key_code;
    char ascii;
    bool pressed;
    bool ctrl;
    bool alt;
    bool shift;
} key_event_t;

extern mouse_state_t g_mouse;

int input_init(void);
void input_poll(void);
void input_set_mouse_bounds(int max_w, int max_h);
void input_inject_mouse(int x, int y, uint32_t buttons);
void input_inject_key(int key_code, char ascii, bool pressed);
bool input_get_key(key_event_t *ev);

#endif /* INPUT_H */

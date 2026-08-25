/*
 * LinuxOSZero - Input Driver Implementation
 */

#include "input.h"
#include "vboxguest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <termios.h>

mouse_state_t g_mouse = {
    .x = 400,
    .y = 300,
    .prev_x = 400,
    .prev_y = 300,
    .dx = 0,
    .dy = 0,
    .buttons = 0,
    .prev_buttons = 0,
    .moved = false,
    .left_clicked = false,
    .right_clicked = false,
    .left_released = false,
    .dragging = false
};

static int mouse_fd = -1;
static int max_width = 1024;
static int max_height = 768;

#define KEY_QUEUE_SIZE 64
static key_event_t key_queue[KEY_QUEUE_SIZE];
static int key_q_head = 0;
static int key_q_tail = 0;

int input_init(void) {
    /* Try opening standard Linux mice node */
    mouse_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) {
        mouse_fd = open("/dev/psaux", O_RDONLY | O_NONBLOCK);
    }
    return 0;
}

void input_set_mouse_bounds(int max_w, int max_h) {
    max_width = max_w > 0 ? max_w : 1024;
    max_height = max_h > 0 ? max_h : 768;
}

void input_inject_mouse(int x, int y, uint32_t buttons) {
    g_mouse.prev_x = g_mouse.x;
    g_mouse.prev_y = g_mouse.y;
    g_mouse.prev_buttons = g_mouse.buttons;

    g_mouse.x = x < 0 ? 0 : (x >= max_width ? max_width - 1 : x);
    g_mouse.y = y < 0 ? 0 : (y >= max_height ? max_height - 1 : y);
    g_mouse.dx = g_mouse.x - g_mouse.prev_x;
    g_mouse.dy = g_mouse.y - g_mouse.prev_y;
    g_mouse.buttons = buttons;

    g_mouse.moved = (g_mouse.dx != 0 || g_mouse.dy != 0);
    g_mouse.left_clicked = !(g_mouse.prev_buttons & MOUSE_BTN_LEFT) && (g_mouse.buttons & MOUSE_BTN_LEFT);
    g_mouse.right_clicked = !(g_mouse.prev_buttons & MOUSE_BTN_RIGHT) && (g_mouse.buttons & MOUSE_BTN_RIGHT);
    g_mouse.left_released = (g_mouse.prev_buttons & MOUSE_BTN_LEFT) && !(g_mouse.buttons & MOUSE_BTN_LEFT);
    g_mouse.dragging = (g_mouse.buttons & MOUSE_BTN_LEFT) && g_mouse.moved;
}

void input_inject_key(int key_code, char ascii, bool pressed) {
    int next = (key_q_head + 1) % KEY_QUEUE_SIZE;
    if (next != key_q_tail) {
        key_queue[key_q_head].key_code = key_code;
        key_queue[key_q_head].ascii = ascii;
        key_queue[key_q_head].pressed = pressed;
        key_queue[key_q_head].ctrl = false;
        key_queue[key_q_head].alt = false;
        key_queue[key_q_head].shift = false;
        key_q_head = next;
    }
}

bool input_get_key(key_event_t *ev) {
    if (key_q_head == key_q_tail) return false;
    if (ev) {
        *ev = key_queue[key_q_tail];
    }
    key_q_tail = (key_q_tail + 1) % KEY_QUEUE_SIZE;
    return true;
}

void input_poll(void) {
    /* VirtualBox Integration Pointer */
    if (vboxguest_is_active()) {
        int vx, vy;
        uint32_t vb;
        if (vboxguest_get_mouse_position(&vx, &vy, &vb) == 0) {
            input_inject_mouse(vx, vy, vb);
            return;
        }
    }

    /* PS/2 / USB Relative Mouse Reading */
    if (mouse_fd >= 0) {
        signed char buf[3];
        int bytes = read(mouse_fd, buf, sizeof(buf));
        if (bytes == 3) {
            uint32_t btns = 0;
            if (buf[0] & 0x01) btns |= MOUSE_BTN_LEFT;
            if (buf[0] & 0x02) btns |= MOUSE_BTN_RIGHT;
            if (buf[0] & 0x04) btns |= MOUSE_BTN_MIDDLE;

            int new_x = g_mouse.x + buf[1];
            int new_y = g_mouse.y - buf[2]; /* Invert Y delta */
            input_inject_mouse(new_x, new_y, btns);
        }
    }
}

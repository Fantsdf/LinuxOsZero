/*
 * LinuxOSZero - Input Driver Implementation (Mouse & Keyboard)
 * Architecture: x86_64
 */

#include "input.h"
#include "vboxguest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <errno.h>

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
static int evdev_kbd_fds[8];
static int evdev_kbd_count = 0;
static int max_width = 1024;
static int max_height = 768;

static struct termios orig_termios;
static bool termios_saved = false;

#define KEY_QUEUE_SIZE 256
static key_event_t key_queue[KEY_QUEUE_SIZE];
static int key_q_head = 0;
static int key_q_tail = 0;

/* Modifier states */
static bool s_shift = false;
static bool s_ctrl = false;
static bool s_alt = false;
static bool s_caps = false;

/* Convert Linux input evdev code to ASCII / Keycode */
static int evdev_to_keycode(int code, bool shift, bool caps, char *out_ascii) {
    *out_ascii = 0;

    switch (code) {
        case KEY_ESC:       return KEY_ESC;
        case KEY_ENTER:     *out_ascii = '\n'; return KEY_ENTER;
        case KEY_BACKSPACE: *out_ascii = '\b'; return KEY_BACKSPACE;
        case KEY_TAB:       *out_ascii = '\t'; return KEY_TAB;
        case KEY_SPACE:     *out_ascii = ' ';  return KEY_SPACE;

        case KEY_UP:        return KEY_UP;
        case KEY_DOWN:      return KEY_DOWN;
        case KEY_LEFT:      return KEY_LEFT;
        case KEY_RIGHT:     return KEY_RIGHT;
        case KEY_PAGEUP:    return KEY_PAGE_UP;
        case KEY_PAGEDOWN:  return KEY_PAGE_DOWN;
        case KEY_HOME:      return KEY_HOME;
        case KEY_END:       return KEY_END;
        case KEY_INSERT:    return KEY_INSERT;
        case KEY_DELETE:    return KEY_DELETE;

        case KEY_F1:        return KEY_F1;
        case KEY_F2:        return KEY_F2;
        case KEY_F3:        return KEY_F3;
        case KEY_F4:        return KEY_F4;
        case KEY_F5:        return KEY_F5;
        case KEY_F6:        return KEY_F6;
        case KEY_F7:        return KEY_F7;
        case KEY_F8:        return KEY_F8;
        case KEY_F9:        return KEY_F9;
        case KEY_F10:       return KEY_F10;
        case KEY_F11:       return KEY_F11;
        case KEY_F12:       return KEY_F12;

        case KEY_LEFTMETA:
        case KEY_RIGHTMETA: return KEY_SUPER;
        case KEY_CAPSLOCK:  return KEY_CAPS_LOCK;

        /* Number Row */
        case KEY_1: *out_ascii = shift ? '!' : '1'; return *out_ascii;
        case KEY_2: *out_ascii = shift ? '@' : '2'; return *out_ascii;
        case KEY_3: *out_ascii = shift ? '#' : '3'; return *out_ascii;
        case KEY_4: *out_ascii = shift ? '$' : '4'; return *out_ascii;
        case KEY_5: *out_ascii = shift ? '%' : '5'; return *out_ascii;
        case KEY_6: *out_ascii = shift ? '^' : '6'; return *out_ascii;
        case KEY_7: *out_ascii = shift ? '&' : '7'; return *out_ascii;
        case KEY_8: *out_ascii = shift ? '*' : '8'; return *out_ascii;
        case KEY_9: *out_ascii = shift ? '(' : '9'; return *out_ascii;
        case KEY_0: *out_ascii = shift ? ')' : '0'; return *out_ascii;
        case KEY_MINUS: *out_ascii = shift ? '_' : '-'; return *out_ascii;
        case KEY_EQUAL: *out_ascii = shift ? '+' : '='; return *out_ascii;

        /* Letters */
        case KEY_Q: *out_ascii = (shift ^ caps) ? 'Q' : 'q'; return *out_ascii;
        case KEY_W: *out_ascii = (shift ^ caps) ? 'W' : 'w'; return *out_ascii;
        case KEY_E: *out_ascii = (shift ^ caps) ? 'E' : 'e'; return *out_ascii;
        case KEY_R: *out_ascii = (shift ^ caps) ? 'R' : 'r'; return *out_ascii;
        case KEY_T: *out_ascii = (shift ^ caps) ? 'T' : 't'; return *out_ascii;
        case KEY_Y: *out_ascii = (shift ^ caps) ? 'Y' : 'y'; return *out_ascii;
        case KEY_U: *out_ascii = (shift ^ caps) ? 'U' : 'u'; return *out_ascii;
        case KEY_I: *out_ascii = (shift ^ caps) ? 'I' : 'i'; return *out_ascii;
        case KEY_O: *out_ascii = (shift ^ caps) ? 'O' : 'o'; return *out_ascii;
        case KEY_P: *out_ascii = (shift ^ caps) ? 'P' : 'p'; return *out_ascii;
        case KEY_A: *out_ascii = (shift ^ caps) ? 'A' : 'a'; return *out_ascii;
        case KEY_S: *out_ascii = (shift ^ caps) ? 'S' : 's'; return *out_ascii;
        case KEY_D: *out_ascii = (shift ^ caps) ? 'D' : 'd'; return *out_ascii;
        case KEY_F: *out_ascii = (shift ^ caps) ? 'F' : 'f'; return *out_ascii;
        case KEY_G: *out_ascii = (shift ^ caps) ? 'G' : 'g'; return *out_ascii;
        case KEY_H: *out_ascii = (shift ^ caps) ? 'H' : 'h'; return *out_ascii;
        case KEY_J: *out_ascii = (shift ^ caps) ? 'J' : 'j'; return *out_ascii;
        case KEY_K: *out_ascii = (shift ^ caps) ? 'K' : 'k'; return *out_ascii;
        case KEY_L: *out_ascii = (shift ^ caps) ? 'L' : 'l'; return *out_ascii;
        case KEY_Z: *out_ascii = (shift ^ caps) ? 'Z' : 'z'; return *out_ascii;
        case KEY_X: *out_ascii = (shift ^ caps) ? 'X' : 'x'; return *out_ascii;
        case KEY_C: *out_ascii = (shift ^ caps) ? 'C' : 'c'; return *out_ascii;
        case KEY_V: *out_ascii = (shift ^ caps) ? 'V' : 'v'; return *out_ascii;
        case KEY_B: *out_ascii = (shift ^ caps) ? 'B' : 'b'; return *out_ascii;
        case KEY_N: *out_ascii = (shift ^ caps) ? 'N' : 'n'; return *out_ascii;
        case KEY_M: *out_ascii = (shift ^ caps) ? 'M' : 'm'; return *out_ascii;

        /* Punctuation */
        case KEY_LEFTBRACE:  *out_ascii = shift ? '{' : '['; return *out_ascii;
        case KEY_RIGHTBRACE: *out_ascii = shift ? '}' : ']'; return *out_ascii;
        case KEY_BACKSLASH:  *out_ascii = shift ? '|' : '\\'; return *out_ascii;
        case KEY_SEMICOLON:  *out_ascii = shift ? ':' : ';'; return *out_ascii;
        case KEY_APOSTROPHE: *out_ascii = shift ? '"' : '\''; return *out_ascii;
        case KEY_GRAVE:      *out_ascii = shift ? '~' : '`'; return *out_ascii;
        case KEY_COMMA:      *out_ascii = shift ? '<' : ','; return *out_ascii;
        case KEY_DOT:        *out_ascii = shift ? '>' : '.'; return *out_ascii;
        case KEY_SLASH:      *out_ascii = shift ? '?' : '/'; return *out_ascii;

        default: break;
    }
    return 0;
}

int input_init(void) {
    /* 1. Open standard Linux mouse nodes */
    mouse_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) {
        mouse_fd = open("/dev/psaux", O_RDONLY | O_NONBLOCK);
    }

    /* 2. Open Linux evdev keyboard devices (/dev/input/event*) */
    evdev_kbd_count = 0;
    for (int i = 0; i < 16 && evdev_kbd_count < 8; i++) {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0) {
            evdev_kbd_fds[evdev_kbd_count++] = fd;
        }
    }

    /* 3. Configure stdin in raw non-blocking mode if running in terminal */
    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &orig_termios) == 0) {
            termios_saved = true;
            struct termios raw = orig_termios;
            raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
            raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
            raw.c_cflag |= (CS8);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        }
        /* Make stdin non-blocking */
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        }
    }

    return 0;
}

void input_close(void) {
    if (mouse_fd >= 0) {
        close(mouse_fd);
        mouse_fd = -1;
    }
    for (int i = 0; i < evdev_kbd_count; i++) {
        if (evdev_kbd_fds[i] >= 0) {
            close(evdev_kbd_fds[i]);
            evdev_kbd_fds[i] = -1;
        }
    }
    evdev_kbd_count = 0;

    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        termios_saved = false;
    }
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
    input_inject_key_full(key_code, ascii, pressed, s_ctrl, s_alt, s_shift);
}

void input_inject_key_full(int key_code, char ascii, bool pressed, bool ctrl, bool alt, bool shift) {
    int next = (key_q_head + 1) % KEY_QUEUE_SIZE;
    if (next != key_q_tail) {
        key_queue[key_q_head].key_code = key_code;
        key_queue[key_q_head].ascii = ascii;
        key_queue[key_q_head].pressed = pressed;
        key_queue[key_q_head].ctrl = ctrl;
        key_queue[key_q_head].alt = alt;
        key_queue[key_q_head].shift = shift;
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

bool input_has_key(void) {
    return (key_q_head != key_q_tail);
}

static void poll_evdev_keyboards(void) {
    struct input_event evs[32];
    for (int k = 0; k < evdev_kbd_count; k++) {
        int fd = evdev_kbd_fds[k];
        if (fd < 0) continue;

        int bytes = read(fd, evs, sizeof(evs));
        if (bytes <= 0) continue;

        int count = bytes / sizeof(struct input_event);
        for (int i = 0; i < count; i++) {
            if (evs[i].type == EV_KEY) {
                int code = evs[i].code;
                int val = evs[i].value; /* 0 = release, 1 = press, 2 = repeat */

                /* Track modifier keys */
                if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
                    s_shift = (val != 0);
                } else if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) {
                    s_ctrl = (val != 0);
                } else if (code == KEY_LEFTALT || code == KEY_RIGHTALT) {
                    s_alt = (val != 0);
                } else if (code == KEY_CAPSLOCK && val == 1) {
                    s_caps = !s_caps;
                }

                char ascii = 0;
                int keycode = evdev_to_keycode(code, s_shift, s_caps, &ascii);
                if (keycode != 0) {
                    input_inject_key_full(keycode, ascii, val != 0, s_ctrl, s_alt, s_shift);
                }
            }
        }
    }
}

static void poll_stdin_keyboard(void) {
    unsigned char buf[64];
    int n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0) return;

    for (int i = 0; i < n; i++) {
        if (buf[i] == 27) { /* ESC or ANSI sequence */
            if (i + 2 < n && buf[i + 1] == '[') {
                char c = buf[i + 2];
                if (c == 'A')      { input_inject_key(KEY_UP, 0, true); i += 2; continue; }
                else if (c == 'B') { input_inject_key(KEY_DOWN, 0, true); i += 2; continue; }
                else if (c == 'C') { input_inject_key(KEY_RIGHT, 0, true); i += 2; continue; }
                else if (c == 'D') { input_inject_key(KEY_LEFT, 0, true); i += 2; continue; }
                else if (c == 'H') { input_inject_key(KEY_HOME, 0, true); i += 2; continue; }
                else if (c == 'F') { input_inject_key(KEY_END, 0, true); i += 2; continue; }
                else if (c >= '1' && c <= '6' && i + 3 < n && buf[i + 3] == '~') {
                    if (c == '3')      input_inject_key(KEY_DELETE, 0, true);
                    else if (c == '5') input_inject_key(KEY_PAGE_UP, 0, true);
                    else if (c == '6') input_inject_key(KEY_PAGE_DOWN, 0, true);
                    i += 3;
                    continue;
                }
            } else if (i + 2 < n && buf[i + 1] == 'O') {
                char c = buf[i + 2];
                if (c >= 'P' && c <= 'S') {
                    input_inject_key(KEY_F1 + (c - 'P'), 0, true);
                    i += 2;
                    continue;
                }
            }
            input_inject_key(KEY_ESC, 27, true);
        } else if (buf[i] == 127 || buf[i] == 8) {
            input_inject_key(KEY_BACKSPACE, '\b', true);
        } else if (buf[i] == '\r' || buf[i] == '\n') {
            input_inject_key(KEY_ENTER, '\n', true);
        } else if (buf[i] == '\t') {
            input_inject_key(KEY_TAB, '\t', true);
        } else if (buf[i] >= 32) {
            input_inject_key((int)buf[i], (char)buf[i], true);
        }
    }
}

void input_poll(void) {
    /* 1. VirtualBox Integration Pointer */
    if (vboxguest_is_active()) {
        int vx, vy;
        uint32_t vb;
        if (vboxguest_get_mouse_position(&vx, &vy, &vb) == 0) {
            input_inject_mouse(vx, vy, vb);
        }
    }

    /* 2. PS/2 / USB Relative Mouse Reading */
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

    /* 3. Evdev Keyboard polling */
    poll_evdev_keyboards();

    /* 4. TTY / Stdin Keyboard polling */
    poll_stdin_keyboard();
}

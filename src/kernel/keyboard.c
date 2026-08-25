/*
 * LinuxOSZero - PS/2 Keyboard Driver Implementation
 * Architecture: x86_64
 */

#include "keyboard.h"
#include "kernel.h"
#include <stdbool.h>

#define KBD_BUFFER_SIZE 256

static volatile uint8_t kbd_ring[KBD_BUFFER_SIZE];
static volatile int kbd_head = 0;
static volatile int kbd_tail = 0;

static volatile key_event_t kbd_event_ring[KBD_BUFFER_SIZE];
static volatile int kbd_ev_head = 0;
static volatile int kbd_ev_tail = 0;

static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool caps_lock = false;
static bool num_lock = true;
static bool scroll_lock = false;
static bool extended_mode = false;
static int current_layout = KBD_LAYOUT_US;

/* US QWERTY Scan Code Set 1 Table (Unshifted) */
static const char kbd_us_normal[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0, /* Alt */
    ' ', /* Space */
    0, /* Caps Lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1 - F10 */
    0, /* Num Lock */
    0, /* Scroll Lock */
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3',
    '0', '.',
    0, 0, 0,
    0, 0 /* F11, F12 */
};

/* US QWERTY Scan Code Set 1 Table (Shifted) */
static const char kbd_us_shifted[128] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, /* Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, /* Right Shift */
    '*',
    0, /* Alt */
    ' ', /* Space */
    0, /* Caps Lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1 - F10 */
    0, /* Num Lock */
    0, /* Scroll Lock */
    '7', '8', '9', '-',
    '4', '5', '6', '+',
    '1', '2', '3',
    '0', '.',
    0, 0, 0,
    0, 0 /* F11, F12 */
};

static void kbd_wait_input(void) {
    int timeout = 100000;
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL) && --timeout) {
        io_wait();
    }
}

static void kbd_update_leds(void) {
    uint8_t leds = 0;
    if (scroll_lock) leds |= KBD_LED_SCROLL_LOCK;
    if (num_lock)    leds |= KBD_LED_NUM_LOCK;
    if (caps_lock)   leds |= KBD_LED_CAPS_LOCK;

    kbd_wait_input();
    outb(PS2_DATA_PORT, 0xED);
    io_wait();
    kbd_wait_input();
    outb(PS2_DATA_PORT, leds);
}

void keyboard_init(void) {
    /* Flush existing buffer */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL) {
        inb(PS2_DATA_PORT);
        io_wait();
    }

    /* Enable scanning on keyboard controller */
    kbd_wait_input();
    outb(PS2_COMMAND_PORT, 0xAE); /* Enable first PS/2 port */

    kbd_wait_input();
    outb(PS2_DATA_PORT, 0xF4);    /* Enable keyboard scanning */

    kbd_head = 0;
    kbd_tail = 0;
    kbd_ev_head = 0;
    kbd_ev_tail = 0;
    shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    caps_lock = false;
    num_lock = true;
    scroll_lock = false;
    extended_mode = false;
    current_layout = KBD_LAYOUT_US;

    kbd_update_leds();
}

static void queue_char(uint8_t ch) {
    int next = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_tail) {
        kbd_ring[kbd_head] = ch;
        kbd_head = next;
    }
}

static void queue_event(int key_code, char ascii, bool pressed) {
    int next = (kbd_ev_head + 1) % KBD_BUFFER_SIZE;
    if (next != kbd_ev_tail) {
        kbd_event_ring[kbd_ev_head].key_code = key_code;
        kbd_event_ring[kbd_ev_head].ascii = ascii;
        kbd_event_ring[kbd_ev_head].pressed = pressed;
        kbd_event_ring[kbd_ev_head].ctrl = ctrl_pressed;
        kbd_event_ring[kbd_ev_head].alt = alt_pressed;
        kbd_event_ring[kbd_ev_head].shift = shift_pressed;
        kbd_ev_head = next;
    }
}

void keyboard_handle_scancode(uint8_t scancode) {
    /* Handle Extended Scancode prefix 0xE0 */
    if (scancode == 0xE0) {
        extended_mode = true;
        return;
    }

    bool released = (scancode & 0x80) != 0;
    uint8_t code = scancode & 0x7F;

    if (extended_mode) {
        extended_mode = false;
        int special_key = 0;
        switch (code) {
            case 0x48: special_key = KEY_UP; break;
            case 0x50: special_key = KEY_DOWN; break;
            case 0x4B: special_key = KEY_LEFT; break;
            case 0x4D: special_key = KEY_RIGHT; break;
            case 0x1D: ctrl_pressed = !released; return; /* Right Ctrl */
            case 0x38: alt_pressed = !released; return;  /* Right Alt */
            default: break;
        }
        if (special_key) {
            queue_event(special_key, 0, !released);
            if (!released) {
                queue_char((uint8_t)(special_key & 0xFF));
            }
        }
        return;
    }

    /* Modifiers */
    if (code == 0x2A || code == 0x36) { /* Left / Right Shift */
        shift_pressed = !released;
        return;
    }
    if (code == 0x1D) { /* Left Ctrl */
        ctrl_pressed = !released;
        return;
    }
    if (code == 0x38) { /* Left Alt */
        alt_pressed = !released;
        return;
    }
    if (code == 0x3A && !released) { /* Caps Lock */
        caps_lock = !caps_lock;
        kbd_update_leds();
        return;
    }
    if (code == 0x45 && !released) { /* Num Lock */
        num_lock = !num_lock;
        kbd_update_leds();
        return;
    }
    if (code == 0x46 && !released) { /* Scroll Lock */
        scroll_lock = !scroll_lock;
        kbd_update_leds();
        return;
    }

    /* Layout Switch on Alt + Shift */
    if (alt_pressed && (code == 0x2A || code == 0x36) && !released) {
        current_layout = (current_layout == KBD_LAYOUT_US) ? KBD_LAYOUT_RU : KBD_LAYOUT_US;
        return;
    }

    /* Function Keys */
    if (code >= 0x3B && code <= 0x44 && !released) {
        int fkey = KEY_F1 + (code - 0x3B);
        queue_event(fkey, 0, true);
        return;
    }

    if (code < 128) {
        bool use_upper = (shift_pressed ^ caps_lock);
        char ch = use_upper ? kbd_us_shifted[code] : kbd_us_normal[code];

        /* Map common control keys */
        int key_code = ch;
        if (code == 0x01) key_code = KEY_ESC;
        else if (code == 0x1C) key_code = KEY_ENTER;
        else if (code == 0x0E) key_code = KEY_BACKSPACE;
        else if (code == 0x0F) key_code = KEY_TAB;

        queue_event(key_code, ch, !released);

        if (!released && ch != 0) {
            queue_char((uint8_t)ch);
        }
    }
}

void keyboard_isr(void) {
    if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL) {
        uint8_t scancode = inb(PS2_DATA_PORT);
        keyboard_handle_scancode(scancode);
    }
    /* Send EOI to PIC */
    outb(0x20, 0x20);
}

void keyboard_poll(void) {
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL) {
        uint8_t scancode = inb(PS2_DATA_PORT);
        keyboard_handle_scancode(scancode);
    }
}

bool keyboard_has_char(void) {
    keyboard_poll();
    return (kbd_head != kbd_tail);
}

int keyboard_getchar(void) {
    while (!keyboard_has_char()) {
        hlt();
    }
    uint8_t ch = kbd_ring[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return (int)ch;
}

bool keyboard_get_event(key_event_t *ev) {
    keyboard_poll();
    if (kbd_ev_head == kbd_ev_tail) return false;
    if (ev) {
        *ev = kbd_event_ring[kbd_ev_tail];
    }
    kbd_ev_tail = (kbd_ev_tail + 1) % KBD_BUFFER_SIZE;
    return true;
}

void keyboard_set_layout(int layout) {
    if (layout == KBD_LAYOUT_US || layout == KBD_LAYOUT_RU) {
        current_layout = layout;
    }
}

int keyboard_get_layout(void) {
    return current_layout;
}

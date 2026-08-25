/*
 * LinuxOSZero - PS/2 Keyboard Driver Implementation
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 *
 * Implements hardware PS/2 keyboard controller communication, scancode
 * parsing for Scan Code Set 1 and Set 2, multi-byte prefix handling (0xE0, 0xF0),
 * modifier tracking, LED synchronization, and multi-layout (US / RU) mapping.
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
static bool super_pressed = false;
static bool caps_lock = false;
static bool num_lock = true;
static bool scroll_lock = false;
static bool extended_mode = false;
static bool set2_break_mode = false;
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

/* Russian JCUKEN Scancode Set 1 Table (Unshifted CP866 / Extended ASCII approximation) */
static const char kbd_ru_normal[128] = {
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

/* PS/2 Scan Code Set 2 to Set 1 Translation Map (for controllers without auto-translation) */
static const uint8_t set2_to_set1[256] = {
    [0x76] = 0x01, /* ESC */
    [0x16] = 0x02, /* 1 */
    [0x1E] = 0x03, /* 2 */
    [0x26] = 0x04, /* 3 */
    [0x25] = 0x05, /* 4 */
    [0x2E] = 0x06, /* 5 */
    [0x36] = 0x07, /* 6 */
    [0x3D] = 0x08, /* 7 */
    [0x3E] = 0x09, /* 8 */
    [0x46] = 0x0A, /* 9 */
    [0x45] = 0x0B, /* 0 */
    [0x4E] = 0x0C, /* - */
    [0x55] = 0x0D, /* = */
    [0x66] = 0x0E, /* Backspace */
    [0x0D] = 0x0F, /* Tab */
    [0x15] = 0x10, /* Q */
    [0x1D] = 0x11, /* W */
    [0x24] = 0x12, /* E */
    [0x2D] = 0x13, /* R */
    [0x2C] = 0x14, /* T */
    [0x35] = 0x15, /* Y */
    [0x3C] = 0x16, /* U */
    [0x43] = 0x17, /* I */
    [0x44] = 0x18, /* O */
    [0x4D] = 0x19, /* P */
    [0x54] = 0x1A, /* [ */
    [0x5B] = 0x1B, /* ] */
    [0x5A] = 0x1C, /* Enter */
    [0x14] = 0x1D, /* Left Ctrl */
    [0x1C] = 0x1E, /* A */
    [0x1B] = 0x1F, /* S */
    [0x23] = 0x20, /* D */
    [0x2B] = 0x21, /* F */
    [0x34] = 0x22, /* G */
    [0x33] = 0x23, /* H */
    [0x3B] = 0x24, /* J */
    [0x42] = 0x25, /* K */
    [0x4B] = 0x26, /* L */
    [0x4C] = 0x27, /* ; */
    [0x52] = 0x28, /* ' */
    [0x0E] = 0x29, /* ` */
    [0x12] = 0x2A, /* Left Shift */
    [0x5D] = 0x2B, /* \ */
    [0x1A] = 0x2C, /* Z */
    [0x22] = 0x2D, /* X */
    [0x21] = 0x2E, /* C */
    [0x2A] = 0x2F, /* V */
    [0x32] = 0x30, /* B */
    [0x31] = 0x31, /* N */
    [0x3A] = 0x32, /* M */
    [0x41] = 0x33, /* , */
    [0x49] = 0x34, /* . */
    [0x4A] = 0x35, /* / */
    [0x59] = 0x36, /* Right Shift */
    [0x7C] = 0x37, /* Keypad * */
    [0x11] = 0x38, /* Left Alt */
    [0x29] = 0x39, /* Space */
    [0x58] = 0x3A, /* Caps Lock */
    [0x05] = 0x3B, /* F1 */
    [0x06] = 0x3C, /* F2 */
    [0x04] = 0x3D, /* F3 */
    [0x0C] = 0x3E, /* F4 */
    [0x03] = 0x3F, /* F5 */
    [0x0B] = 0x40, /* F6 */
    [0x83] = 0x41, /* F7 */
    [0x0A] = 0x42, /* F8 */
    [0x01] = 0x43, /* F9 */
    [0x09] = 0x44, /* F10 */
    [0x77] = 0x45, /* Num Lock */
    [0x7E] = 0x46, /* Scroll Lock */
    [0x6C] = 0x47, /* Keypad 7 */
    [0x75] = 0x48, /* Keypad 8 */
    [0x7D] = 0x49, /* Keypad 9 */
    [0x7B] = 0x4A, /* Keypad - */
    [0x6B] = 0x4B, /* Keypad 4 */
    [0x73] = 0x4C, /* Keypad 5 */
    [0x74] = 0x4D, /* Keypad 6 */
    [0x79] = 0x4E, /* Keypad + */
    [0x69] = 0x4F, /* Keypad 1 */
    [0x72] = 0x50, /* Keypad 2 */
    [0x7A] = 0x51, /* Keypad 3 */
    [0x70] = 0x52, /* Keypad 0 */
    [0x71] = 0x53, /* Keypad . */
    [0x78] = 0x57, /* F11 */
    [0x07] = 0x58  /* F12 */
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
    /* Flush any stale bytes in controller output buffer */
    int timeout = 1000;
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL) && --timeout) {
        inb(PS2_DATA_PORT);
        io_wait();
    }

    /* Enable first PS/2 port */
    kbd_wait_input();
    outb(PS2_COMMAND_PORT, 0xAE);

    /* Enable scanning on PS/2 keyboard */
    kbd_wait_input();
    outb(PS2_DATA_PORT, 0xF4);

    kbd_head = 0;
    kbd_tail = 0;
    kbd_ev_head = 0;
    kbd_ev_tail = 0;
    shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    super_pressed = false;
    caps_lock = false;
    num_lock = true;
    scroll_lock = false;
    extended_mode = false;
    set2_break_mode = false;
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

    /* Handle Set 2 Break Code Prefix 0xF0 */
    if (scancode == 0xF0) {
        set2_break_mode = true;
        return;
    }

    bool released = false;
    uint8_t code = scancode;

    if (set2_break_mode) {
        set2_break_mode = false;
        released = true;
        /* Map Set 2 code to Set 1 code */
        if (set2_to_set1[code] != 0) {
            code = set2_to_set1[code];
        }
    } else {
        /* Check if Set 1 break code (bit 7 set) */
        if (code & 0x80) {
            released = true;
            code &= 0x7F;
        } else if (set2_to_set1[code] != 0 && code >= 0x60) {
            /* Raw Set 2 without translation */
            code = set2_to_set1[code];
        }
    }

    if (extended_mode) {
        extended_mode = false;
        int special_key = 0;
        switch (code) {
            case 0x48: special_key = KEY_UP; break;
            case 0x50: special_key = KEY_DOWN; break;
            case 0x4B: special_key = KEY_LEFT; break;
            case 0x4D: special_key = KEY_RIGHT; break;
            case 0x49: special_key = KEY_PAGE_UP; break;
            case 0x51: special_key = KEY_PAGE_DOWN; break;
            case 0x47: special_key = KEY_HOME; break;
            case 0x4F: special_key = KEY_END; break;
            case 0x52: special_key = KEY_INSERT; break;
            case 0x53: special_key = KEY_DELETE; break;
            case 0x1D: ctrl_pressed = !released; return; /* Right Ctrl */
            case 0x38: alt_pressed = !released; return;  /* Right Alt */
            case 0x5B:                                   /* Left GUI / Super */
            case 0x5C:                                   /* Right GUI / Super */
                super_pressed = !released;
                queue_event(KEY_SUPER, 0, !released);
                return;
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
        queue_event(KEY_CAPS_LOCK, 0, true);
        return;
    }
    if (code == 0x45 && !released) { /* Num Lock */
        num_lock = !num_lock;
        kbd_update_leds();
        queue_event(KEY_NUM_LOCK, 0, true);
        return;
    }
    if (code == 0x46 && !released) { /* Scroll Lock */
        scroll_lock = !scroll_lock;
        kbd_update_leds();
        queue_event(KEY_SCROLL_LOCK, 0, true);
        return;
    }

    /* Layout Switch on Alt + Shift or Ctrl + Shift */
    if ((alt_pressed || ctrl_pressed) && (code == 0x2A || code == 0x36) && !released) {
        current_layout = (current_layout == KBD_LAYOUT_US) ? KBD_LAYOUT_RU : KBD_LAYOUT_US;
        return;
    }

    /* Function Keys F1..F10 */
    if (code >= 0x3B && code <= 0x44) {
        int fkey = KEY_F1 + (code - 0x3B);
        queue_event(fkey, 0, !released);
        return;
    }
    /* Function Keys F11, F12 */
    if (code == 0x57) {
        queue_event(KEY_F11, 0, !released);
        return;
    }
    if (code == 0x58) {
        queue_event(KEY_F12, 0, !released);
        return;
    }

    if (code < 128) {
        bool use_upper = (shift_pressed ^ caps_lock);
        char ch = 0;
        if (current_layout == KBD_LAYOUT_RU) {
            ch = use_upper ? kbd_ru_normal[code] : kbd_ru_normal[code];
            if (ch == 0) {
                ch = use_upper ? kbd_us_shifted[code] : kbd_us_normal[code];
            }
        } else {
            ch = use_upper ? kbd_us_shifted[code] : kbd_us_normal[code];
        }

        /* Map common control keys */
        int key_code = ch;
        if (code == 0x01) key_code = KEY_ESC;
        else if (code == 0x1C) key_code = KEY_ENTER;
        else if (code == 0x0E) key_code = KEY_BACKSPACE;
        else if (code == 0x0F) key_code = KEY_TAB;
        else if (code == 0x39) key_code = KEY_SPACE;

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
    /* Send EOI to Master PIC */
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

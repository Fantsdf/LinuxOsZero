/*
 * LinuxOSZero - PS/2 Keyboard Driver Core Header
 * Architecture: x86_64
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/input.h"

/* PS/2 Controller I/O Ports */
#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64
#define PS2_COMMAND_PORT    0x64

/* PS/2 Status Register Bits */
#define PS2_STATUS_OUTPUT_BUFFER_FULL   0x01
#define PS2_STATUS_INPUT_BUFFER_FULL    0x02
#define PS2_STATUS_SYSTEM_FLAG          0x04
#define PS2_STATUS_COMMAND_DATA         0x08
#define PS2_STATUS_KEYBOARD_LOCKED      0x10
#define PS2_STATUS_AUX_OUTPUT_FULL      0x20
#define PS2_STATUS_TIMEOUT_ERROR        0x40
#define PS2_STATUS_PARITY_ERROR         0x80

/* Keyboard LED Bits */
#define KBD_LED_SCROLL_LOCK 0x01
#define KBD_LED_NUM_LOCK    0x02
#define KBD_LED_CAPS_LOCK   0x04

/* Keyboard Layouts */
#define KBD_LAYOUT_US   0
#define KBD_LAYOUT_RU   1

/* Functions */
void keyboard_init(void);
void keyboard_isr(void);
void keyboard_poll(void);
bool keyboard_has_char(void);
int keyboard_getchar(void);
bool keyboard_get_event(key_event_t *ev);
void keyboard_set_layout(int layout);
int keyboard_get_layout(void);

#endif /* KEYBOARD_H */

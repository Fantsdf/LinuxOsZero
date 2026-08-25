/*
 * LinuxOSZero - Sound Driver Implementation
 *
 * Plays UI sounds through the PC speaker using the Linux console bell
 * (KIOCSOUND ioctl on /dev/console). This is the standard way a minimalist
 * OS without a full ALSA/Pulse stack produces audible feedback.
 */

#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

static int console_fd = -1;
static bool muted = false;

/* The PC speaker is optional; every call must be robust if unavailable. */
static int get_console_fd(void) {
    if (console_fd >= 0) return console_fd;
    console_fd = open("/dev/console", O_WRONLY | O_NOCTTY);
    return console_fd;
}

int sound_init(void) {
    /* Just try to open the console; don't fail hard if absent. */
    get_console_fd();
    return 0;
}

void sound_tone(int freq_hz, int duration_ms) {
    if (muted || freq_hz <= 0 || duration_ms <= 0) return;
    int fd = get_console_fd();
    if (fd >= 0) {
        /* KIOCSOUND: pass frequency to start tone, 0 to stop. */
        ioctl(fd, KIOCSOUND, freq_hz);
        /* Sleep in small chunks so we remain responsive. */
        struct timespec ts = { 0, duration_ms * 1000000L };
        nanosleep(&ts, NULL);
        ioctl(fd, KIOCSOUND, 0);
    } else {
        /* Fallback: console bell. */
        fputc('\a', stderr);
        fflush(stderr);
    }
}

static const struct { int f0; int f1; int ms; } SND_TABLE[SND_COUNT] = {
    { 523, 784, 180 },  /* SND_STARTUP      : C5 -> G5 */
    { 659, 880, 120 },  /* SND_WINDOW_OPEN  : E5 -> A5 */
    { 392, 330, 120 },  /* SND_WINDOW_CLOSE : G4 -> E4 */
    { 700, 700,  30 },  /* SND_CLICK        : short tick */
    { 220, 180, 200 },  /* SND_ERROR        : low buzz */
    { 784, 1046, 140 }  /* SND_SUCCESS      : G5 -> C6 */
};

void sound_play(sound_event_t ev) {
    if (muted || ev >= SND_COUNT) return;
    sound_tone(SND_TABLE[ev].f0, SND_TABLE[ev].ms / 2);
    sound_tone(SND_TABLE[ev].f1, SND_TABLE[ev].ms / 2);
}

void sound_set_muted(bool m) {
    muted = m;
}

bool sound_is_muted(void) {
    return muted;
}

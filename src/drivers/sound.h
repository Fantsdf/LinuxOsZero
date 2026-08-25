/*
 * LinuxOSZero - Sound Driver (PC Speaker + Console Bell)
 *
 * Provides lightweight UI sound effects without a full audio stack:
 *  - PC speaker tones via /dev/console KIOCSOUND (ioctl)
 *  - Fallback to console bell (\a)
 *
 * Used by ZeroDesktop for startup, window open/close and click feedback.
 */

#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

/* UI sound event IDs */
typedef enum {
    SND_STARTUP = 0,
    SND_WINDOW_OPEN,
    SND_WINDOW_CLOSE,
    SND_CLICK,
    SND_ERROR,
    SND_SUCCESS,
    SND_COUNT
} sound_event_t;

/* Initialize the sound subsystem. Returns 0 on success. */
int sound_init(void);

/* Play a named UI event. No-op if sound is disabled. */
void sound_play(sound_event_t ev);

/* Play a raw frequency (Hz) for a duration (ms). */
void sound_tone(int freq_hz, int duration_ms);

/* Global mute switch. */
void sound_set_muted(bool muted);
bool sound_is_muted(void);

#endif /* SOUND_H */

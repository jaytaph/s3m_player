#include <stddef.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <i86.h>

#include "S3M.H"

#define PIT_SCALE   1193181
#define PIT_CONTROL 0x43
#define PIT_SET     0x36
#define PIT_A       0x40

typedef struct {
    t_s3m *s3m;         // current s3m file playing
    char playing;       // 0 is stop/paused. 1 = playing

    unsigned char order_idx;      // current order currently playing
    unsigned char row;            // current row playing
    unsigned char row_tick;       // current tick in the row

    t_s3m_pattern pattern;          // Current pattern playing
    unsigned char cur_pattern;      // current pattern index
    unsigned char cur_speed;        // current speed (number of ticks per row)
    unsigned char cur_tempo;        // current tempo (BPM in which IRQ is triggered)

    float msec_per_ticks;           // how many ms per tick are added
    float msec_ticks;               // number of ticks in this second in millisecs
    int elapsed;                    // number of seconds elapsed since play
} t_cp;

volatile t_cp cp;

// Interrupt handler. This will be called 50 times per second
void interrupt irq_handler(void) {
    // Acknowledge irq (8)
    outp(0x20, 0x20);

    if (cp.playing == 0) {
        return;
    }

    // Increase ticks per second, and update elapsed when a second has passed
    cp.msec_ticks += cp.msec_per_ticks;
    if (cp.msec_ticks >= 1000) {   // Got 1000 milliseconds, increase time elapsed
        cp.elapsed++;
        cp.msec_ticks -= 1000;
    }

    // Update tick counter in info
    cp.row_tick++;
    if (cp.row_tick > cp.cur_speed) {
        cp.row_tick = 0;

        cp.row++;
        if (cp.row == 64) {
            cp.row = 0;

            cp.order_idx++;
            if (cp.order_idx == cp.s3m->header.ord_num) {
                // Stop playback when finished
                cp.playing = 0;
                s3m_reset();
            }

            cp.cur_pattern = cp.s3m->order[cp.order_idx];
            s3m_get_pattern(cp.s3m, cp.cur_pattern, (t_s3m_pattern *)&cp.pattern);
        }
    }

    if (cp.row_tick == 0) {
        // First tick. Do effects and stuff
        printf("first tick\n");

    } else {
        // Another tick, continue effect stuff if needed
        printf("tick %d\n", cp.row_tick);
    }
}

void interrupt (*oldhandler)(void);

void _setup_handler(void) {
    // Save old handler and set our new handler
    oldhandler = _dos_getvect(8);
    _dos_setvect(8, irq_handler);
}

void _pit_set_speed(float hz) {
    unsigned int divisor = PIT_SCALE / hz;

    printf("Setting PIT to %.2f (%u) Hz\n", hz, divisor);

    _disable();
    outp(PIT_CONTROL, PIT_SET);
    outp(PIT_A, divisor & 0xFF);
    outp(PIT_A, (divisor >> 8) & 0xFF);
    _enable();
}

void _fini_handler(void) {
    _dos_setvect(8, oldhandler);
}

// Resets the song back to the beginning. Does not automatically play the song and can be used during playback
void s3m_reset(void) {
    cp.order_idx = 0;
    cp.msec_ticks = 0;
    cp.msec_per_ticks = 0;

    cp.cur_pattern = cp.s3m->order[cp.order_idx];
    s3m_get_pattern(cp.s3m, cp.cur_pattern, (t_s3m_pattern *)&cp.pattern);

    cp.cur_speed = cp.s3m->header.initial_speed;
    cp.cur_tempo = cp.s3m->header.initial_tempo;

    _pit_set_speed(2 * cp.cur_tempo / 5.0);   // tempo (in BPM) to hz
    cp.msec_per_ticks = 1000 / (2 * cp.cur_tempo / 5.0);

    cp.row = 0;
    cp.row_tick = -1;       // Start with -1, so first tick will be 0

    cp.elapsed = 0;
}

// Init the song (and possibly start playing)
void s3m_play(t_s3m *s3m, int autostart) {
    cp.s3m = s3m;
    s3m_reset();

    cp.playing = (autostart == 1);

    _setup_handler();
}

// Resume playing
void s3m_resume(void) {
    cp.playing = 1;
}

// (temporary) halt playing
void s3m_pause(void) {
    cp.playing = 0;
}

// Stops the song. Does keep track of the current position, but cannot be resumed anymore
void s3m_stop(void) {
    cp.playing = 0;
    _fini_handler();
}

t_s3m_info info;

// Returns the current playing information
t_s3m_info *s3m_get_playinfo(void) {
    info.elapsed = cp.elapsed;
    info.pattern = (t_s3m_pattern *)&cp.pattern;
    info.cur_pattern = cp.cur_pattern;
    info.cur_row = cp.row;
    info.tick = cp.row_tick;
    info.playing = cp.playing;

    return &info;
}
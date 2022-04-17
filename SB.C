#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "SB.H"

int sb_baseport = 0x220;        // Default settings for soundblaster cards
int sb_irq = 7;
int sb_dma = 1;
int sb_version = 0;

volatile unsigned char *sample_off;         // Position in the sample to read next
volatile unsigned int sample_idx;           // Position in the sample to read next
volatile unsigned int sample_len;           // Length of the sample to play
volatile unsigned char block_half;          // either 0 (first half) or 1 (second half)

void auto_init_playback(void);

int sb_detect(void) {
    int i;

    // detect port 0x200 to 0x280, except 0x270
    for (i=1; i<9; i++) {
        if (i != 7 && sb_dsp_reset(0x200 + (i << 4))) {
            break;
        }
    }
    if (i == 9) {
        return 0;
    }

    // Read sb version
    sb_dsp_write(0xe1);
    sb_version = sb_dsp_read() << 8;
    sb_version += sb_dsp_read();

    return 1;
}


void sb_free_sample(t_sample *sample) {
    printf("freeing sample: %08X\n", &sample);

    if (!sample) {
        return;
    }

    if (sample->data != NULL) {
        free(sample->data);
    }

    free(sample);
}

void sb_play(t_sample *sample) {
    int len, i;

    sb_dsp_write(SB_SET_PLAYBACK_FREQUENCY);
    sb_dsp_write(256 - 1000000 / sample->freq);

    sb_dsp_write(SB_ENABLE_SPEAKER);

    len = sample->len;
    if (len > DMA_BUFSIZE / 2) {
        len = DMA_BUFSIZE / 2;
    }

    // initialize dma for samples
    sample_off = sample->data;
    sample_len = sample->len;
    sample_idx = 0;
    block_half = 0;

    // @TODO: This actually only fills half the buffer
    if (!sb_dma_fill_buf()) {
        printf("Single cycle\n");
        sb_dma_single_cycle_playback();
    } else {
        printf("AutoInit\n");
        sb_dma_auto_init_playback();
    }
}

void sb_stop(void) {
    sb_dsp_write(SB_STOP_AUTOINIT_PLAYBACK);
    // Turn off speaker
    sb_dsp_write(SB_DISABLE_SPEAKER);
}

void sb_init(void) {
    sb_dma_alloc_buf(DMA_BUFSIZE);
    sb_irq_init();

    sb_dsp_write(SB_ENABLE_SPEAKER);

    atexit(sb_fini);
}

void sb_fini(void) {
    sb_stop();

    sb_irq_fini();
    sb_dma_free_buf();
}
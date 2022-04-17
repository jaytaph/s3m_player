#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "SB.H"

void *old_irq_handler2;

void interrupt sb_irq_handler() {
    int len;

    inp(sb_baseport + SB_READ_DATA_STATUS);

    if (!sb_dma_fill_buf()) {
        // sample done. Stop auto playback
        printf("done playing\n");
        sb_dsp_write(0xD0);
//        sb_dsp_write(SB_STOP_AUTOINIT_PLAYBACK);
        block_half = 0;
    }

    // Ack IRQ
    outp(0x20, 0x20);
    if( sb_irq == 2 || sb_irq == 10 || sb_irq == 11 ) {
        // acknowledge high irq's as well
        outp(0xA0, 0x20);
    }
}

void *set_irq_handler(int irq, void *handler) {
    int i, j;
    void *old_handler;

    switch (irq) {
        case 2:
            i = 0x71;   // interrupt number
            j = 253;    // mask for PIC
            break;
        case 10:
            i = 0x72;
            j = 251;
            break;
        case 11:
            i = 0x73;
            j = 247;
            break;
        default:
            i = sb_irq + 8;
            break;
    }

    printf("Setting vector: %02X\n", i);
    old_handler = (void *)_dos_getvect(i);
    _dos_setvect(i, (void interrupt *)handler);

    if (i >= 0x71) {
        outp(0xA1, inp(0xA1) & j);
        outp(0x21, inp(0x21) & 251);
    } else {
        outp(0x21, inp(0x21) & !(1 << sb_irq));
    }

    return old_handler;
}

void sb_irq_init(void) {
    old_irq_handler2 = set_irq_handler(sb_irq, (void *)sb_irq_handler);
}

void sb_irq_fini(void) {
    set_irq_handler(sb_irq, old_irq_handler2);
}
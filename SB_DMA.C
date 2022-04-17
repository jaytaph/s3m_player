#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "SB.H"

unsigned int dma_lba;

void sb_dma_alloc_buf(int len) {
    union REGS regs;
    struct SREGS sregs;

    printf(" * Allocating DMA buffer of %d (%04x) bytes...\n", len, len);

    memset(&sregs, 0, sizeof(sregs));
    regs.w.ax = 0x0100;
    regs.w.bx = ((len * 2) + 15) >> 4;     // number of paragraphs
    int386x(0x31, &regs, &regs, &sregs);
    dma_lba = regs.w.ax << 4;
    dma_lba += len;

    printf("Allocated DMA buffer at: %08X", dma_lba);
}

void sb_dma_free_buf(void) {
    // do nothing
}

// sb_dma_fill_buf will transfer a part of the sample to the correct dma block and updates all the sample_* info
// will return the number of bytes transferred. (0 means nothing left to do)
int sb_dma_fill_buf(void) {
    int len;

    if (sample_idx >= sample_len) {
        // sample completed
        return 0;
    }

    len = sample_len - sample_idx;
    if (len > (DMA_BUFSIZE / 2)) {
        len = (DMA_BUFSIZE / 2);
    }

    // copy in the correct half
    printf("LEN: %d  LEN %d IDX: %d  BH: %d (%d togo)\n", len, sample_len, sample_idx, block_half, sample_len - sample_idx);
    memcpy((unsigned char *)dma_lba + (block_half * (DMA_BUFSIZE / 2)), (unsigned char *)sample_off + sample_idx, len);
    sample_idx += len;

    // Fill remainder of the buffer with 0x80 (no sound)
    if (len < (DMA_BUFSIZE / 2)) {
        memset((unsigned char *) dma_lba + (block_half * (DMA_BUFSIZE / 2)) + len, 0x80, (DMA_BUFSIZE / 2) - len);
    }

    block_half = !block_half;

    return sample_idx != sample_len;
}

void sb_dma_single_cycle_playback(void) {
    unsigned short page, offset;

    // get page and offset of our dma buffer
    page = (long)(dma_lba >> 16);
    offset = dma_lba & 0xFFFF;

    outp(0x0A, 4 | sb_dma);             // write single mask register (select correct DMA chan)

    outp(0x0C, 0);                      // reset flip-flop
    outp(0x0B, 0x48 | sb_dma);          // write: single cycle playback on given DMA channel

    outp((sb_dma << 1) + 1, (sample_len-1) & 0xFF);    // output length of DMA buffer size (lo 8 hits)
    outp((sb_dma << 1) + 1, (sample_len-1) >> 8);      // output length of DMA buffer size (hi 8 bits)

    outp(sb_dma << 1, offset & 0xFF);   // set DMA offset (lo 8bits))
    outp(sb_dma << 1, offset >> 8);     // set DMA offset (hi 8bits)

    switch (sb_dma) {                   // set DMA page number (in 64k pages)
        case 0:
            outp(0x87, page);
            break;
        case 1:
            outp(0x83, page);
            break;
        case 3:
            outp(0x82, page);
            break;
    }

    outp(0x0A, sb_dma);                             // restore dma write mask

    sb_dsp_write(SB_START_SINGLE_CYCLE);
}

void sb_dma_auto_init_playback(void) {
    unsigned short page, offset;

    // get page and offset of our dma buffer
    page = (long)(dma_lba >> 16);
    offset = dma_lba & 0xFFFF;

//    printf("AIP: LBA: %08X\n", dma_lba);
//    printf("AIP: PAGE: %04X\n", page);
//    printf("AIP: OFFS: %04X\n", offset);

    outp(0x0A, 4 | sb_dma);             // write single mask register (select correct DMA chan)

    outp(0x0C, 0);                      // reset flip-flop
    outp(0x0B, 0x58 | sb_dma);          // write: auto-init playback on given DMA channel

    outp((sb_dma << 1) + 1, DMA_BUFSIZE & 0xFF);    // output length of DMA buffer size (lo 8 hits)
    outp((sb_dma << 1) + 1, DMA_BUFSIZE >> 8);      // output length of DMA buffer size (hi 8 bits)

    outp(sb_dma << 1, offset & 0xFF);   // set DMA offset (lo 8bits))
    outp(sb_dma << 1, offset >> 8);     // set DMA offset (hi 8bits)

    switch (sb_dma) {                   // set DMA page number (in 64k pages)
        case 0:
            outp(0x87, page);
            break;
        case 1:
            outp(0x83, page);
            break;
        case 3:
            outp(0x82, page);
            break;
    }

    outp(0x0A, sb_dma);                             // restore dma write mask


    sb_dsp_write(SB_SET_BLOCK_SIZE);                // set block size
    sb_dsp_write((DMA_BUFSIZE / 2) & 0xFF);         // trigger sb interrupt at half size of the block
    sb_dsp_write((DMA_BUFSIZE / 2) >> 8);
    sb_dsp_write(SB_START_AUTOINIT_PLAYBACK);       // start auto-init dma transfer
}
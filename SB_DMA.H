#ifndef __SB_DMA_H__
#define __SB_DMA_H__

    extern unsigned int dma_lba;

    #define DMA_BUFSIZE     (1024 * 16)         // DMA buffer in bytes

    void sb_dma_alloc_buf(int len);
    void sb_dma_free_buf(void);
    int sb_dma_fill_buf(void);
    void sb_dma_auto_init_playback(void);
    void sb_dma_single_cycle_playback(void);

#endif
#ifndef __SB_H__
#define __SB_H__

    #include "SB_DSP.H"
    #include "SB_IRQ.H"
    #include "SB_DMA.H"

    typedef struct {
        unsigned char *data;    // sample data
        int len;                // length of data
        int chan;               // 0 = mono 1 = stereo
        int bit;                // bitsize (8 or 16)
        int freq;               // 11025hz or more
    } t_sample;

    extern int sb_baseport;
    extern int sb_irq;
    extern int sb_dma;
    extern int sb_version;

    extern volatile unsigned char *sample_off;      // Sample offset
    extern volatile unsigned int sample_len;        // Length of the sample to play
    extern volatile unsigned int sample_idx;        // Position in the sample next to read (done when sample_idx = sample_len)
    extern volatile unsigned char block_half;       // either 0 (first half) or 1 (second half)

    int sb_detect(void);

    void sb_free_sample(t_sample *sample);

    void sb_play(t_sample *sample);
    void sb_stop(void);

    void sb_init(void);
    void sb_fini(void);

#endif // __SB_H__
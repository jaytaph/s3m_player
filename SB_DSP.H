#ifndef __SB_DSP_H__
#define __SB_DSP_H__

    #define SB_TIMER1_DATA          0x02
    #define SB_TIMER2_DATA          0x03
    #define SB_TC_PORT              0x04
    #define SB_RESET                0x06
    #define SB_READ_DATA            0x0A
    #define SB_WRITE_DATA_STATUS    0x0C
    #define SB_WRITE_DATA           0x0C
    #define SB_READ_DATA_STATUS     0x0E
    #define SB_MIXER                0x24
    #define SB_MIXER_DATA           0x25

    #define SB_STOP_AUTOINIT_PLAYBACK   0xDA
    #define SB_ENABLE_SPEAKER           0xD1
    #define SB_DISABLE_SPEAKER          0xD3
    #define SB_SET_PLAYBACK_FREQUENCY   0x40
    #define SB_START_SINGLE_CYCLE       0xD4
    #define SB_START_AUTOINIT_PLAYBACK  0x1C
    #define SB_SET_BLOCK_SIZE           0x48

    int sb_dsp_reset(int port);
    void sb_dsp_write(unsigned char val);
    unsigned char sb_dsp_read(void);

#endif
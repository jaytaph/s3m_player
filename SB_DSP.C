#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "SB.H"
#include "SB_DSP.H"

int sb_dsp_reset(int port) {
    outp(port + SB_RESET, 1);
    delay(10);
    outp(port + SB_RESET, 0);
    delay(10);

    if (((inp(port + SB_READ_DATA_STATUS) & 0x80) == 0x80) && (inp(port + SB_READ_DATA) == 0xAA)) {
        sb_baseport = port;
        return 1;
    }

   return 0;
}

void sb_dsp_write(unsigned char val) {
    while (inp(sb_baseport + SB_WRITE_DATA_STATUS) & 0x80) ;
    outp(sb_baseport + SB_WRITE_DATA, val);
}

unsigned char sb_dsp_read(void) {
    while (inp(sb_baseport + SB_READ_DATA_STATUS) == 0) ;
    return inp(sb_baseport + SB_READ_DATA);
}
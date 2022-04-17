#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <dos.h>
#include "S3M.H"

char notelist[12][2] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };

// Frees a pattern allocated by s3m_get_pattern()
void s3m_free_pattern(t_s3m_pattern *pat) {
    if (pat) {
        free(pat);
    }
}

// Unpacks a pattern into a fully unpacked structure (10KB)
t_s3m_pattern *s3m_get_pattern(t_s3m *s3m, int patnr, t_s3m_pattern *pat) {
    unsigned char *ptr;
    unsigned char row, b, chan;

    // clear pattern data
    memset(pat, 0, sizeof(t_s3m_pattern));

    // unpack data pattern
    ptr = s3m->pattern_data[patnr];
    row = 0;
    while (row < 64) {
        b = *ptr++;
        if (b == 0) {
            // row completed
            row++;
            continue;
        }

        chan = b & 31;
        if ((b & 32) == 32) {
            pat->rows[row][chan].note = *ptr++;
            pat->rows[row][chan].instrument = *ptr++;
        }
        if ((b & 64) == 64) {
            pat->rows[row][chan].volume = *ptr++;
        }
        if ((b & 128) == 128) {
            pat->rows[row][chan].special_command = *ptr++;
            pat->rows[row][chan].command_info = *ptr++;
        }
    }

    return pat;
}

// Read a complete S3m file: all samples are loaded into EMS, but pattern data stays packed
t_s3m *s3m_readfile(const char *filename) {
    FILE *f;
    t_s3m *s3m;
    int i, j;
    unsigned short int len, pp;
    unsigned long l, curpos, lpp;
    unsigned int total_bytes;


    printf("=========================================================\n");
    printf("Opening file %s: ", filename);
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open file for reading");
        return NULL;
    }
    printf("ok\n");

    s3m = (t_s3m *)malloc(sizeof(t_s3m));
    if (!s3m) {
        fprintf(stderr, "Cannot allocate memory");
        s3m_freefile(s3m);
        return NULL;
    }
    memset(s3m, 0, sizeof(t_s3m));

    // Read header
    printf("Reading header: ");
    fseek(f, 0, SEEK_SET);
    fread(&s3m->header, 1, sizeof(t_s3m_header), f);

    if (s3m->header.magic[0] != 'S' || s3m->header.magic[1] != 'C' || s3m->header.magic[2] != 'R' || s3m->header.magic[3] != 'M') {
        fprintf(stderr, "Did not detect a valid SRCM file");
        s3m_freefile(s3m);
        return NULL;
    }

    // Read orders
    if (s3m->header.ord_num > 255) {
        s3m->header.ord_num = 255;  // sanity, as ord_num is byte
    }
    fread(s3m->order, 1, s3m->header.ord_num, f);
    printf("ok\n");


    // Read instruments headers (not the actual samples)
    printf("Reading instruments: ");
    for (i=0; i!=s3m->header.ins_num; i++) {
        fread(&pp, sizeof(unsigned short int), 1, f);
        curpos = ftell(f);

        // seek and load instrument
        s3m->instrument[i] = (t_s3m_instrument *)malloc(sizeof(t_s3m_instrument));
        if (! s3m->instrument[i]) {
            fprintf(stderr, "Cannot allocate memory");
            s3m_freefile(s3m);
            return NULL;
        }
        fseek(f, (long)pp * 16, SEEK_SET);
        fread(s3m->instrument[i], 1, sizeof(t_s3m_instrument), f);

        // go back to the last instrument position
        fseek(f, curpos, SEEK_SET);
    }
    printf("ok\n");

    // Read patterns
    printf("Reading patterns: ");
    for (i=0; i!=s3m->header.pat_num; i++) {
        // read parapointer, and store current position for later to return
        fread(&pp, sizeof(unsigned short int), 1, f);
        curpos = ftell(f);

        if (pp == 0) {
            continue;
        }

        // parapointer to the actual packed data
        fseek(f, (long)pp * 16, SEEK_SET);

        // Read length of packed data
        fread(&len, sizeof(unsigned short int), 1, f);

        // seek and load instrument
        s3m->pattern_data[i] = (unsigned char *)malloc(len);
        if (! s3m->pattern_data[i]) {
            fprintf(stderr, "Cannot allocate memory");
            s3m_freefile(s3m);
            return NULL;
        }

        fread(s3m->pattern_data[i], 1, len, f);

        // go back to the last pattern parapointer position
        fseek(f, curpos, SEEK_SET);
    }
    printf("ok\n");


    total_bytes = 0;
    printf("Reading sample data: ");
    for (i=0; i!=s3m->header.ins_num; i++) {
        if (s3m->instrument[i]->type != S3M_INST_TYPE_SAMPLE) {
            // only read samples
            continue;
        }

        // calculate parapointer from memseg
        lpp = (s3m->instrument[i]->data.sample.memseg[0] << 16) + (s3m->instrument[i]->data.sample.memseg[2]  << 8) + s3m->instrument[i]->data.sample.memseg[1];

        // seek and load sample
        len = s3m->instrument[i]->data.sample.length;
//        printf("  %02d: %05u %s \n", i, len, s3m->instrument[i]->data.sample.sample_name);

        s3m->sample[i] = (unsigned char *)malloc(len);
        if (! s3m->sample[i]) {
            fprintf(stderr, "Cannot allocate memory");
            s3m_freefile(s3m);
            return NULL;
        }

        total_bytes += len;

        fseek(f, lpp * 16, SEEK_SET);
        j = fread(s3m->sample[i], 1, len, f);
        if (j != len) {
            fprintf(stderr, "false read\n");
            return NULL;
        }
    }
    printf("%dKB: ok\n", total_bytes / 1024);

    printf("=========================================================\n");
    printf("\n\n");

    return s3m;
}

// Free an s3m file from memory
void s3m_freefile(t_s3m *s3m) {
    int i;

    if (!s3m) {
        return;
    }

    // release patterns
    for (i=0; i!=255; i++) {
        if (s3m->pattern_data[i]) {
            free(s3m->pattern_data[i]);
        }
    }

    // Release instruments and sample data
    for (i=0; i!=s3m->header.ins_num; i++) {
        if (s3m->instrument[i]) {
            free(s3m->instrument[i]);
        }
        if (s3m->sample[i]) {
            free(s3m->sample[i]);
        }
    }

    free(s3m);
}

// Returns a 'A#3 01 25 D03' string from the given row. Assumes we have enough space in string 'dst'
void s3m_get_rowstr(t_s3m_row *row, char *dst) {
    char buf[4];

    sprintf(dst, "... .. .. .00");

    if (row->note != 0) {
        sprintf(buf, "...");
        if (row->note == 255) {
            sprintf(buf, "^^^");
        } else if (row->note == 254) {
            sprintf(buf, "===");
        } else {
            buf[0] = notelist[row->note & 0x0F][0];
            buf[1] = notelist[row->note & 0x0F][1];
            buf[2] = '0' + (row->note >> 4);
        }
        memcpy(dst, buf, 3);
    }
    if (row->instrument != 0) {
        sprintf(buf, "%02d", row->instrument);
        memcpy(dst + 4, buf, 2);
    }
    if (row->volume != 0) {
        sprintf(buf, "%02d", row->volume);
        memcpy(dst + 7, buf, 2);
    }

    if (row->special_command != 0) {
        sprintf(buf, "%c%02X", 'A' - 1 + row->special_command, row->command_info);
        memcpy(dst + 10, buf, 3);
    }
}
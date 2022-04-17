#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>

#include "SB.H"
#include "S3M.H"

int main(int argc, char *argv[]) {
    int i, c, v, smp;
    t_s3m *s3m;
    t_sample sam;
    char s[14];
    int current_playing_pattern, current_playing_row;
    t_s3m_info *info;
    char done;
    char row_changed,pat_changed;
    t_s3m_pattern *cp;

    if (argc <= 1) {
        printf("usage: %s <filename.s3m>\n", argv[0]);
        exit(1);
    }

    if (!sb_detect()) {
        printf("Soundblaster card not detected.\n");
        exit(1);
    }

    // Set 80x50
    _setvideomoderows(_TEXTC80, _MAXTEXTROWS);

    sb_init();

    s3m = s3m_readfile(argv[1]);
    if (!s3m) {
        printf("\n\nuh oh.. no s3m for you!");
        exit(1);
    }

    printf("Song name: %s\n", s3m->header.song_name);

    s3m_play(s3m, 0);

    info = s3m_get_playinfo();

    smp = -1;
    done = 0;
    while (!done) {
        // handle keyboard commands
        if (kbhit()) {
            c = getch();
            if (c == 27) {
                // escape quits
                done = 1;
            }
            if (c == 'p') {
                info = s3m_get_playinfo();

                if (info->playing == 1) {
                    printf("Pausing...\n");
                    s3m_pause();
                } else {
                    printf("Resume...\n");
                    s3m_resume();
                }
            }

            if (c == 'a') {

                do {
                    smp++;
                } while (smp < 100 && s3m->instrument[smp]->type != S3M_INST_TYPE_SAMPLE);
                printf("SMP: %d\n", smp);

                sam.bit = (s3m->instrument[smp]->data.sample.flags & 4) ? 16 : 8;
                sam.chan = (s3m->instrument[smp]->data.sample.flags & 2) ? 1 : 0;
                sam.freq = 8363 * 16 * (1712) / s3m->instrument[smp]->data.sample.c2speed;
                sam.len = s3m->instrument[smp]->data.sample.length;
                sam.data = s3m->sample[smp];
                sb_play(&sam);
            }
            if (c == 's') {
                do {
                    smp--;
                } while (smp > 0 && s3m->instrument[smp]->type != S3M_INST_TYPE_SAMPLE);

                printf("SMP: %d\n", smp);
                sam.bit = (s3m->instrument[smp]->data.sample.flags & 4) ? 16 : 8;
                sam.chan = (s3m->instrument[smp]->data.sample.flags & 2) ? 1 : 0;
                sam.freq = 8363 * 16 * (1712) / s3m->instrument[smp]->data.sample.c2speed;
                sam.len = s3m->instrument[smp]->data.sample.length;
                sam.data = s3m->sample[smp];
                sb_play(&sam);
            }

        }

        info = s3m_get_playinfo();
        if (info->playing == 1 && (current_playing_pattern != info->cur_pattern || current_playing_row != info->cur_row)) {
            pat_changed = current_playing_pattern != info->cur_pattern;
            row_changed = current_playing_row != info->cur_row;
            current_playing_pattern = info->cur_pattern;
            current_playing_row = info->cur_row;

            cp = info->pattern;

            if (row_changed) {
                printf("%02d|", current_playing_row);
                for (c=0; c!=5; c++) {
                    s3m_get_rowstr(&cp->rows[current_playing_row][c], s);
                    printf("%s|", s);
                }
                printf("\n");
            }
        }
    }

    s3m_stop();

    s3m_freefile(s3m);

    sb_fini();
    return 0;
}

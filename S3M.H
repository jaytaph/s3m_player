#ifndef __S3M_H__
#define __S3M_H__

#pragma pack(push, 1)
typedef struct {
    unsigned char   song_name[28];
    unsigned char   marker;   // 0x1A
    unsigned char   type;
    unsigned char   _res1;
    unsigned char   _res2;

    unsigned short int             ord_num;
    unsigned short int             ins_num;
    unsigned short int             pat_num;
    unsigned short int             flags;
    unsigned short int             created_with_tracker_version;
    unsigned short int             file_format_version;
    unsigned char   magic[4]; // SCRM

    unsigned char   global_volume;
    unsigned char   initial_speed;
    unsigned char   initial_tempo;
    unsigned char   master_volume;
    char            _res3[10];
    unsigned short int             special;
    unsigned char   channel_settings[32];
} t_s3m_header;
#pragma pack(pop)


typedef struct {
    unsigned char   note;
    unsigned char   instrument;
    unsigned char   volume;
    unsigned char   special_command;
    unsigned char   command_info;
} t_s3m_row;

typedef struct {
    t_s3m_row   rows[64][32];
} t_s3m_pattern;

#define S3M_INST_TYPE_SAMPLE      1
#define S3M_INST_TYPE_ADLIB       2
#define S3M_INST_TYPE_ADLIB_DRUM  3

#pragma pack(push, 1)
typedef struct {
    char                    memseg[3];
    unsigned short int      length;
    unsigned short int      length_high;
    unsigned short int      loop_begin;
    unsigned short int      loop_begin_high;
    unsigned short int      loop_end;
    unsigned short int      loop_end_high;
    char                    volume;
    char                    _res1;
    char                    packed;
    char                    flags;
    unsigned short int      c2speed;
    unsigned short int      c2speed_high;
    char                    _res2[4];
    unsigned short int      gravis_gp;
    long                    lasted_used;
    unsigned short int      sb_loop_expansion;
    unsigned char           sample_name[28];
    unsigned char           marker[4];  // SCRS
} t_s3m_sample;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    char            zero[3];
    char            d0[12];
    char            vol;
    char            dsk;
    char            _res1[2];
    unsigned short int       c2speed;
    unsigned short int       c2speed_high;
    char            _res2[12];
    unsigned char   sample_name[28];
    unsigned char   marker[4];  // SCRI
} t_s3m_adlib;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    char            type;
    unsigned char   filename[12];
    union {
        t_s3m_sample sample;
        t_s3m_adlib adlib;
    } data;
} t_s3m_instrument;
#pragma pack(pop)

typedef struct {
    t_s3m_header        header;
    unsigned char       order[255];
    t_s3m_instrument    *instrument[100];
    unsigned char       *pattern_data[255];
    unsigned char       *sample[100];
} t_s3m;

typedef struct {
    int elapsed;                    // How many seconds elapsed in the song
    t_s3m_pattern *pattern;         // Current unpacked pattern
    int cur_pattern;                // Current pattern being played
    int cur_row;                    // Current row being played
    int tick;                       // Current tick
    char playing;                   // 0 = song paused/stopped  1 = song playing
} t_s3m_info;

t_s3m *s3m_readfile(const char *filename);
void s3m_freefile(t_s3m *s3m);

//t_s3m_pattern *s3m_get_pattern(t_s3m *s3m, int patnr, t_s3m_pattern *dst);
//void s3m_free_pattern(t_s3m_pattern *pat);

void s3m_get_rowstr(t_s3m_row *row, char *s);

void s3m_play(t_s3m *s3m, int autostart);
void s3m_stop(void);
void s3m_pause(void);
void s3m_resume(void);
void s3m_reset(void);
t_s3m_info *s3m_get_playinfo(void);
t_s3m_pattern *s3m_get_pattern(t_s3m *s3m, int patnr, t_s3m_pattern *pat);

#endif // __S3M_H__
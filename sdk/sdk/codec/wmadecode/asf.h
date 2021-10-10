#ifndef __ASF_H__
#define __ASF_H__

#include "types.h"
#include "metadata.h"

/* ASF codec IDs */
#define ASF_CODEC_ID_WMAV1 0x160
#define ASF_CODEC_ID_WMAV2 0x161

enum asf_error_e {
    ASF_ERROR_INTERNAL       = -1,  /* incorrect input to API calls */
    ASF_ERROR_OUTOFMEM       = -2,  /* some malloc inside program failed */
    ASF_ERROR_EOF            = -3,  /* unexpected end of file */
    ASF_ERROR_IO             = -4,  /* error reading or writing to file */
    ASF_ERROR_INVALID_LENGTH = -5,  /* length value conflict in input data */
    ASF_ERROR_INVALID_VALUE  = -6,  /* other value conflict in input data */
    ASF_ERROR_INVALID_OBJECT = -7,  /* ASF object missing or in wrong place */
    ASF_ERROR_OBJECT_SIZE    = -8,  /* invalid ASF object size (too small) */
    ASF_ERROR_SEEKABLE       = -9,  /* file not seekable */
    ASF_ERROR_SEEK           = -10, /* file is seekable but seeking failed */
    ASF_ERROR_ENCRYPTED      = -11  /* file is encrypted */
};

#define ID3V2_BUF_SIZE 300

struct mp3entry {
    char* title;
    char* artist;
    char* album;
    char* genre_string;
    char* disc_string;
    char* track_string;
    char* year_string;
    char* composer;
    char* comment;
    char* albumartist;
    char* grouping;
    int discnum;
    int tracknum;
    int version;
    int layer;
    int year;
    unsigned int bitrate;
    unsigned long frequency;
    unsigned long length;   /* song length in ms */
    unsigned char toc[100]; /* table of contents */

    /* these following two fields are used for local buffering */
    char id3v2buf[ID3V2_BUF_SIZE];
    char id3v1buf[4][92];
    short score;
};

struct asf_waveformatex_s {
    uint32_t packet_size;
    int audiostream;
    uint16_t codec_id;
    uint16_t channels;
    uint32_t rate;
    uint32_t bitrate;
    uint16_t blockalign;
    uint16_t bitspersample;
    uint16_t datalen;
    uint8_t data[6];
};

typedef struct asf_waveformatex_s asf_waveformatex_t;

/*MMP_INLINE*/
int get_asf_metadata(struct mp3entry* id3);

#endif // __ASF_H__


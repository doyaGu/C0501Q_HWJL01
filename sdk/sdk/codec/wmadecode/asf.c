/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * $Id: asf.c 14134 2007-08-02 04:47:33Z saratoga $
 *
 * Copyright (C) 2007 Dave Chapman
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "wmafixed.h"
#include "metadata.h"
#include "asf.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define MASK   0xC0 /* 11000000 */
#define COMP   0x80 /* 10x      */

/* TODO: Just read the GUIDs into a 16-byte array, and use memcmp to compare */
struct guid_s {
    uint32_t v1;
    uint16_t v2;
    uint16_t v3;
    uint8_t  v4[8];
};
typedef struct guid_s guid_t;

struct asf_object_s {
    guid_t       guid;
    uint64_t     size;
    uint64_t     datalen;
};
typedef struct asf_object_s asf_object_t;

static const guid_t asf_guid_null =
    { 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

/* top level object guids */

static const guid_t asf_guid_header =
    { 0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C} };

static const guid_t asf_guid_data =
    { 0x75B22636, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C} };

static const guid_t asf_guid_index =
    { 0x33000890, 0xE5B1, 0x11CF, {0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB} };

/* header level object guids */

static const guid_t asf_guid_file_properties = { 0x8cabdca1, 0xa947, 0x11cf, {0x8E, 0xe4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65} };

static const guid_t asf_guid_stream_properties =
{0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_content_description =
{0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_extended_content_description =
{0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}};

static const guid_t asf_guid_content_encryption =
{0x2211b3fb, 0xbd23, 0x11d2, {0xb4, 0xb7, 0x00, 0xa0, 0xc9, 0x55, 0xfc, 0x6e}};

static const guid_t asf_guid_extended_content_encryption =
{0x298ae614, 0x2622, 0x4c17, {0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c}};

/* stream type guids */

static const guid_t asf_guid_stream_type_audio =
{0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}};

static const unsigned char utf8comp[6] =
{
    0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

/* Encode a UCS value as UTF-8 and return a pointer after this UTF-8 char. */
static unsigned char* utf8encode(unsigned long ucs, unsigned char *utf8)
{
    int tail = 0;

    if (ucs > 0x7F)
        while (ucs >> (5*tail + 6))
            tail++;

    *utf8++ = (ucs >> (6*tail)) | utf8comp[tail];
    while (tail--)
        *utf8++ = ((ucs >> (6*tail)) & (MASK ^ 0xFF)) | COMP;

    return utf8;
}

static int asf_guid_match(const guid_t *guid1, const guid_t *guid2)
{
    if((guid1->v1 != guid2->v1) ||
       (guid1->v2 != guid2->v2) ||
       (guid1->v3 != guid2->v3) ||
       (memcmp(guid1->v4, guid2->v4, 8))) {
        return 0;
    }

    return 1;
}

/* Read the 16 byte GUID from a file */
static void asf_readGUID(guid_t* guid)
{
    read_uint32le(&guid->v1);
    read_uint16le(&guid->v2);
    read_uint16le(&guid->v3);
    read_filebuf(&guid->v4, sizeof(char)*8);
}

static void asf_read_object_header(asf_object_t *obj)
{
    asf_readGUID(&obj->guid);
    read_uint64le(&obj->size);
    obj->datalen = 0;
}

/* Parse an integer from the extended content object - we always
   convert to an int, regardless of native format.
*/
static int asf_intdecode(int type, int length)
{
    uint16_t tmp16;
    uint32_t tmp32;
    uint64_t tmp64;

    if (type==3) {
        read_uint32le(&tmp32);
        advance_buffer(length - 4);
        return (int)tmp32;
    } else if (type==4) {
        read_uint64le(&tmp64);
        advance_buffer(length - 8);
        return (int)tmp64;
    } else if (type == 5) {
        read_uint16le(&tmp16);
        advance_buffer(length - 2);
        return (int)tmp16;
    }

    return 0;
}

/* Decode a LE utf16 string from a disk buffer into a fixed-sized
   utf8 buffer.
*/

static void asf_utf16LEdecode(uint16_t utf16bytes,
                              unsigned char **utf8,
                              int* utf8bytes
                             )
{
    unsigned long ucs;
    int n;
    unsigned char utf16buf[256];
    unsigned char* utf16 = utf16buf;
    unsigned char* newutf8;

    n = read_filebuf(utf16buf, sizeof(char)*MIN(sizeof(utf16buf), utf16bytes));
    utf16bytes -= n;

    while (n > 0) {
        /* Check for a surrogate pair */
        if (utf16[1] >= 0xD8 && utf16[1] < 0xE0) {
            if (n < 4) {
                /* Run out of utf16 bytes, read some more */
                utf16buf[0] = utf16[0];
                utf16buf[1] = utf16[1];

                n = read_filebuf(utf16buf + 2, sizeof(char)*MIN(sizeof(utf16buf)-2, utf16bytes));
                utf16 = utf16buf;
                utf16bytes -= n;
                n += 2;
            }

            if (n < 4) {
                /* Truncated utf16 string, abort */
                break;
            }
            ucs = 0x10000 + ((utf16[0] << 10) | ((utf16[1] - 0xD8) << 18)
                             | utf16[2] | ((utf16[3] - 0xDC) << 8));
            utf16 += 4;
            n -= 4;
        } else {
            ucs = (utf16[0] | (utf16[1] << 8));
            utf16 += 2;
            n -= 2;
        }

        if (*utf8bytes > 6) {
            newutf8 = utf8encode(ucs, *utf8);
            *utf8bytes -= (newutf8 - *utf8);
            *utf8 += (newutf8 - *utf8);
        }

        /* We have run out of utf16 bytes, read more if available */
        if ((n == 0) && (utf16bytes > 0)) {
            n = read_filebuf(utf16buf, sizeof(char)*MIN(sizeof(utf16buf), utf16bytes));
            utf16 = utf16buf;
            utf16bytes -= n;
        }
    }

    *utf8[0] = 0;
    --*utf8bytes;

    if (utf16bytes > 0) {
        /* Skip any remaining bytes */
        advance_buffer(utf16bytes);
    }
    return;
}

static int asf_parse_header(struct mp3entry* id3, asf_waveformatex_t* wfx)
{
    asf_object_t current;
    asf_object_t header;
    uint64_t datalen;
    int i;
    int fileprop = 0;
    uint64_t play_duration;
    uint16_t flags;
    uint32_t subobjects;
    uint8_t utf8buf[512];
    int id3buf_remaining = sizeof(id3->id3v2buf) + sizeof(id3->id3v1buf);
    unsigned char* id3buf = (unsigned char*)id3->id3v2buf;

    asf_read_object_header((asf_object_t *) &header);

    PRINTF("header.size=%ld\n",(int32_t)header.size);
    //header size should not bigger than  header.size > 20 * 64 *1024
    if (header.size < 30  || header.size > 20 * 64 *1024) {
        /* invalid size for header object */
        return ASF_ERROR_OBJECT_SIZE;
    }

    read_uint32le(&subobjects);

    /* Two reserved bytes - do we need to read them? */
    advance_buffer(2);

    PRINTF("Read header - size=%d, subobjects=%d\n",(int)header.size, (int)subobjects);

    if (subobjects > 0) {
        header.datalen = header.size - 30;

        /* TODO: Check that we have datalen bytes left in the file */
        datalen = header.datalen;

        for (i=0; i<(int)subobjects; i++) {
            PRINTF("Parsing header object %d - datalen=%d\n",i,(int)datalen);
            if (datalen < 24) {
                PRINTF("not enough data for reading object\n");
                break;
            }

            asf_read_object_header(&current);

            if (current.size > datalen || current.size < 24) {
                PRINTF("invalid object size - current.size=%d, datalen=%d\n",(int)current.size,(int)datalen);
                break;
            }

            if (asf_guid_match(&current.guid, &asf_guid_file_properties)) {
                    if (current.size < 104)
                        return ASF_ERROR_OBJECT_SIZE;

                    if (fileprop) {
                        /* multiple file properties objects not allowed */
                        return ASF_ERROR_INVALID_OBJECT;
                    }

                    fileprop = 1;
                    /* All we want is the play duration - uint64_t at offset 40 */
                    advance_buffer(40);

                    read_uint64le(&play_duration);
                    #if defined(__USE_INT64_LIB__)
                    id3->length = __udiv64(play_duration, 10000);
                    #else
                    id3->length = play_duration / 10000;
                    #endif

                    PRINTF("****** length = %lums\n", id3->length);

                    /* Read the packet size - uint32_t at offset 68 */
                    advance_buffer(20);
                    read_uint32le(&wfx->packet_size);

                    /* Skip bytes remaining in object */
                    advance_buffer(current.size - 24 - 72);
            } else if (asf_guid_match(&current.guid, &asf_guid_stream_properties)) {
                    guid_t guid;
                    uint32_t propdatalen;

                    if (current.size < 78)
                        return ASF_ERROR_OBJECT_SIZE;

#if 0
                    asf_byteio_getGUID(&guid, current->data);
                    datalen = asf_byteio_getDWLE(current->data + 40);
                    flags = asf_byteio_getWLE(current->data + 48);
#endif

                    asf_readGUID(&guid);

                    advance_buffer(24);
                    read_uint32le(&propdatalen);
                    advance_buffer(4);
                    read_uint16le(&flags);

                    if (!asf_guid_match(&guid, &asf_guid_stream_type_audio)) {
                        PRINTF("Found stream properties for non audio stream, skipping\n");
                        advance_buffer(current.size - 24 - 50);
                    } else if (wfx->audiostream == -1) {
                        advance_buffer(4);
                        PRINTF("Found stream properties for audio stream %d\n",flags&0x7f);

                        if (propdatalen < 18) {
                            return ASF_ERROR_INVALID_LENGTH;
                        }

#if 0
                        if (asf_byteio_getWLE(data + 16) > datalen - 16) {
                            return ASF_ERROR_INVALID_LENGTH;
                        }
#endif
                        read_uint16le(&wfx->codec_id);
                        read_uint16le(&wfx->channels);
                        read_uint32le(&wfx->rate);
                        read_uint32le(&wfx->bitrate);
                        wfx->bitrate *= 8;
                        read_uint16le(&wfx->blockalign);
                        read_uint16le(&wfx->bitspersample);
                        read_uint16le(&wfx->datalen);

                        /* Round bitrate to the nearest kbit */
                        id3->bitrate = (wfx->bitrate + 500) / 1000;
                        id3->frequency = wfx->rate;

                        if (wfx->codec_id == ASF_CODEC_ID_WMAV1) {
                            read_filebuf(wfx->data, sizeof(char)*4);
                            advance_buffer(current.size - 24 - 72 - 4);
                            wfx->audiostream = flags&0x7f;
                        } else if (wfx->codec_id == ASF_CODEC_ID_WMAV2) {
                            read_filebuf(wfx->data, sizeof(char)*6);
                            advance_buffer(current.size - 24 - 72 - 6);
                            wfx->audiostream = flags&0x7f;
                        } else {
                            advance_buffer(current.size - 24 - 72);
                        }

                    }
            } else if (asf_guid_match(&current.guid, &asf_guid_content_description)) {
                    /* Object contains five 16-bit string lengths, followed by the five strings:
                       title, artist, copyright, description, rating
                     */
                    uint16_t strlength[5];
                    int m;

                    PRINTF("Found GUID_CONTENT_DESCRIPTION - size=%d\n",(int)(current.size - 24));

                    /* Read the 5 string lengths - number of bytes included trailing zero */
                    for (m=0; m<5; m++) {
                        read_uint16le(&strlength[m]);
                        PRINTF("strlength = %u\n",strlength[m]);
                    }

                    if (strlength[0] > 0) {  /* 0 - Title */
                        id3->title = id3buf;
                        asf_utf16LEdecode(strlength[0], &id3buf, &id3buf_remaining);
                    }

                    if (strlength[1] > 0) {  /* 1 - Artist */
                        id3->artist = id3buf;
                        asf_utf16LEdecode(strlength[1], &id3buf, &id3buf_remaining);
                    }

                    advance_buffer(strlength[2]); /* 2 - copyright */

                    if (strlength[3] > 0) {  /* 3 - description */
                        id3->comment = id3buf;
                        asf_utf16LEdecode(strlength[3], &id3buf, &id3buf_remaining);
                    }

                    advance_buffer(strlength[4]); /* 4 - rating */
            } else if (asf_guid_match(&current.guid, &asf_guid_extended_content_description)) {
                    uint16_t count;
                    int k;
                    int bytesleft = current.size - 24;
                    PRINTF("Found GUID_EXTENDED_CONTENT_DESCRIPTION\n");

                    read_uint16le(&count);
                    bytesleft -= 2;
                    PRINTF("extended metadata count = %u\n",count);

                    for (k=0; k < count; k++) {
                        uint16_t length, type;
                        unsigned char* utf8 = utf8buf;
                        int utf8length = 512;

                        read_uint16le(&length);
                        asf_utf16LEdecode(length, &utf8, &utf8length);
                        bytesleft -= 2 + length;

                        read_uint16le(&type);
                        read_uint16le(&length);

                        if (!strcmp("WM/TrackNumber",utf8buf)) {
                            if (type == 0) {
                                id3->track_string = id3buf;
                                asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                                id3->tracknum = atoi(id3->track_string);
                            } else if ((type >=2) && (type <= 5)) {
                                id3->tracknum = asf_intdecode(type, length);
                            } else {
                                advance_buffer(length);
                            }
                        } else if ((!strcmp("WM/Genre",utf8buf)) && (type == 0)) {
                            id3->genre_string = id3buf;
                            asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                        } else if ((!strcmp("WM/AlbumTitle",utf8buf)) && (type == 0)) {
                            id3->album = id3buf;
                            asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                        } else if ((!strcmp("WM/AlbumArtist",utf8buf)) && (type == 0)) {
                            id3->albumartist = id3buf;
                            asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                        } else if ((!strcmp("WM/Composer",utf8buf)) && (type == 0)) {
                            id3->composer = id3buf;
                            asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                        } else if (!strcmp("WM/Year",utf8buf)) {
                            if (type == 0) {
                                id3->year_string = id3buf;
                                asf_utf16LEdecode(length, &id3buf, &id3buf_remaining);
                                id3->year = atoi(id3->year_string);
                            } else if ((type >=2) && (type <= 5)) {
                                id3->year = asf_intdecode(type, length);
                            } else {
                                advance_buffer(length);
                            }
                        } else {
                            advance_buffer(length);
                        }
                        bytesleft -= 4 + length;
                    }

                    advance_buffer(bytesleft);
            } else if (asf_guid_match(&current.guid, &asf_guid_content_encryption)
                       || asf_guid_match(&current.guid, &asf_guid_extended_content_encryption)) {
                PRINTF("File is encrypted\n");
                return ASF_ERROR_ENCRYPTED;
            } else {
                PRINTF("Skipping %d bytes of object\n",(int)(current.size - 24));
                advance_buffer(current.size - 24);
            }

            PRINTF("Parsed object - size = %d\n",(int)current.size);
            datalen -= current.size;
        }

        if (i != (int)subobjects || datalen != 0) {
            PRINTF("header data doesn't match given subobject count\n");
            return ASF_ERROR_INVALID_VALUE;
        }

        PRINTF("%d subobjects read successfully\n", i);
    }

#if 0
    tmp = asf_parse_header_validate(file, &header);
    if (tmp < 0) {
        /* header read ok but doesn't validate correctly */
        return tmp;
    }
#endif

    PRINTF("header validated correctly\n");

    return 0;
}

/*MMP_INLINE*/
int get_asf_metadata(struct mp3entry* id3)
{
    int res;
    asf_object_t obj;
    asf_waveformatex_t wfx;

    wfx.audiostream = -1;

    res = asf_parse_header(id3, &wfx);

    if (res < 0) {
        PRINTF("ASF: parsing error - %d\n",res);
        return FALSE;
    }

    if (wfx.audiostream == -1) {
        PRINTF("ASF: No WMA streams found\n");
        return FALSE;
    }

    asf_read_object_header(&obj);

    if (!asf_guid_match(&obj.guid, &asf_guid_data)) {
        PRINTF("ASF: No data object found\n");
        return FALSE;
    }

    /* Store the current file position - no need to parse the header
       again in the codec.  The +26 skips the rest of the data object
       header.
     */
    //id3->first_frame_offset = ftell(fd) + 26;
    advance_buffer(26);

    /* We copy the wfx struct to the MP3 TOC field in the id3 struct so
       the codec doesn't need to parse the header object again */
    memcpy(id3->toc, &wfx, sizeof(wfx));

    return TRUE;
}

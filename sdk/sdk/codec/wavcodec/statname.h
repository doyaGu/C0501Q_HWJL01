/**
 * @file statname.h
 *  name mangling macros for static linking.
 */

#ifndef __STATNAME_H__
#define __STATNAME_H__

/* define STAT_PREFIX to a unique name for static linking
 * all the C functions and global variables will be mangled by the preprocessor
 *   e.g. void DCT4(...) becomes void wav_DCT4(...)
 */
#define STAT_PREFIX             wav

#define STATCC1(x,y,z)          STATCC2(x,y,z)
#define STATCC2(x,y,z)          x##y##z

#if defined(STAT_PREFIX)
#  define STATNAME(func)        STATCC1(STAT_PREFIX, _, func)
#else
#  define STATNAME(func)        func
#endif // defined(STAT_PREFIX)

/* these symbols are common to all implementations */
/* main.c */
#define ParseWaveHeader         STATNAME(ParseWaveHeader)
#define DecFillWriteBuffer      STATNAME(DecFillWriteBuffer)
#define DecFillReadBuffer       STATNAME(DecFillReadBuffer)
#define DecClearRdBuffer        STATNAME(DecClearRdBuffer)
#define checkControl            STATNAME(checkControl)
#define getStreamWrPtr          STATNAME(getStreamWrPtr)
#define WAVDecoder              STATNAME(WAVDecoder)
#define wavDecode               STATNAME(wavDecode)
#define WAVEncoder              STATNAME(WAVEncoder)
#define wavEncode               STATNAME(wavEncode)

/* adpcm.c */
#define indexTable              STATNAME(indexTable)
#define stepsizeTable           STATNAME(stepsizeTable)
#define adpcm_coder             STATNAME(adpcm_coder)
#define adpcm_decoder           STATNAME(adpcm_decoder)

/* g711.c */
#define seg_aend                STATNAME(seg_aend)
#define seg_uend                STATNAME(seg_uend)
#define search                  STATNAME(search)
#define linear2alaw             STATNAME(linear2alaw)
#define alaw2linear             STATNAME(alaw2linear)
#define linear2ulaw             STATNAME(linear2ulaw)
#define ulaw2linear             STATNAME(ulaw2linear)
#define pcm8_linear             STATNAME(pcm8_linear)
#define alawDecode              STATNAME(alawDecode)
#define ulawDecode              STATNAME(ulawDecode)
#define pcmDecode               STATNAME(pcmDecode)

/* global ROM tables */
/* main.c */
#if defined(__FREERTOS__)
#define streamBuf               STATNAME(streamBuf)
#define pcmWriteBuf             STATNAME(pcmWriteBuf)
#endif

#endif // __STATNAME_H__


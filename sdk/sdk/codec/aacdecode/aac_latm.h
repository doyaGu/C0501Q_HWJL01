//-----------------------------------------------------------------------------
//
//	AAC Decoder
//
//-----------------------------------------------------------------------------

#include "aacdec.h"
#include "aaccommon.h"
#include "bitstream.h"
#ifdef PARSING_HE_AAC_V2
#include "common_ps.h"
#endif

#define LOAS_HEADER_SIZE 3
#define LATM_MAX_LAYER (8)
#define LATM_MAX_PROGRAM (16)

#ifndef PARSING_HE_AAC_V2
    #ifndef uint8
    typedef unsigned char		uint8_t;
    typedef unsigned short		uint16_t;
    typedef unsigned int		uint32_t;
    typedef unsigned char   BYTE;
    #endif
#endif

#ifndef MAX
#   define MAX(a, b)   ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef MIN
#   define MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif
typedef struct
{
    int nObjectType;
    int nSampleRate;
    int nChannel;

    int nSBR;          // 0: no sbr, 1: sbr, -1: unknown
    int nPs;           // 0: no ps,  1: ps,  -1: unknown

    struct
    {
        int nObjectType;
        int nSampleRate;
    } extension;

    /* GASpecific */
    int nFrameLength;   // 1024 or 960

} mpeg4_cfg_t;

#define LATM_MAX_EXTRA_SIZE 64
typedef struct
{
    int nProgram;
    int nLayer;

    int nFrameLengthType;
    int nFrameLength;         // type 1
    int nFrameLengthIndex;   // type 3 4 5 6 7

    mpeg4_cfg_t cfg;

    /* Raw configuration */
    int     nExtra;
    uint8_t extra[LATM_MAX_EXTRA_SIZE];

} latm_stream_t;


typedef struct
{
    int nSameTiemFraming;
    int nSubFrames;
    int nPrograms;

    int tLayers[LATM_MAX_PROGRAM];

    int tStream[LATM_MAX_PROGRAM][LATM_MAX_LAYER];

    int nStreams;
    latm_stream_t stream[LATM_MAX_PROGRAM*LATM_MAX_LAYER];

    int nOtherData;
    int nCrc;  /* -1 if not set */
} latm_mux_t;

static const int Sample_rates[16] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,  7350,  0,     0,     0
};


int LOASSyncInfo( uint8_t ptHeader[LOAS_HEADER_SIZE] );

int Mpeg4GAProgramConfigElement( BitStreamInfo *bsi );
int Mpeg4GASpecificConfig( mpeg4_cfg_t *p_cfg, BitStreamInfo *bsi );
int Mpeg4ReadAudioObjectType( BitStreamInfo *bsi );
int Mpeg4ReadAudioSamplerate( BitStreamInfo *bsi,AACDecInfo *aacDecInfo, int updateIdx);
int Mpeg4ReadAudioSpecificInfo( mpeg4_cfg_t *p_cfg, int *pi_extra, uint8_t *p_extra, BitStreamInfo *bsi, int i_max_size,AACDecInfo *aacDecInfo );
int LatmGetValue( BitStreamInfo *bsi );
int LatmReadStreamMuxConfiguration( latm_mux_t *m, BitStreamInfo *s,AACDecInfo *aacDecInfo );
int LOASParse(HAACDecoder hAACDecoder, uint8_t *p_buffer, int i_buffer );




/*
 * Voice Removal:
 *
 * It removes the mono channel on stereo sounds. Most singers record their
 * voice on the mono channel, so, if you remove it, you can remove its voice.
 *
 * The bad thing is that the voice cannot be removed if it is not recorded
 * at the mono channel, another bad thing is that any other sound that lies
 * in the mono channel will be removed also. To remove the center channel I
 * add the inverted left to the right and vice-versa, using that, all the
 * sound that is common to both channels is cancelled out!.
 *
 * The voice removal algorithm get from DeFX, which is the plug-in for XMMS,
 * it adds a filter to compensate the mono sound.
 *
 */

#include "aacdec.h"
#include "coder.h"

#if defined(VOICEOFF)

extern AACFrameInfo aacFrameInfo;

///////////////////////////////////////////////////////////////////////////
// Data structure for filtered voice removal
///////////////////////////////////////////////////////////////////////////
#if defined(FILTER_VOICEOFF)
#include "assembly.h"

#define FILTER_PRECESION 12     // The precesion of filter.
#define LEVEL_PRECESION   8     // The precesion of level.

#define MONOLEVEL (int)(0.4*(1<<LEVEL_PRECESION))       // Filtered mono level
#define LEVEL     (int)(0.8*(1<<LEVEL_PRECESION))       // Level
#define Width     100                                   // Bandwidth
#define Band      440                                   // Center Freq.

static const int sampRate[]   = {
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};

/**************************************************************************
static void filterGen(void){
    int i;
    double A, B, C;
    printf("static const int filter[%d][3] = {\n", sizeof(sampRate)/sizeof(int));
    for (i=0; i<sizeof(sampRate)/sizeof(int); i++) {
        int SAMPLERATE = sampRate[i];

        C=exp(-2*PI*Width/SAMPLERATE);
        B=-4*C/(1+C)*cos(2*PI*Band/SAMPLERATE);
        A=sqrt(1-B*B/(4*C))*(1-C);

        filter[i][0] = (int)(A*(1<<FILTER_PRECESION));
        filter[i][1] = (int)(B*(1<<FILTER_PRECESION));
        filter[i][2] = (int)(C*(1<<FILTER_PRECESION));

        printf("    {0x%08x, 0x%08x, 0x%08x}, // %5dHz, %f, %f, %f\n",
               filter[i][0], filter[i][1], filter[i][2],
               SAMPLERATE, A, B, C);
    }
    printf("};\n");
}
**************************************************************************/
static const int filter[9][3] = {
    {0x00000069, 0xffffe313, 0x00000eca}, //  8000Hz, 0.025738, -1.807903, 0.924465
    {0x00000038, 0xffffe1e3, 0x00000f1d}, // 11025Hz, 0.013831, -1.882256, 0.944603
    {0x00000030, 0xffffe1aa, 0x00000f2f}, // 12000Hz, 0.011721, -1.896193, 0.948987
    {0x0000001b, 0xffffe119, 0x00000f62}, // 16000Hz, 0.006663, -1.931538, 0.961491
    {0x0000000e, 0xffffe0b5, 0x00000f8c}, // 22050Hz, 0.003535, -1.956031, 0.971907
    {0x0000000c, 0xffffe0a1, 0x00000f96}, // 24000Hz, 0.002989, -1.960741, 0.974160
    {0x00000006, 0xffffe06f, 0x00000fb0}, // 32000Hz, 0.001688, -1.972980, 0.980557
    {0x00000003, 0xffffe04b, 0x00000fc6}, // 44100Hz, 0.000892, -1.981852, 0.985853
    {0x00000003, 0xffffe044, 0x00000fca}, // 48000Hz, 0.000753, -1.983616, 0.986995
};

static int y1, y2;
static int A, B, C;
#endif // defined(FILTER_VOICEOFF)

void voiceOff(short *buf)
{

#if defined(SIMPLE_VOICE_OFF)
    int i;
    short int left, right;
#elif defined(FILTER_VOICEOFF)
    int i, x, y, o;
    int left, right;
#else
#error "No mode select for voice off"
#endif // defined(SIMPLE_VOICEOFF) or defined(FILTER_VOICEOFF)

    if(aacFrameInfo.nChans !=2){
        return;
    }

///////////////////////////////////////////////////////////////////////////
// simple voice off
///////////////////////////////////////////////////////////////////////////
#if defined(SIMPLE_VOICE_OFF)
    for(i=0; i<aacFrameInfo.outputSamps; i+=2) {
        left  = buf[i  ];
        right = buf[i+1];
        buf[i  ] = left  - right;
        buf[i+1] = right - left;
    }
///////////////////////////////////////////////////////////////////////////
// filterd voice off
///////////////////////////////////////////////////////////////////////////
#elif defined(FILTER_VOICEOFF)
    for(i=0; i<aacFrameInfo.outputSamps; i+=2) {
        // Get left and right inputs.
        left  = (int)buf[i];
        right = (int)buf[i+1];

        // Do filtering
        x  = (left + right) / 2;
        y  = (A*x - B*y1 - C*y2) >> FILTER_PRECESION;
        y2 = y1;
        y1 = y;

        // Filter mono signal
        o  = (y * MONOLEVEL)>>LEVEL_PRECESION;

        // Cut the center
        buf[i  ] = CLIPTOSHORT(left  - ((right * LEVEL) >> LEVEL_PRECESION) + o);
        buf[i+1] = CLIPTOSHORT(right - ((left  * LEVEL) >> LEVEL_PRECESION) + o);
    }
#endif // defined(SIMPLE_VOICEOFF) or defined(FILTER_VOICEOFF)
}

void initVoiceOff(int sample_rate) {
#if defined(FILTER_VOICEOFF)
    int i;

    y1 = y2 = 0;

    for(i=0; i<sizeof(sampRate)/sizeof(int); i++) {
        if (sample_rate <= sampRate[i]) break;
    }

    if (i >= sizeof(sampRate)/sizeof(int)) {
        i--;
    }

    A = filter[i][0];
    B = filter[i][1];
    C = filter[i][2];

#endif // defined(FILTER_VOICEOFF)
}

#endif // defined(VOICEOFF)


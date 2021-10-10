#ifndef __RESAMPLE_H__
#define __RESAMPLE_H__


//#define WIN32
#define PURE_C

#define LOOKUP_TABLE
#define RESAMPLE_FILTER_LENGTH  16

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ?(x):(y))
#endif

#ifndef ABS
#define ABS(x)   ((x)<0   ?(-(x)):(x))
#endif

#ifndef SGN
#define SGN(x)   ((x)<0   ?(-1):((x)==0?(0):(1)))
#endif

typedef short          HWORD;
typedef unsigned short UHWORD;
typedef int            WORD;
typedef unsigned int   UWORD;

#if defined(WIN32)
    typedef __int64 int64;
    typedef unsigned __int64 uint64;
#else
    typedef long long int64;
    typedef unsigned long long uint64;
#endif

#define MAX_HWORD (32767)
#define MIN_HWORD (-32768)

#define MAX_NCHAN 2
#define MAX_FRAMESIZE 1024
#define TEMP_BUFFER_SIZE MAX_FRAMESIZE*8

typedef struct AVResampleContext{
    //const AVClass *av_class;
#ifdef LOOKUP_TABLE
    signed short *filter_bank; //MAX(22,16) * (1024+1)
#else
    signed short filter_bank[11275*2]; //MAX(22,16) * (1024+1) //11,8
#endif

    int filter_length;
    int ideal_dst_incr;
    int dst_incr;
    int index;
    int frac;
    int src_incr;
    int compensation_distance;
    int phase_shift;
    int phase_mask;
    int linear;
}AVResampleContext;

typedef struct ResampleAudioContext{
    int nKeep[2]; // keep last resample pcm
    int nAudioType; // audio plugin type ,1: mp3,2:wav
    int nInSampleRate;
    int nOutSampleRate;
    int nInSize;
    int nInChannels;
    int nInBitsPerSample; // input bits per sample
    int nResampleSize; // resample pcm size
    int nOutputIndex; // output pcm index
    int nUseTempBuffer;
    short nTempBuffer[TEMP_BUFFER_SIZE];
    int nTempBufferRdPtr;
    int nTempBufferWrPtr;
    int nTempBufferLength; // short
    short reSamplePcmInput[2][MAX_FRAMESIZE+100];
    short reSamplePcmOutput[2][MAX_FRAMESIZE*8];
    short outputPcmBuf[MAX_NCHAN * MAX_FRAMESIZE*8];
}ResampleAudioContext;

void *av_resample_init(AVResampleContext *avResampleContext,int out_rate, int in_rate, int filter_size, int phase_shift, int linear, double cutoff);
int av_resample(AVResampleContext *c, short *dst, short *src, int *consumed, int src_size, int dst_size, int update_ctx);

#if defined(PURE_C)
static __inline int MULSHIFT32(int x, int y)
{
    int64 xext,yext;
    xext =(int64)x; yext=(int64)y;
    yext=xext*yext;
    return (int)(yext>>32);
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#elif defined(WIN32) || defined(__CYGWIN__)


static __inline int ADD(int x, int y)
{
    int z = x+y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx+fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif

    return z;
}

static __inline int SUB(int x, int y)
{
    int z = x-y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx-fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif
    return z;
}

static __inline int MULSHIFT(int x, int y, int shift)
{
    int64 xext, yext;
    xext=(int64)x;  yext=(int64)y;
    xext = (xext * yext)>>shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx, fy, diff;
    fx=x; fy = y;
    fx = (fx*fy)*g_ovflw_shift_coef[shift];
    diff = (double)xext - fx;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)xext;
}

static __inline int MULSHIFT32(int x, int y)
{
    int64 xext,yext;
    xext =(int64)x; yext=(int64)y;
    xext=(xext*yext)>>32;

#if defined(OVERFLOW_DEBUG)
    {
    double fx, fy, diff;
    fx=x; fy = y;
    fx = (fx*fy)*g_ovflw_shift_coef[32];
    diff = (double)xext - fx;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)xext;
}

static __inline int MADDSHIFT(int x1, int y1, int x2, int y2, int shift)
{
    int64 ex1, ey1, ex2, ey2;
    ex1=(int64)x1;  ey1=(int64)y1;
    ex2=(int64)x2;  ey2=(int64)y2;

    ex1 = ex1 * ey1;
    ex2 = ex2 * ey2;
    ex1 = (ex1 + ex2) >> shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx1, fy1, fx2, fy2, diff;
    fx1=x1; fy1 = y1; fx2=x2; fy2 = y2;
    fx1 = ((fx1*fy1)+(fx2*fy2))*g_ovflw_shift_coef[shift];
    diff = (double)ex1 - fx1;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)ex1;
}

static __inline int MSUBSHIFT(int x1, int y1, int x2, int y2, int shift)
{
    int64 ex1, ey1, ex2, ey2;
    ex1=(int64)x1;  ey1=(int64)y1;
    ex2=(int64)x2;  ey2=(int64)y2;

    ex1 = ex1 * ey1;
    ex2 = ex2 * ey2;
    ex1 = (ex1 - ex2) >> shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx1, fy1, fx2, fy2, diff;
    fx1=x1; fy1 = y1; fx2=x2; fy2 = y2;
    fx1 = ((fx1*fy1)-(fx2*fy2))*g_ovflw_shift_coef[shift];
    diff = (double)ex1 - fx1;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)ex1;
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#elif defined(__OR32__)

//#define OVERFLOW_DEBUG

#define SETACC64(lo, hi)   ({  \
    asm volatile ("l.mtspr r0, %0, 0x2801" : : "r"(lo)); \
    asm volatile ("l.mtspr r0, %0, 0x2802" : : "r"(hi)); \
})

#define WRACC64_ADD(x, y)   ({  \
    asm volatile ("l.mac %0, %1"::"%r" (x), "r"(y));  \
})

#define WRACC64_SUB(x, y)   ({  \
    asm volatile ("l.msb %0, %1"::"%r" (x), "r"(y));  \
})

#define RDACC64_SHIFT(shift)   ({  \
    register int ret;	\
    asm volatile ("l.macrc %0, %1" : "=r"(ret) : "i"(shift));	\
	ret;	\
})

static __inline int ADD(int x, int y)
{
    int z = x+y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx+fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif

    return z;
}

static __inline int SUB(int x, int y)
{
    int z = x-y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx-fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif
    return z;
}

#define MULSHIFT(x, y, shift)   ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x), "r"(y));   \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

#define MADDSHIFT(x1, y1, x2, y2, shift)    ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x1), "r"(y1)); \
    asm volatile ("l.mac %0,%1" : : "%r"(x2), "r"(y2)); \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

#define MSUBSHIFT(x1, y1, x2, y2, shift)    ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x1), "r"(y1)); \
    asm volatile ("l.msb %0,%1" : : "%r"(x2), "r"(y2)); \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

static __inline int MULSHIFT32(int x, int y)
{
    asm volatile ("l.mac %0,%1" : : "%r"(x), "r"(y));
    asm volatile ("l.macrc %0,32" : "=r"(y));
    return y;
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#else

#error Unsupported platform

#endif  /* platforms */


#endif  /* __RESAMPLE_H__ */

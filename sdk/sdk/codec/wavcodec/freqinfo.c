
#include "wav_config.h"
#include <stdlib.h>
#include "plugin.h"

#ifndef abs
#define abs(x)  (((x)>=0) ? (x):-(x))
#endif

/* macros */
#define TWO_PI (6.2831853071795864769252867665590057683943L)

#define N_WAVE      1024    /* full length of Sinewave[] */ 
#define LOG2_N_WAVE 10      /* log2(N_WAVE) */

typedef short int16_t;

#define USE_FREQ_FUNC 0

static int tab[FREQINFOCNT+1] = 
{
#if (USE_FREQ_FUNC == 0)
  1,   25,   50,  75,  100,  130,  155,  180,  205, 235, 270, 300, 325, 350, 375, 405,
425, 450, 480, 500, 510,
#elif (USE_FREQ_FUNC == 1)
 // by equation 0.7 * (N+1)^(2.1) + 0.5, N = 0 ... 20
    1,   3,   7,  13,  21,  30,  42,  55,  71,  88, 108, 129, 153, 179, 206, 236,
  269, 303, 339, 378, 419,
#endif  
};

#define USE_SCALE_FUNC 0

#if (USE_SCALE_FUNC == 0)
// Scale function 0: No scale
static unsigned char scale[] = {
   0,   1,  2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};
#elif (USE_SCALE_FUNC == 1)
// Scale function 1: ((i==0) ? 0 : (int)(log(i)*256/log(256)))
static unsigned char scale[] = {
   0,   0,  32,  50,  64,  74,  82,  89,  95, 101, 106, 110, 114, 118, 121, 125,
 128, 130, 133, 135, 138, 140, 142, 144, 146, 148, 150, 152, 153, 155, 157, 158,
 160, 161, 162, 164, 165, 166, 167, 169, 170, 171, 172, 173, 174, 175, 176, 177,
 178, 179, 180, 181, 182, 183, 184, 185, 185, 186, 187, 188, 189, 189, 190, 191,
 191, 192, 193, 194, 194, 195, 196, 196, 197, 198, 198, 199, 199, 200, 201, 201,
 202, 202, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210,
 210, 211, 211, 212, 212, 213, 213, 213, 214, 214, 215, 215, 216, 216, 217, 217,
 217, 218, 218, 219, 219, 219, 220, 220, 221, 221, 221, 222, 222, 222, 223, 223,
 223, 224, 224, 225, 225, 225, 226, 226, 226, 227, 227, 227, 228, 228, 228, 229,
 229, 229, 230, 230, 230, 231, 231, 231, 231, 232, 232, 232, 233, 233, 233, 234,
 234, 234, 234, 235, 235, 235, 236, 236, 236, 236, 237, 237, 237, 237, 238, 238,
 238, 238, 239, 239, 239, 239, 240, 240, 240, 241, 241, 241, 241, 241, 242, 242,
 242, 242, 243, 243, 243, 243, 244, 244, 244, 244, 245, 245, 245, 245, 245, 246,
 246, 246, 246, 247, 247, 247, 247, 247, 248, 248, 248, 248, 249, 249, 249, 249,
 249, 250, 250, 250, 250, 250, 251, 251, 251, 251, 251, 252, 252, 252, 252, 252,
 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255
};
#endif



/*
  Since we only use 3/4 of N_WAVE, we define only
  this many samples, in order to conserve data space.
*/
short Sinewave[N_WAVE-N_WAVE/4] = {
      0,    201,    402,    603,    804,   1005,   1206,   1406,
   1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
   3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
   4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
   6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
   7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
   9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
  11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
  12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
  14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
  15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
  16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
  18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
  19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
  20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
  22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
  23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
  24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
  25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
  26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
  27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
  28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
  28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
  29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
  30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
  30851,  30918,  30984,  31049,  31113,  31175,  31236,  31297,
  31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
  31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
  32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
  32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
  32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
  32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
  32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
  32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
  32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
  32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
  32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
  31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
  31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
  30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
  30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
  29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
  28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
  28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
  27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
  26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
  25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
  24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
  23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
  22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
  20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
  19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
  18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
  16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
  15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
  14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
  12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
  11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
   9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
   7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
   6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
   4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
   3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
   1607,   1406,   1206,   1005,    804,    603,    402,    201,
      0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
  -1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
  -3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
  -4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
  -6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
  -7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
  -9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
 -11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
 -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
 -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
 -15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
 -16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
 -18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
 -19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
 -20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
 -22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
 -23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
 -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
 -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
 -26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
 -27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
 -28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
 -28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
 -29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
 -30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
 -30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
 -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
 -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
 -32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
 -32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
 -32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
 -32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
};


char freqinfo[FREQINFOCNT];
static int gnReadSize; 
static int gnIsShowSpectrum; // 32k,44.1k, 48k use ,reduce loading 
static int gnCount;
static int gnMode;

static short* ptFFT;           /* pointer to frequency-domain samples */

static  short wavENDIAN_LE16(unsigned char *n) 
{
    short num = (short)(((unsigned short)n[0]) + ((unsigned short)n[1] << 8));

    return (num);
}


/*
bool isPowerOfTwo(int n)
{
    return ((n>0) && ((n&(n-1))==0));
}

//log2 of exact powers of two
int log2(int n)
{
    int i=-1;
    while (n>0) {
        n >>= 1;
        i++;
    }
    return i;
}
*/

/*
  FIX_MPY() - fixed-point multiplication & scaling.
  Substitute inline assembly for hardware-specific
  optimization suited to a particluar DSP processor.
  Scaling ensures that result remains 16-bit.
*/
inline short FIX_MPY(short a, short b)
{
        /* shift right one less bit (i.e. 15-1) */
        int c = ((int)a * (int)b) >> 14;
        /* last bit shifted out = rounding-bit */
        b = c & 0x01;
        /* last shift + rounding bit */
        a = (c >> 1) + b;
        return a;
}

/*
  fix_fft() - perform forward/inverse fast Fourier transform.
  fr[n],fi[n] are real and imaginary arrays, both INPUT AND
  RESULT (in-place FFT), with 0 <= n < 2**m; set inverse to
  0 for forward transform (FFT), or 1 for iFFT.
*/
int fix_fft(short fr[], short fi[], short m, short inverse)
{
        int mr, nn, i, j, l, k, istep, n, scale, shift;
        short qr, qi, tr, ti, wr, wi;

        n = 1 << m;

        /* max FFT size = N_WAVE */
        if (n > N_WAVE)
                return -1;

        mr = 0;
        nn = n - 1;
        scale = 0;

        /* decimation in time - re-order data */
        for (m=1; m<=nn; ++m)
        {
                l = n;
                do 
                {
                    l >>= 1;
                } while (mr+l > nn);
                mr = (mr & (l-1)) + l;

                if (mr <= m)
                    continue;
                tr = fr[m];
                fr[m] = fr[mr];
                fr[mr] = tr;
                ti = fi[m];
                fi[m] = fi[mr];
                fi[mr] = ti;
        }

        l = 1;
        k = LOG2_N_WAVE-1;
        while (l < n) 
        {
                if (inverse) 
                {
                        /* variable scaling, depending upon data */
                        shift = 0;
                        for (i=0; i<n; ++i) 
                        {
                                j = fr[i];
                                if (j < 0)
                                        j = -j;
                                m = fi[i];
                                if (m < 0)
                                        m = -m;
                                if (j > 16383 || m > 16383) 
                                {
                                        shift = 1;
                                        break;
                                }
                        }
                        if (shift)
                                ++scale;
                }
                else
                {
                        /*
                          fixed scaling, for proper normalization --
                          there will be log2(n) passes, so this results
                          in an overall factor of 1/n, distributed to
                          maximize arithmetic accuracy.
                        */
                        shift = 1;
                }
                /*
                  it may not be obvious, but the shift will be
                  performed on each data point exactly once,
                  during this pass.
                */
                istep = l << 1;
                for (m=0; m<l; ++m) 
                {
                        j = m << k;
                        /* 0 <= j < N_WAVE/2 */
                        wr =  Sinewave[j+N_WAVE/4];
                        wi = -Sinewave[j];
                        if (inverse)
                                wi = -wi;
                        if (shift) 
                        {
                                wr >>= 1;
                                wi >>= 1;
                        }
                        for (i=m; i<n; i+=istep) 
                        {
                                j = i + l;
                                tr = FIX_MPY(wr,fr[j]) - FIX_MPY(wi,fi[j]);
                                ti = FIX_MPY(wr,fi[j]) + FIX_MPY(wi,fr[j]);
                                qr = fr[i];
                                qi = fi[i];
                                if (shift)
                                {
                                    qr >>= 1;
                                    qi >>= 1;
                                }
                                fr[j] = qr - tr;
                                fi[j] = qi - ti;
                                fr[i] = qr + tr;
                                fi[i] = qi + ti;
                        }
                }
                --k;
                l = istep;
        }
        return scale;
}

// mode:0 decode
// mode:1 encode
void freqInfoInitialize(int nSize,int nMode)
{
   gnReadSize = nSize/(sizeof(short)*WaveInfo.nChans);
   gnMode = nMode;
   /* Allocate time- and frequency-domain memory. */
   ptFFT = PalHeapAlloc(0,2*gnReadSize*sizeof(short));
   
   memset(ptFFT, 0, sizeof(*ptFFT));
   if( WaveInfo.sampRate == 32000 || WaveInfo.sampRate == 44100 || WaveInfo.sampRate == 48000)
   {
      gnIsShowSpectrum = 1;
      gnCount = 0;
//  printf("Wav ShowSpectrum = MMP_TRUE\n");
   }
   else
   {
      gnIsShowSpectrum = 0;
//  printf("Wav ShowSpectrum = MMP_FALSE\n");      
   }
  
  // printf("Wav freqInfo init  nSample %d sampling rate %d gnMode %d size of ptFFT %d %d \n",gnReadSize,WaveInfo.sampRate,gnMode,sizeof(*ptFFT),2*gnReadSize*sizeof(short));
}

void freqInfoTerminiate()
{

    if(ptFFT)
    {
        PalHeapFree(0,ptFFT);
    }
}

/*
  fix_fftr() - forward/inverse FFT on array of real numbers.
  Real FFT/iFFT using half-size complex FFT by distributing
  even/odd samples into real/imaginary arrays respectively.
  In order to save data space (i.e. to avoid two arrays, one
  for real, one for imaginary samples), we proceed in the
  following two steps: a) samples are rearranged in the real
  array so that all even samples are in places 0-(N/2-1) and
  all imaginary samples in places (N/2)-(N-1), and b) fix_fft
  is called with fr and fi pointing to index 0 and index N/2
  respectively in the original array. The above guarantees
  that fix_fft "sees" consecutive real samples as alternating
  real and imaginary samples in the complex array.
*/
int fix_fftr(short f[], int m, int inverse)
{
        int i, N = 1<<(m-1), scale = 0;
        short tt, *fr=f, *fi=&f[N];

        if (inverse)
        {
            scale = fix_fft(fi, fr, m-1, inverse);
        }
        for (i=1; i<N; i+=2) 
        {
            tt = f[N+i-1];
            f[N+i-1] = f[i];
            f[i] = tt;
        }
        if (! inverse)
        {
            scale = fix_fft(fi, fr, m-1, inverse);
        }
        return scale;
}

void updateFreqInfo(int nPCMBytes)
{
    int nSampleRate;
    int nFreqStep;   // sampling rate/N
    int nFreq;
    int i; // general index;    
    unsigned int nTemp;
    int nBlockSize;
    int m =10;  // sample size 512
    int N = 1<<(m-1);
    //PAL_CLOCK_T tClockFftPerformance;
    long nFftPerformance;  // avg of frame   
    short nTemp1,nTemp2 ;
    nSampleRate = WaveInfo.sampRate;
    nFreqStep = nSampleRate/gnReadSize;
    if(gnIsShowSpectrum == 1)
    {
        if(gnCount==3)
        {
            gnCount = 0;
            return;
        }
        gnCount++;
        if( gnCount==2 || gnCount ==3)
        {
            return;            
        }
    }
    
    if(nPCMBytes >= 2)
    {
        i = nPCMBytes;
        while(i==2*(i/2))
        {
           i = i/2;  /* While i is even, factor out a 2. */
        }
    }  /* For N >=2, we now have N = 2^n iff i = 1. */
    if(nPCMBytes < 2 || i != 1)
    {
        printf(", which does not equal 2^n for an integer n >= 1.");
        return;
    }
    nBlockSize = gnReadSize/20;

    if(gnMode ==0)
    {
        nTemp =pcmWriteIdx;
    }
    else
    {
        if(pcmReadIdx>nPCMBytes)
        {
            nTemp = pcmReadIdx-nPCMBytes;
        }
        else
        {
            nTemp =  sizeof(pcmWriteBuf)-(nPCMBytes-pcmReadIdx);
        }
    }
    
    for(i=0; i<gnReadSize; i++)
    {        
        if(WaveInfo.nChans==1)
        {
            ptFFT[i] = (short)wavENDIAN_LE16(&pcmWriteBuf[nTemp+i*2]);
            ptFFT[i+N]=0;        
            if(nTemp+i*2>= sizeof(pcmWriteBuf))
            {
                nTemp-=sizeof(pcmWriteBuf);
            }
        }
        else if (WaveInfo.nChans ==2)
        {
 
            nTemp1= (short)wavENDIAN_LE16(&pcmWriteBuf[nTemp+i*2*WaveInfo.nChans]);            
            nTemp2= (short)wavENDIAN_LE16(&pcmWriteBuf[nTemp+i*2*WaveInfo.nChans +2]);             
            ptFFT[i] = (nTemp1+nTemp2)/2;
            ptFFT[i+N]=0;                    
            if( (nTemp+i*2*WaveInfo.nChans) >= sizeof(pcmWriteBuf))
            {
                nTemp-=sizeof(pcmWriteBuf);
            }
            
        }
        
    }

//tClockFftPerformance = PalGetClock();
    /* Calculate FFT. */
    fix_fftr(ptFFT,  m, 0);
//printf("Wav Freqinfo fft performance %d m %d\n",PalGetDuration(tClockFftPerformance),m);      

    for(i=0; i<FREQINFOCNT; i++)
    {
        unsigned int f = 0;
        int j;
        for(j=tab[i]; j<tab[i+1]; j++) 
        {
            f += (short)abs(ptFFT[j]) + (short)abs(ptFFT[j+N]);
        }

        f=f>>10;
        // cliping the 'f' is between 0~255.
        if (f > 255) 
        {
            f = 255;
        }

        freqinfo[i] = scale[f];
    }

}


/***************************************************************
A.2 aec.cxx
***************************************************************/

/* aec.cxx
 * Acoustic Echo Cancellation NLMS-pw algorithm
 * Author: Andre Adrian, DFS Deutsche Flugsicherung
 * <Andre.Adrian@dfs.de>
 *
 * Version 1.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "defines.h"
#include "aec.h"

#ifdef OPRISCENG
#include "engine.h"
#endif

/* End 2005.05.30 */

/* dB Values */
//const float M3dB = 0.71f;
//const float M6dB = 0.50f;

/* dB values for 16bit PCM */
//const float M10dB_PCM = 10362.0f;
//const float M20dB_PCM = 3277.0f;
//const float M25dB_PCM = 1843.0f;
//const float M30dB_PCM = 1026.0f;
//const float M35dB_PCM = 583.0f;
//const float M40dB_PCM = 328.0f;
//const float M45dB_PCM = 184.0f;
//const float M50dB_PCM = 104.0f;
//const float M55dB_PCM = 58.0f;
//const float M60dB_PCM = 33.0f;

//const float MAXPCM = 32767.0f;

const int M10dB_PCM = 10362;
const int M20dB_PCM = 3277;
const int M25dB_PCM = 1843;
const int M30dB_PCM = 1026;
const int M35dB_PCM = 583;
const int M40dB_PCM = 328;
const int M45dB_PCM = 184;
const int M50dB_PCM = 104;
const int M55dB_PCM = 58;
const int M60dB_PCM = 33;

const int MAXPCM = 32767;

/* 2005.05.26 S034 */
#ifdef WIN32
//const float M_PI = 3.1415926535f;
#endif

/* Design constants (Change to fine tune the algorithms */

/* For Normalized Least Means Square - Pre-whitening */
// NLMS_LEN MOVE TO defines.h
//#define NLMS_LEN  (80*5)            /* NLMS filter length in taps */
//#define NLMS_LEN  (80*1)            /* NLMS filter length in taps */

//const float PreWhiteTransferFreq = 4000.0f;
const int PreWhiteTransferFreq = 4000;

/* for Geigel Double Talk Detector */
//const float GeigelThreshold = M6dB;
//const float GeigelThreshold = 0.50f;
//const int Thold = 30*8;                     /* DTD hangover in taps */
const int Thold = 30*8;                   /* DTD hangover in taps */
//const float UpdateThreshold = M50dB_PCM;
//const float UpdateThreshold = 104.0f;
const int UpdateThreshold = 104;
const int UpdateThresholdS = 10816;

/* for Non Linear Processor */
//const float NLPAttenuation = 0.50f;

    //IIR dc0, dc1;           // DC-level running average (IIR highpass)
    //IIR6 hp0;              // 300Hz cut-off Highpass
    //FIR1 Fx, Fe;            // pre-whitening Filter for x, e

    // Geigel DTD (Double Talk Detector)
    int max_max_x;                // max(|x[0]|, .. |x[L-1]|)
    int hangover;
    int max_x[NLMS_LEN/DTD_LEN];  // optimize: less calculations for max()
    int dtdCnt;
    int dtdNdx;

    // NLMS-pw
    //float x[NLMS_LEN+NLMS_EXT];     // tap delayed loudspeaker signal
    //float xf[NLMS_LEN+NLMS_EXT];    // pre-whitening tap delayed signal
    //float w[NLMS_LEN];              // tap weights
    int x[NLMS_LEN+NLMS_EXT];     // tap delayed loudspeaker signal
    int xf[NLMS_LEN+NLMS_EXT];    // pre-whitening tap delayed signal
    int w[NLMS_LEN];              // tap weights
    int j;                            // optimize: less memory copies
    int lastupdate;                   // optimize: iterative dotp(x,x)
    //float dotp_xf_xf;               // optimize: iterative dotp(x,x)
    int dotp_xf_xf;               // optimize: iterative dotp(x,x)

    /* 2005.06.27 S034 */
    unsigned int uiFragCount;
    int dwFragTime;
    int dwFrag[FRAG_LEN];
    /* End 2005.06.27 S034 */

    /* 2005.05.30 S034 */
    //float inputgain;
    int outputgain;
    int init;
    int dwMicMaxNoise;
    int dwSpkMaxNoise;
    int dwMicDC;
    int dwSpkDC;
    /* End 2005.05.30 S034 */

    // FIR
    int Fx_a[2], Fx_b1;
    int Fx_last[2];
    int Fe_a[2], Fe_b1;
    int Fe_last[2];
    // IIR
    int dc0_dwLowPassF;
    int dc1_dwLowPassF;
    //IIR6
    int hp0_dwLowPassF[2*POL+1];
    int hp0_dwHighPassF[2*POL+1];

/* ================================================================ */
/* Exponential Smoothing or IIR Infinite Impulse Response Filter */

void IIR6_init()
{
#if 0
    int i;
    for (i = 0; i < 2*POL+1; i++) {
        hp0_dwLowPassF[i] = 0;
        hp0_dwHighPassF[i] = 0;
    }
#else
    //memset(this, 0, sizeof(IIR6));
    memset(hp0_dwLowPassF, 0, sizeof(hp0_dwLowPassF));
    memset(hp0_dwHighPassF, 0, sizeof(hp0_dwHighPassF));
#endif
}

int IIR6_highpass(int in, int dwLowPassF[], int dwHighPassF[]) {
    ///* 2005.05.27 S034 */
    ////const float AlphaHp = 0.075;  /* controls Transfer Frequence */
    //const float AlphaHp = 0.075f; /* controls Transfer Frequence */
    ///* End 2005.05.27 S034 */
    //const float Gain6   = 1.45f;  /* gain to undo filter attenuation */
    ///* Highpass = Signal - Lowpass. Lowpass = Exponential Smoothing */
    //highpassf[0] = in;
    //int i;
    //for (i = 0; i < 2*POL; ++i) {
    //  lowpassf[i+1] += AlphaHp*(highpassf[i] - lowpassf[i+1]);
    //  highpassf[i+1] = highpassf[i] - lowpassf[i+1];
    //}
    //return (Gain6 * highpassf[2*POL]);
    int i;
    const int AlphaHp = 19;     // s15.8 = 0.075
                                    /* Controls Transfer Frequence */
    const int Gain6 = 371;      // s15.8 =  1.45
                                    /* gain to undo filter attenuation */
    dwHighPassF[0] = in << 8;

    for (i = 0; i < 2*POL; ++i) {
        dwLowPassF[i+1] += (AlphaHp * (dwHighPassF[i] - dwLowPassF[i+1])) >> 8;
        dwHighPassF[i+1] = dwHighPassF[i] - dwLowPassF[i+1];
    }
    return ((Gain6 * dwHighPassF[2*POL]) >> 16);
}

void IIR_init() {
    //lowpassf = 0.0f;
    dc0_dwLowPassF = 0;
    dc1_dwLowPassF = 0;
}

//float highpass(float in) {
int IIR_highpass(int in, int* dwLowPassF) {
    //const float ALPHADC = 0.01f;  /* controls Transfer Frequence */
    //
    //lowpassf += ALPHADC*(in - lowpassf);
    //return in - lowpassf;
    const int ALPHADC = 3;                          // sn.8 = 0.018
                                                        /* Controls Transfer Frequence */
    //int dwIn = in << 8;                               // s15 -> s15.8
    //dwLowPassF += (ALPHADC*(dwIn - dwLowPassF)) >> 8; // s15.8 = (s15.16 >> 8)
    //return (in - (dwLowPassF >> 8));                  // s15 = (s15.8 >> 8)

    (*dwLowPassF) += (ALPHADC*(in - (*dwLowPassF))) >> 8;       // s15 = (s15.8 >> 8)
    return (in - (*dwLowPassF));                            // s15
}

/* ================================================================ */

/*
 * Algorithm:  Recursive single pole FIR high-pass filter
 *
 * Reference: The Scientist and Engineer's Guide to Digital Processing
 */

void FIR1_init()
{
    ///* 2005.05.27 S034 */
    ////float x = exp(-2.0 * M_PI * PreWhiteTransferFreq/8000.0f);
    //float x = (float)exp(-2.0f * M_PI * PreWhiteTransferFreq/8000.0f);
    ///* End 2005.05.27 S034 */
    //a0 = (1.0f + x) / 2.0f;
    //a1 = -(1.0f + x) / 2.0f;
    //b1 = x;
    //last_in = 0.0f;
    //last_out = 0.0f;
    ///* 2005.05.27 S034 */
    //float x = exp(-2.0 * M_PI * PreWhiteTransferFreq/8000.0f);

    int x = 2832;       // s15.16: 0.04321391826 = 2832
    //a0 = 34184;           // s15.16
    //a1 = -34184;      // s15.16
    //b1 = x;
    //last_in = 0;
    //last_out = 0;
    Fx_a[0] = 34184;        // s15.16
    Fx_a[1] = -34184;       // s15.16
    Fx_b1 = x;
    Fx_last[0] = 0;     // last_in
    Fx_last[1] = 0;     // last_out
    Fe_a[0] = 34184;        // s15.16
    Fe_a[1] = -34184;       // s15.16
    Fe_b1 = x;
    Fe_last[0] = 0;     // last_in
    Fe_last[1] = 0;     // last_out
}

int FIR1_highpass(int in, int a[], int b1, int last[])  {
    //int out = (a0 * in + a1 * last_in + b1 * last_out) >> 16;
    // BAD
    // last_out * b1 maybe overflow

    //int out = (a[0] * in + a[1] * last[0] + ((b1 * last[1]) >> 8)) >> 8;      // a0, a1, b1: s15.16
    //last[0] = in;     // s15
    //last[1] = out;        // s15.8
    //return (out >> 8);    // s15
    int out = (a[0] * in + a[1] * last[0] + b1 * last[1]) >> 16;        // a0, a1, b1: s15.16
    last[0] = in;       // s15
    last[1] = out;      // s15.8
    return (out);   // s15

}

/* ================================================================ */

/* Vector Dot Product */
//float dotp(float a[], float b[]) {
//  float sum0 = 0.0, sum1 = 0.0;
//  int j;
//
//  for (j = 0; j < NLMS_LEN; j+= 2) {
//      // optimize: partial loop unrolling
//      sum0 += a[j] * b[j];
//      sum1 += a[j+1] * b[j+1];
//
//  }
//  return sum0+sum1;
//}
static __inline int dotp(int a[] /* s15 */) {

#ifdef OPRISCASM
    #ifdef OPRISCENG
    int sum;
    VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) a;
        //*(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) a;
        *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x3       << P_SAdd2) |
                                    (0x3       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_FIFOA << P_Mul0R) |
                                    (SRC_C0    << P_Mul1L) |
                                    (SRC_C0    << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

        *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (0          << P_Dst0 ) |
                                    (0          << P_Dst1 ) |
                                    (0          << P_Dst2 ) |
                                    (0          << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE        << P_Fire      )|
                                    (FALSE       << P_EnInitAcc )|
                                    (TRUE        << P_Sat       )|
                                    (DATA32      << P_RdDT      )|
                                    (DATA32      << P_WrDT      )|
                                    (LARGE       << P_RdGranSize)|
                                    (GRANULE_1   << P_RdGranule )|
                                    (NoDst       << P_WrGranSize)|
                                    (NONE        << P_WrGranule )|
                                    (NLMS_LEN    << P_Len       )|
                                    (0           << P_RdIncr    )|
                                    (0           << P_WrIncr    );
        WaitEngineDone();
        sum = *(volatile int *)ALU_P0;
    );
    return (sum);
    #else

    int jj;

    int sum0;
    for (jj = 0; jj < NLMS_LEN; jj+= 8) {
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj]), "r"(a[jj]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+1]), "r"(a[jj+1]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+2]), "r"(a[jj+2]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+3]), "r"(a[jj+3]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+4]), "r"(a[jj+4]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+5]), "r"(a[jj+5]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+6]), "r"(a[jj+6]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+7]), "r"(a[jj+7]));
    }
    asm volatile ("l.macrc %0, 0"           : "=r"(sum0));
    return (sum0);      /* s15 */
    #endif
#else
    int jj;
    int sum0 = 0;
    int sum1 = 0;
    for (jj = 0; jj < NLMS_LEN; jj+= 2) {
        // optimize: partial loop unrolling
        sum0 += a[jj] * a[jj];
        sum1 += a[jj+1] * a[jj+1];
    }
//#ifdef FILEOUT
//  macrc = fopen("macrc.dat", "a");
//  fprintf(macrc, "(Hi,Lo)=>(%08x, %08x) >> 00000000 = %08x\n", (sum0 >> 32), sum0, sum0);
//  fclose(macrc);
//#endif
    return (int)(sum0 + sum1);      /* s15 */
#endif
}

static __inline int dotp_25(int a[] /* sn.25 */, int b[] /* s15 */) {

#ifdef OPRISCASM
    #ifdef OPRISCENG
    int sum;
    VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) a;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) b;
        *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x3       << P_SAdd2) |
                                    (0x3       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_FIFOC << P_Mul0R) |
                                    (SRC_C0    << P_Mul1L) |
                                    (SRC_C0    << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

        *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (0          << P_Dst0 ) |
                                    (0          << P_Dst1 ) |
                                    (0          << P_Dst2 ) |
                                    (0          << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE        << P_Fire      )|
                                    (FALSE       << P_EnInitAcc )|
                                    (TRUE        << P_Sat       )|
                                    (DATA32      << P_RdDT      )|
                                    (DATA32      << P_WrDT      )|
                                    (LARGE       << P_RdGranSize)|
                                    (GRANULE_2   << P_RdGranule )|
                                    (NoDst       << P_WrGranSize)|
                                    (NONE        << P_WrGranule )|
                                    (NLMS_LEN    << P_Len       )|
                                    (0           << P_RdIncr    )|
                                    (0           << P_WrIncr    );
        WaitEngineDone();
        sum = *(volatile int *)ALU_P0;
    );
    return (sum >> 25);
    #else
    int jj;

    int sum0;
    for (jj = 0; jj < NLMS_LEN; jj+= 8) {
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj]), "r"(b[jj]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+1]), "r"(b[jj+1]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+2]), "r"(b[jj+2]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+3]), "r"(b[jj+3]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+4]), "r"(b[jj+4]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+5]), "r"(b[jj+5]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+6]), "r"(b[jj+6]));
        asm volatile ("l.mac %0, %1"            : : "r"(a[jj+7]), "r"(b[jj+7]));
    }
    //asm volatile ("l.macrc %0, 15"            : "=r"(sum0));
    asm volatile ("l.macrc %0, 0"           : "=r"(sum0));
    sum0 = sum0 >> 25;
    return (sum0);      /* s15 */
    #endif
#else
    int jj;
    int sum0 = 0;
    int sum1 = 0;
    //__int64 sum0 = 0;
    //__int64 sum1 = 0;
    for (jj = 0; jj < NLMS_LEN; jj+= 2) {
        // optimize: partial loop unrolling
        sum0 += a[jj]   * b[jj];
        sum1 += a[jj+1] * b[jj+1];
    }
#ifdef FILEOUT
//  hi = (unsigned int)(sum0 >> 32);
//  lo = (unsigned int)(sum0 & 0xFFFFFFFF);
//  macrc = fopen("macrc.dat", "a");
//  fprintf(macrc, "(Hi,Lo)=>(%08x, %08x) >> 00000019 = %08x\n", hi, lo, (sum0 >> 25 & 0xFFFFFFFF));
//  fclose(macrc);
#endif
    return (int)((sum0+sum1)>> 25);     /* s15.16 */
#endif
}

int ck_div_s (int var1, int var2)   /* Output: sn.15 */
{
    int var_out = 0;
    //Word16 iteration;
    int iteration;
    int L_num;
    int L_denom;

//    if ((var1 > var2) || (var1 < 0) || (var2 < 0) || (var2 == 0) || (var1 == 0) )
//    {
//        printf ("Division Error var1=%d  var2=%d\n", var1, var2);
//        abort(); /* exit (0); */
//    }
//    if (var2 == 0)
//    {
//        printf ("Division by 0, Fatal error \n");
//        abort(); /* exit (0); */
//    }
//    if (var1 == 0)
//    {
//        var_out = 0;
//    }
//    else
    {
        if (var1 == var2)
        {
            //var_out = MAX_16;
            //var_out = 0x7FFF;
            var_out = 0x8000;
        }
        else
        {
            L_num = (int)var1;
            L_denom = (int)var2;
            iteration = 0;
            do
            {
                var_out <<= 1;
                L_num <<= 1;

                if (L_num >= L_denom)
                {
                    L_num -= L_denom;
                    var_out ++;
                }
                iteration++;
            } while (iteration < 15);
        }
    }

//#if (WMOPS)
//    multiCounter[currCounter].div_s++;
//#endif
    return (var_out);
}

void AEC_init()
{
#if 0
    int i;
    for(i = 0; i < NLMS_LEN/DTD_LEN; i++) {
        max_x[i] = 0;
    }
    for(i = 0; i < NLMS_LEN+NLMS_EXT; i++) {
        x[i] = 0;
        xf[i] = 0;
    }
    for(i = 0; i < NLMS_LEN; i++) {
        w[i] = 0;
    }
    for(i = 0; i < FRAG_LEN; i++) {
        dwFrag[i] = 0;
    }
#else
    memset(max_x, 0, sizeof(max_x));
    memset(x, 0, sizeof(x));
    memset(xf, 0, sizeof(xf));
    memset(w, 0, sizeof(w));
    memset(dwFrag, 0, sizeof(dwFrag));
#endif

    max_max_x = 0;
    hangover = 0;
    dtdCnt = dtdNdx = 0;

    j = NLMS_EXT;
    lastupdate = 0;
    dotp_xf_xf = 0;

    /* 2005.05.30 S034 */
    outputgain = 65536;
    //inputgain = 1.0f;
    init = 0;
    dwMicMaxNoise = 0;
    dwSpkMaxNoise = 0;
    dwMicDC = 0;
    dwSpkDC = 0;
    uiFragCount = 0;
    dwFragTime = 0;
    /* End 2005.05.30 S034 */
}

/* Normalized Least Mean Square Algorithm pre-whitening (NLMS-pw)
 * The LMS algorithm was developed by Bernard Widrow
 * book: Widrow/Stearns, Adaptive Signal Processing, Prentice-Hall, 1985
 *
 * in mic: microphone sample (PCM as floating point value)
 * in spk: loudspeaker sample (PCM as floating point value)
 * in update: 0 for convolve only, 1 for convolve and update
 * return: echo cancelled microphone sample
 */
//float AEC::nlms_pw(float mic, float spk, int update)
//{
//  float d = mic;                    // desired signal
//  x[j] = spk;
//  xf[j] = Fx.highpass(spk);       // pre-whitening of x
//
//  // calculate error value (mic signal - estimated mic signal from spk signal)
//  float e = d - dotp(w, x + j);
//
//  if (update) {
//    float ef = Fe.highpass(e);    // pre-whitening of e
//
//    if (lastupdate) {
//      // optimize: iterative dotp(xf, xf)
//      dotp_xf_xf += (xf[j]*xf[j] - xf[j+NLMS_LEN-1]*xf[j+NLMS_LEN-1]);
//    } else {
//      dotp_xf_xf = dotp(xf+j, xf+j);
//    }
//
//    // calculate variable step size
//    float mikro_ef = 0.5f/dotp_xf_xf * ef;
//
//    // update tap weights (filter learning)
//    int i;
//    for (i = 0; i < NLMS_LEN; i += 2) {
//      // optimize: partial loop unrolling
//      w[i] += mikro_ef*xf[i+j];
//      w[i+1] += mikro_ef*xf[i+j+1];
//    }
//  }
//  lastupdate = update;
//
//  if (--j < 0) {
//    // optimize: decrease number of memory copies
//    j = NLMS_EXT;
//    memmove(x+j+1, x, (NLMS_LEN-1)*sizeof(float));
//    memmove(xf+j+1, xf, (NLMS_LEN-1)*sizeof(float));
//  }
//
//  return e;
//}
int nlms_pw(int mic, int spk, int update)
{
    //float d = mic;                    // desired signal
    int e;
    int fInverter;
    int mikro_ef;
    int d = mic;                        // s15.16: desired signal
    int i;
    x[j] = spk;
    //xf[j] = Fx.highpass(spk);         // pre-whitening of x
    xf[j] = FIR1_highpass(spk, Fx_a, Fx_b1, Fx_last);           // pre-whitening of x

    // calculate error value (mic signal - estimated mic signal from spk signal)
    /* 2005.06.27 S034 */
#ifdef SPEEDFRAG
    if (uiFragCount < FRAG_LEN) {
        dwFrag[uiFragCount] = dotp_25(w, x + j);        // dotp: s15.16
        e = d - dwFrag[uiFragCount];
        uiFragCount++;
    } else {
        e = d - dwFrag[uiFragCount - FRAG_LEN];
        uiFragCount++;
        if (uiFragCount == 2*FRAG_LEN) {
            uiFragCount = 0;
        }
    }
#else
    e = d - (dotp_25(w, x + j));        // dotp: s15.16
#endif

#ifdef DEPEAKMIPS
    dotp_xf_xf += (xf[j]*xf[j] - xf[j+NLMS_LEN-1]*xf[j+NLMS_LEN-1]);
#endif

    if (update) {
        //float ef = Fe.highpass(e);    // pre-whitening of e
        //int ef = Fe.highpass(e);      // pre-whitening of e

        /* Bugs for fix2float, e have to shift for well done */
        int ef = FIR1_highpass(e, Fe_a, Fe_b1, Fe_last);        // pre-whitening of e

#ifndef DEPEAKMIPS
        if (lastupdate) {
        // optimize: iterative dotp(xf, xf)
            dotp_xf_xf += (xf[j]*xf[j] - xf[j+NLMS_LEN-1]*xf[j+NLMS_LEN-1]);
        } else {
            dotp_xf_xf = dotp(xf+j);
        }
#endif

        // calculate variable step size
        //float mikro_ef = 0.5f/dotp_xf_xf * ef;

        //float fInverter = (1 / ((float)dotp_xf_xf)) * 32736;              // sn.15
        //int fInverter = (int)((1 / ((float)dotp_xf_xf)) * 32736);     // sn.15
        //fInverter = 32768 / dotp_xf_xf;                               // sn.15
        fInverter = ck_div_s((int)1, dotp_xf_xf);                   // sn.15
        mikro_ef = ef * ((int)fInverter);                           // sn.15

        //float fInverter = (1 / ((float)dotp_xf_xf)) * 33554432;           // sn.25
        //int mikro_ef = ef * ((int)fInverter) >> 1;                    // sn.25

#ifdef OPRISCENG
    VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) &xf[j];
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) w;
        *(volatile int *)UsrDefC0 = mikro_ef;
        *(volatile int *)Dst0Base = (FALSE << P_RdDec) | (int) w;

        *(volatile int *)(ALU_OP0+4) =
                                    (TRUE      << P_BP   ) |
                                    (0x4       << P_SAdd2) |
                                    (0x4       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_UDC0  << P_Mul0R) |
                                    (SRC_FIFOC << P_Mul1L) |
                                    (SRC_C1    << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

        *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (DST_P0     << P_Dst0 ) |
                                    (DST_NoWr   << P_Dst1 ) |
                                    (DST_NoWr   << P_Dst2 ) |
                                    (DST_NoWr   << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE        << P_Fire      )|
//                                    (FALSE       << P_EnINT     )|
                                    (FALSE       << P_EnInitAcc )|
                                    (FALSE       << P_Sat       )|
                                    (DATA32      << P_RdDT      )|
                                    (DATA32      << P_WrDT      )|
                                    (LARGE       << P_RdGranSize)|
                                    (GRANULE_2   << P_RdGranule )|
                                    (LARGE       << P_WrGranSize)|
                                    (GRANULE_1   << P_WrGranule )|
                                    (NLMS_LEN    << P_Len       )|
                                    (0           << P_RdIncr    )|
                                    (0           << P_WrIncr    );
        WaitEngineDone();
    );
#else
        // update tap weights (filter learning)
        for (i = 0; i < NLMS_LEN; i += 4) {
            // optimize: partial loop unrolling
            //w[i] += mikro_ef*xf[i+j];
            //w[i+1] += mikro_ef*xf[i+j+1];
            //w[i] += mikro_ef*xf[i+j] * 65536;
            //w[i+1] += mikro_ef*xf[i+j+1] * 65536;
            w[i] += (mikro_ef*xf[i+j]);
            w[i+1] += (mikro_ef*xf[i+j+1]);
            w[i+2] += (mikro_ef*xf[i+j+2]);
            w[i+3] += (mikro_ef*xf[i+j+3]);
        }
#endif
    }
    lastupdate = update;

    if (--j < 0) {
        // optimize: decrease number of memory copies
        j = NLMS_EXT;
#ifdef __OR32__
    #ifdef OPRISCENGMEM
        memmov32(x+j+1, x, NLMS_LEN-1);
        memmov32(xf+j+1, xf, NLMS_LEN-1);
    #else
        for(i = NLMS_LEN + j - 1; i > j; i--) {
            x[i] = x[i - j - 1];
            xf[i] = xf[i - j - 1];

        }
    #endif
#else
        memmove(x+j+1, x, (NLMS_LEN-1)*sizeof(int));
        memmove(xf+j+1, xf, (NLMS_LEN-1)*sizeof(int));
#endif
    }
    return e;
}

/* Geigel Double-Talk Detector
 *
 * in d: microphone sample (PCM as floating point value)
 * in x: loudspeaker sample (PCM as floating point value)
 * return: 0 for no talking, 1 for talking
 */
//int AEC::dtd(float d, float x)
//{
//  // optimized implementation of max(|x[0]|, |x[1]|, .., |x[L-1]|):
//  // calculate max of block (DTD_LEN values)
//  x = fabsf(x);
//  if (x > max_x[dtdNdx]) {
//    max_x[dtdNdx] = x;
//    if (x > max_max_x) {
//      max_max_x = x;
//    }
//  }
//  if (++dtdCnt >= DTD_LEN) {
//    dtdCnt = 0;
//    // calculate max of max
//    max_max_x = 0.0f;
//    for (int i = 0; i < NLMS_LEN/DTD_LEN; ++i) {
//      if (max_x[i] > max_max_x) {
//        max_max_x = max_x[i];
//      }
//    }
//    // rotate Ndx
//    if (++dtdNdx >= NLMS_LEN/DTD_LEN) dtdNdx = 0;
//    max_x[dtdNdx] = 0.0f;
//  }
//
//  // The Geigel DTD algorithm with Hangover timer Thold
//  if (fabsf(d) >= GeigelThreshold * max_max_x) {
//    hangover = Thold;
//  }
//
//  if (hangover) --hangover;
//
//  /* 2005.05.30 S034 */
//#ifdef NOISEDET
//  if (max_max_x < dwMicMaxNoise) {
//#else
//  if (max_max_x < UpdateThreshold) {
//#endif
//  /* 2005.05.30 S034 */
//    // avoid update with silence
//    return 1;
//  } else {
//    return (hangover > 0);
//  }
//}
int dtd(int d, int x)
{
    // optimized implementation of max(|x[0]|, |x[1]|, .., |x[L-1]|):
    // calculate max of block (DTD_LEN values)
    int i;
    //int dwX = abs(x);
    int dwX = x * x;
    if (dwX > max_x[dtdNdx]) {
        max_x[dtdNdx] = dwX;
        if (dwX > max_max_x) {
            max_max_x = dwX;
        }
    }

    if (++dtdCnt >= DTD_LEN) {
        dtdCnt = 0;
        // calculate max of max
        max_max_x = 0;
        for (i = 0; i < NLMS_LEN/DTD_LEN; ++i) {
            if (max_x[i] > max_max_x) {
                max_max_x = max_x[i];
            }
        }
        // rotate Ndx
        if (++dtdNdx >= NLMS_LEN/DTD_LEN)
            dtdNdx = 0;
        max_x[dtdNdx] = 0;
    }

    // The Geigel DTD algorithm with Hangover timer Thold
    //if (fabsf(d) >= GeigelThreshold * max_max_x) {
    //if (abs(d) >= (max_max_x >> 1)) {     // M6dB = 0.5
    // 2005.12.12 Cory  Tuning params: d from microphone, x from speaker's ref
    //if (d*d >= (max_max_x >> 2)) {        // M6dB = 0.5
    if (d*d >= (max_max_x >> 3)) {
        hangover = Thold;
    }

    if (hangover) --hangover;

    /* 2005.05.30 S034 */
#ifdef NOISEDET
    if (max_max_x < dwMicMaxNoise) {
    //if (max_max_x < dwMicMaxNoise * dwMicMaxNoise) {
#else
    //if (max_max_x < UpdateThreshold) {
    if (max_max_x < UpdateThresholdS) {
#endif
    /* 2005.05.30 S034 */
    // avoid update with silence
        return 1;
    } else {
        return (hangover > 0);
    }
}

int doAEC(int d, int x)
{
    //float s0 = (float)d;
    //float s1 = (float)x;
    int s0 = d;
    int s1 = x;
    int update;

    // Mic and Spk signal remove DC (IIR highpass filter)
    //s0 = dc0.highpass(s0);
    //s1 = dc1.highpass(s1);
    s0 = IIR_highpass(s0, &dc0_dwLowPassF);
    s1 = IIR_highpass(s1, &dc1_dwLowPassF);

    /* 2005.05.30 S034
        Noise Detection
    */
#ifdef NOISEDET
    if (init < 800) {
            if (s0 > dwMicMaxNoise) {
                dwMicMaxNoise = s0;
            }
            if (s1 > dwSpkMaxNoise) {
                dwSpkMaxNoise = s1;
            }
            //micmaxnoise += 0.00125 * s0;
            //spkmaxnoise += 0.00125 * s1;
        init++;
    } else {
    }
#endif
    /* End 2005.05.30 S034 */

    // Mic Highpass Filter - telephone users are used to 300Hz cut-off
    //s0 = hp0.highpass(s0);
    s0 = IIR6_highpass(s0, hp0_dwLowPassF, hp0_dwHighPassF);

    // Double Talk Detector
    //int update = !dtd((float)s0, (float)s1);
    update = !dtd(s0, s1);
    // 2005.05.27 S034
    //  Cancel DTD, always update!
    //update = 1;
    // End 2005.05.27 S034

    // Acoustic Echo Cancellation
    s0 = nlms_pw(s0, s1, update);

    // Saturation
    /* 2005.05.26 S034 */
#ifdef OUTGAINCONTROL
#ifdef NOISEDET
    if (s0 < dwMicMaxNoise * 1 && s0 > - dwMicMaxNoise * 1) {
    //if (s0 < M30dB_PCM && s0 > -M30dB_PCM ) {
    //if (s0 < 1000 && s0 > -1000 ) {
#else
    if (s0 < M30dB_PCM && s0 > -M30dB_PCM ) {
    //if (s0 < M35dB_PCM && s0 > -M35dB_PCM ) {
#endif
        if (update) {
            //if (outputgain >= 0.001f) {
            //  outputgain = outputgain / 1.005f;
            //}
            if (outputgain > 4) {                           // OutputGain s1.16
                                                            /* OutputGain can't less than 4!!! Otherwise,
                                                               OutputGain * 1331 >> 10 will always be 1 */
                outputgain = (outputgain * 1018) >> 10;     // s1.10 1018 = 1/1.005
            }
        }
        //outputgain = 0;
    } else {
        if (!update) {
            //if (outputgain < 1) {
            //  outputgain = outputgain * 1.5;
            //}
            if (outputgain < 65536) {                       // OutputGain s1.16
                //outputgain = (outputgain * 1536) >> 10;   // s1.10 1536 = 1.5
                outputgain = (outputgain * 1331) >> 10;     // s1.30 1331 = 1.3
            } else {
                outputgain = 65536;
            }
        }
        //outputgain = 1;
    }
    s0 = (s0 * outputgain) >> 16;       // s15 * s1.16 >> 16 = s15
#else
    // Acoustic Echo Suppression
    if (update) {
        // Non Linear Processor (NLP): attenuate low volumes
        //s0 *= NLPAttenuation;
        s0 = s0 >> 1;
    }

#endif

    //if (s0 > MAXPCM) {
    //  return (int)MAXPCM;
    //} else if (s0 < -MAXPCM) {
    //  return (int)-MAXPCM;
    //} else {
    //  //return (int)roundf(s0);
    //  return (int)(s0);
    //}
    if (s0 > 32767) {
        return 32767;
    } else if (s0 < -32768) {
        return -32768;
    } else {
        //return (int)roundf(s0);
        return s0;
    }

}

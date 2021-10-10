/***************************************************************
A.1 aec.h
***************************************************************/

#ifndef _AEC_H  /* include only once */

/* aec.h
 * Acoustic Echo Cancellation NLMS-pw algorithm
 * Author: Andre Adrian, DFS Deutsche Flugsicherung
 * <Andre.Adrian@dfs.de>
 *
 * Version 1.1
 */

/* Exponential Smoothing or IIR Infinite Impulse Response Filter */
/* 2005.06.06 S034 Float2Fix */
//class IIR {
    //float lowpassf;
    //int dwLowPassF;

//public:
    void IIR_init();

//float highpass(float in) {
    int IIR_highpass(int in, int* dwlowpassf);

//};
/* End 2005.06.06 S034 */

#define POL     6       /* -6dB attenuation per octave per Pol */

//class IIR6 {
    //float lowpassf[2*POL+1];
    //float highpassf[2*POL+1];
    //int dwLowPassF[2*POL+1];
    //int dwHighPassF[2*POL+1];

//public:
    void IIR6_init();
    int IIR6_highpass(int in, int dwLowPassF[], int dwHighPassF[]);

    //float lowpass(float in)
    //{
    //  const float AlphaLp = 0.15f;    /* controls Transfer Frequence */
    //
    //  /* Lowpass = Exponential Smoothing */
    //  lowpassf[0] = in;
    //  int i;
    //  for (i = 0; i < 2*POL; ++i) {
    //      lowpassf[i+1] += AlphaLp*(lowpassf[i] - lowpassf[i+1]);
    //  }
    //  return lowpassf[2*POL];
    //}

//};

/* Recursive single pole FIR Finite Impule response filter */
//class FIR1 {
    //float a0, a1, b1;
    //float last_in, last_out;
    //int a0, a1, b1;
    //int last_in, last_out;

//public:
    void FIR1_init();
    int FIR1_highpass(int in, int a[], int b, int last[]);

    //float highpass(float in)  {
    //  float out = a0 * in + a1 * last_in + b1 * last_out;
    //  last_in = in;
    //  last_out = out;
    //  return out;
    //}

    // BAD

//};

#define NLMS_EXT  (10*8)    // Extention in taps to optimize mem copies
#define DTD_LEN 16          // block size in taps to optimize DTD calculation

//class AEC {
    // Time domain Filters

//public:
    void AEC_init();
    //int dtd(float d, float x);
    int dtd(int d, int x);
    //float nlms_pw(float mic, float spk, int update);
    int nlms_pw(int mic, int spk, int update);

/* Acoustic Echo Cancellation and Suppression of one sample
 * in   s0: microphone signal with echo
 * in   s1: loudspeaker signal
 * return:  echo cancelled microphone signal
 */
    int doAEC(int d, int x);
//};

#define _AEC_H
#endif

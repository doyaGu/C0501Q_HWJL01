/*
 * hd_aec.h
 *
 *  Created on: 2015¦~9¤ë21¤é
 *      Author: ite01527
 */
#include "type_def.h"
#include "webrtc_lib/include/ring_buffer.h"

#ifndef HD_AEC_H_
#define HD_AEC_H_
#define CHANNEL_NUM 1
#define FRAME_LEN 128
#define nDFT 256
#define part_len (nDFT >> 1)
#define part_len1 (part_len + 1)
#define part_len2 (part_len << 1)
#define part_len4 (part_len << 2)
#define PART_LEN 256
#define PART_LEN1 (PART_LEN + 1)
#define PART_LEN2 (PART_LEN << 1)
#define PART_LEN4 (PART_LEN << 2)
#define BARK_LEN 22
#define M 4 // tail-length 64ms
#define Q 3 // tail-length 256/16*3=48ms
#define RR 128 // analysis hopsize
#define SUPGAIN_DEFAULT 4
#define Noise_Floor 33
//#define DelayEst//compound instructions for if else statements

typedef struct {
	Word16 a1, a2;
	Word16 b1, b2;
	Word16 s1, s2;
} biquad;

typedef struct {
	Word16 alpha_m;
	Word16 BlkExp2;
	Word16 BlkExp1;
	Word16 currentVADvalue;
	Word16 nlpFlag;
	Word16 buffer_rrin[nDFT];
	Word16 buffer_ssin[nDFT];
	Word16 buffer_olpa[nDFT - RR];
	Word16 qDomainOld[M][part_len1];
	Word16 xfaQDomainOld[M][part_len1];
	Word16 dfaQDomainOld[M];
	Word16 vrfaQDomainOld[M][part_len1];
	Word16 vifaQDomainOld[M][part_len1];
	ComplexInt16 XF16m[M][part_len1];
	ComplexInt16 y16k[part_len];
	ComplexInt32 Yk[part_len];
	ComplexInt32 XFm[M][part_len1];
	Word32 GAINNO1[part_len1];
	Word32 GAMMANO1[part_len1];
	Word32 Gainno1[part_len1]; // for residual echo estimations
	Word32 Gainno2[part_len1]; // for roughly speech estimations
	Word32 gammano1[part_len1]; // post snr over echo-free near
	Word32 gammano2[part_len1]; // post snr over residual echo
	Word32 S32_ss[M][part_len1];
	Word32 S32_vs[M][part_len1 << 1];
	Word32 S32_ee[part_len1];
	Word32 S32_nn[part_len1];
	Word32 near32Filt[part_len1];
	Word32 echoEst32Gained[part_len1];
	Word32 LAMBDA[part_len1];
	Word32 noiseEst[part_len1];
	Word64 S_ss[M][part_len1]; // S_ss=&S_ss[0], Word64 (* const S_ss)[1+nDFT/2]
	Word64 S_vs[M][part_len1 << 1]; // S_ss+(1=sizeof(Word64)*part_len1)
	Word64 S_nn[part_len1]; // comfort noise
	Word64 S_ee[part_len1]; // error signals
	Word64 lambda[part_len1];
	Word64 nearFilt[part_len1]; // filtered near-end magnitude
	Word64 echoEst64Gained[part_len1]; // filtered echo magnitude
	UWord32 seed;
	UWord32 noiseEstCtr;

	unsigned int ecStartup; //EC valid
	Word16 farendOld[2][FRAME_LEN];
	Word32 frame_size;
	RingBuffer *farendBuf;
	RingBuffer* farFrameBuf;
	RingBuffer* nearNoisyFrameBuf;
	RingBuffer *outFrameBuf;
	biquad Biquad[5]; // 5-Bands Equalize

} FDSR_STATE;

typedef struct {
	Word16 xBuf[PART_LEN2];
	Word16 dBufNoisy[PART_LEN2];
	Word16 outBuf[PART_LEN2];
	UWord32 nearFilt[BARK_LEN];
	UWord32 echoFilt[BARK_LEN];
	UWord32 XFm[Q][BARK_LEN];
	UWord64 Sxx[Q][BARK_LEN];
	UWord64 Sxy[Q][BARK_LEN];
} AecmCore;

extern FDSR_STATE aec_config[];
extern AecmCore aecm;
extern Word16 sqrt_hann_256[];
extern Word16 squared_hann[];

Word32 norm2Word16(void *Sin, Word32 N);
void ComfortNoise(FDSR_STATE *aecm, ComplexInt32 *v32W, Word16 *hnl);
void EchoCapture(void *aecm, Word16 const *nearendNoisy, Word16 *out,
		Word16 nrOfSamples);
void EchoPlayBack(void *aecm, Word16 const *play);
void AEC_Create(FDSR_STATE *st);
void AEC_Init(FDSR_STATE *st);
void AEC_Destroy(FDSR_STATE *st);
Word32 PBFDSR(Word16 * const __restrict Sin, Word16 * const __restrict Rin,
		Word16 * const out, FDSR_STATE * __restrict st);
void PAES_Process_Block(AecmCore * __restrict aecm,
		const Word16 * __restrict Sin, const Word16 * __restrict Rin,
		Word16 *output);
void Parametric_Equalizer(Word16 *time_signal, biquad b[]);
void BandSplit(Word16 *InPut, Word16 *OutL, Word16 *OutH, Word16 (*buffer)[32],
		Word16 *memoryL, Word16 *memoryH);
void BandSynthesis(Word16 *InPutL, Word16 *InPutH, Word16 *Out);
void IIR_q14(Word16 *x, biquad *b, Word32 N);

#endif /* HD_AEC_H_ */

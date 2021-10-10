/*
 * rfft_256.h
 *
 *  Created on: 2015¦~7¤ë20¤é
 *      Author: ych
 */

#ifndef RFFT_256_H_
#define RFFT_256_H_

#include "type_def.h"
#include "webrtc_lib/include/ring_buffer.h"

#define NFFT 256
#define U_Size 4
#define V_Size 32
#define PartLen (NFFT>>1)
#define PartLen1 (PartLen+1)
#define Ra 48 // analysis hops
#define Rs 64 // synthesis hops

enum {
	kMaxFFTOrder = 10
};

extern Word32 CB_FREQ_INDICES_256[];
extern Word32 CB_OFST[];
extern const Word32 sprd_table[][18];
extern Word32 T_ABS[];

enum {
	Wiener_Mode = 0, Auditory_Mask_Mode, Wind_Mode
};

typedef struct {
	Word16 buffer[NFFT - Ra]; // prev
	Word16 sc_buffer[NFFT]; // single channel
	Word16 mOnSet[PartLen1];
	Word16 mEdge[PartLen1];
	Word16 mInterFere[PartLen1];

	Word32 nrStartup;
	Word32 cntr;
	Word32 mode;
	Word32 beta;
	Word32 gamma_prev[PartLen1];
	Word32 xi[PartLen1];
	Word32 Gh1[PartLen1];
	Word32 bufferGamma[3][PartLen1];

	Word64 S[PartLen1];
	Word64 S_tilt[PartLen1];
	Word64 Smin[PartLen1];
	Word64 Smin_sw[PartLen1];
	Word64 Smin_tilt[PartLen1];
	Word64 Smin_sw_tilt[PartLen1];
	Word64 lambda[PartLen1];
	Word64 Sxx[PartLen1];
	Word64 Nxx[PartLen1];

} nr_state;

typedef struct {
	Word16 MonoBuffer[NFFT];
	Word16 SynsBuffer[NFFT - Rs];
	Word32 ResetPhase;
	Word32 Cntr;

	//Float32 phi[PartLen1];
	//Float32 plf[PartLen1];
	//Float32 alf[PartLen1];
	//Float32 prf[PartLen1];
	//Float32 arf[PartLen1];

	Word32 alf[PartLen1];
	Word32 arf[PartLen1];

	Word32 phi[PartLen1];
	Word16 plf[PartLen1];
	Word16 prf[PartLen1];

	RingBuffer *outFrameBuf;
	RingBuffer *farFrameBuf;

} sound_stretch;

extern Word16 sqrt_hann_win[256];
extern nr_state anr_config[];
extern sound_stretch sndstrh_config[];

Word32 Complex_FFT(ComplexInt16 *frfi, Word32 stages);
Word32 Real_FFT(ComplexInt16 *RealDataIn, Word32 Order);
void HS_IFFT(ComplexInt16 *complex_data_in, Word16 *real_data_out, Word32 Order);
void NR_Create(nr_state *st, Word32 mode);
void NoiseSuppression(nr_state *Inst, ComplexInt16 *Sin, Word32 blk_exponent);
void MaskThresholds(Word64 *ps, Word64 * const mask, Word32 N, Word32 *idx);
void Overlap_Add(Word16 *Sin, Word16 *Sout, nr_state *NRI);
void TimeStretch(Word16 *Sin, Word16 *Out, sound_stretch *src);
void TimeStretch_Create(sound_stretch *src);
void TimeStretch_Init(sound_stretch *src);
void TimeStretch_Destroy(sound_stretch *src);
Word32 atan2Cordic(Word16 y, Word16 x);

#endif /* RFFT_256_H_ */

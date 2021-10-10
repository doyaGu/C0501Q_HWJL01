/*
 * dft_filter_bank.h
 *
 *  Created on: 2017¦~4¤ë19¤é
 *      Author: ite01527
 */

#ifndef DFT_FILTER_BANK_H_
#define DFT_FILTER_BANK_H_

#include "webrtc_lib/include/ring_buffer.h"
#define Nrc 64
#define R 32
#define NLen (Nrc>>1)
#define NLen1 (1+(NLen))

typedef struct {
	Word16 buffer[256];
	Word32 Sf[NLen1];
	Word16 bufferSB[Nrc][8];
	Word32 Cntr;
	Word16 qDomainOld[NLen1];
	Word32 gs[NLen1];
	Word16 qsDomainOld[NLen1];
	Word32 alpha_A; // attack
	Word32 alpha_R; // release
	Word16 qSBDomainOld[8];
	RingBuffer *farFrameBuf;
	RingBuffer *outFrameBuf;
} MultiBands;

extern MultiBands drc_config[];

void Uniform_Filt_Bank_Init(MultiBands *st);
void UniForm_Filt_Bank_Create(MultiBands *st);
void UniForm_Filt_Bank_Destroy(MultiBands *st);
void Uniform_Filt_Bank(Word16 *Sin, Word16 *Sout, MultiBands *st, Word16 nSize);

#endif /* DFT_FILTER_BANK_H_ */

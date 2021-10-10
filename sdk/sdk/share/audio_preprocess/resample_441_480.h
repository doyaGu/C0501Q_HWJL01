/*
 * resample_441_480.h
 *
 *  Created on: 2016¦~2¤ë24¤é
 *      Author: ych
 */

#ifndef RESAMPLE_441_480_H_
#define RESAMPLE_441_480_H_

#include "type_def.h"

typedef struct {
	Word16 buffer_intp2[132]; // upsampling via 2
	Word16 buffer_intp3[88]; // upsampling via 3
	Word16 buffer_intpf[192]; // fractional delay buffer
	Word16 fdf_coeff[80][192]; // fractional delay coeffs
} src_state;

void init_resample_441_480(src_state *st);
void resample_320_480(Word16 *input, Word16 *output, src_state *st);
void intp_2(const Word16 *input, Word16 * __restrict output, src_state *st,
		Word32 K);
void intp_3(const Word16 *input, Word16 * __restrict output, src_state *st,
		Word32 K);
void resample_441_480(Word16 *input, Word16 *output, src_state *st);

extern src_state src_config[];

#endif /* RESAMPLE_441_480_H_ */

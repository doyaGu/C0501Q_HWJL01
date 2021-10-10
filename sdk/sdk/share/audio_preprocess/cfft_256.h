/*
 * cfft_256.h
 *
 *  Created on: 2015¦~10¤ë15¤é
 *      Author: ite01527
 */

#ifndef CFFT_256_H_
#define CFFT_256_H_

#include "type_def.h"

void Init_Twiddle();
void cordic(Word32 gamma, ComplexInt16 *vector);
Word16 InPut_Sel(Word32 ctrl, Word32 mux);
Word16 MUX_Sel(Word32 ctrl, Word32 mux);

Word16 cfft_256(ComplexInt16 *, ComplexInt16 *);

typedef struct {
	Word32 a :2;
} Word2;

#endif /* CFFT_256_H_ */

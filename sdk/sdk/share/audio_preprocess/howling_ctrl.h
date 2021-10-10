/*
 * howling_ctrl.h
 *
 *  Created on: 2015¦~5¤ë11¤é
 *      Author: ite01527
 */

#ifndef HOWLING_CTRL_H_
#define HOWLING_CTRL_H_

#include "type_def.h"

extern Word16 hilbert_coeff[32];
extern Word16 hilbert_memory[159];
extern Word16 sin_table[32];
extern Word16 cos_table[32];

void oscillator_core(Word32 beta, Word16 Vector[], Word16 *sin_table,
		Word16 *cos_table);
void howling_ctrl(Word16 *Sin, Word16 *Sout, Word16 N);

#endif /* HOWLING_CTRL_H_ */

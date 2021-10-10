/*
 * HowlingCtrl.s
 *
 *  Created on: 2016¦~3¤ë21¤é
 *      Author: ite01527
 */

	.section .data, "aw"
	.align 4
n:
	.word 0
hilbert_mem:
	.space 320
hilbert_coeff:
	.short -111
	.short 0
	.short -192
	.short 0
	.short -440
	.short 0
	.short -922
	.short 0
	.short -1753
	.short 0
	.short -3213
	.short 0
	.short -6343
	.short 0
	.short -20651
	.short 0
	.short 20651
	.short 0
	.short 6343
	.short 0
	.short 3213
	.short 0
	.short 1753
	.short 0
	.short 922
	.short 0
	.short 440
	.short 0
	.short 192
	.short 0
	.short 111
	.short 0

	.section .text, "ax"
	.align 4

	.weak cos_table
	.weak howling_ctrl
	.weak oscillator_core
	.weak sin_table
	.weak memmove
	.global howling_ctrl

howling_ctrl:
	stmfd r13!, {r4-r11, r14} @ 9
	stmfd r13!, {r0-r2} @ 3
	sub r13, r13, #8 @ vector[2],stack 8-byte aligned

	ldr r4, =hilbert_mem
	mov r5, r0 @ r5=Sin
	mov r0, r4 @ r0=&hilbert_memory[0]
	add r1, r4, #256 @ r1=&hilbert_memory[128]
	mov r2, #62 @ r2=sizeof(int16_t)*31

	BL memmove

	add r0, r4, #62 @ r0=&hilbert_memory[31]
	mov r1, r5 @ r1=Sin
	mov r2, #256 @ r2=sizeof(int16_t)*128

	BL memmove

	mov r12, #0
	str r12, [r13, #4]

Label_no1:
	mov r11, #0 @ a0=0
	mov r4, #0 @ a1=0

	ldr r5, =hilbert_coeff
	ldr r6, =hilbert_mem
	add r5, r5, #60 @ r5=&hilbert_coeff[30]
	add r6, r6, r12, lsl #1 @ r6=&hilbert_memory[i]

	ldr r7, [r5], #-4 @ r7={c0,c1}
	ldr r8, [r5], #-4 @ r8={c2,c3}
	ldr r9, [r6], #4 @ r9={x1,x0}
	ldr r10, [r6], #4 @ r10={x3,x2}

	mov r14, #32
Label_no2:
	smlatb r11, r7, r9, r11 @ a0+=c0*x0
	smlabt r11, r7, r9, r11 @ a0+=c1*x1
	smlatb r11, r8, r10, r11 @ a0+=c2*x2
	smlabt r11, r8, r10, r11 @ a0+=c3*x3

	smlatt r4, r7, r9, r4 @ a1+=c0*x1
	smlabb r4, r7, r10, r4 @ a1+=c1*x2
	smlatt r4, r8, r10, r4 @ a1+=c2*x3

	ldr r9, [r6], #4 @ r9={x5,x4}
	ldr r10, [r6], #4 @ r10={x7,x6}

	smlabb r4, r8, r9, r4 @ a1+=c3*x0

	ldr r7, [r5], #-4 @ r7={c0,c1}
	ldr r8, [r5], #-4 @ r8={c2,c3}
	pld [r5, #0]
	pld [r5, #-4]
	subs r14, r14, #4
	bgt Label_no2

	ldr r0, =n @ r0=&n
	mov r1, r13 @ r1=vector
	ldr r0, [r0] @ r0=n1
	ldr r2, =sin_table
	ldr r3, =cos_table

	BL oscillator_core

	ldr r0, =n
	mov r4, r4, asr #15 @ a1>>15
	mov r11, r11, asr #15 @ a0>>15
	mvn r1, #0x80000000
	mov r1, r1, asr #16
	rsb r4, r4, #0 @ -a1
	rsb r11, r11, #0 @ -a0
	mov r6, r11, asr #15
	teq r6, r11, asr #31
	eorne r11, r1, r11, asr #31
	mov r6, r4, asr #15
	teq r6, r4, asr #31
	eorne r4, r1, r4, asr #31

	ldr r6, [r0] @ r6=n
	ldr r1, =10737418
	add r6, r6, r1
	str r6, [r0]
	mov r0, r6
	ldr r2, =hilbert_mem
	ldr r1, [r13] @ r1={vect[1],vect[0]}
	ldr r12, [r13, #4]
	add r2, r2, #32 @ r2=&hilbert_memory[16]
	ldr r5, [r2, r12, lsl #1]

	smulbb r6, r1, r5
	smlatb r6, r1, r11, r6
	mov r6, r6, asr #14
	mvn r1, #0x80000000
	mov r1, r1, asr #16
	mov r11, r6, asr #15
	teq r11, r6, asr #31
	eorne r6, r1, r6, asr #31

	@ mov r6, r11
	@ mov r6, r5
	ldr r1, [r13, #12]
	strh r6, [r1]

	mov r1, r13
	ldr r2, =sin_table
	ldr r3, =cos_table

	BL oscillator_core

	ldr r0, =n
	ldr r6, [r0]
	ldr r1, =10737418
	add r6, r6, r1
	str r6, [r0]
	ldr r1, [r13]

	smulbt r6, r1, r5
	smlatb r6, r1, r4, r6
	mov r6, r6, asr #14
	mvn r1, #0x80000000
	mov r1, r1, asr #16
	mov r4, r6, asr #15
	teq r4, r6, asr #31
	eorne r6, r1, r6, asr #31

	@ mov r6, r4
	@ mov r6, r5, asr #16

	ldr r1, [r13, #12]
	strh r6, [r1, #2]
	add r1, r1, #4
	ldr r12, [r13, #4]
	str r1, [r13, #12]

	add r12, r12, #2
	cmp r12, #128
	str r12, [r13, #4]
	blt Label_no1

	add r13, r13, #20
	ldmfd r13!, {r4-r11, r15}

	.end

/*
 * basic_op2.s
 *
 *  Created on: 2016¦~1¤ë22¤é
 *      Author: ite01527
 */

.section .text, "ax"
.align 4
.code 32
.global asr
.global btr_fly_r2
.global complex_asr
.global complex_mul
.global complex32_mul
.global exp_adj
.global mul_64_64
.global umul_64_128
.global udiv_64_32
.global udiv_128_64
.global rsqrt_ll
.global rsqrt
.weak ursqrt_table
.weak udiv_128_64_tab

asr:
	cmp r1, #0
	ble Label_no3
	movs r0, r0, asr r1
	adc r0, r0, #0
	bx r14
Label_no3:
	rsb r1, r1, #0
	mvn r3, #0x80000000
	mov r2, r0, lsl r1
	teq r0, r2, asr r1
	eorne r2, r3, r0, asr #31
	mov r0, r2
	bx r14

btr_fly_r2:
	ldrsh r2, [r0] @ a->real
	ldrsh r3, [r1] @ b->real
	mov r2, r2, lsl #16
	mov r3, r3, lsl #16
	qadd r12, r2, r3
	qsub r3, r2, r3
	mov r12, r12, asr #16
	mov r3, r3, asr #16
	strh r12, [r0]
	strh r3, [r1]
	ldrsh r2, [r0, #2]
	ldrsh r3, [r1, #2]
	mov r2, r2, lsl #16
	mov r3, r3, lsl #16
	qadd r12, r2, r3
	qsub r3, r2, r3
	mov r12, r12, asr #16
	mov r3, r3, asr #16
	strh r12, [r0, #2]
	strh r3, [r1, #2]
	bx r14

complex_asr:
	cmp r1, #0 @ r0=a, r1=b
	mov r2, r0, lsl #16
	mov r2, r2, asr #16 @ r2=a.real
	mov r3, r0, asr #16 @ r3=a.imag
	blt Label_no1
	@ movs r2, r2, asr r1 @ r2>>b
	@ adc r2, r2, #0
	@ movs r3, r3, asr r1
	@ adc r3, r3, #0
	mov r2, r2, asr r1
	mov r3, r3, asr r1
	mov r3, r3, lsl #16
	mov r2, r2, lsl #16
	orr r0, r3, r2, lsr #16
	bx r14

Label_no1:
	rsb r1, r1, #0
	mov r12, r2, lsl r1
	mvn r0, #0x80000000
	teq r2, r12, asr r1
	eorne r2, r0, r2, asr #31

	mov r12, r3, lsl r1
	teq r3, r12, asr r1
	eorne r3, r0, r3, asr #31

	mov r3, r3, lsl #16
	mov r2, r2, lsl #16
	orr r0, r3, r2, asr #16
	bx r14

complex32_mul:
	stmfd r13!, {r4-r9, r14}

	ldr r3, [r1] @ a.real
	ldr r12, [r2] @ b.real
	ldr r6, [r1, #4] @ a.imag
	ldr r7, [r2, #4] @ b.imag

	rsb r8, r6, #0
	smull r4, r5, r3, r12
	smlal r4, r5, r8, r7
	movs r4, r4, lsr #15
	orr r4, r4, r5, lsl #17
	mov r5, r5, asr #15
	adcs r4, r4, #0
	adc r5, r5, #0
	mvn r14, #0x80000000
	mov r9, r4, asr #31
	teq r9, r5, asr #31
	eorne r4, r14, r5, asr #31
	str r4, [r0]

	smull r8, r9, r3, r7
	smlal r8, r9, r6, r12
	movs r4, r8, lsr #15
	orr r4, r4, r9, lsl #17
	mov r5, r9, asr #15
	adcs r4, r4, #0
	adc r5, r5, #0
	mov r9, r4, asr #31
	teq r9, r5, asr #31
	eorne r4, r14, r5, asr #31
	str r4, [r0, #4]

	ldmfd r13!, {r4-r9, r15}

complex_mul:
	@smulbb r2, r0, r1
	@smultt r3, r0, r1
	@qsub r2, r2, r3
	@smultb r3, r0, r1
	@smulbt r12, r0, r1
	@qadd r3, r3, r12
	@qadd r2, r2, r2
	@qadd r3, r3, r3
	@mov r2, r2, lsr #16
	@mov r3, r3, lsr #16
	@orr r0, r2, r3, lsl #16
	@bx r14

	smulbb r2, r0, r1
	smultt r3, r0, r1

	qsub r2, r2, r3
	smulbt r3, r0, r1
	mov r12, r3, asr #31
	smlaltb r3, r12, r0, r1
	qadd r2, r2, r2
	mov r1, #0x8000
	qadd r2, r2, r1

	movs r3, r3, lsr #15
	orr r3, r3, r12, lsl #17
	adc r3, r3, #0
	mov r12, r3, asr #15
	teq r12, r3, asr #31
	ldr r12, =0x7fff
	mov r0, r2, lsr #16
	eorne r3, r12, r3, asr #31
	orr r0, r0, r3, lsl #16
	bx r14

exp_adj:
	mov r2, r0, lsl #16
	eor r2, r2, r2, lsl #1
	clz r2, r2
	rsb r0, r2, #0
	cmp r0, r1
	movlt r0, r1
	bx r14

mul_64_64:
	stmfd r13!, {r4-r6, r14}
	umull r4, r5, r0, r2
	mla r5, r0, r3, r5
	mla r5, r1, r2, r5
	mov r0, r4
	mov r1, r5
	ldmfd r13!, {r4-r6, r15}

rsqrt:
	stmfd r13!, {r4-r11, r14}
	clz r1, r0
	bic r1, r1, #1
	movs r2, r0, lsl r1 @ 0.25<=r2<1,Q32
	beq div32_by_zero
	mov r3, r2, lsr #25 @ 32<=r3<=127
	ldr r4, =ursqrt_table
	sub r5, r3, #32 @ (r3*2^25)/2^32
	ldr r5, [r4, r5, lsl #2]
	add r5, r5, #256 @ Q8
	smulbb r6, r5, r5 @ Q16
	mov r7, r2, lsr #17 @ Q15
	smulwb r6, r6, r7 @ Q15
	mov r7, r5, lsl #7 @ Q15
	rsb r6, r6, #3<<15
	mul r5, r6, r7 @ Q31
	mov r1, r1, asr #1 @ s/2
	rsb r1, r1, #15
	mov r0, r5, lsr r1 @ Q16
	ldmfd r13!, {r4-r11, r15}

div32_by_zero:
	mvn r0, #0x80000000
	ldmfd r13!, {r4-r11, r15}

rsqrt_ll:
	stmfd r13!, {r4-r11, r14}

	clz r2, r1 @ sign bits
	clz r3, r0
	cmp r2, #32
	addge r2, r2, r3
	mov r14, r2 @ r14=s
	bic r2, r2, #1 @ &~1
	cmp r2, #32
	movlt r1, r1, lsl r2 @ r1<<s
	rsblt r4, r2, #32 @ 32-s
	orrlt r1, r1, r0, lsr r4
	movlt r0, r0, lsl r2
	subge r4, r2, #32 @ s>32?
	movge r1, r0, lsl r4 @ r1=r0<<(s-32)
	movge r0, #0
	cmp r14, #64
	beq div_by_zero

	mov r14, r2 @ r14=s
	ldr r2, =ursqrt_table @ r2=&usqrt_table[0]
	mov r3, r1, lsr #25 @ 57-32=25
	add r2, r2, r3, lsl #2 @ 4B
	ldr r2, [r2, #-128] @ -32*4B
	add r2, r2, #256 @ q+=256
	smulbb r3, r2, r2 @ r3:a
	mov r4, r0, lsr #17 @ b=d>>17
	orr r4, r4, r1, lsl #15
	mov r5, r1, lsr #17 @ {r5,r4}=b=d>>17

	umull r6, r7, r3, r4 @ r6:[31:0]
	mov r12, #0 @ r7:[63:32]
	umlal r7, r12, r3, r5 @ r12:[95:64]
	mov r6, r6, lsr #16
	orr r6, r6, r7, lsl #16
	mov r7, r7, lsr #16
	orr r7, r7, r12, lsl #16 @ a={r7,r6}
	mov r3, r2, lsl #7 @ b

	mov r10, #0
	mov r11, #0x18000
	sub r10, r10, r6
	sbc r11, r11, r7 @ a

	mov r2, r14, asr #1
	rsb r2, r2, #31
	rsb r4, r2, #32

	umull r6, r7, r3, r10
	mov r12, #0
	umlal r7, r12, r3, r11

	mov r6, r6, lsr r2
	orr r0, r6, r7, lsl r4
	mov r7, r7, lsr r2
	orr r1, r7, r12, lsl r4

	ldmfd r13!, {r4-r11, r15}
div_by_zero:
	mvn r0, #0
	mvn r1, #0x80000000
	ldmfd r13!, {r4-r11, r15}

umul_64_128:
	stmfd r13!, {r4-r6, r14} @ 16B
	mov r6, r0
	ldrd r0, [r13, #16]

	umull r4, r5, r0, r2
	mov r12, #0
	umlal r5, r12, r0, r3
	mov r14, #0
	umlal r5, r14, r1, r2
	mov r0, #0
	adds r12, r12, r14
	adc r14, r0, #0
	umlal r12, r14, r1, r3

	mov r0, r6
	str r4, [r0, #0]
	str r5, [r0, #4]
	str r12, [r0, #8]
	str r14, [r0, #12]

	ldmfd r13!, {r4-r6, r15}

udiv_64_32:
	stmfd r13!, {r4-r6, r14}
	cmp r0, r1@
	ble Label_no4

	clz r2, r0 @ s=clz(d)
	mov r0, r0, lsl r2 @ 0.5 <= d < 1, Q32 format
	mov r1, r1, lsl r2 @ r1<<=s, {0xf000,16{0}}
	ldr r3, =udiv_128_64_tab @ r3=&udiv_128_64[0]
	mov r4, r0, lsr #24 @ high 8-bit, Q8:r4=d[31:24]
	sub r4, r4, #128 @ r4=d[31:24]-128
	ldr r14, [r3, r4, lsl #2] @ r3=q,init value
	add r3, r14, #256 @ r3=q+=256, Q8
	smulbb r4, r3, r3 @ r4=a=q.^2,q16
	umull r5, r6, r0, r4 @[r6:r5]=temp
	rsb r3, r6, r3, lsl #9 @ q=r3=q<<9-a
	umull r5, r6, r3, r3 @[r6:r5]=q.^2
	mov r5, r5, lsr #1 @t>>=1
	orr r5, r5, r6, lsl #31
	mov r6, r6, lsr #1

	umull r12, r14, r0, r5 @ r12=r0*r5.L
	mov r2, #0 @ r14=r0*r5.H+r0*r6.L
	umlal r14, r2, r0, r6 @r2=r0*r6.H
	mov r12, r3, lsr #16 @ temp={r2,r14}
	mov r3, r3, lsl #16 @ r12=q<<16.H
	subs r14, r3, r14 @{r12,r3}=q<<16
	sbc r2, r12, r2 @{r2,r14}=temp

	umull r5, r6, r1, r14
	mov r3, #0
	umlal r6, r3, r1, r2

	mov r6, r6, lsr #16
	orr r0, r6, r3, lsl #16

	ldmfd r13!, {r4-r6, r15}

Label_no4:
	mvn r0, #0x80000000
	mov r0, r0, lsr #16

	ldmfd r13!, {r4-r6, r15}

udiv_128_64:
	cmp r3, #0
	cmpeq r2, #0
	moveq r0, #0
	moveq r1, #0
	bxeq r14

	cmp r3, r1
	cmpeq r2, r0
	mvncs r0, #0x80000000
	movcs r0, r0, lsr #16
	movcs r1, #0
	bxcs r14

	stmfd r13!, {r4-r11, r14} @ 36B
	clz r4, r1 @
	clz r5, r0 @
	cmp r4, #32
	addeq r4, r4, r5
	cmp r4, #32
	movlt r1, r1, lsl r4
	rsblt r5, r4, #32
	orrlt r1, r1, r0, lsr r5
	movlt r0, r0, lsl r4 @ {norm([r1,r0])}
	movlt r3, r3, lsl r4
	orrlt r3, r3, r2, lsr r5
	movlt r2, r2, lsl r4
	subge r5, r4, #32
	movge r1, r0, lsl r5
	movge r0, #0
	movge r3, r2, lsl r5 @ modified
	movge r2, #0

	ldr r5, =udiv_128_64_tab
	mov r6, r1, lsr #24 @ d[31:24]
	sub r6, r6, #128
	ldr r7, [r5, r6, lsl #2]
	add r7, r7, #256
	mul r8, r7, r7 @ a

	umull r9, r10, r8, r0
	mov r11, #0
	umlal r10, r11, r8, r1 @ a=r11
	rsb r7, r11, r7, lsl #9 @ q=(q<<9)-a

	umull r8, r12, r7, r7
	mov r8, r8, lsr #1
	orr r8, r8, r12, lsl #31
	mov r12, r12, lsr #1

	umull r9, r10, r8, r0
	mov r11, #0
	umlal r10, r11, r12, r0
	mov r14, #0
	umlal r10, r14, r8, r1
	mov r4, #0
	adds r14, r14, r11
	adc r5, r4, #0
	umlal r14, r5, r12, r1

	mov r8, r7, lsr #16
	mov r7, r7, lsl #16
	subs r7, r7, r14
	sbc r8, r8, r5

	umull r9, r10, r7, r2
	mov r11, #0
	umlal r10, r11, r8, r2
	mov r14, #0
	umlal r10, r14, r7, r3
	mov r4, #0
	adds r14, r14, r11
	adc r5, r4, #0
	umlal r14, r5, r8, r3

	mov r0, r14, lsr #16
	orr r0, r0, r5, lsl #16
	mov r1, r5, lsr #16

	ldmfd r13!, {r4-r11, r15}

.end

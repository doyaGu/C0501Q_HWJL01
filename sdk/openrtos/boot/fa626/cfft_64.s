/*
 * cfft_64.s
 *
 *  Created on: 2016¦~4¤ë13¤é
 *      Author: ite01527
 */
.section .text, "ax"
.align 4
.code 32
.global cfft_arm9e
.global cfft_128_temp
.weak bitrevtab
.weak TWD_64
.weak twd_128

  @ sp[0], S
  @ sp[4], R
  @ sp[8], group
  @ sp[12], stage
  @ sp[16], s*R
  @ sp[20], block_exponent

  @ sp[0], blk_exp
  @ sp[4], stage
  @ sp[8], group
  @ sp[12], R
  @ sp[16], S
  @ sp[20], tmp32no1
  @ sp[24], tmp32no2
  @ sp[28], tmp32no3
  @ sp[32], tmp32no4

cfft_128_temp: @ decimation in frequency
	stmfd r13!, {r4-r11, r14} @ odd
	sub r13, r13, #76 @ 8-byte aligned, block exponent

	add r3, r0, #256 @ 64*4B,r3=Sin+64*(2+2)B
	mov r12, #0 @ s = 0
	str r12, [r13] @ blk_exp = 0
	mov r10, #4 @ sb = 4
	ldr r4, [r0], #4 @ r4 = a{imag,real}
	ldr r5, [r3], #4 @ r5 = b{imag,real}
L1:
	mov r6, r4, asr #16 @ r6=a.imag
	sub r7, r6, r6, lsr #31 @ r7=abs(r6)
	eor r6, r7, r7, asr #31 @ r6=abs(r6)
	mov r7, r4, lsl #16 @
	mov r7, r7, asr #16 @ r7=a.real
	sub r8, r7, r7, lsr #31 @ r8=abs(r7)
	eor r7, r8, r8, asr #31 @ r7=abs(r7)
	mov r6, r6, lsl #16
	mov r7, r7, lsl #16
	clz r8, r6 @ r8=clz(abs(a[s].imag))
	clz r9, r7 @ r9=clz(abs(a[s].real))
	cmp r10, r9 @ r10=expadj(r10, r9)
	movgt r10, r9
	cmp r10, r8
	movgt r10, r8 @ r10=expadj(r10, r8)

	mov r6, r5, asr #16 @ r6=b.imag
	sub r7, r6, r6, lsr #31
	eor r6, r7, r7, asr #31 @ r6=abs(r6)
	mov r7, r5, lsl #16
	mov r7, r7, asr #16 @ r7=b.real
	sub r8, r7, r7, lsr #31
	eor r7, r8, r8, asr #31 @ r7=abs(r7)
	mov r6, r6, lsl #16
	mov r7, r7, lsl #16
	clz r8, r6 @ r8=clz(abs(b[s].imag))
	clz r9, r7 @ r9=clz(abs(b[s].real))
	cmp r10, r9
	movgt r10, r9
	cmp r10, r8
	movgt r10, r8
	ldr r4, [r0], #4 @ r4 = a{imag,real}
	ldr r5, [r3], #4 @ r5 = b{imag,real}
	add r12, r12, #1
	cmp r12, #64
	blt L1

	sub r0, r0, #260 @ 64*4B

	mov r12, #0 @ s=0
	cmp r10, #4 @ r10={1,2,3,others}
	mov r3, #32768
	ldr r4, [r0], #4 @ r4=Sin[s]
	ldrlt r3, [r15, r10, lsl #2]
	b L2
	.word 2048 @ sb = 1, 0
	.word 4096 @ sb = 0, 1
	.word 8192 @ sb = -1, 2
	.word 16384 @ sb = -2, 3
L2:
	mov r5, r4, asr #16 @ r5=Sin[s].imag
	mul r6, r3, r5 @ scaling*r5
	mov r5, r4, lsl #16 @ r5=Sin[s].real
	movs r6, r6, asr #15 @ r6=round(r6/32768.f)
	adc r6, r6, #0
	mov r5, r5, asr #16 @ r5=Sin[s].real
	mul r7, r3, r5
	movs r7, r7, asr #15
	adc r7, r7, #0 @ r7=round(r7/32768.f)
	mov r7, r7, lsl #16
	mov r7, r7, lsr #16
	orr r7, r7, r6, lsl #16
	str r7, [r0, #-4] @ Sin[s]=r7
	ldr r4, [r0], #4
	add r12, r12, #1
	cmp r12, #128
	blt L2

	ldr r12, [r13] @ r12=blk_exp
	rsb r10, r10, #4
	add r12, r12, r10
	str r12, [r13]

	sub r0, r0, #516 @ a = Sin
	add r3, r0, #256 @ b = Sin+64*4B
	ldr r2, =twd_128 @ r2=&twd_128[0]
	mov r12, #0 @ s=0
	mov r10, #5 @ r10=sb=-4
	ldr r4, [r0], #4 @ r4=a[s]
	ldr r5, [r3], #4 @ r5=b[s]
L3:
	mov r6, r4, asr #16
	mov r6, r6, lsl #16 @ r6=a[s].imag<<16
	mov r7, r4, lsl #16 @ r7=a[s].real<<16
	mov r8, r5, asr #16
	mov r8, r8, lsl #16 @ r8=b[s].imag<<16
	mov r9, r5, lsl #16 @ r9=b[s].real<<16

	qadd r11, r7, r9 @ a[s].real<<16
	qadd r14, r6, r8 @ a[s].imag<<16
	orr r1, r14, r11, lsr #16 @ r1=a[s]
	str r1, [r0, #-4]
	sub r1, r11, r11, lsr #31
	eor r11, r1, r1, asr #31 @ r11=abs(a[s].real<<16)
	clz r1, r11
	cmp r10, r1
	movgt r10, r1
	sub r1, r14, r14, lsr #31
	eor r14, r1, r1, asr #31 @ r14=abs(a[s].imag<<16)
	clz r1, r14
	cmp r10, r1
	movgt r10, r1

	qsub r11, r7, r9 @ b[s].real<<16
	qsub r14, r6, r8 @ b[s].imag<<16
	orr r1, r14, r11, lsr #16 @ r1={b[s].imag,b[s].real}
	ldr r6, [r2], #4 @ r6=twd_128[s]

	smulbb r7, r1, r6 @ r7=a.real*b.real
	smultt r8, r1, r6 @ r8=a.imag*b.imag

	qsub r7, r7, r8 @ r7=b[s].real<<15
	smulbt r9, r1, r6 @ r9=a.real*b.imag
	mov r8, r9, asr #31 @ r8={32{r9[31]}}
	smlaltb r9, r8, r1, r6
	movs r7, r7, asr #15
	adc r7, r7, #0
	movs r9, r9, lsr #15
	orr r9, r9, r8, lsl #17 @ r9=b[s].imag
	adc r9, r9, #0
	mov r8, r9, asr #15
	teq r8, r9, asr #31
	ldr r8, =0x7fff
	mov r1, r7, lsl #16
	eorne r9, r8, r9, asr #31
	sub r7, r1, r1, lsr #31
	eor r7, r7, r7, asr #31
	clz r6, r7
	cmp r10, r6
	movgt r10, r6

	mov r1, r1, lsr #16
	orr r1, r1, r9, lsl #16
	str r1, [r3, #-4]
	mov r1, r9, lsl #16
	sub r7, r1, r1, lsr #31
	eor r7, r7, r7, asr #31
	clz r6, r7
	cmp r10, r6
	movgt r10, r6
	ldr r4, [r0], #4
	ldr r5, [r3], #4
	add r12, r12, #1
	cmp r12, #64
	blt L3

	mov r12, #0
	mov r3, #32768
	sub r0, r0, #260 @ reset *Sin
	ldr r4, [r0], #4
	cmp r10, #5
	ldrlt r3, [r15, r10, lsl #2]
	b L4
	.word 1024
	.word 2048
	.word 4096
	.word 8192
	.word 16384
L4:
	mov r5, r4, lsl #16
	mov r5, r5, asr #16 @ r5=Sin[s].real
	mul r6, r3, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	mov r7, r4, asr #16
	mul r8, r3, r7
	movs r8, r8, asr #15
	adc r8, r8, #0
	mov r6, r6, lsl #16
	mov r8, r8, lsl #16
	orr r8, r8, r6, lsr #16
	str r8, [r0, #-4]
	ldr r4, [r0], #4
	add r12, r12, #1
	cmp r12, #128
	blt L4

	ldr r12, [r13] @ r12=blk_exp
	rsb r10, r10, #5
	add r12, r12, r10
	str r12, [r13]

	sub r0, r0, #516 @ reset *Sin
	str r0, [r13, #32] @ r0=Sin

	mov r2, #64 @ S=64
	mov r1, #1 @ R=1
	str r2, [r13, #16]
	mov r2, #0 @ stage=0
	str r1, [r13, #12]
	str r2, [r13, #4]

next_stage_loop:
	mov r14, #0 @ s=0
	str r14, [r13, #36]
	mov r10, #5 @ sb=-4
next_btrfly_loop:
	ldr r0, [r13, #32]
	add r3, r0, r14, lsl #2 @ r3=Sin+s*4B
	ldr r2, [r13, #16] @ r2=S
	mov r2, r2, asr #2 @ r2=S/4
	add r4, r3, r2, lsl #2 @ r4=b=a+S/4
	add r5, r4, r2, lsl #2 @ r5=c=b+S/4
	add r6, r5, r2, lsl #2 @ r6=d=c+S/4
	str r3, [r13, #40]
	str r4, [r13, #44]
	str r5, [r13, #48]
	str r6, [r13, #52]

	add r3, r0, #256 @ Sin+64*4B
	add r3, r3, r14, lsl #2
	add r4, r3, r2, lsl #2
	add r5, r4, r2, lsl #2
	add r6, r5, r2, lsl #2

	str r3, [r13, #56]
	str r4, [r13, #60]
	str r5, [r13, #64]
	str r6, [r13, #68]

	mov r12, #0
	str r12, [r13, #8] @ group=0
	ldr r12, =twd_128
	ldr r11, [r13, #12] @ r11=R
	mul r9, r11, r14 @ r9=s*R
	mov r9, r9, lsl #1 @ 2*(s*R)
	ldr r7, [r12, r9, lsl #2] @ r7=ytab
	add r8, r9, r9 @ r8=2*(2*s*R)
	str r7, [r13, #20] @ tmp32no1=ytab
	ldr r7, [r12, r8, lsl #2] @ r7=xtab
	add r8, r8, r9 @ r8=2*(3*s*R)
	str r7, [r13, #24] @ tmp32no2=xtab
	ldr r7, [r12, r8, lsl #2] @ r7=ztab
	str r7, [r13, #28] @ tmp32no3=ztab

next_block_loop:
	ldr r3, [r13, #40] @ a
	ldr r4, [r13, #44] @ b
	ldr r5, [r13, #48] @ c
	ldr r6, [r13, #52] @ d

	BL block_process

	str r3, [r13, #40]
	str r4, [r13, #44]
	str r5, [r13, #48]
	str r6, [r13, #52]

	ldr r3, [r13, #56] @ w
	ldr r4, [r13, #60] @ x
	ldr r5, [r13, #64] @ y
	ldr r6, [r13, #68] @ z

	BL block_process

	str r3, [r13, #56] @ w
	str r4, [r13, #60] @ x
	str r5, [r13, #64] @ y
	str r6, [r13, #68] @ z

	ldr r12, [r13, #8] @ r12=group
	ldr r11, [r13, #12] @ r11=R
	add r12, r12, #1
	cmp r12, r11
	str r12, [r13, #8]
	blt next_block_loop
	ldr r14, [r13, #36]
	add r14, r14, #1
	str r14, [r13, #36]
	ldr r12, [r13, #16]
	cmp r14, r12, asr #2
	blt next_btrfly_loop

	mov r3, #32768
	mov r14, #0
	ldr r0, [r13, #32] @ r0=Sin
	cmp r10, #5
	ldr r9, [r0], #4 @ r9=Sin[s]
	ldrlt r3, [r15, r10, lsl #2]
	b L5
	.word 1024
	.word 2048
	.word 4096
	.word 8192
	.word 16384
L5:
	ldr r12, [r13, #4] @ r12=stage
	mov r11, r9, asr #16 @ r11=Sin[s].imag
	cmp r12, #2
	bge L6
	mov r9, r9, lsl #16
	mov r9, r9, asr #16
	mul r12, r3, r11 @ imag
	mul r8, r3, r9 @ real
	movs r12, r12, asr #15
	adc r12, r12, #0
	movs r8, r8, asr #15
	adc r8, r8, #0
	mov r12, r12, lsl #16
	mov r8, r8, lsl #16
	orr r12, r12, r8, lsr #16
	str r12, [r0, #-4]
	add r14, r14, #1
	ldr r9, [r0], #4
	cmp r14, #128
	blt L5
L6:
	ldr r12, [r13, #4]
	cmp r12, #2
	ldrlt r8, [r13] @ blk_exp
	rsblt r10, r10, #5
	addlt r8, r8, r10
	strlt r8, [r13] @
	ldr r12, [r13, #16] @ S
	ldr r11, [r13, #12] @ R
	mov r12, r12, asr #2
	mov r11, r11, lsl #2
	str r12, [r13, #16] @ S
	str r11, [r13, #12] @ R
	ldr r12, [r13, #4] @ stage
	add r12, r12, #1
	cmp r12, #3
	str r12, [r13, #4]
	blt next_stage_loop

	mov r12, #0
	ldr r4, =bitrevtab
	ldr r5, [r13, #32] @ *out
L7:
	ldr r6, [r4], #4
	cmp r6, r12
	ldrgt r7, [r5, r12, lsl #2]
	ldrgt r8, [r5, r6, lsl #2]
	strgt r7, [r5, r6, lsl #2]
	strgt r8, [r5, r12, lsl #2]
	add r12, r12, #1
	cmp r12, #128
	blt L7

	ldr r0, [r13]

	add r13, r13, #76
	ldmfd r13!, {r4-r11, r15}

block_process:
	ldrsh r1, [r3] @ r1=a.real
	ldrsh r7, [r5] @ r7=c.real
	ldrsh r9, [r3, #2] @ r9=a.imag
	ldrsh r11, [r5, #2] @ r11=c.imag

	mov r1, r1, lsl #16 @ r1={a.real,16'b0}
	mov r7, r7, lsl #16 @ r7={c.real,16'b0}
	mov r9, r9, lsl #16 @ r9={a.imag,16'b0}
	mov r11, r11, lsl #16 @ r11={c.imag,16'b0}

	qadd r8, r1, r7 @ r8=a.real
	qadd r12, r9, r11 @ r12=a.imag
	orr r12, r12, r8, lsr #16
	str r12, [r3] @ a[0]

	qsub r8, r1, r7 @ r8=c.real
	qsub r12, r9, r11 @ r12=c.imag
	orr r12, r12, r8, lsr #16
	str r12, [r5] @ c[0]

	ldrsh r1, [r4] @ r1=b.real
	ldrsh r7, [r6] @ r7=d.real
	ldrsh r9, [r4, #2] @ r9=b.imag
	ldrsh r11, [r6, #2] @ r11=d.imag

	mov r1, r1, lsl #16 @ r1={b.real,16'b0}
	mov r7, r7, lsl #16 @ r7={d.real,16'b0}
	mov r9, r9, lsl #16 @ r9={b.imag,16'b0}
	mov r11, r11, lsl #16 @ r11={d.imag,16'b0}

	qadd r8, r1, r7 @ r8=b.real
	qadd r12, r9, r11 @ r12=b.imag
	orr r12, r12, r8, lsr #16
	str r12, [r4] @ b[0]

	qsub r8, r1, r7 @ r8=d.real
	qsub r12, r9, r11 @ r12=d.imag
	orr r12, r12, r8, lsr #16
	str r12, [r6] @ d[0]

	ldrsh r1, [r3] @ r1=a.real
	ldrsh r7, [r4] @ r7=b.real
	ldrsh r9, [r3, #2] @ r9=a.imag
	ldrsh r11, [r4, #2] @ r11=b.imag

	mov r1, r1, lsl #16 @ r1={a.real,16'b0}
	mov r7, r7, lsl #16 @ r7={b.real,16'b0}
	mov r9, r9, lsl #16 @ r9={a.imag,16'b0}
	mov r11, r11, lsl #16 @ r11={b.imag,16'b0}

	qadd r8, r1, r7 @ {a.real,16'b0}
	sub r12, r8, r8, lsr #31
	eor r12, r12, r12, asr #31 @ abs(a.real<<16)
	clz r12, r12
	cmp r10, r12
	movgt r10, r12

	qadd r12, r9, r11 @ r12={a.imag,16'b0}
	sub r0, r12, r12, lsr #31
	eor r0, r0, r0, asr #31 @ abs(a.imag<<16)
	clz r0, r0
	cmp r10, r0
	movgt r10, r0
	orr r12, r12, r8, lsr #16
	str r12, [r3], r2, lsl #4 @ a+=S

	qsub r8, r1, r7 @ b.real
	qsub r12, r9, r11 @ b.imag
	orr r12, r12, r8, lsr #16
	@ str r12, [r4], r2, lsl #4 @ b[0]
	str r12, [r4]

	ldrsh r1, [r5] @ c.real
	ldrsh r7, [r6] @ d.real
	ldrsh r9, [r5, #2] @ c.imag
	ldrsh r11, [r6, #2] @ d.imag

	mov r1, r1, lsl #16 @ {c.real,16'b0}
	mov r7, r7, lsl #16 @ {d.real,16'b0}
	mov r9, r9, lsl #16 @ {c.imag,16'b0}
	mov r11, r11, lsl #16 @ {d.imag,16'b0}

	qadd r8, r1, r11 @ r8=real((a+jb)+(d-jc))
	qsub r12, r9, r7 @ r12=imag((a+jb)+(d-jc))
	orr r12, r12, r8, lsr #16
	@ str r12, [r5], r2, lsl #4 @ c[0]
	str r12, [r5]

	qsub r8, r1, r11 @ r8=real((a+jb)-(d-jc))
	qadd r12, r9, r7 @ r12=imag((a+jb)-(d-jc))
	orr r12, r12, r8, lsr #16
	@ str r12, [r6], r2, lsl #4 @ d[0]
	str r12, [r6]

	ldr r7, [r4] @ r7=b[0]
	ldr r9, [r13, #24] @ xtab
	smulbb r12, r7, r9
	smultt r11, r7, r9

	qsub r12, r12, r11
	smulbt r11, r7, r9
	mov r1, r11, asr #31
	smlaltb r11, r1, r7, r9
	movs r12, r12, asr #15
	adc r12, r12, #0 @ real
	movs r11, r11, lsr #15
	orr r1, r11, r1, lsl #17
	adc r1, r1, #0
	ldr r11, =0x7fff
	mov r7, r1, asr #15
	teq r7, r1, asr #31
	eorne r1, r11, r1, asr #31
	mov r7, r12, lsl #16 @ r7=real<<16
	sub r11, r7, r7, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	mov r7, r7, lsr #16
	mov r8, r1, lsl #16
	sub r11, r8, r8, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	orr r1, r7, r1, lsl #16
	str r1, [r4], r2, lsl #4 @ b+=S

	ldr r7, [r5] @ r7=c[0]
	ldr r9, [r13, #20] @ ytab
	smulbb r12, r7, r9
	smultt r11, r7, r9

	qsub r12, r12, r11
	smulbt r11, r7, r9
	mov r1, r11, asr #31
	smlaltb r11, r1, r7, r9
	movs r12, r12, asr #15
	adc r12, r12, #0 @ real
	movs r11, r11, lsr #15
	orr r1, r11, r1, lsl #17
	adc r1, r1, #0
	ldr r11, =0x7fff
	mov r7, r1, asr #15
	teq r7, r1, asr #31
	eorne r1, r11, r1, asr #31
	mov r7, r12, lsl #16
	sub r11, r7, r7, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	mov r7, r7, lsr #16
	mov r8, r1, lsl #16
	sub r11, r8, r8, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	orr r1, r7, r1, lsl #16
	str r1, [r5], r2, lsl #4 @ c+=S

	ldr r7, [r6] @ r7=d[0]
	ldr r9, [r13, #28] @ ztab
	smulbb r12, r7, r9
	smultt r11, r7, r9

	qsub r12, r12, r11
	smulbt r11, r7, r9
	mov r1, r11, asr #31
	smlaltb r11, r1, r7, r9
	movs r12, r12, asr #15
	adc r12, r12, #0 @ real
	movs r11, r11, lsr #15
	orr r1, r11, r1, lsl #17
	adc r1, r1, #0
	ldr r11, =0x7fff
	mov r7, r1, asr #15
	teq r7, r1, asr #31
	eorne r1, r11, r1, asr #31
	mov r7, r12, lsl #16
	sub r11, r7, r7, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	mov r7, r7, lsr #16
	mov r8, r1, lsl #16
	sub r11, r8, r8, lsr #31
	eor r11, r11, r11, asr #31
	clz r8, r11
	cmp r10, r8
	movgt r10, r8
	orr r1, r7, r1, lsl #16
	str r1, [r6], r2, lsl #4 @ d+=S
	bx r14

cfft_arm9e:
	stmfd r13!, {r4-r12, r14}
	sub r13, r13, #24
	@ adr r2, block_exponent
	mov r1, #0
	@ str r1, [r2]
	str r1, [r13, #20]

	mov r2, #64
	mov r1, #1
	str r2, [r13] @ S
	str r1, [r13, #4] @ R
	mov r2, #0
	str r2, [r13, #12] @ stage

next_stage_arm9e:
	mov r2, #0
	str r2, [r13, #8] @ group
	ldr r2, [r13] @ S
next_block_arm9e:
	mov r14, r2, lsr #2 @ s=S/4
	sub r14, r14, #1 @ S/4-1

	ldr r1, [r13, #8] @ group
	mul r3, r1, r2 @ group*S

	add r6, r0, r3, lsl #2 @ grp*S*4B
	add r6, r6, r14, lsl #2 @ &a[group*S+S/4-1]
	add r7, r6, r2 @ &b[s]
	add r8, r7, r2 @ &c[s]
	add r9, r8, r2 @ &d[s]

next_butterfly_arm9e:
	ldrsh r2, [r6] @ a.real
	ldrsh r3, [r6, #2] @ a.imag
	ldrsh r4, [r8] @ c.real
	ldrsh r5, [r8, #2] @ c.imag

	mov r2, r2, lsl #16 @ {a.real, 16'b0}
	mov r4, r4, lsl #16 @ {c.real, 16'b0}
	qadd r10, r2, r4 @ {a[s].real, 16'b0}
	mov r3, r3, lsl #16
	mov r5, r5, lsl #16
	qadd r11, r3, r5 @ {a[s].imag, 16'b0}

	orr r11, r11, r10, lsr #16 @ {a.imag, a.real}
	str r11, [r6] @ a

	qsub r10, r2, r4 @ {c.real, 16'b0}
	qsub r11, r3, r5 @ {c.imag, 16'b0}
	orr r11, r11, r10, lsr #16
	str r11, [r8] @ {c.imag, c.real}

	ldrsh r2, [r7] @ b.real
	ldrsh r3, [r7, #2] @ b.imag
	ldrsh r4, [r9] @ d.real
	ldrsh r5, [r9, #2] @ d.imag

	mov r2, r2, lsl #16 @ {b.real, 16'b0}
	mov r4, r4, lsl #16 @ {d.real, 16'b0}
	qadd r10, r2, r4
	mov r3, r3, lsl #16 @ {b.imag, 16'b0}
	mov r5, r5, lsl #16 @ {d.imag, 16'b0}
	qadd r11, r3, r5

	orr r12, r11, r10, lsr #16 @ r12={b.imag, b.real}

	qsub r10, r2, r4
	qsub r11, r3, r5
	orr r11, r11, r10, lsr #16 @ d
	str r11, [r9]

	ldrsh r2, [r6] @ a.real
	ldrsh r3, [r6, #2] @ a.imag
	mov r4, r12, lsl #16 @ r4={b.real, 16'b0}
	mov r5, r12, lsr #16 @ r5={b.imag, 16'b0}
	mov r5, r5, lsl #16
	mov r2, r2, lsl #16 @ r2={a.real, 16'b0}
	mov r3, r3, lsl #16 @ r3={a.imag, 16'b0}
	qadd r10, r2, r4
	qadd r11, r3, r5
	orr r11, r11, r10, lsr #16
	str r11, [r6], #-4

	qsub r10, r2, r4
	qsub r11, r3, r5
	ldr r12, =TWD_64
	ldr r3, [r13, #4] @ R
	orr r11, r11, r10, lsr #16 @ b=a+jb
	mul r1, r3, r14 @ s*R
	str r1, [r13, #16]
	ldr r2, [r12, r1, lsl #3] @ 4B*(2*s), r2=x_tab=c+jd

 	smulbb r1, r2, r11 @ r*r
 	smultt r3, r2, r11 @ i*i
 	qsub r1, r1, r3 @ rr-ii
 	qadd r1, r1, r1 @ Q16, b.real
 	smulbt r4, r2, r11
 	smlatb r4, r2, r11, r4
 	qadd r4, r4, r4 @ Q16, b.imag
 	mov r4, r4, lsr #16
 	mov r4, r4, lsl #16
	orr r11, r4, r1, lsr #16 @ {b.imag, b.real}

	str r11, [r7], #-4

	ldrsh r2, [r8] @ c.real
	ldrsh r4, [r9] @ d.real
	ldrsh r3, [r8, #2] @ c.imag
	ldrsh r5, [r9, #2] @ d.imag
	mov r2, r2, lsl #16 @ {c.real, 16'b0}
	mov r4, r4, lsl #16 @ {d.real, 16'b0}
	mov r3, r3, lsl #16 @ {c.imag, 16'b0}
	mov r5, r5, lsl #16 @ {d.imag, 16'b0}
	qadd r10, r2, r5 @ real
	qsub r11, r3, r4 @ imag
	orr r11, r11, r10, lsr #16 @ c=a+jb

	qsub r10, r2, r5 @ real
	qadd r1, r3, r4 @ imag
	orr r1, r1, r10, lsr #16 @ d

	ldr r5, [r13, #16] @ s*R
	ldr r2, [r12, r5, lsl #2] @ r2=y_tab=c+jd
	smulbb r5, r2, r11
	smultt r10, r2, r11
	qsub r5, r5, r10
	qadd r5, r5, r5 @ Q16
	smulbt r4, r2, r11
	smlatb r4, r2, r11, r4
	qadd r4, r4, r4 @ Q16
	mov r4, r4, lsr #16
	mov r4, r4, lsl #16
	orr r11, r4, r5, lsr #16 @ c

	str r11, [r8], #-4
	ldr r5, [r13, #16] @ s*R
	add r10, r5, r5, lsl #1 @ 3*s*R
	ldr r2, [r12, r10, lsl #2] @ c+jd

	smulbb r5, r2, r1
	smultt r10, r2, r1
	qsub r5, r5, r10
	qadd r5, r5, r5 @ Q16
	smulbt r4, r2, r1
	smlatb r4, r2, r1, r4
 	qadd r4, r4, r4 @ Q16
 	mov r4, r4, lsr #16
 	mov r4, r4, lsl #16
 	orr r11, r4, r5, lsr #16

	str r11, [r9], #-4

	subs r14, r14, #1
	bge next_butterfly_arm9e

	ldr r2, [r13, #8] @ group
	ldr r1, [r13, #4] @ R
	add r2, r2, #1
	str r2, [r13, #8]
	cmp r2, r1
	ldr r2, [r13] @ S
	blt next_block_arm9e

	@ ldr r3, [r13, #12] @ stage
	@ cmp r3, #2
	@ bge Label_no1_arm9e

	mov r12, #64
	mov r1, #5 @ sb
	mov r2, r0 @ &Sin[0]
	ldr r3, [r2], #4

block_float_arm9e:
	mov r4, r3, lsr #16
	mov r4, r4, lsl #16 @ .imag
	mov r3, r3, lsl #16 @ .real
	sub r5, r4, r4, lsr #31 @ abs
	eor r5, r5, r5, asr #31
	sub r6, r3, r3, lsr #31
	eor r6, r6, r6, asr #31
	clz r7, r5
	clz r8, r6
	cmp r7, r1
	movlt r1, r7
	cmp r8, r1
	movlt r1, r8
	subs r12, r12, #1
	ldr r3, [r2], #4
	bgt block_float_arm9e

	@ adr r2, block_exponent
	@ ldr r3, [r2]
	ldr r3, [r13, #20]
	mov r12, #64
	rsb r4, r1, #5
	add r3, r3, r4
	@ str r3, [r2]
	str r3, [r13, #20]
	mov r2, r0
	ldrsh r3, [r2], #2 @ real
	ldrsh r4, [r2], #2 @ imag
block_norm_arm9e:
	cmp r1, #5
	addlt pc, pc, r1, lsl #2
	b method_d
	b method_d
	b method_1
	b method_2
	b method_3
	b method_4

Label_no2_arm9e:
	subs r12, r12, #1
	ldrsh r3, [r2], #2 @ real
	ldrsh r4, [r2], #2 @ imag
	bgt block_norm_arm9e

Label_no1_arm9e:
	ldr r3, [r13, #4] @ R
	ldr r2, [r13] @ S
	mov r2, r2, asr #2
	mov r3, r3, lsl #2
	str r2, [r13]
	str r3, [r13, #4]

	ldr r2, [r13, #12]
	add r2, r2, #1
	@ cmp r2, #3
	cmp r2, #2
	str r2, [r13, #12]
	blt next_stage_arm9e

Last_Stage_arm9e:
	mov r12, #16 @ group
	mov r6, r0
	add r7, r6, #4 @ 4B
	add r8, r7, #4
	add r9, r8, #4

Last_Stage_Loop:
	ldrsh r2, [r6] @ a.real
	ldrsh r3, [r6, #2] @ a.imag
	ldrsh r4, [r8] @ c.real
	ldrsh r5, [r8, #2] @ c.imag

	mov r2, r2, lsl #16 @ {a.real, 16'b0}
	mov r4, r4, lsl #16 @ {c.real, 16'b0}
	qadd r10, r2, r4
	mov r3, r3, lsl #16 @ {a.imag, 16'b0}
	mov r5, r5, lsl #16 @ {c.imag, 16'b0}
	qadd r11, r3, r5

	orr r11, r11, r10, lsr #16
	str r11, [r6] @ a

	qsub r10, r2, r4
	qsub r11, r3, r5
	orr r11, r11, r10, lsr #16
	str r11, [r8] @ c

	ldrsh r2, [r7] @ b.real
	ldrsh r3, [r7, #2] @ b.imag
	ldrsh r4, [r9] @ d.real
	ldrsh r5, [r9, #2] @ d.imag

	mov r2, r2, lsl #16 @ {b.real, 16'b0}
	mov r4, r4, lsl #16 @ {d.real, 16'b0}
	qadd r10, r2, r4
	mov r3, r3, lsl #16 @ {b.imag, 16'b0}
	mov r5, r5, lsl #16 @ {d.imag, 16'b0}
	qadd r11, r3, r5

	orr r14, r11, r10, lsr #16 @ r14:b

	qsub r10, r2, r4
	qsub r11, r3, r5
	orr r11, r11, r10, lsr #16 @ d

	str r11, [r9]

	ldrsh r2, [r6] @ a.real
	ldrsh r3, [r6, #2] @ a.imag
	mov r4, r14, lsl #16 @ {b.real, 16'b0}
	mov r5, r14, lsr #16 @ {b.imag, 16'b0}
	mov r5, r5, lsl #16

	mov r2, r2, lsl #16 @ {a.real, 16'b0}
	mov r3, r3, lsl #16 @ {a.imag, 16'b0}
	qadd r10, r2, r4
	qadd r11, r3, r5
	orr r11, r11, r10, lsr #16
	str r11, [r6], #16

	qsub r10, r2, r4
	qsub r11, r3, r5
	orr r11, r11, r10, lsr #16 @ b
	str r11, [r7], #16

	ldrsh r2, [r8] @ c.real
	ldrsh r4, [r9] @ d.real
	ldrsh r3, [r8, #2] @ c.imag
	ldrsh r5, [r9, #2] @ d.imag

	mov r2, r2, lsl #16
	mov r4, r4, lsl #16
	mov r3, r3, lsl #16
	mov r5, r5, lsl #16
	qadd r10, r2, r5
	qsub r11, r3, r4
	orr r11, r11, r10, lsr #16

	qsub r10, r2, r5
	qadd r2, r3, r4
	orr r10, r2, r10, lsr #16

	str r11, [r8], #16
	str r10, [r9], #16

	subs r12, r12, #1
	bgt Last_Stage_Loop

	@ adr r2, block_exponent
	@ ldr r0, [r2]
	ldr r0, [r13, #20]
	add r13, r13, #24
	ldmfd r13!, {r4-r12, r15}

method_d:
	b Label_no2_arm9e

method_1:
	mov r5, #2048
	smulbb r6, r3, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-4]
	smulbb r6, r4, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-2]
	b Label_no2_arm9e

method_2:
	mov r5, #4096
	smulbb r6, r3, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-4]
	smulbb r6, r4, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-2]
	b Label_no2_arm9e

method_3:
	mov r5, #8192
	smulbb r6, r3, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-4]
	smulbb r6, r4, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-2]
	b Label_no2_arm9e

method_4:
	mov r5, #16384
	smulbb r6, r3, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-4]
	smulbb r6, r4, r5
	movs r6, r6, asr #15
	adc r6, r6, #0
	strh r6, [r2, #-2]
	b Label_no2_arm9e

.end

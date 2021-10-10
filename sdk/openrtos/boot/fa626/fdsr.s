/*
 * fdsr.s
 *
 *  Created on: 2016¦~2¤ë2¤é
 *      Author: ite01527
 */

.section .text, "x"
.align 4
.code 32
.weak CB_OFST
.weak dc_gain
.weak sprd_table
.weak T_ABS
.weak udiv_64_64
.global MaskThresholds

MaskThresholds:
	stmfd r13!, {r4-r11, r14}
	sub r13, r13, #148 @ union { Word64 bark_power[18]@ Word64 T[18]@ }

	mov r4, #0
	mov r5, #0
	mov r12, #0 @ i
memclr:
	mov r6, r12, lsl #3 @8*i
	strd r4, [r13, r6]
	add r12, r12, #1
	cmp r12, #18
	blt memclr

	mov r12, #0 @ i
	ldr r4, =sprd_table
label_no1:
	mov r14, r2, asr #1 @ N/2
	sub r14, r14, #1 @ j=N/2-1
	@ ldr r5, [r4, r12, lsl #2] @ r5=sprd_table[i]
	add r5, r4, r12, lsl #6 @ r5=addr(sprd_table)+64*i
	add r5, r5, r12, lsl #3 @ r5+=8*i,r5=addr(sprd_table)+18*(4B)*i
	mov r8, r14, lsl #3 @ 8*j
	ldr r9, [r3, r14, lsl #2] @ idx[j]
	@ ldrd r6, [r0, r8] @ [r7:r6]=ps[j],r5=&sprd_table[i][0]
	ldr r6, [r0, r8] @ low
	add r8, r8, #4
	ldr r7, [r0, r8] @ not 8 bytes aligned
	ldr r10, [r5, r9, lsl #2] @ sprd_table[i][j]

label_no2:
	umull r9, r11, r10, r6
	mov r8, #0
	umlal r11, r8, r10, r7
	mov r9, r9, lsr #31
	orr r9, r9, r11, lsl #1
	mov r11, r11, lsr #31
	orr r8, r11, r8, lsl #1
	mov r6, r12, lsl #3
	ldrd r10, [r13, r6] @ [r10,r11]=bark_power[i]

	adds r10, r10, r9 @ [r8,r9]
	adc r11, r11, r8

	strd r10, [r13, r6]
	subs r14, r14, #1
	movgt r8, r14, lsl #3 @ 8*j
	ldrgt r9, [r3, r14, lsl #2] @ idx[j]
	@ ldrd r6, [r0, r8] @ [r7:r6]=ps[j]
	ldrgt r6, [r0, r8] @ low
	addgt r8, r8, #4
	ldrgt r7, [r0, r8] @ high
	ldrgt r10, [r5, r9, lsl #2] @ sprd_table[i][j]
	bgt label_no2

	add r12, r12, #1
	cmp r12, #18
	@ ldr r4, =sprd_table
	blt label_no1

	mov r4, r0 @ *ps
	mov r5, r1 @ *mask

	clz r6, r2
	str r2, [r13, #144] @ mem[r13,18*8B]=N
	rsb r6, r6, #31
	mov r7, r3 @ idx

	mov r10, #0
label_no3:
	mov r9, r10, lsl #3 @8*i
	ldr r8, =CB_OFST
	ldrd r2, [r13, r9] @ bark_power[i]
	ldr r0, [r8, r10, lsl #2] @ cb_ofst[i]
	mov r1, #0

	bl udiv_64_64
	ldr r11, =dc_gain
	ldr r8, [r11, r10, lsl #2]

	umull r12, r14, r0, r8
	mov r11, #0
	umlal r14, r11, r1, r8
	mov r12, r12, lsr #15
	orr r12, r12, r14, lsl #17
	mov r14, r14, lsr #15
	orr r14, r14, r11, lsl #17
	ldr r11, =T_ABS
	ldr r11, [r11, r10, lsl #2]
	@ mov r11, r11, asr r6
	mov r0, r11, asr #31
	subs r1, r12, r11
	sbcs r2, r14, r0
	movlt r14, r0
	movlt r12, r11
	str r12, [r13, r9]
	add r9, r9, #4
	str r14, [r13, r9]
	add r10, r10, #1
	cmp r10, #18
	blt label_no3

	mov r12, #0
	ldr r6, [r13, #144] @ N
	mov r6, r6, asr #1

label_no4:
	ldr r2, [r7, r12, lsl #2] @ idx[i]
	mov r2, r2, lsl #3
	mov r3, r12, lsl #3
	ldrd r0, [r13, r2]
	@ strd r0, [r5, r3]
	str r0, [r5, r3]
	add r3, r3, #4
	str r1, [r5, r3]

	add r12, r12, #1
	cmp r12, r6
	ble label_no4

	add r13, r13, #148
	ldmfd r13!, {r4-r11, r15}

.end

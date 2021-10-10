/*
 * basic_op.h
 *
 *  Created on: 2015¦~3¤ë23¤é
 *      Author: ych
 */
#ifndef BASIC_OP_H_
#define BASIC_OP_H_

#include "type_def.h"

void circshift(Word32 a[], Word32 m, Word32 size);
Word32 permutate(Word32 n, Word32 N);
UWord32 rsqrt(UWord32 q);
UWord64 rsqrt_ll(UWord64 q);
UWRD128 uadd_128(UWRD128 a, UWRD128 b);
UWord32 udiv_32_32(UWord32 d, UWord32 n);
UWord64 udiv_64_64(UWord64 d, UWord64 n);
UWord32 udiv_64_32(UWord32 d, UWord32 n);
UWord64 udiv_q15_arm9E(UWord64 d, UWord64 n);
UWord32 udiv_q15_arm9e(UWord32 d, UWord32 n);
UWord64 udiv_128_64(UWord64 d, UWord64 n);
UWord32 uexp_32(UWord32 n);
UWord32 uexp2_32(UWord32 n);
Word32 ulog2(UWord64 a); //only integer part
Word32 ulog2_32(UWord32 n);
UWRD128 umul_64_128(UWord64 b, UWord64 c);
UWRD128 usub_128(UWRD128 a, UWRD128 b);
UWord32 usqrt(UWord32 q);
UWord64 usqrt_ll(UWord64 q);
Word64 sat_128_64(UWRD128 a);
Word32 sdiv_32_32(Word32 d, Word32 n);
Word64 sdiv_64_64(Word64 d, Word64 n);
UWRD128 smul_64_128(Word64 b, Word64 c);
void swap_data(Complex16_t *a, Complex16_t *b);

/*.asm*/
#if defined(__arm__)&&!defined(_MSC_VER)
extern UWRD128 umul_64to128_arm9e(UWord64 b, UWord64 c); //cv:const,volatile
extern Word32 asr(Word32 a, Word32 b);//mutable
extern void btr_fly_r2(ComplexInt16 * __restrict a, ComplexInt16 * __restrict b);//extern auto register\static
extern ComplexInt32 complex32_mul(ComplexInt32 * __restrict a, ComplexInt32 * __restrict b);
extern ComplexInt16 complex_mul(Complex16_t, Complex16_t);
extern ComplexInt16 complex_asr(ComplexInt16 a, Word32 b);
extern Word64 mul_64_64(Word64 b, Word64 c);
extern exp_adj(Word16 a, Word16 b);
#endif

extern UWord32 ulog2_table[64];
extern UWord32 uexp2_table[32];
extern UWord32 udiv_table[64];
extern UWord32 ursqrt_table[96];
extern UWord32 udiv_128_64_tab[128];
extern Word64 volatile MAC_HI_LO;

#if (defined(__arm__)&&!defined(_MSC_VER))^(defined(__OR32__)||defined(__OR1K__))
typedef union {
	Word64 a;
#ifdef __OR32__
	struct {
		Word32 hi32;
		UWord32 lo32; // big endianness for open-risc
	}b;
#else
	struct {
		UWord32 lo32;
		Word32 hi32;
	}b;
#endif
}U64;

static __inline UWRD128 umul_64_128_arm9e(UWord64 b, UWord64 c) {

	Word32 t; // cpsr
	UWord32 c0, c1;
	UWord32 b0, b1;
	UWord32 a0, a1, a2, a3;
	UWRD128 a;

	b0 = b & 0xffffffff;
	b1 = b >> 32;
	c0 = c & 0xffffffff;
	c1 = c >> 32;

#if 0

#ifdef __GNUC__
	__asm__ __volatile__(
			"umull %[a0], %[a1], %[b0], %[b1]\n\t"
			"mov %[a2], %[zeros]\n\t"
			"umlal %[a1], %[a2], %[b0], %[c1]\n\t"
			"mov %[a3], %[zeros]\n\t"
			"umlal %[a1], %[a3], %[b1], %[c0]\n\t"
			"mov %[b0], %[zeros]\n\t"
			"adds %[a2], %[a2], %[a3]\n\t"
			"adc %[a3], %[b0], %[zeros]\n\t"
			"umlal %[a2], %[a3], %[b1], %[c1]\n\t"
			:[a1]"+r"(a1), [a2]"+r"(a2), [a3]"+r"(a3), [b0]"+r"(b0)
			:[zeros]"I"(0), [a0]"r"(a0), [b1]"r"(b1), [c0]"r"(c0), [c1]"r"(c1)
			:"cc"
	);

#else
	__asm {
		mrs t, CPSR
		umull a0, a1, b0, c0
		mov a2,
#0
		umlal a1, a2, b0, c1
		mov a3,
#0
		umlal a1, a3, b1, c0
		mov b0,
#0
		adds a2, a2, a3
		adc a3, b0,
#0
		umlal a2, a3, b1, c1
		msr CPSR_f, t
	}

#endif
	a.a0 = a0;
	a.a1 = a1;
	a.a2 = a2;
	a.a3 = a3;

#else
	a = umul_64_128(b, c);

#endif
	return (a);
}

static __inline Word16 sat_32_16(Word32 a) { // gcc pass arguments wide, return values narrow
#ifdef __OR32__
	Word32 temp;

	asm volatile("l.addhs %0, %1, r0" :"=r"(temp) :"r"(a) :"cc"); // l.addhs temp, a, #0
	return (temp);// signed int 2 signed short

#elif defined(__arm__)
	Word32 m;
	Word32 b;
	Word32 t;

	m = 0x00007fff;

#ifdef __GNUC__
//asm volatile("mov %0, %1, asr #15" :"=&r"(b) :"r"(a)); // mov b, a, asr #15
//asm volatile("teq %0, %1, asr #31" : :"r"(b), "r"(a) :"cc");// teq b, a, asr #31
//asm volatile("eorne %0, %1, %0, asr #31" :"+r"(a) :"r"(m));

	__asm__ __volatile__(
			"mov %0, %1, asr #15\n\t"
			"teq %0, %1, asr #31\n\t"
			"eorne %1, %2, %1, asr #31\n\t"
			:"+r"(b), "+r"(a)
			:"r"(m)
			:"cc"
	);

#else
	__asm {
		mrs t, CPSR
		mov b, a, asr
#15
		teq b, a, asr
#31
		eorne a, m, a, asr
#31
		msr CPSR_f, t
	}
#endif

	return (a);
#endif
}

static __inline Word32 lsl(Word32 a, Word32 b) {
#ifdef __OR32__
	if(b >= 0) {
		asm volatile("l.slls %0, %0, %1" :"+r"(a) :"r"(b)); // l.slls a, a, b
	} else {
		asm volatile("l.sra %0, %0, %1" :"+r"(a) :"r"(-b)); // l.sra a, a, -b
	}

	return (a);

#elif defined(__arm__)
	Word32 m;
	Word32 c;
	Word32 t;

	m = 0x7fffffff;

	if (b >= 0) {
#ifdef __GNUC__
		asm volatile(
				"mov %0, %1, lsl %2\n\t"
				"teq %1, %0, asr %2\n\t"
				"eorne %0, %3, %1, asr #31\n\t"
				:"+r"(c)
				:"r"(a), "r"(b), "r"(m)
				:"cc"
		);

#else
		__asm {
			mrs t, CPSR
			mov c, a, lsl b
			teq a, c, asr b
			eorne c, m, a, asr
#31
			msr CPSR_f, t
		}
#endif
	} else {
#ifdef __GNUC__
		asm volatile("rsb %0, %0, #0" :"+r"(b));
		// rsb b, b, #0
		asm volatile("mov %0, %1, asr %2" :"=r"(c) :"r"(a), "r"(b));
		// mov c, a, asr b

#else
		__asm {
			rsb b, b,
#0
			mov c, a, asr b
		}
#endif
	}

	return (c);
#endif
}

static __inline Word32 L_add(Word32 L_var1, Word32 L_var2) {
#ifdef __OR32__
	Word32 L_var_out;

	asm volatile("l.adds %0, %1, %2" :"=r"(L_var_out) :"r"(L_var1), "r"(L_var2) :"cc");
	return (L_var_out);

#elif defined(__arm__)
	Word32 L_var_out;

#ifdef __GNUC__
	asm volatile("qadd %0, %1, %2" :"=r"(L_var_out) :"r"(L_var1), "r"(L_var2));
	// qadd L_var_out, L_var1, L_var2
#else
	__asm {
		qadd L_var_out, L_var1, L_var2
	}
#endif

	return (L_var_out);
#endif
}

static __inline Word32 L_sub(Word32 L_var1, Word32 L_var2) {
#ifdef __OR32__
	Word32 L_var_out;

	L_var2 = -L_var2;
	asm volatile("l.adds %0, %1, %2" :"=r"(L_var_out) :"r"(L_var1), "r"(L_var2) :"cc");
	return (L_var_out);

#elif defined(__arm__)
	Word32 L_var_out;

#ifdef __GNUC__
	asm volatile("qsub %0, %1, %2" :"=r"(L_var_out) :"r"(L_var1), "r"(L_var2));
#else
	__asm {
		qsub L_var_out, L_var1, L_var2
	}
#endif

	return (L_var_out);
#endif
}

static __inline Word32 L_mult(Word16 var1, Word16 var2) {
#ifdef __OR32__
	Word32 L_var_out;

//asm volatile("l.mul %0, %1, %2" :"=&r"(L_var_out) :"r"((Word32)var1), "r"((Word32)var2) :"cc");
//asm volatile("l.slls %0, %0, %1" :"+r"(L_var_out) :"r"((Word32)1));

	__asm__ __volatile__(
			"l.mul %0, %1, %2\n\t"
			"l.adds %0, %0, %0\n\t"
			:"+r"(L_var_out)
			:"r"((Word32)var1), "r"((Word32)var2)
			:"cc"
	);

	return (L_var_out);
#elif defined(__arm__)
	Word32 L_var_out;

#ifdef __GNUC__
//asm volatile("smulbb %0, %1, %2" :"=r"(L_var_out) :"r"((Word32)var1), "r"((Word32)var2));
//asm volatile("qadd %0, %0, %0" :"+r"(L_var_out)); // smulbb L_var_out, var1, var2

	__asm__ __volatile__(
			"smulbb %0, %1, %2\n\t"
			"qadd %0, %0, %0\n\t"
			:"+r"(L_var_out)
			:"r"((Word32)var1), "r"((Word32)var2)
	);

#else
	__asm {
		smulbb L_var_out, var1, var2
		qadd L_var_out, L_var_out, L_var_out
	}
#endif

	return (L_var_out);
#endif
}

static __inline Word32 L_mac(Word32 a, Word16 x, Word16 y) {
#ifdef __OR32__
	Word32 temp;

	temp = L_mult(x, y);
	asm volatile ("l.adds %0, %0, %1" :"+r"(a) :"r"(temp) :"cc");
	return (a);

#elif defined(__arm__)
	Word32 temp;

#ifdef __GNUC__
//asm volatile("smulbb %0, %1, %2" :"=r"(temp) :"r"((Word32)x), "r"((Word32)y));
//asm volatile("qdadd %0, %0, %1" :"+r"(a) :"r"(temp));

//more error-prone
	__asm__ __volatile__(
			"smulbb %0, %2, %3\n\t"
			"qdadd %1, %1, %0\n\t"
			:"+r"(temp), "+r"(a)
			:"r"((Word32)x), "r"((Word32)y)
	);

#else
	__asm {
		smulbb temp, x, y
		qdadd a, a, temp
	}
#endif
	return (a);
#endif
}

static __inline Word32 L_msu(Word32 a, Word16 x, Word16 y) {
#ifdef __OR32__
	Word32 temp;

	x = -x;
	temp = L_mult(x, y);
	asm volatile("l.adds %0, %0, %1" :"+r"(a) :"r"(temp) :"cc");
	return (a);

#elif defined(__arm__)
	Word32 temp;

#ifdef __GNUC__
//asm volatile("smulbb %0, %1, %2" :"=r"(temp) :"r"((Word32)x), "r"((Word32)y));
//asm volatile("qdsub %0, %0, %1" :"+r"(a) :"r"(temp));

	__asm__ __volatile__(
			"smulbb %0, %2, %3\n\t"
			"qdsub %1, %1, %0\n\t"
			:"+r"(temp), "+r"(a)
			:"r"((Word32)x), "r"((Word32)y)
	);

#else
	__asm {
		smulbb temp, x, y
		qdsub a, a, temp
	}
#endif
	return (a);
#endif
}

static __inline Word32 CLZ(Word32 a) {
#ifdef __OR32__
	Word32 numZeros;

	asm volatile("l.fl1 %0, %1" :"=r"(numZeros) :"r"(a));
	return (32 - numZeros);

#elif defined(__arm__)
	Word32 numZeros;

#ifdef __GNUC__
	asm volatile("clz %0, %1" :"=r"(numZeros) :"r"(a));
#else
	__asm {
		clz numZeros, a
	}
#endif
	return (numZeros);
#endif
}

static __inline void MLAI(Word64 b) { // 16-bit immediate
#ifdef __OR32__
//UWord32 a, c;
//a = b >> 1;
//c = b & 1;
//asm volatile ("l.maci %0, 1" : :"r"(a));
//asm volatile ("l.maci %0, 1" : :"r"(a));
//asm volatile ("l.maci %0, 1" : :"r"(c));
	U64 u;
	u.a = b;

	asm volatile ("l.mtspr r0, %0, 0x2801" ::"r"(u.b.lo32));
	asm volatile ("l.mtspr r0, %0, 0x2802" ::"r"(u.b.hi32));
#elif defined(__arm__)
	MAC_HI_LO = b;
#endif
}

static __inline void MLA(Word32 a, Word32 b) {
#ifdef __OR32__
	asm volatile ("l.mac %0, %1" ::"r"(a), "r"(b)); // (32*32)%32 + 64
#else
	register Word32 RdLo;
	register Word32 RdHi;
	U64 u;

	u.a = MAC_HI_LO;
	RdLo = u.b.lo32;
	RdHi = u.b.hi32;

#ifdef __GNUC__

	asm volatile("smlal %0, %1, %2, %3" :"+r"(RdLo), "+r"(RdHi) :"r"(a), "r"(b));

#else

	__asm {
		smlal RdLo, RdHi, a, b
	}

#endif
	u.b.lo32 = RdLo;
	u.b.hi32 = RdHi;
	MAC_HI_LO = u.a;
#endif
}

#if defined(__arm__)
static __inline Word32 SMULWB(Word32 x, Word32 y) {
	Word32 register Rd;

#if defined(__GNUC__)
	asm volatile("smulwb %0, %1, %2" :"=r"(Rd) :"r"(x),"r"(y));
#else
	__asm {
		smulwb Rd, x, y
	}
#endif
	return (Rd);
}

static __inline Word32 SMLAWB(Word32 a, Word32 x, Word32 y) {
	Word32 register Rd;
#if defined(__GNUC__)
	asm volatile("smlawb %0, %1, %2, %3" :"=r"(Rd) :"r"(x), "r"(y), "r"(a));
#else
	__asm {
		smlawb Rd, x, y, a
	}
#endif
	return (Rd);
}
#endif

static __inline Word64 SMULL(Word32 x, Word32 y) {
#ifdef __OR32__
	U64 u;
	u.a = 0;
	asm volatile("l.mtspr r0, %0, 0x2801" ::"r"(u.b.lo32));
	asm volatile("l.mtspr r0, %0, 0x2802" ::"r"(u.b.hi32));
	asm volatile("l.mac %0, %1" ::"r"(x), "r"(y));
	asm volatile("l.mfspr %0, r0, 0x2801" :"=r"(u.b.lo32));
	asm volatile("l.macrc %0, 32" :"=r"(u.b.hi32));
	return (u.a);
#else
	Word32 register RdHi;
	Word32 register RdLo;
	U64 u;

#ifdef __GNUC__
	asm volatile("smull %0, %1, %2, %3" :"=r"(RdLo), "=r"(RdHi) :"r"(x), "r"(y));
#else
	__asm {
		smull RdLo, RdHi, x, y
	}
#endif
	u.b.lo32 = RdLo;
	u.b.hi32 = RdHi;

	return (u.a);
#endif
}

static __inline Word64 SMLAL(Word64 a, Word32 x, Word32 y) {
#ifdef __OR32__
	U64 u;
	u.a = a;
	asm volatile("l.mtspr r0, %0, 0x2801" ::"r"(u.b.lo32));
	asm volatile("l.mtspr r0, %0, 0x2802" ::"r"(u.b.hi32));
	asm volatile("l.mac %0, %1" ::"r"(x), "r"(y));
	asm volatile("l.mfspr %0, r0, 0x2801" :"=r"(u.b.lo32));
	asm volatile("l.macrc %0, 32" :"=r"(u.b.hi32));
	return (u.a);
#else
	register Word32 RdHi;
	register Word32 RdLo;
	U64 u;
	u.a = a;
	RdHi = u.b.hi32;
	RdLo = u.b.lo32;

#ifdef __GNUC__
	asm volatile("smlal %0, %1, %2, %3" :"+r"(RdLo), "+r"(RdHi) :"r"(x), "r"(y));
#else
	__asm {
		smlal RdLo, RdHi, x, y
	}
#endif
	u.b.lo32 = RdLo;
	u.b.hi32 = RdHi;

	return (u.a);
#endif
}

static __inline Word32 READ_MAC_SHR0() {
#ifdef __OR32__
	Word32 temp;

	asm volatile ("l.macrc %0, 0" :"=r"(temp));
	return (temp);
#else
	Word32 temp;

	temp = (Word32) MAC_HI_LO;
	MAC_HI_LO = 0;

	return (temp);
#endif
}

#else

static __inline Word16 sat_32_16(Word32 b) {
	Word32 m = 0x00007fff;
	Word32 a;
	a = b >> 15;
#if 0
	if (a == b >> 31)
	return (b);
	else
	return (m ^ (b >> 31));
#else
	return ((a == b >> 31) ? (Word16) b : (Word16) (m ^ (b >> 31)));
#endif
}

static __inline Word32 lsl(Word32 a, Word32 b) { // left shift with saturations
	Word32 m = 0x7fffffff;
	Word32 temp = a;
	if (b >= 0) {
		temp <<= b;
		return ((a == temp >> b) ? temp : m ^ (a >> 31));
	} else {
		return (temp >> (-b));
	}
}

static __inline Word32 L_add(Word32 L_var1, Word32 L_var2) {
	Word32 L_var_out;
	L_var_out = L_var1 + L_var2;
	if (((L_var1 ^ L_var2) & 0x80000000) == 0) { // same sign
		if ((L_var_out ^ L_var1) & 0x80000000) {
			L_var_out = (L_var1 < 0) ? INT_MIN : INT_MAX;
		}
	}
	return (L_var_out);
}

static __inline Word32 L_sub(Word32 L_var1, Word32 L_var2) {
	Word32 L_var_out;
	L_var_out = L_var1 - L_var2;
	if ((L_var1 ^ L_var2) & 0x80000000) {
		if ((L_var_out ^ L_var1) & 0x80000000) {
			L_var_out = (L_var1 > 0) ? INT_MAX : INT_MIN;
		}
	}
	return (L_var_out);
}

static __inline Word32 L_mult(Word16 var1, Word16 var2) {
	Word32 L_var_out;
	L_var_out = var1 * var2; // int * int
	L_var_out = (L_var_out != 0x40000000) ? L_var_out << 1 : INT_MAX;
	return (L_var_out);
}

static __inline Word32 L_mac(Word32 a, Word16 x, Word16 y) {
	Word32 temp;
	temp = L_mult(x, y);
	a = L_add(a, temp);
	return (a);
}

static __inline Word32 L_msu(Word32 a, Word16 x, Word16 y) {
	Word32 temp;

	temp = L_mult(x, y);
	a = L_sub(a, temp);
	return (a);
}

static __inline Word32 CLZ(Word32 a) {
	Word32 numZeros;
	if (!a)
		return (32);
#if defined(__GNUC__)
	numZeros = __builtin_clz(a);
#else
	numZeros = 1; // pre-added
	if (!((UWord32) a >> 16)) {
		numZeros += 16;
		a <<= 16;
	}
	if (!((UWord32) a >> 24)) {
		numZeros += 8;
		a <<= 8;
	}
	if (!((UWord32) a >> 28)) {
		numZeros += 4;
		a <<= 4;
	}
	if (!((UWord32) a >> 30)) {
		numZeros += 2;
		a <<= 2;
	}
	numZeros -= ((UWord32) a >> 31);
#endif
	return (numZeros);
}

static __inline void MLAI(UWord64 b) {
	extern Word64 volatile MAC_HI_LO;
	MAC_HI_LO = b;
}

static __inline void MLA(Word32 a, Word32 b) {
	Word32 temp;
	temp = a * b;
	MAC_HI_LO += (Word64) temp;
}

static __inline Word64 SMULL(Word32 x, Word32 y) {
	return ((Word64) x * y);
}

static __inline Word64 SMLAL(Word64 a, Word32 x, Word32 y) {
	MAC_HI_LO = a;
	MAC_HI_LO += (Word64) x * y;
	a = MAC_HI_LO;
	MAC_HI_LO = 0;
	return (a);
}

static __inline Word32 READ_MAC_SHR0() {
	Word64 temp;

	temp = MAC_HI_LO;
	MAC_HI_LO = 0;
	return (temp & 0xffffffff);
}

#endif

static __inline Word64 umul_32_32(UWord32 b, UWord32 c) {
	UWord16 b1, b0;
	UWord16 c1, c0;
	UWord16 carry;
	UWord32 temp;
	UWord16 a0;
	UWord16 a1;
	UWord16 a2;
	UWord16 a3;

	b0 = b & 0xffff; // b.l
	b1 = b >> 16;
	c0 = c & 0xffff;
	c1 = c >> 16;
	temp = b0 * c0;
	a0 = temp & 0xffff;
	a1 = temp >> 16;
	temp = b0 * c1;
	a2 = temp & 0xffff;
	a3 = temp >> 16;
	temp = b1 * c1;
	c1 = temp & 0xffff;
	b0 = temp >> 16;
	temp = a1 + a2;
	a1 = temp & 0xffff;
	carry = temp >> 16;
	temp = a3 + c1 + carry;
	a2 = temp & 0xffff;
	carry = temp >> 16;
	a3 = b0 + carry;
	temp = b1 * c0;
	c0 = temp & 0xffff;
	b0 = temp >> 16;
	temp = a1 + c0;
	a1 = temp & 0xffff;
	carry = temp >> 16;
	temp = a2 + b0 + carry;
	a2 = temp & 0xffff;
	carry = temp >> 16;
	a3 += carry;
	return (a0 + ((UWord64) a1 << 16) + ((UWord64) a2 << 32)
			+ ((UWord64) a3 << 48));
}

#if !defined(__arm__)||defined(__MSC_VER)
static __inline Word64 mul_64_64(Word64 b, Word64 c) {
	Word32 b1;
	UWord32 b0;
	Word32 c1;
	UWord32 c0;
	Word32 a1;
	UWord32 a0;
	UWord64 temp;

	b0 = b & 0xffffffff; // b[31:0]
	b1 = b >> 32 & 0xffffffff; // b[63:32]
	c0 = c & 0xffffffff; // c[31:0]
	c1 = c >> 32 & 0xffffffff; // c[63:32]
//temp = (UWord64) b0 * c0;
	temp = umul_32_32(b0, c0);
	a0 = temp & 0xffffffff;
	a1 = temp >> 32;
//a1 += (Word32) b0 * c1;
//a1 += b1 * (Word32) c0;
	MLAI(a1);
	MLA(b0, c1);
	MLA(b1, c0);
	a1 = READ_MAC_SHR0();
	return (a0 + ((Word64) a1 << 32)); // 32-bit
}
#endif

static __inline Word32 CLZ64(UWord64 a) {
	Word32 numZeros;
	numZeros = CLZ(a >> 32 & 0xffffffff);
	numZeros += (numZeros == 32) ? CLZ(a & 0xffffffff) : 0;

	return (numZeros);
}

#if !defined(__arm__)||defined(_MSC_VER)
static __inline Word32 asr(Word32 a, Word32 b) {

	if (a) {
		return ((b > 0) ? (a + (1LL << (b - 1))) >> b : lsl(a, -b));
	} else {
		return (0);
	}

}
#endif

static __inline Word32 sat_64_32(Word64 b) {
	Word64 m = 0x7fffffffLL;
	Word64 a;
	a = b >> 31;
	return ((a == b >> 63) ? (Word32) b : (Word32) (m ^ (b >> 63)));
}

static __inline Complex16_t complex_add(Complex16_t a, Complex16_t b) {
	Complex16_t c;
	register Word32 L_var_out;
	//c.real = sat_32_16(a.real + b.real);
	//c.imag = sat_32_16(a.imag + b.imag);
	L_var_out = L_add(a.real << 16, b.real << 16);
	c.real = L_var_out >> 16;
	L_var_out = L_add(a.imag << 16, b.imag << 16);
	c.imag = L_var_out >> 16;

	return (c);
}

static __inline Complex32_t complex32_add(Complex32_t a, Complex32_t b) {
	Complex32_t c;
	register Word32 L_var_out;
	L_var_out = L_add(a.real, b.real);
	c.real = L_var_out;
	L_var_out = L_add(a.imag, b.imag);
	c.imag = L_var_out;
	return (c);
}

static __inline Complex16_t complex_sub(Complex16_t a, Complex16_t b) {
	Complex16_t c;
	register Word32 L_var_out;
	//c.real = sat_32_16(a.real - b.real);
	//c.imag = sat_32_16(a.imag - b.imag);
	L_var_out = L_sub(a.real << 16, b.real << 16);
	c.real = L_var_out >> 16;
	L_var_out = L_sub(a.imag << 16, b.imag << 16);
	c.imag = L_var_out >> 16;
	return (c);
}

static __inline Complex32_t complex32_sub(Complex32_t a, Complex32_t b) {
	Complex32_t c;
	register Word32 L_var_out;
	L_var_out = L_sub(a.real, b.real);
	c.real = L_var_out;
	L_var_out = L_sub(a.imag, b.imag);
	c.imag = L_var_out;
	return (c);
}

#if !defined(__arm__)||defined(_MSC_VER)
static __inline Complex16_t complex_mul(Complex16_t a, Complex16_t b) { // assume b exploits Q15 format
	Complex16_t c; // (-2^15) * (-2^15) + 2^30 = 2^31, 0~2^32-1, 2^31 / 2^15
	//Word64 acc;
	Word32 acc;
	/*c.real = sat_32_16(
	 (Word32) (((Word64) a.real * b.real - a.imag * b.imag + 0x4000)
	 >> 15)); // int * int + int * int = 2^30 * 2 = 2^31, [0~2^32-1]
	 c.imag = sat_32_16(
	 (Word32) (((Word64) a.real * b.imag + a.imag * b.real + 0x4000)
	 >> 15));*/

	acc = L_mac(0, a.real, b.real);
	acc = L_msu(acc, a.imag, b.imag);
	acc = L_add(acc, 0x8000);
	acc >>= 16;
	c.real = acc;

	acc = L_mac(0, a.real, b.imag);
	acc = L_mac(acc, a.imag, b.real);
	acc = L_add(acc, 0x8000);
	acc >>= 16;
	c.imag = acc;

	/*acc = SMULL(a.real, b.real);
	 acc = SMLAL(acc, -a.imag, b.imag);
	 acc += 0x4000; // int
	 acc >>= 15;
	 c.real = sat_32_16(acc);
	 acc = SMULL(a.real, b.imag);
	 acc = SMLAL(acc, a.imag, b.real);
	 acc += 0x4000;
	 acc >>= 15;
	 c.imag = sat_32_16(acc);*/

	return (c);
}
#endif

#if !defined(__arm__)||defined(_MSC_VER)
static __inline Complex32_t complex32_mul(Complex32_t * __restrict a,
		Complex32_t * __restrict b) {
	Complex32_t c;
	Word64 acc;
	acc = SMULL(a->real, b->real); // 2^31 * 2^31 = 2^62
	acc = SMLAL(acc, -a->imag, b->imag);
	acc += 0x4000; // int
	acc >>= 15;
	c.real = sat_64_32(acc);
	acc = SMULL(a->real, b->imag);
	acc = SMLAL(acc, a->imag, b->real);
	acc += 0x4000;
	acc >>= 15;
	c.imag = sat_64_32(acc);
	return (c);
}
#endif

static __inline Complex16_t complex_conj(Complex16_t a) {
	Complex16_t c;
	c.real = a.real;
	c.imag = -a.imag;
	return (c);
}

static __inline Complex16_t complex_swap(Complex16_t a) { // * -j
	Complex16_t c;
	c.real = a.imag;
	c.imag = -a.real;
	return (c);
}

#if !defined(__arm__)||defined(_MSC_VER)
static __inline Complex16_t complex_asr(Complex16_t a, Word32 b) {
#if 0
//a.real = (b > 0) ? (a.real + (1 << (b - 1))) >> b : sat_32_16(lsl(a.real, -b));
//a.imag = (b > 0) ? (a.imag + (1 << (b - 1))) >> b : sat_32_16(lsl(a.imag, -b));

	a.real =
	(b > 0) ?
	((Word32) a.real & 1 << (b - 1)) ?
	(a.real >> b) + 1 : a.real >> b
	: sat_32_16(lsl(a.real, -b));
	a.imag =
	(b > 0) ?
	((Word32) a.imag & 1 << (b - 1)) ?
	(a.imag >> b) + 1 : a.imag >> b
	: sat_32_16(lsl(a.imag, -b));

#else
	a.real = (b > 0) ? a.real >> b : sat_32_16(lsl(a.real, -b));
	a.imag = (b > 0) ? a.imag >> b : sat_32_16(lsl(a.imag, -b));
#endif

	return (a);
}
#endif

#if !defined(__arm__)||defined(_MSC_VER)
static __inline Word16 exp_adj(Word16 a, Word16 b) { // counting leading zeros
	Word32 se;
	Word32 temp;

#if 0
	temp = (a < 0) ? ~a : a;
	se = CLZ(temp << 16);
	se = 1 - se;
#else
	temp = a << 16;
	se = temp ^ temp << 1;
	se = -CLZ(se);
#endif
	return ((se > b) ? se : b);
}
#endif

#if !defined(__arm__)||defined(_MSC_VER)
static __inline void btr_fly_r2(Complex16_t * __restrict a,
		Complex16_t * __restrict b) {
#if 0
	a->real = L_add(a->real << 16, b->real << 16) >> 16;
	a->imag = L_add(a->imag << 16, b->imag << 16) >> 16;
	b->real = sat_32_16(L_msu(a->real, 1L, b->real));
	b->imag = sat_32_16(L_msu(a->imag, 1L, b->imag));
#else
	assert(((uintptr_t ) a & 0x3) == 0);
	assert(((uintptr_t ) b & 0x3) == 0);
#if 1
	b->real = L_sub(a->real << 16, b->real << 16) >> 16;
	b->imag = L_sub(a->imag << 16, b->imag << 16) >> 16;
	a->real = L_add(L_sub(a->real << 16, b->real << 16), a->real << 16) >> 16;
	a->imag = L_add(L_sub(a->imag << 16, b->imag << 16), a->imag << 16) >> 16;
#else
	UWord32 c, d;
	c = ((UWord32*) a)[0] + ((UWord32*) b)[0];
	d = ((UWord32*) a)[0] - ((UWord32*) b)[0];
	((UWord32*) a)[0] = c;
	((UWord32*) b)[0] = d;
#endif
#endif
}
#endif

static __inline Word32 btr_fly_r4(Complex16_t *a, Complex16_t *b,
		Complex16_t *c, Complex16_t *d, Word32 sb) {

	btr_fly_r2(&a[0], &c[0]);
	btr_fly_r2(&b[0], &d[0]);

	d[0] = complex_swap(d[0]); // * -j
	btr_fly_r2(&a[0], &b[0]);
	btr_fly_r2(&c[0], &d[0]);

	sb = exp_adj(a[0].real, sb); // bfp scaling
	sb = exp_adj(a[0].imag, sb); // neg(clz - 1)
	return (sb);
}

#endif /* BASIC_OP_H_ */

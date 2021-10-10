/*
 */
#ifndef MATHOPS_H
#define MATHOPS_H

#include "common.h"
#include "config.h"

#define  CRC_ERR_FAIL_CRC1          -1
#define  CRC_ERR_FAIL_CRC2          -2

#  if defined(__OR32__)
static __inline int32_t MUL_Shift_30(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 30" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_18(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 18" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_32(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 32" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_24(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 24" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_26(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 26" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_27(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 27" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_28(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 28" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_29(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 29" : "=r"(result) );
    return result;
}

static __inline int32_t MUL_Shift_23(int32_t a, int32_t b, int32_t s) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 23" : "=r"(result) );
    return result;
}

static __inline int32_t MUL(int32_t a, int32_t b) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.macrc %0, 30" : "=r"(result) );
    return result;
}
static __inline int32_t MUL_Add(int32_t a, int32_t b, int32_t c, int32_t d) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.mac %0, %1" : : "%r"(c), "r"(d));
    asm volatile("l.macrc %0, 30" : "=r"(result) );
    return result;
}
static __inline int32_t MUL_Sub(int32_t a, int32_t b, int32_t c, int32_t d) {
    int result;
    asm volatile("l.mac %0, %1" : : "%r"(a), "r"(b));
    asm volatile("l.msb %0, %1" : : "%r"(c), "r"(d));
    asm volatile("l.macrc %0, 30" : "=r"(result) );
    return result;
}

static __inline int CLZ(int x)
{
    int numZeros;
    asm volatile ("l.fl1 %0, %1" : "=r" (numZeros) : "r"(x));
    return 32 - numZeros;
}
#if 1
static __inline int32_t SATSHIFT(int32_t x, uint32_t n)
{
    //register int ret;
    //asm volatile ("l.slls %0,%1,%2" : "=r"(ret) : "r"(x) : "r"(n)); 
    if(x>=(signed int)(1<<(31-n))) 
    	x = (signed int)0x7fffffff;
    else if(x<(signed int)(-1<<(31-n))) 
    	x = (signed int)0x80000000;
    else x<<=n;
    return x;

}
#else
#define SATSHIFT(y, n) ({ \
    register int ret;   \
    asm volatile ("l.slls %0,%1,%2" : "=r"(ret) : "r"(y), "r"(n)); \
    ret;    \
})
#endif

#define CLIPTOSHORT(x)({  \
    register int ret;   \
    asm volatile ("l.addhs %0,%1,r0" : "=r"(ret) : "r"(x)); \
    ret;    \
})
/*
#define SATSHIFT(x, n)    ({  \
    register int ret;   \
	asm volatile ("l.slls %0,%1,%2" : "=r"(ret) : "r"(x) : "r"(n)); \
    ret;    \
})*/

#  else

#  define MUL_Shift(a,b,s) ((int)((((int64_t)(a) * (b))/*+(1<<(s-1))*/) >> s))
#  define MUL(a,b) ((int)((((int64_t)(a) * (b))/* + 0x20000000*/) >> 30))
#  define MUL_Add(a,b,c,d) ((int)(((((int64_t)(a) * (b)) + ((int64_t)(c) * (d)))/* + 0x20000000*/)  >> 30))
#  define MUL_Sub(a,b,c,d) ((int)(((((int64_t)(a) * (b)) - ((int64_t)(c) * (d)))/* + 0x20000000*/) >> 30))

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return 32;

    /* count leading zeros with binary search */
    numZeros = 1;
    if (!((unsigned int)x >> 16)) { numZeros += 16; x <<= 16; }
    if (!((unsigned int)x >> 24)) { numZeros +=  8; x <<=  8; }
    if (!((unsigned int)x >> 28)) { numZeros +=  4; x <<=  4; }
    if (!((unsigned int)x >> 30)) { numZeros +=  2; x <<=  2; }

    numZeros -= ((uint32_t)x >> 31);

    return numZeros;
}

static __inline int16_t CLIPTOSHORT(int32_t x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
	if (sign != (x >> 15)) {
        x = sign ^ ((1 << 15) - 1);
	}
    return (int16_t)x;
}


static __inline int32_t SATSHIFT(int32_t x, uint32_t n)
{
	if(x>=(signed int)(1<<(31-n))) 
		x = (signed int)0x7fffffff;
	else if(x<(signed int)(-1<<(31-n))) 
		x = (signed int)0x80000000;
	else x<<=n;
    return x;
}

#  endif // __OR32__

int InvRNormalized(int r);
int sqrtfix(int q, int fBitsIn, int *fBitsOut);
int dither_gen(int type, int16_t *dith_state);
/*!
****************************************************************
*   This function is used by encoders and decoders to calculate
*   the CRC word and check for corrupted bitstreams, respectively.
*   Calling functions are responsible for acting on the value output
*   CRC syndrome.
*
*	\return
*   - #ERR_NO_ERROR if no errors occurrred.
*****************************************************************/
short crc_calcfwd(
					const short	*p_inbuf,			/*!< \in: Pointer to frame buffer	*/
					const short	crc_nwords,			/*!< \in: Number of words to use in calculation	*/
					short		*p_crc_syndrome  	/*!< \out:
															 -# When called by a decoder, a non-zero syndrome value
															    means the bitstream is corrupt.
															 -# When called by an encoder, the sydnrome value is
															    used as the CRC word. */
				  );
/*!
****************************************************************
*	This function performs a CRC check on a Dolby Digital (DD)
*	frame.  It checks both <i>crc1</i> and <i>crc2</i> and returns
*   which ones fail.
*
*	\return
*   - #CRC_ERR_FAIL_CRC1 if crc1 fails.
*   - #CRC_ERR_FAIL_CRC2 if crc2 fails.
*   - #ERR_NO_ERROR if both crcs pass.
*****************************************************************/
short crc_chkddfrm
(
	short		frm_nwords,		/*!< \in: The number of words in the DD frame. */
	const short *p_ddfrmbuf		/*!< \in: Pointer to a complete DD frame.*/
);
#endif /* MATHOPS_H */


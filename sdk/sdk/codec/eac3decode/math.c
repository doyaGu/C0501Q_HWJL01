/*
*/
#include "math.h"

#define Q28_2   0x20000000  /* Q28: 2.0 */
#define Q28_15  0x30000000  /* Q28: 1.5 */

#define NUM_ITER_IRN        5
static const unsigned short crcfwdtab[] =
{   0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
    0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
    0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
    0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
    0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
    0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
    0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
    0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
    0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
    0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
    0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
    0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
    0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
    0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
    0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
    0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
    0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202 };

/**************************************************************************************
 * Function:    InvRNormalized
 *
 * Description: use Newton's method to solve for x = 1/r
 *
 * Inputs:      r = Q31, range = [0.5, 1) (normalize your inputs to this range)
 *
 * Outputs:     none
 *
 * Return:      x = Q29, range ~= [1.0, 2.0]
 *
 * Notes:       guaranteed to converge and not overflow for any r in [0.5, 1)
 *
 *              xn+1  = xn - f(xn)/f'(xn)
 *              f(x)  = 1/r - x = 0 (find root)
 *                    = 1/x - r
 *              f'(x) = -1/x^2
 *
 *              so xn+1 = xn - (1/xn - r) / (-1/xn^2)
 *                      = xn * (2 - r*xn)
 *
 *              NUM_ITER_IRN = 2, maxDiff = 6.2500e-02 (precision of about 4 bits)
 *              NUM_ITER_IRN = 3, maxDiff = 3.9063e-03 (precision of about 8 bits)
 *              NUM_ITER_IRN = 4, maxDiff = 1.5288e-05 (precision of about 16 bits)
 *              NUM_ITER_IRN = 5, maxDiff = 3.0034e-08 (precision of about 24 bits)
 **************************************************************************************/
int InvRNormalized(int r)
{
    int i, xn, t;

    /* r =   [0.5, 1.0)
     * 1/r = (1.0, 2.0]
     *   so use 1.5 as initial guess
     */
    xn = Q28_15;

    /* xn = xn*(2.0 - r*xn) */
    for (i = NUM_ITER_IRN; i != 0; i--) {
        t = MUL_Shift_32(r, xn, 32);          /* Q31*Q29 = Q28 */
        t = Q28_2 - t;                  /* Q28 */
        xn = MUL_Shift_28(xn, t, 28);    /* Q29*Q28 << 4 = Q29 */
    }

    return xn;
}

/**************************************************************************************
 * Function:    SqrtFix
 *
 * Description: use binary search to calculate sqrt(q)
 *
 * Inputs:      q = Q30
 *              number of fraction bits in input
 *
 * Outputs:     number of fraction bits in output
 *
 * Return:      lo = Q(fBitsOut)
 *
 * Notes:       absolute precision varies depending on fBitsIn
 *              normalizes input to range [0x200000000, 0x7fffffff] and takes
 *                floor(sqrt(input)), and sets fBitsOut appropriately
 **************************************************************************************/
int sqrtfix(int q, int fBitsIn, int *fBitsOut)
{
    int z, lo, hi, mid;

    if (q <= 0) {
        *fBitsOut = fBitsIn;
        return 0;
    }

    /* force even fBitsIn */
    z = fBitsIn & 0x01;
    q >>= z;
    fBitsIn -= z;

    /* for max precision, normalize to [0x20000000, 0x7fffffff] */
    z = (CLZ(q) - 1);
    z >>= 1;
    q <<= (2*z);

    /* choose initial bounds */
    lo = 1;
    if (q >= 0x10000000)
        lo = 16384; /* (int)sqrt(0x10000000) */
    hi = 46340;     /* (int)sqrt(0x7fffffff) */

    /* do binary search with 32x32->32 multiply test */
    do {
        mid = (lo + hi) >> 1;
        if (mid*mid > q)
            hi = mid - 1;
        else
            lo = mid + 1;
    } while (hi >= lo);
    lo--;

    *fBitsOut = ((fBitsIn + 2*z) >> 1);
    return lo;
}

#define DSP_DITHMULT    47989                   /*!< Linear congruential multiplier */

int dither_gen(int type, int16_t *dith_state)
{
    int dithscale;
    int tmp_noise;
    int noise;

    *dith_state = (int16_t)((DSP_DITHMULT * (*dith_state)) & 0xffff);
    noise = *dith_state<<16;

    if(type==0) {
        noise = noise>>1;
        *dith_state = (int16_t)((DSP_DITHMULT * (*dith_state)) & 0xffff);
        tmp_noise = *dith_state<<15;
        noise = noise+tmp_noise;
    }        
    return noise>>(31-QUANT_SHIFT);
}

#define     GBL_5_8THS(n)           (((n) >> 3) + ((n) >> 1))
/*****************************************************************
* crc_calcfwd:
*****************************************************************/
short crc_calcfwd(
            const short*    p_inbuf,            /* input    */
            const short     crc_nwords,         /* input    */
            short *         p_crc_syndrome)     /* output   */
{
    /* Declare local variables */
    int  i;

    /* Initialize syndrome to zero */
    *p_crc_syndrome = 0;

    /* Perform cyclic redundancy check */
    for (i = 0; i < crc_nwords; i++)
    {
        *p_crc_syndrome = (short)(((*p_crc_syndrome << 8) & 0xff00)
            ^ ((p_inbuf[i] >> 8) & 0xff) ^ crcfwdtab[(*p_crc_syndrome >> 8) & 0xff]);
        *p_crc_syndrome = (short)(((*p_crc_syndrome << 8) & 0xff00)
            ^ (p_inbuf[i] & 0xff) ^ crcfwdtab[(*p_crc_syndrome >> 8) & 0xff]);
    }

    /* Check result of CRC calculation */
    return 0;
}

/*
    See crc_chkddfrm.h for a description on calling this routine.
*/
short crc_chkddfrm(short frm_nwords,const short *p_ddfrmbuf )
{
    short   frm_5_8ths;
    short   crc_syndrome;
    short   err;

    /* Derive number of words in 5/8ths of frame */

    frm_5_8ths = GBL_5_8THS(frm_nwords) - 1;

    /* Perform CRC checking on first 5/8ths of frame */
    err = crc_calcfwd(p_ddfrmbuf + 1, frm_5_8ths, &crc_syndrome);

    /* If CRC failed on first 5/8ths of frame, return full frame error */
    if (crc_syndrome != 0)
        return CRC_ERR_FAIL_CRC1;

    /* Perform CRC check on last 3/8ths of frame */
    err = crc_calcfwd(p_ddfrmbuf + frm_5_8ths + 1, frm_nwords - frm_5_8ths - 1, &crc_syndrome);

    /* If CRC failed on last 5/8th of frame, return partial frame error */
    if (crc_syndrome != 0)
        return CRC_ERR_FAIL_CRC2;

    return 0;
}


#include <string.h>
#include "config.h"
#include "ps_dec.h"
#include "fixed.h"
#include "ps_tables.h"


#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif

#ifdef PARSING_HE_AAC_V2
/* constants */
#define NEGATE_IPD_MASK            (0x1000)
#define DECAY_SLOPE                FRAC_CONST(0.05)
//#define  COEF_SQRT2                 COEF_CONST(1.4142135623731)


typedef struct 
{
    uint8_t frame_len;
    uint8_t resolution20[3];
    uint8_t resolution34[5];

    qmf_t work[44];
    qmf_t buffer[5][32];
    qmf_t temp[32][12];
} hyb_info;

/* tables */
/* filters are mirrored in coef 6, second half left out */
static const real_t p8_13_20[7] =
{
    FRAC_CONST(0.00746082949812),
    FRAC_CONST(0.02270420949825),
    FRAC_CONST(0.04546865930473),
    FRAC_CONST(0.07266113929591),
    FRAC_CONST(0.09885108575264),
    FRAC_CONST(0.11793710567217),
    FRAC_CONST(0.125)
};

static const real_t p2_13_20[7] =
{
    FRAC_CONST(0.0),
    FRAC_CONST(0.01899487526049),
    FRAC_CONST(0.0),
    FRAC_CONST(-0.07293139167538),
    FRAC_CONST(0.0),
    FRAC_CONST(0.30596630545168),
    FRAC_CONST(0.5)
};

static const real_t p12_13_34[7] =
{
    FRAC_CONST(0.04081179924692),
    FRAC_CONST(0.03812810994926),
    FRAC_CONST(0.05144908135699),
    FRAC_CONST(0.06399831151592),
    FRAC_CONST(0.07428313801106),
    FRAC_CONST(0.08100347892914),
    FRAC_CONST(0.08333333333333)
};

static const real_t p8_13_34[7] =
{
    FRAC_CONST(0.01565675600122),
    FRAC_CONST(0.03752716391991),
    FRAC_CONST(0.05417891378782),
    FRAC_CONST(0.08417044116767),
    FRAC_CONST(0.10307344158036),
    FRAC_CONST(0.12222452249753),
    FRAC_CONST(0.125)
};

static const real_t p4_13_34[7] =
{
    FRAC_CONST(-0.05908211155639),
    FRAC_CONST(-0.04871498374946),
    FRAC_CONST(0.0),
    FRAC_CONST(0.07778723915851),
    FRAC_CONST(0.16486303567403),
    FRAC_CONST(0.23279856662996),
    FRAC_CONST(0.25)
};

#ifdef PARAM_32KHZ
static const uint8_t delay_length_d[2][NO_ALLPASS_LINKS] = {
    { 1, 2, 3 } /* d_24kHz */,
    { 3, 4, 5 } /* d_48kHz */
};
#else
static const uint8_t delay_length_d[NO_ALLPASS_LINKS] = {
    3, 4, 5 /* d_48kHz */
};
#endif
static const real_t filter_a[NO_ALLPASS_LINKS] = { /* a(m) = exp(-d_48kHz(m)/7) */
    FRAC_CONST(0.65143905753106),
    FRAC_CONST(0.56471812200776),
    FRAC_CONST(0.48954165955695)
};

static const uint8_t group_border20[10+12 + 1] =
{
    6, 7, 0, 1, 2, 3, /* 6 subqmf subbands */
    9, 8,             /* 2 subqmf subbands */
    10, 11,           /* 2 subqmf subbands */
    3, 4, 5, 6, 7, 8, 9, 11, 14, 18, 23, 35, 64
};

static const uint8_t group_border34[32+18 + 1] =
{
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, /* 12 subqmf subbands */
     12, 13, 14, 15, 16, 17, 18, 19,                 /*  8 subqmf subbands */
     20, 21, 22, 23,                                 /*  4 subqmf subbands */
     24, 25, 26, 27,                                 /*  4 subqmf subbands */
     28, 29, 30, 31,                                 /*  4 subqmf subbands */
     32-27, 33-27, 34-27, 35-27, 36-27, 37-27, 38-27, 40-27, 42-27, 44-27, 46-27, 48-27, 51-27, 54-27, 57-27, 60-27, 64-27, 68-27, 91-27
};

static const uint16_t map_group2bk20[10+12] =
{
    (NEGATE_IPD_MASK | 1), (NEGATE_IPD_MASK | 0),
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
};

static const uint16_t map_group2bk34[32+18] =
{
    0,  1,  2,  3,  4,  5,  6,  6,  7, (NEGATE_IPD_MASK | 2), (NEGATE_IPD_MASK | 1), (NEGATE_IPD_MASK | 0),
    10, 10, 4,  5,  6,  7,  8,  9,
    10, 11, 12, 9,
    14, 11, 12, 13,
    14, 15, 16, 13,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
};

static const int channel_filter2IdxTab0[32][13] = 
{ {  0, 1, 2, 3, 4,5,6, 7, 8, 9 , 10 , 11 , 12},
   {  0+1, 1+1, 2+1, 3+1, 4+1,5+1,6+1, 7+1, 8+1, 9+1 , 10+1 , 11+1 , 12+1},
   {  0+2, 1+2, 2+2, 3+2, 4+2,5+2,6+2, 7+2, 8+2, 9+2 , 10+2 , 11+2 , 12+2},
   {  0+3, 1+3, 2+3, 3+3, 4+3,5+3,6+3, 7+3, 8+3, 9+3 , 10+3 , 11+3 , 12+3},
   {  0+4, 1+4, 2+4, 3+4, 4+4,5+4,6+4, 7+4, 8+4, 9+4 , 10+4 , 11+4 , 12+4},
   {  0+5, 1+5, 2+5, 3+5, 4+5,5+5,6+5, 7+5, 8+5, 9+5 , 10+5 , 11+5 , 12+5},
   {  0+6, 1+6, 2+6, 3+6, 4+6,5+6,6+6, 7+6, 8+6, 9+6 , 10+6 , 11+6 , 12+6},
   {  0+7, 1+7, 2+7, 3+7, 4+7,5+7,6+7, 7+7, 8+7, 9+7 , 10+7 , 11+7 , 12+7},
   {  0+8, 1+8, 2+8, 3+8, 4+8,5+8,6+8, 7+8, 8+8, 9+8 , 10+8 , 11+8 , 12+8},
   {  0+9, 1+9, 2+9, 3+9, 4+9,5+9,6+9, 7+9, 8+9, 9+9 , 10+9 , 11+9 , 12+9},
   {  0+10, 1+10, 2+10, 3+10, 4+10,5+10,6+10, 7+10, 8+10, 9+10 , 10+10 , 11+10 , 12+10},
   {  0+11, 1+11, 2+11, 3+11, 4+11,5+11,6+11, 7+11, 8+11, 9+11 , 10+11 , 11+11 , 12+11},
   {  0+12, 1+12, 2+12, 3+12, 4+12,5+12,6+12, 7+12, 8+12, 9+12 , 10+12 , 11+12 , 12+12},
   {  0+13, 1+13, 2+13, 3+13, 4+13,5+13,6+13, 7+13, 8+13, 9+13 , 10+13 , 11+13 , 12+13},
   {  0+14, 1+14, 2+14, 3+14, 4+14,5+14,6+14, 7+14, 8+14, 9+14 , 10+14 , 11+14 , 12+14},
   {  0+15, 1+15, 2+15, 3+15, 4+15,5+15,6+15, 7+15, 8+15, 9+15 , 10+15 , 11+15 , 12+15},
   {  0+16, 1+16, 2+16, 3+16, 4+16,5+16,6+16, 7+16, 8+16, 9+16 , 10+16 , 11+16 , 12+16},
   {  0+17, 1+17, 2+17, 3+17, 4+17,5+17,6+17, 7+17, 8+17, 9+17 , 10+17 , 11+17 , 12+17},
   {  0+18, 1+18, 2+18, 3+18, 4+18,5+18,6+18, 7+18, 8+18, 9+18 , 10+18 , 11+18 , 12+18},
   {  0+19, 1+19, 2+19, 3+19, 4+19,5+19,6+19, 7+19, 8+19, 9+19 , 10+19 , 11+19 , 12+19},  
   {  0+20, 1+20, 2+20, 3+20, 4+20,5+20,6+20, 7+20, 8+20, 9+20 , 10+20 , 11+20 , 12+20},
   {  0+21, 1+21, 2+21, 3+21, 4+21,5+21,6+21, 7+21, 8+21, 9+21 , 10+21 , 11+21 , 12+21},
   {  0+22, 1+22, 2+22, 3+22, 4+22,5+22,6+22, 7+22, 8+22, 9+22 , 10+22 , 11+22 , 12+22},
   {  0+23, 1+23, 2+23, 3+23, 4+23,5+23,6+23, 7+23, 8+23, 9+23 , 10+23 , 11+23 , 12+23},
   {  0+24, 1+24, 2+24, 3+24, 4+24,5+24,6+24, 7+24, 8+24, 9+24 , 10+24 , 11+24 , 12+24},
   {  0+25, 1+25, 2+25, 3+25, 4+25,5+25,6+25, 7+25, 8+25, 9+25 , 10+25 , 11+25 , 12+25},
   {  0+26, 1+26, 2+26, 3+26, 4+26,5+26,6+26, 7+26, 8+26, 9+26 , 10+26 , 11+26 , 12+26},
   {  0+27, 1+27, 2+27, 3+27, 4+27,5+27,6+27, 7+27, 8+27, 9+27 , 10+27 , 11+27 , 12+27},
   {  0+28, 1+28, 2+28, 3+28, 4+28,5+28,6+28, 7+28, 8+28, 9+28 , 10+28 , 11+28 , 12+28},
   {  0+29, 1+29, 2+29, 3+29, 4+29,5+29,6+29, 7+29, 8+29, 9+29 , 10+29 , 11+29 , 12+29}, 
   {  0+30, 1+30, 2+30, 3+30, 4+30,5+30,6+30, 7+30, 8+30, 9+30 , 10+30 , 11+30 , 12+30},
   {  0+31, 1+31, 2+31, 3+31, 4+31,5+31,6+31, 7+31, 8+31, 9+31 , 10+31 , 11+31 , 12+31}       
   };

static const real_t nChannel_filter4_input_re1 =  FRAC_CONST(-0.70710678118655);
static const real_t nChannel_filter4_input_re2  = FRAC_CONST(0.70710678118655);
static const real_t nChannel_filter4_input_im1 =  FRAC_CONST(0.70710678118655);
static const real_t nChannel_filter4_input_im2 =  FRAC_CONST(-0.70710678118655);
static const real_t nDCT3_4_unscaled_f0 =  FRAC_CONST(0.7071067811865476);
static const real_t nDCT3_4_unscaled_f4 =  COEF_CONST(1.3065629648763766);
static const real_t nDCT3_4_unscaled_f5 =  FRAC_CONST(-0.9238795325112866);
static const real_t nDCT3_4_unscaled_f6 =  FRAC_CONST(-0.5411961001461967);

static const real_t nDCT3_6_unscaled_f0 =  FRAC_CONST(0.70710678118655);
static const real_t nDCT3_6_unscaled_f3 =  FRAC_CONST(0.70710678118655);
static const real_t nDCT3_6_unscaled_f4 =  FRAC_CONST(0.86602540378444);
static const real_t nDCT3_6_unscaled_f4_1 = FRAC_CONST(0.5);
static const real_t nDCT3_6_unscaled_f6_1 =  FRAC_CONST(0.96592582628907);
static const real_t nDCT3_6_unscaled_f6_5 =  FRAC_CONST(0.25881904510252);

//static const real_t COEF_SQRT2 = COEF_CONST(1.4142135623731);

/* static function declarations */
 void ps_data_decode(ps_info *ps);
 hyb_info *hybrid_init(uint8_t numTimeSlotsRate);
 void channel_filter2(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                            qmf_t *buffer, qmf_t X_hybrid[32][12]);
 void INLINE DCT3_4_unscaled(real_t *y, real_t *x);
 void channel_filter8(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                            qmf_t *buffer, qmf_t X_hybrid[32][12]);
 void hybrid_analysis(hyb_info *hyb, qmf_t X[32][64], qmf_t X_hybrid[32][32],
                            uint8_t use34, uint8_t numTimeSlotsRate);
 void hybrid_synthesis(hyb_info *hyb, qmf_t X[32][64], qmf_t X_hybrid[32][32],
                             uint8_t use34, uint8_t numTimeSlotsRate);
 int8_t delta_clip(int8_t i, int8_t min, int8_t max);
 void delta_decode(uint8_t enable, int8_t *index, int8_t *index_prev,
                         uint8_t dt_flag, uint8_t nr_par, uint8_t stride,
                         int8_t min_index, int8_t max_index);
 void delta_modulo_decode(uint8_t enable, int8_t *index, int8_t *index_prev,
                                uint8_t dt_flag, uint8_t nr_par, uint8_t stride,
                                int8_t and_modulo);
 void map20indexto34(int8_t *index, uint8_t bins);
#ifdef PS_LOW_POWER
static void map34indexto20(int8_t *index, uint8_t bins);
#endif
 void ps_data_decode(ps_info *ps);
 void ps_decorrelate(ps_info *ps, qmf_t X_left[38][64], qmf_t X_right[38][64],
                           qmf_t X_hybrid_left[32][32], qmf_t X_hybrid_right[32][32]);
 void ps_mix_phase(ps_info *ps, qmf_t X_left[38][64], qmf_t X_right[38][64],
                         qmf_t X_hybrid_left[32][32], qmf_t X_hybrid_right[32][32]);
static ps_info gPS_Info;

static hyb_info gHyb_info;

/* real filter, size 2 */
 void channel_filter2(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                            qmf_t *buffer, qmf_t X_hybrid[32][12])
{
    uint8_t i;
    uint8_t i1,i11;
    real_t r135;    
    real_t r0246;

    real_t i135;    
    real_t i0246;

#if 0
    for (i = 0; i < frame_len; i++)
    {
        real_t r0 = MUL_F(filter[0],(QMF_RE(buffer[0+i]) + QMF_RE(buffer[12+i])));
        real_t r1 = MUL_F(filter[1],(QMF_RE(buffer[1+i]) + QMF_RE(buffer[11+i])));
        real_t r2 = MUL_F(filter[2],(QMF_RE(buffer[2+i]) + QMF_RE(buffer[10+i])));
        real_t r3 = MUL_F(filter[3],(QMF_RE(buffer[3+i]) + QMF_RE(buffer[9+i])));
        real_t r4 = MUL_F(filter[4],(QMF_RE(buffer[4+i]) + QMF_RE(buffer[8+i])));
        real_t r5 = MUL_F(filter[5],(QMF_RE(buffer[5+i]) + QMF_RE(buffer[7+i])));
        real_t r6 = MUL_F(filter[6],QMF_RE(buffer[6+i]));
        real_t i0 = MUL_F(filter[0],(QMF_IM(buffer[0+i]) + QMF_IM(buffer[12+i])));
        real_t i1 = MUL_F(filter[1],(QMF_IM(buffer[1+i]) + QMF_IM(buffer[11+i])));
        real_t i2 = MUL_F(filter[2],(QMF_IM(buffer[2+i]) + QMF_IM(buffer[10+i])));
        real_t i3 = MUL_F(filter[3],(QMF_IM(buffer[3+i]) + QMF_IM(buffer[9+i])));
        real_t i4 = MUL_F(filter[4],(QMF_IM(buffer[4+i]) + QMF_IM(buffer[8+i])));
        real_t i5 = MUL_F(filter[5],(QMF_IM(buffer[5+i]) + QMF_IM(buffer[7+i])));
        real_t i6 = MUL_F(filter[6],QMF_IM(buffer[6+i]));

        /* q = 0 */
        QMF_RE(X_hybrid[i][0]) = r0 + r1 + r2 + r3 + r4 + r5 + r6;
        QMF_IM(X_hybrid[i][0]) = i0 + i1 + i2 + i3 + i4 + i5 + i6;

        /* q = 1 */
        QMF_RE(X_hybrid[i][1]) = r0 - r1 + r2 - r3 + r4 - r5 + r6;
        QMF_IM(X_hybrid[i][1]) = i0 - i1 + i2 - i3 + i4 - i5 + i6;
    }
#else

    for (i = 0; i < frame_len; i++)
    {   
       //asm volatile("l.nop 0x1223");               
        r135 = MUL_F_ADD_6(filter[1],(QMF_RE(buffer[1+i]) + QMF_RE(buffer[11+i])),filter[3],(QMF_RE(buffer[3+i]) + QMF_RE(buffer[9+i])),filter[5],(QMF_RE(buffer[5+i]) + QMF_RE(buffer[7+i])));
        r0246 = MUL_F_ADD_8(filter[0],(QMF_RE(buffer[0+i]) + QMF_RE(buffer[12+i])),filter[2],(QMF_RE(buffer[2+i]) + QMF_RE(buffer[10+i])),filter[4],(QMF_RE(buffer[4+i]) + QMF_RE(buffer[8+i])),filter[6],QMF_RE(buffer[6+i]));
        i135 = MUL_F_ADD_6(filter[1],(QMF_IM(buffer[1+i]) + QMF_IM(buffer[11+i])),filter[3],(QMF_IM(buffer[3+i]) + QMF_IM(buffer[9+i])),filter[5],(QMF_IM(buffer[5+i]) + QMF_IM(buffer[7+i])));
        i0246 = MUL_F_ADD_8(filter[0],(QMF_IM(buffer[0+i]) + QMF_IM(buffer[12+i])),filter[2],(QMF_IM(buffer[2+i]) + QMF_IM(buffer[10+i])),filter[4],(QMF_IM(buffer[4+i]) + QMF_IM(buffer[8+i])),filter[6],QMF_IM(buffer[6+i]));
        
        // q = 0 
        QMF_RE(X_hybrid[i][0]) = r135+r0246;
        QMF_IM(X_hybrid[i][0]) = i135+i0246;

        // q = 1 
        QMF_RE(X_hybrid[i][1]) = r0246 - r135;
        QMF_IM(X_hybrid[i][1]) = i0246 - i135;
    }


/*
    for (i = 0; i < frame_len; i++)
    {       
   asm volatile("l.nop 0x1223");               
        asm volatile("l.mac %0, %1" : : "r"(filter[1]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][1]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][11]]))));               
        asm volatile("l.mac %0, %1" : : "r"(filter[3]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][3]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][9]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[5]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][5]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][7]])))); 
        asm volatile("l.macrc %0,%1" : "=r"(r135): "i"(FRAC_BITS));        

        asm volatile("l.mac %0, %1" : : "r"(filter[0]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][0]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][12]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[2]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][2]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][10]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[4]), "r"((QMF_RE(buffer[channel_filter2IdxTab0[i][4]]) + QMF_RE(buffer[channel_filter2IdxTab0[i][8]]))));        
        asm volatile("l.mac %0, %1" : : "r"(filter[6]), "r"(QMF_RE(buffer[channel_filter2IdxTab0[i][6]])));      
        asm volatile("l.macrc %0,%1" : "=r"(r0246): "i"(FRAC_BITS));                              

        asm volatile("l.mac %0, %1" : : "r"(filter[1]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][1]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][11]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[3]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][3]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][9]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[5]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][5]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][7]]))));        
        asm volatile("l.macrc %0,%1" : "=r"(i135): "i"(FRAC_BITS));                                      

        asm volatile("l.mac %0, %1" : : "r"(filter[0]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][0]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][12]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[2]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][2]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][10]]))));
        asm volatile("l.mac %0, %1" : : "r"(filter[4]), "r"((QMF_IM(buffer[channel_filter2IdxTab0[i][4]]) + QMF_IM(buffer[channel_filter2IdxTab0[i][8]]))));        
        asm volatile("l.mac %0, %1" : : "r"(filter[6]), "r"(QMF_IM(buffer[channel_filter2IdxTab0[i][6]])));                
        asm volatile("l.macrc %0,%1" : "=r"(i0246): "i"(FRAC_BITS));                                              
      
        // q = 0 
        QMF_RE(X_hybrid[i][0]) = r135+r0246;
        QMF_IM(X_hybrid[i][0]) = i135+i0246;

        // q = 1 
        QMF_RE(X_hybrid[i][1]) = r0246 - r135;
        QMF_IM(X_hybrid[i][1]) = i0246 - i135;
    }
*/
#endif
}

/* complex filter, size 4 */
 void channel_filter4(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                            qmf_t *buffer, qmf_t X_hybrid[32][12])
{
    uint8_t i;
    real_t input_re1[2], input_re2[2], input_im1[2], input_im2[2];
    for (i = 0; i < frame_len; i++)
    {
#if 0     
        input_re1[0] = -MUL_F(filter[2], (QMF_RE(buffer[i+2]) + QMF_RE(buffer[i+10]))) +
                                MUL_F(filter[6], QMF_RE(buffer[i+6]));
        input_re1[1] = MUL_F(nChannel_filter4_input_re1,
            (MUL_F(filter[1], (QMF_RE(buffer[i+1]) + QMF_RE(buffer[i+11]))) +
            MUL_F(filter[3], (QMF_RE(buffer[i+3]) + QMF_RE(buffer[i+9]))) -
            MUL_F(filter[5], (QMF_RE(buffer[i+5]) + QMF_RE(buffer[i+7])))));

        input_im1[0] = MUL_F(filter[0], (QMF_IM(buffer[i+0]) - QMF_IM(buffer[i+12]))) -
            MUL_F(filter[4], (QMF_IM(buffer[i+4]) - QMF_IM(buffer[i+8])));
        input_im1[1] = MUL_F(nChannel_filter4_input_im1,
            (MUL_F(filter[1], (QMF_IM(buffer[i+1]) - QMF_IM(buffer[i+11]))) -
            MUL_F(filter[3], (QMF_IM(buffer[i+3]) - QMF_IM(buffer[i+9]))) -
            MUL_F(filter[5], (QMF_IM(buffer[i+5]) - QMF_IM(buffer[i+7])))));

        input_re2[0] = MUL_F(filter[0], (QMF_RE(buffer[i+0]) - QMF_RE(buffer[i+12]))) -
            MUL_F(filter[4], (QMF_RE(buffer[i+4]) - QMF_RE(buffer[i+8])));
        input_re2[1] = MUL_F(nChannel_filter4_input_re2,
            (MUL_F(filter[1], (QMF_RE(buffer[i+1]) - QMF_RE(buffer[i+11]))) -
            MUL_F(filter[3], (QMF_RE(buffer[i+3]) - QMF_RE(buffer[i+9]))) -
            MUL_F(filter[5], (QMF_RE(buffer[i+5]) - QMF_RE(buffer[i+7])))));

        input_im2[0] = -MUL_F(filter[2], (QMF_IM(buffer[i+2]) + QMF_IM(buffer[i+10]))) +
            MUL_F(filter[6], QMF_IM(buffer[i+6]));
        input_im2[1] = MUL_F(nChannel_filter4_input_im2,
            (MUL_F(filter[1], (QMF_IM(buffer[i+1]) + QMF_IM(buffer[i+11]))) +
            MUL_F(filter[3], (QMF_IM(buffer[i+3]) + QMF_IM(buffer[i+9]))) -
            MUL_F(filter[5], (QMF_IM(buffer[i+5]) + QMF_IM(buffer[i+7])))));
#else
        input_re1[0] = MUL_F_SUB(filter[6],QMF_RE(buffer[i+6]),filter[2],QMF_RE(buffer[i+2]) + QMF_RE(buffer[i+10]));
        input_re1[1] = MUL_F(nChannel_filter4_input_re1,
            (MUL_F_ADD(filter[1], (QMF_RE(buffer[i+1]) + QMF_RE(buffer[i+11])),filter[3],(QMF_RE(buffer[i+3]) + QMF_RE(buffer[i+9]))) +
            MUL_SUB_F(filter[5], (QMF_RE(buffer[i+5]) + QMF_RE(buffer[i+7])))));

        input_im1[0] = MUL_F_SUB(filter[0],(QMF_IM(buffer[i+0]) - QMF_IM(buffer[i+12])),filter[4],(QMF_IM(buffer[i+4]) - QMF_IM(buffer[i+8])));
        input_im1[1] = MUL_F(nChannel_filter4_input_im1,
            (MUL_F_SUB(filter[1], (QMF_IM(buffer[i+1]) - QMF_IM(buffer[i+11])),filter[3], (QMF_IM(buffer[i+3]) - QMF_IM(buffer[i+9]))) +
            MUL_SUB_F(filter[5], (QMF_IM(buffer[i+5]) - QMF_IM(buffer[i+7])))));
        input_re2[0] = MUL_F_SUB(filter[0],(QMF_RE(buffer[i+0]) - QMF_RE(buffer[i+12])),filter[4],(QMF_RE(buffer[i+4]) - QMF_RE(buffer[i+8])));       
        
        input_re2[1] = MUL_F(nChannel_filter4_input_re2,
            (MUL_F_SUB(filter[1], (QMF_RE(buffer[i+1]) - QMF_RE(buffer[i+11])),filter[3], (QMF_RE(buffer[i+3]) - QMF_RE(buffer[i+9]))) +
            MUL_SUB_F(filter[5], (QMF_RE(buffer[i+5]) - QMF_RE(buffer[i+7])))));

        input_im2[0] = MUL_F_SUB(filter[6],QMF_IM(buffer[i+6]),filter[2],(QMF_IM(buffer[i+2]) + QMF_IM(buffer[i+10])));
        input_im2[1] = MUL_F(nChannel_filter4_input_im2,
            (MUL_F_ADD(filter[1], (QMF_IM(buffer[i+1]) + QMF_IM(buffer[i+11])),filter[3], (QMF_IM(buffer[i+3]) + QMF_IM(buffer[i+9]))) +
            MUL_SUB_F(filter[5], (QMF_IM(buffer[i+5]) + QMF_IM(buffer[i+7])))));
#endif
        /* q == 0 */
        QMF_RE(X_hybrid[i][0]) =  input_re1[0] + input_re1[1] + input_im1[0] + input_im1[1];
        QMF_IM(X_hybrid[i][0]) = -input_re2[0] - input_re2[1] + input_im2[0] + input_im2[1];

        /* q == 1 */
        QMF_RE(X_hybrid[i][1]) =  input_re1[0] - input_re1[1] - input_im1[0] + input_im1[1];
        QMF_IM(X_hybrid[i][1]) =  input_re2[0] - input_re2[1] + input_im2[0] - input_im2[1];

        /* q == 2 */
        QMF_RE(X_hybrid[i][2]) =  input_re1[0] - input_re1[1] + input_im1[0] - input_im1[1];
        QMF_IM(X_hybrid[i][2]) = -input_re2[0] + input_re2[1] + input_im2[0] - input_im2[1];

        /* q == 3 */
        QMF_RE(X_hybrid[i][3]) =  input_re1[0] + input_re1[1] - input_im1[0] - input_im1[1];
        QMF_IM(X_hybrid[i][3]) =  input_re2[0] + input_re2[1] + input_im2[0] + input_im2[1];
    }
}

 void INLINE DCT3_4_unscaled(real_t *y, real_t *x)
{
    real_t f0, f1, f2, f3, f4, f5, f6, f7, f8;

    f0 = MUL_F(x[2], nDCT3_4_unscaled_f0);
    f1 = x[0] - f0;
    f2 = x[0] + f0;
    f3 = x[1] + x[3];
    f4 = MUL_C(x[1], nDCT3_4_unscaled_f4);
    f5 = MUL_F(f3, nDCT3_4_unscaled_f5);
    f6 = MUL_F(x[3], nDCT3_4_unscaled_f6);
    f7 = f4 + f5;
    f8 = f6 - f5;
    y[3] = f2 - f8;
    y[0] = f2 + f8;
    y[2] = f1 - f7;
    y[1] = f1 + f7;
}

/* complex filter, size 8 */
 void channel_filter8(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                            qmf_t *buffer, qmf_t X_hybrid[32][12])
{
    uint8_t i, n;
    real_t input_re1[4], input_re2[4], input_im1[4], input_im2[4];
    real_t x[4];

    for (i = 0; i < frame_len; i++)
    {
        input_re1[0] =  MUL_F(filter[6],QMF_RE(buffer[6+i]));
        input_re1[1] =  MUL_F(filter[5],(QMF_RE(buffer[5+i]) + QMF_RE(buffer[7+i])));
#if 0        
        input_re1[2] = -MUL_F(filter[0],(QMF_RE(buffer[0+i]) + QMF_RE(buffer[12+i]))) + MUL_F(filter[4],(QMF_RE(buffer[4+i]) + QMF_RE(buffer[8+i])));
        input_re1[3] = -MUL_F(filter[1],(QMF_RE(buffer[1+i]) + QMF_RE(buffer[11+i]))) + MUL_F(filter[3],(QMF_RE(buffer[3+i]) + QMF_RE(buffer[9+i])));
#else
        input_re1[2] = MUL_F_SUB(filter[4],(QMF_RE(buffer[4+i]) + QMF_RE(buffer[8+i])),filter[0],(QMF_RE(buffer[0+i]) + QMF_RE(buffer[12+i])));        
        input_re1[3] = MUL_F_SUB(filter[3],(QMF_RE(buffer[3+i]) + QMF_RE(buffer[9+i])),filter[1],(QMF_RE(buffer[1+i]) + QMF_RE(buffer[11+i])));        
#endif
        input_im1[0] = MUL_F(filter[5],(QMF_IM(buffer[7+i]) - QMF_IM(buffer[5+i])));
#if 0
        input_im1[1] = MUL_F(filter[0],(QMF_IM(buffer[12+i]) - QMF_IM(buffer[0+i]))) + MUL_F(filter[4],(QMF_IM(buffer[8+i]) - QMF_IM(buffer[4+i])));
        input_im1[2] = MUL_F(filter[1],(QMF_IM(buffer[11+i]) - QMF_IM(buffer[1+i]))) + MUL_F(filter[3],(QMF_IM(buffer[9+i]) - QMF_IM(buffer[3+i])));
#else
        input_im1[1] = MUL_F_ADD(filter[0],(QMF_IM(buffer[12+i]) - QMF_IM(buffer[0+i])),filter[4],(QMF_IM(buffer[8+i]) - QMF_IM(buffer[4+i])));        
        input_im1[2] = MUL_F_ADD(filter[1],(QMF_IM(buffer[11+i]) - QMF_IM(buffer[1+i])),filter[3],(QMF_IM(buffer[9+i]) - QMF_IM(buffer[3+i])));                
#endif
        input_im1[3] = MUL_F(filter[2],(QMF_IM(buffer[10+i]) - QMF_IM(buffer[2+i])));

#if 0        
        for (n = 0; n < 4; n++)
        {
            x[n] = input_re1[n] - input_im1[3-n];
        }
#else
        x[0] = input_re1[0] - input_im1[3];
        x[1] = input_re1[1] - input_im1[2];
        x[2] = input_re1[2] - input_im1[1];            
        x[3] = input_re1[3] - input_im1[0];            
#endif
        DCT3_4_unscaled(x, x);
        QMF_RE(X_hybrid[i][7]) = x[0];
        QMF_RE(X_hybrid[i][5]) = x[2];
        QMF_RE(X_hybrid[i][3]) = x[3];
        QMF_RE(X_hybrid[i][1]) = x[1];
#if 0        
        for (n = 0; n < 4; n++)
        {
            x[n] = input_re1[n] + input_im1[3-n];
        }
#else
            x[0] = input_re1[0] - input_im1[3];
            x[1] = input_re1[1] - input_im1[2];
            x[2] = input_re1[2] - input_im1[1];            
            x[3] = input_re1[3] - input_im1[0];            
#endif
        DCT3_4_unscaled(x, x);
        QMF_RE(X_hybrid[i][6]) = x[1];
        QMF_RE(X_hybrid[i][4]) = x[3];
        QMF_RE(X_hybrid[i][2]) = x[2];
        QMF_RE(X_hybrid[i][0]) = x[0];

        input_im2[0] =  MUL_F(filter[6],QMF_IM(buffer[6+i]));
        input_im2[1] =  MUL_F(filter[5],(QMF_IM(buffer[5+i]) + QMF_IM(buffer[7+i])));
#if 0        
        input_im2[2] = -MUL_F(filter[0],(QMF_IM(buffer[0+i]) + QMF_IM(buffer[12+i]))) + MUL_F(filter[4],(QMF_IM(buffer[4+i]) + QMF_IM(buffer[8+i])));
        input_im2[3] = -MUL_F(filter[1],(QMF_IM(buffer[1+i]) + QMF_IM(buffer[11+i]))) + MUL_F(filter[3],(QMF_IM(buffer[3+i]) + QMF_IM(buffer[9+i])));
#else
        input_im2[2] = MUL_F_SUB(filter[4],(QMF_IM(buffer[4+i]) + QMF_IM(buffer[8+i])),filter[0],(QMF_IM(buffer[0+i]) + QMF_IM(buffer[12+i])));        
        input_im2[3] = MUL_F_SUB(filter[3],(QMF_IM(buffer[3+i]) + QMF_IM(buffer[9+i])),filter[1],(QMF_IM(buffer[1+i]) + QMF_IM(buffer[11+i])));                
#endif
        input_re2[0] = MUL_F(filter[5],(QMF_RE(buffer[7+i]) - QMF_RE(buffer[5+i])));
#if 0
        input_re2[1] = MUL_F(filter[0],(QMF_RE(buffer[12+i]) - QMF_RE(buffer[0+i]))) + MUL_F(filter[4],(QMF_RE(buffer[8+i]) - QMF_RE(buffer[4+i])));
        input_re2[2] = MUL_F(filter[1],(QMF_RE(buffer[11+i]) - QMF_RE(buffer[1+i]))) + MUL_F(filter[3],(QMF_RE(buffer[9+i]) - QMF_RE(buffer[3+i])));
#else
        input_re2[1] = MUL_F_ADD(filter[0],QMF_RE(buffer[12+i]) - QMF_RE(buffer[0+i]),filter[4],QMF_RE(buffer[8+i]) - QMF_RE(buffer[4+i]));
        input_re2[2] = MUL_F_ADD(filter[1],QMF_RE(buffer[11+i]) - QMF_RE(buffer[1+i]),filter[3],QMF_RE(buffer[9+i]) - QMF_RE(buffer[3+i]));
#endif
        input_re2[3] = MUL_F(filter[2],(QMF_RE(buffer[10+i]) - QMF_RE(buffer[2+i])));

#if 0        
        for (n = 0; n < 4; n++)
        {
            x[n] = input_im2[n] + input_re2[3-n];
        }
#else
       x[0] = input_im2[0] + input_re2[3]; 
       x[1] = input_im2[1] + input_re2[2]; 
       x[2] = input_im2[2] + input_re2[1]; 
       x[3] = input_im2[3] + input_re2[0];        
#endif

        DCT3_4_unscaled(x, x);
        QMF_IM(X_hybrid[i][7]) = x[0];
        QMF_IM(X_hybrid[i][5]) = x[2];
        QMF_IM(X_hybrid[i][3]) = x[3];
        QMF_IM(X_hybrid[i][1]) = x[1];

#if 0        
        for (n = 0; n < 4; n++)
        {
            x[n] = input_im2[n] - input_re2[3-n];
        }
#else
       x[0] = input_im2[0] - input_re2[3]; 
       x[1] = input_im2[1] - input_re2[2]; 
       x[2] = input_im2[2] - input_re2[1]; 
       x[3] = input_im2[3] - input_re2[0];        
#endif
        DCT3_4_unscaled(x, x);
        QMF_IM(X_hybrid[i][6]) = x[1];
        QMF_IM(X_hybrid[i][4]) = x[3];
        QMF_IM(X_hybrid[i][2]) = x[2];
        QMF_IM(X_hybrid[i][0]) = x[0];
    }
}

 void INLINE DCT3_6_unscaled(real_t *y, real_t *x)
{
    real_t f0, f1, f2, f3, f4, f5, f6, f7;

    f0 = MUL_F(x[3], nDCT3_6_unscaled_f0);
    f1 = x[0] + f0;
    f2 = x[0] - f0;
    f3 = MUL_F((x[1] - x[5]), nDCT3_6_unscaled_f3);
    //f4 = MUL_F(x[2], nDCT3_6_unscaled_f4) + MUL_F(x[4], nDCT3_6_unscaled_f4_1);
    f4 = MUL_F_ADD(x[2],nDCT3_6_unscaled_f4,x[4],nDCT3_6_unscaled_f4_1);
    f5 = f4 - x[4];
    //f6 = MUL_F(x[1], nDCT3_6_unscaled_f6_1) + MUL_F(x[5], nDCT3_6_unscaled_f6_5);
    f6 = MUL_F_ADD(x[1],nDCT3_6_unscaled_f6_1,x[5],nDCT3_6_unscaled_f6_5);
    f7 = f6 - f3;
    y[0] = f1 + f6 + f4;
    y[1] = f2 + f3 - x[4];
    y[2] = f7 + f2 - f5;
    y[3] = f1 - f7 - f5;
    y[4] = f1 - f3 - x[4];
    y[5] = f2 - f6 + f4;
}

/* complex filter, size 12 */
 void channel_filter12(hyb_info *hyb, uint8_t frame_len, const real_t *filter,
                             qmf_t *buffer, qmf_t X_hybrid[32][12])
{
    uint8_t i, n;
    real_t input_re1[6], input_re2[6], input_im1[6], input_im2[6];
    real_t out_re1[6], out_re2[6], out_im1[6], out_im2[6];

    for (i = 0; i < frame_len; i++)
    {
#if 0    
        for (n = 0; n < 6; n++)
        {
            if (n == 0)
            {
                input_re1[0] = MUL_F(QMF_RE(buffer[6+i]), filter[6]);
                input_re2[0] = MUL_F(QMF_IM(buffer[6+i]), filter[6]);
            } else {
                input_re1[6-n] = MUL_F((QMF_RE(buffer[n+i]) + QMF_RE(buffer[12-n+i])), filter[n]);
                input_re2[6-n] = MUL_F((QMF_IM(buffer[n+i]) + QMF_IM(buffer[12-n+i])), filter[n]);
            }
            input_im2[n] = MUL_F((QMF_RE(buffer[n+i]) - QMF_RE(buffer[12-n+i])), filter[n]);
            input_im1[n] = MUL_F((QMF_IM(buffer[n+i]) - QMF_IM(buffer[12-n+i])), filter[n]);
        }
#else
        input_re1[0] = MUL_F(QMF_RE(buffer[6+i]), filter[6]);
        input_re2[0] = MUL_F(QMF_IM(buffer[6+i]), filter[6]);
        input_im2[0] = MUL_F((QMF_RE(buffer[i]) - QMF_RE(buffer[12+i])), filter[0]);
        input_im1[0] = MUL_F((QMF_IM(buffer[i]) - QMF_IM(buffer[12+i])), filter[0]);
        input_re1[5] = MUL_F((QMF_RE(buffer[1+i]) + QMF_RE(buffer[11+i])), filter[1]);
        input_re2[5] = MUL_F((QMF_IM(buffer[1+i]) + QMF_IM(buffer[11+i])), filter[1]);
        input_im2[1] = MUL_F((QMF_RE(buffer[1+i]) - QMF_RE(buffer[11+i])), filter[1]);
        input_im1[1] = MUL_F((QMF_IM(buffer[1+i]) - QMF_IM(buffer[11+i])), filter[1]);
        input_re1[4] = MUL_F((QMF_RE(buffer[2+i]) + QMF_RE(buffer[10+i])), filter[2]);
        input_re2[4] = MUL_F((QMF_IM(buffer[2+i]) + QMF_IM(buffer[10+i])), filter[2]);
        input_im2[2] = MUL_F((QMF_RE(buffer[2+i]) - QMF_RE(buffer[10+i])), filter[2]);
        input_im1[2] = MUL_F((QMF_IM(buffer[2+i]) - QMF_IM(buffer[10+i])), filter[2]);
        input_re1[3] = MUL_F((QMF_RE(buffer[3+i]) + QMF_RE(buffer[9+i])), filter[3]);
        input_re2[3] = MUL_F((QMF_IM(buffer[3+i]) + QMF_IM(buffer[9+i])), filter[3]);
        input_im2[3] = MUL_F((QMF_RE(buffer[3+i]) - QMF_RE(buffer[9+i])), filter[3]);
        input_im1[3] = MUL_F((QMF_IM(buffer[3+i]) - QMF_IM(buffer[9+i])), filter[3]);
        input_re1[2] = MUL_F((QMF_RE(buffer[4+i]) + QMF_RE(buffer[8+i])), filter[4]);
        input_re2[2] = MUL_F((QMF_IM(buffer[4+i]) + QMF_IM(buffer[8+i])), filter[4]);
        input_im2[4] = MUL_F((QMF_RE(buffer[4+i]) - QMF_RE(buffer[8+i])), filter[4]);
        input_im1[4] = MUL_F((QMF_IM(buffer[4+i]) - QMF_IM(buffer[8+i])), filter[4]);
        input_re1[1] = MUL_F((QMF_RE(buffer[5+i]) + QMF_RE(buffer[7+i])), filter[5]);
        input_re2[1] = MUL_F((QMF_IM(buffer[5+i]) + QMF_IM(buffer[7+i])), filter[5]);
        input_im2[5] = MUL_F((QMF_RE(buffer[5+i]) - QMF_RE(buffer[7+i])), filter[5]);
        input_im1[5] = MUL_F((QMF_IM(buffer[5+i]) - QMF_IM(buffer[7+i])), filter[5]);             

#endif

        DCT3_6_unscaled(out_re1, input_re1);
        DCT3_6_unscaled(out_re2, input_re2);

        DCT3_6_unscaled(out_im1, input_im1);
        DCT3_6_unscaled(out_im2, input_im2);

#if 0
        for (n = 0; n < 6; n += 2)
        {
            QMF_RE(X_hybrid[i][n]) = out_re1[n] - out_im1[n];
            QMF_IM(X_hybrid[i][n]) = out_re2[n] + out_im2[n];
            QMF_RE(X_hybrid[i][n+1]) = out_re1[n+1] + out_im1[n+1];
            QMF_IM(X_hybrid[i][n+1]) = out_re2[n+1] - out_im2[n+1];

            QMF_RE(X_hybrid[i][10-n]) = out_re1[n+1] - out_im1[n+1];
            QMF_IM(X_hybrid[i][10-n]) = out_re2[n+1] + out_im2[n+1];
            QMF_RE(X_hybrid[i][11-n]) = out_re1[n] + out_im1[n];
            QMF_IM(X_hybrid[i][11-n]) = out_re2[n] - out_im2[n];
        }
#else
            QMF_RE(X_hybrid[i][0]) = out_re1[0] - out_im1[0];
            QMF_IM(X_hybrid[i][0]) = out_re2[0] + out_im2[0];
            QMF_RE(X_hybrid[i][1]) = out_re1[1] + out_im1[1];
            QMF_IM(X_hybrid[i][1]) = out_re2[1] - out_im2[1];

            QMF_RE(X_hybrid[i][10]) = out_re1[1] - out_im1[1];
            QMF_IM(X_hybrid[i][10]) = out_re2[1] + out_im2[1];
            QMF_RE(X_hybrid[i][11]) = out_re1[0] + out_im1[0];
            QMF_IM(X_hybrid[i][11]) = out_re2[0] - out_im2[0];

            QMF_RE(X_hybrid[i][2]) = out_re1[2] - out_im1[2];
            QMF_IM(X_hybrid[i][2]) = out_re2[2] + out_im2[2];
            QMF_RE(X_hybrid[i][3]) = out_re1[3] + out_im1[3];
            QMF_IM(X_hybrid[i][3]) = out_re2[3] - out_im2[3];

            QMF_RE(X_hybrid[i][8]) = out_re1[3] - out_im1[3];
            QMF_IM(X_hybrid[i][8]) = out_re2[3] + out_im2[3];
            QMF_RE(X_hybrid[i][9]) = out_re1[2] + out_im1[2];
            QMF_IM(X_hybrid[i][9]) = out_re2[2] - out_im2[2];

            QMF_RE(X_hybrid[i][4]) = out_re1[4] - out_im1[4];
            QMF_IM(X_hybrid[i][4]) = out_re2[4] + out_im2[4];
            QMF_RE(X_hybrid[i][5]) = out_re1[5] + out_im1[5];
            QMF_IM(X_hybrid[i][5]) = out_re2[5] - out_im2[5];

            QMF_RE(X_hybrid[i][6]) = out_re1[5] - out_im1[5];
            QMF_IM(X_hybrid[i][6]) = out_re2[5] + out_im2[5];
            QMF_RE(X_hybrid[i][7]) = out_re1[4] + out_im1[4];
            QMF_IM(X_hybrid[i][7]) = out_re2[4] - out_im2[4];
            

#endif
    }
}

/* Hybrid analysis: further split up QMF subbands
 * to improve frequency resolution
 */
void hybrid_analysis(hyb_info *hyb, qmf_t X[32][64], qmf_t X_hybrid[32][32],
                            uint8_t use34, uint8_t numTimeSlotsRate)
{
    uint8_t k, n, band;
    uint8_t offset = 0;
    uint8_t qmf_bands = (use34) ? 5 : 3;
    uint8_t *resolution = (use34) ? hyb->resolution34 : hyb->resolution20;

    for (band = 0; band < qmf_bands; band++)
    {
        /* build working buffer */
        memcpy(hyb->work, hyb->buffer[band], 12 * sizeof(qmf_t));

        /* add new samples */
        for (n = 0; n < hyb->frame_len; n++)
        {
            QMF_RE(hyb->work[12 + n]) = QMF_RE(X[n + 6 /*delay*/][band]);
            QMF_IM(hyb->work[12 + n]) = QMF_IM(X[n + 6 /*delay*/][band]);
        }

        /* store samples */
        memcpy(hyb->buffer[band], hyb->work + hyb->frame_len, 12 * sizeof(qmf_t));

        switch(resolution[band])
        {
        case 2:
            /* Type B real filter, Q[p] = 2 */
            channel_filter2(hyb, hyb->frame_len, p2_13_20, hyb->work, hyb->temp);
            break;
        case 4:
            /* Type A complex filter, Q[p] = 4 */
            channel_filter4(hyb, hyb->frame_len, p4_13_34, hyb->work, hyb->temp);
            break;
        case 8:
            /* Type A complex filter, Q[p] = 8 */
            channel_filter8(hyb, hyb->frame_len, (use34) ? p8_13_34 : p8_13_20,
                hyb->work, hyb->temp);
            break;
        case 12:
            /* Type A complex filter, Q[p] = 12 */
            channel_filter12(hyb, hyb->frame_len, p12_13_34, hyb->work, hyb->temp);
            break;
        }

        for (n = 0; n < hyb->frame_len; n++)
        {
            for (k = 0; k < resolution[band]; k++)
            {
                QMF_RE(X_hybrid[n][offset + k]) = QMF_RE(hyb->temp[n][k]);
                QMF_IM(X_hybrid[n][offset + k]) = QMF_IM(hyb->temp[n][k]);
            }
        }
        offset += resolution[band];
    }

    /* group hybrid channels */
    if (!use34)
    {
        for (n = 0; n < numTimeSlotsRate; n++)
        {
            QMF_RE(X_hybrid[n][3]) += QMF_RE(X_hybrid[n][4]);
            QMF_IM(X_hybrid[n][3]) += QMF_IM(X_hybrid[n][4]);
            QMF_RE(X_hybrid[n][4]) = 0;
            QMF_IM(X_hybrid[n][4]) = 0;

            QMF_RE(X_hybrid[n][2]) += QMF_RE(X_hybrid[n][5]);
            QMF_IM(X_hybrid[n][2]) += QMF_IM(X_hybrid[n][5]);
            QMF_RE(X_hybrid[n][5]) = 0;
            QMF_IM(X_hybrid[n][5]) = 0;
        }
    }
}

void hybrid_synthesis(hyb_info *hyb, qmf_t X[32][64], qmf_t X_hybrid[32][32],
                             uint8_t use34, uint8_t numTimeSlotsRate)
{
    uint8_t k, n, band;
    uint8_t offset = 0;
    uint8_t qmf_bands = (use34) ? 5 : 3;
    uint8_t *resolution = (use34) ? hyb->resolution34 : hyb->resolution20;

    for(band = 0; band < qmf_bands; band++)
    {
        for (n = 0; n < hyb->frame_len; n++)
        {
            QMF_RE(X[n][band]) = 0;
            QMF_IM(X[n][band]) = 0;
#if 0
            for (k = 0; k < resolution[band]; k++)
            {
                QMF_RE(X[n][band]) += QMF_RE(X_hybrid[n][offset + k]);
                QMF_IM(X[n][band]) += QMF_IM(X_hybrid[n][offset + k]);
            }
#else
        for (k = 0; k < resolution[band]; k++)
        {
            asm volatile("l.mac %0, %1" : : "r"(QMF_RE(X_hybrid[n][offset + k])), "r"(1));           
        }
        asm volatile("l.macrc %0, 0" : "=r"(QMF_RE(X[n][band])) );            
        for (k = 0; k < resolution[band]; k++)
        {
            asm volatile("l.mac %0, %1" : : "r"(QMF_IM(X_hybrid[n][offset + k])), "r"(1));                       
        }
        asm volatile("l.macrc %0, 0" : "=r"(QMF_IM(X[n][band])) );            
#endif
        }
        offset += resolution[band];
    }
}

/* limits the value i to the range [min,max] */
 int8_t delta_clip(int8_t i, int8_t min, int8_t max)
{
    if (i < min)
        return min;
    else if (i > max)
        return max;
    else
        return i;
}

//int iid = 0;

/* delta decode array */
 void delta_decode(uint8_t enable, int8_t *index, int8_t *index_prev,
                         uint8_t dt_flag, uint8_t nr_par, uint8_t stride,
                         int8_t min_index, int8_t max_index)
{
    int8_t i;

    if (enable == 1)
    {
        if (dt_flag == 0)
        {
            /* delta coded in frequency direction */
            //index[0] = 0 + index[0];
            index[0] = delta_clip(index[0], min_index, max_index);

            for (i = 1; i < nr_par; i++)
            {
                index[i] = index[i-1] + index[i];
                index[i] = delta_clip(index[i], min_index, max_index);
            }
        } else {
            /* delta coded in time direction */
            for (i = 0; i < nr_par; i++)
            {
                //int8_t tmp2;
                //int8_t tmp = index[i];

                //printf("%d %d\n", index_prev[i*stride], index[i]);
                //printf("%d\n", index[i]);

                index[i] = index_prev[i*stride] + index[i];
                //tmp2 = index[i];
                index[i] = delta_clip(index[i], min_index, max_index);

                //if (iid)
                //{
                //    if (index[i] == 7)
                //    {
                //        printf("%d %d %d\n", index_prev[i*stride], tmp, tmp2);
                //    }
                //}
            }
        }
    }
    else
    {  
        /* set indices to zero */
        for (i = 0; i < nr_par; i++)
        {
            index[i] = 0;
        }
    }

    /* coarse */
    if (stride == 2)
    {
        for (i = (nr_par<<1)-1; i > 0; i--)
        {
            index[i] = index[i>>1];
        }
    }
}

/* delta modulo decode array */
/* in: log2 value of the modulo value to allow using AND instead of MOD */
 void delta_modulo_decode(uint8_t enable, int8_t *index, int8_t *index_prev,
                                uint8_t dt_flag, uint8_t nr_par, uint8_t stride,
                                int8_t and_modulo)
{
    int8_t i;

    if (enable == 1)
    {
        if (dt_flag == 0)
        {
            /* delta coded in frequency direction */
            index[0] = 0 + index[0];
            index[0] &= and_modulo;
            for (i = 1; i < nr_par; i++)
            {
                index[i] = index[i-1] + index[i];
                index[i] &= and_modulo;
            }
        } else {
            /* delta coded in time direction */
            for (i = 0; i < nr_par; i++)
            {
                index[i] = index_prev[i*stride] + index[i];
                index[i] &= and_modulo;
            }
        }
    }
    else {
        /* set indices to zero */
        for (i = 0; i < nr_par; i++)
        {
            index[i] = 0;
        }
    }

    /* coarse */
    if (stride == 2)
    {
        index[0] = 0;
        for (i = (nr_par<<1)-1; i > 0; i--)
        {
            index[i] = index[i>>1];
        }
    }
}

#ifdef PS_LOW_POWER
void map34indexto20(int8_t *index, uint8_t bins)
{
    index[0] = (2*index[0]+index[1])/3;
    index[1] = (index[1]+2*index[2])/3;
    index[2] = (2*index[3]+index[4])/3;
    index[3] = (index[4]+2*index[5])/3;
    index[4] = (index[6]+index[7])/2;
    index[5] = (index[8]+index[9])/2;
    index[6] = index[10];
    index[7] = index[11];
    index[8] = (index[12]+index[13])/2;
    index[9] = (index[14]+index[15])/2;
    index[10] = index[16];

    if (bins == 34)
    {
        index[11] = index[17];
        index[12] = index[18];
        index[13] = index[19];
        index[14] = (index[20]+index[21])/2;
        index[15] = (index[22]+index[23])/2;
        index[16] = (index[24]+index[25])/2;
        index[17] = (index[26]+index[27])/2;
        index[18] = (index[28]+index[29]+index[30]+index[31])/4;
        index[19] = (index[32]+index[33])/2;
    }
}
#endif

void map20indexto34(int8_t *index, uint8_t bins)
{
    index[0] = index[0];
    index[1] = (index[0] + index[1])>>1;
    index[2] = index[1];
    index[3] = index[2];
    index[4] = (index[2] + index[3])>>1;
    index[5] = index[3];
    index[6] = index[4];
    index[7] = index[4];
    index[8] = index[5];
    index[9] = index[5];
    index[10] = index[6];
    index[11] = index[7];
    index[12] = index[8];
    index[13] = index[8];
    index[14] = index[9];
    index[15] = index[9];
    index[16] = index[10];

    if (bins == 34)
    {
        index[17] = index[11];
        index[18] = index[12];
        index[19] = index[13];
        index[20] = index[14];
        index[21] = index[14];
        index[22] = index[15];
        index[23] = index[15];
        index[24] = index[16];
        index[25] = index[16];
        index[26] = index[17];
        index[27] = index[17];
        index[28] = index[18];
        index[29] = index[18];
        index[30] = index[18];
        index[31] = index[18];
        index[32] = index[19];
        index[33] = index[19];
    }
}

/* parse the bitstream data decoded in ps_data() */
 void ps_data_decode(ps_info *ps)
{
    uint8_t env, bin;

    /* ps data not available, use data from previous frame */
    if (ps->ps_data_available == 0)
    {
        ps->num_env = 0;
    }
    for (env = 0; env < ps->num_env; env++)
    {
        int8_t *iid_index_prev;
        int8_t *icc_index_prev;
        int8_t *ipd_index_prev;
        int8_t *opd_index_prev;

        int8_t num_iid_steps = (ps->iid_mode < 3) ? 7 : 15 /*fine quant*/;
        if (env == 0)
        {
            /* take last envelope from previous frame */
            iid_index_prev = ps->iid_index_prev;
            icc_index_prev = ps->icc_index_prev;
            ipd_index_prev = ps->ipd_index_prev;
            opd_index_prev = ps->opd_index_prev;
        } else {
            /* take index values from previous envelope */
            iid_index_prev = ps->iid_index[env - 1];
            icc_index_prev = ps->icc_index[env - 1];
            ipd_index_prev = ps->ipd_index[env - 1];
            opd_index_prev = ps->opd_index[env - 1];
        }

//        iid = 1;
        /* delta decode iid parameters */
        delta_decode(ps->enable_iid, ps->iid_index[env], iid_index_prev,
            ps->iid_dt[env], ps->nr_iid_par,
            (ps->iid_mode == 0 || ps->iid_mode == 3) ? 2 : 1,
            -num_iid_steps, num_iid_steps);
//        iid = 0;

        /* delta decode icc parameters */
        delta_decode(ps->enable_icc, ps->icc_index[env], icc_index_prev,
            ps->icc_dt[env], ps->nr_icc_par,
            (ps->icc_mode == 0 || ps->icc_mode == 3) ? 2 : 1,
            0, 7);

        /* delta modulo decode ipd parameters */
        delta_modulo_decode(ps->enable_ipdopd, ps->ipd_index[env], ipd_index_prev,
            ps->ipd_dt[env], ps->nr_ipdopd_par, 1, 7);

        /* delta modulo decode opd parameters */
        delta_modulo_decode(ps->enable_ipdopd, ps->opd_index[env], opd_index_prev,
            ps->opd_dt[env], ps->nr_ipdopd_par, 1, 7);
    }

    /* handle error case */
    if (ps->num_env == 0)
    {
        /* force to 1 */
        ps->num_env = 1;

        if (ps->enable_iid)
        {
            for (bin = 0; bin < 34; bin++)
                ps->iid_index[0][bin] = ps->iid_index_prev[bin];
        }
        else
        {
            for (bin = 0; bin < 34; bin++)
                ps->iid_index[0][bin] = 0;
        }

        if (ps->enable_icc)
        {
            for (bin = 0; bin < 34; bin++)
                ps->icc_index[0][bin] = ps->icc_index_prev[bin];
        }
        else
        {
            for (bin = 0; bin < 34; bin++)
                ps->icc_index[0][bin] = 0;
        }

        if (ps->enable_ipdopd)
        {
            for (bin = 0; bin < 17; bin++)
            {
                ps->ipd_index[0][bin] = ps->ipd_index_prev[bin];
                ps->opd_index[0][bin] = ps->opd_index_prev[bin];
            }
        }
        else
        {
            for (bin = 0; bin < 17; bin++)
            {
                ps->ipd_index[0][bin] = 0;
                ps->opd_index[0][bin] = 0;
            }
        }
    }

    /* update previous indices */
    for (bin = 0; bin < 34; bin++)
        ps->iid_index_prev[bin] = ps->iid_index[ps->num_env-1][bin];
    for (bin = 0; bin < 34; bin++)
        ps->icc_index_prev[bin] = ps->icc_index[ps->num_env-1][bin];
    for (bin = 0; bin < 17; bin++)
    {
        ps->ipd_index_prev[bin] = ps->ipd_index[ps->num_env-1][bin];
        ps->opd_index_prev[bin] = ps->opd_index[ps->num_env-1][bin];
    }

    ps->ps_data_available = 0;

    if (ps->frame_class == 0)
    {
        ps->border_position[0] = 0;
        for (env = 1; env < ps->num_env; env++)
        {
            ps->border_position[env] = (env * ps->numTimeSlotsRate) / ps->num_env;
        }
        ps->border_position[ps->num_env] = ps->numTimeSlotsRate;
    }
    else
    {
        ps->border_position[0] = 0;

        if (ps->border_position[ps->num_env] < ps->numTimeSlotsRate)
        {
            for (bin = 0; bin < 34; bin++)
            {
                ps->iid_index[ps->num_env][bin] = ps->iid_index[ps->num_env-1][bin];
                ps->icc_index[ps->num_env][bin] = ps->icc_index[ps->num_env-1][bin];
            }
            for (bin = 0; bin < 17; bin++)
            {
                ps->ipd_index[ps->num_env][bin] = ps->ipd_index[ps->num_env-1][bin];
                ps->opd_index[ps->num_env][bin] = ps->opd_index[ps->num_env-1][bin];
            }
            ps->num_env++;
            ps->border_position[ps->num_env] = ps->numTimeSlotsRate;
        }

        for (env = 1; env < ps->num_env; env++)
        {
            int8_t thr = ps->numTimeSlotsRate - (ps->num_env - env);

            if (ps->border_position[env] > thr)
            {
                ps->border_position[env] = thr;
            } 
            else
            {
                thr = ps->border_position[env-1]+1;
                if (ps->border_position[env] < thr)
                {
                    ps->border_position[env] = thr;
                }
            }
        }
    }

    /* make sure that the indices of all parameters can be mapped
     * to the same hybrid synthesis filterbank
     */
#ifdef PS_LOW_POWER
    for (env = 0; env < ps->num_env; env++)
    {
        if (ps->iid_mode == 2 || ps->iid_mode == 5)
            map34indexto20(ps->iid_index[env], 34);
        if (ps->icc_mode == 2 || ps->icc_mode == 5)
            map34indexto20(ps->icc_index[env], 34);

        /* disable ipd/opd */
        for (bin = 0; bin < 17; bin++)
        {
            ps->aaIpdIndex[env][bin] = 0;
            ps->aaOpdIndex[env][bin] = 0;
        }
    }
#else
    if (ps->use34hybrid_bands)
    {
        for (env = 0; env < ps->num_env; env++)
        {
            if (ps->iid_mode != 2 && ps->iid_mode != 5)
                map20indexto34(ps->iid_index[env], 34);
            if (ps->icc_mode != 2 && ps->icc_mode != 5)
                map20indexto34(ps->icc_index[env], 34);
            if (ps->ipd_mode != 2 && ps->ipd_mode != 5)
            {
                map20indexto34(ps->ipd_index[env], 17);
                map20indexto34(ps->opd_index[env], 17);
            }
        }
    }
#endif

#if 0
    for (env = 0; env < ps->num_env; env++)
    {
        printf("iid[env:%d]:", env);
        for (bin = 0; bin < 34; bin++)
        {
            printf(" %d", ps->iid_index[env][bin]);
        }
        printf("\n");
    }
    for (env = 0; env < ps->num_env; env++)
    {
        printf("icc[env:%d]:", env);
        for (bin = 0; bin < 34; bin++)
        {
            printf(" %d", ps->icc_index[env][bin]);
        }
        printf("\n");
    }
    for (env = 0; env < ps->num_env; env++)
    {
        printf("ipd[env:%d]:", env);
        for (bin = 0; bin < 17; bin++)
        {
            printf(" %d", ps->ipd_index[env][bin]);
        }
        printf("\n");
    }
    for (env = 0; env < ps->num_env; env++)
    {
        printf("opd[env:%d]:", env);
        for (bin = 0; bin < 17; bin++)
        {
            printf(" %d", ps->opd_index[env][bin]);
        }
        printf("\n");
    }
    printf("\n");
#endif
}



/* decorrelate the mono signal using an allpass filter */
void ps_decorrelate(ps_info *ps, qmf_t X_left[38][64], qmf_t X_right[38][64],
                           qmf_t X_hybrid_left[32][32], qmf_t X_hybrid_right[32][32])
{
    uint8_t gr, n, m, bk;
    uint8_t temp_delay;
    uint8_t sb, maxsb;
    const complex_t *Phi_Fract_SubQmf;
    uint8_t temp_delay_ser[NO_ALLPASS_LINKS];
    real_t P_SmoothPeakDecayDiffNrg, nrg;
    real_t P[32][34];
    real_t G_TransientRatio[32][34] = {{0}};
    complex_t inputLeft;
    static  const real_t gamma = COEF_CONST(1.5);
    static  const real_t nReal1 = REAL_CONST(1.0);
    static  const real_t nFrac1 =  FRAC_CONST(1.0);
    real_t g_DecaySlope;
    real_t g_DecaySlope_filt[NO_ALLPASS_LINKS];
    
    int nTemp,nTemp2;
    int nTemp3,i;
    nTemp = REAL_BITS-1;
    /* chose hybrid filterbank: 20 or 34 band case */
    if (ps->use34hybrid_bands)
    {
        Phi_Fract_SubQmf = Phi_Fract_SubQmf34;
    } else{
        Phi_Fract_SubQmf = Phi_Fract_SubQmf20;
    }
    /* clear the energy values */
   // for (n = 0; n < 32; n++)
   // {
   //     for (bk = 0; bk < 34; bk++)
   //     {
   //         P[n][bk] = 0;
   //     }
   // }
    memset(P, 0, sizeof(P));
    nTemp2 = 8;
    /* calculate the energy in each parameter band b(k) */
    for (gr = 0; gr < ps->num_groups; gr++)
    {
        /* select the parameter index b(k) to which this group belongs */
        bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];
        /* select the upper subband border for this group */
        maxsb = (gr < ps->num_hybrid_groups) ? ps->group_border[gr]+1 : ps->group_border[gr+1];

        for (sb = ps->group_border[gr]; sb < maxsb; sb++)
        {
            for (n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n=n+nTemp2)//n++)
            {
#ifdef FIXED_POINT
                uint32_t in_re, in_im;
#endif
                /* input from hybrid subbands or QMF subbands */
                if (gr < ps->num_hybrid_groups)
                {
                    //RE(inputLeft) = QMF_RE(X_hybrid_left[n][sb]);
                    //IM(inputLeft) = QMF_IM(X_hybrid_left[n][sb]);
                    
                    in_re = ((QMF_RE(X_hybrid_left[n][sb])+(1<<(nTemp)))>>REAL_BITS);
                    in_im = (((QMF_IM(X_hybrid_left[n][sb]))+(1<<(nTemp)))>>REAL_BITS);                                       

                    //asm volatile("l.mac %0, %1" : : "r"(QMF_RE(X_hybrid_left[n][sb])>>REAL_BITS), "r"(QMF_RE(X_hybrid_left[n][sb])>>REAL_BITS));
                    //asm volatile("l.mac %0, %1" : : "r"(QMF_IM(X_hybrid_left[n][sb])>>REAL_BITS), "r"(QMF_IM(X_hybrid_left[n][sb])>>REAL_BITS));

                } else {
                    //RE(inputLeft) = QMF_RE(X_left[n][sb]);
                    //IM(inputLeft) = QMF_IM(X_left[n][sb]);
                    
                    in_re = ((QMF_RE(X_left[n][sb])+(1<<(nTemp)))>>REAL_BITS);
                    in_im = ((QMF_IM(X_left[n][sb])+(1<<(nTemp)))>>REAL_BITS);      

                    //asm volatile("l.mac %0, %1" : : "r"(QMF_RE(X_left[n][sb])>>REAL_BITS), "r"(QMF_RE(X_left[n][sb])>>REAL_BITS));
                    //asm volatile("l.mac %0, %1" : : "r"(QMF_IM(X_left[n][sb])>>REAL_BITS), "r"(QMF_IM(X_left[n][sb])>>REAL_BITS));
                }

                /* accumulate energy */
#ifdef FIXED_POINT
                /* NOTE: all input is scaled by 2^(-5) because of fixed point QMF
                 * meaning that P will be scaled by 2^(-10) compared to floating point version
                 */
    #if 0                 
                in_re = ((abs(RE(inputLeft))+(1<<(REAL_BITS-1)))>>REAL_BITS);
                in_im = ((abs(IM(inputLeft))+(1<<(REAL_BITS-1)))>>REAL_BITS);
                P[n][bk] += in_re*in_re + in_im*in_im;
    #else 
                //in_re = ((abs(RE(inputLeft))+(1<<(nTemp)))>>REAL_BITS);
                //in_im = ((abs(IM(inputLeft))+(1<<(nTemp)))>>REAL_BITS);

                //asm volatile("l.mac %0, %1" : : "r"(in_re), "r"(in_re));
                //asm volatile("l.mac %0, %1" : : "r"(in_im), "r"(in_im));        
                P[n][bk] = MUL_R_ADD(in_re,in_re,in_im,in_im);
    #endif
#else
                P[n][bk] += MUL_R(RE(inputLeft),RE(inputLeft)) + MUL_R(IM(inputLeft),IM(inputLeft));
#endif
            }
            //asm volatile("l.macrc %0,%1" : "=r"(P[n-1][bk]): "i"(REAL_BITS));                                                                                                                                
                           
        }
    }

    /* calculate transient reduction ratio for each parameter band b(k) */
    for (bk = 0; bk < ps->nr_par_bands; bk++)
    {
        ps->P_PeakDecayNrg[bk] = MUL_F(ps->P_PeakDecayNrg[bk], ps->alpha_decay);
        for (n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n=n+nTemp2)//n++)
        {
            //ps->P_PeakDecayNrg[bk] = MUL_F(ps->P_PeakDecayNrg[bk], ps->alpha_decay);
            if (ps->P_PeakDecayNrg[bk] < P[n][bk])
            {
                ps->P_PeakDecayNrg[bk] = P[n][bk];
            }

            /* apply smoothing filter to peak decay energy */

            P_SmoothPeakDecayDiffNrg = ps->P_SmoothPeakDecayDiffNrg_prev[bk];
            P_SmoothPeakDecayDiffNrg += MUL_F((ps->P_PeakDecayNrg[bk] - P[n][bk] - ps->P_SmoothPeakDecayDiffNrg_prev[bk]), ps->alpha_smooth);           
            ps->P_SmoothPeakDecayDiffNrg_prev[bk] = P_SmoothPeakDecayDiffNrg;
     
            /* apply smoothing filter to energy */
            nrg = ps->P_prev[bk];
            nrg += MUL_F((P[n][bk] - ps->P_prev[bk]), ps->alpha_smooth);           
            ps->P_prev[bk] = nrg;
            /* calculate transient ratio */
#if 0            
            if (MUL_C(P_SmoothPeakDecayDiffNrg, gamma) <= nrg)
            {
                G_TransientRatio[n][bk] = nReal1;
            }
            else
            {
                G_TransientRatio[n][bk] = DIV_R(nrg, (MUL_C(P_SmoothPeakDecayDiffNrg, gamma)));
            }
#else
            nTemp = MUL_C(P_SmoothPeakDecayDiffNrg, gamma);
            if (nTemp<= nrg)
            {
                G_TransientRatio[n][bk] = nReal1;
            }
            else
            {
                G_TransientRatio[n][bk] = DIV_R(nrg, nTemp);
            }
            if (nTemp2 >1)
            {
                for (i=1;i<nTemp2;i++)
                {
                    G_TransientRatio[n+i][bk] = G_TransientRatio[n][bk];
                }
            }

       /* if (nTemp2>0)
            {
                for (i=1;i<nTemp2;i++)
                {
                    G_TransientRatio[n+i][bk] = G_TransientRatio[n][bk];
                }
            }
        */
#endif
        }
    }

#if 0
    for (n = 0; n < 32; n++)
    {
        for (bk = 0; bk < 34; bk++)
        {
#ifdef FIXED_POINT
            printf("%d %d: %f\n", n, bk, G_TransientRatio[n][bk]/(float)REAL_PRECISION);
#else
            printf("%d %d: %f\n", n, bk, G_TransientRatio[n][bk]);
#endif
        }
    }
#endif

//return ;
    /* apply stereo decorrelation filter to the signal */
    for (gr = 0; gr < ps->num_groups; gr++)
    {
        if (gr < ps->num_hybrid_groups)
            maxsb = ps->group_border[gr] + 1;
        else
            maxsb = ps->group_border[gr + 1];

        /* QMF channel */
        for (sb = ps->group_border[gr]; sb < maxsb; sb++)
        {
            /* g_DecaySlope: [0..1] */
            if (gr < ps->num_hybrid_groups || sb <= ps->decay_cutoff)
            {
                g_DecaySlope = nFrac1;
            }
            else
            {
                int8_t decay = ps->decay_cutoff - sb;
                if (decay <= -20 /* -1/DECAY_SLOPE */)
                {
                    g_DecaySlope = 0;
                }
                else
                {
                    /* decay(int)*decay_slope(frac) = g_DecaySlope(frac) */
                    g_DecaySlope = nFrac1+ DECAY_SLOPE * decay;
                }
            }

            /* calculate g_DecaySlope_filt for every m multiplied by filter_a[m] */
#if 0            
            for (m = 0; m < NO_ALLPASS_LINKS; m++)
            {
                g_DecaySlope_filt[m] = MUL_F(g_DecaySlope, filter_a[m]);
            }
#else
            g_DecaySlope_filt[0] = MUL_F(g_DecaySlope, filter_a[0]);
            g_DecaySlope_filt[1] = MUL_F(g_DecaySlope, filter_a[1]);
            g_DecaySlope_filt[2] = MUL_F(g_DecaySlope, filter_a[2]);                
            //g_DecaySlope_filt[1] = g_DecaySlope_filt[0];
            //g_DecaySlope_filt[2] = g_DecaySlope_filt[0];                

            
#endif
            /* set delay indices */
            temp_delay = ps->saved_delay;
#if 0            
            for (n = 0; n < NO_ALLPASS_LINKS; n++)
                temp_delay_ser[n] = ps->delay_buf_index_ser[n];
#else
            temp_delay_ser[0] = ps->delay_buf_index_ser[0];
            temp_delay_ser[1] = ps->delay_buf_index_ser[1];
            temp_delay_ser[2] = ps->delay_buf_index_ser[2];
#endif

            for (n = ps->border_position[0]; n < ps->border_position[ps->num_env]; n = n+nTemp2) //n++)
            {
                complex_t tmp, tmp0, R0;

                if (gr < ps->num_hybrid_groups)
                {
                    /* hybrid filterbank input */
                    RE(inputLeft) = QMF_RE(X_hybrid_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_hybrid_left[n][sb]);
                }
                else
                {
                    /* QMF filterbank input */
                    RE(inputLeft) = QMF_RE(X_left[n][sb]);
                    IM(inputLeft) = QMF_IM(X_left[n][sb]);
                }

                if (sb > ps->nr_allpass_bands && gr >= ps->num_hybrid_groups)
                {
                    /* delay */
                    /* never hybrid subbands here, always QMF subbands */
                    RE(R0) = RE(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]);
                    IM(R0) = IM(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]);
                    //RE(R0) = RE(tmp);
                    //IM(R0) = IM(tmp);
                    RE(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]) = RE(inputLeft);
                    IM(ps->delay_Qmf[ps->delay_buf_index_delay[sb]][sb]) = IM(inputLeft);
                }
                else
                {
                    /* allpass filter */
                    uint8_t m;
                    complex_t Phi_Fract;

                    /* fetch parameters */
                    if (gr < ps->num_hybrid_groups)
                    {
                        /* select data from the hybrid subbands */
                        //RE(tmp0) = RE(ps->delay_SubQmf[temp_delay][sb]);
                        //IM(tmp0) = IM(ps->delay_SubQmf[temp_delay][sb]);
                        RE(ps->delay_SubQmf[temp_delay][sb]) = RE(inputLeft);
                        IM(ps->delay_SubQmf[temp_delay][sb]) = IM(inputLeft);
                        //RE(Phi_Fract) = RE(Phi_Fract_SubQmf[sb]);
                        //IM(Phi_Fract) = IM(Phi_Fract_SubQmf[sb]);
                        ComplexMult(&RE(R0), &IM(R0), RE(ps->delay_SubQmf[temp_delay][sb]), IM(ps->delay_SubQmf[temp_delay][sb]), RE(Phi_Fract_SubQmf[sb]), IM(Phi_Fract_SubQmf[sb]));
                    } 
                    else 
                    {
                        /* select data from the QMF subbands */
                        //RE(tmp0) = RE(ps->delay_Qmf[temp_delay][sb]);
                        //IM(tmp0) = IM(ps->delay_Qmf[temp_delay][sb]);
                        RE(ps->delay_Qmf[temp_delay][sb]) = RE(inputLeft);
                        IM(ps->delay_Qmf[temp_delay][sb]) = IM(inputLeft);
                        //RE(Phi_Fract) = RE(Phi_Fract_Qmf[sb]);
                        //IM(Phi_Fract) = IM(Phi_Fract_Qmf[sb]);
                        ComplexMult(&RE(R0), &IM(R0), RE(ps->delay_Qmf[temp_delay][sb]), IM(ps->delay_Qmf[temp_delay][sb]), RE(Phi_Fract_Qmf[sb]), IM(Phi_Fract_Qmf[sb]));
                    }

                    /* z^(-2) * Phi_Fract[k] */
                    //ComplexMult(&RE(tmp), &IM(tmp), RE(tmp0), IM(tmp0), RE(Phi_Fract), IM(Phi_Fract));
                    //RE(R0) = RE(tmp);
                    //IM(R0) = IM(tmp);
                    for (m = 0; m < NO_ALLPASS_LINKS; m++)
                    {
                        complex_t Q_Fract_allpass, tmp2;

                        /* fetch parameters */
                        if (gr < ps->num_hybrid_groups)
                        {
                            /* select data from the hybrid subbands */
                            //RE(tmp0) = RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]);
                            //IM(tmp0) = IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]);
                            if (ps->use34hybrid_bands)
                            {
                                //RE(Q_Fract_allpass) = RE(Q_Fract_allpass_SubQmf34[sb][m]);
                                //IM(Q_Fract_allpass) = IM(Q_Fract_allpass_SubQmf34[sb][m]);
                                ComplexMult(&RE(tmp), &IM(tmp), RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]), IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]), RE(Q_Fract_allpass_SubQmf34[sb][m]), IM(Q_Fract_allpass_SubQmf34[sb][m]));
                            } else {
                                //RE(Q_Fract_allpass) = RE(Q_Fract_allpass_SubQmf20[sb][m]);
                                //IM(Q_Fract_allpass) = IM(Q_Fract_allpass_SubQmf20[sb][m]);
                                ComplexMult(&RE(tmp), &IM(tmp), RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]), IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]), RE(Q_Fract_allpass_SubQmf20[sb][m]), IM(Q_Fract_allpass_SubQmf20[sb][m]));
                            }
                        }
                        else
                        {
                            /* select data from the QMF subbands */
                            //RE(tmp0) = RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]);
                            //IM(tmp0) = IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]);
                            //RE(Q_Fract_allpass) = RE(Q_Fract_allpass_Qmf[sb][m]);
                            //IM(Q_Fract_allpass) = IM(Q_Fract_allpass_Qmf[sb][m]);
                            ComplexMult(&RE(tmp), &IM(tmp), RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]), IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]), RE(Q_Fract_allpass_Qmf[sb][m]), IM(Q_Fract_allpass_Qmf[sb][m]));
                        }

                        /* delay by a fraction */
                        /* z^(-d(m)) * Q_Fract_allpass[k,m] */
                        //ComplexMult(&RE(tmp), &IM(tmp), RE(tmp0), IM(tmp0), RE(Q_Fract_allpass), IM(Q_Fract_allpass));

                        /* -a(m) * g_DecaySlope[k] */
                        RE(tmp) += MUL_SUB_F(g_DecaySlope_filt[m], RE(R0));
                        IM(tmp) += MUL_SUB_F(g_DecaySlope_filt[m], IM(R0));

                        /* -a(m) * g_DecaySlope[k] * Q_Fract_allpass[k,m] * z^(-d(m)) */
                        //RE(tmp2) = RE(R0) + MUL_F(g_DecaySlope_filt[m], RE(tmp));
                        //IM(tmp2) = IM(R0) + MUL_F(g_DecaySlope_filt[m], IM(tmp));

                        /* store sample */
                        if (gr < ps->num_hybrid_groups)
                        {
                            //RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = RE(tmp2);
                            //IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = IM(tmp2);
                            RE(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = RE(R0) + MUL_F(g_DecaySlope_filt[m], RE(tmp));
                            IM(ps->delay_SubQmf_ser[m][temp_delay_ser[m]][sb]) = IM(R0) + MUL_F(g_DecaySlope_filt[m], IM(tmp));                           
                        }
                        else
                        {
                            //RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = RE(tmp2);
                            //IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = IM(tmp2);
                            RE(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = RE(R0) + MUL_F(g_DecaySlope_filt[m], RE(tmp));
                            IM(ps->delay_Qmf_ser[m][temp_delay_ser[m]][sb]) = IM(R0) + MUL_F(g_DecaySlope_filt[m], IM(tmp));                            
                        }

                        /* store for next iteration (or as output value if last iteration) */
                        RE(R0) = RE(tmp);
                        IM(R0) = IM(tmp);
                    }
                }

                /* select b(k) for reading the transient ratio */
                bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];

                /* duck if a past transient is found */
                //RE(R0) = MUL_R(G_TransientRatio[n][bk], RE(R0));
                //IM(R0) = MUL_R(G_TransientRatio[n][bk], IM(R0));
                if (gr < ps->num_hybrid_groups)
                {
                    /* hybrid */
                    //QMF_RE(X_hybrid_right[n][sb]) = RE(R0);
                    //QMF_IM(X_hybrid_right[n][sb]) = IM(R0);
                    QMF_RE(X_hybrid_right[n][sb]) = MUL_R(G_TransientRatio[n][bk], RE(R0));
                    QMF_IM(X_hybrid_right[n][sb]) = MUL_R(G_TransientRatio[n][bk], IM(R0));   
                    if (nTemp2>1)
                    {
                        nTemp = ((QMF_RE(X_hybrid_right[n][sb])+QMF_RE(X_hybrid_right[n-1][sb]))>>1)<<1;
                        nTemp3= ((QMF_IM(X_hybrid_right[n][sb])+QMF_IM(X_hybrid_right[n-1][sb]))>>1)<<1;     
                    
                        for (i =1;i<nTemp2;i++)
                        {
                            if (n>0 && i>(nTemp2/2))
                            {
                                QMF_RE(X_hybrid_right[n+i][sb]) = nTemp;
                                QMF_IM(X_hybrid_right[n+i][sb]) = nTemp3;   
                            }                        
                            else
                            {
                                QMF_RE(X_hybrid_right[n+i][sb]) = QMF_RE(X_hybrid_right[n][sb]);
                                QMF_IM(X_hybrid_right[n+i][sb]) = QMF_IM(X_hybrid_right[n][sb]);   
                            }
                        }
                    }
                } 
                else
                {
                    /* QMF */
                    //QMF_RE(X_right[n][sb]) = RE(R0);
                    //QMF_IM(X_right[n][sb]) = IM(R0);
                    QMF_RE(X_right[n][sb]) = MUL_R(G_TransientRatio[n][bk], RE(R0));
                    QMF_IM(X_right[n][sb]) = MUL_R(G_TransientRatio[n][bk], IM(R0));
                    if (nTemp2>1)
                    {
                        nTemp = ((QMF_RE(X_right[n][sb])+QMF_RE(X_right[n-1][sb]))>>1)<<1;
                        nTemp3= ((QMF_IM(X_right[n][sb])+QMF_IM(X_right[n-1][sb]))>>1)<<1;     
                    
                        for (i =1;i<nTemp2;i++)
                        {                       
                            if (n>0 && i>(nTemp2/2))
                            {
                                QMF_RE(X_right[n+i][sb]) = nTemp;
                                QMF_IM(X_right[n+i][sb]) = nTemp3;   
                            }
                            else
                            {
                                QMF_RE(X_right[n+i][sb]) = QMF_RE(X_right[n][sb]);
                                QMF_IM(X_right[n+i][sb]) = QMF_IM(X_right[n][sb]);   
                            }
                        }
                    }                   
                }

                /* Update delay buffer index */
                if (++temp_delay >= 2)
                {
                    temp_delay = 0;
                }

                /* update delay indices */
                if (sb > ps->nr_allpass_bands && gr >= ps->num_hybrid_groups)
                {
                    /* delay_D depends on the samplerate, it can hold the values 14 and 1 */
                    if (++ps->delay_buf_index_delay[sb] >= ps->delay_D[sb])
                    {
                        ps->delay_buf_index_delay[sb] = 0;
                    }
                }

                for (m = 0; m < NO_ALLPASS_LINKS; m++)
                {
                    if (++temp_delay_ser[m] >= ps->num_sample_delay_ser[m])
                    {
                        temp_delay_ser[m] = 0;
                    }
                }
            }
        }
    }

    /* update delay indices */
    ps->saved_delay = temp_delay;

    //for (m = 0; m < NO_ALLPASS_LINKS; m++)
  //    ps->delay_buf_index_ser[m] = temp_delay_ser[m];
    ps->delay_buf_index_ser[0] = temp_delay_ser[0];    
    ps->delay_buf_index_ser[1] = temp_delay_ser[1];    
    ps->delay_buf_index_ser[2] = temp_delay_ser[2];        
    
}

#ifdef FIXED_POINT
#define step(shift) \
    if ((0x40000000l >> shift) + root <= value)       \
    {                                                 \
        value -= (0x40000000l >> shift) + root;       \
        root = (root >> 1) | (0x40000000l >> shift);  \
    } else {                                          \
        root = root >> 1;                             \
    }

/* fixed point square root approximation */
static real_t ps_sqrt(real_t value)
{
    real_t root = 0;

    step( 0); step( 2); step( 4); step( 6);
    step( 8); step(10); step(12); step(14);
    step(16); step(18); step(20); step(22);
    step(24); step(26); step(28); step(30);

    if (root < value)
        ++root;

    root <<= (REAL_BITS/2);

    return root;
}
#else
#define ps_sqrt(A) sqrt(A)
#endif

static const real_t ipdopd_cos_tab[] = {
    FRAC_CONST(1.000000000000000),
    FRAC_CONST(0.707106781186548),
    FRAC_CONST(0.000000000000000),
    FRAC_CONST(-0.707106781186547),
    FRAC_CONST(-1.000000000000000),
    FRAC_CONST(-0.707106781186548),
    FRAC_CONST(-0.000000000000000),
    FRAC_CONST(0.707106781186547),
    FRAC_CONST(1.000000000000000)
};

static const real_t ipdopd_sin_tab[] = {
    FRAC_CONST(0.000000000000000),
    FRAC_CONST(0.707106781186547),
    FRAC_CONST(1.000000000000000),
    FRAC_CONST(0.707106781186548),
    FRAC_CONST(0.000000000000000),
    FRAC_CONST(-0.707106781186547),
    FRAC_CONST(-1.000000000000000),
    FRAC_CONST(-0.707106781186548),
    FRAC_CONST(-0.000000000000000)
};

static real_t magnitude_c(complex_t c)
{
#ifdef FIXED_POINT
#define ps_abs(A) (((A) > 0) ? (A) : (-(A)))
#define ALPHA FRAC_CONST(0.948059448969)
#define BETA  FRAC_CONST(0.392699081699)

    real_t abs_inphase = ps_abs(RE(c));
    real_t abs_quadrature = ps_abs(IM(c));

    if (abs_inphase > abs_quadrature)
    {      
        return MUL_F(abs_inphase, ALPHA) + MUL_F(abs_quadrature, BETA);       
        //return MUL_F_ADD(abs_inphase,ALPHA,abs_quadrature,BETA);
    }
    else
    {   
        return MUL_F(abs_quadrature, ALPHA) + MUL_F(abs_inphase, BETA);
        //return MUL_F_ADD(abs_quadrature,ALPHA,abs_inphase,BETA);        
    }
#else
    return sqrt(RE(c)*RE(c) + IM(c)*IM(c));
#endif
}

void ps_mix_phase(ps_info *ps, qmf_t X_left[38][64], qmf_t X_right[38][64],
                         qmf_t X_hybrid_left[32][32], qmf_t X_hybrid_right[32][32])
{
    uint8_t n;
    uint8_t gr;
    uint8_t bk = 0;
    uint8_t sb, maxsb;
    uint8_t env;
    uint8_t nr_ipdopd_par;
    complex_t h11, h12, h21, h22;
    complex_t H11, H12, H21, H22;
    complex_t deltaH11, deltaH12, deltaH21, deltaH22;
    complex_t tempLeft;
    complex_t tempRight;
    complex_t phaseLeft;
    complex_t phaseRight;
    real_t L;
    const real_t *sf_iid;
    uint8_t no_iid_steps;
    int8_t nTemp;

    if (ps->iid_mode >= 3)
    {
        no_iid_steps = 15;
        sf_iid = sf_iid_fine;
    } 
    else
    {
        no_iid_steps = 7;
        sf_iid = sf_iid_normal;
    }

    if (ps->ipd_mode == 0 || ps->ipd_mode == 3)
    {
        nr_ipdopd_par = 11; /* resolution */
    } else {
        nr_ipdopd_par = ps->nr_ipdopd_par;
    }

    for (gr = 0; gr < ps->num_groups; gr++)
    {
        bk = (~NEGATE_IPD_MASK) & ps->map_group2bk[gr];

        /* use one channel per group in the subqmf domain */
        maxsb = (gr < ps->num_hybrid_groups) ? ps->group_border[gr] + 1 : ps->group_border[gr + 1];

        for (env = 0; env < ps->num_env; env++)
        {
            nTemp = abs(ps->iid_index[env][bk]);
        
            if (ps->icc_mode < 3)
            {
                /* type 'A' mixing as described in 8.6.4.6.2.1 */
                real_t c_1, c_2;
                real_t cosa, sina;
                real_t cosb, sinb;
                real_t ab1, ab2;
                real_t ab3, ab4;

                /*
                c_1 = sqrt(2.0 / (1.0 + pow(10.0, quant_iid[no_iid_steps + iid_index] / 10.0)));
                c_2 = sqrt(2.0 / (1.0 + pow(10.0, quant_iid[no_iid_steps - iid_index] / 10.0)));
                alpha = 0.5 * acos(quant_rho[icc_index]);
                beta = alpha * ( c_1 - c_2 ) / sqrt(2.0);
                */

                //printf("%d\n", ps->iid_index[env][bk]);

                /* calculate the scalefactors c_1 and c_2 from the intensity differences */
                c_1 = sf_iid[no_iid_steps + ps->iid_index[env][bk]];
                c_2 = sf_iid[no_iid_steps - ps->iid_index[env][bk]];

                /* calculate alpha and beta using the ICC parameters */
                cosa = cos_alphas[ps->icc_index[env][bk]];
                sina = sin_alphas[ps->icc_index[env][bk]];

                if (ps->iid_mode >= 3)
                {
                    if (ps->iid_index[env][bk] < 0)
                    {
                        cosb =  cos_betas_fine[nTemp][ps->icc_index[env][bk]];
                        //sinb = -sin_betas_fine[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = sin_betas_fine_m[nTemp][ps->icc_index[env][bk]];
                    } 
                    else
                    {
                        cosb = cos_betas_fine[nTemp][ps->icc_index[env][bk]];
                        sinb = sin_betas_fine[nTemp][ps->icc_index[env][bk]];
                    }
                }
                else
                {
                    if (ps->iid_index[env][bk] < 0)
                    {
                        cosb =  cos_betas_normal[nTemp][ps->icc_index[env][bk]];
                        //sinb = -sin_betas_normal[-ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                        sinb = sin_betas_normal_m[nTemp][ps->icc_index[env][bk]];
                    } 
                    else
                    {
                        cosb = cos_betas_normal[nTemp][ps->icc_index[env][bk]];
                        sinb = sin_betas_normal[nTemp][ps->icc_index[env][bk]];
                    }
                }

#if 1   
                ab1 = MUL_C(cosb, cosa);
                ab2 = MUL_C(sinb, sina);
                ab3 = MUL_C(sinb, cosa);
                ab4 = MUL_C(cosb, sina);

                /* h_xy: COEF */
                RE(h11) = MUL_C(c_2, (ab1 - ab2));
                RE(h12) = MUL_C(c_1, (ab1 + ab2));
                RE(h21) = MUL_C(c_2, (ab3 + ab4));
                RE(h22) = MUL_C(c_1, (ab3 - ab4));
#else                
                RE(h11) = MUL_C(c_2,MUL_C_SUB(cosb,cosb,sinb,sina));
                RE(h12) = MUL_C(c_1,MUL_C_ADD(cosb,cosb,sinb,sina));
                RE(h21) = MUL_C(c_2, MUL_C_ADD(sinb,cosa,cosb,sina));
                RE(h22) = MUL_C(c_1, MUL_C_SUB(sinb,cosa,cosb,sina));
#endif                

           
            }
            else  // ps->icc_mode < 3
            {
                /* type 'B' mixing as described in 8.6.4.6.2.2 */
                real_t sina, cosa;
                real_t cosg, sing;

                /*
                real_t c, rho, mu, alpha, gamma;
                uint8_t i;

                i = ps->iid_index[env][bk];
                c = (real_t)pow(10.0, ((i)?(((i>0)?1:-1)*quant_iid[((i>0)?i:-i)-1]):0.)/20.0);
                rho = quant_rho[ps->icc_index[env][bk]];

                if (rho == 0.0f && c == 1.)
                {
                    alpha = (real_t)M_PI/4.0f;
                    rho = 0.05f;
                } else {
                    if (rho <= 0.05f)
                    {
                        rho = 0.05f;
                    }
                    alpha = 0.5f*(real_t)atan( (2.0f*c*rho) / (c*c-1.0f) );

                    if (alpha < 0.)
                    {
                        alpha += (real_t)M_PI/2.0f;
                    }
                    if (rho < 0.)
                    {
                        alpha += (real_t)M_PI;
                    }
                }
                mu = c+1.0f/c;
                mu = 1+(4.0f*rho*rho-4.0f)/(mu*mu);
                gamma = (real_t)atan(sqrt((1.0f-sqrt(mu))/(1.0f+sqrt(mu))));
                */

                if (ps->iid_mode >= 3)
                {
                    //uint8_t abs_iid = nTemp; //abs(ps->iid_index[env][bk]);

                    cosa = sincos_alphas_B_fine[no_iid_steps + ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    sina = sincos_alphas_B_fine[30 - (no_iid_steps + ps->iid_index[env][bk])][ps->icc_index[env][bk]];
                    cosg = cos_gammas_fine[nTemp][ps->icc_index[env][bk]];
                    sing = sin_gammas_fine[nTemp][ps->icc_index[env][bk]];
                } else {
                    //uint8_t abs_iid = nTemp; //abs(ps->iid_index[env][bk]);

                    cosa = sincos_alphas_B_normal[no_iid_steps + ps->iid_index[env][bk]][ps->icc_index[env][bk]];
                    sina = sincos_alphas_B_normal[14 - (no_iid_steps + ps->iid_index[env][bk])][ps->icc_index[env][bk]];
                    cosg = cos_gammas_normal[nTemp][ps->icc_index[env][bk]];
                    sing = sin_gammas_normal[nTemp][ps->icc_index[env][bk]];
                }

                //RE(h11) = MUL_C(COEF_SQRT2, MUL_C(cosa, cosg));
                //RE(h12) = MUL_C(COEF_SQRT2, MUL_C(sina, cosg));
                //RE(h21) = MUL_C(COEF_SQRT2, MUL_C(-cosa, sing));
                //RE(h22) = MUL_C(COEF_SQRT2, MUL_C(sina, sing));
                RE(h11) = MUL_C(cosa, cosg);
                RE(h12) = MUL_C(sina, cosg);
                RE(h21) = MUL_SUB_C(cosa, sing);
                RE(h22) = MUL_C(sina, sing);                
            }

            /* calculate phase rotation parameters H_xy */
            /* note that the imaginary part of these parameters are only calculated when
               IPD and OPD are enabled
             */
            if ((ps->enable_ipdopd) && (bk < nr_ipdopd_par))
            {
                int8_t i;
                real_t xy, pq, xypq;

                /* ringbuffer index */
                i = ps->phase_hist;

                /* previous value */
#ifdef FIXED_POINT
                /* divide by 4, shift right 2 bits */
                RE(tempLeft)  = RE(ps->ipd_prev[bk][i]) >> 2;
                IM(tempLeft)  = IM(ps->ipd_prev[bk][i]) >> 2;
                RE(tempRight) = RE(ps->opd_prev[bk][i]) >> 2;
                IM(tempRight) = IM(ps->opd_prev[bk][i]) >> 2;
#else
                RE(tempLeft)  = MUL_F(RE(ps->ipd_prev[bk][i]), FRAC_CONST(0.25));
                IM(tempLeft)  = MUL_F(IM(ps->ipd_prev[bk][i]), FRAC_CONST(0.25));
                RE(tempRight) = MUL_F(RE(ps->opd_prev[bk][i]), FRAC_CONST(0.25));
                IM(tempRight) = MUL_F(IM(ps->opd_prev[bk][i]), FRAC_CONST(0.25));
#endif

                /* save current value */
                RE(ps->ipd_prev[bk][i]) = ipdopd_cos_tab[abs(ps->ipd_index[env][bk])];
                IM(ps->ipd_prev[bk][i]) = ipdopd_sin_tab[abs(ps->ipd_index[env][bk])];
                RE(ps->opd_prev[bk][i]) = ipdopd_cos_tab[abs(ps->opd_index[env][bk])];
                IM(ps->opd_prev[bk][i]) = ipdopd_sin_tab[abs(ps->opd_index[env][bk])];

                /* add current value */
                RE(tempLeft)  += RE(ps->ipd_prev[bk][i]);
                IM(tempLeft)  += IM(ps->ipd_prev[bk][i]);
                RE(tempRight) += RE(ps->opd_prev[bk][i]);
                IM(tempRight) += IM(ps->opd_prev[bk][i]);

                /* ringbuffer index */
                if (i == 0)
                {
                    i = 2;
                }
                i--;

                /* get value before previous */
#ifdef FIXED_POINT
                /* dividing by 2, shift right 1 bit */
                RE(tempLeft)  += (RE(ps->ipd_prev[bk][i]) >> 1);
                IM(tempLeft)  += (IM(ps->ipd_prev[bk][i]) >> 1);
                RE(tempRight) += (RE(ps->opd_prev[bk][i]) >> 1);
                IM(tempRight) += (IM(ps->opd_prev[bk][i]) >> 1);
#else
                RE(tempLeft)  += MUL_F(RE(ps->ipd_prev[bk][i]), FRAC_CONST(0.5));
                IM(tempLeft)  += MUL_F(IM(ps->ipd_prev[bk][i]), FRAC_CONST(0.5));
                RE(tempRight) += MUL_F(RE(ps->opd_prev[bk][i]), FRAC_CONST(0.5));
                IM(tempRight) += MUL_F(IM(ps->opd_prev[bk][i]), FRAC_CONST(0.5));
#endif

#if 0 /* original code */
                ipd = (float)atan2(IM(tempLeft), RE(tempLeft));
                opd = (float)atan2(IM(tempRight), RE(tempRight));

                /* phase rotation */
                RE(phaseLeft) = (float)cos(opd);
                IM(phaseLeft) = (float)sin(opd);
                opd -= ipd;
                RE(phaseRight) = (float)cos(opd);
                IM(phaseRight) = (float)sin(opd);
#else

                // x = IM(tempLeft)
                // y = RE(tempLeft)
                // p = IM(tempRight)
                // q = RE(tempRight)
                // cos(atan2(x,y)) = y/sqrt((x*x) + (y*y))
                // sin(atan2(x,y)) = x/sqrt((x*x) + (y*y))
                // cos(atan2(x,y)-atan2(p,q)) = (y*q + x*p) / ( sqrt((x*x) + (y*y)) * sqrt((p*p) + (q*q)) );
                // sin(atan2(x,y)-atan2(p,q)) = (x*q - y*p) / ( sqrt((x*x) + (y*y)) * sqrt((p*p) + (q*q)) );

                xy = magnitude_c(tempRight);
                pq = magnitude_c(tempLeft);

                if (xy != 0)
                {
                    RE(phaseLeft) = DIV_R(RE(tempRight), xy);
                    IM(phaseLeft) = DIV_R(IM(tempRight), xy);
                } else {
                    RE(phaseLeft) = 0;
                    IM(phaseLeft) = 0;
                }

                xypq = MUL_R(xy, pq);

                if (xypq != 0)
                {
                    real_t tmp1 = MUL_R(RE(tempRight), RE(tempLeft)) + MUL_R(IM(tempRight), IM(tempLeft));
                    real_t tmp2 = MUL_R(IM(tempRight), RE(tempLeft)) - MUL_R(RE(tempRight), IM(tempLeft));

                    RE(phaseRight) = DIV_R(tmp1, xypq);
                    IM(phaseRight) = DIV_R(tmp2, xypq);
                } else {
                    RE(phaseRight) = 0;
                    IM(phaseRight) = 0;
                }

#endif

                /* MUL_F(COEF, REAL) = COEF */
                IM(h11) = MUL_R(RE(h11), IM(phaseLeft));
                IM(h12) = MUL_R(RE(h12), IM(phaseRight));
                IM(h21) = MUL_R(RE(h21), IM(phaseLeft));
                IM(h22) = MUL_R(RE(h22), IM(phaseRight));

                RE(h11) = MUL_R(RE(h11), RE(phaseLeft));
                RE(h12) = MUL_R(RE(h12), RE(phaseRight));
                RE(h21) = MUL_R(RE(h21), RE(phaseLeft));
                RE(h22) = MUL_R(RE(h22), RE(phaseRight));
            }

            /* length of the envelope n_e+1 - n_e (in time samples) */
            /* 0 < L <= 32: integer */
            L = (real_t)(ps->border_position[env + 1] - ps->border_position[env]);

            /* obtain final H_xy by means of linear interpolation */
            RE(deltaH11) = (RE(h11) - RE(ps->h11_prev[gr])) / L;
            RE(deltaH12) = (RE(h12) - RE(ps->h12_prev[gr])) / L;
            RE(deltaH21) = (RE(h21) - RE(ps->h21_prev[gr])) / L;
            RE(deltaH22) = (RE(h22) - RE(ps->h22_prev[gr])) / L;

            RE(H11) = RE(ps->h11_prev[gr]);
            RE(H12) = RE(ps->h12_prev[gr]);
            RE(H21) = RE(ps->h21_prev[gr]);
            RE(H22) = RE(ps->h22_prev[gr]);

            RE(ps->h11_prev[gr]) = RE(h11);
            RE(ps->h12_prev[gr]) = RE(h12);
            RE(ps->h21_prev[gr]) = RE(h21);
            RE(ps->h22_prev[gr]) = RE(h22);

            /* only calculate imaginary part when needed */
            if ((ps->enable_ipdopd) && (bk < nr_ipdopd_par))
            {
                /* obtain final H_xy by means of linear interpolation */
                IM(deltaH11) = (IM(h11) - IM(ps->h11_prev[gr])) / L;
                IM(deltaH12) = (IM(h12) - IM(ps->h12_prev[gr])) / L;
                IM(deltaH21) = (IM(h21) - IM(ps->h21_prev[gr])) / L;
                IM(deltaH22) = (IM(h22) - IM(ps->h22_prev[gr])) / L;

                IM(H11) = IM(ps->h11_prev[gr]);
                IM(H12) = IM(ps->h12_prev[gr]);
                IM(H21) = IM(ps->h21_prev[gr]);
                IM(H22) = IM(ps->h22_prev[gr]);

                if ((NEGATE_IPD_MASK & ps->map_group2bk[gr]) != 0)
                {
                    IM(deltaH11) = -IM(deltaH11);
                    IM(deltaH12) = -IM(deltaH12);
                    IM(deltaH21) = -IM(deltaH21);
                    IM(deltaH22) = -IM(deltaH22);

                    IM(H11) = -IM(H11);
                    IM(H12) = -IM(H12);
                    IM(H21) = -IM(H21);
                    IM(H22) = -IM(H22);
                }

                IM(ps->h11_prev[gr]) = IM(h11);
                IM(ps->h12_prev[gr]) = IM(h12);
                IM(ps->h21_prev[gr]) = IM(h21);
                IM(ps->h22_prev[gr]) = IM(h22);
            }

            /* apply H_xy to the current envelope band of the decorrelated subband */
            for (n = ps->border_position[env]; n < ps->border_position[env + 1]; n++)
            {
                /* addition finalises the interpolation over every n */
                RE(H11) += RE(deltaH11);
                RE(H12) += RE(deltaH12);
                RE(H21) += RE(deltaH21);
                RE(H22) += RE(deltaH22);
                if ((ps->enable_ipdopd) && (bk < nr_ipdopd_par))
                {
                    IM(H11) += IM(deltaH11);
                    IM(H12) += IM(deltaH12);
                    IM(H21) += IM(deltaH21);
                    IM(H22) += IM(deltaH22);
                }

                /* channel is an alias to the subband */
                for (sb = ps->group_border[gr]; sb < maxsb; sb++)
                {
                    complex_t inLeft, inRight;

                    /* load decorrelated samples */
                    if (gr < ps->num_hybrid_groups)
                    {
                        RE(inLeft) =  RE(X_hybrid_left[n][sb]);
                        IM(inLeft) =  IM(X_hybrid_left[n][sb]);
                        RE(inRight) = RE(X_hybrid_right[n][sb]);
                        IM(inRight) = IM(X_hybrid_right[n][sb]);
                    } else {
                        RE(inLeft) =  RE(X_left[n][sb]);
                        IM(inLeft) =  IM(X_left[n][sb]);
                        RE(inRight) = RE(X_right[n][sb]);
                        IM(inRight) = IM(X_right[n][sb]);
                    }

                    /* apply mixing */
                    //RE(tempLeft) =  MUL_C(RE(H11), RE(inLeft)) + MUL_C(RE(H21), RE(inRight));
                    //IM(tempLeft) =  MUL_C(RE(H11), IM(inLeft)) + MUL_C(RE(H21), IM(inRight));                    
                    
                    //RE(tempRight) = MUL_C(RE(H12), RE(inLeft)) + MUL_C(RE(H22), RE(inRight));
                    //IM(tempRight) = MUL_C(RE(H12), IM(inLeft)) + MUL_C(RE(H22), IM(inRight));

                    /* only perform imaginary operations when needed */
                    if ((ps->enable_ipdopd) && (bk < nr_ipdopd_par))
                    {
                        /* apply rotation */
                        RE(tempLeft)  -= MUL_C(IM(H11), IM(inLeft)) + MUL_C(IM(H21), IM(inRight));
                        IM(tempLeft)  += MUL_C(IM(H11), RE(inLeft)) + MUL_C(IM(H21), RE(inRight));
                        RE(tempRight) -= MUL_C(IM(H12), IM(inLeft)) + MUL_C(IM(H22), IM(inRight));
                        IM(tempRight) += MUL_C(IM(H12), RE(inLeft)) + MUL_C(IM(H22), RE(inRight));
                    }

                    /* store final samples */
                    if (gr < ps->num_hybrid_groups)
                    {
                        RE(X_hybrid_left[n][sb]) = MUL_C_ADD(RE(H11),RE(inLeft), RE(H21), RE(inRight));
                        IM(X_hybrid_left[n][sb]) = MUL_C_ADD(RE(H11),IM(inLeft), RE(H21), IM(inRight));
                        RE(X_hybrid_right[n][sb]) = MUL_C_ADD(RE(H12),RE(inLeft),RE(H22), RE(inRight));
                        IM(X_hybrid_right[n][sb]) = MUL_C_ADD(RE(H12),IM(inLeft),RE(H22), IM(inRight));
                    
                        //RE(X_hybrid_left[n][sb])  = RE(tempLeft);
                        //IM(X_hybrid_left[n][sb])  = IM(tempLeft);
                        //RE(X_hybrid_right[n][sb]) = RE(tempRight);
                        //IM(X_hybrid_right[n][sb]) = IM(tempRight);
                    } else {
                        RE(X_left[n][sb]) = MUL_C_ADD(RE(H11),RE(inLeft), RE(H21), RE(inRight));                       
                        IM(X_left[n][sb]) = MUL_C_ADD(RE(H11),IM(inLeft),RE(H21), IM(inRight));
                        RE(X_right[n][sb]) = MUL_C_ADD(RE(H12),RE(inLeft),RE(H22), RE(inRight));
                        IM(X_right[n][sb]) = MUL_C_ADD(RE(H12),IM(inLeft),RE(H22), IM(inRight));
                    
                        //RE(X_left[n][sb])  = RE(tempLeft);
                        //IM(X_left[n][sb])  = IM(tempLeft);
                        //RE(X_right[n][sb]) = RE(tempRight);
                        //IM(X_right[n][sb]) = IM(tempRight);
                    }
                }
            }

            /* shift phase smoother's circular buffer index */
            ps->phase_hist++;
            if (ps->phase_hist == 2)
            {
                ps->phase_hist = 0;
            }
        }
    }
}

hyb_info *hybrid_init(uint8_t numTimeSlotsRate)
{
    uint8_t i;

    hyb_info *hyb = &gHyb_info;

    hyb->resolution34[0] = 12;
    hyb->resolution34[1] = 8;
    hyb->resolution34[2] = 4;
    hyb->resolution34[3] = 4;
    hyb->resolution34[4] = 4;

    hyb->resolution20[0] = 8;
    hyb->resolution20[1] = 2;
    hyb->resolution20[2] = 2;
    hyb->frame_len = numTimeSlotsRate;
    memset(hyb->work, 0, (44) * sizeof(qmf_t));
    memset(hyb->buffer, 0, (32)*5 * sizeof(qmf_t));    
    memset(hyb->temp, 0, (32)*12 * sizeof(qmf_t));    

    return hyb;
}

void hybrid_free(hyb_info *hyb)
{
    uint8_t i;

    if (!hyb) return;
}

ps_info *ps_init(uint8_t sr_index, uint8_t numTimeSlotsRate)
{
    unsigned char i;
    unsigned char short_delay_band;
 
    ps_info *ps = &gPS_Info;
    memset(ps, 0, sizeof(ps_info));
#if 1
    ps->hyb = hybrid_init(numTimeSlotsRate);
    ps->numTimeSlotsRate = numTimeSlotsRate;

    ps->ps_data_available = 0;

    /* delay stuff*/
    ps->saved_delay = 0;

    for (i = 0; i < 64; i++)
    {
        ps->delay_buf_index_delay[i] = 0;
    }

    for (i = 0; i < NO_ALLPASS_LINKS; i++)
    {
        ps->delay_buf_index_ser[i] = 0;
#ifdef PARAM_32KHZ
        if (sr_index <= 5) /* >= 32 kHz*/
        {
            ps->num_sample_delay_ser[i] = delay_length_d[1][i];
        } else {
            ps->num_sample_delay_ser[i] = delay_length_d[0][i];
        }
#else
        /* THESE ARE CONSTANTS NOW */
        ps->num_sample_delay_ser[i] = delay_length_d[i];
#endif
    }

#ifdef PARAM_32KHZ
    if (sr_index <= 5) /* >= 32 kHz*/
    {
        short_delay_band = 35;
        ps->nr_allpass_bands = 22;
        ps->alpha_decay = FRAC_CONST(0.76592833836465);
        ps->alpha_smooth = FRAC_CONST(0.25);
    } else {
        short_delay_band = 64;
        ps->nr_allpass_bands = 45;
        ps->alpha_decay = FRAC_CONST(0.58664621951003);
        ps->alpha_smooth = FRAC_CONST(0.6);
    }
#else
    /* THESE ARE CONSTANTS NOW */
    short_delay_band = 35;
    ps->nr_allpass_bands = 22;
    ps->alpha_decay = FRAC_CONST(0.76592833836465);
    ps->alpha_smooth = FRAC_CONST(0.25);
#endif

    /* THESE ARE CONSTANT NOW IF PS IS INDEPENDANT OF SAMPLERATE */
    for (i = 0; i < short_delay_band; i++)
    {
        ps->delay_D[i] = 14;
    }
    for (i = short_delay_band; i < 64; i++)
    {
        ps->delay_D[i] = 1;
    }

    /* mixing and phase */
    for (i = 0; i < 50; i++)
    {
        RE(ps->h11_prev[i]) = 1;
        IM(ps->h12_prev[i]) = 1;
        RE(ps->h11_prev[i]) = 1;
        IM(ps->h12_prev[i]) = 1;
    }

    ps->phase_hist = 0;

    for (i = 0; i < 20; i++)
    {
        RE(ps->ipd_prev[i][0]) = 0;
        IM(ps->ipd_prev[i][0]) = 0;
        RE(ps->ipd_prev[i][1]) = 0;
        IM(ps->ipd_prev[i][1]) = 0;
        RE(ps->opd_prev[i][0]) = 0;
        IM(ps->opd_prev[i][0]) = 0;
        RE(ps->opd_prev[i][1]) = 0;
        IM(ps->opd_prev[i][1]) = 0;
    }
#endif
    return ps;
}

void ps_free(ps_info *ps)
{
    /* free hybrid filterbank structures */
    hybrid_free(ps->hyb);
}

/* main Parametric Stereo decoding function */
uint8_t ps_decode(ps_info *ps, qmf_t X_left[38][64], qmf_t X_right[38][64])
{
    qmf_t X_hybrid_left[32][32] ;
    qmf_t X_hybrid_right[32][32] ;

    /* delta decoding of the bitstream data */
    ps_data_decode(ps);

    /* set up some parameters depending on filterbank type */
    if (ps->use34hybrid_bands)
    {
        ps->group_border = (uint8_t*)group_border34;
        ps->map_group2bk = (uint16_t*)map_group2bk34;
        //ps->num_groups = 32+18;
        ps->num_groups = 50;
        ps->num_hybrid_groups = 32;
        ps->nr_par_bands = 34;
        ps->decay_cutoff = 5;
    } else {
        ps->group_border = (uint8_t*)group_border20;
        ps->map_group2bk = (uint16_t*)map_group2bk20;
        //ps->num_groups = 10+12;
        ps->num_groups = 22;
        ps->num_hybrid_groups = 10;
        ps->nr_par_bands = 20;
        ps->decay_cutoff = 3;
    }

    /* Perform further analysis on the lowest subbands to get a higher
     * frequency resolution
     */
    hybrid_analysis((hyb_info*)ps->hyb, X_left, X_hybrid_left,
        ps->use34hybrid_bands, ps->numTimeSlotsRate);

    /* decorrelate mono signal */
    ps_decorrelate(ps, X_left, X_right, X_hybrid_left, X_hybrid_right);

    /* apply mixing and phase parameters */
    ps_mix_phase(ps, X_left, X_right, X_hybrid_left, X_hybrid_right);

    /* hybrid synthesis, to rebuild the SBR QMF matrices */
    hybrid_synthesis((hyb_info*)ps->hyb, X_left, X_hybrid_left,
        ps->use34hybrid_bands, ps->numTimeSlotsRate);

    hybrid_synthesis((hyb_info*)ps->hyb, X_right, X_hybrid_right,
        ps->use34hybrid_bands, ps->numTimeSlotsRate);

    return 0;
}

#endif

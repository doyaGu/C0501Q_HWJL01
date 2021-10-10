/*!
*  \file tpnp.c
*
* \brief SPX module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"
#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

/*! Table of hanning window values */
static fract_t hannwin[256] =
{
   FRACT(0.00003735666513),
   FRACT(0.00014942107845),
   FRACT(0.00033617649455),
   FRACT(0.00059759500718),
   FRACT(0.00093363755345),
   FRACT(0.00134425391965),
   FRACT(0.00182938274874),
   FRACT(0.00238895154954),
   FRACT(0.00302287670756),
   FRACT(0.00373106349747),
   FRACT(0.00451340609730),
   FRACT(0.00536978760419),
   FRACT(0.00630008005191),
   FRACT(0.00730414442999),
   FRACT(0.00838183070442),
   FRACT(0.00953297784014),
   FRACT(0.01075741382509),
   FRACT(0.01205495569588),
   FRACT(0.01342540956517),
   FRACT(0.01486857065061),
   FRACT(0.01638422330545),
   FRACT(0.01797214105079),
   FRACT(0.01963208660938),
   FRACT(0.02136381194109),
   FRACT(0.02316705828000),
   FRACT(0.02504155617302),
   FRACT(0.02698702552019),
   FRACT(0.02900317561653),
   FRACT(0.03108970519546),
   FRACT(0.03324630247383),
   FRACT(0.03547264519852),
   FRACT(0.03776840069456),
   FRACT(0.04013322591488),
   FRACT(0.04256676749154),
   FRACT(0.04506866178856),
   FRACT(0.04763853495621),
   FRACT(0.05027600298694),
   FRACT(0.05298067177271),
   FRACT(0.05575213716389),
   FRACT(0.05858998502966),
   FRACT(0.06149379131990),
   FRACT(0.06446312212852),
   FRACT(0.06749753375834),
   FRACT(0.07059657278737),
   FRACT(0.07375977613655),
   FRACT(0.07698667113896),
   FRACT(0.08027677561047),
   FRACT(0.08362959792175),
   FRACT(0.08704463707176),
   FRACT(0.09052138276259),
   FRACT(0.09405931547577),
   FRACT(0.09765790654980),
   FRACT(0.10131661825926),
   FRACT(0.10503490389505),
   FRACT(0.10881220784619),
   FRACT(0.11264796568275),
   FRACT(0.11654160424025),
   FRACT(0.12049254170528),
   FRACT(0.12450018770245),
   FRACT(0.12856394338261),
   FRACT(0.13268320151231),
   FRACT(0.13685734656456),
   FRACT(0.14108575481082),
   FRACT(0.14536779441415),
   FRACT(0.14970282552368),
   FRACT(0.15409020037019),
   FRACT(0.15852926336290),
   FRACT(0.16301935118746),
   FRACT(0.16755979290503),
   FRACT(0.17214991005258),
   FRACT(0.17678901674423),
   FRACT(0.18147641977375),
   FRACT(0.18621141871817),
   FRACT(0.19099330604240),
   FRACT(0.19582136720500),
   FRACT(0.20069488076489),
   FRACT(0.20561311848924),
   FRACT(0.21057534546220),
   FRACT(0.21558082019476),
   FRACT(0.22062879473555),
   FRACT(0.22571851478259),
   FRACT(0.23084921979601),
   FRACT(0.23602014311171),
   FRACT(0.24123051205586),
   FRACT(0.24647954806045),
   FRACT(0.25176646677954),
   FRACT(0.25709047820654),
   FRACT(0.26245078679219),
   FRACT(0.26784659156349),
   FRACT(0.27327708624334),
   FRACT(0.27874145937106),
   FRACT(0.28423889442362),
   FRACT(0.28976856993766),
   FRACT(0.29532965963223),
   FRACT(0.30092133253228),
   FRACT(0.30654275309278),
   FRACT(0.31219308132365),
   FRACT(0.31787147291521),
   FRACT(0.32357707936435),
   FRACT(0.32930904810137),
   FRACT(0.33506652261731),
   FRACT(0.34084864259198),
   FRACT(0.34665454402252),
   FRACT(0.35248335935245),
   FRACT(0.35833421760136),
   FRACT(0.36420624449505),
   FRACT(0.37009856259615),
   FRACT(0.37601029143524),
   FRACT(0.38194054764242),
   FRACT(0.38788844507931),
   FRACT(0.39385309497145),
   FRACT(0.39983360604115),
   FRACT(0.40582908464059),
   FRACT(0.41183863488543),
   FRACT(0.41786135878865),
   FRACT(0.42389635639472),
   FRACT(0.42994272591412),
   FRACT(0.43599956385802),
   FRACT(0.44206596517337),
   FRACT(0.44814102337808),
   FRACT(0.45422383069649),
   FRACT(0.46031347819500),
   FRACT(0.46640905591794),
   FRACT(0.47250965302347),
   FRACT(0.47861435791975),
   FRACT(0.48472225840111),
   FRACT(0.49083244178438),
   FRACT(0.49694399504525),
   FRACT(0.50305600495475),
   FRACT(0.50916755821562),
   FRACT(0.51527774159889),
   FRACT(0.52138564208025),
   FRACT(0.52749034697653),
   FRACT(0.53359094408206),
   FRACT(0.53968652180500),
   FRACT(0.54577616930351),
   FRACT(0.55185897662192),
   FRACT(0.55793403482662),
   FRACT(0.56400043614198),
   FRACT(0.57005727408588),
   FRACT(0.57610364360528),
   FRACT(0.58213864121135),
   FRACT(0.58816136511457),
   FRACT(0.59417091535941),
   FRACT(0.60016639395885),
   FRACT(0.60614690502855),
   FRACT(0.61211155492069),
   FRACT(0.61805945235758),
   FRACT(0.62398970856476),
   FRACT(0.62990143740385),
   FRACT(0.63579375550495),
   FRACT(0.64166578239864),
   FRACT(0.64751664064755),
   FRACT(0.65334545597748),
   FRACT(0.65915135740802),
   FRACT(0.66493347738269),
   FRACT(0.67069095189863),
   FRACT(0.67642292063565),
   FRACT(0.68212852708479),
   FRACT(0.68780691867635),
   FRACT(0.69345724690722),
   FRACT(0.69907866746772),
   FRACT(0.70467034036777),
   FRACT(0.71023143006234),
   FRACT(0.71576110557638),
   FRACT(0.72125854062894),
   FRACT(0.72672291375666),
   FRACT(0.73215340843651),
   FRACT(0.73754921320781),
   FRACT(0.74290952179346),
   FRACT(0.74823353322046),
   FRACT(0.75352045193955),
   FRACT(0.75876948794414),
   FRACT(0.76397985688829),
   FRACT(0.76915078020399),
   FRACT(0.77428148521741),
   FRACT(0.77937120526445),
   FRACT(0.78441917980524),
   FRACT(0.78942465453780),
   FRACT(0.79438688151076),
   FRACT(0.79930511923511),
   FRACT(0.80417863279500),
   FRACT(0.80900669395760),
   FRACT(0.81378858128183),
   FRACT(0.81852358022625),
   FRACT(0.82321098325577),
   FRACT(0.82785008994742),
   FRACT(0.83244020709497),
   FRACT(0.83698064881254),
   FRACT(0.84147073663710),
   FRACT(0.84590979962981),
   FRACT(0.85029717447632),
   FRACT(0.85463220558585),
   FRACT(0.85891424518918),
   FRACT(0.86314265343544),
   FRACT(0.86731679848769),
   FRACT(0.87143605661739),
   FRACT(0.87549981229755),
   FRACT(0.87950745829472),
   FRACT(0.88345839575975),
   FRACT(0.88735203431725),
   FRACT(0.89118779215381),
   FRACT(0.89496509610495),
   FRACT(0.89868338174074),
   FRACT(0.90234209345020),
   FRACT(0.90594068452423),
   FRACT(0.90947861723741),
   FRACT(0.91295536292824),
   FRACT(0.91637040207825),
   FRACT(0.91972322438953),
   FRACT(0.92301332886104),
   FRACT(0.92624022386345),
   FRACT(0.92940342721263),
   FRACT(0.93250246624166),
   FRACT(0.93553687787148),
   FRACT(0.93850620868010),
   FRACT(0.94141001497034),
   FRACT(0.94424786283611),
   FRACT(0.94701932822729),
   FRACT(0.94972399701306),
   FRACT(0.95236146504379),
   FRACT(0.95493133821144),
   FRACT(0.95743323250846),
   FRACT(0.95986677408512),
   FRACT(0.96223159930544),
   FRACT(0.96452735480148),
   FRACT(0.96675369752617),
   FRACT(0.96891029480454),
   FRACT(0.97099682438347),
   FRACT(0.97301297447981),
   FRACT(0.97495844382698),
   FRACT(0.97683294172000),
   FRACT(0.97863618805891),
   FRACT(0.98036791339062),
   FRACT(0.98202785894921),
   FRACT(0.98361577669455),
   FRACT(0.98513142934939),
   FRACT(0.98657459043483),
   FRACT(0.98794504430412),
   FRACT(0.98924258617491),
   FRACT(0.99046702215986),
   FRACT(0.99161816929558),
   FRACT(0.99269585557001),
   FRACT(0.99369991994809),
   FRACT(0.99463021239581),
   FRACT(0.99548659390270),
   FRACT(0.99626893650253),
   FRACT(0.99697712329244),
   FRACT(0.99761104845046),
   FRACT(0.99817061725126),
   FRACT(0.99865574608035),
   FRACT(0.99906636244655),
   FRACT(0.99940240499282),
   FRACT(0.99966382350545),
   FRACT(0.99985057892155),
   FRACT(0.99996264333487)
};

/*****************************************************************
* eac3_tpnd_decode: performs TPNP decode
*****************************************************************/
int eac3_tpnp_decode(AC3DecodeContext *s, int  blk, int  ch)               
{
    int tranprocflag = s->tpndinfo[ch].tranprocflag;
    int cblknum  = s->tpndinfo[ch].cblknum;
    int tranlen = s->tpndinfo[ch].tranlen;
    int tranloc = s->tpndinfo[ch].tranloc;
    int opnflag = s->tpndinfo[ch].opnflag;
    int lasttranloc = s->tpndinfo[ch].lasttranloc;
    int *synthwrite = s->tpndinfo[ch].synthwrite;
    int *synthread = s->tpndinfo[ch].synthread;
    int *work_buf = (int*)s->work_buf[ch];
    int ret = 0;

    /* Update block info */
    /* update synthesis buffer write pointer, wrapping around if needed */
    if (tranprocflag == 0) {
        /* looking for transient to process */
        if (lasttranloc < AC3_MINTRANDIST*AC3_BLOCK_SIZE) {
            lasttranloc += AC3_BLOCK_SIZE;
        }
        if (s->transient_proc) {
            if (s->transproc[ch]) {
                tranprocflag = 1;
                cblknum = blk - 2;
                tranlen = s->transproclen[ch];
                tranloc = 4 * s->transprocloc[ch];
                if ((tranloc > AC3_MAXTRANLOC) || (tranloc < AC3_MINTRANLOC)) {
                    printf("TPNP Decode: transient location designated incorrectly "
                                       "in bitstream; skipping pre-noise processing.\n");
                    tranprocflag = 0;
                    ret = -1;
                }
                /* must be at least six blocks between transients */
                if (lasttranloc + tranloc - (blk-1)*AC3_BLOCK_SIZE < AC3_MINTRANDIST * AC3_BLOCK_SIZE) {
                    printf("TPNP Decode: transient locations must be separated "
                                       "by at least six blocks; skipping pre-noise processing.\n");
                    tranprocflag = 0;
                    ret = -1;
                }
            }
        }
    }

    if (tranprocflag == 1) {
        /* currently processing a transient */
        cblknum++;
        if (cblknum*AC3_BLOCK_SIZE > tranloc) {
            /* transient is in past now */
            lasttranloc = cblknum*AC3_BLOCK_SIZE - tranloc;
            tranprocflag = 0;
            opnflag = 0;
        } else {
            /* update overwrite-pre-noise flag */
            int prenoiselen = tranloc - ((tranloc >> 8) - 1) * AC3_BLOCK_SIZE;
            int sbufdeststart = tranloc - prenoiselen - tranlen - AC3_TC1;
            if (sbufdeststart < (cblknum + 1) * AC3_BLOCK_SIZE)
                opnflag = 1;
            else
                opnflag = 0;
        }
    }

    /* copy current block to synth buffer */
    synthwrite += AC3_BLOCK_SIZE;
    if (synthwrite >= (int*)(s->tpndsynthbuf[ch] + AC3_SYNTHBUFLEN)) {
        synthwrite = (int*)s->tpndsynthbuf[ch];
    }
    memcpy(synthwrite, work_buf, (AC3_BLOCK_SIZE * sizeof(int)));

    if (opnflag==1)
    {
        /* Calculate length of pre-noise, assuming 256 <= PN <= 511 */
        /* The prenoise is defined to start at the beginning of the block
           prior to the block containing the transient */
        int prenoiselen = tranloc - ((tranloc / AC3_BLOCK_SIZE) - 1) * AC3_BLOCK_SIZE;
        int firstsamp = cblknum * AC3_BLOCK_SIZE;
        int lastsamp = firstsamp + AC3_BLOCK_SIZE-1;
        int sbufdestlen = prenoiselen + tranlen + AC3_TC1;
        int sbufdeststart = tranloc - sbufdestlen;
        int sampndx, i=0;
        int tmp[256];

        if (firstsamp/AC3_BLOCK_SIZE == sbufdeststart/AC3_BLOCK_SIZE) {
        /* in first block of synthesis buffer destination */
        /* initialize pointer to beginning of synthesis buffer */
            synthread = synthwrite - prenoiselen;
            if (tranlen == 0)
                synthread -= AC3_BLOCK_SIZE;
            
            if (synthread < (int*)s->tpndsynthbuf[ch])
                synthread += AC3_SYNTHBUFLEN;
        }

        for(i=0;i<256;i++)
            tmp[i] = synthread[i];

        /* overwrite pre-noise with values in synthesis buffer */
        for (sampndx = firstsamp, i=0; sampndx <= lastsamp; sampndx++,i++) {
            int tmp_pcm;
            if ((sampndx >= sbufdeststart) && (sampndx < sbufdeststart+AC3_TC1)) {
                int 
                /* In first cross-fade of synthesis buffer:
                   fade-in the synthesis buffer, fade-out the original audio */
                tmp_pcm =  MUL_Shift_30(work_buf[i], hannwin[sbufdeststart+AC3_TC1-sampndx-1], FRACT_SHIFT);
                tmp_pcm += MUL_Shift_30(*synthread, hannwin[sampndx-sbufdeststart], FRACT_SHIFT); 
                work_buf[i] = tmp_pcm;
            } else if ((sampndx >= sbufdeststart+AC3_TC1) && (sampndx < tranloc-AC3_TC2)) {
                /* Copy synthesis buffer to output */
                work_buf[i] = *synthread;
            } else if ((sampndx >= tranloc-AC3_TC2) && (sampndx < tranloc)) {
                /* In second cross-fade of synthesis buffer:
                   fade-in the original audio, fade-out the synthesis buffer */
                tmp_pcm = MUL_Shift_30(work_buf[i], hannwin[(sampndx-tranloc+AC3_TC2)*2], FRACT_SHIFT);
                tmp_pcm += MUL_Shift_30(*synthread, hannwin[(tranloc-sampndx-1)*2], FRACT_SHIFT); 
                work_buf[i] = tmp_pcm;
            }
            if (sampndx >= sbufdeststart && sampndx < tranloc) {
                synthread++;
                if (synthread >= (int*)(s->tpndsynthbuf[ch] + AC3_SYNTHBUFLEN))
                    synthread -= AC3_SYNTHBUFLEN;
            }
        } /* for loop, firstamp -> lastsamp */
    }

    /* restore modified local variables to persistent structs */
    s->tpndinfo[ch].tranprocflag = tranprocflag;
    s->tpndinfo[ch].cblknum = cblknum;
    s->tpndinfo[ch].tranlen = tranlen;
    s->tpndinfo[ch].tranloc = tranloc;
    s->tpndinfo[ch].opnflag = opnflag;
    s->tpndinfo[ch].lasttranloc = lasttranloc;
    s->tpndinfo[ch].synthwrite = synthwrite;
    s->tpndinfo[ch].synthread = synthread;

    return 0;
}

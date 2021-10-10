/*!
*  \file dnmx.c
*
* \brief downmix module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"

extern int32_t ac3_imdct_window[256];

/*! <i>acmod</i> to channels in front */
static const int nfront[8] =
{	2, 1, 2, 3, 2, 3, 2, 3 };

/*! <i>acmod</i> to channels in back */
static const int nrear[8] =
{	0, 0, 0, 0, 1, 1, 2, 2 };

/*! Center mix level table */
static const level_t cmixtab[4] =
{
	LEVEL_M3DB,		/*!< -3.0 dB	*/
	LEVEL_M4_5DB,		/*!< -4.5 dB	*/
	LEVEL_M6DB,		/*!< -6.0 dB	*/
	LEVEL_M4_5DB		/*!< -4.5 dB	*/
};

/*! Surround mix level table */
static const level_t surmixtab[4] =
{
	LEVEL_M3DB,		/*!< -3.0 dB	*/
	LEVEL_M6DB,		/*!< -6.0 dB	*/
	LEVEL_0,			/*!< -inf dB	*/
	LEVEL_M6DB		/*!< -6.0 dB	*/
};

/*! Downmix table for LoRo/LtRt dmxd_process defined in Annex D*/
static const level_t altmixtab[8] =
{
	LEVEL_M3DB,			/*!< +3.0 dB / 2 = M3DB	*/
	LEVEL_M4_5DB,		/*!< +1.5 dB / 2 = M4_5DB	*/
	LEVEL_M6DB,		/*!<  0.0 dB / 2 = M6DB	*/
	LEVEL_M7_5DB,		/*!< -1.5 dB / 2 = M7_5DB	*/
	LEVEL_M9DB,			/*!< -3.0 dB / 2 = M9DB	*/
	LEVEL_M10_5DB,		/*!< -4.5 dB / 2 = M10_5DB	*/
	LEVEL_M12DB,			/*!< -6.0 dB / 2 = M12DB	*/
	LEVEL_0				/*!< -inf dB		*/
};

static const level_t pliisurmixtab[8] =
{
	-LEVEL_1,	/*!< bse_surmixlev = +3.0 dB, pliisurmixval = +6.0 dB */
	-LEVEL_M1_5DB,	/*!< bse_surmixlev = +1.5 dB, pliisurmixval = +4.5 dB */
	-LEVEL_M3DB,	/*!< bse_surmixlev =  0.0 dB, pliisurmixval = +3.5 dB */
	-LEVEL_M4_5DB,	/*!< bse_surmixlev = -1.5 dB, pliisurmixval = +1.5 dB */
	-LEVEL_M6DB,	/*!< bse_surmixlev = -3.0 dB, pliisurmixval =  0.0 dB */
	-LEVEL_M7_5DB,	/*!< bse_surmixlev = -4.5 dB, pliisurmixval = -1.5 dB */
	-LEVEL_M9DB,	/*!< bse_surmixlev = -6.0 dB, pliisurmixval = -3.0 dB */
	0			/*!< bse_surmixlev = -inf dB, pliisurmixval = -inf dB */
};

static const level_t globalgain[8] =
{
	-LEVEL_M6DB,		/*!< bse_cmixlev = +3.0 dB, global gain = -6.0 dB */
	-LEVEL_M4_5DB,	/*!< bse_cmixlev = +1.5 dB, global gain = -4.5 dB */
	-LEVEL_M3DB,		/*!< bse_cmixlev =  0.0 dB, global gain = -3.0 dB */
	-LEVEL_M1_5DB,	/*!< bse_cmixlev = -1.5 dB, global gain = -1.5 dB */
	-LEVEL_1,		/*!< bse_cmixlev = -3.0 dB, global gain =  0.0 dB */
	-LEVEL_1,		/*!< bse_cmixlev = -4.5 dB, global gain =  0.0 dB */
	-LEVEL_1,		/*!< bse_cmixlev = -6.0 dB, global gain =  0.0 dB */
	-LEVEL_1		/*!< bse_cmixlev =  inf dB, global gain =  0.0 dB */
};



/*!
	LFE mix levels table.

	All entries of the <i>lfemixlevtab</i> have been scaled down by 4
	to ensure that no entry exceeds 1.0.
*/
static const level_t lfemixlevtab[32] = 
{
   LEVEL(3.174802104),	/*!< +10 dB */
   LEVEL(2.828427125),	/*!< +9 dB */
   LEVEL(2.519842100),	/*!< +8 dB */
   LEVEL(2.244924097),	/*!< +7 dB */
   LEVEL(2.000000000),	/*!< +6 dB */
   LEVEL(1.781797436),	/*!< +5 dB */
   LEVEL(1.587401052),	/*!< +4 dB */
   LEVEL(1.414213562),	/*!< +3 dB */
   LEVEL(1.259921050),	/*!< +2 dB */
   LEVEL(1.122462048),	/*!< +1 dB */
   LEVEL(1.000000000),	/*!< +0 dB */
   LEVEL(0.890898718),	/*!< -1 dB */
   LEVEL(0.793700526),	/*!< -2 dB */
   LEVEL(0.707106781),	/*!< -3 dB */
   LEVEL(0.629960525),	/*!< -4 dB */
   LEVEL(0.561231024),	/*!< -5 dB */
   LEVEL(0.500000000),	/*!< -6 dB */
   LEVEL(0.445449359),	/*!< -7 dB */
   LEVEL(0.396850263),	/*!< -8 dB */
   LEVEL(0.353553391),	/*!< -9 dB */
   LEVEL(0.314980262),	/*!< -10 dB */
   LEVEL(0.280615512),	/*!< -11 dB */
   LEVEL(0.250000000),	/*!< -12 dB */
   LEVEL(0.222724680),	/*!< -13 dB */
   LEVEL(0.198425131),	/*!< -14 dB */
   LEVEL(0.176776695),	/*!< -15 dB */
   LEVEL(0.157490131),	/*!< -16 dB */
   LEVEL(0.140307756),	/*!< -17 dB */
   LEVEL(0.125000000),	/*!< -18 dB */
   LEVEL(0.111362340),	/*!< -19 dB */
   LEVEL(0.099212566),	/*!< -20 dB */
   LEVEL(0.088388348),	/*!< -21 dB */
};

/**
 * Table for default stereo downmixing coefficients
 * reference: Section 7.8.2 Downmixing Into Two Channels
 */
static const level_t ac3_default_coeffs[8][5][2] = {
	{ { LEVEL_1, LEVEL_0 }, {0, 0}, { LEVEL_0, LEVEL_1 }, },
    { {0, 0}, { LEVEL_M3DB, LEVEL_M3DB }, },
    { { LEVEL_1, LEVEL_0 }, {0, 0}, { LEVEL_0, LEVEL_1 }, },
    { { LEVEL_1, LEVEL_0 }, { LEVEL_M3DB, LEVEL_M3DB }, { LEVEL_0, LEVEL_1 }, },
    { { LEVEL_1, LEVEL_0 }, {0, 0}, { LEVEL_0, LEVEL_1 }, { -LEVEL_M3DB, LEVEL_M3DB }, },
    { { LEVEL_1, LEVEL_0 }, { LEVEL_M3DB, LEVEL_M3DB },{ LEVEL_0, LEVEL_1 }, { -LEVEL_M3DB, LEVEL_M3DB }, },
    { { LEVEL_1, LEVEL_0 }, {0, 0}, { LEVEL_0, LEVEL_1 }, { -LEVEL_M3DB, LEVEL_M3DB }, { -LEVEL_M3DB, LEVEL_M3DB }, },
	{ { LEVEL_1, LEVEL_0 }, { LEVEL_M3DB, LEVEL_M3DB },{ LEVEL_0, LEVEL_1 }, { -LEVEL_M3DB, LEVEL_M3DB }, { -LEVEL_M3DB, LEVEL_M3DB } }
};

/**
 * Set stereo downmixing coefficients based on frame header info.
 * reference: Section 7.8.2 Downmixing Into Two Channels
 */
void set_downmix_coeffs(AC3DecodeContext *s)
{
    int i,j, ch;
    int stereodmix;
    level_t ggain;
    level_t cmixval;
    level_t surmixval;
    level_t ltrtcmixval;
    level_t ltrtsurmixval;
    level_t pliisurmixval;
    level_t lorocmixval;
    level_t lorosurmixval;
    level_t lfemixval;
    level_t tmpval, tmpval1;
    int infront, inrear, outfront, outrear;
    int out_channels = ff_ac3_channels_tab[s->dev_mode] + s->dev_lfeon;

    if (s->dev_stereomode == AC3_STEREOMODE_AUTO) { /* Let bitstream choose stereomode */      
        if (s->dmixmodd) {  /* If bitstream element, dmixmod defined */           
            switch (s->dmixmod) {
                case AC3_DMIXMOD_NIND:
                    if (s->eac3) /* Use PLII downmix for DD Plus streams */                          
                       stereodmix = AC3_STEREODMIX_PLII;
                    else  /* Use LTRT downmix for DD streams */                           
                        stereodmix = AC3_STEREODMIX_LTRT;
                    break;
                case AC3_DMIXMOD_LORO:
                    /* always use LORO downmix */
                    stereodmix = AC3_STEREODMIX_LORO;
                    break;
                case AC3_DMIXMOD_PLII:
                    /* always use PLII downmix */
                    stereodmix = AC3_STEREODMIX_PLII;
                    break;
                default:
                    /* default to LTRT downmix */
                    stereodmix = AC3_STEREODMIX_LTRT;
                    break;
            }
        } else { /* p_bsi->dmixmodd */
            if (s->eac3) /* Use PLII downmix for DD Plus streams */
                stereodmix = AC3_STEREODMIX_PLII;
            else  /* Use LTRT downmix for DD streams */                   
                stereodmix = AC3_STEREODMIX_LTRT;
        }
    } else if (s->dev_stereomode == AC3_STEREOMODE_SRND) {       
        if ((s->dmixmodd && (s->dmixmod == AC3_DMIXMOD_PLII)) || s->eac3) /* Use PLII for DD Plus streams and DD streams with PLII stereo downmix indicated */
            stereodmix = AC3_STEREODMIX_PLII;
        else /* Use LTRT for DD streams with PLII stereo downmix not indicated */            
            stereodmix = AC3_STEREODMIX_LTRT;
    } else { /* p_dmxmodes->stereomode == GBL_STEREOMODE_STEREO */
        stereodmix = AC3_STEREODMIX_LORO;
    } /* outmode = 2/0 */

    /* set up downmix parameters */
    infront         = nfront[s->channel_mode];              /* number of front input channels   */
    inrear          = nrear[s->channel_mode];               /* number of rear input channels    */
    outfront        = nfront[s->dev_mode];              /* number of front input channels   */
    outrear         = nrear[s->dev_mode];               /* number of rear input channels    */
    cmixval         = cmixtab[s->legacy_cmixlev];           /* center mix coefficient           */
    surmixval       = surmixtab[s->legacy_surmixlev];       /* surround mix coefficient         */
    ltrtcmixval     = altmixtab[s->ltrt_cmixlev];           /* Lt/Rt center mix coefficient     */
    ltrtsurmixval   = altmixtab[s->ltrt_surmixlev];     /* Lt/Rt surround mix coefficient   */
    pliisurmixval   = pliisurmixtab[s->ltrt_surmixlev]; /* PLII surround mix coefficient    */
    lorocmixval     = altmixtab[s->loro_cmixlev];           /* Lo/Ro center mix coefficient     */
    lorosurmixval   = altmixtab[s->loro_surmixlev];     /* Lo/Ro surround mix coefficient   */
    if (s->dev_compmode == AC3_COMP_RF)
        lfemixval   = 0;                                                    /* LFE mix coefficient              */
    else
        lfemixval   = lfemixlevtab[s->lfemixlevcod];                /* LFE mix coefficient              */

    for(i=0; i<AC3_MAX_CHANNELS-1; i++)
        for(j=0; j<AC3_MAX_CHANNELS-1; j++)
            s->downmix_coeffs[j][j] = 0;


/*****************************************************************************/
/*                                                                           */
/*  Downmix for mono (outmode = GBL_MODE10)                                  */
/*                                                                           */
/*****************************************************************************/
    if( s->dev_mode == AC3_CHMODE_MONO) {
        if (infront == 1) {
            s->downmix_coeffs[1][1] = LEVEL_1;
        } else if (infront == 2) {
            s->downmix_coeffs[0][1] = LEVEL_M6DB;
            s->downmix_coeffs[2][1] = LEVEL_M6DB;
        } else if (infront == 3) {
            s->downmix_coeffs[0][1] = LEVEL_M6DB;
            s->downmix_coeffs[2][1] = LEVEL_M6DB;
            tmpval = MUL_Shift_26(cmixval, LEVEL_M6DB, LEVEL_SHIFT);
            s->downmix_coeffs[1][1] = tmpval;
        }

        if (inrear == 1) {
            tmpval = MUL_Shift_26(surmixval, LEVEL_M6DB, LEVEL_SHIFT);
            s->downmix_coeffs[4][1] = tmpval;
        } else if (inrear == 2) {
            tmpval = MUL_Shift_26(surmixval, LEVEL_M6DB, LEVEL_SHIFT);
            s->downmix_coeffs[3][1] = tmpval;
            s->downmix_coeffs[4][1] = tmpval;
        } if ((s->dev_lfeon == 0) && (s->lfemixlevcode)) {
            if (infront == 1) {
                s->downmix_coeffs[5][1] = lfemixval;
            } else {
                tmpval = MUL_Shift_26(lfemixval, LEVEL_M6DB, LEVEL_SHIFT);
                s->downmix_coeffs[5][1] = tmpval;
            }
        }
/*****************************************************************************/
/*                                                                           */
/*  Downmix to stereo (LORO, LTRT, PLII)                                     */
/*                                                                           */
/*****************************************************************************/
    } else if( s->dev_mode == AC3_CHMODE_STEREO || s->dev_mode == AC3_CHMODE_DUALMONO) {

        for(i=0; i<s->fbw_channels; i++) {
            ch = ff_channeltab[s->channel_mode][i];
            s->downmix_coeffs[ch][0] = ac3_default_coeffs[s->channel_mode][ch][0];
            s->downmix_coeffs[ch][2] = ac3_default_coeffs[s->channel_mode][ch][1];
        }

        if (stereodmix == AC3_STEREODMIX_LTRT) {
            ggain = globalgain[s->ltrt_cmixlev];
            if (s->lfe_on) {
                if ((s->dev_lfeon == 0) && s->lfemixlevcode) {
                    tmpval = MUL_Shift_26(lfemixval, LEVEL_M4_5DB, LEVEL_SHIFT);
                    if(infront>2)
                        tmpval = MUL_Shift_26(tmpval, -ggain, LEVEL_SHIFT);
                    s->downmix_coeffs[5][0] = tmpval;
                    s->downmix_coeffs[5][2] = tmpval;
                } else {
                    s->downmix_coeffs[5][0] = 0;
                    s->downmix_coeffs[5][2] = 0;
                }
            }

            if (s->channel_mode == AC3_CHMODE_DUALMONO) { /* Downmix for dual mono */
                if (s->dev_dualmode == AC3_DUAL_STEREO) {
                    s->downmix_coeffs[0][0] = LEVEL_1;  /* use both channels (straight out) */
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = LEVEL_1;
                } else if (s->dev_dualmode == AC3_DUAL_LEFTMONO) {
                    s->downmix_coeffs[0][0] = LEVEL_M3DB;   /* use left channel */
                    s->downmix_coeffs[0][2] = LEVEL_M3DB;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = 0;
                } else if (s->dev_dualmode == AC3_DUAL_RGHTMONO) {
                    s->downmix_coeffs[0][0] = 0;    
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = LEVEL_M3DB;  /* use right channel */
                    s->downmix_coeffs[2][2] = LEVEL_M3DB;
                } else {  // AC3_DUAL_MIXMONO
                    s->downmix_coeffs[0][0] = LEVEL_M6DB;   /* mix both channels into L and R */
                    s->downmix_coeffs[0][2] = LEVEL_M6DB;
                    s->downmix_coeffs[2][0] = LEVEL_M6DB;
                    s->downmix_coeffs[2][2] = LEVEL_M6DB;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F1R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[3][0] = (-ltrtsurmixval)<<1;
                    s->downmix_coeffs[3][2] = ltrtsurmixval<<1;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F1R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(ltrtsurmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[3][2] = -tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F2R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[3][0] = (-ltrtsurmixval)<<1;
                    s->downmix_coeffs[4][0] = (-ltrtsurmixval)<<1;
                    s->downmix_coeffs[3][2] = ltrtsurmixval<<1;
                    s->downmix_coeffs[4][2] = ltrtsurmixval<<1;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F2R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(-ltrtsurmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[3][0] = -tmpval;
                    s->downmix_coeffs[4][0] = -tmpval;
                    s->downmix_coeffs[3][2] = tmpval;
                    s->downmix_coeffs[4][2] = tmpval;
                }
            }
        } else if (stereodmix == AC3_STEREODMIX_LORO) {
            ggain = globalgain[s->loro_cmixlev];
            if (s->lfe_on) {
                if ((s->dev_lfeon == 0) && s->lfemixlevcode) {
                    tmpval = MUL_Shift_26(lfemixval, LEVEL_M4_5DB, LEVEL_SHIFT);
                    if(infront>2)
                        tmpval = MUL_Shift_26(tmpval, -ggain, LEVEL_SHIFT);
                    s->downmix_coeffs[5][0] = tmpval;
                    s->downmix_coeffs[5][2] = tmpval;
                } else {
                    s->downmix_coeffs[5][0] = 0;
                    s->downmix_coeffs[5][2] = 0;
                }
            }

            if (s->channel_mode == AC3_CHMODE_DUALMONO) { /* Downmix for dual mono */
                if (s->dev_dualmode == AC3_DUAL_STEREO) {
                    s->downmix_coeffs[0][0] = LEVEL_1;  /* use both channels (straight out) */
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = LEVEL_1;
                } else if (s->dev_dualmode == AC3_DUAL_LEFTMONO) {
                    s->downmix_coeffs[0][0] = LEVEL_M3DB;   /* use left channel */
                    s->downmix_coeffs[0][2] = LEVEL_M3DB;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = 0;
                } else if (s->dev_dualmode == AC3_DUAL_RGHTMONO) {
                    s->downmix_coeffs[0][0] = 0;    
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = LEVEL_M3DB;  /* use right channel */
                    s->downmix_coeffs[2][2] = LEVEL_M3DB;
                } else {  // AC3_DUAL_MIXMONO
                    s->downmix_coeffs[0][0] = LEVEL_M6DB;   /* mix both channels into L and R */
                    s->downmix_coeffs[0][2] = LEVEL_M6DB;
                    s->downmix_coeffs[2][0] = LEVEL_M6DB;
                    s->downmix_coeffs[2][2] = LEVEL_M6DB;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(lorocmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                } else {
                    s->downmix_coeffs[1][0] = cmixval;
                    s->downmix_coeffs[1][2] = cmixval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F1R) {
                if (s->lxrxmixlevsd) {
                    tmpval = MUL_Shift_26(lorosurmixval, LEVEL_M3DB, LEVEL_SHIFT) <<1;
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[3][2] = tmpval;
                } else {
                    tmpval = MUL_Shift_26(surmixval, LEVEL_M3DB, LEVEL_SHIFT);
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[3][2] = tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F1R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(lorocmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(LEVEL_M3DB, ggain, LEVEL_SHIFT);
                    tmpval = MUL_Shift_26(tmpval, lorosurmixval, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[3][0] = -tmpval;
                    s->downmix_coeffs[3][2] = -tmpval;
                } else {
                    s->downmix_coeffs[1][0] = cmixval;
                    s->downmix_coeffs[1][2] = cmixval;
                    tmpval = MUL_Shift_26(LEVEL_M3DB, surmixval, LEVEL_SHIFT);
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[3][2] = tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F2R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[3][0] = lorosurmixval<<1;
                    s->downmix_coeffs[4][0] = 0;
                    s->downmix_coeffs[3][2] = 0;
                    s->downmix_coeffs[4][2] = lorosurmixval<<1;
                } else {
                    s->downmix_coeffs[3][0] = surmixval;
                    s->downmix_coeffs[4][0] = 0;
                    s->downmix_coeffs[3][2] = 0;
                    s->downmix_coeffs[4][2] = surmixval;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F2R) {  // ok
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(lorocmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(lorosurmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[3][0] = -tmpval;
                    s->downmix_coeffs[4][0] = 0;
                    s->downmix_coeffs[3][2] = 0;
                    s->downmix_coeffs[4][2] = -tmpval;
                } else {
                    s->downmix_coeffs[1][0] = cmixval;
                    s->downmix_coeffs[1][2] = cmixval;
                    s->downmix_coeffs[3][0] = surmixval;
                    s->downmix_coeffs[4][0] = 0;
                    s->downmix_coeffs[3][2] = 0;
                    s->downmix_coeffs[4][2] = surmixval;
                }
            }
        } else { // (stereodmix == PL II)
            ggain = globalgain[s->ltrt_cmixlev];
            if (s->lfe_on) {
                if ((s->dev_lfeon == 0) && s->lfemixlevcode) {
                    tmpval = MUL_Shift_26(lfemixval, LEVEL_M4_5DB, LEVEL_SHIFT);
                    if(infront>2)
                        tmpval = MUL_Shift_26(tmpval, -ggain, LEVEL_SHIFT);
                    s->downmix_coeffs[5][0] = tmpval;
                    s->downmix_coeffs[5][2] = tmpval;
                } else {
                    s->downmix_coeffs[5][0] = 0;
                    s->downmix_coeffs[5][2] = 0;
                }
            }

            if (s->channel_mode == AC3_CHMODE_DUALMONO) { /* Downmix for dual mono */
                if (s->dev_dualmode == AC3_DUAL_STEREO) {
                    s->downmix_coeffs[0][0] = LEVEL_1;  /* use both channels (straight out) */
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = LEVEL_1;
                } else if (s->dev_dualmode == AC3_DUAL_LEFTMONO) {
                    s->downmix_coeffs[0][0] = LEVEL_M3DB;   /* use left channel */
                    s->downmix_coeffs[0][2] = LEVEL_M3DB;
                    s->downmix_coeffs[2][0] = 0;
                    s->downmix_coeffs[2][2] = 0;
                } else if (s->dev_dualmode == AC3_DUAL_RGHTMONO) {
                    s->downmix_coeffs[0][0] = 0;    
                    s->downmix_coeffs[0][2] = 0;
                    s->downmix_coeffs[2][0] = LEVEL_M3DB;  /* use right channel */
                    s->downmix_coeffs[2][2] = LEVEL_M3DB;
                } else {  // AC3_DUAL_MIXMONO
                    s->downmix_coeffs[0][0] = LEVEL_M6DB;   /* mix both channels into L and R */
                    s->downmix_coeffs[0][2] = LEVEL_M6DB;
                    s->downmix_coeffs[2][0] = LEVEL_M6DB;
                    s->downmix_coeffs[2][2] = LEVEL_M6DB;
                }
            }else if(s->channel_mode == AC3_CHMODE_3F) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F1R) {
                if (s->lxrxmixlevsd) {
                    tmpval = ltrtsurmixval <<1;
                    s->downmix_coeffs[3][0] = -tmpval;
                    s->downmix_coeffs[3][2] = tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F1R) {
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(ggain, ltrtsurmixval, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[3][2] = -tmpval;
                }
            } else if(s->channel_mode == AC3_CHMODE_2F2R) {
                if (s->lxrxmixlevsd) {
                    tmpval = MUL_Shift_26(pliisurmixval, LEVEL_M1_2DB, LEVEL_SHIFT)<<1;
                    tmpval1 = MUL_Shift_26(pliisurmixval, LEVEL_M6_2DB, LEVEL_SHIFT)<<1;
                    s->downmix_coeffs[3][0] = tmpval;
                    s->downmix_coeffs[4][0] = tmpval1;
                    s->downmix_coeffs[3][2] = -tmpval1;
                    s->downmix_coeffs[4][2] = -tmpval;
                } else {
                    s->downmix_coeffs[3][0] = -LEVEL_M1_2DB;
                    s->downmix_coeffs[4][0] = -LEVEL_M6_2DB;
                    s->downmix_coeffs[3][2] = LEVEL_M6_2DB;
                    s->downmix_coeffs[4][2] = LEVEL_M1_2DB;
                }
            } else if(s->channel_mode == AC3_CHMODE_3F2R) {  //ok
                if (s->lxrxmixlevsd) {
                    s->downmix_coeffs[0][0] = -ggain;
                    s->downmix_coeffs[2][2] = -ggain;
                    tmpval = MUL_Shift_26(ltrtcmixval, ggain, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    s->downmix_coeffs[1][0] = -tmpval;
                    s->downmix_coeffs[1][2] = -tmpval;
                    tmpval = MUL_Shift_26(pliisurmixval, ggain, LEVEL_SHIFT);
                    tmpval = MUL_Shift_26(tmpval, LEVEL_M1_2DB, LEVEL_SHIFT);
                    if(tmpval>0x02000000) tmpval = 0x04000000;
                    else if(tmpval<-0x02000000) tmpval = 0x04000000;
                    else tmpval<<=1;
                    tmpval1 = MUL_Shift_26(pliisurmixval, ggain, LEVEL_SHIFT);
                    tmpval1 = MUL_Shift_26(tmpval1, LEVEL_M6_2DB, LEVEL_SHIFT);
                    if(tmpval1>0x02000000) tmpval1 = 0x04000000;
                    else if(tmpval1<-0x02000000) tmpval1 = -0x04000000;
                    else tmpval1<<=1;
                    s->downmix_coeffs[3][0] = -tmpval;
                    s->downmix_coeffs[4][0] = -tmpval1;
                    s->downmix_coeffs[3][2] = tmpval1;
                    s->downmix_coeffs[4][2] = tmpval;
                } else {
                    s->downmix_coeffs[3][0] = -LEVEL_M1_2DB;
                    s->downmix_coeffs[4][0] = -LEVEL_M6_2DB;
                    s->downmix_coeffs[3][2] = LEVEL_M6_2DB;
                    s->downmix_coeffs[4][2] = LEVEL_M1_2DB;
                }
            }
        }
/*****************************************************************************/
/*                                                                           */
/*  Downmix all other cases                                                  */
/*                                                                           */
/*****************************************************************************/
    } else {
        if (outfront == 2) {
            if (infront == 1) {
                s->downmix_coeffs[1][0] = LEVEL_M3DB;
                s->downmix_coeffs[1][2] = LEVEL_M3DB;
            } else if (infront == 2) {
                s->downmix_coeffs[0][0] = LEVEL_1;
                s->downmix_coeffs[2][2] = LEVEL_1;
            } else if (infront == 3) {
                s->downmix_coeffs[0][0] = LEVEL_1;
                s->downmix_coeffs[2][2] = LEVEL_1;
                s->downmix_coeffs[1][0] = cmixval;
                s->downmix_coeffs[1][2] = cmixval;
            }
        } else if (outfront == 3) {
            if (infront == 1) {
                s->downmix_coeffs[1][1] = LEVEL_1;
            } else if (infront == 2) {
                s->downmix_coeffs[0][0] = LEVEL_1;
                s->downmix_coeffs[2][2] = LEVEL_1;
            } else if (infront == 3) {
                s->downmix_coeffs[0][0] = LEVEL_1;
                s->downmix_coeffs[2][2] = LEVEL_1;
                s->downmix_coeffs[1][1] = LEVEL_1;
            }
        }

        if (outrear == 0) {
            if (inrear == 1) {
                tmpval = MUL_Shift_26(surmixval, LEVEL_M3DB, LEVEL_SHIFT);
                s->downmix_coeffs[3][0] = tmpval;
                s->downmix_coeffs[3][2] = tmpval;
            } else if (inrear == 2) {
                s->downmix_coeffs[3][0] = surmixval;
                s->downmix_coeffs[4][2] = surmixval;
            }
        } else if (outrear == 1) {
            if (inrear == 1) {
                s->downmix_coeffs[3][3] = LEVEL_1;
            } else if (inrear == 2) {
                s->downmix_coeffs[3][3] = surmixval;
                s->downmix_coeffs[4][3] = surmixval;
            }
        } else if (outrear == 2) {
            if (inrear == 1) {
                s->downmix_coeffs[3][3] = LEVEL_M3DB;
                s->downmix_coeffs[3][4] = LEVEL_M3DB;
            } else if (inrear == 2) {
                s->downmix_coeffs[3][3] = LEVEL_1;
                s->downmix_coeffs[4][4] = LEVEL_1;
            }
        }

        if (s->lfe_on && s->dev_lfeon)
            s->downmix_coeffs[5][5] = LEVEL_1;
    }


    s->dnmix_active = 0;
    for (i = 0; i < out_channels; i++)
    {
        int rowsum = 0;
        int out_ch = ff_channeltab[s->dev_mode][i];
        for (j = 0; j < s->channels; j++) 
        {
            int in_ch;
            in_ch = ff_channeltab[s->channel_mode][j];
            rowsum += abs(s->downmix_coeffs[in_ch][out_ch]);
        }
        if (rowsum > LEVEL_1)
        {
            s->dnmix_active = 1;
            break;
        }
        else if (s->lfemixlevcode)
        {
            /* set dnmix_active if LFE downmixing is in use */
            s->dnmix_active = 1;
            break;
        }
    }
}

/**
 * Downmix the output to mono or stereo.
 */
void ac3_downmix(AC3DecodeContext *s, int ch)						
{
    int i, j, k, v;
    int out_channels;
    level_t fact, last_fact;

    out_channels = ff_ac3_channels_tab[s->dev_mode] + s->dev_lfeon;
    for(j=0;j<out_channels;j++) {
        k = ff_channeltab[s->dev_mode][j];
        fact = s->downmix_coeffs[ch][k];
        last_fact = s->last_downmix[ch][k];
        if((fact==0) && (last_fact==0))
            continue;
        for(i=0; i<256; i++) {
            v = 0;
            if(fact==last_fact) {
                v += MUL_Shift_27(s->work_buf[ch][i], fact, LEVEL_SHIFT+1);
            }    
            else {
                level_t factor, factor_in, factor_out;
                int fadin = ac3_imdct_window[i];
                int fadout = ac3_imdct_window[255-i];
                int tmp; 
                tmp = MUL_Shift_29(fadout, fadout, 29);
                factor_out = MUL_Shift_29(last_fact, tmp, 29);
                tmp = MUL_Shift_29(fadin, fadin, 29);
                factor_in = MUL_Shift_29(fact, tmp, 29);
                factor = factor_out + factor_in;
                v += MUL_Shift_27(s->work_buf[ch][i], factor, LEVEL_SHIFT+1);  
            }
            if(s->dnmixbuf_used[k])
                s->dnmixed_buf[k][i] += v;
            else
                s->dnmixed_buf[k][i] = v;
        }
        s->dnmixbuf_used[k] = 1;
    }            
}

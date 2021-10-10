/*
 * Common code between the AC-3 and E-AC-3 decoders
 * Copyright (c) 2007 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Common code between the AC-3 and E-AC-3 decoders.
 *
 * Summary of MDCT Coefficient Grouping:
 * The individual MDCT coefficient indices are often referred to in the
 * (E-)AC-3 specification as frequency bins.  These bins are grouped together
 * into subbands of 12 coefficients each.  The subbands are grouped together
 * into bands as defined in the bitstream by the band structures, which
 * determine the number of bands and the size of each band.  The full spectrum
 * of 256 frequency bins is divided into 1 DC bin + 21 subbands = 253 bins.
 * This system of grouping coefficients is used for channel bandwidth, stereo
 * rematrixing, channel coupling, enhanced coupling, and spectral extension.
 *
 * +-+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+-+
 * |1|  |12|  |  [12|12|12|12]  |  |  |  |  |  |  |  |  |  |  |  |  |3|
 * +-+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+-+
 * ~~~  ~~~~     ~~~~~~~~~~~~~                                      ~~~
 *  |     |            |                                             |
 *  |     |            |                    3 unused frequency bins--+
 *  |     |            |
 *  |     |            +--1 band containing 4 subbands
 *  |     |
 *  |     +--1 subband of 12 frequency bins
 *  |
 *  +--DC frequency bin
 */

#ifndef AVCODEC_AC3DEC_H
#define AVCODEC_AC3DEC_H

#include "ac3.h"
#include "get_bits.h"

/* define eace decoded */
#define CONFIG_EAC3_DECODER

/* override ac3.h to include coupling channel */
#undef AC3_MAX_CHANNELS
#define AC3_MAX_CHANNELS 7
#define CPL_CH 0

#define AC3_OUTPUT_LFEON  8

#define SPX_MAX_BANDS    17

typedef struct AC3BitAllocParameters {
    int sr_code;
    int sr_shift;
    int slow_gain, slow_decay, fast_decay, db_per_bit, floor;
    int cpl_fast_leak, cpl_slow_leak;
} AC3BitAllocParameters;

typedef struct {
    int b1_mant[2];
    int b2_mant[2];
    int b4_mant;
    int b1;
    int b2;
    int b4;
} mant_groups;

typedef struct
{
    int     opnflag;       /*!< Overwrite Pre-Noise flag */
    int     tranloc;       /*!< Index of transient location (<i>4*transprocloc</i>) */
    int     tranlen;       /*!< Length of transient processing (<i>transproclen</i>) */
    int     cblknum;       /*!< Current block number in processing */
    int     tranprocflag;  /*!< Currently performing transient processing */
    int     lasttranloc;   /*!< Length to last transient */
    int     *synthwrite;
    int     *synthread;
} TPND_INFO;

/**
 * @struct AC3HeaderInfo
 * Coded AC-3 header values up to the lfeon element, plus derived values.
 */
typedef struct {
    /** @defgroup coded Coded elements
     * @{
     */
    uint16_t sync_word;
    uint16_t crc1;
    uint8_t sr_code;
    uint8_t bitstream_id;
    uint8_t bitstream_mode;
    uint8_t channel_mode;
    uint8_t lfe_on;
    uint8_t frame_type;
    int substreamid;                        ///< substream identification
    int legacy_cmixlev;                   ///< Center mix level index
    int legacy_surmixlev;                 ///< Surround mix level index
    uint16_t channel_map;
    int num_blocks;                         ///< number of audio blocks
    /** @} */

    /** @defgroup derived Derived values
     * @{
     */
    uint8_t sr_shift;
    uint16_t sample_rate;
    uint32_t bit_rate;
    uint8_t channels;
    uint16_t frame_size;
    //int64_t channel_layout;
    /** @} */
} AC3HeaderInfo;

typedef struct {
    //AVCodecContext *avctx;                  ///< parent context
    GetBitContext gbc;                      ///< bitstream reader
    //uint8_t *input_buffer;                  ///< temp buffer to prevent overread

///@defgroup bsi bit stream information
///@{
    int bitstream_id;
    int frame_type;                         ///< frame type                             (strmtyp)
    int substreamid;                        ///< substream identification
    int frame_size;                         ///< current frame size, in bytes
    int bit_rate;                           ///< stream bit rate, in bits-per-second
    int sample_rate;                        ///< sample frequency, in Hz
    int num_blocks;                         ///< number of audio blocks
    int bitstream_mode;                     ///< bitstream mode                         (bsmod)
    int channel_mode;                       ///< channel mode                           (acmod)
    int last_channel_mode;                  ///< last channel mode                           (acmod)
    //int channel_layout;                     ///< channel layout
    int lfe_on;                             ///< lfe channel in use
    int last_lfe_on;                        ///< last lfe channel in use
    int channel_map;                        ///< custom channel map
    int legacy_cmixlev;                   ///< Center mix level index
    int legacy_surmixlev;                 ///< Surround mix level index
    int ltrt_cmixlev;                   ///< Center mix level index
    int ltrt_surmixlev;                 ///< Surround mix level index
    int loro_cmixlev;                   ///< Center mix level index
    int loro_surmixlev;                 ///< Surround mix level index
    int lxrxmixlevsd;
    int dmixmodd;
    int dmixmod;
    int lfemixlevcode;
    int lfemixlevcod;

    int dialnorm[2];                        ///< Dialog normalization
    int compre[2];                      ///< compress flag
    int compr[2];                       ///< compress value
    int eac3;                               ///< indicates if current frame is E-AC-3
///@}

///@defgroup audfrm frame syntax parameters
    int snr_offset_strategy;                ///< SNR offset strategy                    (snroffststr)
    int block_switch_syntax;                ///< block switch syntax enabled            (blkswe)
    int dither_flag_syntax;                 ///< dither flag syntax enabled             (dithflage)
    int bit_allocation_syntax;              ///< bit allocation model syntax enabled    (bamode)
    int fast_gain_syntax;                   ///< fast gain codes enabled                (frmfgaincode)
    int dba_syntax;                         ///< delta bit allocation syntax enabled    (dbaflde)
    int skip_syntax;                        ///< skip field syntax enabled              (skipflde)
 ///@}

///@defgroup cpl standard coupling
    int cpl_in_use[AC3_MAX_BLOCKS];         ///< coupling in use                        (cplinu)
    int cpl_strategy_exists[AC3_MAX_BLOCKS];///< coupling strategy exists               (cplstre)
    int channel_in_cpl[AC3_MAX_CHANNELS];   ///< channel in coupling                    (chincpl)
    int phase_flags_in_use;                 ///< phase flags in use                     (phsflginu)
    int phase_flags;                        ///< phase flags                            (phsflg)
    int num_cpl_bands;                      ///< number of coupling bands               (ncplbnd)
    int cpl_start_band;
    uint8_t cpl_band_sizes[AC3_MAX_CPL_BANDS]; ///< number of coeffs in each coupling band
    uint8_t cpl_band_struct[AC3_MAX_CPL_BANDS];
    int first_cpl_coords[AC3_MAX_CHANNELS]; ///< first coupling coordinates states      (firstcplcos)
    int cplco_exp[AC3_MAX_CHANNELS][AC3_MAX_CPL_BANDS]; ///< coupling coordinates      (cplco exp)
    int cplco_mant[AC3_MAX_CHANNELS][AC3_MAX_CPL_BANDS]; ///< coupling coordinates      (cplco mant)
///@}

///@enhanced coupling
    int ecplinu;                    /*!< Enhanced coupling in use flag */
    int necplbnd;                   /*!< Number of enhanced coupling bands - based on banding structure */
    int ecplbndstrc[AC3_ECPD_MAXNUMECPBNDS];
    int ecpbndbinoffst[AC3_ECPD_MAXNUMECPBNDS + 1];         /*!< Array containing start bin of each ecp band */
    int ecplamp[AC3_MAX_CHANNELS][AC3_ECPD_MAXNUMECPBNDS];  /*!< Channel enhanced coupling amplitudes */

///@}


///@defgroup spx spectral extension
///@{
    int spx_in_use;                             ///< spectral extension in use              (spxinu)
    uint8_t channel_uses_spx[AC3_MAX_CHANNELS]; ///< channel uses spectral extension        (chinspx)
    int8_t spx_atten_code[AC3_MAX_CHANNELS];    ///< spx attenuation code                   (spxattencod)
    int spx_begf;
    int spx_endf;
    int spx_beg_subbnd;                     ///< spx start subbnd bin
    int spx_end_subbnd;                       ///< spx end subbnd bin
    int spx_start_subbnd;                     ///< spx starting subbnd bin for copying (copystartmant)
                                                ///< the copy region ends at the start of the spx region.
    int num_spx_bands;                          ///< number of spx bands                    (nspxbnds)
    uint8_t spx_band_struct[SPX_MAX_BANDS];
    uint8_t spx_band_sizes[SPX_MAX_BANDS];      ///< number of bins in each spx band
    uint8_t first_spx_coords[AC3_MAX_CHANNELS]; ///< first spx coordinates states           (firstspxcos)
    fract_t spx_coord_nblend[AC3_MAX_CHANNELS][SPX_MAX_BANDS]; 
    fract_t spx_coord_sblend[AC3_MAX_CHANNELS][SPX_MAX_BANDS];
///@}

///@defgroup aht adaptive hybrid transform
    int channel_uses_aht[AC3_MAX_CHANNELS];                         ///< channel AHT in use (chahtinu)
    int pre_mantissa[AC3_MAX_CHANNELS][AC3_MAX_COEFS][AC3_MAX_BLOCKS];  ///< pre-IDCT mantissas
///@}

/// transient pre-noise process
    int transient_proc;
    int transproc[AC3_MAX_CHANNELS];
    int transprocloc[AC3_MAX_CHANNELS];
    int transproclen[AC3_MAX_CHANNELS];
    TPND_INFO tpndinfo[AC3_MAX_CHANNELS];

///@defgroup channel channel
    int fbw_channels;                           ///< number of full-bandwidth channels
    int channels;                               ///< number of total channels
    int lfe_ch;                                 ///< index of LFE channel
    level_t downmix_coeffs[AC3_MAX_CHANNELS-1][AC3_MAX_CHANNELS-1];  ///< stereo downmix coefficients
    level_t last_downmix[AC3_MAX_CHANNELS-1][AC3_MAX_CHANNELS-1];  ///< last stereo downmix coefficients
    int downmixed;                              ///< indicates if coeffs are currently downmixed
    int dev_channels;
    int dev_mode;
    int dev_lfeon;
    int dev_compmode;
    int dev_stereomode;
    int dev_dualmode;
    int dev_dynscalelow;
    int dev_dynscalehigh;
    level_t dev_gain;

// downmix
    int dnmix_active;
    int dnmixbuf_used[AC3_MAX_CHANNELS-1];   // downmix output buffer have value or not

///@}

///@defgroup dynrng dynamic range
    //float dynamic_range[2];                 ///< dynamic range
    level_t dynamic_range[2]; 
///@}

///@defgroup bandwidth bandwidth
    int start_freq[AC3_MAX_CHANNELS];       ///< start frequency bin                    (strtmant)
    int end_freq[AC3_MAX_CHANNELS];         ///< end frequency bin                      (endmant)
    int babnd_start[AC3_MAX_CHANNELS];
///@}

///@defgroup rematrixing rematrixing
    int num_rematrixing_bands;              ///< number of rematrixing bands            (nrematbnd)
    int rematrixing_flags[4];               ///< rematrixing flags                      (rematflg)
///@}

///@defgroup exponents exponents
    int num_exp_groups[AC3_MAX_CHANNELS];           ///< Number of exponent groups      (nexpgrp)
    int8_t dexps[AC3_MAX_CHANNELS][AC3_MAX_COEFS];  ///< decoded exponents
    int exp_strategy[AC3_MAX_BLOCKS][AC3_MAX_CHANNELS]; ///< exponent strategies        (expstr)
///@}

///@defgroup bitalloc bit allocation
    AC3BitAllocParameters bit_alloc_params;         ///< bit allocation parameters
    int first_cpl_leak;                             ///< first coupling leak state      (firstcplleak)
    int snr_offset[AC3_MAX_CHANNELS];               ///< signal-to-noise ratio offsets  (snroffst)
    int fast_gain[AC3_MAX_CHANNELS];                ///< fast gain values/SMR's         (fgain)
    uint8_t bap[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< bit allocation pointers
    int16_t psd[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< scaled exponents
    int16_t band_psd[AC3_MAX_CHANNELS][AC3_CRITICAL_BANDS]; ///< interpolated exponents
    int16_t mask[AC3_MAX_CHANNELS][AC3_CRITICAL_BANDS];     ///< masking curve values
    int dba_mode[AC3_MAX_CHANNELS];                 ///< delta bit allocation mode
    int dba_nsegs[AC3_MAX_CHANNELS];                ///< number of delta segments
    uint8_t dba_offsets[AC3_MAX_CHANNELS][8];       ///< delta segment offsets
    uint8_t dba_lengths[AC3_MAX_CHANNELS][8];       ///< delta segment lengths
    uint8_t dba_values[AC3_MAX_CHANNELS][8];        ///< delta values for each segment
    uint8_t bit_alloc_stages[AC3_MAX_CHANNELS];

///@}

///@defgroup dithering zero-mantissa dithering
    int dither_flag[AC3_MAX_CHANNELS];      ///< dither flags                           (dithflg)
    int16_t dith_state;                       ///< for dither generation
///@}

///@defgroup imdct IMDCT
    int block_switch[AC3_MAX_CHANNELS];     ///< block switch flags                     (blksw)

///Mixing Metadata
    int pgmscle[2];
    int pgmscl[2];
    int extpgmscle;                 /*!< External program scale factor exists flags */
    int extpgmscl;                  /*!< External program scale factor exists */
    int mixdef;
    int premixcmpsel;
    int drcsrc;
    int premixcmpscl;
    int mixdata2e;
    int extpgmlscle;        /*!< External program left scale factor exists */
    int extpgmcscle;        /*!< External program center scale factor exists */
    int extpgmrscle;        /*!< External program right scale factor exists */
    int extpgmlsscle;       /*!< External program left surround scale factor exists */
    int extpgmrsscle;       /*!< External program right surround scale factor exists */
    int extpgmlfescle;      /*!< External program LFE scale factor exists */
    int extpgmlscl;         /*!< External program left scale factor */
    int extpgmcscl;         /*!< External program center scale factor */
    int extpgmrscl;         /*!< External program right scale factor */
    int extpgmlsscl;        /*!< External program left surround scale factor */
    int extpgmrsscl;        /*!< External program right surround scale factor */
    int extpgmlfescl;       /*!< External program LFE scale factor */
    int dmixscle;           /*!< Downmix scale factor exists */
    int dmixscl;            /*!< Downmix scale factor */
    int addche;             /*!< Additional scale factors exist */
    int extpgmaux1scle;     /*!< External program 1st auxiliary channel scale factor exists */
    int extpgmaux1scl;      /*!< External program 1st auxiliary channel scale factor */
    int extpgmaux2scle;     /*!< External program 2nd auxiliary channel scale factor exists */
    int extpgmaux2scl;      /*!< External program 2nd auxiliary channel scale factor */
    int frmmixcfginfoe;                 /*!< Frame mixing configuration information exists flag */
    int blkmixcfginfoe;                 /*!< Block mixing configuration information exists flag */
    int blkmixcfginfo[6];   /*!< Block mixing configuration information */
    int paninfoe[2];        /*!< Pan information exists flag */
    int panmean[2];         /*!< Pan mean angle data */
    int paninfo[2];         /*!< Pan information */

    short x;
    short y;
    int outmode; /* output channel configuration     */
///@defgroup arrays aligned arrays
    int32_t fixed_coeffs[AC3_MAX_CHANNELS][AC3_MAX_COEFS];       ///< fixed-point transform coefficients
    int32_t delay[AC3_MAX_CHANNELS-1][AC3_BLOCK_SIZE];             ///< delay - added to the next block
    int32_t work_buf[AC3_MAX_CHANNELS-1][AC3_BLOCK_SIZE];            ///< output after imdct
    int32_t dnmixed_buf[AC3_MAX_CHANNELS-1][AC3_BLOCK_SIZE];            ///< output after downmixed
    int32_t tpndsynthbuf[AC3_MAX_CHANNELS-1][AC3_SYNTHBUFLEN];  /*!< Array of pointers to TPND buffers */

///@}
} AC3DecodeContext;

int ac3_syncinfo(uint8_t *buf);
int parse_frame_header(AC3DecodeContext *s);

int spx_unp(AC3DecodeContext *s, int blk);
int cpl_unp(AC3DecodeContext *s, int blk);
int bitalloc_unp(AC3DecodeContext *s, int blk);
void ac3_mant_ch_unp(AC3DecodeContext *s, int ch_index, mant_groups *m);
void aht_mant_ch_unp(AC3DecodeContext *s, int ch);
int decode_audio_block(AC3DecodeContext *s, int blk);


void cpld_decouple(AC3DecodeContext *s);
void ecpd_decpamponly(AC3DecodeContext *s);
void spx_synthesizetcs(AC3DecodeContext *s, int ch);
int eac3_tpnp_decode(AC3DecodeContext *s, int  blk, int  ch);
void set_downmix_coeffs(AC3DecodeContext *s);
void ac3_downmix(AC3DecodeContext *s, int ch);

int ac3_decode_init(AC3DecodeContext *s);
int ac3_decode_end(AC3DecodeContext *s);
int ac3_decode_frame(AC3DecodeContext * s, void *in_buf, int *byte_left, void *out_buf, int *out_size);
int ac3_syncinfo(uint8_t *buf);


#endif /* AVCODEC_AC3DEC_H */

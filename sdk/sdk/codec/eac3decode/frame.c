/*!
*  \file frame.c
*
* \brief frame module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"

#define EAC3_SR_CODE_REDUCED  3

/* possible frequencies */
const uint16_t ff_ac3_sample_rate_tab[3] = { 48000, 44100, 32000 };

/* possible bitrates */
const uint16_t ff_ac3_bitrate_tab[19] = {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
};

const int ac3_lxrx2cmix_tab[8] =
{
	0,	/*!< +3.0 dB -> -3.0 dB */
	0,	/*!< +1.5 dB -> -3.0 dB */
	0,	/*!<  0   dB -> -3.0 dB */
	0,	/*!< -1.5 dB -> -3.0 dB */
	0,	/*!< -3.0 dB -> -3.0 dB */
	1,	/*!< -4.5 dB -> -4.5 dB */
	2,	/*!< -6.0 dB -> -6.0 dB */
	2		/*!< -oo  dB -> -6.0 dB */
};

const int ac3_lxrx2surmix_tab[8] = {
	0,	/*!< +3.0 dB -> -3.0 dB */
	0,	/*!< +1.5 dB -> -3.0 dB */
	0,	/*!<  0   dB -> -3.0 dB */
	0,	/*!< -1.5 dB -> -3.0 dB */
	0,	/*!< -3.0 dB -> -3.0 dB */
	1,	/*!< -4.5 dB -> -6.0 dB */
	1,	/*!< -6.0 dB -> -6.0 dB */
	2	/*!< -oo  dB -> -inf dB */
};

/**
 * Table E2.14 Frame Exponent Strategy Combinations
 */
const uint8_t ff_eac3_frm_expstr[32][6] = {
{    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE},
{    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,    EXP_D45},
{    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,    EXP_D25,  EXP_REUSE},
{    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,    EXP_D45,    EXP_D45},
{    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D25,  EXP_REUSE,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D25,  EXP_REUSE,    EXP_D45},
{    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D45,    EXP_D25,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D45,    EXP_D45,    EXP_D45},
{    EXP_D25,  EXP_REUSE,    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D45},
{    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D45},
{    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D25,  EXP_REUSE,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D45},
{    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE},
{    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D45},
{    EXP_D45,    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE},
{    EXP_D45,    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE,    EXP_D45},
{    EXP_D45,    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D25,  EXP_REUSE},
{    EXP_D45,    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D45,    EXP_D45},
{    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE,  EXP_REUSE},
{    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE,    EXP_D45},
{    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D25,  EXP_REUSE},
{    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D45,    EXP_D45},
{    EXP_D45,    EXP_D45,    EXP_D15,  EXP_REUSE,  EXP_REUSE,  EXP_REUSE},
{    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE,  EXP_REUSE,    EXP_D45},
{    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D25,  EXP_REUSE},
{    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D45,    EXP_D45},
{    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE,  EXP_REUSE},
{    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE,    EXP_D45},
{    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D25,  EXP_REUSE},
{    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D45,    EXP_D45},
};

/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 */
const uint16_t ff_ac3_frame_size_tab[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
};

static int eac3_parse_header(AC3DecodeContext *s)
{
    int i, blk, ch;
    int ac3_exponent_strategy, parse_aht_info, parse_spx_atten_data;
    int num_cpl_blocks;
    GetBitContext *gbc = &s->gbc;

    /* An E-AC-3 stream can have multiple independent streams which the
       application can select from. each independent stream can also contain
       dependent streams which are used to add or replace channels. */
    if (s->frame_type == EAC3_FRAME_TYPE_DEPENDENT) {
        PRINTF("Dependent substream decoding\n");
        return AC3_PARSE_ERROR_FRAME_TYPE;
    } else if (s->frame_type == EAC3_FRAME_TYPE_RESERVED) {
        PRINTF("Reserved frame type\n");
        return AC3_PARSE_ERROR_FRAME_TYPE;
    }

    /* The substream id indicates which substream this frame belongs to. each
       independent stream has its own substream id, and the dependent streams
       associated to an independent stream have matching substream id's. */
    if (s->substreamid) {
        /* only decode substream with id=0. skip any additional substreams. */
        PRINTF("Additional substreams\n");
        return AC3_PARSE_ERROR_FRAME_TYPE;
    }

    if (s->bit_alloc_params.sr_code == EAC3_SR_CODE_REDUCED) {
        /* The E-AC-3 specification does not tell how to handle reduced sample
           rates in bit allocation.  The best assumption would be that it is
           handled like AC-3 DolbyNet, but we cannot be sure until we have a
           sample which utilizes this feature. */
        PRINTF("Reduced sampling rates");
        return -1;
    }
    skip_bits(gbc, 5); // skip bitstream id

    /* volume control params */
    for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
        s->dialnorm[i] = get_bits(gbc, 5); // dialog normalization
		s->compre[i] = get_bits1(gbc);
        if (s->compre[i])
            s->compr[i] = get_sbits(gbc, 8); // compression
    }

    /* dependent stream channel map */
    if (s->frame_type == EAC3_FRAME_TYPE_DEPENDENT) {
        if (get_bits1(gbc)) {
            skip_bits(gbc, 16); // skip custom channel map
        }
    }

    if (get_bits1(gbc)) {
        /* center and surround mix levels */
        if (s->channel_mode > AC3_CHMODE_STEREO) {
            s->dmixmod = get_bits(gbc, 2);  // stereo downmix mode
            s->dmixmodd = 1;
            if (s->channel_mode & 1) {
                /* if three front channels exist */
				s->ltrt_cmixlev = get_bits(gbc, 3); // Lt/Rt center mix level
                s->loro_cmixlev = get_bits(gbc, 3);
                s->lxrxmixlevsd = 1;
                /* Derive legacy cmixlev from loro cmixlev */
                s->legacy_cmixlev = ac3_lxrx2cmix_tab[s->loro_cmixlev];
            }
            if (s->channel_mode & 4) {
                /* if a surround channel exists */
                s->ltrt_surmixlev = get_bits(gbc, 3); // Lt/Rt surround mix level
                s->loro_surmixlev = get_bits(gbc, 3);
                s->lxrxmixlevsd = 1;
				if (s->ltrt_surmixlev < 3)
					s->ltrt_surmixlev = 3;
				if (s->loro_surmixlev < 3)
					s->loro_surmixlev = 3;
                s->legacy_surmixlev = ac3_lxrx2surmix_tab[s->loro_surmixlev];
            }
        }

        /* lfe mix level */
		if (s->lfe_on) {
		    s->lfemixlevcode = get_bits1(gbc);
			if(s->lfemixlevcode)
                s->lfemixlevcod = get_bits(gbc, 5); //LFE mix level code
        }

        /* info for mixing with other streams and substreams */
        if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT) {
            for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
                // TODO: apply program scale factor
				s->pgmscle[i] = get_bits1(gbc);
                if (s->pgmscle[i])
                    s->pgmscl[i] = get_bits(gbc, 6);  // program scale factor
                else
                    s->pgmscl[i] = 51;
            }
            s->extpgmscle = get_bits1(gbc);
            if (s->extpgmscle)
                s->extpgmscl = get_bits(gbc, 6);  //external program scale factor
            else
                s->extpgmscl = 51;

			s->mixdef = get_bits(gbc, 2);
            switch(s->mixdef) {
                case 1: 
					s->premixcmpsel = get_bits1(gbc);
					s->drcsrc = get_bits1(gbc);
					s->premixcmpscl = get_bits(gbc, 3);
					break;
                case 2: 
					skip_bits(gbc, 12); break;
                case 3: {
                    int mix_data_size = (get_bits(gbc, 5) + 2)<<3;

					s->mixdata2e = get_bits1(gbc);
                    mix_data_size --;
                    if (s->mixdata2e) {
                        s->premixcmpsel = get_bits1(gbc);
                        s->drcsrc = get_bits1(gbc);
                        s->premixcmpscl = get_bits(gbc, 3);
                        s->extpgmlscle = get_bits1(gbc);
                        mix_data_size -= 6;
                        if (s->extpgmlscle) {
                            s->extpgmlscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->extpgmcscle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->extpgmcscle) {
                            s->extpgmcscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->extpgmrscle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->extpgmrscle) {
                            s->extpgmrscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->extpgmlsscle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->extpgmlsscle){
                            s->extpgmlsscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->extpgmrsscle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->extpgmrsscle) {
                            s->extpgmrsscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->extpgmlfescle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->extpgmlfescle){
                            s->extpgmlfescl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->dmixscle = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->dmixscle) {
                            s->dmixscl = get_bits(gbc, 4);
                            mix_data_size -= 4;
                        }

                        s->addche = get_bits1(gbc);
                        mix_data_size -= 1;
                        if (s->addche) {
                            s->extpgmaux1scle = get_bits1(gbc);
                            mix_data_size -= 1;
                            if (s->extpgmaux1scle) {
                                s->extpgmaux1scl = get_bits(gbc, 4);
                                mix_data_size -= 4;
                            }

                            s->extpgmaux2scle = get_bits1(gbc);
                            mix_data_size -= 1;
                            if (s->extpgmaux2scle) {
                                s->extpgmaux2scl = get_bits(gbc, 4);
                                mix_data_size -= 4;
                            }
                        }
                    }
                    skip_bits_long(gbc, mix_data_size);
                    break;
                }
            }
            /* pan information for mono or dual mono source */
            if (s->channel_mode < AC3_CHMODE_STEREO) {
                for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
					s->paninfoe[i] = get_bits1(gbc);
                    if (s->paninfoe[i]) {
                        s->panmean[i] = get_bits(gbc, 8);  // pan mean direction index
                        s->paninfo[i] = get_bits(gbc, 6);  // reserved paninfo bits
                    }
                }
            }
            /* mixing configuration information */
			s->frmmixcfginfoe = get_bits1(gbc);
            if (s->frmmixcfginfoe) {
                if (s->num_blocks == 1)
					    s->blkmixcfginfo[0] = get_bits(gbc, 5);
				else {
                    for (blk = 0; blk < s->num_blocks; blk++) {
					    if( get_bits1(gbc))
					        s->blkmixcfginfo[i] = get_bits(gbc, 5);
                    }
                }
            }
        }
    }

    /* informational metadata */
    if (get_bits1(gbc)) {
        s->bitstream_mode = get_bits(gbc, 3);
        skip_bits(gbc, 2); // skip copyright bit and original bitstream bit
        if (s->channel_mode == AC3_CHMODE_STEREO) {
            skip_bits(gbc, 4); // skip Dolby surround and headphone mode
        }
        if (s->channel_mode >= AC3_CHMODE_2F2R) {
            skip_bits(gbc, 2); // skip Dolby surround EX mode
        }
        for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
            if (get_bits1(gbc)) {
                skip_bits(gbc, 8); // skip mix level, room type, and A/D converter type
            }
        }
        if (s->bit_alloc_params.sr_code != EAC3_SR_CODE_REDUCED) {
            skip_bits(gbc, 1); // skip source sample rate code
        }
    }

    /* converter synchronization flag
       If frames are less than six blocks, this bit should be turned on
       once every 6 blocks to indicate the start of a frame set.
       reference: RFC 4598, Section 2.1.3  Frame Sets */
    if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT && s->num_blocks != 6) {
        skip_bits(gbc, 1); // skip converter synchronization flag
    }

    /* original frame size code if this stream was converted from AC-3 */
    if (s->frame_type == EAC3_FRAME_TYPE_AC3_CONVERT &&
            (s->num_blocks == 6 || get_bits1(gbc))) {
        skip_bits(gbc, 6); // skip frame size code
    }

    /* additional bitstream info */
    if (get_bits1(gbc)) {
        int addbsil = get_bits(gbc, 6);
        for (i = 0; i < addbsil + 1; i++) {
            skip_bits(gbc, 8); // skip additional bit stream info
        }
    }

    /* audio frame syntax flags, strategy data, and per-frame data */

    if (s->num_blocks == 6) {
        ac3_exponent_strategy = get_bits1(gbc);
        parse_aht_info        = get_bits1(gbc);
    } else {
        /* less than 6 blocks, so use AC-3-style exponent strategy syntax, and
           do not use AHT */
        ac3_exponent_strategy = 1;
        parse_aht_info = 0;
    }

    s->snr_offset_strategy    = get_bits(gbc, 2);
    s->transient_proc = get_bits1(gbc);

    s->block_switch_syntax = get_bits1(gbc);
    if (!s->block_switch_syntax)
        memset(s->block_switch, 0, sizeof(s->block_switch));

    s->dither_flag_syntax = get_bits1(gbc);
    if (!s->dither_flag_syntax) {
        for (ch = 1; ch <= s->fbw_channels; ch++)
            s->dither_flag[ch] = 1;
    }
    s->dither_flag[CPL_CH] = s->dither_flag[s->lfe_ch] = 0;

    s->bit_allocation_syntax = get_bits1(gbc);
    if (!s->bit_allocation_syntax) {
        /* set default bit allocation parameters */
        s->bit_alloc_params.slow_decay = ff_ac3_slow_decay_tab[2];
        s->bit_alloc_params.fast_decay = ff_ac3_fast_decay_tab[1];
        s->bit_alloc_params.slow_gain  = ff_ac3_slow_gain_tab [1];
        s->bit_alloc_params.db_per_bit = ff_ac3_db_per_bit_tab[2];
        s->bit_alloc_params.floor      = ff_ac3_floor_tab     [7];
    }

    s->fast_gain_syntax  = get_bits1(gbc);
    s->dba_syntax        = get_bits1(gbc);
    s->skip_syntax       = get_bits1(gbc);
    parse_spx_atten_data = get_bits1(gbc);

    /* coupling strategy occurance and coupling use per block */
    num_cpl_blocks = 0;
    if (s->channel_mode > 1) {
        for (blk = 0; blk < s->num_blocks; blk++) {
            s->cpl_strategy_exists[blk] = (!blk || get_bits1(gbc));
            if (s->cpl_strategy_exists[blk]) {
                s->cpl_in_use[blk] = get_bits1(gbc);
            } else {
                s->cpl_in_use[blk] = s->cpl_in_use[blk-1];
            }
            num_cpl_blocks += s->cpl_in_use[blk];
        }
    } else {
        memset(s->cpl_in_use, 0, sizeof(s->cpl_in_use));
    }

    /* exponent strategy data */
    if (ac3_exponent_strategy) {
        /* AC-3-style exponent strategy syntax */
        for (blk = 0; blk < s->num_blocks; blk++) {
            for (ch = !s->cpl_in_use[blk]; ch <= s->fbw_channels; ch++) {
                s->exp_strategy[blk][ch] = get_bits(gbc, 2);
            }
        }
    } else {
        /* LUT-based exponent strategy syntax */
        for (ch = !((s->channel_mode > 1) && num_cpl_blocks); ch <= s->fbw_channels; ch++) {
            int frmchexpstr = get_bits(gbc, 5);
            for (blk = 0; blk < 6; blk++) {
                s->exp_strategy[blk][ch] = ff_eac3_frm_expstr[frmchexpstr][blk];
            }
        }
    }
    /* LFE exponent strategy */
    if (s->lfe_on) {
        for (blk = 0; blk < s->num_blocks; blk++) {
            s->exp_strategy[blk][s->lfe_ch] = get_bits1(gbc);
        }
    }
    /* original exponent strategies if this stream was converted from AC-3 */
    if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT &&
            (s->num_blocks == 6 || get_bits1(gbc))) {
        skip_bits(gbc, 5 * s->fbw_channels); // skip converter channel exponent strategy
    }

    /* determine which channels use AHT */
    if (parse_aht_info) {
        /* For AHT to be used, all non-zero blocks must reuse exponents from
           the first block.  Furthermore, for AHT to be used in the coupling
           channel, all blocks must use coupling and use the same coupling
           strategy. */
        s->channel_uses_aht[CPL_CH]=0;
        for (ch = (num_cpl_blocks != 6); ch <= s->channels; ch++) {
            int use_aht = 1;
            for (blk = 1; blk < 6; blk++) {
                if ((s->exp_strategy[blk][ch] != EXP_REUSE) ||
                        (!ch && s->cpl_strategy_exists[blk])) {
                    use_aht = 0;
                    break;
                }
            }
            s->channel_uses_aht[ch] = use_aht && get_bits1(gbc);
        }
    } else {
        memset(s->channel_uses_aht, 0, sizeof(s->channel_uses_aht));
    }

    /* per-frame SNR offset */
    if (!s->snr_offset_strategy) {
        int csnroffst = (get_bits(gbc, 6) - 15) << 4;
        int snroffst = (csnroffst + get_bits(gbc, 4)) << 2;
        for (ch = 0; ch <= s->channels; ch++)
            s->snr_offset[ch] = snroffst;
    }

    /* transient pre-noise processing data */
    if (s->transient_proc) {
        for (i = 0; i < s->fbw_channels; i++) {
           	ch = ff_channeltab[s->channel_mode][i];
			s->transproc[ch] = get_bits1(gbc);
            if (s->transproc[ch]) { // channel in transient processing
                s->transprocloc[ch] = get_bits(gbc, 10); // transient processing location
                s->transproclen[ch] = get_bits(gbc, 8);  // transient processing length
            }
        }
    }

    /* spectral extension attenuation data */
    for (ch = 1; ch <= s->fbw_channels; ch++) {
        if (parse_spx_atten_data && get_bits1(gbc)) {
            s->spx_atten_code[ch] = get_bits(gbc, 5);
        } else {
            s->spx_atten_code[ch] = -1;
        }
    }

    /* block start information */
    if (s->num_blocks > 1 && get_bits1(gbc)) {
        /* reference: Section E2.3.2.27
           nblkstrtbits = (numblks - 1) * (4 + ceiling(log2(words_per_frame)))
           The spec does not say what this data is or what it's used for.
           It is likely the offset of each block within the frame. */
        int block_start_bits = (s->num_blocks-1) * (4 + av_log2(s->frame_size-2));
        skip_bits_long(gbc, block_start_bits);
        PRINTF("Block start info", 1);
    }

    /* syntax state initialization */
    for (ch = 1; ch <= s->fbw_channels; ch++) {
        s->first_spx_coords[ch] = 1;
        s->first_cpl_coords[ch] = 1;
    }
    s->first_cpl_leak = 1;

    return 0;
}

/**
 * Parse the 'sync info' and 'bit stream info' from the AC-3 bitstream.
 * GetBitContext within AC3DecodeContext must point to
 * the start of the synchronized AC-3 bitstream.
 */
static int ac3_parse_header(AC3DecodeContext *s)
{
    GetBitContext *gbc = &s->gbc;
    int i;

    /* read the rest of the bsi. read twice for dual mono mode. */
    for (i = 0; i < (s->channel_mode ? 1 : 2); i++) {
		s->dialnorm[i] = get_bits(gbc, 5);
		s->compre[i] = get_bits1(gbc);
		if (s->compre[i])
            s->compr[i] = get_sbits(gbc, 8); // compression
        if (get_bits1(gbc))
            skip_bits(gbc, 8); //skip language code
        if (get_bits1(gbc))
            skip_bits(gbc, 7); //skip audio production information
    }

    skip_bits(gbc, 2); //skip copyright bit and original bitstream bit

	if (s->bitstream_id == 6) {
		if (get_bits1(gbc)) {
			if (s->channel_mode >= 3) {
				s->dmixmod = get_bits(gbc, 2);
				s->ltrt_cmixlev = get_bits(gbc, 3);
				s->ltrt_surmixlev = get_bits(gbc, 3);
				s->loro_cmixlev = get_bits(gbc, 3);
				s->loro_surmixlev = get_bits(gbc, 3);
				s->lxrxmixlevsd = 1;
				s->dmixmodd = 1;

				/* Limit lxrxsurmixlev to -1.5dB max per Annex D spec */
				if (s->ltrt_surmixlev < 3)
					s->ltrt_surmixlev = 3;
				if (s->loro_surmixlev < 3)
					s->loro_surmixlev = 3;
			} else {
				skip_bits(gbc, 14);
			}
		}
        if (get_bits1(gbc)) 
			skip_bits(gbc, 14);
	}
	else /* if (bsid != 6) skip the timecodes (or extra bitstream information for Alternate Syntax) */
	{
        if (get_bits1(gbc))
            skip_bits(gbc, 14); //skip timecode1 / xbsi1
        if (get_bits1(gbc))
            skip_bits(gbc, 14); //skip timecode2 / xbsi2
	}

    /* skip additional bitstream info */
    if (get_bits1(gbc)) {
        i = get_bits(gbc, 6);
        do {
            skip_bits(gbc, 8);
        } while(i--);
    }

    return 0;
}

#define AC3_HEADER_SIZE 7


static const uint8_t eac3_blocks[4] = {
    1, 2, 3, 6
};

static int parse_header(GetBitContext *gbc, AC3HeaderInfo *hdr, int isEac3)
{
    int frame_size_code;
	int ac3_bsid = 0, eac3_bsid = 0;

    memset(hdr, 0, sizeof(*hdr));

    hdr->sync_word = get_bits(gbc, 16);
    if(hdr->sync_word != 0x0B77)
        return AC3_PARSE_ERROR_SYNC;

    /* read ahead to bsid to distinguish between AC-3 and E-AC-3 */
    hdr->bitstream_id = show_bits_long(gbc, 29) & 0x1F;
	if(hdr->bitstream_id < 9)
		ac3_bsid = 1;
	else if(hdr->bitstream_id>10 && hdr->bitstream_id<=16)
		eac3_bsid = 1;

    hdr->num_blocks = 6;

    /* set default mix levels */
    hdr->legacy_cmixlev   = 0;  // -3db
    hdr->legacy_surmixlev = 0;  // -3db

    if(ac3_bsid || (eac3_bsid==0 && isEac3==0)) {
        /* Normal AC-3 */
        hdr->crc1 = get_bits(gbc, 16);
        hdr->sr_code = get_bits(gbc, 2);
        if(hdr->sr_code == 3)
            return AC3_PARSE_ERROR_SAMPLE_RATE;

        frame_size_code = get_bits(gbc, 6);
        if(frame_size_code > 37)
            return AC3_PARSE_ERROR_FRAME_SIZE;

        skip_bits(gbc, 5); // skip bsid, already got it

        hdr->bitstream_mode = get_bits(gbc, 3);
        hdr->channel_mode = get_bits(gbc, 3);

        if(hdr->channel_mode == AC3_CHMODE_STEREO) {
            skip_bits(gbc, 2); // skip dsurmod
        } else {
            if((hdr->channel_mode & 1) && hdr->channel_mode != AC3_CHMODE_MONO)
                hdr->legacy_cmixlev = get_bits(gbc, 2);
            if(hdr->channel_mode & 4)
                hdr->legacy_surmixlev = get_bits(gbc, 2);
        }

        hdr->lfe_on = get_bits1(gbc);

        hdr->sr_shift = FFMAX(hdr->bitstream_id, 8) - 8;
        hdr->sample_rate = ff_ac3_sample_rate_tab[hdr->sr_code] >> hdr->sr_shift;
        hdr->bit_rate = (ff_ac3_bitrate_tab[frame_size_code>>1] * 1000) >> hdr->sr_shift;
        hdr->channels = ff_ac3_channels_tab[hdr->channel_mode] + hdr->lfe_on;
        hdr->frame_size = ff_ac3_frame_size_tab[frame_size_code][hdr->sr_code] * 2;
        hdr->frame_type = EAC3_FRAME_TYPE_AC3_CONVERT; //EAC3_FRAME_TYPE_INDEPENDENT;
        hdr->substreamid = 0;
	} else {
        /* Enhanced AC-3 */
        hdr->crc1 = 0;
        hdr->frame_type = get_bits(gbc, 2);

        hdr->substreamid = get_bits(gbc, 3);

        hdr->frame_size = (get_bits(gbc, 11) + 1) << 1;
        if(hdr->frame_size < AC3_HEADER_SIZE)
            return AC3_PARSE_ERROR_FRAME_SIZE;

        hdr->sr_code = get_bits(gbc, 2);
        if (hdr->sr_code == 3) {
            int sr_code2 = get_bits(gbc, 2);
            if(sr_code2 == 3)
                return AC3_PARSE_ERROR_SAMPLE_RATE;
            hdr->sample_rate = ff_ac3_sample_rate_tab[sr_code2] / 2;
            hdr->sr_shift = 1;
        } else {
            hdr->num_blocks = eac3_blocks[get_bits(gbc, 2)];
            hdr->sample_rate = ff_ac3_sample_rate_tab[hdr->sr_code];
            hdr->sr_shift = 0;
        }

        hdr->channel_mode = get_bits(gbc, 3);
        hdr->lfe_on = get_bits1(gbc);

        hdr->bit_rate = (uint32_t)(8.0 * hdr->frame_size * hdr->sample_rate /
                        (hdr->num_blocks * 256.0));
        hdr->channels = ff_ac3_channels_tab[hdr->channel_mode] + hdr->lfe_on;
	} //else
      //  return AC3_PARSE_ERROR_BSID;

    //hdr->channel_layout = ff_ac3_channel_layout_tab[hdr->channel_mode];
    //if (hdr->lfe_on)
    //    hdr->channel_layout |= LOW_FREQUENCY;

	if( ac3_bsid==0 && eac3_bsid==0)
        return AC3_PARSE_ERROR_BSID;

    return 0;
}

/**
 * Common function to parse AC-3 or E-AC-3 frame header
 */
int parse_frame_header(AC3DecodeContext *s)
{
    AC3HeaderInfo hdr;
    int err;
	int i;

    err = parse_header(&s->gbc, &hdr, s->eac3);

    /* get decoding parameters from header info */
    s->bit_alloc_params.sr_code     = hdr.sr_code;
    s->bitstream_mode               = hdr.bitstream_mode;
    s->channel_mode                 = hdr.channel_mode;
    //s->channel_layout               = hdr.channel_layout;
    s->lfe_on                       = hdr.lfe_on;
    s->bit_alloc_params.sr_shift    = hdr.sr_shift;
    s->sample_rate                  = hdr.sample_rate;
    s->bit_rate                     = hdr.bit_rate;
    s->channels                     = hdr.channels;
    s->fbw_channels                 = s->channels - s->lfe_on;
    s->lfe_ch                       = s->fbw_channels + 1;
    s->frame_size                   = hdr.frame_size;
    s->legacy_cmixlev               = hdr.legacy_cmixlev;
    s->legacy_surmixlev             = hdr.legacy_surmixlev;
    s->num_blocks                   = hdr.num_blocks;
    s->frame_type                   = hdr.frame_type;
    s->substreamid                  = hdr.substreamid;
    s->bitstream_id                 = hdr.bitstream_id;

    if(err)
        return err;

    if(s->lfe_on) {
        s->start_freq[s->lfe_ch] = 0;
        s->end_freq[s->lfe_ch] = 7;
        s->num_exp_groups[s->lfe_ch] = 2;
        s->channel_in_cpl[s->lfe_ch] = 0;
    }

	s->lxrxmixlevsd = 0;
    s->dmixmodd = 0;
	s->lfemixlevcode	= 0;	/* Initialize LFE Mix Level Code Exist Flag to 0 */
	s->lfemixlevcod		= 10;	/* Initialize LFE Mix Level Code to 10 (0 dB Gain) */
	s->ltrt_cmixlev     = 4;    // -3db
	s->ltrt_surmixlev   = 4;    // -3db
	s->loro_cmixlev     = 4;    // -3db
	s->loro_surmixlev   = 4;    // -3db

	s->transient_proc = 0;
	for(i=0;i<s->fbw_channels;i++)
		s->transproc[i] = 0;

    if (hdr.bitstream_id < 9) {
        s->eac3                  = 0;
        s->snr_offset_strategy   = 2;
        s->block_switch_syntax   = 1;
        s->dither_flag_syntax    = 1;
        s->bit_allocation_syntax = 1;
        s->fast_gain_syntax      = 0;
        s->first_cpl_leak        = 0;
        s->dba_syntax            = 1;
        s->skip_syntax           = 1;
        memset(s->channel_uses_aht, 0, sizeof(s->channel_uses_aht));
        return ac3_parse_header(s);
#ifdef CONFIG_EAC3_DECODER
    } else {
        s->eac3 = 1;
        return eac3_parse_header(s);
#endif
    }
}

int nAC3Plus = 0;
int ac3_syncinfo(uint8_t *buf)
{
    int frame_size_code;
	int sr_code;
	int size;
	uint8_t byte2, byte3, byte4, byte5;
	
	if(buf[0]==0x77 && buf[1]==0x0B)
	{ // byte-swapped AC3
		byte2 = buf[3];
		byte3 = buf[2];
		byte4 = buf[5];
	    byte5 = buf[4];
	}
	else if(buf[1]==0x77 && buf[0]==0x0B)
	{ // normal AC3
		byte2 = buf[2];
		byte3 = buf[3];
		byte4 = buf[4];
		byte5 = buf[5];
	}
	else
		return AC3_PARSE_ERROR_SYNC;

    if(((byte5>>3)&0x1f) < 9)  // AC3
	{
		sr_code = ((byte4>>6)&0x3);
		if(sr_code==3) return -1;
	       
        frame_size_code = (byte4&0x3f);
        if(frame_size_code > 37) return -1;

        nAC3Plus = 0;
            
        size = ff_ac3_frame_size_tab[frame_size_code][sr_code] * 2;
    }
	else   // E-AC3
    {
        nAC3Plus = 1;
		size = ((int)((byte2&0x7)<<8) + (int)byte3 + 1)<<1;
    }

	return size;  
}

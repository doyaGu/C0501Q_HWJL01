/*
 * Common code between the AC-3 encoder and decoder
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
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
 * Common code between the AC-3 encoder and decoder.
 */

#ifndef AVCODEC_AC3_H
#define AVCODEC_AC3_H

#define AC3_MAX_CODED_FRAME_SIZE 3840 /* in bytes */
#define AC3_MAX_CHANNELS 7 /* including LFE channel */

#define AC3_MAX_COEFS   256
#define AC3_BLOCK_SIZE  256
#define AC3_MAX_BLOCKS    6
#define AC3_FRAME_SIZE (AC3_MAX_BLOCKS * 256)
#define AC3_WINDOW_SIZE (AC3_BLOCK_SIZE * 2)
#define AC3_CRITICAL_BANDS 50
#define AC3_MAX_CPL_BANDS  18
#define AC3_SYNTHBUFLEN    1024
#define AC3_TC1  256
#define AC3_TC2  128
#define AC3_MINTRANDIST    6   /*!< Minimum number of blocks between transients */
#define AC3_MINTRANLOC  1024   /*!< Minimum value for designated transient location (4*transprocloc) */
#define AC3_MAXTRANLOC  2556   /*!< Maximum value for designated transient location (4*transprocloc) */

#define AC3_ECPD_BEGFTABSIZE	(16)		/*!< Size of <i>ecplbegf</i> table */
#define AC3_ECPD_MAXNUMECPBNDS	(22)		/*!< Maximum number of enhanced coupling bands allowed */
#define AC3_ECPD_MINNUMINDPBNDS	(9)		    /*!< Minimum number of independent bands (subbands 0 - 8) */

#include "common.h"
#include "ac3tab.h"

/* exponent encoding strategy */
#define EXP_REUSE 0
#define EXP_NEW   1

#define EXP_D15   1
#define EXP_D25   2
#define EXP_D45   3

/* pre-defined gain values */
#define LEVEL_P6DB          LEVEL(2.0)
#define LEVEL_P3DB          LEVEL(1.4142135623730950)
#define LEVEL_P1P_5DB       LEVEL(1.1892071150027209)
#define	LEVEL_M1_2DB		LEVEL(0.870963590)		/*!< -1.2 dB */
#define LEVEL_M1_5DB        LEVEL(0.8408964152537145)
#define LEVEL_M3DB          LEVEL(0.7071067811865476)
#define LEVEL_M4_5DB        LEVEL(0.5946035575013605)
#define LEVEL_M6DB          LEVEL(0.5000000000000000)
#define	LEVEL_M6_2DB		LEVEL(0.489778819)		/*!< -6.2 dB */
#define LEVEL_M7_5DB        LEVEL(0.4204482076268572)
#define LEVEL_M9DB          LEVEL(0.3535533905932738)
#define LEVEL_M10_5DB       LEVEL(0.2973017787506802)
#define LEVEL_M12DB         LEVEL(0.2500000000000000)
#define LEVEL_1             LEVEL(1.0000000000000000)
#define LEVEL_0             LEVEL(0.0000000000000000)

#define		PLUS11DB		7509   /* 7509 is described in the comment block above */
#define		MINUS11DB		-7509  /* 7509 is described in the comment block above */

/* Audio channel number */
#define FRONT_LEFT             0
#define FRONT_CENTER           1
#define FRONT_RIGHT            2
#define BACK_LEFT              3
#define BACK_CENTER            3
#define BACK_RIGHT             4
#define LOW_FREQUENCY          5
#define NONE                   -1

/* dmixmod values */
#define		AC3_DMIXMOD_NIND		0		/*!< Not indicated */
#define		AC3_DMIXMOD_LTRT		1		/*!< Lt/Rt downmix preferred */
#define		AC3_DMIXMOD_LORO		2		/*!< Lo/Ro downmix preferred */
#define		AC3_DMIXMOD_RSVD		3		/*!< Reserved */
#define		AC3_DMIXMOD_PLII		3		/*!< PL II downmix preferred */

/** Delta bit allocation strategy */
typedef enum {
    DBA_REUSE = 0,
    DBA_NEW,
    DBA_NONE,
    DBA_RESERVED
} AC3DeltaStrategy;

/** Channel mode (audio coding mode) */
typedef enum {
    AC3_CHMODE_DUALMONO = 0,
    AC3_CHMODE_MONO,
    AC3_CHMODE_STEREO,
    AC3_CHMODE_3F,
    AC3_CHMODE_2F1R,
    AC3_CHMODE_3F1R,
    AC3_CHMODE_2F2R,
    AC3_CHMODE_3F2R
} AC3ChannelMode;

/* compression mode */
enum { 
	AC3_COMP_LINE = 0, 
	AC3_COMP_RF, 
	AC3_COMP_CUSTOM_0, 
    AC3_COMP_CUSTOM_1 
};

/* preferred stereo mode */
enum { 
	AC3_STEREOMODE_AUTO=0, 
	AC3_STEREOMODE_SRND, 
	AC3_STEREOMODE_STEREO 
};

enum { 
	AC3_STEREODMIX_LTRT=0, 
	AC3_STEREODMIX_LORO,
	AC3_STEREODMIX_PLII 
};

/* dual mono downmix mode */
enum { 
	AC3_DUAL_STEREO=0, 
	AC3_DUAL_LEFTMONO, 
	AC3_DUAL_RGHTMONO, 
	AC3_DUAL_MIXMONO 
};

typedef enum {
    AC3_PARSE_ERROR_SYNC        = -1,
    AC3_PARSE_ERROR_BSID        = -2,
    AC3_PARSE_ERROR_SAMPLE_RATE = -3,
    AC3_PARSE_ERROR_FRAME_SIZE  = -4,
    AC3_PARSE_ERROR_FRAME_TYPE  = -5,
    AC3_PARSE_ERROR_CRC         = -6,
    AC3_PARSE_ERROR_CHANNEL_CFG = -7,
} AC3ParseError;

typedef enum {
    EAC3_FRAME_TYPE_INDEPENDENT = 0,
    EAC3_FRAME_TYPE_DEPENDENT,
    EAC3_FRAME_TYPE_AC3_CONVERT,
    EAC3_FRAME_TYPE_RESERVED
} EAC3FrameType;

#endif /* AVCODEC_AC3_H */

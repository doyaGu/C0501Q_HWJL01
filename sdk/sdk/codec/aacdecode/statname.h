/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: statname.h,v 1.1 2005/02/26 01:47:34 jrecker Exp $
 *
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com)
 * February 2005
 *
 * statname.h - name mangling macros for static linking
 **************************************************************************************/

#ifndef _STATNAME_H
#define _STATNAME_H

/* define STAT_PREFIX to a unique name for static linking
 * all the C functions and global variables will be mangled by the preprocessor
 *   e.g. void DCT4(...) becomes void raac_DCT4(...)
 */
#define STAT_PREFIX     smaac

#define STATCC1(x,y,z)  STATCC2(x,y,z)
#define STATCC2(x,y,z)  x##y##z

#if defined(STAT_PREFIX)
#define STATNAME(func)  STATCC1(STAT_PREFIX, _, func)
#else
#define STATNAME(func)  func
#endif // defined(STAT_PREFIX)

#if !defined(__FREERTOS__) && !defined(USE_RENAME)
/* these symbols are common to all implementations */
#define AllocateBuffers         STATNAME(AllocateBuffers)
#define FreeBuffers             STATNAME(FreeBuffers)

#define SetRawBlockParams       STATNAME(SetRawBlockParams)
#define PrepareRawBlock         STATNAME(PrepareRawBlock)
#define FlushCodec              STATNAME(FlushCodec)

#define UnpackADTSHeader        STATNAME(UnpackADTSHeader)
#define GetADTSChannelMapping   STATNAME(GetADTSChannelMapping)
#define UnpackADIFHeader        STATNAME(UnpackADIFHeader)
#define DecodeNextElement       STATNAME(DecodeNextElement)
#define DecodeNoiselessData     STATNAME(DecodeNoiselessData)
#define Dequantize              STATNAME(Dequantize)
#define StereoProcess           STATNAME(StereoProcess)
#define DeinterleaveShortBlocks STATNAME(DeinterleaveShortBlocks)
#define PNS                     STATNAME(PNS)
#define TNSFilter               STATNAME(TNSFilter)
#define IMDCT                   STATNAME(IMDCT)

#define InitSBR                 STATNAME(InitSBR)
#define DecodeSBRBitstream      STATNAME(DecodeSBRBitstream)
#define DecodeSBRData           STATNAME(DecodeSBRData)
#define FlushCodecSBR           STATNAME(FlushCodecSBR)
#define FreeSBR                 STATNAME(FreeSBR)

/* global ROM tables */
#define sampRateTab             STATNAME(sampRateTab)
#define predSFBMax              STATNAME(predSFBMax)
#define channelMapTab           STATNAME(channelMapTab)
#define elementNumChans         STATNAME(elementNumChans)
#define sfBandTotalShort        STATNAME(sfBandTotalShort)
#define sfBandTotalLong         STATNAME(sfBandTotalLong)
#define sfBandTabShortOffset    STATNAME(sfBandTabShortOffset)
#define sfBandTabShort          STATNAME(sfBandTabShort)
#define sfBandTabLongOffset     STATNAME(sfBandTabLongOffset)
#define sfBandTabLong           STATNAME(sfBandTabLong)
#define tnsMaxBandsShortOffset  STATNAME(tnsMaxBandsShortOffset)
#define tnsMaxBandsShort        STATNAME(tnsMaxBandsShort)
#define tnsMaxOrderShort        STATNAME(tnsMaxOrderShort)
#define tnsMaxBandsLongOffset   STATNAME(tnsMaxBandsLongOffset)
#define tnsMaxBandsLong         STATNAME(tnsMaxBandsLong)
#define tnsMaxOrderLong         STATNAME(tnsMaxOrderLong)

/* in your implementation's top-level include file (e.g. real\coder.h) you should
 *   add new #define sym STATNAME(sym) lines for all the
 *   additional global functions or variables which your
 *   implementation uses
 */

#else
/* these symbols are common to all implementations */
#define AllocateBuffers         STATNAME(f1)
#define FreeBuffers             STATNAME(f2)

#define SetRawBlockParams       STATNAME(f3)
#define PrepareRawBlock         STATNAME(f4)
#define FlushCodec              STATNAME(f5)

#define UnpackADTSHeader        STATNAME(f6)
#define GetADTSChannelMapping   STATNAME(f7)
#define UnpackADIFHeader        STATNAME(f8)
#define DecodeNextElement       STATNAME(f9)
#define DecodeNoiselessData     STATNAME(f10)
#define Dequantize              STATNAME(f11)
#define StereoProcess           STATNAME(f12)
#define DeinterleaveShortBlocks STATNAME(f13)
#define PNS                     STATNAME(f14)
#define TNSFilter               STATNAME(f15)
#define IMDCT                   STATNAME(f16)

#define InitSBR                 STATNAME(f17)
#define DecodeSBRBitstream      STATNAME(f18)
#define DecodeSBRData           STATNAME(f19)
#define FlushCodecSBR           STATNAME(f20)
#define FreeSBR                 STATNAME(f21)

/* global ROM tables */
#define sampRateTab             STATNAME(d1)
#define predSFBMax              STATNAME(d2)
#define channelMapTab           STATNAME(d3)
#define elementNumChans         STATNAME(d4)
#define sfBandTotalShort        STATNAME(d5)
#define sfBandTotalLong         STATNAME(d6)
#define sfBandTabShortOffset    STATNAME(d7)
#define sfBandTabShort          STATNAME(d8)
#define sfBandTabLongOffset     STATNAME(d9)
#define sfBandTabLong           STATNAME(d10)
#define tnsMaxBandsShortOffset  STATNAME(d11)
#define tnsMaxBandsShort        STATNAME(d12)
#define tnsMaxOrderShort        STATNAME(d13)
#define tnsMaxBandsLongOffset   STATNAME(d14)
#define tnsMaxBandsLong         STATNAME(d15)
#define tnsMaxOrderLong         STATNAME(d16)
#endif // !defined(__FREERTOS__)

#endif /* _STATNAME_H */


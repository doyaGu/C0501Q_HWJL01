/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: buffers.c,v 1.1 2005/02/26 01:47:34 jrecker Exp $
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
 * buffers.c - allocation and deallocation of internal AAC decoder buffers
 **************************************************************************************/
#include <string.h>

#include "coder.h"

#if defined(USE_STATNAME)
#if !defined(__FREERTOS__) && !defined(USE_RENAME)
# define aacDecInfo_                     STATNAME(aacDecInfo_)
# define PSInfoBase_                     STATNAME(PSInfoBase_)
#else // !defined(__FREERTOS__)
# define aacDecInfo_                     STATNAME(d1000)
# define PSInfoBase_                     STATNAME(d1001)
#endif // !defined(__FREERTOS__)
#endif // defined(USE_STATNAME)

static AACDecInfo aacDecInfo_;
static PSInfoBase PSInfoBase_;

/**************************************************************************************
 * Function:    AllocateBuffers
 *
 * Description: allocate all the memory needed for the AAC decoder
 *
 * Inputs:      none
 *
 * Outputs:     none
 *
 * Return:      pointer to AACDecInfo structure, cleared to all 0's (except for
 *                pointer to platform-specific data structure)
 *
 * Notes:       if one or more mallocs fail, function frees any buffers already
 *                allocated before returning
 **************************************************************************************/
AACDecInfo *AllocateBuffers(void)
{
    AACDecInfo *aacDecInfo;
    aacDecInfo = &aacDecInfo_;
    memset(aacDecInfo, 0, sizeof(AACDecInfo));
    aacDecInfo->psInfoBase = &PSInfoBase_;
    memset(aacDecInfo->psInfoBase, 0, sizeof(PSInfoBase));

    return aacDecInfo;
}


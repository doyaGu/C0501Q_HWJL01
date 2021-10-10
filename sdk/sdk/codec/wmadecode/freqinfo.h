/*******************************************************************************
 * Copyright (c) 2006-2008 ITE Tech. Corp., All Rights Reserved.
 *
 * FreqInfo
 *******************************************************************************/

#ifndef __FREQINFO_H__
#define __FREQINFO_H__

// Scale function 0: No scale
// Scale function 1: ((i==0) ? 0 : (int)(log(i)*256/log(256)))
// Scale function 2: ((i*4/3 > 255) ? 255 : i*4/3)
// Scale function 3: (int)(255/pow(255,0.45))*pow(i,0.45)
#define USE_SCALE_FUNC 2

void updateFreqInfo();

#endif // __FREQINFO_H__


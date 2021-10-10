/***************************************************************************
* Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
*
* @file
* Header file for Win32 porting.
*
* @author Kuoping Hsu
* @version 1.0
*
***************************************************************************/

#ifndef __WIN32_H__
#define __WIN32_H__

#if defined(WIN32) || defined(__CYGWIN__)

///////////////////////////////////////////////////////////////////////////
//                              Constant
///////////////////////////////////////////////////////////////////////////
    #define DrvDecode_RdPtr 0
    #define DrvDecode_WrPtr 2
    #define DrvAudioCtrl    4
    #define DrvDecode_Frame 6
// define DrvDecode_Frame+2 8

    #define DrvDecode_EOF   (1 << 0)
    #define DrvDecode_STOP  (1 << 1)

///////////////////////////////////////////////////////////////////////////
//                              Function
///////////////////////////////////////////////////////////////////////////
MMP_INLINE void dc_invalidate(void);
MMP_INLINE void or32_invalidate_cache(void *start, unsigned bytes);
MMP_INLINE void win32_init(void);
MMP_INLINE int isVoiceOff(void);
MMP_INLINE int isEOF(void);
MMP_INLINE int isSTOP(void);
MMP_INLINE int isPAUSE(void);
MMP_INLINE void MMIO_Write(int reg, int val);
MMP_INLINE int MMIO_Read(int reg);
MMP_INLINE void SetOutWrPtr(int val);
MMP_INLINE int GetOutRdPtr(void);
MMP_INLINE int GetOutWrPtr(void);
MMP_INLINE void or32_sleep(int ticks);
MMP_INLINE void or32_delay(int ms);
MMP_INLINE void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
MMP_INLINE void GetParam(int argc, char **argv);
MMP_INLINE void pauseDAC(int flag);
MMP_INLINE void deactiveDAC(void);

///////////////////////////////////////////////////////////////////////////
//                              Macro
///////////////////////////////////////////////////////////////////////////
    #define memset32(addr, c, n) memset(addr, c, n * sizeof(int))
    #define memset16(addr, c, n) memset(addr, c, n * sizeof(short))

#endif    // defined(WIN32) || defined(__CYGWIN__)
#endif  /* __WIN32_H__ */
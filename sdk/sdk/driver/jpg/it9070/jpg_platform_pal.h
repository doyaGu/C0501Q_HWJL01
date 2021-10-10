#ifndef __jpg_pal_platform_H_OYHH7ln4_emtK_IGnj_Xash_KX6UXPiwE0I3__
#define __jpg_pal_platform_H_OYHH7ln4_emtK_IGnj_Xash_KX6UXPiwE0I3__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/itp.h"
#include "pal/pal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define jpg_sleep(ms)                           PalSleep(ms)

#define jpgReadReg(add, data)                   (*data = ithReadRegH(add))
#define jpgWriteReg(add, data)                  ithWriteRegH(add, data)
#define jpgWriteRegMark(add, data, mark)        ithWriteRegMaskH(add, data, mark)

#define jpgEnableClock()                        ithJpegEnableClock()
#define jpgDisableClock()                       ithJpegDisableClock()

#define jpgEnableVideoClock()                        ithJpegVideoEnableClock()
#define jpgDisableVideoClcok()                       ithJpegVideoDisableClock()

#define jpgResetHwReg()                         ithJpegResetReg()
#define jpgResetHwEngine()                      ithJpegResetEngine()

#define jpgReadVram(dest, src, size)            HOST_ReadBlockMemory(dest, src, size)
#define jpgWriteVram(dest, src, size)           HOST_WriteBlockMemory(dest, src, size)
#define jpgVmemAlloc(size)                      itpVmemAlloc(size)
#define jpgVmemFree(ptr)                        itpVmemFree(ptr)

#define ispResetHwReg()
#define ispResetHwEngine()                      HOST_ISP_Reset()
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif

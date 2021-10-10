#ifndef __jpg_itp_platform_H_2JKKRp6M_PzAB_0kMR_BGPS_csZfzFIaIg23__
#define __jpg_itp_platform_H_2JKKRp6M_PzAB_0kMR_BGPS_csZfzFIaIg23__

#ifdef __cplusplus
extern "C" {
#endif


#include "ite/itp.h"
#include <unistd.h>
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define jpg_sleep(ms)                           usleep(1000*ms)

#define jpgReadReg(add, data)                   (*data = ithReadRegH(add))
#define jpgWriteReg(add, data)                  ithWriteRegH(add, data)
#define jpgWriteRegMark(add, data, mark)        ithWriteRegMaskH(add, data, mark)

#define jpgEnableClock()                        ithJpegEnableClock()
#define jpgDisableClock()                       ithJpegDisableClock()

#define jpgEnableVideoClock()                        ithJpegVideoEnableClock()
#define jpgDisableVideoClcok()                       ithJpegVideoDisableClock()

#define jpgResetHwReg()                         ithJpegResetReg()
#define jpgResetHwEngine()                      ithJpegResetEngine()

#define jpgReadVram(dest, src, size)            ithReadVram((void*)dest, src, size)
#define jpgWriteVram(dest, src, size)           ithWriteVram(dest, (const void*)src, size)
#define jpgVmemAlloc(size)                      itpVmemAlloc(size)
#define jpgVmemFree(ptr)                        itpVmemFree(ptr)

#define ispResetHwReg()                         ithIspResetReg()
#define ispResetHwEngine()                      ithIspResetEngine()
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

#ifndef __jpg_null_platform_H_hoB8Zew7_L3PE_Meuo_y53U_g0r8nyMKBUny__
#define __jpg_null_platform_H_hoB8Zew7_L3PE_Meuo_y53U_g0r8nyMKBUny__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define jpg_sleep(ms)

#define jpgReadReg(add, data)
#define jpgWriteReg(add, data)
#define jpgWriteRegMark(add, data, mark)

#define jpgEnableClock()
#define jpgDisableClock()

#define jpgResetHwReg()
#define jpgResetHwEngine()

#define jpgReadVram(dest, src, size)
#define jpgWriteVram(dest, src, size)
#define jpgVmemAlloc(size)
#define jpgVmemFree(ptr)

#define ispResetHwReg()
#define ispResetHwEngine()
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

#ifndef _XD_ECC_H__
#define _XD_ECC_H__

#include "xd_type.h"

////////////////////////////////////////////////////////////
//
//	Constant variable or Marco
//
////////////////////////////////////////////////////////////
#define XD_ECC_NO_ERROR      0
#define XD_ECC_CORRECTED     1
#define XD_ECC_ERROR_ECC     2
#define XD_ECC_UNCORRECTABLE 3
#define XD_ECC_UNKNOWN_ERROR 4

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

MMP_BOOL  XD_EccInitial();
MMP_BOOL  XD_EccCalculate(MMP_UINT8* pBuffer, MMP_UINT8* pEcc1, MMP_UINT8* pEcc2, MMP_UINT8* pEcc3);
MMP_UINT8 XD_EccCorrectData(MMP_UINT8* pBuffer, MMP_UINT8 Ecc1, MMP_UINT8 Ecc2, MMP_UINT8 Ecc3);
void      XD_EccTerminate();

#ifdef __cplusplus
}
#endif

#endif

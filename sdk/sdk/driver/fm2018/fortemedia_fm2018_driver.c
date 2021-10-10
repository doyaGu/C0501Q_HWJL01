
#include "ite/ith.h"

         
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static void 
_FM2018_DSP_Initialize(
    void)
{
    FM2018_MemWriteI2C_16Bit(0x1E6F, 0x8005);
    FM2018_MemWriteI2C_16Bit(0x1E3A, 0x0000);
    ithDelay(250000);
    FM2018_MemWriteI2C_16Bit(0x1E33, 0x0000);
    FM2018_MemWriteI2C_16Bit(0x1E34, 0x0088);
    FM2018_MemWriteI2C_16Bit(0x1E36, 0x001F);
    FM2018_MemWriteI2C_16Bit(0x1E3D, 0x0200);
    FM2018_MemWriteI2C_16Bit(0x1E41, 0x0101);
    FM2018_MemWriteI2C_16Bit(0x1E44, 0x0001);
    FM2018_MemWriteI2C_16Bit(0x1E45, 0x03FC);
    FM2018_MemWriteI2C_16Bit(0x1E46, 0x0011);
    FM2018_MemWriteI2C_16Bit(0x1E47, 0x2000);
    FM2018_MemWriteI2C_16Bit(0x1E4D, 0x0080);
    FM2018_MemWriteI2C_16Bit(0x1E63, 0x0005);
    FM2018_MemWriteI2C_16Bit(0x1E86, 0x0008);
    FM2018_MemWriteI2C_16Bit(0x1E87, 0x0002);
    FM2018_MemWriteI2C_16Bit(0x1E88, 0x2000);
    FM2018_MemWriteI2C_16Bit(0x1E89, 0x0009);
    FM2018_MemWriteI2C_16Bit(0x1E8B, 0x0050);
    FM2018_MemWriteI2C_16Bit(0x1E8C, 0x0018);
    FM2018_MemWriteI2C_16Bit(0x1E92, 0x2000);
    FM2018_MemWriteI2C_16Bit(0x1EBC, 0x6800);
    FM2018_MemWriteI2C_16Bit(0x1EBD, 0x0100);
    FM2018_MemWriteI2C_16Bit(0x1EFF, 0x4000);
    FM2018_MemWriteI2C_16Bit(0x1E4C, 0x0A00);
    FM2018_MemWriteI2C_16Bit(0x1E3A, 0x0000);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void 
itpFM2018Initialize(
    void)
{
    _FM2018_DSP_Initialize();
}
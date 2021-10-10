#include "encoder/encoder_types.h"
#include "sensor/ar0130_driver.h"


//=============================================================================
//                Constant Definition
//=============================================================================
#define GPIO_BASE               0xDE000000

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

//=============================================================================
//                Public Function Definition
//=============================================================================
void
mmpAR0130Initialize(
    void)
{
    printf("init AR0130\n");
    //Set GPIO30 to Low, GPIO31 to High
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 30), (1 << 30));
    AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 31), (1 << 31));
    //Set GPIO30, 31 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 30), (1 << 30));
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 31), (1 << 31));
    //Set GPIO30, 31 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (14 * 2)), (0x3 << (14 * 2)));
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (15 * 2)), (0x3 << (15 * 2)));
}

void
mmpAR0130PowerOn(
    void)
{
    //GPIO30 PWDN
    //Set GPIO30 to Low
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 30), (1 << 30));
    //Set GPIO30 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 30), (1 << 30));
    //Set GPIO30 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (14 * 2)), (0x3 << (14 * 2)));
}

void
mmpAR0130PowerOff(
	void)
{
	AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 30), (1 << 30));
}

void
mmpAR0130LedOn(
    void)
{
    //GPIO31 LED_EN
    //Set GPIO31 to High
    AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 31), (1 << 31));
    //Set GPIO31 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 31), (1 << 31));
    //Set GPIO31 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (15 * 2)), (0x3 << (15 * 2)));
}

void
mmpAR0130LedOff(
    void)
{
	AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 31), (1 << 31));
}

void
mmpAR0130SetAntiFlicker50Hz(
    void)
{
    return;
};

void
mmpAR0130SetAntiFlicker60Hz(
    void)
{
    return;
};


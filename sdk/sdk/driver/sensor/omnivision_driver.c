#include "encoder/encoder_types.h"
#include "sensor/omnivision_io.h"
#include "sensor/omnivision_driver.h"


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
mmpOmnivisionInitialize(
    void)
{
    printf("init Omnivision\n");
    //Set GPIO30 to Low, GPIO31 to High
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 30), (1 << 30));
    AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 31), (1 << 31));
    //Set GPIO30, 31 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 30), (1 << 30));
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 31), (1 << 31));
    //Set GPIO30, 31 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (14 * 2)), (0x3 << (14 * 2)));
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (15 * 2)), (0x3 << (15 * 2)));

    Omnivision_WriteI2C_Byte(0x09, 0x03); //[1:0]: driving, 00: 1x, 01: 2x, 10: 3x, 11: 4x
    uint8_t data;
    data = Omnivision_ReadI2C_Byte(0x09);    
    printf("set Omnivision driving %x\n", data);
}

void
mmpOmnivisionPowerOn(
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
mmpOmnivisionPowerOff(
	void)
{
	AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 30), (1 << 30));
}

void
mmpOmnivisionLedOn(
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
mmpOmnivisionLedOff(
    void)
{
	AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 31), (1 << 31));
}

void
mmpOmnivisionSetAntiFlicker50Hz(
    void)
{
    Omnivision_WriteI2C_Byte(0x2b,0x9e);
};

void
mmpOmnivisionSetAntiFlicker60Hz(
    void)
{
    Omnivision_WriteI2C_Byte(0x2b,0x0);
};


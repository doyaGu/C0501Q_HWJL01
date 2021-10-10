
#include "encoder/encoder_types.h"
#include "sensor/novatek_nt99141_io.h"
#include "sensor/novatek_nt99141_driver.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define AntiFlicker_60Hz_MODE 8
#define AntiFlicker_50Hz_MODE 4
#define GPIO_BASE             0xDE000000

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static bool gbNovatekInit = MMP_FALSE;
static uint8_t gFlickerMode = 0;

//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_NOVATEK_NT99141_SMPTE_30fps_50Hz(
    void)
{
	//[YUYV_1280x720_30.00_30.04_Fps]    //MCLK=24MHz & 50Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xDF); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x7B); 
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F); 
    NOVATEK_WriteI2C_8Bit(0x3028, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3029, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3); 
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C); 
    NOVATEK_WriteI2C_8Bit(0x300C, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x300D, 0xE6); 
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0); 
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x31); 
    NOVATEK_WriteI2C_8Bit(0x32BB, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x38); 
    NOVATEK_WriteI2C_8Bit(0x32BD, 0x3C); 
    NOVATEK_WriteI2C_8Bit(0x32BE, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06); 
	NOVATEK_WriteI2C_8Bit(0x3060, 0x01);

	NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xDF); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x7A); 
	NOVATEK_WriteI2C_8Bit(0x32DB, 0x7B);
	
}

static void
_NOVATEK_NT99141_SMPTE_25fps_50Hz(
    void)
{
	//[YUYV_1280x720_25.00_25.01_Fps]    //MCLK=24MHz & 50Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xC1); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x78); 
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F); 
    NOVATEK_WriteI2C_8Bit(0x3028, 0x1F); 
    NOVATEK_WriteI2C_8Bit(0x3029, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3); 
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C); 
    NOVATEK_WriteI2C_8Bit(0x300C, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x300D, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0); 
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x31); 
    NOVATEK_WriteI2C_8Bit(0x32BB, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x38); 
    NOVATEK_WriteI2C_8Bit(0x32BD, 0x3C); 
    NOVATEK_WriteI2C_8Bit(0x32BE, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06); 
	NOVATEK_WriteI2C_8Bit(0x3060, 0x01);

	NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xC1); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80); 
	NOVATEK_WriteI2C_8Bit(0x32DB, 0x78);
	
}

static void
_NOVATEK_NT99141_SMPTE_30fps_60Hz(
    void)
{
	//[YUYV_1280x720_30.00_30.04_Fps]    //MCLK=24MHz & 60Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xDF); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5A); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x7A); 
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x7B); 
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F); 
    NOVATEK_WriteI2C_8Bit(0x3028, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3029, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3); 
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C); 
    NOVATEK_WriteI2C_8Bit(0x300C, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x300D, 0xE6); 
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0); 
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x31); 
    NOVATEK_WriteI2C_8Bit(0x32BB, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x38); 
    NOVATEK_WriteI2C_8Bit(0x32BD, 0x3C); 
    NOVATEK_WriteI2C_8Bit(0x32BE, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06); 
	NOVATEK_WriteI2C_8Bit(0x3060, 0x01);

	NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x5F); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x5F); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xBA); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x5F); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x7F); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x7F); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x7F); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80); 
	NOVATEK_WriteI2C_8Bit(0x32DB, 0x77);
}

static void
_NOVATEK_NT99141_SMPTE_25fps_60Hz(
    void)
{
	//[YUYV_1280x720_25.00_25.01_Fps]    //MCLK=24MHz & 60Hz
    //////////////////////////////////////// start up////////////////////////////////////////
    NOVATEK_WriteI2C_8Bit(0x320A, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xC1); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32DB, 0x78); 
    NOVATEK_WriteI2C_8Bit(0x3200, 0x3E); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x0F); 
    NOVATEK_WriteI2C_8Bit(0x3028, 0x1F); 
    NOVATEK_WriteI2C_8Bit(0x3029, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x302A, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3022, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x3023, 0x24); 
    NOVATEK_WriteI2C_8Bit(0x3002, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3003, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3004, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3005, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3006, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3007, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3008, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3009, 0xD3); 
    NOVATEK_WriteI2C_8Bit(0x300A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x300B, 0x7C); 
    NOVATEK_WriteI2C_8Bit(0x300C, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x300D, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x300E, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x300F, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3010, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3011, 0xD0); 
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x31); 
    NOVATEK_WriteI2C_8Bit(0x32BB, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x38); 
    NOVATEK_WriteI2C_8Bit(0x32BD, 0x3C); 
    NOVATEK_WriteI2C_8Bit(0x32BE, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x3201, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x3021, 0x06); 
	NOVATEK_WriteI2C_8Bit(0x3060, 0x01);

	NOVATEK_WriteI2C_8Bit(0x32BF, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x32C0, 0x63); 
    NOVATEK_WriteI2C_8Bit(0x32C1, 0x63); 
    NOVATEK_WriteI2C_8Bit(0x32C2, 0x63); 
    NOVATEK_WriteI2C_8Bit(0x32C3, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C4, 0x27); 
    NOVATEK_WriteI2C_8Bit(0x32C5, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C6, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x32C7, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32C8, 0xA1); 
    NOVATEK_WriteI2C_8Bit(0x32C9, 0x63); 
    NOVATEK_WriteI2C_8Bit(0x32CA, 0x83); 
    NOVATEK_WriteI2C_8Bit(0x32CB, 0x83); 
    NOVATEK_WriteI2C_8Bit(0x32CC, 0x83); 
    NOVATEK_WriteI2C_8Bit(0x32CD, 0x83); 
	NOVATEK_WriteI2C_8Bit(0x32DB, 0x74);
    
}

static void
_NOVATEK_NT99141_Internal_Rom_Setting(
    void)
{
    NOVATEK_WriteI2C_8Bit(0x3069, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x306A, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x320A, 0xB2); 
    NOVATEK_WriteI2C_8Bit(0x3109, 0x84); 
    NOVATEK_WriteI2C_8Bit(0x3040, 0x04); 
    NOVATEK_WriteI2C_8Bit(0x3041, 0x02); 
    NOVATEK_WriteI2C_8Bit(0x3055, 0x40); 
    NOVATEK_WriteI2C_8Bit(0x3054, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3042, 0xFF); 
    NOVATEK_WriteI2C_8Bit(0x3043, 0x08); 
    NOVATEK_WriteI2C_8Bit(0x3052, 0xE0); 
    NOVATEK_WriteI2C_8Bit(0x305F, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x3100, 0x07); 
    NOVATEK_WriteI2C_8Bit(0x3106, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3105, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x3108, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3110, 0x22); 
    NOVATEK_WriteI2C_8Bit(0x3111, 0x57); 
    NOVATEK_WriteI2C_8Bit(0x3112, 0x22); 
    NOVATEK_WriteI2C_8Bit(0x3113, 0x55); 
    NOVATEK_WriteI2C_8Bit(0x3114, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3135, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32F0, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3290, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x3291, 0x88); 
    NOVATEK_WriteI2C_8Bit(0x3296, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x3297, 0x71); 
    NOVATEK_WriteI2C_8Bit(0x3250, 0xC0); 
    NOVATEK_WriteI2C_8Bit(0x3251, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3252, 0xDF); 
    NOVATEK_WriteI2C_8Bit(0x3253, 0x95); 
    NOVATEK_WriteI2C_8Bit(0x3254, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3255, 0xCB); 
    NOVATEK_WriteI2C_8Bit(0x3256, 0x8A); 
    NOVATEK_WriteI2C_8Bit(0x3257, 0x40); 
    NOVATEK_WriteI2C_8Bit(0x3258, 0x0A); 
    NOVATEK_WriteI2C_8Bit(0x329B, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x32A1, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32A2, 0xA0); 
    NOVATEK_WriteI2C_8Bit(0x32A3, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x32A4, 0xC8); 
    NOVATEK_WriteI2C_8Bit(0x32A5, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x32A6, 0x28); 
    NOVATEK_WriteI2C_8Bit(0x32A7, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x32A8, 0xFC); 
    NOVATEK_WriteI2C_8Bit(0x32A9, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x32B0, 0x14); 
    NOVATEK_WriteI2C_8Bit(0x32B1, 0x14); 
	NOVATEK_WriteI2C_8Bit(0x32B2, 0xBE); 
    NOVATEK_WriteI2C_8Bit(0x32B3, 0x55); 
    NOVATEK_WriteI2C_8Bit(0x3210, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x3211, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x3212, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x3213, 0x11); 
    NOVATEK_WriteI2C_8Bit(0x3214, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x3215, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x3216, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x3217, 0x10); 	
    NOVATEK_WriteI2C_8Bit(0x3218, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x3219, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x321A, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x321B, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x321C, 0x0f); 
    NOVATEK_WriteI2C_8Bit(0x321D, 0x0f); 
	NOVATEK_WriteI2C_8Bit(0x321E, 0x0f); 
    NOVATEK_WriteI2C_8Bit(0x321F, 0x0f); 
    NOVATEK_WriteI2C_8Bit(0x3231, 0x68); 
    NOVATEK_WriteI2C_8Bit(0x3232, 0xC4); 
    NOVATEK_WriteI2C_8Bit(0x3270, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x3271, 0x12); 
    NOVATEK_WriteI2C_8Bit(0x3272, 0x1e); 
    NOVATEK_WriteI2C_8Bit(0x3273, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x3274, 0x48); 	
    NOVATEK_WriteI2C_8Bit(0x3275, 0x59); 
	NOVATEK_WriteI2C_8Bit(0x3276, 0x74); 
    NOVATEK_WriteI2C_8Bit(0x3277, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x3278, 0x98); 
    NOVATEK_WriteI2C_8Bit(0x3279, 0xa5); 
    NOVATEK_WriteI2C_8Bit(0x327A, 0xbc); 
    NOVATEK_WriteI2C_8Bit(0x327B, 0xd0); 
    NOVATEK_WriteI2C_8Bit(0x327C, 0xE4); 
    NOVATEK_WriteI2C_8Bit(0x327D, 0xF5); 
    NOVATEK_WriteI2C_8Bit(0x327E, 0xFF); 
    NOVATEK_WriteI2C_8Bit(0x3302, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3303, 0x40); 
    NOVATEK_WriteI2C_8Bit(0x3304, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3305, 0x96); 
    NOVATEK_WriteI2C_8Bit(0x3306, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3307, 0x29); 
    NOVATEK_WriteI2C_8Bit(0x3308, 0x07); 
	NOVATEK_WriteI2C_8Bit(0x3309, 0xBA); 
	NOVATEK_WriteI2C_8Bit(0x330A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x330B, 0xF5); 
    NOVATEK_WriteI2C_8Bit(0x330C, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x330D, 0x51); 
    NOVATEK_WriteI2C_8Bit(0x330E, 0x01); 
    NOVATEK_WriteI2C_8Bit(0x330F, 0x30); 
    NOVATEK_WriteI2C_8Bit(0x3310, 0x07); 
    NOVATEK_WriteI2C_8Bit(0x3311, 0x16); 
    NOVATEK_WriteI2C_8Bit(0x3312, 0x07); 
    NOVATEK_WriteI2C_8Bit(0x3313, 0xBA); 
    NOVATEK_WriteI2C_8Bit(0x3326, 0x03); 
    NOVATEK_WriteI2C_8Bit(0x3327, 0x0A); 
    NOVATEK_WriteI2C_8Bit(0x3328, 0x0A); 
    NOVATEK_WriteI2C_8Bit(0x3329, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x332A, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x332B, 0x1C); 
	NOVATEK_WriteI2C_8Bit(0x332C, 0x1C); 
	NOVATEK_WriteI2C_8Bit(0x332D, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x332E, 0x1D); 
    NOVATEK_WriteI2C_8Bit(0x332F, 0x1F); 
    NOVATEK_WriteI2C_8Bit(0x32F6, 0xCF); 
    NOVATEK_WriteI2C_8Bit(0x32F9, 0x21); 
    NOVATEK_WriteI2C_8Bit(0x32FA, 0x12); 
    NOVATEK_WriteI2C_8Bit(0x3325, 0x5F); 
    NOVATEK_WriteI2C_8Bit(0x3330, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x3331, 0x05); 
    NOVATEK_WriteI2C_8Bit(0x3332, 0xdc); 
    NOVATEK_WriteI2C_8Bit(0x3338, 0x08); 
    NOVATEK_WriteI2C_8Bit(0x3339, 0x63); 
    NOVATEK_WriteI2C_8Bit(0x333A, 0x36); 
    NOVATEK_WriteI2C_8Bit(0x333F, 0x07); 
    NOVATEK_WriteI2C_8Bit(0x3360, 0x0A); 
    NOVATEK_WriteI2C_8Bit(0x3361, 0x14); 
	NOVATEK_WriteI2C_8Bit(0x3362, 0x1F); 
	NOVATEK_WriteI2C_8Bit(0x3363, 0x37); 
    NOVATEK_WriteI2C_8Bit(0x3364, 0x98); 
    NOVATEK_WriteI2C_8Bit(0x3365, 0x88); 
    NOVATEK_WriteI2C_8Bit(0x3366, 0x78); 
    NOVATEK_WriteI2C_8Bit(0x3367, 0x60); 
    NOVATEK_WriteI2C_8Bit(0x3368, 0x90); 
    NOVATEK_WriteI2C_8Bit(0x3369, 0x70); 
    NOVATEK_WriteI2C_8Bit(0x336A, 0x50); 
    NOVATEK_WriteI2C_8Bit(0x336B, 0x40); 
    NOVATEK_WriteI2C_8Bit(0x336C, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x336D, 0x20); 
    NOVATEK_WriteI2C_8Bit(0x336E, 0x1C); 
    NOVATEK_WriteI2C_8Bit(0x336F, 0x18); 
    NOVATEK_WriteI2C_8Bit(0x3370, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x3371, 0x38); 
    NOVATEK_WriteI2C_8Bit(0x3372, 0x3C); 
	NOVATEK_WriteI2C_8Bit(0x3373, 0x3F); 
	NOVATEK_WriteI2C_8Bit(0x3374, 0x3F); 
    NOVATEK_WriteI2C_8Bit(0x3375, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x3376, 0x06); 
    NOVATEK_WriteI2C_8Bit(0x3377, 0x08); 
    NOVATEK_WriteI2C_8Bit(0x3378, 0x0A);
	NOVATEK_WriteI2C_8Bit(0x338A, 0x34); 
    NOVATEK_WriteI2C_8Bit(0x338B, 0x7F);
	NOVATEK_WriteI2C_8Bit(0x338C, 0x10); 
    NOVATEK_WriteI2C_8Bit(0x338D, 0x23); 
    NOVATEK_WriteI2C_8Bit(0x338E, 0x7F); 
    NOVATEK_WriteI2C_8Bit(0x338F, 0x14); 
	NOVATEK_WriteI2C_8Bit(0x32C4, 0x22); 
	NOVATEK_WriteI2C_8Bit(0x3053, 0x4F); 
    NOVATEK_WriteI2C_8Bit(0x32F2, 0x80); 
    NOVATEK_WriteI2C_8Bit(0x32FC, 0x00); 
    NOVATEK_WriteI2C_8Bit(0x32B8, 0x36); 
    NOVATEK_WriteI2C_8Bit(0x32B9, 0x2A);
	NOVATEK_WriteI2C_8Bit(0x32BB, 0x87); 
    NOVATEK_WriteI2C_8Bit(0x32BC, 0x30);
	NOVATEK_WriteI2C_8Bit(0x32BD, 0x33);
	NOVATEK_WriteI2C_8Bit(0x32BE, 0x2D); 
    NOVATEK_WriteI2C_8Bit(0x3060, 0x01);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void
mmpNovaTekInitialize(
    void)
{
	gbNovatekInit = MMP_FALSE;
}

void
mmpPixelPlusGetFlickerMode(
    uint8_t *value)
{   
    *value = gFlickerMode;
};

void
mmpNovaTekSetAntiFlicker50Hz(
    void)
{
    printf("set flick 50Hz\n");
    //Cap_EnableClock();
	//set PMCLK output mode
	//ithWriteRegMaskH(0x204A, 0x0 << 13, 0x1 << 13);
	//PalSleep(5);
	//Set GPIO29 to Low for reset
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 29), (1 << 29));
    //Set GPIO29 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 29), (1 << 29));
    //Set GPIO29 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (13 * 2)), (0x3 << (13 * 2)));
	usleep(5000);
	AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 29), (1 << 29));
	usleep(5000);
    _NOVATEK_NT99141_Internal_Rom_Setting();
    _NOVATEK_NT99141_SMPTE_25fps_50Hz();
	//Cap_DisableClock();
	
    gFlickerMode = AntiFlicker_50Hz_MODE;
};

void
mmpNovaTekSetAntiFlicker60Hz(
    void)
{
	
    printf("set flick 60Hz\n");
    //Cap_EnableClock();
	//set PMCLK output mode
	//ithWriteRegMaskH(0x204A, 0x0 << 13, 0x1 << 13);
	//PalSleep(5);
	//Set GPIO30 to Low
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 29), (1 << 29));
    //Set GPIO30 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 29), (1 << 29));
    //Set GPIO30 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (13 * 2)), (0x3 << (13 * 2)));
	usleep(10000);
	AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 29), (1 << 29));
	PalSleep(5);
    _NOVATEK_NT99141_Internal_Rom_Setting();
    _NOVATEK_NT99141_SMPTE_30fps_60Hz();
	//Cap_DisableClock();
		
	gFlickerMode = AntiFlicker_60Hz_MODE;
}

void
mmpNovaTekPowerOn(
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
mmpNovaTekPowerOff(
	void)
{
	//Set GPIO30 to High
	AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 30), (1 << 30));
	usleep(10000);
	//Set GPIO30 to Low after power down
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 30), (1 << 30));
}

void
mmpNovaTekLedOn(
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
mmpNovaTekLedOff(
    void)
{
	  AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 31), (1 << 31));
}


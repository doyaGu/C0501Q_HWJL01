
#include "sensor/mmp_sensor.h"
#include "sensor/omnivision_driver.h"
#include "sensor/pixelplus_driver.h"
#include "sensor/novatek_nt99141_driver.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define SENSOR_CONFIG_NOR_SIZE              64 * 1024
#define SENSOR_CONFIG_NOR_ADDR             ((1 * 1024 * 1024) - (SENSOR_CONFIG_NOR_SIZE) - (128 * 1024))

#define SENSOR_CONFIG_HEADER_SIZE           4
#define SENSOR_CONFIG_PAYLOAD_SIZE          SENSOR_CONFIG_NOR_SIZE - SENSOR_CONFIG_HEADER_SIZE

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
//static MMP_UINT32
//_SensorGetConfigLength(
//    void)
//{
//    MMP_RESULT  norResult  = MMP_RESULT_SUCCESS;
//    MMP_UINT32  headerNorAddr = SENSOR_CONFIG_NOR_ADDR;
//    MMP_UINT8   *headerData = MMP_NULL;
//    MMP_UINT32  configLen = 0;
//
//    headerData = (MMP_UINT8 *)malloc(SENSOR_CONFIG_HEADER_SIZE);
//
//    norResult = NorRead(headerData, headerNorAddr, SENSOR_CONFIG_HEADER_SIZE);
//    if (norResult != MMP_RESULT_SUCCESS)
//    {
//        printf("%s(), load sensor config header error !! (NorRead 0x%08X fail)\n", __FUNCTION__, headerNorAddr);
//        configLen = 0;
//    }
//    else
//    {
//        configLen = (headerData[3] << 8) | headerData[2];
//
//        printf("%x, %x, %x, %x\n", headerData[0], headerData[1], headerData[2], headerData[3]);
//
//        if (headerData[0] != 0xAB || headerData[1] != 0xCD)
//            configLen = 0;
//    }
//
//    free(headerData);
//
//    return configLen;
//}
//
//static MMP_BOOL
//_SensorLoadConfigPayload(
//    MMP_UINT8 *pConfigData)
//{
//    MMP_RESULT  norResult  = MMP_RESULT_SUCCESS;
//    MMP_UINT32  payloadNorAddr = SENSOR_CONFIG_NOR_ADDR + SENSOR_CONFIG_HEADER_SIZE;
//
//    norResult = NorRead(pConfigData, payloadNorAddr, SENSOR_CONFIG_PAYLOAD_SIZE);
//    if (norResult != MMP_RESULT_SUCCESS)
//    {
//        printf("%s(), load sensor config payload error !! (NorRead 0x%08X fail)\n", __FUNCTION__, payloadNorAddr);
//        return MMP_FALSE;
//    }
//
//    return MMP_TRUE;
//}

void Sensor_PowerOn(
      MMP_BOOL bEnable)
{
#if defined(SENSOR_OMNIVISION_OV7725)
    if (bEnable)     	
        mmpOmnivisionPowerOn();
    else
        mmpOmnivisionPowerOff();
#elif defined (SENSOR_PIXELPLUS_PO3100)
    if (bEnable)
        mmpPixelPlusPowerOn();
    else
     	 mmpPixelPlusPowerOff();
#elif defined (SENSOR_AR0130)
	if (bEnable)
		mmpAR0130PowerOn();
	else
		mmpAR0130PowerOff();	 
#elif defined (SENSOR_NOVATEK_NT99141)
	if (bEnable)
		mmpNovaTekPowerOn();
	else
		mmpNovaTekPowerOff();	
#endif	
}

void Sensor_LEDOn(
      MMP_BOOL bEnable)
{
#if defined(SENSOR_OMNIVISION_OV7725)
	if (bEnable)
    	mmpOmnivisionLedOn();
    else
    	mmpOmnivisionLedOff();
#elif defined (SENSOR_PIXELPLUS_PO3100)
    if (bEnable)
    	mmpPixelPlusLedOn();
    else
    	mmpPixelPlusLedOff();
#elif defined (SENSOR_AR0130)
	if (bEnable)
    	mmpAR0130LedOn();
    else
    	mmpAR0130LedOff();	
#elif defined (SENSOR_NOVATEK_NT99141)
	if (bEnable)
    	mmpNovaTekLedOn();
    else
    	mmpNovaTekLedOff();	
#endif	
}

void Sensor_SetAntiFlicker(
      MMP_SENSOR_FLICK_MODE mode)
{	      
#if defined(SENSOR_OMNIVISION_OV7725)
    switch(mode)
    {
        case SENSOR_FLICK_MODE_50HZ: // 50Hz
            mmpOmnivisionSetAntiFlicker50Hz();
            break;
        case SENSOR_FLICK_MODE_60HZ: // 60Hz
            mmpOmnivisionSetAntiFlicker60Hz();
            break;       
        default:
            mmpOmnivisionSetAntiFlicker50Hz();
            break;
    }
#elif defined (SENSOR_PIXELPLUS_PO3100)
    printf("Pixelplus flicker %d\n", mode);
    switch(mode)
    {
        case SENSOR_FLICK_MODE_50HZ: // 50Hz
            mmpPixelPlusSetAntiFlicker50Hz();
            break;
        case SENSOR_FLICK_MODE_60HZ: // 60Hz
            mmpPixelPlusSetAntiFlicker60Hz();
            break;       
        default:
            mmpPixelPlusSetAntiFlickerOff();
            break;
    }
#elif defined (SENSOR_NOVATEK_NT99141)
    printf("Novatek flicker %d\n", mode);
    switch(mode)
    {
        case SENSOR_FLICK_MODE_50HZ: // 50Hz
            mmpNovaTekSetAntiFlicker50Hz();
            break;
        case SENSOR_FLICK_MODE_60HZ: // 60Hz
            mmpNovaTekSetAntiFlicker60Hz();
            break;       
        default:
            break;
    }	
#endif            
}

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
* Sensor Initialize
*/
//=============================================================================
void
mmpSensorInitialize(
    void)
{
#if defined (SENSOR_AR0130)
	mmpAR0130PowerOn();
	mmpAR0130Initialize();
#endif

#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusPowerOn();
    mmpPixelPlusInitialize();

#if defined(CFG_SENSOR_FLICK_60HZ)    
	  Sensor_SetAntiFlicker(SENSOR_FLICK_MODE_60HZ);
#elif defined(CFG_SENSOR_FLICK_50HZ)
    Sensor_SetAntiFlicker(SENSOR_FLICK_MODE_50HZ);
#endif

    mmpPixelPlusSetMirror(MMP_TRUE, MMP_TRUE);
    mmpPixelPlusPowerOff();   
#endif

}

void
mmpSensorPowerOn(
    MMP_BOOL bEnable,
    MMP_BOOL dummy_on)
{	  
		if (bEnable)
		{			  
        Sensor_PowerOn(MMP_TRUE);
        
        if (dummy_on==MMP_FALSE)
		        Sensor_LEDOn(MMP_TRUE);

#if defined(SENSOR_OMNIVISION_OV7725) || defined(SENSOR_NOVATEK_NT99141)		 
       
#if defined(CFG_SENSOR_FLICK_60HZ)    
	      Sensor_SetAntiFlicker(SENSOR_FLICK_MODE_60HZ);
#elif defined(CFG_SENSOR_FLICK_50HZ)
    	  Sensor_SetAntiFlicker(SENSOR_FLICK_MODE_50HZ);
#endif	
	
#endif	  
		} else {
			  Sensor_PowerOn(MMP_FALSE);
			  
			  if (dummy_on==MMP_FALSE)
		        Sensor_LEDOn(MMP_FALSE);
		}		
}

void
mmpSensorSetEffectDefault(
    void)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetEffectDefault();
#endif
}

void
mmpSensorGetEffectDefault(
    MMP_UINT8 *brightness,
    MMP_UINT8 *contrast,
    MMP_UINT8 *saturation,
    MMP_UINT8 *sharpness)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusGetEffectDefault(brightness, contrast, saturation, sharpness);
#endif
}

void
mmpSensorImageMirror(
    MMP_BOOL bEnHorMirror,
    MMP_BOOL bEnVerMirror)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetMirror(bEnHorMirror, bEnVerMirror);
#endif
}

void
mmpSensorSetBrightness(
    MMP_UINT8 value)
{
    MMP_UINT8 regValue;
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetBrightness(value);
#endif
}

void
mmpSensorGetBrightness(
    MMP_UINT8 *value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusGetBrightness(value);
#endif
}

void
mmpSensorSetContrast(
    MMP_UINT8 value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetContrast(value);
#endif
}

void
mmpSensorGetContrast(
    MMP_UINT8 *value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusGetContrast(value);
#endif
}

void
mmpSensorSetSaturation(
    MMP_UINT8 value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetSaturation(value);
#endif
}

void
mmpSensorGetSaturation(
    MMP_UINT8 *value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusGetSaturation(value);
#endif
}

void
mmpSensorSetSharpness(
    MMP_UINT8 value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusSetSharpness(value);
#endif
}

void
mmpSensorGetSharpness(
    MMP_UINT8 *value)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    mmpPixelPlusGetSharpness(value);
#endif
}

MMP_SENSOR_FLICK_MODE
mmpSensorGetFlickMode(
    void)
{
    MMP_SENSOR_FLICK_MODE mode = SENSOR_FLICK_MODE_OFF;
#if defined (SENSOR_PIXELPLUS_PO3100)
    MMP_UINT8 value;
    mmpPixelPlusGetFlickerMode(&value);
    switch(value)
    {
        case 0x04: // 50Hz
            mode = SENSOR_FLICK_MODE_50HZ;
            break;
        case 0x08: // 60Hz
            mode = SENSOR_FLICK_MODE_60HZ;
            break;
        case 0x40: // auto
            mode = SENSOR_FLICK_MODE_AUTO;
            break;
        case 0x0: // off
        default:
            mode = SENSOR_FLICK_MODE_OFF;
            break;
    }
#endif
    return mode;
}

void
mmpSensorSetFlickMode(
    MMP_SENSOR_FLICK_MODE mode)
{
#if defined (SENSOR_PIXELPLUS_PO3100)
    switch(mode)
    {
        case SENSOR_FLICK_MODE_50HZ: // 50Hz
            mmpPixelPlusSetAntiFlicker50Hz();
            break;
        case SENSOR_FLICK_MODE_60HZ: // 60Hz
            mmpPixelPlusSetAntiFlicker60Hz();
            break;
        case SENSOR_FLICK_MODE_AUTO: // auto
            mmpPixelPlusSetAntiFlickerAuto();
            break;
        case SENSOR_FLICK_MODE_OFF: // off
        default:
            mmpPixelPlusSetAntiFlickerOff();
            break;
    }
#endif

#if defined (SENSOR_NOVATEK_NT99141)
    switch(mode)
    {
        case SENSOR_FLICK_MODE_50HZ: // 50Hz
            mmpNovaTekSetAntiFlicker50Hz();
            break;
        case SENSOR_FLICK_MODE_60HZ: // 60Hz
            mmpNovaTekSetAntiFlicker60Hz();
            break;
        case SENSOR_FLICK_MODE_AUTO: // auto
            break;
        case SENSOR_FLICK_MODE_OFF: // off
        default:
            break;
    }
#endif

}

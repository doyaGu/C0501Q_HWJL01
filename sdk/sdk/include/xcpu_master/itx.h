/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Caster operation API
 *
 * @version 0.1
 */
#ifndef ITX_H
#define ITX_H

#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ITX_BUS_NULL (0)
#define ITX_BUS_SPI  (1)
#define ITX_BUS_I2C  (2)
#define ITX_BUS_UART (3)
#define ITX_HOST_BOOT  (0)
#define ITX_FLASH_BOOT (1)

#define ITX_BUS_TYPE ITX_BUS_SPI
#define ITX_BOOT_TYPE ITX_HOST_BOOT

#if ITX_BOOT_TYPE == ITX_HOST_BOOT
#define BOOT_FILE_PATH   CFG_PRIVATE_DRIVE ":/jedi.rom"
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef enum ITX_INPUT_DEVICE_TAG
{
    ITX_INPUT_DEVICE_UNKNOW = 0,
    ITX_INPUT_DEVICE_CVBS,
    ITX_INPUT_DEVICE_SVIDEO,
    ITX_INPUT_DEVICE_YPbPr,
    ITX_INPUT_DEVICE_VGA,
    ITX_INPUT_DEVICE_HDMI,
    ITX_INPUT_DEVICE_SENSOR,
}ITX_INPUT_DEVICE;

typedef enum ITX_INPUT_VIDEO_INFO_TAG
{
    ITX_INPUT_VIDEO_INFO_640X480_60P = 0,
    ITX_INPUT_VIDEO_INFO_720X480_59I,
    ITX_INPUT_VIDEO_INFO_720X480_59P,
    ITX_INPUT_VIDEO_INFO_720X480_60I,
    ITX_INPUT_VIDEO_INFO_720X480_60P,
    ITX_INPUT_VIDEO_INFO_720X576_50I,
    ITX_INPUT_VIDEO_INFO_720X576_50P,
    ITX_INPUT_VIDEO_INFO_1280X720_50P,
    ITX_INPUT_VIDEO_INFO_1280X720_59P,
    ITX_INPUT_VIDEO_INFO_1280X720_60P,
    ITX_INPUT_VIDEO_INFO_1920X1080_23P,
    ITX_INPUT_VIDEO_INFO_1920X1080_24P,
    ITX_INPUT_VIDEO_INFO_1920X1080_25P,
    ITX_INPUT_VIDEO_INFO_1920X1080_29P,
    ITX_INPUT_VIDEO_INFO_1920X1080_30P,
    ITX_INPUT_VIDEO_INFO_1920X1080_50I,
    ITX_INPUT_VIDEO_INFO_1920X1080_50P,
    ITX_INPUT_VIDEO_INFO_1920X1080_59I,
    ITX_INPUT_VIDEO_INFO_1920X1080_59P,
    ITX_INPUT_VIDEO_INFO_1920X1080_60I,
    ITX_INPUT_VIDEO_INFO_1920X1080_60P,
    ITX_INPUT_VIDEO_INFO_800X600_60P,
    ITX_INPUT_VIDEO_INFO_1024X768_60P,
    ITX_INPUT_VIDEO_INFO_1280X768_60P,
    ITX_INPUT_VIDEO_INFO_1280X800_60P,
    ITX_INPUT_VIDEO_INFO_1280X960_60P,
    ITX_INPUT_VIDEO_INFO_1280X1024_60P,
    ITX_INPUT_VIDEO_INFO_1360X768_60P,
    ITX_INPUT_VIDEO_INFO_1366X768_60P,
    ITX_INPUT_VIDEO_INFO_1440X900_60P,
    ITX_INPUT_VIDEO_INFO_1400X1050_60P,
    ITX_INPUT_VIDEO_INFO_1440X1050_60P,
    ITX_INPUT_VIDEO_INFO_1600X900_60P,
    ITX_INPUT_VIDEO_INFO_1600X1200_60P,
    ITX_INPUT_VIDEO_INFO_1680X1050_60P,
    ITX_INPUT_VIDEO_INFO_ALL,
    ITX_INPUT_VIDEO_INFO_NUM,
    ITX_INPUT_VIDEO_INFO_UNKNOWN,
    ITX_INPUT_VIDEO_INFO_CAMERA
}ITX_INPUT_VIDEO_INFO;

typedef enum ITX_OUTPUT_AUD_TYPE_TAG
{
    MPEG = 0,
    AAC,
}ITX_OUTPUT_AUD_TYPE;

typedef enum ITX_STATE_TAG
{
    ITX_STATE_ENCODING,
    ITX_STATE_SIGNAL_STABLE,
    ITX_STATE_SIGNAL_UNSTABLE,
    ITX_STATE_STOP_ENCODING,
}ITX_STATE;

typedef enum ITX_CONSTELLATION_MODE_TAG
{
    ITX_CONSTELATTION_QPSK,
    ITX_CONSTELATTION_16QAM,
    ITX_CONSTELATTION_64QAM
} ITX_CONSTELLATION_MODE;

typedef enum ITX_CODE_RATE_MODE_TAG
{
    ITX_CODE_RATE_1_2,
    ITX_CODE_RATE_2_3,
    ITX_CODE_RATE_3_4,
    ITX_CODE_RATE_5_6,
    ITX_CODE_RATE_7_8
} ITX_CODE_RATE_MODE;

typedef enum ITX_GUARD_INTERVAL_MODE_TAG
{
    ITX_GUARD_INTERVAL_1_32,
    ITX_GUARD_INTERVAL_1_16,
    ITX_GUARD_INTERVAL_1_8,
    ITX_GUARD_INTERVAL_1_4
} ITX_GUARD_INTERVAL_MODE;

typedef enum ITX_COUNTRY_ID_PARA_TAG
{
    ITX_COUNTRY_TAIWAN = 0,
    ITX_COUNTRY_DENMARK,
    ITX_COUNTRY_FINLAND,
    ITX_COUNTRY_NORWAY,
    ITX_COUNTRY_SWEDEN,
    ITX_COUNTRY_GERMANY,
    ITX_COUNTRY_UK,
    ITX_COUNTRY_ITALY,
    ITX_COUNTRY_AUSTRALIA,
    ITX_COUNTRY_NEW_ZEALAND,
    ITX_COUNTRY_FRANCE,
    ITX_COUNTRY_SPAIN,
    ITX_COUNTRY_POLAND,
    ITX_COUNTRY_CZECH,
    ITX_COUNTRY_NETHERLANDS,
    ITX_COUNTRY_GREECE,
    ITX_COUNTRY_RUSSIA,
    ITX_COUNTRY_SWITZERLAND,
    ITX_COUNTRY_SLOVAK,
    ITX_COUNTRY_SLOVENIA,
    ITX_COUNTRY_HUNGARY,
    ITX_COUNTRY_AUSTRIA,
    ITX_COUNTRY_LATIVA,
    ITX_COUNTRY_ISRAEL,
    ITX_COUNTRY_CROATIA,
    ITX_COUNTRY_ESTONIA,
    ITX_COUNTRY_PORTUGAL,
    ITX_COUNTRY_IRELAND, //27
} ITX_COUNTRY_ID_PARA;

typedef enum ITX_TRANSMISSION_MODE_TAG
{
    ITX_TRANSMISSION_MODE_2K,
    ITX_TRANSMISSION_MODE_8K,
    ITX_TRANSMISSION_MODE_4K,
} ITX_TRANSMISSION_MODE;

typedef enum ITX_SDEVICE_BOOT_MODE_TAG
{
    ITX_SPI_BOOT,
    ITX_NOR_BOOT,
} ITX_SDEVICE_BOOT_MODE;

typedef enum SENSOR_CTRL_FLICK_MODE_TYPE_TAG
{
    SENSOR_CTRL_FLICK_MODE_AUTO,
    SENSOR_CTRL_FLICK_MODE_50HZ,
    SENSOR_CTRL_FLICK_MODE_60HZ,
    SENSOR_CTRL_FLICK_MODE_OFF
} SENSOR_CTRL_FLICK_MODE_TYPE;

//=============================================================================
//                              Structure Definition
//=============================================================================
//typedef struct ITX_RETURN_CHANNEL_PARA
//{
//    MMP_UINT16  ir_protocol;
//    MMP_UINT32  ir_key;
//}ITX_RETURN_CHANNEL_PARA;

typedef struct ITX_VERSION_TYPE
{
    MMP_UINT16  customerCode;
    MMP_UINT16  projectCode;
    MMP_UINT16  sdkMajorVersion;
    MMP_UINT16  sdkMinorVersion;
    MMP_UINT16  buildNumber;
} ITX_VERSION_TYPE;

typedef struct ITX_SYSTEM_TIME
{
    MMP_UINT16  year;
    MMP_UINT16  month;
    MMP_UINT16  day;
    MMP_UINT16  hour;
    MMP_UINT16  min;
    MMP_UINT16  sec;
} ITX_SYSTEM_TIME;

typedef struct ITX_MODULATOR_PARA
{
    MMP_UINT32               frequency;
    MMP_UINT16               bandwidth;
    ITX_CONSTELLATION_MODE   constellation;
    ITX_CODE_RATE_MODE       codeRate;
    ITX_GUARD_INTERVAL_MODE  guardInterval;
    ITX_TRANSMISSION_MODE    transmissionMode;
} ITX_MODULATOR_PARA;

typedef struct ITX_NETWORK_NAME_PARA
{
    MMP_UINT8  networkName[255];
    MMP_UINT16 nameLen;
}ITX_NETWORK_NAME_PARA;

typedef struct ITX_SERVICE_LIST_PARA
{
    MMP_UINT16 serviceId;
    MMP_UINT16 serviceType;
}ITX_SERVICE_LIST_PARA;

typedef struct ITX_SERVICE_LCN_PARA
{
    MMP_UINT16 serviceId;
    MMP_UINT16 lcn;
}ITX_SERVICE_LCN_PARA;

typedef struct ITX_SERVICE_NAME_PARA
{
    MMP_UINT16 serviceId;
    MMP_UINT8  serviceName[255];
    MMP_UINT16 serviceNameLen;
    MMP_UINT8  serviceProviderName[255];
    MMP_UINT16 serviceProviderNameLen;
}ITX_SERVICE_NAME_PARA;

typedef struct ITX_SERVICE_ID_PARA
{
    MMP_UINT16 serviceIndex;
    MMP_UINT16 serviceId;
}ITX_SERVICE_ID_PARA;

typedef struct ITX_SERVICE_PMT_PID_PARA
{
    MMP_UINT16 serviceId;
    MMP_UINT16 pmtPid;
}ITX_SERVICE_PMT_PID_PARA;

typedef struct ITX_SERVICE_ES_INFO_PARA
{
    MMP_UINT16 serviceId;
    MMP_UINT16 videoPid;
    MMP_UINT16 audioPid;
}ITX_SERVICE_ES_INFO_PARA;

typedef struct SENSOR_IMAGE_EFFECT_TAG
{
    MMP_UINT8          brightness;
    MMP_UINT8          contrast;
    MMP_UINT8          saturation;
    MMP_UINT8          edgeEnhancement;
} SENSOR_IMAGE_EFFECT;

typedef struct SENSOR_IMAGE_MIRROR_TAG
{
    MMP_BOOL            enHorMirror;        //Horizontal Mirror On/Off  default : OFF
    MMP_BOOL            enVerMirror;        //Vertical Mirror On/Off    default : OFF
} SENSOR_IMAGE_MIRROR;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Boot-up ITX slave device
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
itxBootup(
    ITX_SDEVICE_BOOT_MODE boot_mode);

//=============================================================================
/**
 * Shutdown ITX slave device
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
itxShutdown(
    void);

MMP_RESULT
itxTestBus(
    void);

MMP_RESULT
itxEncoderParaUpdate(
    void);
    
MMP_RESULT
itxEncStart(
    void);

MMP_RESULT
itxEncStop(
    void);

MMP_RESULT
itxEncSetStream(
    uint8_t streamId);
    
MMP_RESULT
itxJPGRecord(
    uint32_t quality);

MMP_RESULT
itxJPGStop(
    void);
    
MMP_RESULT
itxCamPWon(
    uint8_t streamId);
    
MMP_RESULT
itxCamStandBy(
    void);
            
MMP_RESULT
itxGetInputDevice(
    ITX_INPUT_DEVICE* pDevice);

MMP_RESULT
itxSetInputDevice(
    ITX_INPUT_DEVICE  pdevice);

MMP_RESULT
itxGetInputVideoInfo(
    ITX_INPUT_VIDEO_INFO* pInfo);

MMP_RESULT
itxGetOutputEncRes(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          enc_width,
    MMP_UINT16*          enc_height,
    MMP_UINT16*          enc_deinterlace);

MMP_RESULT
itxSetOutputEncRes(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16           enc_width,
    MMP_UINT16           enc_height,
    MMP_UINT16           streamId);
    
MMP_RESULT
itxGetOutputFrameRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          pFrmRate);

MMP_RESULT
itxSetOutputFrameRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           frameRate);

MMP_RESULT
itxGetMaxEnFrameRate(
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16           enWidth,
    MMP_UINT16           enHeight,
    MMP_UINT16*          pFrmRate);

MMP_RESULT
itxGetOutputBitRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          bitrate);

MMP_RESULT
itxSetOutputBitRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           bitrate,
    MMP_UINT16           streamId);

MMP_RESULT
itxGetIFramePeriod(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          Ifmperiod);

MMP_RESULT
itxSetIFramePeriod(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           Ifmperiod);

MMP_RESULT
itxSetDeinterlaceOn(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_BOOL             bInterlaceOn);

MMP_RESULT
itxSetOutputAudType(
    ITX_OUTPUT_AUD_TYPE   btype);

MMP_RESULT
itxGetOutputAudBitRate(
    MMP_UINT16*   bitrate);

MMP_RESULT
itxSetOutputAudBitRate(
    MMP_UINT16   bitrate);

MMP_RESULT
itxGetTime(
    ITX_SYSTEM_TIME* time);

MMP_RESULT
itxSetTime(
    ITX_SYSTEM_TIME time);

MMP_RESULT
itxGetVersion(
    ITX_VERSION_TYPE* version);

MMP_RESULT
itxGetState(
    ITX_STATE* pState);

MMP_RESULT
itxSendReturnChannelIRKey(
    MMP_UINT16 size);

MMP_RESULT
itxGetModulatorParameter(
    ITX_MODULATOR_PARA* pModulatorPara);

MMP_RESULT
itxSetModulatorParameter(
    ITX_MODULATOR_PARA modulatorPara);

MMP_RESULT
itxTsUpdateTransportStreamId(
    MMP_UINT16 transportStreamId);

MMP_RESULT
itxTsUpdateNetworkName(
    ITX_NETWORK_NAME_PARA* pNetworkName);

MMP_RESULT
itxTsUpdateNetworkId(
    MMP_UINT16 networkId);

MMP_RESULT
itxTsUpdateOriginalNetworkId(
    MMP_UINT16 originalNetworkId);

MMP_RESULT
itxTsUpdateServiceListDescriptor(
    ITX_SERVICE_LIST_PARA* pServiceList);

MMP_RESULT
itxTsUpdateCountryId(
    ITX_COUNTRY_ID_PARA countryId);

MMP_RESULT
itxTsUpdateLCN(
    ITX_SERVICE_LCN_PARA* pServiceLCN);

MMP_RESULT
itxTsUpdateServiceName(
    ITX_SERVICE_NAME_PARA* pServiceName);

MMP_RESULT
itxTsGetServiceCount(
    MMP_UINT16* pServcieCount);

MMP_RESULT
itxTsUpdateServiceId(
    ITX_SERVICE_ID_PARA* pServiceId);

MMP_RESULT
itxTsUpdateServicePmtPid(
    ITX_SERVICE_PMT_PID_PARA* pServicePmtPid);

MMP_RESULT
itxTsUpdateServiceEsPid(
    ITX_SERVICE_ES_INFO_PARA* pServiceEsInfo);

MMP_RESULT
itxSetSensorFlickMode(
    SENSOR_CTRL_FLICK_MODE_TYPE flickMode);

MMP_RESULT
itxSetSensorImageMirror(
    SENSOR_IMAGE_MIRROR pCtrl);

MMP_RESULT
itxSetSensorImageEffect(
    SENSOR_IMAGE_EFFECT pCtrl);

#ifdef __cplusplus
}
#endif

#endif /* ITX_H */

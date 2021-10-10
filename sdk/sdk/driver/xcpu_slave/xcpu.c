/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file xcpu.c
 * Used to do xcpu operation
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "xcpu.h"
#include "xcpu_msgq.h"
//#include "info_mgr.h"
#include "xcpu_api.h"
//#include "mmp_aud.h"
//#include "mmp_hdmirx.h"
//#include "mmp_capture.h"
//#include "core_interface.h"
//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct CUSTOM_DATA_TAG
{
	int  header;
	char data[16];
}CUSTOM_DATA;

//=============================================================================
//                              Extern Reference
//=============================================================================

void
xCpuMsgQ_SendBootOK(
    void);

//MMP_RESULT
//smtkFileForceStop(
//    void);

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 *
 *
 * @return  0 if they succeed, and a failure code otherwise.
 */
//=============================================================================
MMP_RESULT
xCpu_Init(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    sysMsgQ_Init(SYS_MSGQ_ID_CMD);
	xCpuMsgQ_Init();
	xCpuMsgQ_InitStandaloneFlag();
	
    return result;
}

//=============================================================================
/**
 *
 *
 * @return  0 if they succeed, and a failure code otherwise.
 */
//=============================================================================
MMP_RESULT
xCpu_Terminate(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    xCpuMsgQ_Terminate();
    sysMsgQ_Terminate(SYS_MSGQ_ID_CMD);

    return result;
}

//=============================================================================
/**
* Wait for load configuration files(config.cfg, service.cfg, custom.cfg) from Host complete.
*
* @return  0 if they succeed, and a failure code otherwise.
*/
//=============================================================================
MMP_RESULT
XCpu_WaitHostFileReady(
    void)
{
    MMP_RESULT    result = MMP_RESULT_ERROR;
    PAL_CLOCK_T   waitTime = PalGetClock();
    SYS_MSG_OBJ  tMToSMsg = {0};

    do
    {
        // ToDo: timeout mechanism
        PalSleep(1);
        if (QUEUE_NO_ERROR
            == sysMsgQ_CheckMessage(SYS_MSGQ_ID_FILE, &tMToSMsg))
        {
            if ((SYS_MSG_ID_FHOST_READY == tMToSMsg.id) &&
                (QUEUE_NO_ERROR == sysMsgQ_ReceiveMessage(SYS_MSGQ_ID_FILE, &tMToSMsg)))
            {
                SYS_MSG_OBJ     tSToMMsg = {0};
                tSToMMsg.type    = SYS_MSG_TYPE_FILE;
                tSToMMsg.id      = SYS_MSG_ID_FHOST_READY_RET;

                xCpuMsgQ_SendMessage(&tSToMMsg);
                result = MMP_SUCCESS;
                break;
            }
        }
    } while( PalGetDuration(waitTime) <= 60000);

    return result;
}

void
xCpu_ProcessCmdMsg(
    void)
{
    SYS_MSG_OBJ tMToSMsg = {0};
    
    // read file command
    while (QUEUE_NO_ERROR == sysMsgQ_ReceiveMessage(SYS_MSGQ_ID_CMD, &tMToSMsg))
    {
		switch( tMToSMsg.id )
		{
		case SYS_MSG_ID_CUSTOM_DATA:
			{
				CUSTOM_DATA* pData = tMToSMsg.msg;
				
				printf("[SLAVE RECV] Header(%d), %s\n", pData->header, pData->data);
			}
			break;

		default:
			printf("tMToSMsg.id = %d\n", tMToSMsg.id);
			break;
		}
    
	#if 0
        switch (tMToSMsg.id)
        {		
        case SYS_MSG_ID_SHUTDOWN:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SHUTDOWN_RET;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_START_ENCODER:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_START_ENCODER_RET;
                corePlay();
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_STOP_ENCODER:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_STOP_ENCODER_RET;
                coreStop();
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_INPUT_VIDEO_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE_RET;
                coreGetInputVideoInfo(&info);
                tSToMMsg.msg[0] = info.src_device;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_INPUT_VIDEO_INFO:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                INPUT_VIDEO_INFO videoInfo = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_INPUT_VIDEO_INFO_RET;
                xCpuAPI_GetInputVideoInfo(&videoInfo);
                tSToMMsg.msg[0] = videoInfo;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION_RET;
                coreGetVideoEncoderInfo(&info);
                tSToMMsg.msg[0] = info.dest_width;
                tSToMMsg.msg[1] = info.dest_height;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION_RET;
                coreGetVideoEncoderInfo(&info);
                info.dest_width = tMToSMsg.msg[0];
                info.dest_height = tMToSMsg.msg[1];
                coreSetVideoEncoderInfo(info);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE_RET;
                coreGetVideoEncoderInfo(&info);
                tSToMMsg.msg[0] = info.frameRate;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE_RET;
                coreGetVideoEncoderInfo(&info);
                info.skipMode = tMToSMsg.msg[0];
                coreSetVideoEncoderInfo(info);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE_RET;
                coreGetVideoEncoderInfo(&info);
                tSToMMsg.msg[0] = info.bitrate;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE_RET;
                coreGetVideoEncoderInfo(&info);
                info.bitrate = tMToSMsg.msg[0];
                coreSetVideoEncoderInfo(info);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD_RET;
                coreGetVideoEncoderInfo(&info);
                tSToMMsg.msg[0] = info.gopSize;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD_RET;
                coreGetVideoEncoderInfo(&info);
                info.gopSize = tMToSMsg.msg[0];
                coreSetVideoEncoderInfo(info);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        //case SYS_MSG_ID_GET_DEINTERLACE:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_GET_DEINTERLACE_RET;
        //        tSToMMsg.msg[0] = coreGetDeinterlace();
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        case SYS_MSG_ID_SET_DEINTERLACE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VIDEO_ENCODE_INFO info = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_DEINTERLACE_RET;
                coreGetVideoEncoderInfo(&info);
                info.bdeinterlaceOn = tMToSMsg.msg[0];
                coreSetVideoEncoderInfo(info);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        //case SYS_MSG_ID_GET_AUDIO_CODEC_TYPE:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_GET_AUDIO_CODEC_TYPE_RET;
        //        tSToMMsg.msg[0] = 0;
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        //case SYS_MSG_ID_SET_AUDIO_CODEC_TYPE:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_SET_AUDIO_CODEC_TYPE_RET;
        //        //tMToSMsg.msg[0];
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        //case SYS_MSG_ID_GET_AUDIO_SAMPLE_RATE:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_GET_AUDIO_SAMPLE_RATE_RET;
        //        tSToMMsg.msg[0] = mmpHDMIRXGetProperty(HDMIRX_AUDIO_SAMPLERATE);
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        //case SYS_MSG_ID_SET_AUDIO_SAMPLE_RATE:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_SET_AUDIO_SAMPLE_RATE_RET;
        //        mmpHDMIRXSetProperty(HDMIRX_AUDIO_SAMPLERATE, tMToSMsg.msg[0]);
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        case SYS_MSG_ID_GET_AUDIO_BIT_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_AUDIO_BIT_RATE_RET;
                //tSToMMsg.msg[0] = taskAudioIn_GetBitRate();
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_AUDIO_BIT_RATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_AUDIO_BIT_RATE_RET;
                //taskAudioIn_SetBitRate(tMToSMsg.msg[0]);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        //case SYS_MSG_ID_GET_INPUT_SOURCE_CAPABILITY:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_GET_INPUT_SOURCE_CAPABILITY_RET;
        //        tSToMMsg.msg[0] = 0;
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        //case SYS_MSG_ID_SET_INPUT_SOURCE_CAPABILITY:
        //    {
        //        XCPU_MSG_OBJ tSToMMsg   = {0};
        //        tSToMMsg.type   = SYS_MSG_TYPE_CMD;
        //        tSToMMsg.id     = SYS_MSG_ID_SET_INPUT_SOURCE_CAPABILITY_RET;
        //        //tMToSMsg.msg[0];
        //        xCpuMsgQ_SendMessage(&tSToMMsg);
        //    }
        //    break;

        case SYS_MSG_ID_GET_TIME:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                MMP_UINT year = 0;
                MMP_UINT month = 0;
                MMP_UINT day = 0;
                MMP_UINT8 hour = 0;
                MMP_UINT8 min = 0;
                MMP_UINT8 sec = 0;

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_TIME_RET;
                coreRtcGetDate(&year, &month, &day);
                coreRtcGetTime(&hour, &min, &sec);
                tSToMMsg.msg[0] = year;
                tSToMMsg.msg[1] = month;
                tSToMMsg.msg[2] = day;
                tSToMMsg.msg[3] = hour;
                tSToMMsg.msg[4] = min;
                tSToMMsg.msg[5] = sec;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_SET_TIME:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_TIME_RET;
                coreRtcSetDateTime(tMToSMsg.msg[0],tMToSMsg.msg[1],tMToSMsg.msg[2],tMToSMsg.msg[3],tMToSMsg.msg[4],tMToSMsg.msg[5]);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_FW_VERSION:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_VERSION_TYPE version = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_FW_VERSION_RET;
                coreGetVersion(&version);
                tSToMMsg.msg[0] = version.customerCode;
                tSToMMsg.msg[1] = version.projectCode; 
                tSToMMsg.msg[2] = version.sdkMajorVersion;
                tSToMMsg.msg[3] = version.sdkMinorVersion;
                tSToMMsg.msg[4] = version.buildNumber;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_STATE:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_STATE_RET;
                tSToMMsg.msg[0] = 0;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
            break;

        case SYS_MSG_ID_GET_MODULATOR_PARA:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_MODULATOR_PARA modulatorPara = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_GET_MODULATOR_PARA_RET;
                coreGetModulatorPara(&modulatorPara);
                tSToMMsg.msg[0] = modulatorPara.frequency;
                tSToMMsg.msg[1] = modulatorPara.bandwidth; 
                tSToMMsg.msg[2] = modulatorPara.constellation;
                tSToMMsg.msg[3] = modulatorPara.codeRate;
                tSToMMsg.msg[4] = modulatorPara.quardInterval;
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
        case SYS_MSG_ID_SET_MODULATOR_PARA:
            {
                XCPU_MSG_OBJ tSToMMsg   = {0};
                CORE_MODULATOR_PARA modulatorPara = {0};

                tSToMMsg.type   = SYS_MSG_TYPE_CMD;
                tSToMMsg.id     = SYS_MSG_ID_SET_MODULATOR_PARA_RET;
                modulatorPara.frequency     = tMToSMsg.msg[0];
                modulatorPara.bandwidth     = tMToSMsg.msg[1];
                modulatorPara.constellation = tMToSMsg.msg[2];
                modulatorPara.codeRate      = tMToSMsg.msg[3];
                modulatorPara.quardInterval = tMToSMsg.msg[4];
                coreSetModulatorPara(modulatorPara);
                xCpuMsgQ_SendMessage(&tSToMMsg);
            }
        }
		#endif
    }
}

void
xCpu_SendBootOK(
    void)
{
    xCpuMsgQ_SendBootOK();
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//#endif

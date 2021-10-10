
#include <semaphore.h>
#include <pthread.h>
#include "video_encoder/video_encoder_it9910.h" 

//=============================================================================
//                Constant Definition
//=============================================================================

//#define CFG_DUAL_STREAM
//#define TEST_MULTISTREAM

#define CAPTURE_RESULT_QUEUE_SIZE   15
#define ENCODED_RESULT_QUEUE_SIZE   CAPTURE_RESULT_QUEUE_SIZE
#define QUEUE_EMPTY                 (-1)
#define QUEUE_NOT_EMPTY             (0)
#define TIMER_NUM                   4
#if defined(CFG_DUAL_STREAM)

#define ISP_TRANSFORM_NUM           2
#define ENCODE_STREAM_NUM           2
#define VIDEO_STREAM_NUM            2
#define VIDEO_SAMPLE_NUM            7

#else

#define ISP_TRANSFORM_NUM           1
#define ENCODE_STREAM_NUM           1
#define VIDEO_STREAM_NUM            1
#define VIDEO_SAMPLE_NUM            7

#endif

//#define VIDEO_SAMPLE_SIZE           500 * 1024
//#define VIDEO_SRC_WIDTH             SENSOR_WIDTH
//#define VIDEO_SRC_HEIGHT            SENSOR_HEIGHT
//#define VIDEO_BIT_RATE              SENSOR_BITRATE
//#define VIDEO_SRC_WIDTH_2           SENSOR_WIDTH_2
//#define VIDEO_SRC_HEIGHT_2          SENSOR_HEIGHT_2
//#define VIDEO_BIT_RATE_2            SENSOR_BITRATE_2

#define VIDEO_FRAME_RATE_30         30
#define VIDEO_FRAME_RATE_25         25

#if defined (CFG_SENSOR_FLICK_50HZ)
#define    VIDEO_FRAME_RATE         VIDEO_FRAME_RATE_25
#else
#define    VIDEO_FRAME_RATE         VIDEO_FRAME_RATE_30
#endif


//=============================================================================
//                Macro Definition
//=============================================================================
typedef MMP_UINT (*frameCount2TimeStamp)(MMP_UINT framecount);

//=============================================================================
//                Structure Definition
//=============================================================================

typedef enum ENCODER_TYPE_TAG
{
    JPEG_ENCODER,
    H264_ENCODER   
} ENCODER_TYPE;

typedef struct CAPTURE_RESULT_TAG
{
    MMP_UINT        resIdx;
    MMP_UINT        frameCount;
    MMP_UINT        timeStamp;
    MMP_UINT        frameIndex;
    MMP_BOOL        bTopField;
    MMP_BOOL        bCapToTran;
    MMP_BOOL        bTranToEnc;
} CAPTURE_RESULT;

typedef MMP_RESULT (*doEncodeDequeue)(CAPTURE_RESULT *ptEntry);

typedef struct AVC_ENCODER_FLOW_TAG
{
    MMP_UINT                    baseTimeStamp;
    MMP_UINT                    currTimeStamp;
    MMP_UINT                    captureFrameCount;
    MMP_UINT                    encodedFrameCount;
    MMP_UINT                    encodedTimeStamp;
    MMP_CAP_FRAMERATE           frameRate;
    MMP_CAP_FRAMERATE           captureFrameRate;
    MMP_CAP_FRAMERATE           detectFrameRate;
//    FRAME_RATE_SETTING_STATE    frameRateState;
    frameCount2TimeStamp        pfToTimeStamp;
    doEncodeDequeue             pfEnDequeue;
//    VIDEO_ENCODER_INPUT_INFO    encoderInfo;
    MMP_BOOL                    bSkipFrame;
    MMP_UINT                    ispFrameRate[ENCODE_STREAM_NUM];
    MMP_UINT                    frameRateDiff[ENCODE_STREAM_NUM];
    MMP_UINT                    frameCount[ENCODE_STREAM_NUM];
    MMP_UINT                    skipCount[ENCODE_STREAM_NUM];
    MMP_UINT                    frmRateChkCount;
    MMP_BOOL                    encodeQue[5];
    MMP_UINT                    maxFrmRateIdx;
} AVC_ENCODER_FLOW;

typedef struct CAPTURE_RESULT_QUEUE_TAG
{
    CAPTURE_RESULT  entry[CAPTURE_RESULT_QUEUE_SIZE];
    MMP_UINT        wIdx;
    MMP_UINT        rIdx;
} CAPTURE_RESULT_QUEUE;

typedef struct ENCODED_RESULT_TAG
{
    MMP_UINT        frameCount;
    MMP_UINT        timeStamp;
    MMP_UINT8*      pData;
    MMP_UINT        dataSize;
    MMP_UINT        InstanceNum;
} ENCODED_RESULT;

typedef struct ENCODED_RESULT_QUEUE_TAG
{
    ENCODED_RESULT  entry[ENCODED_RESULT_QUEUE_SIZE];
    MMP_UINT        wIdx;
    MMP_UINT        rIdx;
} ENCODED_RESULT_QUEUE;

/**
 *  Device Select
 */
typedef enum CAPTURE_DEVICE_TAG
{
    CAPTURE_DEV_UNKNOW      = MMP_CAP_UNKNOW_DEVICE,
    CAPTURE_DEV_ADV7180     = MMP_CAP_DEV_ADV7180,
    CAPTURE_DEV_CAT9883     = MMP_CAP_DEV_CAT9883,
    CAPTURE_DEV_HDMIRX      = MMP_CAP_DEV_HDMIRX,
    CAPTURE_DEV_SENSOR      = MMP_CAP_DEV_SENSOR,
} CAPTURE_DEVICE;

typedef struct ISP_TRANSFORM_PARAMETER_TAG
{
    MMP_UINT16      inWidth;
    MMP_UINT16      inHeight;
    MMP_UINT32      inAddrY[3];
    MMP_UINT32      inAddrUV[3];
    MMP_UINT16      outWidth;
    MMP_UINT16      outHeight;
    MMP_UINT32      outAddrY[5];
    MMP_UINT32      outAddrUV[5];
    MMP_BOOL        deinterlaceOn;
    MMP_BOOL        bframeDouble;
    MMP_BOOL        useTranBuf;
    MMP_UINT32      tranBufAddrY[3];
    MMP_UINT32      tranBufAddrUV[3];
} ISP_TRANSFORM_PARAMETER;

//=============================================================================
//                Global Data Definition
//=============================================================================

static AVC_ENCODER*                 gptAVCEncoder[ENCODE_STREAM_NUM];
static AVC_ENCODER_FLOW             gtAVCEncoderFlow;
static CAPTURE_RESULT_QUEUE         gtCaptureResultQ;
static ENCODED_RESULT_QUEUE         gtEncodedResultQ;
static CAPTURE_RESULT_QUEUE         gtIspResultQ;
static MMP_BOOL                     gbEncodeFire = MMP_FALSE;
static MMP_BOOL                     gbVideoEncoderInit = MMP_FALSE;
static MMP_UINT32                   gEncodeIdx = 0;
static MMP_BOOL                     gbStrBufFire[VIDEO_STREAM_NUM][VIDEO_SAMPLE_NUM];
static MMP_UINT32                   gVideoSelIdx[VIDEO_STREAM_NUM];
static MMP_UINT32                   gVideoWrIdx[VIDEO_STREAM_NUM];
static ENCODE_PARA_CALLBACK         gfpCallback = MMP_NULL;
static sem_t                        gpEncoderSem;
static MMP_BOOL                     gbVideoEncoder = MMP_FALSE;
static MMP_BOOL                     gbStream0Enable = MMP_FALSE;
static MMP_BOOL                     gbStream1Enable = MMP_FALSE;
static MMP_BOOL                     gbJPEGEncoder = MMP_FALSE;
static MMP_UINT32                   gJPEGBufIndex = 0;
static MMP_UINT32                   gOpenEnginCnt = 0;
static pthread_mutex_t              VideoEngineMutex  = PTHREAD_MUTEX_INITIALIZER;

static ISP_TRANSFORM_PARAMETER      gtIspTransformParm[ISP_TRANSFORM_NUM] = {0};
static CAPTURE_RESULT               gtIspFireEntry;
static MMP_UINT32                   gVideoEnWidth[2];
static MMP_UINT32                   gVideoEnHeight[2];
static MMP_UINT32                   gVideoEnBitRate[2];
static MMP_BOOL                     gStartgetVideo[VIDEO_STREAM_NUM];
static MMP_UINT8        			gVideoUserNum[VIDEO_STREAM_NUM];

MMP_UINT32 tickStart;
MMP_UINT32 tickISP;
#ifdef   LOG_ENCODER_PERFORMANCE
MMP_UINT32 tickStart, tickEnd;
unsigned   ticks_per_ms;
MMP_UINT32 Enidx = 0;
MMP_UINT32 EnTime;
#endif

//=============================================================================
//                Private Function Definition
//=============================================================================

// Capture Q function
static void
cap_isr(
    void* arg);

static void
VP_mem_isr(
    void* arg);
    
static void
VP_onfly_isr(
    void* arg);
    
static void
encoder_isr(
    void* arg);    

static MMP_BOOL
_Chk_Skip_Frame(
    MMP_UINT32 InstanceNum);
        
static MMP_INLINE void
_CaptureResultQ_Reset(
    void)
{
    gtCaptureResultQ.wIdx = gtCaptureResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_Check_CaptureResultQ(
    CAPTURE_RESULT*   ptEntry)
{
    if (gtCaptureResultQ.wIdx == gtCaptureResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->resIdx         = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].resIdx;
    ptEntry->frameCount     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameCount;
    ptEntry->timeStamp      = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].timeStamp;
    ptEntry->frameIndex     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameIndex;
    ptEntry->bTopField      = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bTopField;
    ptEntry->bCapToTran     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bCapToTran;
    ptEntry->bTranToEnc     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bTranToEnc;

    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE MMP_RESULT
_CaptureResultQ_Dequeue(
    CAPTURE_RESULT*   ptEntry)
{
    if (gtCaptureResultQ.wIdx == gtCaptureResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->resIdx         = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].resIdx;
    ptEntry->frameCount     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameCount;
    ptEntry->timeStamp      = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].timeStamp;
    ptEntry->frameIndex     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].frameIndex;
    ptEntry->bTopField      = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bTopField;
    ptEntry->bCapToTran     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bCapToTran;
    ptEntry->bTranToEnc     = gtCaptureResultQ.entry[gtCaptureResultQ.rIdx].bTranToEnc;

    gtCaptureResultQ.rIdx++;
    if (gtCaptureResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtCaptureResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_CaptureResultQ_Enqueue(
    CAPTURE_RESULT*   ptEntry)
{
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].resIdx        = ptEntry->resIdx;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].frameCount    = ptEntry->frameCount;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].timeStamp     = ptEntry->timeStamp;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].frameIndex    = ptEntry->frameIndex;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].bTopField     = ptEntry->bTopField;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].bCapToTran    = ptEntry->bCapToTran;
    gtCaptureResultQ.entry[gtCaptureResultQ.wIdx].bTranToEnc    = ptEntry->bTranToEnc;

    gtCaptureResultQ.wIdx++;
    if (gtCaptureResultQ.wIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtCaptureResultQ.wIdx = 0;
}

static MMP_INLINE void
_IspResultQ_Reset(
    void)
{
    gtIspResultQ.wIdx = gtIspResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_IspResultQ_Dequeue(
    CAPTURE_RESULT*   ptEntry)
{
    if (gtIspResultQ.wIdx == gtIspResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->resIdx         = gtIspResultQ.entry[gtIspResultQ.rIdx].resIdx;
    ptEntry->frameCount     = gtIspResultQ.entry[gtIspResultQ.rIdx].frameCount;
    ptEntry->timeStamp      = gtIspResultQ.entry[gtIspResultQ.rIdx].timeStamp;
    ptEntry->frameIndex     = gtIspResultQ.entry[gtIspResultQ.rIdx].frameIndex;
    ptEntry->bCapToTran     = gtIspResultQ.entry[gtIspResultQ.rIdx].bCapToTran;
    ptEntry->bTranToEnc     = gtIspResultQ.entry[gtIspResultQ.rIdx].bTranToEnc;

    gtIspResultQ.rIdx++;
    if (gtIspResultQ.rIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtIspResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_IspResultQ_Enqueue(
    CAPTURE_RESULT*   ptEntry)
{
    gtIspResultQ.entry[gtIspResultQ.wIdx].resIdx            = ptEntry->resIdx;
    gtIspResultQ.entry[gtIspResultQ.wIdx].frameCount        = ptEntry->frameCount;
    gtIspResultQ.entry[gtIspResultQ.wIdx].timeStamp         = ptEntry->timeStamp;
    gtIspResultQ.entry[gtIspResultQ.wIdx].frameIndex        = ptEntry->frameIndex;
    gtIspResultQ.entry[gtIspResultQ.wIdx].bCapToTran        = ptEntry->bCapToTran;
    gtIspResultQ.entry[gtIspResultQ.wIdx].bTranToEnc        = ptEntry->bTranToEnc;

    gtIspResultQ.wIdx++;
    if (gtIspResultQ.wIdx >= CAPTURE_RESULT_QUEUE_SIZE)
        gtIspResultQ.wIdx = 0;
}

static MMP_INLINE void
_EncodedResultQ_Reset(
    void)
{
    gtEncodedResultQ.wIdx = gtEncodedResultQ.rIdx = 0;
}

static MMP_INLINE MMP_RESULT
_EncodedResultQ_Dequeue(
    ENCODED_RESULT* ptEntry)
{
    //MMP_INT result;

    if (gtEncodedResultQ.wIdx == gtEncodedResultQ.rIdx)
        return (MMP_RESULT)QUEUE_EMPTY;

    ptEntry->frameCount = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].frameCount;
    ptEntry->timeStamp  = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].timeStamp;
    ptEntry->pData      = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].pData;
    ptEntry->dataSize   = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].dataSize;
    ptEntry->InstanceNum   = gtEncodedResultQ.entry[gtEncodedResultQ.rIdx].InstanceNum;

    gtEncodedResultQ.rIdx++;
    if (gtEncodedResultQ.rIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.rIdx = 0;
    return (MMP_RESULT)QUEUE_NOT_EMPTY;
}

static MMP_INLINE void
_EncodedResultQ_Enqueue(
    ENCODED_RESULT* ptEntry)
{
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].frameCount = ptEntry->frameCount;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].timeStamp  = ptEntry->timeStamp;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].pData      = ptEntry->pData;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].dataSize   = ptEntry->dataSize;
    gtEncodedResultQ.entry[gtEncodedResultQ.wIdx].InstanceNum = ptEntry->InstanceNum;

    gtEncodedResultQ.wIdx++;
    if (gtEncodedResultQ.wIdx >= ENCODED_RESULT_QUEUE_SIZE)
        gtEncodedResultQ.wIdx = 0;
}

static void
_WaitAllQueue_Empty(
    void)
{
    CAPTURE_RESULT tEntry  = {0};
    MMP_UINT32 timeOut = 0;
     
    while (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry)) // && !gbEncodeFire)
    {
        usleep(30000);
        if (++timeOut > 10)
        {
            ithPrintf("wait QUEUE_NOT_EMPTY timeout %s() #%d\n", __FUNCTION__, __LINE__);
            break;
        }
    }
        
    while (QUEUE_NOT_EMPTY == _IspResultQ_Dequeue(&tEntry)) // && !gbEncodeFire)
    {
        usleep(30000);
        if (++timeOut > 10)
        {
            ithPrintf("wait QUEUE_NOT_EMPTY timeout %s() #%d\n", __FUNCTION__, __LINE__);
            break;
        }
    }  
}

//Time Stamp function
static MMP_INLINE MMP_UINT
frameRate25Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return frameCount * 40;
}

static MMP_INLINE MMP_UINT
frameRate30Hz_frameCount2TimeStamp(
    MMP_UINT frameCount)
{
    return (frameCount / 3) * 100 + (frameCount % 3) * 33;
}

static frameCount2TimeStamp _frameCount2TimeStamp_TABLE[16] =
{
    MMP_NULL,
    frameRate25Hz_frameCount2TimeStamp,   
    MMP_NULL,
    frameRate30Hz_frameCount2TimeStamp,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL,
    MMP_NULL
};

static MMP_INLINE frameCount2TimeStamp
_VIDEO_ENCODER_GetTimeStampConverter(
    MMP_CAP_FRAMERATE   frameRate)
{
    frameCount2TimeStamp pf = MMP_NULL;

    if (frameRate <= 15)
        pf = _frameCount2TimeStamp_TABLE[frameRate];

    return pf;
}

// Capture and ISP function        
MMP_RESULT
_CaptureAndVP_Init(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

#if defined(CFG_DUAL_STREAM)
    mmpCapMemoryInitialize();
#endif    
    //Select Capture Device
    mmpCapSetCaptureDevice(CAPTURE_DEV_SENSOR);
    
    //1st Capture Init, switch IO direction
    mmpCapInitialize();
    
    //2nd Device Init
    mmpCapDeviceInitialize();
    
    //3rd ISP Init
    mmpVPInitialize();
    
    return result;  
}

MMP_RESULT
_CaptureAndVP_Terminate(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    mmpCapDeviceTerminate();
    
    mmpVPTerminate();
   
    mmpCapTerminate();    

    return result;
}

MMP_RESULT
_CaptureAndVP_Fire(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_VP_SEQUENCE_SHARE *ispctxt = MMP_NULL;
    MMP_CAP_SHARE *capctxt  = MMP_NULL;
    MMP_UINT16 i, j;

    //Capture setting
    capctxt = calloc(1, sizeof(MMP_CAP_SHARE));
    if (capctxt == MMP_NULL)
        ithPrintf("capctxt alloc Fail ! %s [#%d]\n", __FILE__, __LINE__);

    memset(capctxt, 0, sizeof(MMP_CAP_SHARE));

    //get device information
    mmpCapGetDeviceInfo(capctxt);
    if (capctxt->bMatchResolution == MMP_FALSE) // ??
        goto lab_end;
    
    PalMemset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    gtAVCEncoderFlow.baseTimeStamp  = 0;
    gtAVCEncoderFlow.frameRate      = gtAVCEncoderFlow.captureFrameRate
                                    = capctxt->FrameRate

#if defined(CFG_SENSOR_FLICK_50HZ)    
                                    = MMP_CAP_FRAMERATE_25HZ;
#else
                                    = MMP_CAP_FRAMERATE_30HZ;
#endif

                                
    gtAVCEncoderFlow.pfToTimeStamp  = _VIDEO_ENCODER_GetTimeStampConverter(gtAVCEncoderFlow.frameRate);

#if defined(CFG_DUAL_STREAM)
    gtAVCEncoderFlow.pfEnDequeue = _IspResultQ_Dequeue;

    mmpVPResetEngine();
    mmpVPContextReset();
    //Register IRQ
    mmpCapRegisterIRQ(cap_isr);
    mmpVPRegisterIRQ(VP_mem_isr);
    mmpCapFunEnable(MMP_CAP_INTERRUPT);
    mmpCapFunDisable(MMP_CAP_ONFLY_MODE);
#if defined (CFG_SENSOR_FLICK_50HZ)
    gtAVCEncoderFlow.ispFrameRate[0] = 25;
   	gtAVCEncoderFlow.frameRateDiff[0] = gtAVCEncoderFlow.ispFrameRate[0] - 25; //EnPara.EnFrameRate;
   	
   	gtAVCEncoderFlow.ispFrameRate[1] = 25;
   	gtAVCEncoderFlow.frameRateDiff[1] = gtAVCEncoderFlow.ispFrameRate[1] - 25; //EnPara.EnFrameRate;
#else
    gtAVCEncoderFlow.ispFrameRate[0] = 30;
   	gtAVCEncoderFlow.frameRateDiff[0] = gtAVCEncoderFlow.ispFrameRate[0] - 30; //EnPara.EnFrameRate;
   	
   	gtAVCEncoderFlow.ispFrameRate[1] = 30;
   	gtAVCEncoderFlow.frameRateDiff[1] = gtAVCEncoderFlow.ispFrameRate[1] - 30; //EnPara.EnFrameRate;
#endif
    gtAVCEncoderFlow.frameCount[0] = 1;
    gtAVCEncoderFlow.skipCount[0] = 1;
    
    gtAVCEncoderFlow.frameCount[1] = 1;
    gtAVCEncoderFlow.skipCount[1] = 1;    

    capctxt->OutWidth = gptAVCEncoder[0]->frameWidth;
    
    for (i = 0; i < ISP_TRANSFORM_NUM; i++)
    {
        gtIspTransformParm[i].inWidth  = capctxt->OutWidth;
        gtIspTransformParm[i].inHeight = capctxt->OutHeight;

        for (j = 0; j < 3; j++)
        {
            gtIspTransformParm[i].inAddrY[j]  = capctxt->OutAddrY[j];
            gtIspTransformParm[i].inAddrUV[j] = capctxt->OutAddrUV[j];            
        }
    }

    for (i = 0; i < ISP_TRANSFORM_NUM; i++)
    {
        for (j = 0; j < gptAVCEncoder[i]->sourceBufCount; j++)
        {
            gtIspTransformParm[i].outAddrY[j]  = (MMP_UINT32) gptAVCEncoder[i]->pSourceBufAdrY[j];
            gtIspTransformParm[i].outAddrUV[j] = (MMP_UINT32) gptAVCEncoder[i]->pSourceBufAdrU[j];
        }

        gtIspTransformParm[i].outWidth = gptAVCEncoder[i]->frameWidth;
        gtIspTransformParm[i].outHeight = gptAVCEncoder[i]->frameHeight;

        gptAVCEncoder[i]->interlaced_frame = MMP_FALSE;
        gtIspTransformParm[i].deinterlaceOn = MMP_FALSE;
        gtIspTransformParm[i].bframeDouble = MMP_FALSE;
    }
#else
    gtAVCEncoderFlow.pfEnDequeue = _CaptureResultQ_Dequeue;
    
    mmpVPResetEngine();
    mmpVPContextReset();   

    //Register IRQ
    mmpVPRegisterIRQ(VP_onfly_isr);

    mmpCapDisableIRQ();
    mmpCapFunDisable(MMP_CAP_INTERRUPT);
    mmpCapFunEnable(MMP_CAP_ONFLY_MODE);

#if defined (CFG_SENSOR_FLICK_50HZ)
    gtAVCEncoderFlow.ispFrameRate[0] = 25;
   	gtAVCEncoderFlow.frameRateDiff[0] = gtAVCEncoderFlow.ispFrameRate[0] - 25; //EnPara.EnFrameRate;
#else
    gtAVCEncoderFlow.ispFrameRate[0] = 30;
   	gtAVCEncoderFlow.frameRateDiff[0] = gtAVCEncoderFlow.ispFrameRate[0] - 30; //EnPara.EnFrameRate;
#endif

    gtAVCEncoderFlow.frameCount[0] = 1;
    gtAVCEncoderFlow.skipCount[0] = 1;

    //ISP Setting
    ispctxt = calloc(1, sizeof(MMP_VP_SEQUENCE_SHARE));
    if (ispctxt == MMP_NULL)
        ithPrintf("ispctxt alloc Fail ! %s [#%d]\n", __FILE__, __LINE__);

    ispctxt->In_Width = capctxt->OutWidth;
    ispctxt->In_Height = capctxt->OutHeight;
    ispctxt->In_PitchY = CAP_MEM_BUF_PITCH;
    ispctxt->In_PitchUV = CAP_MEM_BUF_PITCH;
    ispctxt->In_Format = MMP_VP_IN_NV12;
    ispctxt->In_BufferNum = 0;

    // Signal Process Output Parameter
    for (i = 0; i < gptAVCEncoder[0]->sourceBufCount; ++i)
    {
        ispctxt->Out_AddrY[i] = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrY[i];
        ispctxt->Out_AddrU[i] = (MMP_UINT32) gptAVCEncoder[0]->pSourceBufAdrU[i];
    }

    ispctxt->Out_Width      = gptAVCEncoder[0]->frameWidth;
    ispctxt->Out_Height     = gptAVCEncoder[0]->frameHeight;
    ispctxt->Out_PitchY     = gptAVCEncoder[0]->framePitchY;
    ispctxt->Out_PitchUV    = gptAVCEncoder[0]->framePitchY;
    ispctxt->Out_Format     = MMP_VP_OUT_NV12;
    ispctxt->Out_BufferNum  = gptAVCEncoder[0]->sourceBufCount;

    //for sequence process
    ispctxt->EnCapOnflyMode = MMP_TRUE;
    ispctxt->EnOnflyInFieldMode = capctxt->IsInterlaced;

    mmpVPEnable(MMP_VP_INTERRUPT);
    mmpVPEnable(MMP_VP_REMAP_ADDRESS);
    mmpVPDisable(MMP_VP_DEINTERLACE);
    mmpVPEnable(MMP_VP_SCENECHANGE);
    mmpVPSetSequenceOutputInfo(ispctxt);
    mmpVPSequenceProcess(ispctxt);

    gptAVCEncoder[0]->interlaced_frame = MMP_FALSE;

    //gptAVCEncoder[0]->bitRate = VIDEO_BIT_RATE; //EnPara.EnBitrate;
    //gptAVCEncoder[0]->gopSize = VIDEO_FRAME_RATE; //EnPara.EnGOPSize;
    //gptAVCEncoder[0]->EnFrameRate = VIDEO_FRAME_RATE; //EnPara.EnFrameRate;
    gptAVCEncoder[0]->bISPOnFly = MMP_TRUE;

    //gtAVCEncoderFlow.encoderInfo = index;

    //if (EnPara.EnSkipMode != VIDEO_ENCODER_NO_DROP)
    //    gtAVCEncoderFlow.bSkipFrame = MMP_TRUE;
    //else
    //    gtAVCEncoderFlow.bSkipFrame = MMP_FALSE;

#endif
    if (ispctxt)
        free(ispctxt);

    //set capture parameter
    mmpCapParameterSetting(capctxt);
#if defined(SENSOR_OMNIVISION_OV7725)    
    mmpCapSetSkipMode(MMP_CAPTURE_SKIP_BY_TWO);
#elif defined(SENSOR_AR0130)
	mmpCapSetSkipMode(MMP_CAPTURE_NO_DROP);
#endif    

    //Capture Fire
    mmpCapFire();
    printf("mmpCapFire\n");
lab_end:
    if (capctxt)
        free(capctxt);

    return result; 
}

// AVC Encoder function
static void
OpenEngine(ENCODER_TYPE type)   
{  
    MMP_UINT32 i;
    
    pthread_mutex_lock(&VideoEngineMutex);       
        
    if (type == JPEG_ENCODER)
        gbJPEGEncoder = MMP_TRUE;
   
    if (type == H264_ENCODER)
        gbVideoEncoder = MMP_TRUE;
       
    if (!gbVideoEncoder)
        gbStream0Enable = MMP_TRUE;
    	
    gOpenEnginCnt++;
        
    if (gOpenEnginCnt == 2)  // already open
        goto o_end;    
    
    // JPEG use XCLK & M7CLK & JCLK
    mmpAVCEncodePowerUp();

#if !defined(CFG_DUAL_STREAM)    
    for (i=0; i < ENCODE_STREAM_NUM; i++)
    {        
        if (mmpAVCEncodeCreateHdr(gptAVCEncoder[i]) != 0)
        {
        	ithPrintf("[264 Encoder] mmpAVCEncodeCreateHdr Fail Num : %d\n", i);
            return;
        }
    }
#endif    
    
    mmpAVCEncodeEnableInterrupt(encoder_isr);
              
    mmpVPPowerUp();
    mmpCapPowerUp();
    mmpSensorPowerOn(MMP_TRUE, MMP_FALSE);	  
                 
    // Capture Fire
    _CaptureAndVP_Fire();        

o_end:    
    pthread_mutex_unlock(&VideoEngineMutex);
}

static void
CloseEngine(ENCODER_TYPE type)   
{	  
    MMP_UINT32 i;
    
    pthread_mutex_lock(&VideoEngineMutex);
      
    if (gOpenEnginCnt == 0)
        goto c_end;
    
    if (type == JPEG_ENCODER)
    {
        gbJPEGEncoder = MMP_FALSE;
        if (!gbVideoEncoder)
            gbStream0Enable = MMP_FALSE;
    }
    
    if (type == H264_ENCODER)
        gbVideoEncoder = MMP_FALSE;
        
    gOpenEnginCnt--;
        
    if (gOpenEnginCnt == 1)
        goto c_end;    
      
    //mmpSensorPowerOn(MMP_FALSE, MMP_FALSE);  
    mmpCapEnableInterrupt(MMP_FALSE);
    mmpCapPowerDown();
    _WaitAllQueue_Empty();
    mmpVPPowerDown();
    mmpSensorPowerOn(MMP_FALSE, MMP_FALSE);    
    
    for (i = 0; i < ENCODE_STREAM_NUM; i++)
        mmpAVCEncodeClose(gptAVCEncoder[i]);
        
    mmpAVCEncodePowerDown();

c_end:    
    pthread_mutex_unlock(&VideoEngineMutex);
}

void
VideoEncoder_Init(
   //VIDEO_ENCODE_PARAMETER* enPara)
   void)   
{
    MMP_UINT32 i;

#if defined(CFG_SENSOR_ENABLE)	
    gVideoEnWidth[0] = SENSOR_WIDTH;
#if defined(CFG_DUAL_STREAM)
    if (SENSOR_HEIGHT > 480)
        gVideoEnHeight[0] = 480;
    else
    	gVideoEnHeight[0] = SENSOR_HEIGHT;
#else
    gVideoEnHeight[0] = SENSOR_HEIGHT;
#endif    
    gVideoEnBitRate[0] = SENSOR_BITRATE;

#if defined(CFG_DUAL_STREAM)    
    gVideoEnWidth[1] = SENSOR_WIDTH_2;
    gVideoEnHeight[1] = SENSOR_HEIGHT_2;
    gVideoEnBitRate[1] = SENSOR_BITRATE_2;   
#endif 
#endif

    gbVideoEncoder = MMP_TRUE;
    mmpAVCEncodePowerUp();
    mmpVPPowerUp();
    mmpCapPowerUp();              
      
    //_VIDEO_ENCODER_Init();           
    if (mmpAVCEncodeInit() != 0)
        ithPrintf("[264 TEST] mmpAVCEncodeInit Fail\n");

    for (i=0; i < ENCODE_STREAM_NUM; i++)
    {
        if (MMP_NULL == gptAVCEncoder[i])
            gptAVCEncoder[i] = (AVC_ENCODER *)malloc(sizeof(AVC_ENCODER));

        if (MMP_NULL != gptAVCEncoder[i])
            memset(gptAVCEncoder[i], 0, sizeof(AVC_ENCODER));
    }  
       
    gptAVCEncoder[0]->frameWidth = gVideoEnWidth[0];
    gptAVCEncoder[0]->frameHeight = gVideoEnHeight[0];
    gptAVCEncoder[0]->frameCropTop = 0;
    gptAVCEncoder[0]->frameCropBottom = 0;
    gptAVCEncoder[0]->frameCropLeft = 0;
    gptAVCEncoder[0]->frameCropRight = 0;
    gptAVCEncoder[0]->frameRate = AVC_FRAME_RATE_30HZ;
    gptAVCEncoder[0]->EnFrameRate = VIDEO_FRAME_RATE; 
    gptAVCEncoder[0]->gopSize = VIDEO_FRAME_RATE;
    gptAVCEncoder[0]->bitRate = gVideoEnBitRate[0];
    gptAVCEncoder[0]->enableAutoSkip = 0;
    gptAVCEncoder[0]->initialDelay = 1000;
    gptAVCEncoder[0]->chromaQpOffset = 0; //0x1E; //-12~+12 need to use 5 bits 2's complemment, default -2
    gptAVCEncoder[0]->constrainedIntraPredFlag = 0;
    gptAVCEncoder[0]->disableDeblk = 0;
    gptAVCEncoder[0]->deblkFilterOffsetAlpha = 0;
    gptAVCEncoder[0]->deblkFilterOffsetBeta = 0;
    gptAVCEncoder[0]->vbvBufferSize = 0;
    gptAVCEncoder[0]->intraRefresh = 0;
    gptAVCEncoder[0]->rcIntraQp = 0;
    gptAVCEncoder[0]->userQpMax = 36;
    gptAVCEncoder[0]->userGamma = 24576; // default 0.75*32768
    gptAVCEncoder[0]->RcIntervalMode = 0; // 0:MB, 1:frame, 2:slice, 3:MB_NUM
    gptAVCEncoder[0]->MbInterval = 0;
    gptAVCEncoder[0]->MEUseZeroPmv = 0;  // 0:PMV, 1:ZMV
    gptAVCEncoder[0]->MESearchRange = 1; // 0:(128, 64), 1:(64:32), 2:(32:16), 3:(16,16)
    gptAVCEncoder[0]->IntraCostWeight = 100;
    gptAVCEncoder[0]->PicQS = 27;
    gptAVCEncoder[0]->forceIPicture = 0;
    gptAVCEncoder[0]->streamBufSelect = 0;
    gptAVCEncoder[0]->sourceBufSelect = 0;
    gptAVCEncoder[0]->framecount = 0;    
    
#if defined(CFG_DUAL_STREAM)     
    if (ENCODE_STREAM_NUM == 2)
    {
        //Instance 1
        gptAVCEncoder[1]->frameWidth = gVideoEnWidth[1];
        gptAVCEncoder[1]->frameHeight = gVideoEnHeight[1];
        gptAVCEncoder[1]->frameCropTop = 0;
        gptAVCEncoder[1]->frameCropBottom = 0;
        gptAVCEncoder[1]->frameCropLeft = 0;
        gptAVCEncoder[1]->frameCropRight = 0;
        gptAVCEncoder[1]->frameRate = AVC_FRAME_RATE_30HZ;
        gptAVCEncoder[1]->EnFrameRate = VIDEO_FRAME_RATE;
        gptAVCEncoder[1]->gopSize = VIDEO_FRAME_RATE;
        gptAVCEncoder[1]->bitRate = gVideoEnBitRate[1];
        gptAVCEncoder[1]->enableAutoSkip = 0;
        gptAVCEncoder[1]->initialDelay = 1000;
        gptAVCEncoder[1]->chromaQpOffset = 0; //0x1E; //-12~+12 need to use 5 bits 2's complemment, default -2
        gptAVCEncoder[1]->constrainedIntraPredFlag = 0;
        gptAVCEncoder[1]->disableDeblk = 0;
        gptAVCEncoder[1]->deblkFilterOffsetAlpha = 0;
        gptAVCEncoder[1]->deblkFilterOffsetBeta = 0;
        gptAVCEncoder[1]->vbvBufferSize = 0;
        gptAVCEncoder[1]->intraRefresh = 0;
        gptAVCEncoder[1]->rcIntraQp = 0;
        gptAVCEncoder[1]->userQpMax = 36;
        gptAVCEncoder[1]->userGamma = 24576; // default 0.75*32768
        gptAVCEncoder[1]->RcIntervalMode = 0; // 0:MB, 1:frame, 2:slice, 3:MB_NUM
        gptAVCEncoder[1]->MbInterval = 0;
        gptAVCEncoder[1]->MEUseZeroPmv = 0;  // 0:PMV, 1:ZMV
        gptAVCEncoder[1]->MESearchRange = 1; // 0:(128, 64), 1:(64:32), 2:(32:16), 3:(16,16)
        gptAVCEncoder[1]->IntraCostWeight = 100;
        gptAVCEncoder[1]->PicQS = 27;
        gptAVCEncoder[1]->forceIPicture = 0;

        gptAVCEncoder[1]->streamBufSelect = 0;
        gptAVCEncoder[1]->sourceBufSelect = 0;
        gptAVCEncoder[1]->framecount = 0;
    }             
    
    for (i=0; i < 5; i++)
        gtAVCEncoderFlow.encodeQue[i] = MMP_FALSE;        
#endif       

    _CaptureAndVP_Terminate();          
    _CaptureAndVP_Init();        
        
    for (i=0; i < ENCODE_STREAM_NUM; i++)
    {
        if (mmpAVCEncodeOpen(gptAVCEncoder[i]) != 0)
        {
            ithPrintf("[264 Encoder] mmpAVCEncodeOpen Fail Num : %d\n", i);
            return;
        }
        
        if (mmpAVCEncodeCreateHdr(gptAVCEncoder[i]) != 0)
        {
        	ithPrintf("[264 Encoder] mmpAVCEncodeCreateHdr Fail Num : %d\n", i);
            return;
        }
    }
       
    mmpAVCEncodeEnableInterrupt(encoder_isr);
#ifndef SENSOR_NOVATEK_NT99141
    mmpSensorPowerOn(MMP_TRUE, MMP_TRUE);
#endif
    _CaptureAndVP_Fire();
    mmpAVCEncodeDisableInterrupt();
    
    gbEncodeFire = MMP_FALSE;
    
    for (i=0; i < VIDEO_STREAM_NUM; i++)
	{
        gVideoSelIdx[i] = gVideoWrIdx[i] = 0;
		gStartgetVideo[i] = MMP_FALSE;
		gVideoUserNum[i] = 0;
	}
    gbVideoEncoderInit = MMP_TRUE;
    gbJPEGEncoder = MMP_FALSE;
    gbVideoEncoder = MMP_FALSE;		
        
    mmpCapPowerDown();
    _WaitAllQueue_Empty();
    mmpVPPowerDown();
#ifndef SENSOR_NOVATEK_NT99141	
    mmpSensorPowerOn(MMP_FALSE, MMP_TRUE);
#endif    
    mmpAVCEncodePowerDown();
       
    memset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    _CaptureResultQ_Reset();
    _IspResultQ_Reset();
    sem_init(&gpEncoderSem, 0, 0);
}

void
VideoEncoder_SetStreamBuf(
   MMP_UINT32 stramIndex,
   MMP_UINT32 bufIndex,
   MMP_UINT8* bufAdr)
{	 
	 if (bufIndex < gptAVCEncoder[stramIndex]->streamBufCount)
	 {   
	     gbStrBufFire[stramIndex][bufIndex] = MMP_FALSE;
	     gptAVCEncoder[stramIndex]->pStreamBufAdr[bufIndex] = bufAdr;
	 }
}

void
VideoEncoder_SetStreamBufCallBack(
   ENCODE_PARA_CALLBACK encodeCallback)
{	 	
	 gfpCallback = encodeCallback;
}

void
VideoEncoder_Open(
   MMP_UINT32 stramIndex)
{
    MMP_UINT32 i, j;
	  
    if (!gbVideoEncoderInit)
        return;
    
    if (stramIndex == 0 && gbStream0Enable == MMP_FALSE)
    {
        gbStream0Enable = MMP_TRUE;
        gStartgetVideo[stramIndex] = MMP_TRUE;
		gVideoUserNum[stramIndex] = 1;
    }
    else if (stramIndex == 0 && gbStream0Enable == MMP_TRUE)
		return;
    if (stramIndex == 1)
    {
    	gbStream1Enable = MMP_TRUE;
        gStartgetVideo[stramIndex] = MMP_TRUE;
		gVideoUserNum[stramIndex] = 1;
    }
    if (gbVideoEncoder)
    	return;
    	
    //mmpAVCEncodePowerUp();
    //mmpAVCEncodeEnableInterrupt(encoder_isr);
    
    OpenEngine(H264_ENCODER);
	
	for (i=0; i < VIDEO_STREAM_NUM; i++)              
        for (j=0; j < VIDEO_SAMPLE_NUM; j++)
            gbStrBufFire[i][j] = MMP_FALSE;
    
    gbEncodeFire = MMP_FALSE;
    
    for (i=0; i < VIDEO_STREAM_NUM; i++)
        gVideoSelIdx[i] = gVideoWrIdx[i] = 0;
                 
    gbVideoEncoder = MMP_TRUE;
  
    //for (i=0; i < ENCODE_STREAM_NUM; i++)
    //for (i=0; i < 1; i++)
    //    mmpAVCEncodeCreateHdr(gptAVCEncoder[i]); 
} 

void
VideoEncoder_GetHdrInfo(
   VIDEO_HEADER_INFO* headInfo)
{       
    headInfo->SPSBuf = gptAVCEncoder[headInfo->streamIndex]->pHdrBufAddr[0];
    headInfo->PPSBuf = gptAVCEncoder[headInfo->streamIndex]->pHdrBufAddr[1];
    headInfo->SPS_Size = gptAVCEncoder[headInfo->streamIndex]->ParaSetHdrSize[0];
    headInfo->PPS_Size = gptAVCEncoder[headInfo->streamIndex]->ParaSetHdrSize[1];
}

void
VideoEncoder_Close(
   void)
{
    MMP_UINT32 i;        
         
    if (!gbVideoEncoderInit)
        return;
        
    mmpAVCEncodeDisableInterrupt();        
    
    CloseEngine(H264_ENCODER);
                
    //_CaptureAndVP_Terminate();
    
    //mmpAVCEncodePowerDown();
    
    memset(&gtAVCEncoderFlow, 0, sizeof(gtAVCEncoderFlow));
    
    for (i=0; i < VIDEO_STREAM_NUM; i++)
        gptAVCEncoder[i]->streamBufSelect = 0;
        
    _CaptureResultQ_Reset();
    _IspResultQ_Reset();
    
    gfpCallback = MMP_NULL;
    gbStream0Enable = MMP_FALSE;
    gbStream1Enable = MMP_FALSE;
    gStartgetVideo[0] = MMP_FALSE;
    gStartgetVideo[1] = MMP_FALSE;
	gVideoUserNum[0] = 0;
	gVideoUserNum[1] = 0;
}

MMP_BOOL
VideoEncoder_GetSreamstate(
	MMP_UINT8 stream_id)
{
    return gStartgetVideo[stream_id];   
}

MMP_UINT8
VideoEncoder_GetSreamUserNum(
	MMP_UINT8 stream_id)
{
    //printf(">>>>>%s, %d, %d\n", __FUNCTION__, stream_id, gVideoUserNum[stream_id]);
	return gVideoUserNum[stream_id];
}

void
JPEGEncodeFrame(
   JPEG_ENCODE_PARAMETER* enPara)
{
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};
    uint32_t            jpgEncSize = 0;
    //MMP_UINT32          jpgStart = 0;
	  
    //jpgStart = PalGetClock();
	  
    if (!gbVideoEncoderInit)
        return;    
    
    OpenEngine(JPEG_ENCODER);
  
    // ------------------------------------------------------
    // encode JPEG
    initParam.codecType     = JPG_CODEC_ENC_JPG;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;
    initParam.width         = gptAVCEncoder[0]->frameWidth;
    initParam.height        = gptAVCEncoder[0]->frameHeight;
    initParam.encQuality    = enPara->quality;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);  
    
    if (sem_wait(&gpEncoderSem) == 0)
    {    	    	
        inStreamInfo.streamIOType       = JPG_STREAM_IO_READ;
        inStreamInfo.streamType         = JPG_STREAM_MEM;
            
        // Y
        inStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)gptAVCEncoder[0]->pSourceBufAdrY[gJPEGBufIndex];
        inStreamInfo.jstream.mem[0].pitch  = gptAVCEncoder[0]->framePitchY;
        //inStreamInfo.src.mem[0].length = _Get_Lcd_Width() * _Get_Lcd_Height();
        // U
        inStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)gptAVCEncoder[0]->pSourceBufAdrU[gJPEGBufIndex];
        inStreamInfo.jstream.mem[1].pitch  = gptAVCEncoder[0]->framePitchY;
        //inStreamInfo.src.mem[1].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();
        // V
        inStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)gptAVCEncoder[0]->pSourceBufAdrV[gJPEGBufIndex];
        inStreamInfo.jstream.mem[2].pitch  = gptAVCEncoder[0]->framePitchY;
        //inStreamInfo.src.mem[2].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();
        
        inStreamInfo.validCompCnt = 3;
        
        //if( encName )
        //{
        //    outStreamInfo.streamType = JPG_STREAM_FILE;
        //    outStreamInfo.jstream.path   = (void*)encName;
        //}
        
        outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType         = JPG_STREAM_MEM;
        //outStreamInfo.jpg_reset_stream_info =  _reset_stream_info;        
        outStreamInfo.jstream.mem[0].pAddr  = enPara->strmBuf; 
        outStreamInfo.jstream.mem[0].pitch  = enPara->strmBuf_size;
        outStreamInfo.jstream.mem[0].length = enPara->strmBuf_size;
        outStreamInfo.validCompCnt = 1;   
        
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);
        
        //iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        //printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
        //            jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
        //            jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
        //            initParam.dispMode);
        //printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());
        
        iteJpg_Setup(pHJpeg, 0);
        
        iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);
        
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize/1024);
        enPara->enSize = jpgEncSize;
        iteJpg_DestroyHandle(&pHJpeg, 0);
    }
    
    CloseEngine(JPEG_ENCODER);
    
    //ithPrintf("Encoding Time %d\n", PalGetDuration(jpgStart));
}

static void
_DoIsp(
    void)
{
    CAPTURE_RESULT tEntry  = {0};
    static MMP_VP_SINGLE_SHARE ispctxt = {0};
    MMP_UINT16 inBufferIdx;
    MMP_UINT16 outBufferIdx;
    static MMP_UINT16 srcCount = 0;
    MMP_UINT16 resIdx;

    //ithPrintf("DoISP %d\n", tEntry.resIdx);
    //if (QUEUE_NOT_EMPTY == _Check_CaptureResultQ(&tEntry))
    //{
    //	 if ( gtAVCEncoderFlow.encodeQue[srcCount])
    //     {
    //         //printf("[Warning !] ISP maybe overwrite Encoding Buf tran %d buf %d idx %d\n", tEntry.bTranToEnc, srcCount, tEntry.resIdx);
    //         return;
    //     }
    //}
        
    if (QUEUE_NOT_EMPTY == _CaptureResultQ_Dequeue(&tEntry))
    {
        //ithPrintf("DoISP %d\n", tEntry.resIdx);
        
        //if (tEntry.resIdx == 1)
        //	return;
        	
        resIdx              = tEntry.resIdx;
        inBufferIdx         = tEntry.frameIndex;

        outBufferIdx        = srcCount; //tEntry.frameIndex;
        gJPEGBufIndex       = outBufferIdx;
        tEntry.frameIndex   = outBufferIdx;

        srcCount = (srcCount+1) % gptAVCEncoder[0]->sourceBufCount;

        ispctxt.In_AddrY    = gtIspTransformParm[resIdx].inAddrY[inBufferIdx];
        ispctxt.In_AddrUV   = gtIspTransformParm[resIdx].inAddrUV[inBufferIdx];

        if (inBufferIdx == 0)
            ispctxt.In_AddrYp   = gtIspTransformParm[resIdx].inAddrY[2];
        else
            ispctxt.In_AddrYp   = gtIspTransformParm[resIdx].inAddrY[inBufferIdx - 1];

        ispctxt.In_Width    = gtIspTransformParm[resIdx].inWidth;
        ispctxt.In_Height   = gtIspTransformParm[resIdx].inHeight;
        ispctxt.In_PitchY   = CAP_MEM_BUF_PITCH;
        ispctxt.In_PitchUV  = CAP_MEM_BUF_PITCH;
        ispctxt.In_Format   = MMP_VP_IN_NV12;

        ispctxt.Out_AddrY   = gtIspTransformParm[resIdx].outAddrY[outBufferIdx];
        ispctxt.Out_AddrU   = gtIspTransformParm[resIdx].outAddrUV[outBufferIdx];
        
        ispctxt.Out_Width   = gptAVCEncoder[resIdx]->frameWidth;
        ispctxt.Out_Height  = gptAVCEncoder[resIdx]->frameHeight;
        ispctxt.Out_PitchY  = gptAVCEncoder[resIdx]->framePitchY;
        ispctxt.Out_PitchUV = gptAVCEncoder[resIdx]->framePitchY;
        ispctxt.Out_Format  = MMP_VP_OUT_NV12;
        //ithPrintf("Pitch %d %d\n", ispctxt.In_PitchY, ispctxt.Out_PitchY);
        //ithPrintf("ISP %d %d %d %d\n", ispctxt.In_Width, ispctxt.In_Height, ispctxt.Out_Width, ispctxt.Out_Height);
        mmpVPEnable(MMP_VP_INTERRUPT);

        mmpVPEnable(MMP_VP_REMAP_ADDRESS);

        mmpVPDisable(MMP_VP_DEINTERLACE);

        //mmpVPEnable(MMP_VP_SCENECHANGE);
        
        mmpVPSingleProcess(&ispctxt);
        //tickStart = PalGetClock();
        memcpy(&gtIspFireEntry, &tEntry, sizeof(CAPTURE_RESULT));
    }
}

static void
_DoEncode(
    void)
{
    CAPTURE_RESULT tEntry = {0};
    //ithPrintf("_DoEncode (0) %d %d %d\n", gbEncodeFire, gbStrBufFire[gVideoSelIdx], gVideoSelIdx);
    if (gbEncodeFire || !gbVideoEncoder)
        return;

    if (QUEUE_NOT_EMPTY == gtAVCEncoderFlow.pfEnDequeue(&tEntry))
    {               
        gEncodeIdx = tEntry.resIdx;
        
        //if (gEncodeIdx == 1)
        //	  return;
        //ithPrintf("gEncodeIdx %d\n", gEncodeIdx);
        if (gbStrBufFire[gEncodeIdx][gVideoSelIdx[gEncodeIdx]])
        {
            //ithPrintf("Encoding Buf Full\n");
            return;
        }
            
        gbStrBufFire[gEncodeIdx][gVideoSelIdx[gEncodeIdx]]       = MMP_TRUE;       
       
        gVideoSelIdx[gEncodeIdx]                        = (gVideoSelIdx[gEncodeIdx] + 1) % VIDEO_SAMPLE_NUM;
       
        gtAVCEncoderFlow.encodedFrameCount              = tEntry.frameCount;
        gtAVCEncoderFlow.encodedTimeStamp               = tEntry.timeStamp;
        gptAVCEncoder[gEncodeIdx]->sourceBufSelect      = tEntry.frameIndex;
        gbEncodeFire = MMP_TRUE;
        //tickStart = PalGetClock();
        mmpAVCEncodeFire(gptAVCEncoder[gEncodeIdx]);
    }
}

static void
VP_onfly_isr(
    void* arg)
{
    CAPTURE_RESULT      tEntry  = {0};
    MMP_BOOL            bIsIdle = MMP_FALSE;   
    MMP_UINT16          i;
    MMP_UINT16          captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);

    //ithPrintf("ISP ISR\n");
    //ithPrintf("ISP %d\n", PalGetDuration(tickISP));
    //tickISP = PalGetClock();
    mmpVPClearInterrupt();

    //Error frame not encode
    if ((captureErrState & 0x0F00) && gtAVCEncoderFlow.captureFrameCount == 0)
    {
        ithPrintf("---Error frame not encode---\n");
        return;
    }
    
    //if (gtAVCEncoderFlow.captureFrameCount == 0)        	 
    //    ithPrintf("Capture Init %d\n", PalGetDuration(tickStart));
        
    gJPEGBufIndex = tEntry.frameIndex = mmpVPReturnWrBufIndex();
    tEntry.frameCount = ++gtAVCEncoderFlow.captureFrameCount;
        
    if (gtAVCEncoderFlow.pfToTimeStamp)
    {
        tEntry.timeStamp = gtAVCEncoderFlow.currTimeStamp
                         = gtAVCEncoderFlow.baseTimeStamp
                         + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount);
    }
    
    //Error frame not encode
    if (captureErrState & 0x0F00)
    {
        ithPrintf("---Error frame not encode---\n");
        return;
    }

    //if (!_Chk_Skip_Frame(0))
    _CaptureResultQ_Enqueue(&tEntry);

    //if (MMP_SUCCESS == mmpAVCEncodeQuery(AVC_ENCODER_ENGINE_IDLE, (MMP_UINT32*)&bIsIdle)
    // && bIsIdle)
    if (mmpAVCEncodeIsIdle())
    {
        _DoEncode();
    }
    
    if (gbJPEGEncoder)
    {
        itpSemPostFromISR(&gpEncoderSem);
        gbJPEGEncoder = MMP_FALSE;
    }
}

static void
cap_isr(
    void* arg)
{
    CAPTURE_RESULT      tEntry  = {0};
    MMP_UINT32          dur     = 0;
    static MMP_UINT32   initClock = 0;
    static MMP_UINT32   totalDur = 0;
    MMP_UINT16          captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);
    MMP_UINT16          i;
    MMP_BOOL            bSkipFrame[ENCODE_STREAM_NUM] ;

    //ithPrintf("cap_isr\n");
    //disable Interrupt
    //mmpCapEnableInterrupt(MMP_FALSE);

	//clear interrupt
	mmpCapClearInterrupt();

    tEntry.frameIndex = mmpCapReturnWrBufIndex();
    tEntry.frameCount = ++gtAVCEncoderFlow.captureFrameCount;    

    if (gtAVCEncoderFlow.pfToTimeStamp)
    {
        tEntry.timeStamp = gtAVCEncoderFlow.currTimeStamp
                         = gtAVCEncoderFlow.baseTimeStamp
                         + gtAVCEncoderFlow.pfToTimeStamp(tEntry.frameCount);
    }

    //Error frame not encode
    if (captureErrState & 0x0F00)
    {
        printf("---Error frame not encode---\n");
        return;
        //goto end;
    }

    bSkipFrame[0] = _Chk_Skip_Frame(0);
    bSkipFrame[1] = _Chk_Skip_Frame(1);       

    for (tEntry.resIdx = 0; tEntry.resIdx < ISP_TRANSFORM_NUM; tEntry.resIdx++)
    {        
        if (!bSkipFrame[tEntry.resIdx] && 
        	     ((tEntry.resIdx == 0 && gbStream0Enable) || (tEntry.resIdx == 1 && gbStream1Enable)))
        {            	
            _CaptureResultQ_Enqueue(&tEntry);
        }
    }

end:

    if (mmpVPIsEngineIdle())
    {
        _DoIsp();
    }

    //mmpCapEnableInterrupt(MMP_TRUE);
}

static void
VP_mem_isr(
    void* arg)
{
    MMP_BOOL            bIsIdle = MMP_FALSE;
    //MMP_UINT16          sceneTotalDiff = mmpVPSceneChgTotalDiff();

    mmpVPClearInterrupt();

    //ithPrintf("VP_mem isr(0)\n");
    //ithPrintf("VP_T %d\n", PalGetDuration(tickStart));
    gtAVCEncoderFlow.encodeQue[gtIspFireEntry.frameIndex] = MMP_TRUE;
    _IspResultQ_Enqueue(&gtIspFireEntry);
 
    if (mmpAVCEncodeIsIdle())
    {
        _DoEncode();
    }

    if (gbJPEGEncoder && gtIspFireEntry.resIdx == 0)
    {
        itpSemPostFromISR(&gpEncoderSem);
        gbJPEGEncoder = MMP_FALSE;
    }

    _DoIsp();
}

static void
encoder_isr(
    void* arg)
{
    MMP_UINT32 streamLen;
    MMP_BOOL   bFrmEnd;
    //static MMP_BOOL   bFirst = MMP_TRUE;

    //ithPrintf("encoder_isr %d\n", gEncodeIdx);
    mmpAVCEncodeClearInterrupt();
    gbEncodeFire = MMP_FALSE;

    if (mmpAVCEncodeGetStream(gptAVCEncoder[gEncodeIdx], &streamLen, &bFrmEnd) == 0 && bFrmEnd == MMP_TRUE)
    {
        MMP_UINT8*  pData       = gptAVCEncoder[gEncodeIdx]->pStreamBufAdr[gVideoWrIdx[gEncodeIdx]];
        MMP_UINT    dataSize    = streamLen;
        VIDEO_STREAM_INFO  streamInfo;
        
        //ithPrintf("Get %d %d %d\n", streamLen, gEncodeIdx, gVideoWrIdx[gEncodeIdx]);        
        ithInvalidateDCacheRange(pData, dataSize);        
                
        streamInfo.streamIndex = gEncodeIdx;
        streamInfo.bufIndex    = gVideoWrIdx[gEncodeIdx];
        streamInfo.bIFrame     = gptAVCEncoder[gEncodeIdx]->bIFrame;
        streamInfo.dataSize    = dataSize;
        streamInfo.timeStamp   = gtAVCEncoderFlow.encodedTimeStamp;

#ifdef  TEST_MULTISTREAM      
        if (gfpCallback != MMP_NULL && gEncodeIdx == 0)
#else
        if (gfpCallback != MMP_NULL)
#endif        	
            gfpCallback(&streamInfo); 
        gVideoWrIdx[gEncodeIdx] = (gVideoWrIdx[gEncodeIdx] + 1) % VIDEO_SAMPLE_NUM;
    }
    else
    {
        ithPrintf("Encoder_ISR Error : stremLen %d\n", streamLen);
        mmpAVCEncodeSWRest();
    }

    //ithPrintf("En_T %d\n", PalGetDuration(tickStart));
    //if (bFirst == MMP_TRUE)        	 
    //    ithPrintf("Encode Init %d\n", PalGetDuration(tickStart));
    //bFirst = MMP_FALSE;
    _DoEncode();
}

static MMP_BOOL
_Chk_Skip_Frame(
    MMP_UINT32 InstanceNum)
{
MMP_UINT32 skipNum;
MMP_BOOL   bSkipFrm = MMP_FALSE;

    if (gtAVCEncoderFlow.frameRateDiff[InstanceNum] == 0 || 
    	 (!gbStream0Enable && InstanceNum == 0) || (!gbStream1Enable && InstanceNum == 1) )
    {
        bSkipFrm = MMP_FALSE;
        goto End;
    }

    skipNum = (gtAVCEncoderFlow.ispFrameRate[InstanceNum]/gtAVCEncoderFlow.frameRateDiff[InstanceNum]) * gtAVCEncoderFlow.skipCount[InstanceNum];

    if (gtAVCEncoderFlow.frameCount[InstanceNum]++ == skipNum)
    {
        if (gtAVCEncoderFlow.skipCount[InstanceNum]++ == gtAVCEncoderFlow.frameRateDiff[InstanceNum])
            gtAVCEncoderFlow.skipCount[InstanceNum] = 1;

        if (gtAVCEncoderFlow.frameCount[InstanceNum] == (gtAVCEncoderFlow.ispFrameRate[InstanceNum]+1))
            gtAVCEncoderFlow.frameCount[InstanceNum] = 1;

        bSkipFrm = MMP_TRUE;
    } else
        bSkipFrm = MMP_FALSE;

End :

    //ithPrintf("IsSkip(%d) FC %d SkipCount %d SkipNum %d\n", bSkipFrm, gtAVCEncoderFlow.frameCount[InstanceNum], gtAVCEncoderFlow.skipCount[InstanceNum], skipNum);

    return bSkipFrm;
}

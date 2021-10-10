
//*****************************************************************************
// Name: sxa_dmx_demuxer.c
//
// Description:
//     Decoder APIs for Media Player, Including All Type of Decoders
//
// Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
//*****************************************************************************

//=============================================================================
//                              Include Files
//=============================================================================
#include "sxa_dmx.h"
#include "sxa_dmx_3gpp.h"
#if 0
#include "sxa_dmx_avi.h"
#include "sxa_dmx_mpeg1.h"
#endif


//=============================================================================
//                              Compile Option
//=============================================================================
#if (!SUPPORT_NEW_PCM_INFO)
#define PCM_GREP_SIZE   512
#endif

//=============================================================================
//                              Private Structure Definition
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================


//=============================================================================
//                              Private Function Declaration
//=============================================================================
#if (!SUPPORT_NEW_PCM_INFO)
static SXA_DMXECODE_E
GetPCMSampleInfo(
    SXA_HANDLE_T             hDmx,
    SXA_DMXPCMSAMPLEINFO_T  *pInfo);
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================

signed int
sxaDmxOpen(
    PAL_TCHAR*      ptPathName,
    SXA_HANDLE_T*   phDmx,
	int             nInType)
{
    SXA_DMXOPENEXPARAM_T param;

    void* fp = NULL;
    unsigned int f_size = 0;
#if 0
    fp = fopen(ptPathName, PAL_FILE_RB);
    if (fp == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }
    fseek(fp, 0, SEEK_END);
    f_size = ftell(fp);
    fclose(fp);
#endif
    param.ptPathName = ptPathName;
    param.nOffset    = 0;
    param.nLength    = f_size;
    param.nMode      = 0;
    param.nKey       = 0;
    param.nInType    = nInType;

    return sxaDmxOpenEx(&param, phDmx);
}



signed int
sxaDmxOpenEx(
    SXA_DMXOPENEXPARAM_T*   pParam,
    SXA_HANDLE_T*           phDmx)
{
    GPP_RESULT Ret;

    MMP_3GPP_DECODE_3GPPDATA* p3GppData = NULL;
#if 0
    decAVIDataType*           pAviData = NULL;
    decMPEG1DataType*         pMpeg1Data = NULL;
#endif
    SXA_DMXOPENEXPARAM_T      openExParam;

    openExParam.ptPathName = pParam->ptPathName;
    openExParam.nOffset    = pParam->nOffset;
    openExParam.nLength    = pParam->nLength;
    openExParam.nMode      = pParam->nMode;
    openExParam.nKey       = pParam->nKey;
    openExParam.nInType    = pParam->nInType;

    // Check for 3GPP file format
    p3GppData = sxaDmx3GPP_CreateDec3gppData();
    if (p3GppData == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }
    printf("sxaDmx3GPP_ParseMoovBox \n\n");
    usleep(2000);
    Ret = sxaDmx3GPP_ParseMoovBox(p3GppData, &openExParam,openExParam.nInType);

    if (Ret == GPP_SUCCESS)
    {
        if (p3GppData->pFirstAudioTrak == NULL && p3GppData->pFirstVideoTrak == NULL)
        {
            sxaDmx3GPP_Terminate(p3GppData);
            // Not 3GPP file format, Check for next file format
        }
        else
        {
            // It's 3GPP file format
            p3GppData->dwFileFormat = SXA_DMXFFMT_3GPP;
            *phDmx = (SXA_HANDLE_T *)p3GppData;
            return SXA_DMXECODE_SOK;
        }
    }
    else
    {
        sxaDmx3GPP_Terminate(p3GppData);
        // Not 3GPP file format, Check for next file format
    }

#if 0
    // Check for AVI file format
    pAviData = sxaDmxAvi_CreateDecAVIData();
    if (pAviData == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }

    Ret = sxaDmxAvi_OpenAVI(pAviData, &openExParam);
    if (Ret == MMP_SUCCESS)
    {
        // It's AVI file format
        pAviData->dwFileFormat = SXA_DMXFFMT_AVI;
        *phDmx = (SXA_HANDLE_T *)pAviData;
        return SXA_DMXECODE_SOK;
    }
    else
    {
        sxaDmxAvi_Terminate(pAviData);
        // Not AVI file format, Check for next file format
    }

    // Check for MPEG1 file format
    pMpeg1Data = sxaDmxMPEG1_CreateDecMPEG1Data();
    if (pMpeg1Data == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }

    Ret = sxaDmxMPEG1_OpenMPEG1(pMpeg1Data, &openExParam);
    if (Ret == MMP_SUCCESS)
    {
        // It's MPEG1 file format
        pMpeg1Data->dwFileFormat = SXA_DMXFFMT_PS;
        *phDmx = (SXA_HANDLE_T *)pMpeg1Data;
        return SXA_DMXECODE_SOK;
    }
    else
    {
        sxaDmxMPEG1_Terminate(pMpeg1Data);
        // Not MPEG1 file format
    }
#endif

    return SXA_DMXECODE_EFAIL;
}



signed int
sxaDmxClose(
    SXA_HANDLE_T    hDmx)
{
    unsigned int fileFormat;

    if (hDmx == NULL)
    {
        return SXA_DMXECODE_EHANDLE;
    }

    fileFormat = ((unsigned int *)hDmx)[0];   // [dvyu] The first unsigned int is the FileFormat

    switch (fileFormat)
    {
    case SXA_DMXFFMT_3GPP:
        sxaDmx3GPP_Terminate((MMP_3GPP_DECODE_3GPPDATA *)hDmx);
        break;
#if 0
    case SXA_DMXFFMT_AVI:
        sxaDmxAvi_Terminate((decAVIDataType *)hDmx);
        break;
    case SXA_DMXFFMT_PS:
        sxaDmxMPEG1_Terminate((decMPEG1DataType *)hDmx);
        break;
#endif
    default:
        break;
    }
    return SXA_DMXECODE_SOK;
}



signed int
sxaDmxGetProp(
    SXA_HANDLE_T        hDmx,
    SXA_DMXPROP_E       eProp,
    void*               pvProp)
{
    unsigned int fileFormat;
    MMP_RESULT   Ret = 0;

    if (hDmx == NULL)
    {
        return SXA_DMXECODE_EHANDLE;
    }

    fileFormat = ((unsigned int *)hDmx)[0];   // [dvyu] The first unsigned int is the FileFormat

    switch (eProp)
    {
    case SXA_DMXPROP_MEDIAIFNO:
        {
            SXA_DMXMEDIAINFO_T* mediaInfo = (SXA_DMXMEDIAINFO_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                // File Format
                mediaInfo->nFFmt = SXA_DMXFFMT_3GPP;
                // Total Time
                mediaInfo->nDuration = sxaDmx3GPP_GetTotalTime((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Format
                mediaInfo->nVFmt = sxaDmx3GPP_GetVideoCodecType((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Time Scale
                mediaInfo->nVTimeScale = sxaDmx3GPP_GetVideoTimeScale((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Sample Count
                mediaInfo->nVSampleCount = sxaDmx3GPP_GetVideoSampleCount((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Width
                mediaInfo->nVWidth = sxaDmx3GPP_GetVideoWidth((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Height
                mediaInfo->nVHeight = sxaDmx3GPP_GetVideoHeight((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Key Sample Count
                mediaInfo->nVKeySampleCount = sxaDmx3GPP_GetKeySampleCount((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Video Key Sample Index
                mediaInfo->pnVKeySampleIndex = sxaDmx3GPP_GetKeySampleIndex((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Format
                mediaInfo->nAFmt = sxaDmx3GPP_GetAudioCodecType((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Time Scale
                mediaInfo->nATimeScale = sxaDmx3GPP_GetAudioTimeScale((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Sample Count
                mediaInfo->nASampleCount = sxaDmx3GPP_GetAudioSampleCount((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Sample Rate
                mediaInfo->nASampleRate = sxaDmx3GPP_GetAudioFreq((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Sample Bits
                mediaInfo->nASampleBits = sxaDmx3GPP_GetAudioSampleSize((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // Audio Channel Count
                mediaInfo->nAChannelCount = sxaDmx3GPP_GetAudioChannel((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                // ADPCM Additional Data Size
                mediaInfo->nDataSize = 0;               // [dvyu] Currently 3GPP has no such function
                // ADPCM Additional Data
                mediaInfo->pvData = NULL;               // [dvyu] Currently 3GPP has no such function
                return SXA_DMXECODE_SOK;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                // File Format
                mediaInfo->nFFmt = SXA_DMXFFMT_AVI;
                // Total Time
                mediaInfo->nDuration = sxaDmxAvi_GetTotalTime((decAVIDataType*)hDmx);
                // Video Format
                mediaInfo->nVFmt = sxaDmxAvi_GetVideoCodecType((decAVIDataType*)hDmx);
                // Video Time Scale
                mediaInfo->nVTimeScale = sxaDmxAvi_GetVideoTimeScale((decAVIDataType*)hDmx);
                // Video Sample Count
                mediaInfo->nVSampleCount = sxaDmxAvi_GetVideoSampleCount((decAVIDataType*)hDmx);
                // Video Width
                mediaInfo->nVWidth = sxaDmxAvi_GetVideoWidth((decAVIDataType*)hDmx);
                // Video Height
                mediaInfo->nVHeight = sxaDmxAvi_GetVideoHeight((decAVIDataType*)hDmx);
                // Video Key Sample Count
                mediaInfo->nVKeySampleCount = sxaDmxAvi_GetKeySampleCount((decAVIDataType*)hDmx);
                // Video Key Sample Index
                mediaInfo->pnVKeySampleIndex = sxaDmxAvi_GetKeySampleIndex((decAVIDataType*)hDmx);
                // Audio Format
                mediaInfo->nAFmt = sxaDmxAvi_GetAudioCodecType((decAVIDataType*)hDmx);
                // Audio Time Scale
                mediaInfo->nATimeScale = sxaDmxAvi_GetAudioTimeScale((decAVIDataType*)hDmx);
                // Audio Sample Count
                mediaInfo->nASampleCount = sxaDmxAvi_GetAudioSampleCount((decAVIDataType*)hDmx);
                // Audio Sample Rate
                mediaInfo->nASampleRate = sxaDmxAvi_GetAudioFreq((decAVIDataType*)hDmx);
                // Audio Sample Bits
                mediaInfo->nASampleBits = sxaDmxAvi_GetAudioSampleBits((decAVIDataType*)hDmx);
                // Audio Channel Count
                mediaInfo->nAChannelCount = sxaDmxAvi_GetAudioChannel((decAVIDataType*)hDmx);
                // ADPCM Additional Data Size
                mediaInfo->nDataSize = sxaDmxAvi_GetADPCMAdditionalInfoSize((decAVIDataType*)hDmx);
                // ADPCM Additional Data
                mediaInfo->pvData = sxaDmxAvi_GetADPCMAdditionalInfo((decAVIDataType*)hDmx);
                return SXA_DMXECODE_SOK;
                break;
            case SXA_DMXFFMT_PS:
                // File Format
                mediaInfo->nFFmt = SXA_DMXFFMT_PS;
                // Total Time
                mediaInfo->nDuration = sxaDmxMPEG1_GetTotalTime((decMPEG1DataType*)hDmx);
                // Video Format
                mediaInfo->nVFmt = sxaDmxMPEG1_GetVideoCodecType((decMPEG1DataType*)hDmx);
                // Video Time Scale
                mediaInfo->nVTimeScale = 1;             // [dvyu] Currently MPEG1 has no such function
                // Video Sample Count
                mediaInfo->nVSampleCount = sxaDmxMPEG1_GetVideoSampleCount((decMPEG1DataType*)hDmx);
                // Video Width
                mediaInfo->nVWidth = sxaDmxMPEG1_GetVideoWidth((decMPEG1DataType*)hDmx);
                // Video Height
                mediaInfo->nVHeight = sxaDmxMPEG1_GetVideoHeight((decMPEG1DataType*)hDmx);
                // Video Key Sample Count
                mediaInfo->nVKeySampleCount = 0;        // [dvyu] Currently MPEG1 has no such function
                // Video Key Sample Index
                mediaInfo->pnVKeySampleIndex = NULL;    // [dvyu] Currently MPEG1 has no such function
                // Audio Format
                mediaInfo->nAFmt = sxaDmxMPEG1_GetAudioCodecType((decMPEG1DataType*)hDmx);
                // Audio Time Scale
                mediaInfo->nATimeScale = 1;             // [dvyu] Currently MPEG1 has no such function
                // Audio Sample Count
                mediaInfo->nASampleCount = sxaDmxMPEG1_GetAudioSampleCount((decMPEG1DataType*)hDmx);
                // Audio Sample Rate
                mediaInfo->nASampleRate = sxaDmxMPEG1_GetAudioFreq((decMPEG1DataType*)hDmx);
                // Audio Sample Bits
                mediaInfo->nASampleBits = sxaDmxMPEG1_GetAudioSampleBits((decMPEG1DataType*)hDmx);
                // Audio Channel Count
                mediaInfo->nAChannelCount = sxaDmxMPEG1_GetAudioChannel((decMPEG1DataType*)hDmx);
                // ADPCM Additional Data Size
                mediaInfo->nDataSize = 0;               // [dvyu] Currently MPEG1 has no such function
                // ADPCM Additional Data
                mediaInfo->pvData = NULL;               // [dvyu] Currently MPEG1 has no such function
                return SXA_DMXECODE_SOK;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_ASAMPLEINFO:
        {
            SXA_DMXSAMPLEINFO_T* sample_info = (SXA_DMXSAMPLEINFO_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_GetAudioSampleInfo((MMP_3GPP_DECODE_3GPPDATA*)hDmx, sample_info);
                return Ret;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                Ret = sxaDmxAvi_GetAudioSampleInfo((decAVIDataType*)hDmx, sample_info);
                return Ret;
                break;
            case SXA_DMXFFMT_PS:
                Ret = sxaDmxMPEG1_GetAudioSampleInfo((decMPEG1DataType*)hDmx, sample_info);
                return Ret;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_VSAMPLEINFO:
        {
            SXA_DMXSAMPLEINFO_T* sample_info = (SXA_DMXSAMPLEINFO_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_GetVideoSampleInfo((MMP_3GPP_DECODE_3GPPDATA*)hDmx, sample_info);
                return Ret;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                Ret = sxaDmxAvi_GetVideoSampleInfo((decAVIDataType*)hDmx, sample_info);
                return Ret;
                break;
            case SXA_DMXFFMT_PS:
                Ret = sxaDmxMPEG1_GetVideoSampleInfo((decMPEG1DataType*)hDmx, sample_info);
                return Ret;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_SEEKFROMINDEX:
        {
            SXA_DMXSEEKINFO_T* seek_info = (SXA_DMXSEEKINFO_T *)pvProp;
            unsigned int index = seek_info->nSeek;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_SeekSampleInfoFromIdx((MMP_3GPP_DECODE_3GPPDATA*)hDmx, index, &seek_info->tVSampleInfo, &seek_info->tASampleInfo);
                if (Ret == GPP_SUCCESS)
                {
                    return SXA_DMXECODE_SOK;
                }
                else
                {
                    return SXA_DMXECODE_EFAIL;
                }
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                Ret = sxaDmxAvi_SeekSampleInfoFromIdx((decAVIDataType*)hDmx, index, &seek_info->tVSampleInfo, &seek_info->tASampleInfo);
                if (Ret == MMP_SUCCESS)
                {
                    return SXA_DMXECODE_SOK;
                }
                else
                {
                    return SXA_DMXECODE_EFAIL;
                }
                return Ret;
                break;
            case SXA_DMXFFMT_PS:
                // [dvyu] Currently MPEG1 has no seek functions
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_SEEKFROMTIME:
        {
            SXA_DMXSEEKINFO_T* seek_info = (SXA_DMXSEEKINFO_T *)pvProp;
            unsigned int time = seek_info->nSeek;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_SeekSampleInfoFromTime((MMP_3GPP_DECODE_3GPPDATA*)hDmx, time, &seek_info->tVSampleInfo, &seek_info->tASampleInfo);
                if (Ret == GPP_SUCCESS)
                {
                    return SXA_DMXECODE_SOK;
                }
                else
                {
                    return SXA_DMXECODE_EFAIL;
                }
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                Ret = sxaDmxAvi_SeekSampleInfoFromTime((decAVIDataType*)hDmx, time, &seek_info->tVSampleInfo, &seek_info->tASampleInfo);
                if (Ret == MMP_SUCCESS)
                {
                    return SXA_DMXECODE_SOK;
                }
                else
                {
                    return SXA_DMXECODE_EFAIL;
                }
                return Ret;
                break;
            case SXA_DMXFFMT_PS:
                // [dvyu] Currently MPEG1 has no seek functions
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_VOLINFO:
        {
            void** ppvprop = (void*)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                *ppvprop = sxaDmx3GPP_GetVideoVolInfo((MMP_3GPP_DECODE_3GPPDATA*)hDmx);
                return SXA_DMXECODE_SOK;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                *ppvprop = sxaDmxAvi_GetVideoVolInfo((decAVIDataType*)hDmx);
                return SXA_DMXECODE_SOK;
                break;
            case SXA_DMXFFMT_PS:
                *ppvprop = NULL;      // [dvyu] MPEG1 has no VOL info
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_ADTSHEADER:
        {
            SXA_DMXADTSHEADER_T* adts_ptr = (SXA_DMXADTSHEADER_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_CreateADTSHeader((MMP_3GPP_DECODE_3GPPDATA*)hDmx, adts_ptr->nAacFrameLength, adts_ptr->pAdtsHeader);
                return Ret;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                Ret = sxaDmx3GPP_CreateADTSHeader((MMP_3GPP_DECODE_3GPPDATA*)hDmx, adts_ptr->nAacFrameLength, adts_ptr->pAdtsHeader);
                return Ret;
                break;
            case SXA_DMXFFMT_PS:
                // [dvyu] MPEG1 has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;
#if (!SUPPORT_NEW_PCM_INFO)
    case SXA_DMXPROP_PCMSAMPLEINFO:
        {
            SXA_DMXPCMSAMPLEINFO_T* pcm_info = (SXA_DMXPCMSAMPLEINFO_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = GetPCMSampleInfo(hDmx, pcm_info);
                return Ret;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                // [dvyu] AVI has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
            case SXA_DMXFFMT_PS:
                // [dvyu] MPEG1 has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;
#endif
    case SXA_DMXPROP_QTABLE:
        {
            SXA_DMXQTABLE_T* q_table = (SXA_DMXQTABLE_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                // [dvyu] 3GPP has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                // [dvyu] AVI has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
            case SXA_DMXFFMT_PS:
                Ret = sxaDmxMPEG1_GetQTable((decMPEG1DataType*)hDmx, q_table);
                return Ret;
                break;
#endif
            default:
                break;
            }
        }
        break;

    case SXA_DMXPROP_USERDATA:
        {
            SXA_DMXUSERDATA_T* user_data = (SXA_DMXUSERDATA_T *)pvProp;

            switch (fileFormat)
            {
            case SXA_DMXFFMT_3GPP:
                Ret = sxaDmx3GPP_GetUserData((MMP_3GPP_DECODE_3GPPDATA*)hDmx, user_data);
                return Ret;
                break;
#if 0
            case SXA_DMXFFMT_AVI:
                // [dvyu] AVI has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
            case SXA_DMXFFMT_PS:
                // [dvyu] MPEG has no such functions
                return SXA_DMXECODE_EFAIL;
                break;
#endif
            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return SXA_DMXECODE_EFAIL;
}



signed int
sxaDmxParseShortHeader(
                       SXA_DMXSHORTHEADERINFO_T*  pSHeaderInfo,
                       SXA_DMXDECODESIZE_T*       pDecodeSize,
                       unsigned char*             stream,
                       unsigned int               size)
{
    GPP_RESULT Ret;
    BIT_STREAM vopStream = {0};

    sxaDmx3GPP_BitstreamInit(&vopStream, (unsigned long *)stream, size);

    Ret = sxaDmx3GPP_GetShortHeaderParameter(pSHeaderInfo, pDecodeSize, &vopStream);
    return Ret;
}



signed int
sxaDmxParseVop(
               SXA_DMXVOPINFO_T*     pVOPInfo,
               SXA_DMXDECODESIZE_T*  pDecodeSize,
               SXA_DMXVOLINFO_T*     pVOLInfo,
               unsigned char*        stream,
               unsigned int          size)
{
    GPP_RESULT Ret;
    BIT_STREAM vopStream = {0};

    if (size == 0)
    {
        return SXA_DMXECODE_ENULL_FRAME;
    }

    sxaDmx3GPP_BitstreamInit(&vopStream,(unsigned long *)stream, size);

    Ret = sxaDmx3GPP_GetVOPParameter(pVOPInfo, pDecodeSize, pVOLInfo, &vopStream);
    return Ret;
}



#if (!SUPPORT_NEW_PCM_INFO)

static SXA_DMXECODE_E
GetPCMSampleInfo(
    SXA_HANDLE_T             hDmx,
    SXA_DMXPCMSAMPLEINFO_T  *pInfo)
{
    SXA_DMXECODE_E Ret;
    MMP_3GPP_DECODE_3GPPDATA* pGppData = (MMP_3GPP_DECODE_3GPPDATA*)hDmx;
    SXA_DMXSAMPLEINFO_T       tmpInfo;

    unsigned long       totalSample;
    unsigned long       grepSample;

    static MMP_BOOL     constSample;
    static unsigned int constSize;
    static unsigned int constDelta;
    unsigned long       curSTSCCount;
    unsigned long       curSTSCIdx;
    MMP_BOOL            bRet;
    unsigned long       sampleNumber = pInfo->nIndex;

    totalSample = sxaDmx3GPP_GetAudioSampleCount(pGppData);

    tmpInfo.nIndex = sampleNumber;
    Ret = sxaDmx3GPP_GetAudioSampleInfo(pGppData, &tmpInfo);
    if (Ret != SXA_DMXECODE_SOK)
    {
        return Ret;
    }

    pInfo->nDelta = tmpInfo.nDelta;
    pInfo->nOffset = tmpInfo.nOffset;
    pInfo->nIndex = tmpInfo.nIndex;
    pInfo->nSize = tmpInfo.nSize;
    pInfo->nTimeStamp = tmpInfo.nTimeStamp;

    Ret = sxaDmx3GPP_IsConstSizeDelta(pGppData->pFirstAudioTrak, &bRet);
    if (Ret != GPP_SUCCESS)
    {
        return SXA_DMXECODE_EFAIL;
    }

    if (bRet == MMP_TRUE)
    {
        constSample = MMP_TRUE;
        constSize = tmpInfo.nSize;
        constDelta = tmpInfo.nDelta;
    }
    else
    {
        constSample = MMP_FALSE;
    }

    if (constSample == MMP_TRUE)
    {
        sxaDmx3GPP_GetChunkInfo(pGppData->pFirstAudioTrak, &curSTSCCount, &curSTSCIdx);
        if ((PCM_GREP_SIZE/constSize) > (curSTSCCount - curSTSCIdx))
        {
            grepSample = curSTSCCount - curSTSCIdx;
        }
        else if ((sampleNumber + PCM_GREP_SIZE/constSize) > totalSample)
        {
            grepSample = totalSample - sampleNumber + 1;
        }
        else
        {
            grepSample = PCM_GREP_SIZE/constSize;
        }

        if (grepSample > 0)
        {
            pInfo->nDelta = constDelta * grepSample;
            pInfo->nSize = constSize * grepSample;
            sampleNumber += grepSample;
        }
        else if (grepSample == 0)
        {
            sampleNumber++;
        }
        else
        {
            return SXA_DMXECODE_EFAIL;
        }
    }
    else
    {
        sampleNumber++;
        while (sampleNumber <= totalSample)
        {
            tmpInfo.nIndex = sampleNumber;
            Ret = sxaDmx3GPP_GetAudioSampleInfo(pGppData, &tmpInfo);
            if (Ret != SXA_DMXECODE_SOK)
            {
                return Ret;
            }

            if ((tmpInfo.nOffset == pInfo->nOffset + pInfo->nSize) && pInfo->nSize < 512)
            {
                pInfo->nDelta += tmpInfo.nDelta;
                pInfo->nSize += tmpInfo.nSize;
                pInfo->nTimeStamp = tmpInfo.nTimeStamp;
                sampleNumber++;
            }
            else
            {
                break;
            }

        }
    }

    pInfo->nNextIndex = sampleNumber;
    return SXA_DMXECODE_SOK;
}

#endif



signed int
sxaDmxGet1stVideoFrameInfo(
    SXA_DMXOPENEXPARAM_T*    pParam,
    SXA_DMX1STFRAMEINFO_T*   pFrameInfo)
{
    GPP_RESULT Ret;

    // Check for 3GPP file format
    Ret = sxaDmx3GPP_Get1stVideoFrameInfo(pParam, pFrameInfo);
    if (Ret == GPP_SUCCESS)
    {
        // It's 3GPP file format
        pFrameInfo->nFFmt = SXA_DMXFFMT_3GPP;
        return SXA_DMXECODE_SOK;
    }

#if 0
    // Check for AVI file format
    Ret = sxaDmxAvi_Get1stVideoFrameInfo(pParam, pFrameInfo);
    if (Ret == GPP_SUCCESS)
    {
        // It's AVI file format
        pFrameInfo->nFFmt = SXA_DMXFFMT_AVI;
        return SXA_DMXECODE_SOK;
    }

    // Check for MPEG1 file format
    Ret = sxaDmxMPEG1_Get1stVideoFrameInfo(pParam, pFrameInfo);
    if (Ret == GPP_SUCCESS)
    {
        // It's MPEG1 file format
        pFrameInfo->nFFmt = SXA_DMXFFMT_PS;
        return SXA_DMXECODE_SOK;
    }
#endif

    return SXA_DMXECODE_EFAIL;
}


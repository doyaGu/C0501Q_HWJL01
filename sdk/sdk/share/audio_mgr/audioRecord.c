

#include "audio_mgr.h"
//#include "fat\api_f.h"

#if 1 //def SMTK_AUDIO_RECORD_ENABLE

#undef  AUDIO_RECORD_DEBUG
#define AUDIO_RECORD_DEBUG                            1

#undef  LOG_ZONES
#undef  LOG_ERROR
#undef  LOG_WARNING
#undef  LOG_INFO
#undef  LOG_DEBUG
#undef  LOG_ENTER
#undef  LOG_LEAVE
#undef  LOG_ENTERX
#undef  LOG_LEAVEX
#undef  LOG_END

#define AUDIO_RECORD_SETBIT(n)                                                                               \
    (((unsigned int) 0x00000001) << (n))

#define LOG_ZONES                                                                                   \
        (                                                                                           \
            AUDIO_RECORD_SETBIT(1 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_ERROR   */              \
            AUDIO_RECORD_SETBIT(2 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_WARNING */              \
            AUDIO_RECORD_SETBIT(3 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_INFO    */              \
            AUDIO_RECORD_SETBIT(4 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_DENUG   */              \
            AUDIO_RECORD_SETBIT(0 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_ENTER   */              \
            AUDIO_RECORD_SETBIT(0 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_LEAVE   */              \
            AUDIO_RECORD_SETBIT(0 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_ENTERX  */              \
            AUDIO_RECORD_SETBIT(0 & (AUDIO_RECORD_DEBUG ? 0xffffffff : 0)) | /* LOG_LEAVEX  */              \
            0                                                                                       \
        )                                                                                           \

#define LOG_ERROR   ((void) ((AUDIO_RECORD_SETBIT(1) & LOG_ZONES) ? (printf("[Audio Record]" "[x] "
#define LOG_WARNING ((void) ((AUDIO_RECORD_SETBIT(2) & LOG_ZONES) ? (printf("[Audio Record]" "[!] "
#define LOG_INFO    ((void) ((AUDIO_RECORD_SETBIT(3) & LOG_ZONES) ? (printf("[Audio Record]" "[i] "
#define LOG_DEBUG   ((void) ((AUDIO_RECORD_SETBIT(4) & LOG_ZONES) ? (printf("[Audio Record]" "[?] "
#define LOG_ENTER   ((void) ((AUDIO_RECORD_SETBIT(5) & LOG_ZONES) ? (printf("[Audio Record]" "[+] "
#define LOG_LEAVE   ((void) ((AUDIO_RECORD_SETBIT(6) & LOG_ZONES) ? (printf("[Audio Record]" "[-] "
#define LOG_ENTERX  ((void) ((AUDIO_RECORD_SETBIT(7) & LOG_ZONES) ? (printf("[Audio Record]" "[+] "
#define LOG_LEAVEX  ((void) ((AUDIO_RECORD_SETBIT(8) & LOG_ZONES) ? (printf("[Audio Record]" "[-] "
#define LOG_END     )), 1 : 0));

#define RECORD_SIZE       (30*1024)

#define AUDIO_MGR_POLL_STACK_SIZE     (255 * 1024)

#define SMTK_AUDIO_WAV 12
#define SMTK_AUDIO_RECORD_LIMIT_TIME_OFFSET 30

#define getFreeLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ? ((len) - ((wrptr) - (rdptr)) - 2) : ((rdptr) - (wrptr) - 2))
#define getRecordLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ? ( (wrptr) -(rdptr)) : ((len) - ((rdptr) - (wrptr)) ))

typedef struct SMTK_AUDIO_RECORD_MGR_TAG
{
    char*           ptPathName;
    unsigned char*              ptRecordBuf;
    MMP_INT                   nChannel;
    MMP_INT                   nSampleRate;
    MMP_INT                   nRecordType;
    MMP_INT                   nReadSize;
    void*                        ptFileRecord;
    MMP_INT                  nError;
    MMP_INT                  nCurrentTime;
    MMP_INT                  nLimit;
    MMP_INT                  nLimitTime;
    MMP_UINT                nAvgBytesPerSec;
    MMP_INT64               nLimitSize;
    MMP_INT64              nDataSize;
    MMP_INT                  nTotalTime;
    MMP_INT                  nShowSpectrum;
    int                             nLineIn;
    SMTK_AUDIO_RECORD_STATUS nStatus;
} SMTK_AUDIO_RECORD_MGR;

static unsigned char ptWaveHeader[48];

static SMTK_AUDIO_RECORD_MGR audioRecordMgr;

//static MMP_EVENT eventAudioRecordMgrToThread;
//static MMP_EVENT eventAudioRecordThreadToMgr;

static sem_t gAudioMgrRecordSemaphore;

static sem_t* gpAudioMgrRecordSemaphore = &gAudioMgrRecordSemaphore;

static char* gpRecordI2sBuf;

static int gRecordI2sBufSize = 2*RECORD_SIZE;

// init i2s adc
static int AudioRecordInitAdc(MMP_INT nChannels , MMP_INT nSampleRate,int nLineIn)
{
    STRC_I2S_SPEC spec_ad = {0};

    i2s_deinit_ADC();

    printf("AudioRecordInitAdc %d %d %d \n",nChannels,nSampleRate,nLineIn);

    memset((void*)&spec_ad, 0, sizeof(STRC_I2S_SPEC));
    /* ADC Spec */
    spec_ad.channels      = nChannels;
    spec_ad.sample_rate   =nSampleRate;
    spec_ad.buffer_size   = gRecordI2sBufSize;

    spec_ad.is_big_endian = 0;
    spec_ad.base_i2s      = gpRecordI2sBuf;

    spec_ad.sample_size   = 16;
    spec_ad.record_mode   = 1;
    if (nLineIn){
        spec_ad.from_LineIN   = 1;
        spec_ad.from_MIC_IN   = 0;
    } else {
        spec_ad.from_LineIN   = 0;
        spec_ad.from_MIC_IN   = 1;
    }

    i2s_init_ADC(&spec_ad);

}

// get available buffer length
static unsigned short AudioRecordGetAvailBufferLength()
{
    unsigned short rd = I2S_AD16_GET_RP();
    unsigned short wr = I2S_AD16_GET_WP();
    //printf("r ptr %d w ptr %d \n",rd,wr);
    return getRecordLen(I2S_AD16_GET_RP(),I2S_AD16_GET_WP(),gRecordI2sBufSize);

}

// get audio data
static int AudioRecordGetI2sData(unsigned short nLength)
{
    unsigned short nTemp;

    nTemp = I2S_AD16_GET_RP();
    audioRecordMgr.nReadSize = (int)nLength;
//    printf("%d %d %d \n",nTemp,I2S_AD16_GET_WP(),nLength);
    memcpy( audioRecordMgr.ptRecordBuf,gpRecordI2sBuf+nTemp ,nLength);
    nTemp+=nLength;
    if (nTemp>=gRecordI2sBufSize)
        nTemp =0;
    I2S_AD16_SET_RP(nTemp);

}


// get encode time
// return unit (ms)
static int AudioRecordGetRecordTime()
{

    return (audioRecordMgr.nDataSize/(audioRecordMgr.nChannel*(audioRecordMgr.nSampleRate/1000)*2));
}


static int AudioRecordPackWaveHeader(MMP_INT nType,MMP_INT nChannels , MMP_INT nSampleRate)
{
    SMTK_AUDIO_RECORD_WAVE_WAVEINFO tWaveInfo;
    MMP_INT samplesPerBlock = 0;
    MMP_INT nBlockAlign =0;
    tWaveInfo.format = nType;
    // Chunk ID "RIFF"
    ptWaveHeader[0] = 'R';
    ptWaveHeader[1] = 'I';
    ptWaveHeader[2] = 'F';
    ptWaveHeader[3] = 'F';

    // Chunk Size (4+n) (dont care)
    ptWaveHeader[4] = (unsigned char)(( (audioRecordMgr.nDataSize+36)>>  0) & 0xff);
    ptWaveHeader[5] = (unsigned char)(( (audioRecordMgr.nDataSize+36) >> 8) & 0xff);
    ptWaveHeader[6] = (unsigned char)(( (audioRecordMgr.nDataSize+36) >> 16) & 0xff);
    ptWaveHeader[7] = (unsigned char)(( (audioRecordMgr.nDataSize+36) >> 24) & 0xff);

    if (tWaveInfo.format == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ADPCM)
    { // ADPCM
        ptWaveHeader[44] = (unsigned char)((audioRecordMgr.nDataSize>>  0) & 0xff);
        ptWaveHeader[45] = (unsigned char)((audioRecordMgr.nDataSize >> 8) & 0xff);
        ptWaveHeader[46] = (unsigned char)((audioRecordMgr.nDataSize >> 16) & 0xff);
        ptWaveHeader[47] = (unsigned char)((audioRecordMgr.nDataSize >> 24) & 0xff);
    }
    else
    {
        ptWaveHeader[40] = (unsigned char)((audioRecordMgr.nDataSize>>  0) & 0xff);
        ptWaveHeader[41] = (unsigned char)((audioRecordMgr.nDataSize >> 8) & 0xff);
        ptWaveHeader[42] = (unsigned char)((audioRecordMgr.nDataSize >> 16) & 0xff);
        ptWaveHeader[43] = (unsigned char)((audioRecordMgr.nDataSize >> 24) & 0xff);
    }

    nSampleRate =audioRecordMgr.nSampleRate;
#if 0
    switch(audioRecordMgr.nSampleRate)
    {
        case SMTK_AUDIO_RECORD_SAMPLERATE_6000:
            nSampleRate = 6000;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_8000:
            nSampleRate = 8000;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_11025:
            nSampleRate = 11025;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_12000:
            nSampleRate = 12000;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_16000:
            nSampleRate = 16000;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_22050:
            nSampleRate = 22050;
        break;

        case SMTK_AUDIO_RECORD_SAMPLERATE_24000:
            nSampleRate = 24000;
        break;
        case SMTK_AUDIO_RECORD_SAMPLERATE_32000:
            nSampleRate = 32000;
        break;
        case SMTK_AUDIO_RECORD_SAMPLERATE_44100:
            nSampleRate = 44100;
        break;
        case SMTK_AUDIO_RECORD_SAMPLERATE_48000:
            nSampleRate = 48000;
        break;


        default:
            nSampleRate = 16000;
        break;

     }
#endif

    // WAVE ID
    ptWaveHeader[8] = 'W';
    ptWaveHeader[9] = 'A';
    ptWaveHeader[10] = 'V';
    ptWaveHeader[11] = 'E';

    // Chunk ID "fmt "
    ptWaveHeader[12] = 'f';
    ptWaveHeader[13] = 'm';
    ptWaveHeader[14] = 't';
    ptWaveHeader[15] = ' ';

    // Chunk Size: ADPCM is 20, others is 16
    if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ADPCM)
    {
        ptWaveHeader[16] = 20;
        ptWaveHeader[36]=0x02;
        ptWaveHeader[37]=0;
        // sample per block

       samplesPerBlock = ( 256*nChannels*((nSampleRate <= 11000)?1:(nSampleRate/11000)) - 4*nChannels )*8 /(4*nChannels) + 1;

        printf("samplesPerBlock %d \n",samplesPerBlock);
        ptWaveHeader[38] = (unsigned char)((samplesPerBlock>>  0) & 0xff);
        ptWaveHeader[39] = (unsigned char)((samplesPerBlock >>  8) & 0xff);

    }
    else
    {
        ptWaveHeader[16] = 16;
    }
    ptWaveHeader[17] = 0;
    ptWaveHeader[18] = 0;
    ptWaveHeader[19] = 0;

    // Format Code (1->PCM, 6->aLaw, 7->uLaw)
    if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_MULAW)
    {
        ptWaveHeader[20] = 7;
    }
    else  if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ALAW)
    {
        ptWaveHeader[20] = 6;
    }
    else  if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16 || nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM8)
    {
        ptWaveHeader[20] = 1;
    }else if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ADPCM)
    {
        ptWaveHeader[20] = 17;
    }

    ptWaveHeader[21] = 0;

    // Channel
    ptWaveHeader[22] = nChannels;
    ptWaveHeader[23] = 0;

    tWaveInfo.sampRate = nSampleRate;
    // Sampling rate
    ptWaveHeader[24] = (unsigned char)((tWaveInfo.sampRate >>  0) & 0xff);
    ptWaveHeader[25] = (unsigned char)((tWaveInfo.sampRate >>  8) & 0xff);
    ptWaveHeader[26] = (unsigned char)((tWaveInfo.sampRate >> 16) & 0xff);
    ptWaveHeader[27] = (unsigned char)((tWaveInfo.sampRate >> 24) & 0xff);

    // Bits per sample, (PCM16->16, PCM8/aLaw/uLaw->8, ADPCM->4)
     if (nType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16)
    {
        ptWaveHeader[34] = 16;
    }
     else  if (nType != SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ADPCM)
    {
         ptWaveHeader[34] = 8;
    }
    else
    {
        ptWaveHeader[34] = 4;
    }
    ptWaveHeader[35] = 0;

    // Data block size (dont care)  block align = bits per sample / 8 * channels
    nBlockAlign = ptWaveHeader[34] /8 * nChannels;
    ptWaveHeader[32] = (unsigned char)((nBlockAlign>>  0) & 0xff);
    ptWaveHeader[33] = (unsigned char)((nBlockAlign>>  8) & 0xff);


    // Data rate (dont care)  Average byte per seconds == sample rate * block align
    audioRecordMgr.nAvgBytesPerSec = tWaveInfo.sampRate * nBlockAlign;
    ptWaveHeader[28] = (unsigned char)((  (tWaveInfo.sampRate*nBlockAlign) >>  0) & 0xff);
    ptWaveHeader[29] = (unsigned char)((  (tWaveInfo.sampRate*nBlockAlign) >>  8) & 0xff);
    ptWaveHeader[30] = (unsigned char)((  (tWaveInfo.sampRate*nBlockAlign) >> 16) & 0xff);
    ptWaveHeader[31] = (unsigned char)((  (tWaveInfo.sampRate*nBlockAlign) >> 24) & 0xff);


    if (tWaveInfo.format == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ADPCM) { // ADPCM
        ptWaveHeader[40] = 'd';
        ptWaveHeader[41] = 'a';
        ptWaveHeader[42] = 't';
        ptWaveHeader[43] = 'a';

        return 48;
    } else {
        ptWaveHeader[36] = 'd';
        ptWaveHeader[37] = 'a';
        ptWaveHeader[38] = 't';
        ptWaveHeader[39] = 'a';

	return 44;
    }
}


MMP_INT
AudioRecordPrintf(
    const MMP_WCHAR* strPtr)
{
    MMP_WCHAR* tempPtr = strPtr;
	MMP_INT result = 0;

	 while(*tempPtr)
	 	printf("%c", *tempPtr++);

	 printf("\n");
    return result;
}

MMP_INT
AudioRecordSetStatus(
  const MMP_INT nStatus,const MMP_INT nError)
{
    MMP_INT nResult = 0;
    audioRecordMgr.nStatus = nStatus;
    audioRecordMgr.nError = nError;

    printf("AudioRecordSetStatus %d %d\n", nStatus, nError);

    if (nStatus == SMTK_AUDIO_RECORD_STATUS_INITIALIZE && nError == SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL){
//        smtkFlashMgrSetError(1);
    }

    return nResult;
}

MMP_INT
AudioRecordSetLimitCondition(
)
{
    MMP_INT nResult = 0;
    if( audioRecordMgr.nLimit != SMTK_AUDIO_RECORD_LIMIT_NOT_CARE)
    {
        if( audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_SIZE || audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE)
        {

        }

        if( audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_TIME || audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE)
        {

        }

    }

    return nResult;
}

MMP_INT
AudioRecordCheckLimitHappen(
)
{
    MMP_INT nResult = SMTK_AUDIO_RECORD_ERROR_NO_ERROR;
    MMP_INT nTemp;
    if( audioRecordMgr.nLimit != SMTK_AUDIO_RECORD_LIMIT_NOT_CARE)
    {
        if( audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_TIME || audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE)
        {
              //iteAudioGetEncodeTime(&nTemp);
              nTemp = AudioRecordGetRecordTime();
              if (nTemp > audioRecordMgr.nLimitTime + SMTK_AUDIO_RECORD_LIMIT_TIME_OFFSET)
              {
                  LOG_ERROR
                      "Limit Time Error curr %d limit %d %s\n",nTemp,audioRecordMgr.nLimitTime, __FUNCTION__
                  LOG_END
                  AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL);

                  if( (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16) || (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM8) ||
                       (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ALAW) || (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_MULAW) )
                  {
                        nTemp = audioRecordMgr.nAvgBytesPerSec * audioRecordMgr.nLimitTime/1000;
                        LOG_DEBUG
                          " nTemp %d ,nDataSize %d ,nReadSize %d lns %d \n",nTemp,audioRecordMgr.nDataSize,audioRecordMgr.nReadSize,__LINE__
                        LOG_END

                        if( ((audioRecordMgr.nDataSize + audioRecordMgr.nReadSize) > nTemp) && (nTemp >audioRecordMgr.nDataSize))
                        {
                            if ( (nTemp - audioRecordMgr.nDataSize) != fwrite(audioRecordMgr.ptRecordBuf, sizeof(char), (nTemp - audioRecordMgr.nDataSize), audioRecordMgr.ptFileRecord))
                            {
                                LOG_ERROR
                                    "Can not write audio output file  lns %d \n",__LINE__
                                LOG_END
                                AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL);
                            }
                            audioRecordMgr.nDataSize +=  (nTemp - audioRecordMgr.nDataSize) ;

                            LOG_DEBUG
                              " save diff %d nDataSize %d lns %d\n",nTemp,audioRecordMgr.nDataSize,__LINE__
                            LOG_END
                        }
                        else if ( ((audioRecordMgr.nDataSize + audioRecordMgr.nReadSize) <= nTemp) && (nTemp >audioRecordMgr.nDataSize))
                        {
                            LOG_DEBUG
                              " nDataSize + nReadSize < nTemp lns  %d \n",__LINE__
                            LOG_END
                            if ( (nTemp - audioRecordMgr.nDataSize) != fwrite(audioRecordMgr.ptRecordBuf, sizeof(char), (nTemp - audioRecordMgr.nDataSize), audioRecordMgr.ptFileRecord))
                            {
                                LOG_ERROR
                                    "Can not write audio output file  lns %d \n",__LINE__
                                LOG_END
                                AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL);
                            }
                            audioRecordMgr.nDataSize +=  (nTemp - audioRecordMgr.nDataSize) ;

                            LOG_DEBUG
                              " save diff %d nDataSize %d lns %d\n",nTemp,audioRecordMgr.nDataSize,__LINE__
                            LOG_END

                        }

                  }

                  return SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL;

              }
        }

        if( audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_SIZE || audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE)
        {
            if ( (audioRecordMgr.nDataSize + audioRecordMgr.nReadSize) > audioRecordMgr.nLimitSize)
            {
                LOG_ERROR
                    "Limit Size Error curr %d limit %d %s\n",(audioRecordMgr.nDataSize + audioRecordMgr.nReadSize),audioRecordMgr.nLimitSize, __FUNCTION__
                LOG_END
                AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL);

                if( (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16) || (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM8) ||
                     (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_ALAW) || (audioRecordMgr.nRecordType == SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_MULAW) )
                {

                      LOG_DEBUG
                        " nDataSize %d ,nReadSize %d lns %d \n",audioRecordMgr.nDataSize,audioRecordMgr.nReadSize,__LINE__
                      LOG_END

                      if( ((audioRecordMgr.nDataSize + audioRecordMgr.nReadSize) > audioRecordMgr.nLimitSize) && (audioRecordMgr.nLimitSize >audioRecordMgr.nDataSize))
                      {
                          if ( (audioRecordMgr.nLimitSize - audioRecordMgr.nDataSize) != fwrite(audioRecordMgr.ptRecordBuf, sizeof(char), (audioRecordMgr.nLimitSize - audioRecordMgr.nDataSize), audioRecordMgr.ptFileRecord))
                          {
                              LOG_ERROR
                                  "Can not write audio output file  lns %d \n",__LINE__
                              LOG_END
                              AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL);
                          }
                          audioRecordMgr.nDataSize += (audioRecordMgr.nLimitSize - audioRecordMgr.nDataSize);
                          LOG_DEBUG
                            " save diff %d nDataSize %d lns %d\n",(audioRecordMgr.nLimitSize-audioRecordMgr.nDataSize),audioRecordMgr.nDataSize,__LINE__
                          LOG_END

                      }

                }

                return SMTK_AUDIO_RECORD_ERROR_LIMIT_SIZE_FAIL;

            }

        }

    }

    return nResult;

}

static void*
AudioRecodeThreadFunc(
    void* arg)
{
   MMP_INT nResult;

   MMP_INT nTemp = 0;
   MMP_ULONG bufSize = 0;
   unsigned short nLength;

   LOG_ENTER
      " ThreadFunc \n"
   LOG_END

    sem_wait(gpAudioMgrRecordSemaphore);
    //nResult = PalWaitEvent(eventAudioRecordMgrToThread, PAL_EVENT_INFINITE);

    for( ; ; )
    {
       audioRecordMgr.nReadSize = 0;
        while (1)
        {
            //printf("AudioRecodeThreadFunc a \n");
            if (audioRecordMgr.nStatus == SMTK_AUDIO_RECORD_STATUS_PAUSE)
            {
                LOG_DEBUG
                " pause \n"
                LOG_END

                sem_wait(gpAudioMgrRecordSemaphore);
                //nResult = PalWaitEvent(eventAudioRecordMgrToThread, PAL_EVENT_INFINITE);
            }
            else if( audioRecordMgr.nStatus == SMTK_AUDIO_RECORD_STATUS_INITIALIZE)
            {
                LOG_DEBUG
                  " nStatus == SMTK_AUDIO_RECORD_STATUS_INITIALIZE \n"
                LOG_END
                // stop to write buffer data
                //nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_INPUT_BUFFER, &bufSize);
                nLength = AudioRecordGetAvailBufferLength();
                if (nLength>0)
                {
                    AudioRecordGetI2sData(nLength);
//                    iteAudioReadStream((MMP_UINT8*) audioRecordMgr.ptRecordBuf + (MMP_INT)audioRecordMgr.nReadSize, bufSize, 0);
                    fwrite(audioRecordMgr.ptRecordBuf, sizeof(char), audioRecordMgr.nReadSize, audioRecordMgr.ptFileRecord);
                    printf("[Auido Record] stop bufSize %d \n",audioRecordMgr.nReadSize);
                    audioRecordMgr.nDataSize += audioRecordMgr.nReadSize;
                    nResult = fseek(audioRecordMgr.ptFileRecord,0,SEEK_SET);
                    nResult = fseek(audioRecordMgr.ptFileRecord, 0, SEEK_END);
                    audioRecordMgr.nDataSize = ftell(audioRecordMgr.ptFileRecord);
                    audioRecordMgr.nDataSize -= 44;
                    audioRecordMgr.nReadSize = 0;
                }

                goto stop;
            }

            //nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_INPUT_BUFFER, &bufSize);
            nLength = AudioRecordGetAvailBufferLength();
            //printf("AudioRecodeThreadFunc b %d \n",nLength);
            if (nLength < RECORD_SIZE+640)
            {
                usleep(1000);
                continue;
            }
            //printf("AudioRecodeThreadFunc c \n");
            //bufSize = bufSize > (RECORD_SIZE - audioRecordMgr.nReadSize) ? (RECORD_SIZE - audioRecordMgr.nReadSize) : bufSize;
            //iteAudioReadStream((MMP_UINT8*) audioRecordMgr.ptRecordBuf + (MMP_INT)audioRecordMgr.nReadSize, bufSize, 0);
            //audioRecordMgr.nReadSize += bufSize;
            AudioRecordGetI2sData(RECORD_SIZE);
            if (nLength >= RECORD_SIZE)
            {
                if (audioRecordMgr.nReadSize != fwrite(audioRecordMgr.ptRecordBuf, sizeof(char), audioRecordMgr.nReadSize, audioRecordMgr.ptFileRecord))
                {
                    LOG_ERROR
                        "Can not write audio output file\n"
                    LOG_END
                    AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL);

                    goto stop;
                }
                else
                {
                    audioRecordMgr.nDataSize += audioRecordMgr.nReadSize;

                     LOG_DEBUG
                        " Save Encode file  %d  nDataSize %d  %d lns %d \n",audioRecordMgr.nReadSize, audioRecordMgr.nDataSize,AudioRecordGetRecordTime(),__LINE__
                     LOG_END
                 }
                 audioRecordMgr.nReadSize = 0;
            }
            //printf("AudioRecodeThreadFunc d \n");
            //usleep(1000);
            //iteAudioGetEncodeTime(&audioRecordMgr.nCurrentTime);
            audioRecordMgr.nCurrentTime = AudioRecordGetRecordTime();
            if (audioRecordMgr.nCurrentTime >0 && audioRecordMgr.nCurrentTime-nTemp>1000)
            {
                nTemp = audioRecordMgr.nCurrentTime;
                LOG_DEBUG
                " Current Time %d \n",nTemp/1000
                LOG_END
            }
            //printf("AudioRecodeThreadFunc e \n");
            nResult = AudioRecordCheckLimitHappen();
            if(nResult != SMTK_AUDIO_RECORD_ERROR_NO_ERROR)
            {
                 goto stop;
             }
        }

    stop:

        audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_INITIALIZE;

        if ( audioRecordMgr.ptFileRecord)
        {
            nResult = fseek(audioRecordMgr.ptFileRecord, 0, SEEK_SET);

            nResult = AudioRecordPackWaveHeader(audioRecordMgr.nRecordType, audioRecordMgr.nChannel, audioRecordMgr.nSampleRate);
            if (nResult != fwrite(ptWaveHeader, sizeof(char), nResult, audioRecordMgr.ptFileRecord))
            {
                LOG_ERROR
                    "Can not write audio output file\n"
                LOG_END
            }
            else
            {
                 LOG_DEBUG
                    " Save Wave file header   %d \n",nResult
                 LOG_END
             }

            fclose(audioRecordMgr.ptFileRecord);
            audioRecordMgr.ptFileRecord = NULL;
            LOG_DEBUG
               " Encode file close %s \n",__FUNCTION__
            LOG_END
        }

        audioRecordMgr.nCurrentTime = 0;

        nTemp =0;
        //nResult = PalWaitEvent(eventAudioRecordMgrToThread, PAL_EVENT_INFINITE);
        sem_wait(gpAudioMgrRecordSemaphore);


    }

end:

    sem_post(gpAudioMgrRecordSemaphore);

    //nResult = PalSetEvent(eventAudioRecordThreadToMgr);

    LOG_LEAVE
        "ThreadFunc()=MMP_NULL\r\n"
    LOG_END
    return MMP_NULL;


}

//static PAL_THREAD thread;

MMP_INT
smtkAudioMgrRecordInitialize(
    void)
{
    MMP_INT nResult = 0;
    MMP_UINT i;

    pthread_t task;
    pthread_attr_t attr;


    LOG_ENTER
        "smtkAudioRecordMgrInitialize()\r\n"
    LOG_END

    memset(&audioRecordMgr, 0, sizeof (audioRecordMgr));
    audioRecordMgr.ptPathName = malloc(512 * sizeof(char));

    audioRecordMgr.ptRecordBuf = malloc(RECORD_SIZE);


    if (!audioRecordMgr.ptRecordBuf)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_MEMORY_NOT_CREATE;
        goto end;
    }

    gpRecordI2sBuf = malloc (gRecordI2sBufSize);

    if (!gpRecordI2sBuf)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_MEMORY_NOT_CREATE;
        goto end;
    }


    nResult = sem_init(gpAudioMgrRecordSemaphore, 0, 0);
    if (nResult == -1)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_UNKNOW_FAIL;
        goto end;
    }


    audioRecordMgr.nDataSize = 0;
/*    eventAudioRecordMgrToThread = PalCreateEvent(PAL_EVENT_AUDIO_RECORD_MGR_TO_THREAD);
    if (eventAudioRecordMgrToThread == MMP_NULL)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_UNKNOW_FAIL;
        goto end;
    }
    eventAudioRecordThreadToMgr = PalCreateEvent(PAL_EVENT_AUDIO_RECORD_THREAD_TO_MGR);
    if (eventAudioRecordMgrToThread == MMP_NULL)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_UNKNOW_FAIL;
        goto end;
    }*/

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, AUDIO_MGR_POLL_STACK_SIZE);
    pthread_create(&task, &attr, AudioRecodeThreadFunc, NULL);

    /*thread = PalCreateThread(PAL_THREAD_RECORD, ThreadFunc, MMP_NULL, 4096,
        PAL_THREAD_PRIORITY_NORMAL);
    if (thread == MMP_NULL)
    {
        nResult = SMTK_AUDIO_RECORD_ERROR_UNKNOW_FAIL;
        goto end;
    }*/
     audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_INITIALIZE;


end:
    LOG_LEAVE
        "smtkaudioRecordMgrInitialize()=%d\r\n", nResult
    LOG_END
    return nResult;
}

MMP_INT
smtkAudioMgrRecordSetting(
  SMTK_AUDIO_RECORD_PARAM*   pParam)
{
    MMP_INT nResult = 0;
    MMP_INT nSampleRate;
    LOG_ENTER
        "SmtkAudioRecordMgrInFile()\r\n"
    LOG_END
    audioRecordMgr.nError = SMTK_AUDIO_RECORD_ERROR_NO_ERROR;

    if (audioRecordMgr.nStatus != SMTK_AUDIO_RECORD_STATUS_INITIALIZE)
    {
         LOG_ERROR
             " nStatus != SMTK_AUDIO_RECORD_STATUS_INITIALIZE\n"
         LOG_END
         AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_STATUS_FAIL);
         return SMTK_AUDIO_RECORD_ERROR_STATUS_FAIL;

    }

    if (pParam->nSampleRate > 48000 || pParam->nSampleRate < 6000)
    {
        LOG_ERROR
           " sample rate not support %d \n",pParam->nSampleRate
        LOG_END
        AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_SAMPLERATE_FAIL);
        return SMTK_AUDIO_RECORD_ERROR_SAMPLERATE_FAIL;
    }
    else
    {
        audioRecordMgr.nSampleRate = pParam->nSampleRate;
    }

   // if (pParam->nType> SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_MULAW|| pParam->nType < SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16)
    if (pParam->nType != SMTK_AUDIO_RECORD_TYPE_WAVE_FORMAT_PCM16)
    {
        LOG_ERROR
           " type not support %d \n",pParam->nType
        LOG_END
        AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_TYPE_FAIL);
        return SMTK_AUDIO_RECORD_ERROR_TYPE_FAIL;
    }
    else
    {
        audioRecordMgr.nRecordType= pParam->nType;
    }

    if (pParam->nChannel< 1 || pParam->nChannel > 2  )
    {
        LOG_ERROR
           " channel not support %d \n",pParam->nChannel
        LOG_END
        AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_CHANNEL_FAIL);
        return SMTK_AUDIO_RECORD_ERROR_CHANNEL_FAIL;
    }
    else
    {
        audioRecordMgr.nChannel = pParam->nChannel;
    }

    switch(pParam->nFlag & (SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_TIME |SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_SIZE ))
    {
         case 0:
            audioRecordMgr.nLimit = SMTK_AUDIO_RECORD_LIMIT_NOT_CARE;
         break;

         case SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_TIME:
            audioRecordMgr.nLimit = SMTK_AUDIO_RECORD_LIMIT_ONLY_TIME;
         break;

         case SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_SIZE:
            audioRecordMgr.nLimit = SMTK_AUDIO_RECORD_LIMIT_ONLY_SIZE;
         break;

         case (SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_TIME |SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_SIZE ):
            audioRecordMgr.nLimit = SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE;
         break;

         default:
             audioRecordMgr.nLimit = SMTK_AUDIO_RECORD_LIMIT_NOT_CARE;
         break;

     }
     LOG_DEBUG
         " nflag %d line %d func %d \n",pParam->nFlag & (SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_TIME |SMTK_AUDIO_RECORD_PARAM_FLAG_LIMIT_SIZE ),__LINE__,__FUNCTION__
     LOG_END


    if ( audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_BOTH_TIME_SIZE)
    {
        if(pParam->nLimitTime<=0)
        {
            LOG_ERROR
               " nLimitTime <=0  %d \n",pParam->nLimitTime
            LOG_END
            AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL);
            return SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL;
        }
        else if(pParam->nLimitSize <=0)
        {
            LOG_ERROR
               " nLimitSize <=0  %d \n",pParam->nLimitSize
            LOG_END
            AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_SIZE_FAIL);
            return SMTK_AUDIO_RECORD_ERROR_LIMIT_SIZE_FAIL;
        }
        audioRecordMgr.nLimitTime = pParam->nLimitTime;
        audioRecordMgr.nLimitSize = pParam->nLimitSize;

    }
    else if(audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_TIME)
    {
        if(pParam->nLimitTime<=0)
        {
            LOG_ERROR
               " nLimitTime <=0  %d \n",pParam->nLimitTime
            LOG_END
            AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL);
            return SMTK_AUDIO_RECORD_ERROR_LIMIT_TIME_FAIL;
        }
        else
        {
            audioRecordMgr.nLimitTime = pParam->nLimitTime;
            audioRecordMgr.nLimitSize = 0;
        }
    }
    else if(audioRecordMgr.nLimit == SMTK_AUDIO_RECORD_LIMIT_ONLY_SIZE)
    {
        if(pParam->nLimitSize<=0)
        {
            LOG_ERROR
               " nLimitSize <=0  %d \n",pParam->nLimitSize
            LOG_END
            AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_LIMIT_SIZE_FAIL);
            return SMTK_AUDIO_RECORD_ERROR_LIMIT_SIZE_FAIL;
        }
        audioRecordMgr.nLimitTime = 0;
        audioRecordMgr.nLimitSize = pParam->nLimitSize;

    }

    audioRecordMgr.nLineIn = pParam->nLineIn;

    wcscpy(audioRecordMgr.ptPathName, pParam->ptPathName);
    //audioRecordMgr.ptPathName =  pParam->ptPathName;
    AudioRecordPrintf(audioRecordMgr.ptPathName);
    audioRecordMgr.ptFileRecord = fopen( audioRecordMgr.ptPathName, "wb");
    if (audioRecordMgr.ptFileRecord  == NULL)
    {
#if defined(__OPENRTOS__)
     //  LOG_ERROR
     //      "Can not create audio output file %d \n",f_getlasterror()
      // LOG_END
#endif
       AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_NOT_OPEN);

       return SMTK_AUDIO_RECORD_ERROR_FILE_NOT_OPEN;
    }

    nResult = AudioRecordPackWaveHeader(pParam->nType, pParam->nChannel, nSampleRate);
    printf(" %d \n",nResult);

    if (nResult != fwrite(ptWaveHeader, sizeof(char), nResult, audioRecordMgr.ptFileRecord))
    {
        LOG_ERROR
            "Can not write audio output file\n"
        LOG_END
       AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL);
        return SMTK_AUDIO_RECORD_ERROR_FILE_LENGTH_FAIL;
    }
    else
    {
         LOG_DEBUG
            " Save Wave file header   %d \n",nResult
         LOG_END
     }
    LOG_DEBUG
       " smtkAudioRecordMgrInFile type %d nchannels %d nSampleRate %d avgBytePerSec %d lines %d \n",audioRecordMgr.nRecordType,audioRecordMgr.nChannel,audioRecordMgr.nSampleRate,audioRecordMgr.nAvgBytesPerSec,__LINE__
    LOG_END
    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_FILEOPEN;
    LOG_LEAVE
        "SmtkAudioRecordMgrInFile()=%d\r\n", nResult
    LOG_END
    return SMTK_AUDIO_RECORD_ERROR_NO_ERROR;

}

MMP_INT
smtkAudioMgrRecordStart(SMTK_AUDIO_RECORD_PARAM*   pParam)
{
    MMP_INT nResult = 0;
    MMP_UINT i;
    LOG_ENTER
        "SmtkAudioRecordMgrStart()\r\n"
    LOG_END

    nResult = smtkAudioMgrRecordSetting(pParam);
    if (nResult){
         return SMTK_AUDIO_RECORD_ERROR_STATUS_FAIL;
    }

    if (smtkAudioMgrGetState() == SMTK_AUDIO_PLAY ||smtkAudioMgrGetState() == SMTK_AUDIO_PAUSE)
    {
        LOG_DEBUG
            " audio is playing smtkAudioRecordMgrStart() %d \n",__LINE__
        LOG_END
        iteAudioStopQuick();
    }

    if (audioRecordMgr.nStatus != SMTK_AUDIO_RECORD_STATUS_FILEOPEN)
    {
         LOG_ERROR
             " nStatus != SMTK_AUDIO_RECORD_STATUS_FILEOPEN\n"
         LOG_END
         AudioRecordSetStatus(SMTK_AUDIO_RECORD_STATUS_INITIALIZE, SMTK_AUDIO_RECORD_ERROR_STATUS_FAIL);
         return SMTK_AUDIO_RECORD_ERROR_STATUS_FAIL;

    }
    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_RECORD;
    audioRecordMgr.nDataSize = 0;

    //nResult = iteAudioOpenEngine(SMTK_AUDIO_WAV);
//    HOST_WriteRegisterMask(0x166A,0xf9f9,0xffff);
    AudioRecordInitAdc(audioRecordMgr.nChannel,audioRecordMgr.nSampleRate,audioRecordMgr.nLineIn);

    sem_post(gpAudioMgrRecordSemaphore);

//    nResult = PalSetEvent(eventAudioRecordMgrToThread);
    LOG_DEBUG
       " start to record \n"
    LOG_END

    LOG_LEAVE
        "SmtkAudioRecordMgrStart()=%d\r\n", nResult
    LOG_END
    return nResult;

}


MMP_INT
smtkAudioMgrRecordPause( void )
{
    MMP_INT nResult = 0;
    MMP_UINT i;
    LOG_ENTER
        "SmtkAudioRecordMgrPause()\r\n"
    LOG_END

    //nResult = iteAudioPause(MMP_TRUE);
    LOG_DEBUG
       " SmtkAudioRecordMgrPause %d\n",nResult
    LOG_END
    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_PAUSE;

    LOG_LEAVE
        "SmtkAudioRecordMgrPause()=%d\r\n", nResult
    LOG_END
    return nResult;

}

MMP_INT
smtkAudioMgrRecordResume( void )
{
    MMP_INT nResult = 0;
    MMP_UINT i;
    LOG_ENTER
        "SmtkAudioRecordMgrResume()\r\n"
    LOG_END

    //nResult = iteAudioPause(MMP_FALSE);
    LOG_DEBUG
       " SmtkAudioRecordMgrResume %d\n",nResult
    LOG_END

    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_RECORD;
    sem_post(gpAudioMgrRecordSemaphore);

    //nResult = PalSetEvent(eventAudioRecordMgrToThread);
    LOG_DEBUG
       " record to resume \n"
    LOG_END

    LOG_LEAVE
        "SmtkAudioRecordMgrResume()=%d\r\n", nResult
    LOG_END
    return nResult;

}

MMP_INT
smtkAudioMgrRecordStop( void )
{
    MMP_INT nResult = 0;

    LOG_ENTER
        "SmtkAudioRecordMgrStop()\r\n"
    LOG_END

    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_INITIALIZE;
    audioRecordMgr.nDataSize = 0;
//    nResult = PalSetEvent(eventAudioRecordMgrToThread);

    LOG_LEAVE
        "SmtkAudioRecordMgrStop()=%d\r\n", nResult
    LOG_END
    return nResult;
}

MMP_INT
smtkAudioMgrRecordGetStatus(
  SMTK_AUDIO_RECORD_INFO* tEncodeInfo)
{
    MMP_INT nResult = 0;

    LOG_ENTER
        "SmtkAudioRecordMgrGetStatus()\r\n"
    LOG_END

    tEncodeInfo->nError= audioRecordMgr.nError;
//    iteAudioGetEncodeTime(&nResult);
    nResult =AudioRecordGetRecordTime();
    audioRecordMgr.nCurrentTime = nResult;
    tEncodeInfo->nEncodeTime = nResult;
    tEncodeInfo->nEncodeStatus = audioRecordMgr.nStatus;
    nResult = SMTK_AUDIO_RECORD_ERROR_NO_ERROR;
    LOG_LEAVE
        "SmtkAudioRecordMgrGetStatus()=%d\r\n", nResult
    LOG_END
    return nResult;

}


MMP_INT
smtkAudioMgrRecordTerminate(
    void)
{
    MMP_INT nResult = 0;
    LOG_ENTER
        "smtkAudioRecordMgrTerminate()\r\n"
    LOG_END

    sem_destroy(gpAudioMgrRecordSemaphore);
/*
    nResult = PalSetEvent(eventAudioRecordMgrToThread);
    nResult = PalWaitEvent(eventAudioRecordThreadToMgr, PAL_EVENT_INFINITE);
    nResult = PalDestroyEvent(eventAudioRecordMgrToThread);
    nResult = PalDestroyEvent(eventAudioRecordThreadToMgr);
    nResult = PalDestroyThread(thread);*/

    if(gpRecordI2sBuf)
    {
        free(gpRecordI2sBuf);
    }

    if (audioRecordMgr.ptRecordBuf)
    {
        free(audioRecordMgr.ptRecordBuf);
        audioRecordMgr.ptRecordBuf = MMP_NULL;
    }

    if(audioRecordMgr.ptPathName)
    {
        free(audioRecordMgr.ptPathName);
    }

    if ( audioRecordMgr.ptFileRecord)
    {
        fclose(audioRecordMgr.ptFileRecord);
        audioRecordMgr.ptFileRecord = NULL;
        LOG_DEBUG
           " Recode file close %s \n",__FUNCTION__
        LOG_END
    }
    audioRecordMgr.nStatus = SMTK_AUDIO_RECORD_STATUS_INITIALIZE;

    LOG_LEAVE
        "smtkAudioRecordMgrTerminate()=%d\r\n", nResult
    LOG_END
    return nResult;
}

#endif



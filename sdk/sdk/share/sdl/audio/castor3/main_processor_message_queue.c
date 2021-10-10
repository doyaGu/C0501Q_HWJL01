#if 1//defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

#include "main_processor_message_queue.h"
#include "ite/audio.h"
#include "i2s/i2s.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#if defined(__FREERTOS__)
#include "api_f.h"
#endif
#define TOINT(n) ((((n)>>24)&0xff)+(((n)>>8)&0xff00)+(((n)<<8)&0xff0000)+(((n)<<24)&0xff000000))
#define TOSHORT(n) ((((n)>>8)&0x00ff)+(((n)<<8)&0xff00))
int mainProcessorParsingAudioPluginCmd(unsigned short nRegisterStatus);
static unsigned int tDumpWave[256] ;

static FILE *fout = NULL;
static unsigned char ptWaveHeader[44];
static ITE_WmaInfo gWmaInfo;
static unsigned long gnDataSize;
static int gSave=0;
static int *ptThread;
static char cSpecTrum[100];

static int MainProcessorPackWaveHeader(int nChannels,int nSampleRate,int nDataSize)
{
    int samplesPerBlock = 0;
    int nBlockAlign =0;
    unsigned int nAvgBytesPerSec = 0;
    // Chunk ID "RIFF"
    ptWaveHeader[0] = 'R';
    ptWaveHeader[1] = 'I';
    ptWaveHeader[2] = 'F';
    ptWaveHeader[3] = 'F';
    printf("[Main Processor Message Queue] wave nChennels %d nSampleRate %d nDataSize %d \n",nChannels,nSampleRate,nDataSize);

    // Chunk Size (4+n)
    ptWaveHeader[4] = (unsigned char)(((nDataSize+36)>>  0) & 0xff);
    ptWaveHeader[5] = (unsigned char)(((nDataSize+36) >> 8) & 0xff);
    ptWaveHeader[6] = (unsigned char)(((nDataSize+36) >> 16) & 0xff);
    ptWaveHeader[7] = (unsigned char)(((nDataSize+36) >> 24) & 0xff);

    ptWaveHeader[40] = (unsigned char)((nDataSize>>  0) & 0xff);
    ptWaveHeader[41] = (unsigned char)((nDataSize >> 8) & 0xff);
    ptWaveHeader[42] = (unsigned char)((nDataSize >> 16) & 0xff);
    ptWaveHeader[43] = (unsigned char)((nDataSize >> 24) & 0xff);

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
    ptWaveHeader[16] = 16;
    ptWaveHeader[17] = 0;
    ptWaveHeader[18] = 0;
    ptWaveHeader[19] = 0;
    ptWaveHeader[20] = 1;
    ptWaveHeader[21] = 0;

    // Channel
    ptWaveHeader[22] = nChannels;
    ptWaveHeader[23] = 0;

    // Sampling rate
    ptWaveHeader[24] = (unsigned char)((nSampleRate >>  0) & 0xff);
    ptWaveHeader[25] = (unsigned char)((nSampleRate >>  8) & 0xff);
    ptWaveHeader[26] = (unsigned char)((nSampleRate >> 16) & 0xff);
    ptWaveHeader[27] = (unsigned char)((nSampleRate >> 24) & 0xff);
    ptWaveHeader[34] = 16;
    ptWaveHeader[35] = 0;

    // Data block size (dont care)  block align = bits per sample / 8 * channels
    nBlockAlign = ptWaveHeader[34] /8 * nChannels;
    ptWaveHeader[32] = (unsigned char)((nBlockAlign>>  0) & 0xff);
    ptWaveHeader[33] = (unsigned char)((nBlockAlign>>  8) & 0xff);


    // Data rate (dont care)  Average byte per seconds == sample rate * block align
    nAvgBytesPerSec = nSampleRate * nBlockAlign;
    ptWaveHeader[28] = (unsigned char)(((nSampleRate*nBlockAlign) >>  0) & 0xff);
    ptWaveHeader[29] = (unsigned char)(((nSampleRate*nBlockAlign) >>  8) & 0xff);
    ptWaveHeader[30] = (unsigned char)(((nSampleRate*nBlockAlign) >> 16) & 0xff);
    ptWaveHeader[31] = (unsigned char)(((nSampleRate*nBlockAlign) >> 24) & 0xff);
    ptWaveHeader[36] = 'd';
    ptWaveHeader[37] = 'a';
    ptWaveHeader[38] = 't';
    ptWaveHeader[39] = 'a';
    return 0;
}

int
mainProcessorParsingAudioPluginCmd(
    unsigned short nRegisterStatus)
{
    unsigned int nTemp;

    // parsing audio cpu id
    nTemp = (nRegisterStatus & 0xc000)>>14;
    //printf("main processor parsing cpu id %d \n",nTemp);
    if (nTemp!=SMTK_AUDIO_PROCESSOR_ID) {
        //dbg_msg(DBG_MSG_TYPE_ERROR, "[MainProcessorExecuteAudioPlugin] Parsing error SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CPU_ID %d #line %d \n",nTemp,__LINE__);
        return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CPU_ID;
    }

    // parsing audio plugin cmd
    nTemp = nRegisterStatus & 0x3fff;
    if (nTemp ==0 || nTemp > SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF) {
        //dbg_msg(DBG_MSG_TYPE_ERROR, "[MainProcessorExecuteAudioPlugin] Parsing error SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CMD_ID %d #line %d \n",nTemp,__LINE__);
        return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CMD_ID;
    }

    return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NO_ERROR;

}

int
mainProcessorExcuteFileOpen()
{
    int nResult=0;
    uint32_t nLength;
    unsigned short* pBuf;
    unsigned int nTemp;
    int nChannels;
    int nSampeRate;
#if defined(__OPENRTOS__)
    pBuf = (unsigned char*)iteAudioGetAudioCodecAPIBuffer(&nLength);
#endif
    //memset(&tDumpWave, 0, sizeof(tDumpWave));

    //memcpy(&nTemp, &pBuf[0], SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);
    //memcpy(&tDumpWave[0], &pBuf[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], nTemp);

    //printf("mainProcessorExcuteFileOpen name length %d ,%c%c%c%c #line %d \n",nTemp,pBuf[2],pBuf[3],pBuf[4],pBuf[5],__LINE__);
#if 1 //defined(__FREERTOS__)
    //f = fopen( "A:/mp3.codecs", "rb");

    if (!fout) {
        fout = fopen("A:/dumpAudio.wav", "wb");
        printf("[Main Processor Message Queue] file  open  \n");
        if (fout == NULL) {
            printf("[Main Processor Message Queue] Can not create audio output file  #line %d \n",__LINE__);
            return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_FILE_OPEN_FAIL;
        }
        iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_CHANNEL, &nChannels);
        iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nSampeRate);
        gnDataSize = 44;
        MainProcessorPackWaveHeader(nChannels,nSampeRate,gnDataSize);
        fwrite(ptWaveHeader, sizeof(char), sizeof(ptWaveHeader), fout);
        return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NO_ERROR;
    }
#endif
    return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_FILE_OPEN_FAIL;
}

int
mainProcessorExcuteFileWrite()
{
    int nResult=0;
    uint32_t nLength;
    unsigned int* pt;
    unsigned char* pBuf;
    unsigned int nTemp;
#if defined(__OPENRTOS__)

    pBuf = (unsigned int*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    pt = (unsigned int*)iteAudioGetAudioCodecAPIBuffer(&nLength);

    nTemp = TOINT(pt[0]);
    //nTemp = 4096;
   // gSave++;
    //if (gSave == 1100){
    //    mainProcessorExcuteFileClose();
    //    while(1);
    //}
#if 1
    if (fout)
        fwrite(&pBuf[SMTK_AUDIO_PLUGIN_CMD_FILE_WRITE_LENGTH], sizeof(char), nTemp, fout);

#endif
    gnDataSize=gnDataSize+ nTemp ;
    //printf("[Main Processor Message Queue] write %d 0x%x 0x%x nDataSize %d %d\n",nTemp,TOINT(pt[0]),pt[0],gnDataSize,gSave);
#endif
    return SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NO_ERROR;

}

int
mainProcessorExcuteFileRead()
{

return 0;
}

int
mainProcessorExcuteFileClose()
{
    int nResult=0;
    int nChannels;
    int nSampeRate;

    iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_CHANNEL, &nChannels);
    iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nSampeRate);
    MainProcessorPackWaveHeader(nChannels,nSampeRate,gnDataSize);
#if 1 //defined(__FREERTOS__)
    if (fout) {
        fseek(fout, 0, SEEK_SET);
        fwrite(ptWaveHeader, sizeof(char), sizeof(ptWaveHeader), fout);
        printf("[Main Processor Message Queue] PalFileClose\n");
        fclose(fout);
        fout = NULL;
    }
#endif
    return nResult;

}
int mainProcessorExcuteI2SInitDac()
{
    unsigned char* I2SBuf;
    int nChannels;
    int nSampeRate;
    int nBufferLength;
    int nTemp = 1;
    unsigned int* pBuf;
    unsigned int nLength;
    STRC_I2S_SPEC spec;
#if defined(__OPENRTOS__)

    pBuf = (unsigned int*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    I2SBuf = TOINT(pBuf[0])+iteAudioGetAudioCodecBufferBaseAddress();
    nChannels = TOINT(pBuf[1]);
    nSampeRate = TOINT(pBuf[2]);
    nBufferLength = TOINT(pBuf[3]);
    iteAudioSetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nSampeRate);
    iteAudioSetAttrib(ITE_AUDIO_CODEC_SET_CHANNEL, &nChannels);
    iteAudioSetAttrib(ITE_AUDIO_CODEC_SET_BUFFER_LENGTH, &nBufferLength);    
    iteAudioSetAttrib(ITE_AUDIO_I2S_INIT, &nTemp);
    iteAudioSetAttrib(ITE_AUDIO_I2S_PTR, I2SBuf);    

    printf("[Main Processor Message Queue]sdl  I2SInitDac 0x%x  %d %d %d \n",I2SBuf,nChannels,nSampeRate,nBufferLength);
    if (nChannels>2 || nChannels<=0){
        //smtkAudioMgrSetNetworkError(1);
        return 0;
    }
    if (nSampeRate>48000 || nSampeRate<8000){
        //smtkAudioMgrSetNetworkError(1);
        return 0;
    }

//    initDAC(I2SBuf, nChannels, nSampeRate, nBufferLength, 1,0);

    /* init I2S */
    if(i2s_get_DA_running)
        i2s_deinit_DAC();
    memset(&spec,0,sizeof(spec));
    spec.channels                 = nChannels;
    spec.sample_rate              = nSampeRate;
    spec.buffer_size              = nBufferLength;
    spec.is_big_endian            = 1;
    spec.base_i2s                 = I2SBuf;

    spec.enable_Speaker          = 1;
    spec.sample_size              = 16;
    spec.num_hdmi_audio_buffer    = 1;
    spec.is_dac_spdif_same_buffer = 1;
    #if (CFG_CHIP_FAMILY == 9850)
    #else
    spec.base_hdmi[0]             = I2SBuf;
    spec.base_hdmi[1]             = I2SBuf;
    spec.base_hdmi[2]             = I2SBuf;
    spec.base_hdmi[3]             = I2SBuf;
    spec.base_spdif               = I2SBuf;
    #endif

    i2s_init_DAC(&spec);
#endif


    return 0;
}

int mainProcessorExcuteI2SInitAdc()
{
    int I2SBuf;
    int nChannels;
    int nSampeRate;
    int nBufferLength;
    unsigned int* pBuf;
    unsigned int nLength;

#if defined(__OPENRTOS__)
    pBuf = (unsigned int*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    I2SBuf = TOINT(pBuf[0]);
    nChannels = TOINT(pBuf[1]);
    nSampeRate = TOINT(pBuf[2]);
    nBufferLength = TOINT(pBuf[3]);
    //memcpy(&I2SBuf, &pBuf[0], SMTK_AUDIO_PLUGIN_CMD_I2S_BUFFER_ADDRESS_LENGTH);
    //memcpy(&nChannels, &pBuf[SMTK_AUDIO_PLUGIN_CMD_I2S_BUFFER_ADDRESS_LENGTH], SMTK_AUDIO_PLUGIN_CMD_I2S_CHANNELS_LENGTH);
    //memcpy(&nSampeRate, &pBuf[8], SMTK_AUDIO_PLUGIN_CMD_I2S_SAMPLERATE_LENGTH);
    //memcpy(&nBufferLength, &pBuf[12], SMTK_AUDIO_PLUGIN_CMD_I2S_BUFFER_LENGTH);
    //initDAC(I2SBuf, nChannels, nSampeRate, nBufferLength, 0,0);
#endif
    printf("[Main Processor Message Queue] 1 I2SInitDac 0x%x  %d %d %d \n",I2SBuf,nChannels,nSampeRate,nBufferLength);
    return 0;
}

int
mainProcessorExcutePauseDAC()
{
    unsigned char* pBuf;
    uint32_t nLength;
    int nTemp;
#if defined(__OPENRTOS__)
    pBuf = (unsigned int*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    nTemp = TOINT(pBuf[0]);
    iteAudioGetAttrib(ITE_AUDIO_IS_PAUSE, &nTemp);  
  
    //memcpy(&nTemp, &pBuf[0], 4);
    //pauseDAC(nTemp);
    i2s_pause_DAC(nTemp);
#endif
    printf("[Main Processor Message Queue] pauseDAC %d \n",nTemp);
    return 0;
}

int mainProcessorExcuteDeactiveDAC()
{
    //deactiveDAC();
#if defined(__OPENRTOS__)
    i2s_deinit_DAC();
#endif
    //ithInvalidateDCache();
    printf("[Main Processor Message Queue] deactiveDAC \n");
    return 0;
}

int mainProcessorExcuteDeactiveADC()
{
    //deactiveADC();

#if defined(__OPENRTOS__)
    i2s_deinit_ADC();
#endif
    printf("[Main Processor Message Queue] deactiveADC \n");
    return 0;
}

int mainProcessorExcutePrintf()
{
    unsigned char* pBuf;
    unsigned  int nLength;
    int nTemp;

#if defined(__OPENRTOS__)
    pBuf = (unsigned char*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    puts(pBuf);
    memset(pBuf, 0, nLength);
#ifdef CFG_CPU_WB
    ithFlushDCacheRange(pBuf, nLength);
    ithFlushMemBuffer();
#endif
#endif
    return 0;
}

int mainProcessorExcuteWmaInfo()
{
    unsigned char* pBuf;
    unsigned  int nLength;
    int nTemp;
    printf("[Main Processor Message Queue] mainProcessorExcuteWmaInfo \n");

#if defined(__OPENRTOS__)
    pBuf = (unsigned char*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    memcpy(&gWmaInfo,pBuf,sizeof(gWmaInfo));
    iteAudioSaveWmaInfo(gWmaInfo);
#endif
    return 0;
}

int mainProcessorExcuteSpectrum()
{
#if defined(CFG_BUILD_AUDIO_MGR)
    unsigned int* pBuf;
    unsigned int nLength;
    int nTemp;
 
#if defined(__OPENRTOS__)

    pBuf = (unsigned char*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    nTemp = TOINT(pBuf[0]);

    memcpy(cSpecTrum,&pBuf[4],sizeof(cSpecTrum));
    //printf("[Main Processor Message Queue] spectrum %d [0x%x] 0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x, 0x%x, 0x%x",nTemp,cSpecTrum,cSpecTrum[0],cSpecTrum[1],cSpecTrum[2],cSpecTrum[3],cSpecTrum[4],cSpecTrum[5],cSpecTrum[6],cSpecTrum[7],cSpecTrum[8]);
    //printf("0x%x, 0x%x, 0x%x, 0x%x,  0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n",cSpecTrum[9],cSpecTrum[10],cSpecTrum[11],cSpecTrum[12],cSpecTrum[13],cSpecTrum[14],cSpecTrum[15],cSpecTrum[16],cSpecTrum[17],cSpecTrum[18],cSpecTrum[19]);
    // set spectrum to audio mgr
    smtkAudioMgrSetSpectrum(cSpecTrum);
#endif

#endif
    return 0;
}



int
mainProcessorExcuteAudioPluginCmd(
    unsigned short nRegisterStatus)
{
    unsigned int nTemp;
    int nResult=0;

    //ithInvalidateDCache();

    // execute audio plugin cmd
    nTemp = nRegisterStatus & 0x3fff;
    //printf("[Main Processor] cmd  %d\n",nTemp);
    switch (nTemp)
    {
        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN:
        {
            nResult = mainProcessorExcuteFileOpen();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
        {
            nResult = mainProcessorExcuteFileWrite();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_READ:
        {
            nResult = mainProcessorExcuteFileRead();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE:
        {
            nResult = mainProcessorExcuteFileClose();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC:
        {
            nResult = mainProcessorExcuteI2SInitDac();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC:
        {
            nResult = mainProcessorExcuteI2SInitAdc();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_CODEC:
        {
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
        {
            nResult = mainProcessorExcutePauseDAC();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_ADC:
        {
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
        {
            nResult = mainProcessorExcuteDeactiveDAC();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_ADC:
        {
            nResult = mainProcessorExcuteDeactiveADC();

            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_SLEEP:
        {
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_GET_CLOCK:
        {
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_GET_DURATION:
        {
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:
        {
            nResult = mainProcessorExcutePrintf();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_WMA_INFO:
        {
            nResult = mainProcessorExcuteWmaInfo();
            break;
        }

        case SMTK_AUDIO_PLUGIN_CMD_ID_SPECTRUM:
        {
            nResult = mainProcessorExcuteSpectrum();
            break;
        }

        default:
            break;
    }
    return nResult;

}


int
smtkMainProcessorExecuteAudioPluginCmd(
    unsigned short nRegisterStatus)
{
    int nResult=0;
    unsigned short nRegister;
    if (pthread_self()!=ptThread){
        ptThread = pthread_self();
        printf("[Main Processor] thread 0x%x 0x%x \n",ptThread,pthread_self());
    }

    // parsing audio plugin cmd
    nResult =  mainProcessorParsingAudioPluginCmd(nRegisterStatus);
    if (nResult) {
        //dbg_msg(DBG_MSG_TYPE_ERROR, "[MainProcessorExecuteAudioPlugin] Parsing error %d #line %d \n",nResult,__LINE__);
    }
    // execute audio plugin cmd
    //ithInvalidateDCache();
    nResult =  mainProcessorExcuteAudioPluginCmd(nRegisterStatus);
    if (nResult) {
        //dbg_msg(DBG_MSG_TYPE_ERROR, "[MainProcessorExecuteAudioPlugin] execute audio plugin api error %d #line %d \n",nResult,__LINE__);
    }
    // write audio plugin register
    nRegister = SMTK_MAIN_PROCESSOR_ID << 14 | nResult;
    //printf("[Main processor] write 0x%x \n",nRegister);
    setAudioPluginMessageStatus(nRegister);

    return nResult;

}


#endif


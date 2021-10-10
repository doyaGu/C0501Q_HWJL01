#include <stdio.h>
//#include <string.h>

#ifdef CFG_AUDIO_MGR_RESAMPLE

#include "audio_mgr.h"


#include "wavfilefmt.h"
#include "resample.h"

#define AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN            (64 * 1024)
#define FRAME_BYTES (4*512)

#define getFreeLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ? ((len) - ((wrptr) - (rdptr)) - 2) : ((rdptr) - (wrptr) - 2))
#define getOverWriteLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ?  ((wrptr) - (rdptr) - 2) : ((len) - ((rdptr)-(wrptr)) - 2) )

typedef int(*AudioPlayInterruptCallback)(int state);

int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize);
int getResampleAvailableLength(ResampleAudioContext *audioContext);
static short pcm8_linear(char pcm_val);
static int copy1chTo2ch(int nLength);


//==========================================================================================================================
//	Globals
//==========================================================================================================================
static int gnResampleTask = 0;
static int gnInterruptStart = 0;
//static pthread_t  resampleThreadID=0;
static ResampleAudioContext gResampleAudioContext;
static AVResampleContext avResampleContext;
static FILE *fin = NULL;
static void *Local_player_http_handle = NULL;

static int gnOutch;
static int gnI2SBufLen;

static char* gOutBuffer= NULL;
static short writeBuffer[8*1024];

static  struct timeval gStart, gEnd;

// if input 1 ch , output 2 ch
static short writeBuffer2ch[2*8*1024];
static unsigned int gOutReadPointer=0;
static unsigned int gOutWritePointer=0;
static unsigned int gOutToI2SWP=-1;
static AudioPlayInterruptCallback resampleInterruptCallback;

static int gOverwriteStatus = 0;
static int gInterruptStatus = 0;

// play interrupt sound and music, remove card
static int gInterruptRemoveCard = 0;

static int nDelete = 0;

//#define RESAMPLE_DEBUG

#define TEST_BUFFER

//#define TEST_RESAMPLE
#ifdef TEST_RESAMPLE
// for test
static FILE *fout = NULL;
static char *out = "C:\\P3-48000(8k).wav";
#endif

#ifdef TEST_BUFFER
static char gFileBuffer[128*1024];
static int gFileReadPointer=0;
static int gFileSize=0;

#endif


#ifdef TEST_BUFFER
static int readFileInBuffer(FILE * fp)
{
    unsigned long ulFileSize = 0;
    fseek(fp, 0, SEEK_END);
    ulFileSize = (unsigned long) ftell(fp);
    fseek(fp, 44, SEEK_SET);
    gFileSize = (int) ulFileSize ; 
    gFileReadPointer = 0;
    printf("readFileInBuffer %d \n",gFileSize);        
    
    if (fread(gFileBuffer,1,gFileSize,fp)!=gFileSize){
        printf("readFileInBuffer error  %d \n",gFileSize);        
    }

}

#endif


static int copy1chTo2ch(int nLength)
{
    int i,j;
    if (nLength>0 && nLength<8*1024){

    } else {
        printf("copyCh1data data length error ,#line %d \n",__LINE__);
        return -1;
    }
   
    for (i=0,j=0; i<nLength/2;i++,j+=2){
        writeBuffer2ch[j] = writeBuffer[i];
        writeBuffer2ch[j+1] = writeBuffer[i];
    }
    return 0;
}

static void resample_output_i2s(char *outbuf , int nSize)
{
    int tmp;
    int nTemp;
    int i,j;
    int nOverWriteLength = 0;
    
    char* ptBuf;
    unsigned int res;
    
#if defined(__OPENRTOS__)

    if (nSize <=0) {
        return;        
    }
    i =0;

#if 0    
    // Wait output buffer avaliable
    do {
        gOutReadPointer = I2S_DA32_GET_RP();
        gOutWritePointer = I2S_DA32_GET_WP();
        if (gOutReadPointer <= gOutWritePointer) {
            nTemp = gnI2SBufLen - (gOutWritePointer - gOutReadPointer);
        } else {
            nTemp = gOutReadPointer - gOutWritePointer;
        }
        if ((nTemp-2) < nSize) {
            //printf("[Audio mgr] buffer full  %d %d \n",nTemp,nSize);
            usleep(20);
            i++;
            if (i%1000 ==0){
                printf("[Audio mgr] resample_output_i2s buffer full  %d %d %d \n",nTemp,nSize,i);
            }
        } else {
            break;
        }
    } while(1);
#endif

    if (gOutToI2SWP==-1){
        // first wirte
        // get current i2s read pointer
        // may be avoid noise here
        gOutToI2SWP = I2S_DA32_GET_RP();
#ifdef RESAMPLE_DEBUG
        printf("[Audio mgr]  resample first write \n");
#endif
        iteAudioSetOutputEmpty(1);
            
    } else {


    }
    
    do {
        nOverWriteLength = getOverWriteLen(gOutToI2SWP,I2S_DA32_GET_WP(),gnI2SBufLen);
        if (checkDelete()){
            return;
        }
        
        if ((nOverWriteLength) < nSize) {
            //printf("[Audio mgr] buffer full  %d %d \n",nTemp,nSize);
            usleep(20);
            i++;
            if (i%3000 ==0){
                printf("[Audio mgr] resample_output_i2s buffer full  %d %d %d \n",nOverWriteLength,nSize,i);
            }
        } else {
            break;
        }        
    } while(1);

    if (checkDelete()){
        return;
    }

    // overwrite data
    if (gOutToI2SWP+(nSize) < gnI2SBufLen){
        memcpy(gOutBuffer+ gOutToI2SWP, outbuf, nSize);
        gOutToI2SWP += nSize;
        //gOutWritePointer += nSize;
        //if(gOutWritePointer >= gnI2SBufLen) {
        //    gOutWritePointer = 0;
        //}
        //I2S_DA32_SET_WP(gOutWritePointer);
    }else{
        nTemp = gnI2SBufLen - gOutToI2SWP;
        memcpy(gOutBuffer + gOutToI2SWP, outbuf, nTemp);
        gOutToI2SWP += nTemp;
        //I2S_DA32_SET_WP(gOutWritePointer);
        tmp = nSize - nTemp;
        memcpy(gOutBuffer, &outbuf[nTemp], tmp);
        gOutToI2SWP = tmp;
        //I2S_DA32_SET_WP(gOutWritePointer);
        //audioMgr.ptNetwork.nDecodeTime +=( (gnI2SBufLen/(audioMgr.ptNetwork.nSampleRate*(audioMgr.ptNetwork.nBitPerSample/8)*audioMgr.ptNetwork.nChannels))*1000);
        //printf("\n audio_output_i2s %d %d %d %d %d\n",audioMgr.ptNetwork.nDecodeTime,audioMgr.ptNetwork.nSampleRate,audioMgr.ptNetwork.nBitPerSample,audioMgr.ptNetwork.nChannels);
        //malloc_stats();
    }
 
#endif

}

int checkDelete()
{
#if defined(__OPENRTOS__)
    if (i2s_get_DA_running()==0){
        printf("resample check delete, not i2s #line %d \n",__LINE__);
        return 1;
    }

#endif

    return nDelete;
}

int resampleThread()
{
    char  readBuffer8[8*1024];
    short readBuffer[8*1024];

    int done = 0;
    int nTemp,i,j;
    int nSize;
    int consumed,lenout,nReSampleOutputSize;
    int nOutCh;
    char* outptr;
    int outLength;
    int emptyLength=0;    
    int nRead = -1;

#ifdef TEST_RESAMPLE
    if ((fout=fopen(out, "wb")) == NULL) {
        printf("Can not open file '%s'\n", out);        
    }
    nOutCh = 1;
    WriteWAVHeader(fout, gResampleAudioContext.nOutSampleRate, 16, nOutCh);    
    nRead = 0;
#endif

#ifdef RESAMPLE_DEBUG
    printf("resampleThread start  #line %d \n",__LINE__);
#endif
    gOutToI2SWP=-1;
    nSize = gResampleAudioContext.nInSize;
    
    av_resample_init(&avResampleContext,gResampleAudioContext.nOutSampleRate, gResampleAudioContext.nInSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
    
    do {
        // read input audio
        if (gResampleAudioContext.nInBitsPerSample == 16) {
#ifdef TEST_BUFFER
            if (gFileReadPointer+nSize <= gFileSize) {
                memcpy(readBuffer,&gFileBuffer[gFileReadPointer],nSize);
                gFileReadPointer+=nSize;
            } else {
                printf("TEST_BUFFER resample eof  %d \n",gFileReadPointer);
                if (gFileSize>gFileReadPointer){
                    memcpy(readBuffer,&gFileBuffer[gFileReadPointer],gFileSize-gFileReadPointer);  
                    gFileReadPointer+=(gFileSize-gFileReadPointer);
                } else {
                    break;
                }

                done = 1;
            }
            
#else            
            if (fread(readBuffer,1,nSize,fin)!=nSize){
                printf("resample eof  %d \n",nRead);
                done = 1;
                //break;
            }
            
            nRead+= nSize;
#endif            
        } else if (gResampleAudioContext.nInBitsPerSample == 8) {
            nTemp = nSize/2;
            if (fread(readBuffer8,1,nTemp,fin)!=nTemp){
                printf("eof \n");
                done = 1;
                break;
            }

//        8 to 16
            for(i = 0; i < nTemp; i++) {
                short *s = (short*)&readBuffer[i];
                {
                   *s = pcm8_linear(readBuffer8[i]);                   
                }
            }
//
        }
        // check resample temp buffers
        if (gResampleAudioContext.nUseTempBuffer == 1){
            nTemp = getResampleAvailableLength(&gResampleAudioContext);
            //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
            if (nTemp>=gResampleAudioContext.nInChannels*MAX_FRAMESIZE){
                printf("getResampleAvailableLength %d > %d  \n",nTemp,gResampleAudioContext.nInChannels*MAX_FRAMESIZE);
                getResampleEncodeBuffer(&gResampleAudioContext,writeBuffer,gResampleAudioContext.nInChannels*MAX_FRAMESIZE);
                // output
                //fwrite(writeBuffer,1,gResampleAudioContext.nInChannels*2*MAX_FRAMESIZE,fout);
            }

        }

        if (gResampleAudioContext.nInChannels == 2) {
            for(i = 0,j=0; i < nSize/2; i+=2,j++){
               gResampleAudioContext.reSamplePcmInput[0][gResampleAudioContext.nKeep[0]+j] = readBuffer[i];
               gResampleAudioContext.reSamplePcmInput[1][gResampleAudioContext.nKeep[1]+j] = readBuffer[i+1];
            }
        } if (gResampleAudioContext.nInChannels == 1) {
            for(i = 0; i < nSize; i++){
               gResampleAudioContext.reSamplePcmInput[0][gResampleAudioContext.nKeep[0]+i] = readBuffer[i];
               //resampleAudioContext.reSamplePcmInput[1][resampleAudioContext.nKeep[1]+j] = readBuffer[i+1];
            }
        }
        if (checkDelete()){
            break;
        }
        // resample
        nTemp = resample(&avResampleContext,&gResampleAudioContext,writeBuffer);
        //if (nTemp != 4*MAX_FRAMESIZE)
        //    printf("resample output %d \n",nTemp);
        
        //nTemp = 4*MAX_FRAMESIZE;

        // output
        // prepare output data
        if (checkDelete()){
            break;
        }

        // Byte swap to little endian
        {
            char *buf = (char *)writeBuffer;
            char *in  = (char *)writeBuffer;
            for(i=0; i<nTemp; i+=sizeof(short))
            {
                buf[i]   = in[i+1];
                buf[i+1] = in[i];
            }
        }
        
        // if input  channel == ouput  channel
        if (gResampleAudioContext.nInChannels == gnOutch){
            // use writeBuffer
            outptr = (char*)writeBuffer;
            outLength = nTemp;
        } else if (gResampleAudioContext.nInChannels ==1 && gnOutch==2){
            // use writeBuffer2ch
            copy1chTo2ch(nTemp);
            outptr = (char*)writeBuffer2ch;
            outLength = nTemp*2;
            
        } else {
            printf(" input ch %d output ch %d ,not support \n",gResampleAudioContext.nInChannels,gnOutch);
        }

        if (checkDelete()){
            break;
        }
       
        // output to i2s
        resample_output_i2s(outptr,outLength);

        if (checkDelete()){
            break;
        }

        // 
#ifdef TEST_RESAMPLE        
        fwrite(outptr,1,outLength,fout);
#endif
    } while (!done);
    fclose(fin);

//    printf("resampleThread finish \n");
//    smtkAudioMgrInterruptSoundFinish();

    done = 0;
    emptyLength = gnOutch*2*gResampleAudioContext.nOutSampleRate;
    nTemp = sizeof(writeBuffer);
#ifdef RESAMPLE_DEBUG    
    printf("output empty %d %d\n",emptyLength,nTemp);
#endif
    memset(writeBuffer,0,sizeof(writeBuffer));
    i=0;
    // output 1s empty 
    do {
        outptr = (char*)writeBuffer;
        outLength = nTemp;        
        // output to i2s
        resample_output_i2s(outptr,outLength);
        emptyLength-= nTemp;
        
        if (i==0){
#ifdef RESAMPLE_DEBUG            
            printf("resampleThread finish #line %d \n",__LINE__);
#endif
            if (nDelete==0)
                smtkAudioMgrInterruptSoundFinish();
        }
        if (checkDelete()){
            usleep(20000);
            break;
        }
        i++;

        if (emptyLength<=0)
            done=1;
        
    } while (!done && emptyLength>=0);
#ifdef RESAMPLE_DEBUG
    printf("output empty end %d i %d\n",i,emptyLength);
#endif


#if defined(__OPENRTOS__)
    smtkAudioMgrSetInterruptStatus(0);

    if (checkDelete()){

    } else {
        printf("[Audio mgr] resample thread finish,set output empty disable #line %d \n",__LINE__);
        if (!resampleInterruptCallback)
            iteAudioSetOutputEmpty(0);
    }

#endif
    nDelete = 0;


#ifdef TEST_RESAMPLE    
    fclose(fout);
#endif

}

static int createThread()
{
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);    
    pthread_attr_setstacksize(&attr, SMTK_AUDIO_MGR_READ_BUFFER_STACK_SIZE);
    pthread_create(&task, &attr, resampleThread, NULL);
//    resampleThreadID = task;
    gettimeofday(&gStart, NULL);    
    usleep(30000);
    gnResampleTask = 1;    


    return 0;

}

static int deleteThread()
{
    int nTemp =0;
    int nUnmute = 0;
    

#if defined(__OPENRTOS__)   
    if (i2s_get_DA_running() && smtkAudioMgrGetInterruptStatus()==1){
        i2s_mute_DAC(1);
        memset(gOutBuffer,0,gnI2SBufLen);
        nUnmute = 1;
    }
    if (1){
        gettimeofday(&gEnd, NULL);
        nDelete = 1;
        do {
            usleep(20000);
            nTemp++;
        } while (nDelete==1 && nTemp <20);
#ifdef RESAMPLE_DEBUG        
        printf("[Audio mgr]delete resample task delete %d ,%d ,#line %d\n",nDelete,itpTimevalDiff(&gStart,&gEnd),__LINE__);
#endif

        if (nUnmute==1)
            i2s_mute_DAC(0);
        
/*    if (i2s_get_DA_running()){
        i2s_mute_DAC(1);
        memset(gOutBuffer,0,gnI2SBufLen);
        i2s_mute_DAC(0);
    }*/


//        if (itpTimevalDiff(&gStart,&gEnd)<1500)
//            usleep(60000);
        
//        pthread_cancel(resampleThreadID);
//        resampleThreadID = 0;
        gnResampleTask = 0;
        nDelete = 0;
        memset(&gResampleAudioContext,0,sizeof(ResampleAudioContext));
        usleep(2000);
    }
#endif
}

int getResampleEncodeBuffer(ResampleAudioContext *audioContext,short* writeptr,int nSize){
    // copy to encode buffer
    if (audioContext->nTempBufferRdPtr+nSize < audioContext->nTempBufferLength){
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],nSize*2/* short*/);
        audioContext->nTempBufferRdPtr += nSize;
    } else if (audioContext->nTempBufferRdPtr+nSize == audioContext->nTempBufferLength) {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        // reset rd ptr
        audioContext->nTempBufferRdPtr = 0;
    } else {
        memcpy(writeptr,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr)*2/* short*/);
        memcpy(&writeptr[audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr],audioContext->nTempBuffer,(nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr))*2/* short*/);
        audioContext->nTempBufferRdPtr = nSize-(audioContext->nTempBufferLength-audioContext->nTempBufferRdPtr);
        // reset rd wr ptr
        memcpy(audioContext->nTempBuffer,&audioContext->nTempBuffer[audioContext->nTempBufferRdPtr],(audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr)*2/* short*/);
        audioContext->nTempBufferWrPtr = audioContext->nTempBufferWrPtr - (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
        audioContext->nTempBufferRdPtr = 0;
    }
}

int getResampleAvailableLength(ResampleAudioContext *audioContext){
    if (audioContext->nTempBufferWrPtr>=audioContext->nTempBufferRdPtr){
        return (audioContext->nTempBufferWrPtr-audioContext->nTempBufferRdPtr);
    } else {
        printf("getResampleAvailableLength wr %d > rd %d ptr \n",audioContext->nTempBufferWrPtr,audioContext->nTempBufferRdPtr);
        return audioContext->nTempBufferLength-(audioContext->nTempBufferRdPtr-audioContext->nTempBufferWrPtr);
    }
}

/* pcm8 to pcm16 conversion */
static short pcm8_linear(char pcm_val)
{
    return ((short)(pcm_val-128) * (1 << 8)) + (short)pcm_val;
}


int resample(AVResampleContext *avResampleContext,ResampleAudioContext *audioContext, short *writeptr)
{
    int nTemp,i,j;
    int consumed,lenout,nReSampleOutputSize;
    float ratio = (float)audioContext->nOutSampleRate/(float)audioContext->nInSampleRate;

    
    for (i=0;i<audioContext->nInChannels;i++) {
        int is_last = i + 1 == audioContext->nInChannels;

        lenout= (audioContext->nInSize/audioContext->nInChannels) *ratio + 16;  
        nTemp = (audioContext->nInSize/audioContext->nInChannels)/2+audioContext->nKeep[i];
        nReSampleOutputSize = av_resample(avResampleContext, audioContext->reSamplePcmOutput[i], audioContext->reSamplePcmInput[i],
            &consumed, nTemp, lenout, is_last);
        audioContext->nKeep[i] = nTemp - consumed;
//        printf("resample nReSampleOutputSize %d consumed %d   %d %d %d %d %d \n",nReSampleOutputSize,consumed,audioContext->reSamplePcmOutput[i][1],audioContext->reSamplePcmOutput[i][2],audioContext->reSamplePcmOutput[i][3],audioContext->reSamplePcmOutput[i][4],audioContext->reSamplePcmOutput[i][5]);

    } 
    
    nTemp = audioContext->nKeep[0];
    for (i=0;i<nTemp;i++) {
        for (j=0;j<audioContext->nInChannels;j++)
            audioContext->reSamplePcmInput[j][i] = audioContext->reSamplePcmInput[j][consumed + i];
    }    
    
    if (audioContext->nUseTempBuffer == 0){
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
//                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
            }
        } else if (audioContext->nInChannels==2) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                *writeptr++ = audioContext->reSamplePcmOutput[0][i];
                *writeptr++ = audioContext->reSamplePcmOutput[1][i];
            }
        }
    } else {
        // copy to temp buffer
        if (audioContext->nInChannels==1) {
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
//                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,writeptr,2*MAX_FRAMESIZE);
            
        } else if (audioContext->nInChannels==2) {
            if (nReSampleOutputSize<MAX_FRAMESIZE){
                printf("resample use temp buffers nReSampleOutputSize %d < %d \n",nReSampleOutputSize,MAX_FRAMESIZE);
                nReSampleOutputSize = MAX_FRAMESIZE;
            }
            for (i = 0; i < nReSampleOutputSize; i++) 
            {  // interleave channels
                if (audioContext->nTempBufferWrPtr >= audioContext->nTempBufferLength){
                    audioContext->nTempBufferWrPtr = 0;
                }
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[0][i];
                audioContext->nTempBuffer[audioContext->nTempBufferWrPtr++] = audioContext->reSamplePcmOutput[1][i];
            }
            // copy to encode buffer
            getResampleEncodeBuffer(audioContext,writeptr,audioContext->nInChannels*MAX_FRAMESIZE);
        }
    }

    return nReSampleOutputSize*audioContext->nInChannels*sizeof(short);
}

// resampling talbe
//FILE *ftable = NULL;
//char *tables = "D:\\audio_testing\\resample\\tables.txt";

// return intput size
int getResampleSize(int nInSample,int nOutSample)
{
    int temp = 0;
    float ratio = (float)nOutSample/(float)nInSample;

    temp = (1024/ratio)+1;
    if (temp % 2)
        temp++;

    return temp;    
}

static void Test_Music(char *filename)
{
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;

    int nResult = 0;
    // close handler (if any)
    if (Local_player_http_handle) {
        fclose(Local_player_http_handle);            
        Local_player_http_handle = NULL;
    }

    if (filename)
        Local_player_http_handle = fopen(filename, "rb");

    if (!Local_player_http_handle) {
        printf("[Resample]%s() L#%ld: fopen error %s \r\n", __FUNCTION__, __LINE__,filename);
        return;
    }
    printf("[Resample]%s() L#%ld:  %s success \r\n", __FUNCTION__, __LINE__,filename);
    audiomgr_local.pHandle = Local_player_http_handle;
    audiomgr_local.LocalRead = fread;
    audiomgr_local.nReadLength = AUDIOLINK_LOCAL_PLAYER_BUFFER_LEN;
    audiomgr_local.nType = SMTK_AUDIO_WAV;
    audiomgr_local.bSeek = 0;
    audiomgr_local.nM4A = 0;
    audiomgr_local.bLocalPlay = 1;
    audiomgr_local.pFilename = filename;
    audiomgr_local.audioMgrCallback = resampleInterruptCallback;
    nResult = smtkAudioMgrPlayNetwork(&audiomgr_local);
    
}

int smtkAudioMgrInterruptSetCallbackFunction(int* pApi)
{
    if (pApi)
        resampleInterruptCallback = pApi;
}

void smtkAudioMgrInterruptSoundFinish()
{
    if (resampleInterruptCallback){
        resampleInterruptCallback(AUDIOMGR_STATE_CALLBACK_PLAYING_INTERRUPT_SOUND_FINISH);
    }
}


// nRemove :1 remove ,nRemove:0 finished 
int smtkAudioMgrInterruptSoundGetRemoveCard()
{

    return gInterruptRemoveCard;
}


// nRemove :1 remove ,nRemove:0 finished 
void smtkAudioMgrInterruptSoundSetRemoveCard(int nRemove)
{

#ifdef RESAMPLE_DEBUG
    printf("[Audio mgr] interrupt sound set remove card %d , #line %d \n",nRemove,__LINE__);
#endif
    gInterruptRemoveCard = nRemove;
}



// return 1: interrupt sound playing ,return 0: interrupt sound not playing
int smtkAudioMgrSetInterruptStatus(int nStatus)
{
    gInterruptStatus = nStatus;

#ifdef RESAMPLE_DEBUG
    printf("smtkAudioMgrSetInterruptStatus %d ",gInterruptStatus);
#endif
    
    return gInterruptStatus;


}


// return 1: interrupt sound playing ,return 0: interrupt sound not playing
int smtkAudioMgrGetInterruptStatus()
{
    
    return gInterruptStatus;
}

// 1: not overrite,only play wav
int smtkAudioMgrGetInterruptOverwriteStatus()
{
    return gOverwriteStatus;
}



void smtkAudioMgrSetInterruptOverwriteStatus(int nStatus)
{

    gOverwriteStatus = nStatus;

#ifdef RESAMPLE_DEBUG
    printf("smtkAudioMgrSetInterruptOverwriteStatus %d ",gOverwriteStatus);
#endif
    
    return gOverwriteStatus;
}


// input a wav file to play immediately
int smtkAudioMgrInterruptSound(char* filename,int overwrite,int* pApi)
{
    int nI2S = 0;
    int nInSampleRate,nOutSampleRate;
    int nInCh;
    int nInBits;
    int nTmp;
    int nSize = 0;
    int nAddress;

#ifdef RESAMPLE_DEBUG
    printf("smtkAudioMgrInterruptSound %d \n",smtkAudioMgrGetInterruptOverwriteStatus());
#endif
    if (gnInterruptStart==1){
        printf("[Audio mgr]smtkAudioMgrInterruptSound start, return #line %d \n",__LINE__);
        usleep(20000);
        return -1;
    }
    gnInterruptStart = 1;

    // check if resample task running
    if (gnResampleTask==1){
        deleteThread();
    }

    if (smtkAudioMgrGetInterruptOverwriteStatus()==1){
        usleep(20000);
        smtkAudioMgrQuickStop();

    }

    //if (pApi)
    resampleInterruptCallback = pApi;
    // check if i2s dac using
    nI2S = i2s_get_DA_running();
#ifdef TEST_RESAMPLE
    nI2S = 1;
#endif
     
    // create task to put data in i2s or play local

    //if use
    if (nI2S) {

        // check wav info
        if ((fin=fopen(filename, "rb")) == NULL) {
            printf("Can not open file '%s'\n", filename);
            gnInterruptStart = 0;
            return -1;
        }
         nTmp = ReadWAVHeader(fin,&nInSampleRate,&nInCh,&nInBits);
         if (nTmp) {
            printf("[Audio Mgr]smtkAudioMgrInterruptSound unknow error #line %d \n",__LINE__);
            gnInterruptStart = 0;
            return -1;
        }
#ifdef TEST_BUFFER
        readFileInBuffer(fin);
        
#endif         
         if (nInSampleRate<=0 || nInSampleRate>16000){
            printf("[Audio Mgr]smtkAudioMgrInterruptSound unknow error #line %d \n",__LINE__);
            gnInterruptStart = 0;
            return -1;
        }
        if (nInCh<=0 || nInCh>3){
            printf("[Audio Mgr]smtkAudioMgrInterruptSound unknow error #line %d \n",__LINE__);
            gnInterruptStart = 0;
            return -1;
        }

        // set input,output data info
        iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_CHANNEL, &gnOutch);
        iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_SAMPLE_RATE, &nOutSampleRate);
        iteAudioGetAttrib(ITE_AUDIO_CODEC_SET_BUFFER_LENGTH, &gnI2SBufLen);
        iteAudioGetAttrib(ITE_AUDIO_I2S_PTR, &nAddress);
        gOutBuffer = nAddress;
        if (!gOutBuffer){
            printf("[Audio Mgr]smtkAudioMgrInterruptSound unknow error #line %d \n",__LINE__);
            gnInterruptStart = 0;
            return -1;
        }
#ifdef TEST_RESAMPLE
        gnOutch = 2;
        nOutSampleRate = 48000;
#endif

#ifdef RESAMPLE_DEBUG
        printf("[Audio mgr] smtkAudioMgrInterruptSound in ch %d sample rate %d bits %d , out ch %d sample rate %d ,%d  #line %d \n",nInCh,nInSampleRate,nInBits,gnOutch,nOutSampleRate,nI2S,__LINE__);
        printf(" filename %s ,0x%x\n",filename,resampleInterruptCallback);
#endif        

        // get nSize
        nSize = getResampleSize(nInSampleRate,nOutSampleRate);
        if (nInCh==2) {
            nSize = 4*nSize;
        } else if (nInCh==1) {
            nSize = 4*nSize;
        }

        gResampleAudioContext.nUseTempBuffer = 0;
        gResampleAudioContext.nInSampleRate = nInSampleRate;
        gResampleAudioContext.nOutSampleRate = nOutSampleRate;
        gResampleAudioContext.nInSize = nSize;
        gResampleAudioContext.nInChannels = nInCh;
        gResampleAudioContext.nTempBufferLength = TEMP_BUFFER_SIZE;
        gResampleAudioContext.nTempBufferRdPtr = 0;
        gResampleAudioContext.nTempBufferWrPtr = 0;
        gResampleAudioContext.nInBitsPerSample = nInBits;
        smtkAudioMgrSetInterruptStatus(1);        
        // create resample thread
        createThread();
       
    } else {
        //if not use
        // call smtkAudiomgrNetworkplay
        
        smtkAudioMgrQuickStop();
        Test_Music(filename);       
        smtkAudioMgrSetInterruptStatus(1);        
        smtkAudioMgrSetInterruptOverwriteStatus(1);


    }
        
    gnInterruptStart = 0;
    return 0;

}

# if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(flushdecode_task, params)
# else
int main_resample(int argc, char **argv)
# endif
{
    // input file 
    FILE *fin = NULL;
    char *in = "D:\\audio_testing\\resample\\P3-8k_16.wav";
    int nInSampleRate = 44100;
    int nInCh;
    int nInBits;
    // output file
    FILE *fout = NULL;
    //char *out = "D:\\audio_testing\\resample\\2.river_48000.wav";
    //char *out = "D:\\audio_testing\\resample\\1.bird_44100_48000.wav";
    char *out = "D:\\audio_testing\\resample\\P3-32000(8k_16).wav";

    int nOutSampleRate = 48000;
    int nOutCh;

    // resample 
    ResampleAudioContext resampleAudioContext;

    // buffer
    char wav[44];
    char  readBuffer8[8*1024];
    short readBuffer[8*1024];
    short writeBuffer[8*1024];
    int done = 0;
    int nTemp,i,j;
    int nSize;
    int consumed,lenout,nReSampleOutputSize;
    float ratio = (float)nOutSampleRate/(float)nInSampleRate;
    
    // get input data 
    nInSampleRate = 8000;
    nInCh = 1;
    nInBits=16;

    // get output data
    nOutSampleRate = 32000;
    nOutCh = 1;

    memset(&resampleAudioContext,0,sizeof(ResampleAudioContext));
    memset(writeBuffer,0,sizeof(writeBuffer));

    // get nSize
    nSize = getResampleSize(nInSampleRate,nOutSampleRate);
    if (nInCh==2) {
        nSize = 4*nSize;
    } else if (nInCh==1) {
        nSize = 4*nSize;
    }
/*
    if (nInSampleRate == 32000){
        nSize = 4*686;//4*1024;
        resampleAudioContext.nUseTempBuffer = 1;
    }
    else if (nInSampleRate == 44100){
        nSize = 4*942;//4*1024;
        resampleAudioContext.nUseTempBuffer = 1;
    }
*/
    resampleAudioContext.nUseTempBuffer = 0;
  
    resampleAudioContext.nInSampleRate = nInSampleRate;
    resampleAudioContext.nOutSampleRate = nOutSampleRate;
    resampleAudioContext.nInSize = nSize;
    resampleAudioContext.nInChannels = nInCh;
    resampleAudioContext.nTempBufferLength = TEMP_BUFFER_SIZE;
    resampleAudioContext.nTempBufferRdPtr = 0;
    resampleAudioContext.nTempBufferWrPtr = 0;
    resampleAudioContext.nInBitsPerSample = nInBits;

    if ((fin=fopen(in, "rb")) == NULL) {
        printf("Can not open file '%s'\n", in);
        //exit(-1);
    }
  //  fread(wav, 1, 44, fin);

    if ((fout=fopen(out, "wb")) == NULL) {
        printf("Can not open file '%s'\n", out);
        //exit(-1);
    }

#if 0
    if ((ftable=fopen(tables, "wb")) == NULL) {
        printf("Can not open file '%s'\n", tables);
        exit(-1);
    }
#endif


    WriteWAVHeader(fout, nOutSampleRate, 16, nOutCh);

    av_resample_init(&avResampleContext,nOutSampleRate, nInSampleRate, RESAMPLE_FILTER_LENGTH, 10,0, 0.8);
    
    do {
        // read input audio
        if (resampleAudioContext.nInBitsPerSample == 16) {
            if (fread(readBuffer,1,nSize,fin)!=nSize){
                printf("eof \n");
                done = 1;
                break;
            }
        } else if (resampleAudioContext.nInBitsPerSample == 8) {
            nTemp = nSize/2;
            if (fread(readBuffer8,1,nTemp,fin)!=nTemp){
                printf("eof \n");
                done = 1;
                break;
            }

//
            for(i = 0; i < nTemp; i++)
            {
                short *s = (short*)&readBuffer[i];
                {
                   *s = pcm8_linear(readBuffer8[i]);
                   
                }
            }

//
        }
        // check resample temp buffers
        if (resampleAudioContext.nUseTempBuffer == 1){
            nTemp = getResampleAvailableLength(&resampleAudioContext);
            //printf("getResampleAvailableLength %d  %d  \n",nTemp,2*MAX_FRAMESIZE);
            if (nTemp>=resampleAudioContext.nInChannels*MAX_FRAMESIZE){
                printf("getResampleAvailableLength %d > %d  \n",nTemp,resampleAudioContext.nInChannels*MAX_FRAMESIZE);
                getResampleEncodeBuffer(&resampleAudioContext,writeBuffer,resampleAudioContext.nInChannels*MAX_FRAMESIZE);
                // output
                fwrite(writeBuffer,1,resampleAudioContext.nInChannels*2*MAX_FRAMESIZE,fout);
            }

        }

        if (resampleAudioContext.nInChannels == 2) {
            for(i = 0,j=0; i < nSize/2; i+=2,j++){
               resampleAudioContext.reSamplePcmInput[0][resampleAudioContext.nKeep[0]+j] = readBuffer[i];
               resampleAudioContext.reSamplePcmInput[1][resampleAudioContext.nKeep[1]+j] = readBuffer[i+1];
            }
        } if (resampleAudioContext.nInChannels == 1) {
            for(i = 0; i < nSize; i++){
               resampleAudioContext.reSamplePcmInput[0][resampleAudioContext.nKeep[0]+i] = readBuffer[i];
               //resampleAudioContext.reSamplePcmInput[1][resampleAudioContext.nKeep[1]+j] = readBuffer[i+1];
            }
        }

        // resample
        nTemp = resample(&avResampleContext,&resampleAudioContext,writeBuffer);
        //if (nTemp != 4*MAX_FRAMESIZE)
        //    printf("resample output %d \n",nTemp);
        
        //nTemp = 4*MAX_FRAMESIZE;

        // output
        fwrite(writeBuffer,1,nTemp,fout);
    } while (!done);
    fclose(fin);
    fclose(fout);
}

#else

// return 1: interrupt sound playing ,return 0: interrupt sound not playing
int smtkAudioMgrSetInterruptStatus(int nStatus)
{

    return 0;    
}
// return 1: interrupt sound playing ,return 0: interrupt sound not playing
int smtkAudioMgrGetInterruptStatus()
{

    return 0;    
}


// nRemove :1 remove ,nRemove:0 finished 
int smtkAudioMgrInterruptSoundGetRemoveCard()
{

    return 0;    
}

// nRemove :1 remove ,nRemove:0 finished 
void smtkAudioMgrInterruptSoundSetRemoveCard(int nRemove)
{

    return 0;    
}


void smtkAudioMgrSetInterruptOverwriteStatus(int nStatus)
{

    return 0;    
}    

// 1: not overrite,only play wav
int smtkAudioMgrGetInterruptOverwriteStatus()
{

    return 0;    
}

// input a wav file to play immediately
int smtkAudioMgrInterruptSound(char* filename,int overwrite,int* pCallback)
{

    return 0;    
}


#endif

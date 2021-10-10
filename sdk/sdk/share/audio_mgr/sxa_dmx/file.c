
#include "sxa_dmx.h"


#ifndef UNICODE
    #define UNICODE
#endif

#ifdef SMTK_FILE_PAL
//#include "common/fat.h"
//#include "mmc/single/mmc_smedia.h"
//#include "ftl/src/ftl.h"
//#include "nor/nordrv_f.h"
#endif


#ifndef NULL
    #define NULL 0
#endif

/* Deprecated */
typedef void
(*PAL_FILE_CALLBACK)(
    PAL_FILE* file,
    MMP_ULONG result);

typedef void
(*PAL_FILE_WRITE_CALLBACK)(
    PAL_FILE* file,
    MMP_ULONG result);

typedef unsigned int MMP_SIZE_T;


#ifdef CFG_AUDIO_MGR_M4A



#define STREAM_UPDATE_SIZE 64*1024
#define STREAM_SKIP_SIZE   16*1024
#define STREAM_READ_BUFFER_SIZE 20*64*1024


static int gDataInput =0; // 0:network,1:file
static int gDataSize =0;  // data total size

// compressed data fill to stream buffer
// demuxer get data from stream buffer
static int gSBReadPointer = 0;
static int gSBWritePointer = 0;
static int gSBUsedSize = 0;
static int gStartSB =0;
static int gSBDebug = 0;
static int gSBTempSize = 0;
static MMP_LONG gDataPosition = 0;
static char gTempBuffer[STREAM_UPDATE_SIZE];
static char gSkipBuffer[STREAM_UPDATE_SIZE];
static char gStreamBuf[STREAM_READ_BUFFER_SIZE];
static void* gInFile;
// when parsing m4a, can not over write data
static int gParsing = 0;

// 0: can write data, 1: can not write data
static int gnGetSB = 0;


// get all data
static int gnGetAllData = 0;


int getStreamBufferAvailableSize();
int getStreamBufferFilledSize();
int fillStreamBuffer(int nSize,char* pSource);
int updateStreamBuffer();
int palClearStreamBuffer();

int getStreamBufferAvailableSize()
{
    gSBDebug = __LINE__;
    if (gSBWritePointer-gSBReadPointer>=0){
        return STREAM_READ_BUFFER_SIZE-(gSBWritePointer-gSBReadPointer);
    } else {
        return gSBReadPointer-gSBWritePointer;
    }

}

int getStreamBufferFilledSize()
{
    gSBDebug = __LINE__;
    if (gSBWritePointer-gSBReadPointer>=0){
        return (gSBWritePointer-gSBReadPointer);
    } else {
        return STREAM_READ_BUFFER_SIZE - (gSBReadPointer-gSBWritePointer);
    }

}

int readStreamBuffer(int nSize,char* pSource){
    int nTemp;
    gSBDebug = __LINE__;    
    if (nSize > getStreamBufferFilledSize()){
        printf(" read size %d > fill size %d \n",nSize,getStreamBufferFilledSize());
        return 0;
    } else {        
        if (gSBReadPointer+nSize<=STREAM_READ_BUFFER_SIZE){
            memcpy(pSource,&gStreamBuf[gSBReadPointer],nSize);
            gSBReadPointer+=nSize;
        } else {
            memcpy(pSource,&gStreamBuf[gSBReadPointer],STREAM_READ_BUFFER_SIZE-gSBReadPointer);
            nTemp = gSBReadPointer;
            gSBReadPointer = 0;
            memcpy(pSource+(STREAM_READ_BUFFER_SIZE-nTemp),&gStreamBuf[gSBReadPointer],nSize-(STREAM_READ_BUFFER_SIZE-nTemp));
            gSBReadPointer = nSize -(STREAM_READ_BUFFER_SIZE-nTemp);
        }
        if (gSBReadPointer>STREAM_READ_BUFFER_SIZE){
            gSBReadPointer = 0;
        }
        return nSize;
    }

}

int fillStreamBuffer(int nSize,char* pSource){
    int nTemp;
    gSBDebug = __LINE__;    
    if (nSize > getStreamBufferAvailableSize()){
        printf(" fill size %d > available size %d wptr %d rdptr %d \n",nSize,getStreamBufferAvailableSize(),gSBWritePointer,gSBReadPointer);
        return 1;
    } else {
        if (gSBWritePointer+nSize<=STREAM_READ_BUFFER_SIZE){
            memcpy(&gStreamBuf[gSBWritePointer],pSource,nSize);
            gSBWritePointer+=nSize;
        } else {
            memcpy(&gStreamBuf[gSBWritePointer],pSource,STREAM_READ_BUFFER_SIZE-gSBWritePointer);
            nTemp = gSBWritePointer;
            gSBWritePointer = 0;
            memcpy(&gStreamBuf[gSBWritePointer],pSource+(STREAM_READ_BUFFER_SIZE-nTemp),nSize-(STREAM_READ_BUFFER_SIZE-nTemp));
            gSBWritePointer = nSize -(STREAM_READ_BUFFER_SIZE-nTemp);
        }
        if (gSBWritePointer>STREAM_READ_BUFFER_SIZE){
            gSBWritePointer = 0;
        }
    }

}

int updateStreamBuffer(){

    int result = 0;
    int i=0;
    gSBDebug = __LINE__;
    // when parsing m4a, can not over write data
    if (gParsing == 1 && (gSBWritePointer+STREAM_UPDATE_SIZE>=STREAM_READ_BUFFER_SIZE)){
        return result;
    }

    if ((gSBWritePointer>gSBReadPointer) && (gSBWritePointer-gSBReadPointer>=(4*STREAM_UPDATE_SIZE))){
        return result;
    }
    
    // get all data, no need update stream buffer 
    if (gnGetAllData==1 && gnGetSB==0) {
        return result;
    }
    if (getStreamBufferAvailableSize() >  STREAM_UPDATE_SIZE) {
        if (gDataInput==1) {
            //result = fread(gTempBuffer, 1, STREAM_UPDATE_SIZE, gInFile);
            //fillStreamBuffer(STREAM_UPDATE_SIZE,gTempBuffer);
        } else {
            // read data from network
            do {
                gSBDebug = __LINE__;                
                usleep(20000);
                i++;
            } while (gnGetSB==0 && i<=10000 && gStartSB==1 && gnGetAllData==0);

            if (gStartSB==0)
                return result;

            if (gnGetAllData==1 && gnGetSB==0)
                return result;            
            
            if (i>=10000)
                printf("updateStreamBuffer can not get data \n");
            
            if (gSBTempSize>0 && gSBTempSize<=STREAM_UPDATE_SIZE) {
//                printf("fillStreamBuffer r %d w %d \n",gSBReadPointer,gSBWritePointer);
                result = fillStreamBuffer(gSBTempSize,gTempBuffer);
            } else {
                printf("gSBTempSize size %d error \n",gSBTempSize);
            }
        //  result = fillStreamBuffer(STREAM_UPDATE_SIZE,gTempBuffer);
            
            palClearStreamBuffer();
        }
    } else {
        
    }
    return result;
}


// 1 : empty , need data
int palStreamBufferEmpty()
{
    gSBDebug = __LINE__;
    if (gParsing == 1 && (gSBWritePointer+STREAM_UPDATE_SIZE>=STREAM_READ_BUFFER_SIZE)){
        return 0;
    }

    if (gnGetSB==1)
        return 0;

    if (getStreamBufferAvailableSize() >=  STREAM_UPDATE_SIZE){
        return 1;
    } else {
        return 0;
    }
}

int palFillStreamBuffer(char* pBuf,int nSize){
    gSBDebug = __LINE__;
    memcpy(gTempBuffer,pBuf,nSize);
    gSBTempSize = nSize;
    gnGetSB = 1;
}

int palClearStreamBuffer(){
    gSBDebug = __LINE__;
    memset(gTempBuffer,0,STREAM_UPDATE_SIZE);
    gnGetSB = 0;
}


void setDataInput(int nInType)
{
    gDataInput = nInType;
}

void setDataSize(int nInSize)
{
    gDataSize = nInSize;
}

int getDataParsing()
{
    return gParsing;
}

void setDataParsing(int nParsing)
{
    printf("[PalFile] setDataParsing %d \n",nParsing);
    gParsing = nParsing;
}

void startM4aStreamBuffer()
{
    gStartSB =1;
}

void getAllM4aData()
{
    gnGetAllData =1;
}

void resetM4aStreamBuffer()
{
    printf("resetM4aStreamBuffer %d \n",gSBDebug);
    gSBReadPointer = 0;
    gSBWritePointer = 0;
    gDataPosition = 0;
    gParsing =0;
    gnGetSB =0;
    gSBTempSize = 0;
    gStartSB = 0;
    gnGetAllData = 0;    
    memset(gStreamBuf,0,STREAM_READ_BUFFER_SIZE);
    
}

void setDataPosition(int nPosition)
{
    gDataPosition = nPosition;
}

MMP_INT
PalTFileClose(
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_INT result;
    printf("PalTFileClose \n");
    //result = fclose(stream);
    return result;
}



MMP_LONG
PalTFileTell(
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_LONG result=0;
    //result = ftell(stream);
    //result = gSBReadPointer;
    result = gDataPosition;
    return result;
}



MMP_SIZE_T
PalTFileRead(
    void* buffer,
    MMP_SIZE_T size,
    MMP_SIZE_T count,
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_SIZE_T result;
    // get stream buffer data
    updateStreamBuffer();
    if (buffer==NULL) {
        printf("[PalFile] PalTFileRead error \n");
    }
    result = readStreamBuffer(size*count,(char*)buffer);
    gDataPosition+= (size*count);
    //result = fread(buffer, size, count, stream);
    return count;
}



MMP_INT
PalTFileSeek(
    PAL_FILE* stream,
    MMP_LONG offset,
    MMP_INT origin,
    PAL_FILE_CALLBACK callback)
{
    MMP_INT result=0;
    int nTemp,nRead;
    if (offset<0 || gDataPosition<0) {
        printf("[Palfile] PalTFileSeek offset %d <0 , data position %d \n",offset,gDataPosition);
        return 0;
    }
    updateStreamBuffer();
    if (origin == PAL_SEEK_CUR){
        if (offset>STREAM_SKIP_SIZE){
            printf("[Palfile]skip %d , data position %d \n",offset,gDataPosition);
            if (gDataPosition<0)
                return result;
            nTemp = offset;
            do {
                if (nTemp-STREAM_SKIP_SIZE>=0) {
                    nRead=STREAM_SKIP_SIZE;
                } else {
                    nRead=nTemp;
                }
                result = readStreamBuffer(nRead,(char*)gSkipBuffer);
                nTemp -= nRead;
                updateStreamBuffer();
            } while (nTemp>0 && gStartSB==1 && gnGetAllData==0);
        } else {
            result = readStreamBuffer(offset,(char*)gSkipBuffer);     
        }
        gDataPosition+=offset;
    } else if (origin == PAL_SEEK_SET) {
        gSBReadPointer = offset;
        gDataPosition = offset;
    }
    //result = fseek(stream, offset, origin);
    return result;
}



PAL_FILE*
PalTFileOpen(
    const MMP_WCHAR* filename,
    MMP_UINT mode,
    PAL_FILE_CALLBACK callback)
{
    MMP_INT result = 0;
    void* file;
    if (gDataInput==1){
#if 1
        switch (mode)
        {
        case PAL_FILE_RB:
            file = fopen(filename, "rb");
            break;

        case PAL_FILE_WB:
            file = fopen(filename, "wb");
            break;

        case PAL_FILE_AB:
            file = fopen(filename, "ab");
            break;

        case PAL_FILE_RBP:
            file = fopen(filename, "rb+");
            break;

        case PAL_FILE_WBP:
            file = fopen(filename, "wb+");
            break;

        case PAL_FILE_ABP:
            file = fopen(filename, "ab+");
            break;

        default:
            file = MMP_NULL;
        } 
#else
        switch (mode)
        {
        case PAL_FILE_RB:
            result = _wfopen_s(&file, filename, L"rb");
            break;

        case PAL_FILE_WB:
            result = _wfopen_s(&file, filename, L"w");
            break;

        case PAL_FILE_AB:
            result = _wfopen_s(&file, filename, L"a");
            break;

        case PAL_FILE_RBP:
            result = _wfopen_s(&file, filename, L"r+");
            break;

        case PAL_FILE_WBP:
            result = _wfopen_s(&file, filename, L"w+");
            break;

        case PAL_FILE_ABP:
            result = _wfopen_s(&file, filename, L"a+");
            break;

        default:
            file = MMP_NULL;
        }
#endif        
    } else {
        printf("[PalFile] open stream \n");
        palClearStreamBuffer();
        // wait data
        usleep(200000);
        updateStreamBuffer();        
    }

    if (result == 0) {
        gInFile = file;
        return file;
    } else {
        return NULL;
    }

}



MMP_INT32
PalWGetFileLength(
    const MMP_WCHAR* filename)
{
    MMP_INT32 result;
    void*     file;

    long      size = 0;
#if 0
    result = _wfopen_s(&file, filename, L"r");
    if (result != 0)
    {
        return 0;
    }

    result = fseek(file, 0, SEEK_END);
    if (result != 0)
    {
        return 0;
    }

    size = ftell(file);
#endif
    return size;
}

MMP_SIZE_T
PalTFileWrite(const void* ptr,MMP_SIZE_T size,MMP_SIZE_T count,PAL_FILE* stream,PAL_FILE_WRITE_CALLBACK callback)
{
    int result=0;   
    result = fwrite(ptr, size, count, stream);
    return result;
}

#else

int getStreamBufferAvailableSize()
{

}

int getStreamBufferFilledSize()
{

}

int readStreamBuffer(int nSize,char* pSource){

}

int fillStreamBuffer(int nSize,char* pSource){

}

int updateStreamBuffer(){


}


// 1 : empty , need data
int palStreamBufferEmpty()
{
}

int palFillStreamBuffer(char* pBuf,int nSize){
}

int palClearStreamBuffer(){
}


void setDataInput(int nInType)
{
}

void setDataSize(int nInSize)
{
}

int getDataParsing()
{
}

void setDataParsing(int nParsing)
{
}

void startM4aStreamBuffer()
{
}

void getAllM4aData()
{
}

void resetM4aStreamBuffer()
{
    
}

void setDataPosition(int nPosition)
{
}

MMP_INT
PalTFileClose(
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_INT result;
    return result;
}



MMP_LONG
PalTFileTell(
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_LONG result=0;
    return result;
}



MMP_SIZE_T
PalTFileRead(
    void* buffer,
    MMP_SIZE_T size,
    MMP_SIZE_T count,
    PAL_FILE* stream,
    PAL_FILE_CALLBACK callback)
{
    MMP_SIZE_T result;
    return count;
}



MMP_INT
PalTFileSeek(
    PAL_FILE* stream,
    MMP_LONG offset,
    MMP_INT origin,
    PAL_FILE_CALLBACK callback)
{
    MMP_INT result=0;
 
    return result;
}



PAL_FILE*
PalTFileOpen(
    const MMP_WCHAR* filename,
    MMP_UINT mode,
    PAL_FILE_CALLBACK callback)
{


}



MMP_INT32
PalWGetFileLength(
    const MMP_WCHAR* filename)
{
    long      size = 0;
    return size;
}

MMP_SIZE_T
PalTFileWrite(const void* ptr,MMP_SIZE_T size,MMP_SIZE_T count,PAL_FILE* stream,PAL_FILE_WRITE_CALLBACK callback)
{
    int result=0;   
    return result;
}



#endif


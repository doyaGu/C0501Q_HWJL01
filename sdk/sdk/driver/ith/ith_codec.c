/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Codec functions.
 *
 * @author Vincent Lee
 * @version 1.0
 */
#include "ith_cfg.h"

#include <stdio.h>
#include <string.h>
#include "ite/ith.h"

#ifdef WIN32
#define asm()
#endif



#define TOINT(n)    ((((n)>>24)&0xff)+(((n)>>8)&0xff00)+(((n)<<8)&0xff0000)+(((n)<<24)&0xff000000))
#define CODEC_START_ADDR    0x1000
#define CODEC_MAGIC         0x534D3020
#define CODEC_SIZE          1000*1024

typedef struct CODEC_CONTEXT_TAG
{
    int commandBuffer;
    int commandBufferLength;
    int wiegandBuffer;
    int wiegandBufferLength;
    int printfBuffer;
} CODEC_CONTEXT;

struct _codec_api {
    void    *reserved;
};

struct _codec_header {
    unsigned long  magic;
    unsigned short target_id;
    unsigned short api_version;
    unsigned char *load_addr;
    unsigned char *end_addr;
    int (*entry_point)(struct _codec_api*);
    unsigned char* pStream;
    int (*codec_info)();
};

//static unsigned char gCodec[CODEC_SIZE+sizeof(CODEC_CONTEXT)] __attribute__ ((aligned(32))) = {
//#ifdef WIN32
//    0
//#else
//    #include "speex.hex"
//#endif
//};

//static CODEC_CONTEXT *gCodecCtxt = (CODEC_CONTEXT *)&gCodec[CODEC_SIZE+0x14];
/*
static bool inited = false;
#define CODEC_BASE  (int)&gBuf;
static CODEC_CONTEXT *gCodecCtxt = (CODEC_CONTEXT *)&gBuf[CODEC_SIZE+0x14];


void
ithCodecOpenEngine(
    void)
{
    return;

    int i;
    int check_count = 0;
    unsigned short check_reg;
    struct _codec_header *__header;

    if (inited)
        return;

    // init risc clock
    //ithWriteRegH(0x0044, 0x07FF);
    
    // reset risc
    ithWriteRegMaskH(0x16ca, ((1 << 1) | (0 << 0)), ((1 << 1) | (1 << 0)));
    ithWriteRegMaskH(0x0044, 1 << 14, 1 << 14);
    for (i = 0; i < 2048; i++) asm("");
    ithWriteRegMaskH(0x0044, 0 << 14, 1 << 14);

    // get the header, check if header is corrected
    __header = (struct _codec_header*)(CODEC_BASE+CODEC_START_ADDR);
    //printf("__header->magic 0x%08x, expect value = 0x%08x\n", TOINT(__header->magic), CODEC_MAGIC);

    // give the context and message buffer to risc
    __header->pStream = (char*)TOINT((int)gCodecCtxt-CODEC_BASE);
    
    // fire risc
    ithWriteRegH(0x16de, CODEC_BASE);
    ithWriteRegH(0x16e0, (CODEC_BASE>>16));
    ithWriteRegMaskH(0x168c, ((0 << 1) | (1 << 0)), ((1 << 1) | (1 << 0)));
    for(i=0; i < 10; i++) asm("");
    ithWriteRegMaskH(0x168c, (0 << 0), (1 << 0));

    // used to check if risc is running
    ithWriteRegH(0x16ec, 0);

    // check if risc is running
    do
    {
        check_reg = ithReadRegH(0x16ec);
        usleep(1000);
        check_count++;
    } while (check_reg==0 && check_count<=20000);
    if (check_count>20000)
    {
        printf("wait risc start fail\n");
    }
    else
    {
        inited = true;
        printf("risc start\n");
    }
    
}

bool
ithCodecCommand(
    int command,
    int parameter0,
    int parameter1)
{

    int timeout = 10;
    int commandBuffer = TOINT(gCodecCtxt->commandBuffer)+CODEC_BASE;
    int commandBufferLength = TOINT(gCodecCtxt->commandBufferLength);

    if (inited==false)
        return false;

    printf("--- commandBufferLength = %d ----\n", commandBufferLength);    

    ithInvalidateDCache();
    while (*((int*)commandBuffer) && timeout)
    {
        usleep(1000);
        timeout--;
    }

    if (timeout==0)
        return false;

    *((int*)commandBuffer) = command;
    commandBuffer+=4;
    *((int*)commandBuffer) = parameter0;
    commandBuffer+=4;
    *((int*)commandBuffer) = parameter1;
    ithFlushDCacheRange((void*)commandBuffer, commandBufferLength);    
    return true;
}

unsigned long long
ithCodecWiegandReadCard(
    int index)
{
    unsigned long* value;
    unsigned long long card_id;    
    int wiegandBuffer = TOINT(gCodecCtxt->wiegandBuffer)+CODEC_BASE;
    int wiegandBufferLength = TOINT(gCodecCtxt->wiegandBufferLength);

    if (inited==false)
        return 0;

    // get back card id from buffer of wiegand
    value = (unsigned long*)wiegandBuffer;
    ithInvalidateDCache();
    card_id = (((unsigned long long)TOINT(value[2*index]) << 32) | (unsigned long long)TOINT(value[2*index+1]));

    // reset buffer
    memset((void*)wiegandBuffer, 0, wiegandBufferLength);
    ithFlushDCacheRange((void*)wiegandBuffer, wiegandBufferLength);
    return card_id;

    return false;
}

void
ithCodecPrintfWrite(
    char* string,
    int length)
{

    int printfBuffer = TOINT(gCodecCtxt->printfBuffer)+CODEC_BASE;

    if (inited==false)
        return;

    ithInvalidateDCache();
    if (*((char*)printfBuffer)
     && *((char*)(printfBuffer+=80))
     && *((char*)(printfBuffer+=160))
     && *((char*)(printfBuffer+=240)))
        return;

    if (length>79) length = 79;
    *((char*)printfBuffer) = length;
    printfBuffer++;
    memcpy((void*)printfBuffer, string, length);
    ithFlushDCacheRange((void*)printfBuffer, length);

    return;
}*/

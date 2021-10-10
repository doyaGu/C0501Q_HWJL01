/***************************************************************************
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 *
 * @file
 * Top code of TS demux.
 *
 * @author I-Chun Lai
 * @version 1.0
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "mmio.h"
#ifdef ENABLE_CODECS_PLUGIN
#include "plugin.h"
#endif
#include "type.h"
#include "tsi.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
/* Code for general_printf() */
#define BITS_PER_BYTE    8
#define MINUS_SIGN       1
#define RIGHT_JUSTIFY    2
#define ZERO_PAD         4
#define CAPITAL_HEX      8
#endif

typedef enum RISC_STATE_TAG
{
    RISC_STOP               = 0,
    RISC_RUN
} RISC_STATE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
struct parameters
{
    int     number_of_output_chars;
    short   minimum_field_width;
    char    options;
    short   edited_string_length;
    short   leading_zeros;
};
#endif

typedef struct RISC_TAG
{
    uint8*      pRiscBaseAddr;
    uint32      bValidChannel[MAX_CHANNEL_COUNT];
} RISC;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
static uint     *gtAudioPluginBufferLength;
static uint16   *gtAudioPluginBuffer;
uint8           *gBuf;
int             gPrint;
#endif

extern CHANNEL_INFO_ARRAY   gtShareInfo;
static CHANNEL_INFO_ARRAY*  gptShareInfo = &gtShareInfo;

static RISC                 gtRisc;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

void
AudioPluginAPI(
    int nType);

//=============================================================================
//                              Public Function Definition
//=============================================================================

#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
static void
output_and_count(
    struct parameters   *p,
    int                 c)
{
    if (p->number_of_output_chars >= 0)
    {
        int n = c;
        gBuf[gPrint++] = c;
        if (n >= 0)
            p->number_of_output_chars++;
        else
            p->number_of_output_chars = n;
    }
}

static void
output_field(
    struct parameters   *p,
    char                *s)
{
    short justification_length =
        p->minimum_field_width - p->leading_zeros - p->edited_string_length;
    if (p->options & MINUS_SIGN)
    {
        if (p->options & ZERO_PAD)
            output_and_count(p, '-');
        justification_length--;
    }
    if (p->options & RIGHT_JUSTIFY)
        while (--justification_length >= 0)
            output_and_count(p, p->options & ZERO_PAD ? '0' : ' ');
    if (p->options & MINUS_SIGN && !(p->options & ZERO_PAD))
        output_and_count(p, '-');
    while (--p->leading_zeros >= 0)
        output_and_count(p, '0');
    while (--p->edited_string_length >= 0){
        output_and_count(p, *s++);
    }
    while (--justification_length >= 0)
        output_and_count(p, ' ');
}

int
ithGPrintf(
    const char  *control_string,
    va_list     va)
{
    struct parameters p;
    char              control_char;
    p.number_of_output_chars = 0;
    control_char             = *control_string++;

    while (control_char != '\0')
    {
        if (control_char == '%')
        {
            short precision     = -1;
            short long_argument = 0;
            short base          = 0;
            control_char          = *control_string++;
            p.minimum_field_width = 0;
            p.leading_zeros       = 0;
            p.options             = RIGHT_JUSTIFY;
            if (control_char == '-')
            {
                p.options    = 0;
                control_char = *control_string++;
            }
            if (control_char == '0')
            {
                p.options   |= ZERO_PAD;
                control_char = *control_string++;
            }
            if (control_char == '*')
            {
                //p.minimum_field_width = *argument_pointer++;
                control_char          = *control_string++;
            }
            else
            {
                while ('0' <= control_char && control_char <= '9')
                {
                    p.minimum_field_width =
                        p.minimum_field_width * 10 + control_char - '0';
                    control_char = *control_string++;
                }
            }
            if (control_char == '.')
            {
                control_char = *control_string++;
                if (control_char == '*')
                {
                    //precision    = *argument_pointer++;
                    control_char = *control_string++;
                }
                else
                {
                    precision = 0;
                    while ('0' <= control_char && control_char <= '9')
                    {
                        precision    = precision * 10 + control_char - '0';
                        control_char = *control_string++;
                    }
                }
            }
            if (control_char == 'l')
            {
                long_argument = 1;
                control_char  = *control_string++;
            }
            if (control_char == 'd')
                base = 10;
            else if (control_char == 'x')
                base = 16;
            else if (control_char == 'X')
            {
                base       = 16;
                p.options |= CAPITAL_HEX;
            }
            else if (control_char == 'u')
                base = 10;
            else if (control_char == 'o')
                base = 8;
            else if (control_char == 'b')
                base = 2;
            else if (control_char == 'c')
            {
                base       = -1;
                p.options &= ~ZERO_PAD;
            }
            else if (control_char == 's')
            {
                base       = -2;
                p.options &= ~ZERO_PAD;
            }
            if (base == 0) /* invalid conversion type */
            {
                if (control_char != '\0')
                {
                    output_and_count(&p, control_char);
                    control_char = *control_string++;
                }
            }
            else
            {
                if (base == -1) /* conversion type c */
                {
                    //char c = *argument_pointer++;
                    char c = (char)(va_arg(va, int));
                    p.edited_string_length = 1;
                    output_field(&p, &c);
                }
                else if (base == -2) /* conversion type s */
                {
                    char *string;
                    p.edited_string_length = 0;
                    //string                 = *(char **) argument_pointer;
                    //argument_pointer      += sizeof(char *) / sizeof(int);
                    string = va_arg(va, char*);
                    while (string[p.edited_string_length] != 0)
                        p.edited_string_length++;
                    if (precision >= 0 && p.edited_string_length > precision)
                        p.edited_string_length = precision;
                    output_field(&p, string);
                }
                else /* conversion type d, b, o or x */
                {
                    unsigned long x;
                    char          buffer[BITS_PER_BYTE * sizeof(unsigned long) + 1];
                    p.edited_string_length = 0;
                    if (long_argument)
                    {
                        //x                 = *(unsigned long *) argument_pointer;
                        //argument_pointer += sizeof(unsigned long) / sizeof(int);
                        va_arg(va, unsigned int);
                    }
                    else if (control_char == 'd')
                        //x = (long) *argument_pointer++;
                        x = va_arg(va, long);
                    else
                        //x = (unsigned) *argument_pointer++;
                        x = va_arg(va, int);
                    if (control_char == 'd' && (long) x < 0)
                    {
                        p.options |= MINUS_SIGN;
                        x          = -(long) x;
                    }
                    do
                    {
                        int c;
                        c = x % base + '0';
                        if (c > '9')
                        {
                            if (p.options & CAPITAL_HEX)
                                c += 'A' - '9' - 1;
                            else
                                c += 'a' - '9' - 1;
                        }
                        buffer[sizeof(buffer) - 1 - p.edited_string_length++] = c;
                    } while ((x /= base) != 0);
                    if (precision >= 0 && precision > p.edited_string_length)
                        p.leading_zeros = precision - p.edited_string_length;
                    output_field(&p, buffer + sizeof(buffer) - p.edited_string_length);
                }
                control_char = *control_string++;
            }
        }
        else
        {
            output_and_count(&p, control_char);
            control_char = *control_string++;
        }
    }
    return p.number_of_output_chars;
}

#if 1
int
ithPrintf(
    char    *control,
    ...)
{
    return 0;
}
#else
int
ithPrintf(
    char    *control,
    ...)
{
    va_list va;

    va_start(va,control);
    gPrint  = 0;
    gBuf    = (unsigned char*)gtAudioPluginBuffer;
    ithGPrintf(control, va);
    va_end(va);

    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
    return 0;
}
#endif
#endif

void
AudioPluginAPI(
    int nType)
{
    uint16  nRegister;
    int     i;

    nRegister = (SMTK_AUDIO_PROCESSOR_ID << 14) | nType;
    switch (nType)
    {
    case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:
    default:
        break;
    }

    setAudioPluginMessageStatus(nRegister);
    i = 200000*20;
    do
    {
        nRegister = getAudioPluginMessageStatus();
        nRegister = (nRegister & 0xc000) >> 14;
        if (nRegister== SMTK_MAIN_PROCESSOR_ID)
        {
            break;
        }
        i--;
    } while(i);
}

///////////////////////////////////////////////////////////////////////////
// ¦¬°e command API
XCPU_COMMAND
RISC_GetCommand(
    void)
{
    XCPU_COMMAND    result      = XCPU_COMMAND_NULL;
    uint16          commandReg  = MMIO_Read(XCPU_CMD_REG);
    uint            cpuId       = (commandReg >> 14);
    uint            commandId   = (commandReg &  0x3FFF);


    if (CPU_ID_ARM == cpuId
     && XCPU_COMMAND_NULL < commandId && commandId < XCPU_COMMAND_MAX)
    {
        result = commandId;
    }

    return result;
}

void
RISC_SetReturnCode(
    XCPU_RETURN_CODE    code)
{
    MMIO_Write(XCPU_CMD_REG, (CPU_ID_RISC << 14) | code);
}
///////////////////////////////////////////////////////////////////////////

void
RISC_GetConfig(
    void)
{
    uint32 i;

    dc_invalidate();
    gtRisc.pRiscBaseAddr     = (uint8*)GET32((int)gptShareInfo->pRiscBaseAddr);

    //ithPrintf("[TSDEMUX]validChannelCount(%d) (%d)\n", gtRisc.validChannelCount, gptShareInfo->validChannelCount);
    for (i = 0; i < MAX_CHANNEL_COUNT; ++i)
    {
        CHANNEL_INFO*   ptChanInfo = &gptShareInfo->tChannelArray[i];
        uint32          bValidChannel   = GET32(ptChanInfo->bValidChannel);

        if (!gtRisc.bValidChannel[i])
        {
            if (bValidChannel)  // stop -> run
            {
                uint32          validPidCount;
                uint32          j;

                //ithPrintf("[TSDEMUX]%d\n", __LINE__);
                RISC_TsiInit(i, ptChanInfo, gtRisc.pRiscBaseAddr);
                #if 0
                ithPrintf("[TSDEMUX]pInputTsBuffer(0x%X)tsBufferSize(%d)tsBufferWriteIdx(%d)tsBufferReadIdx(%d)\n",
                    (uint8*)GET32((int)ptChanInfo->pInputTsBuffer),  // add memory offset?
                    GET32(ptChanInfo->tsBufferSize),
                    GET32(ptChanInfo->tsBufferWriteIdx),
                    GET32(ptChanInfo->tsBufferReadIdx));
                #endif

                TS_Init(i);
                //ithPrintf("[TSDEMUX]%d\n", __LINE__);

                validPidCount = GET32(ptChanInfo->validPidCount);
                //ithPrintf("[TSDEMUX]validPidCount(%d)\n", validPidCount);
                for (j = 0; j < validPidCount; ++j)
                {
                    #if 0
                    ithPrintf("[TSDEMUX]pid(%d)pOutPesBuffer(0x%X)pOutPesSampleSize(%d)validPesSampleCount(%d)pesBufferWriteIdx(%d)pesBufferReadIdx(%d)\n",
                         GET32(ptChanInfo->tPidInfo[j].pid),
                         GET32((int)ptChanInfo->tPidInfo[j].pOutPesBuffer),
                         GET32(ptChanInfo->tPidInfo[j].pOutPesSampleSize),
                         GET32(ptChanInfo->tPidInfo[j].validPesSampleCount),
                         GET32(ptChanInfo->tPidInfo[j].pesBufferWriteIdx),
                         GET32(ptChanInfo->tPidInfo[j].pesBufferReadIdx));
                    #endif
                    TS_InsertEsPid(i, j, &ptChanInfo->tPidInfo[j], gtRisc.pRiscBaseAddr);
                }
            }
        }
        else
        {
            if (!bValidChannel) // run -> stop
            {
                TS_Terminate(i);
                RISC_TsiTerminate(i);
            }
        }

        gtRisc.bValidChannel[i] = bValidChannel;
    }
}

extern RISC_TSI             gtRiscTsi[MAX_CHANNEL_COUNT];

void
RISC_ParserRun(
    void)
{
    uint32 i;

    for (i = 0; i < MAX_CHANNEL_COUNT; ++i)
    {
        if (gtRisc.bValidChannel[i])
        {
            uint8*  pData;
            uint32  size;

            if (RISC_TsiGetReady(i, &pData, &size))
            {
                size = TS_Decode(i, pData, size);
                RISC_TsiSetFree(i, size);
            }

            // +
            {
                RISC_TSI* ptTsi = &gtRiscTsi[i];
                ithPrintf("[TSDEMUX]tsi w(%d)r(%d)\n", ptTsi->tsBufferWriteIdx, ptTsi->tsBufferReadIdx);
            }
            // -
        }
    }
}

void
RISC_Loop(
    void)
{
    switch (RISC_GetCommand())
    {                    
    default:
        RISC_ParserRun();
        break;

    case XCPU_COMMAND_CFG_CHANGE:
        RISC_GetConfig();
        RISC_SetReturnCode(XCPU_RETURN_CODE_DONE);
        break;
    }
}

/**************************************************************************************
 * Function     : main
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
#if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(aacdecode_task, params)
#else
int main(int argc, char **argv)
#endif
{
#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
    int                 *codecStream;
    AUDIO_CODEC_STREAM  *audioStream;
#endif

#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
    codecStream = (int*)CODEC_STREAM_START_ADDRESS;
    audioStream = (AUDIO_CODEC_STREAM*)*codecStream;
    audioStream->codecStreamBuf     = (uint8*)&gtShareInfo;
    audioStream->codecStreamLength  = sizeof(gtShareInfo);
    gtAudioPluginBuffer             = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength       = (uint*)audioStream->codecAudioPluginLength;
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);

#endif

#ifdef AUDIO_PLUGIN_MESSAGE_QUEUE
#if 0
    ithPrintf("[TSDEMUX] gtShareInfo(0x%X) (0x%X) codecStream(%X) audioStream(0x%X) &codec_start_addr(%X)\n", audioStream->codecStreamBuf,
        &(audioStream->codecStreamBuf),
        codecStream,
        audioStream,
        &codec_start_addr);
#endif
    //ithPrintf("[TSDEMUX]\n");

#endif

    while (1)
        RISC_Loop();

#ifndef __OPENRTOS__
    return 0;
#endif
}

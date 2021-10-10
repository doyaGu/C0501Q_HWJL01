
#include "ite/audio.h"
#include "ite/ith.h"
#include "ite/ite_risc.h"

bool audioKeySoundPaused = false;

int32_t
iteAudioActiveEngine(
    void)
{
	return 0;
}

int32_t
iteAudioActiveMidiEngine(
    uint8_t* midiData,
    unsigned long  midiDataSize)
{
	return 0;
}

__inline int32_t
iteAudioChangeEqTable(
    ITE_AUDIO_EQINFO* eqTable)
{
    return 0;
}

int32_t
iteAudioChangeEqualizer(
    ITE_AUDIO_EQTYPE eqType)
{
    return 0;
}	

int32_t
iteAudioPause(
    bool enable)
{
    return 0;
}
int32_t
iteAudioStop(
    void)
{
	return 0;
}

int32_t
iteAudioWriteStream(
    uint8_t* stream,
    unsigned long  length)
{
	return 0;
}

int32_t
iteAudioGenWaveDecodeHeader(ITE_WaveInfo* wavInfo,uint8_t* wave_header)
{
    return 0;
}

int32_t
iteAudioGetFlashAvailableBufferLength(
    int nAudioInput,
    unsigned long* bufferLength)
{
    return 0;
}

int32_t
iteAudioGetFlashInputStatus(int nInputNumber,int* nFormat, int* nUsing)
{
    return 0;
}	

int32_t
iteAudioOpenEngine(
    ITE_AUDIO_ENGINE audio_type)
{
    return 0;
}

int32_t
iteAudioStopQuick(
    void)
{
   return 0;
}
int32_t
iteAudioWriteWavInfo()
{
    return 0;
}

int32_t
iteAudioWriteWmaInfo()
{
   return 0;
}

int32_t
iteAudioSetMusicWithoutASFHeader()
{
   return 0;
}

int32_t
iteAudioSetWaveDecodeHeader(ITE_WaveInfo wavInfo)
{
   return 0;
}

int32_t
iteAudioWriteFlashStream(
    int    nAudioInput,
    uint8_t* stream,
    unsigned long  length)
{
    return 0;
}

int32_t
iteAudioGetAvailableBufferLength(
    ITE_AUDIO_BUFFER_TYPE bufferType,
    unsigned long* bufferLength)
{

    return 0;
}    
 int32_t
iteAudioSetFlashInputStatus(int nInputNumber,int nFormat, int nUsing)
{ 
  return 0;
  
}  
int32_t
iteAudioStopEngine(
    void)
{
    return 0;
}
int32_t
iteAudioGetDecodeTime(
    uint32_t *time)
{
    return 0;
}
int32_t
iteAudioGetAttrib(
    const ITE_AUDIO_ATTRIB attrib,
    void* value)
{
    return 0;
}

int32_t
iteAudioSetAttrib(
    const ITE_AUDIO_ATTRIB attrib,
    const void* value)
{
    return 0;
}

int32_t
iteAudioSetEqualizer(
    bool enableEq)
{
    return 0;
}

int32_t
iteAudioTerminate(
    void)
{
    return 0;
}

unsigned int
iteAecCommand(
    unsigned short  command,
    unsigned int    param0,
    unsigned int    param1,
    unsigned int    param2,
    unsigned int    param3,
    unsigned int*   value)
{
    return 0;   
}

bool
ithCodecCommand(
    int command,
    int parameter0,
    int parameter1,
    int parameter2)
{
    return true;
}

void
ithCodecCtrlBoardWrite(
    uint8_t* data,
    int length)
{
}

void
ithCodecCtrlBoardRead(
    uint8_t* data,
    int length)
{
}

void
ithCodecHeartBeatRead(
    uint8_t* data,
    int length)
{
}

int32_t
iteRiscOpenEngine(
    ITE_RISC_ENGINE engine_type, uint32_t bootmode)
{

}

int32_t
iteRiscTerminateEngine(
    void)
{
}


#ifndef __WIN32_H__
#define __WIN32_H__

#if defined(USE_MIXER)
#define ENABLE_MIXER
#endif

void    pauseDAC(int);
void    deactiveDAC(void);
void    dc_invalidate(void);
void    or32_delay(int ms);
int     isEncEOF(void);
void    setEncEOF(void);
void    ClearEncIpEOF(void);
int     isEncIpPAUSE(void);
int     isEncSTOP(void);

int     isDecIpEOF(void);
void    ClearDecIpEOF(void);
int     isDecIpSTOP(void);
void    ClearDecIpSTOP(void);
int     isDecIpPAUSE(void);

void    SetInRdPtr(int idx);
void    SetInWrPtr(int idx);
int     GetInWrPtr(void);
int     GetOutRdPtr(void);
void    SetOutWrPtr(int idx);
void    SetOutRdPtr(int idx);

#if defined(ENABLE_MIXER)
int     GetMixWrPtr(void);
int     SetMixRdPtr(int rdptr);
void    GetMixPCMParam(int* sample, int* channel);
int     isEnableMix(void);
int     isMixEOF(void);
void    ClearMixEOF();
#endif // defined(ENABLE_MIXER)

int     MMIO_Read(int reg);
void    MMIO_Write(int reg, int data);

void    win32_init(int argc, char **argv);
void    win32_destory();
//void win32_setBuffer(int ipSize, int opSize);

void    initADC(
    unsigned char *inbuf,
    int channel,
    int sample,
    int inbuflen,
    int overhead);

void    initDAC(
    unsigned char *outbuf,
    int channel,
    int sample,
    int outbuflen,
    int overhead);

void    initCODEC(
    unsigned char *inbuf,
    unsigned char *outbuf,
    int channel,
    int sample,
    int inbuflen,
    int outbuflen,
    int overhead);

#endif //__WIN32_H__


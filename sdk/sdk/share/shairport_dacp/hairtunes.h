#ifndef _HAIRTUNES_H_
#define _HAIRTUNES_H_
int hairtunes_init(char *pAeskey, char *pAesiv, char *fmtpstr, int pCtrlPort, int pTimingPort,
         int pDataPort, char *pRtpHost, char*pPipeName, char *pLibaoDriver, char *pLibaoDeviceName, char *pLibaoDeviceId,
         int bufStartFill,int nInit);
void hairtunes_flush(int nFlushI2S);
void hairtunes_vol(double f);
int hairtunesSetAudioOutputWakeUp(int nWakeUp);

// default buffer size
// needs to be a power of 2 because of the way BUFIDX(seqno) works
#define BUFFER_FRAMES  512+256

#endif 

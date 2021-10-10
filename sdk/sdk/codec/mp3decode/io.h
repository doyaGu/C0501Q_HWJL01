#ifndef __IO_H__
#define __IO_H__

void init_inputBuf(void);
int  streamHasNBytes(int nwaitbytes);
int  streamRead(unsigned char *bufptr, int nbytes);
int  streamSeekSyncword(int updateStreamRdPtr, int* nbytes);
int  streamCompareSyncword(unsigned char* sync);

int  outputpcm(short *pcmbuf, int nch,int nNewCh, int nsamples, int sr, int nsr);  // add parameter nsr to change sampling rate
int  UpSampling(int nInputSampleRate,int nOutputSampleRate,int nSamples,int nChannels,short *pInputPcmbuf,short *pOutputPcmBuf,int nOutputBits);
int  GetUpSampleRate(int nInputSampleRate);
int  mixpcm(int pcm0eof);

void ResetRdBufferPointer();
void ClearRdBuffer(void);
void ClearI2SBuffer();

void SetFrameNo(int frameno);
void GetFrameNo(int *frameno);
void GetTime(unsigned int *pTime);

void SetChksumRegionStart(unsigned char *baseptr, int startidx);
void SetChksumRegionEnd(unsigned char *baseptr, int endidx);
int ithMp3Printf(char* control, ...);

#endif // __IO_H__


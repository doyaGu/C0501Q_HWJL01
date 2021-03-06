#ifndef __WAVFILEFMT_H__
#define __WAVFILEFMT_H__

int TestBigEndian(void);
void WriteWAVHeader(FILE * fp, int sampRateOut, int bitsPerSample,
                    int nChans);
void UpdateWAVHeader(char *pFileName);
int  ReadWAVHeader(FILE * fp, int *sampRate, int *nChans ,int *bitsPerSample);

#endif // __WAVFILEFMT_H__

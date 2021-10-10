#ifndef __G711_H__
#define __G711_H__

void audioAULawInit(int nch, int sr, int IsALaw);
void audioAULawPause(void);
void audioAULawResume(void);
void audioAULawClose(void);
void audioAULawDec(unsigned char *ptr, int size);

#endif // __G711_H__

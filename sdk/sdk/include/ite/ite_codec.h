#ifndef ITE_CODEC_H
#define ITE_CODEC_H

void
iteCodecOpenEngine(
    void);

unsigned long long
iteCodecWiegandReadCard(
    int index);

void
iteCodecPrintfWrite(
    char* string,
    int length);

#endif // ITE_CODEC_H
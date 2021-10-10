#ifndef __QRENC_H__
#define __QRENC_H__
#include "qrencode.h"

extern int writePNG(QRcode *qrcode, const char *outfile);

extern void qrencode(const unsigned char *intext, int length, const char *outfile);

#endif /* __QRENC_H__ */
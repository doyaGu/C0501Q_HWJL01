#ifndef	__ITE_COMMON_H__
#define	__ITE_COMMON_H__

//#include "mmp_types.h" 

//#define __cpu_to_le16       cpu_to_le16

long simple_strtol(const char *cp,char **endp,unsigned int base);

void iteDbgPrintf(int level, char *fmt, ...);

int find_next_zero_bit(unsigned int* map, unsigned char startBit, unsigned char endBit);

void clear_bit(unsigned int* map, unsigned char i);

unsigned int get32(unsigned char *ul);

unsigned short get16(unsigned char *ul);

unsigned char *put32(unsigned char *cp, unsigned int x);

unsigned char *put16(unsigned char *cp, unsigned short x);

#endif


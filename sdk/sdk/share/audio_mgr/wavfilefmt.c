

#include "resample.h"

#if 1 //defined(WIN32) || defined(__CYGWIN__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG_FOURCC_RIFF       0x52494646
#define TAG_FOURCC_WAVE       0x57415645
#define TAG_FOURCC_fmt        0x666d7420
#define TAG_FOURCC_data       0x64617461
#define TAG_FORMAT_PCM        0x0001

int TestBigEndian(void)
{
    unsigned long test = 0x000000FF;
    char *temp = (char *) &test;
    // on big endian machines, the first byte should be 0x00, on little endian 0xFF.
    return (temp[0] == 0);
}

static void pack32BE(unsigned long ulValue, unsigned char **ppBuf,
                     unsigned long *pulLen)
{
    if (ppBuf && *ppBuf && pulLen && *pulLen >= 4) {
        unsigned char *pBuf = *ppBuf;
        pBuf[0] = (unsigned char) ((ulValue & 0xFF000000) >> 24);
        pBuf[1] = (unsigned char) ((ulValue & 0x00FF0000) >> 16);
        pBuf[2] = (unsigned char) ((ulValue & 0x0000FF00) >> 8);
        pBuf[3] = (unsigned char) (ulValue & 0x000000FF);
        *ppBuf += 4;
        *pulLen -= 4;
    }
}

static void pack32LE(unsigned long ulValue, unsigned char **ppBuf,
                     unsigned long *pulLen)
{
    if (ppBuf && *ppBuf && pulLen && *pulLen >= 4) {
        unsigned char *pBuf = *ppBuf;
        pBuf[0] = (unsigned char) (ulValue & 0x000000FF);
        pBuf[1] = (unsigned char) ((ulValue & 0x0000FF00) >> 8);
        pBuf[2] = (unsigned char) ((ulValue & 0x00FF0000) >> 16);
        pBuf[3] = (unsigned char) ((ulValue & 0xFF000000) >> 24);
        *ppBuf += 4;
        *pulLen -= 4;
    }
}

static void pack16LE(unsigned short usValue, unsigned char **ppBuf,
                     unsigned long *pulLen)
{
    if (ppBuf && *ppBuf && pulLen && *pulLen >= 2) {
        unsigned char *pBuf = *ppBuf;
        pBuf[0] = (unsigned char) (usValue & 0x000000FF);
        pBuf[1] = (unsigned char) ((usValue & 0x0000FF00) >> 8);
        *ppBuf += 2;
        *pulLen -= 2;
    }
}

void WriteWAVHeader(FILE * fp, int sampRateOut, int bitsPerSample,
                    int nChans)
{
    // Only works for numChannels <= 2 now
    if (fp && nChans <= 2) {
        unsigned char ucTmp[44];
        unsigned char *pTmp = &ucTmp[0];
        unsigned long ulTmp = sizeof(ucTmp);
        int lByteRate = sampRateOut * nChans * 2;
        int lBlockAlign = nChans * 2;
        // Pack the RIFF four cc
        pack32BE(TAG_FOURCC_RIFF, &pTmp, &ulTmp);
        // Pack 0 for the RIFF chunk size - update in UpdateWAVHeader.
        pack32LE(0, &pTmp, &ulTmp);
        // Pack the WAVE four cc
        pack32BE(TAG_FOURCC_WAVE, &pTmp, &ulTmp);
        // Pack the 'fmt ' subchunk four cc
        pack32BE(TAG_FOURCC_fmt, &pTmp, &ulTmp);
        // Pack the fmt subchunk size
        pack32LE(16, &pTmp, &ulTmp);
        // Pack the audio format
        pack16LE((unsigned short) TAG_FORMAT_PCM, &pTmp, &ulTmp);
        // Pack the number of channels
        pack16LE((unsigned short) nChans, &pTmp, &ulTmp);
        // Pack the sample rate
        pack32LE((unsigned long) sampRateOut, &pTmp, &ulTmp);
        // Pack the byte rate
        pack32LE((unsigned long) lByteRate, &pTmp, &ulTmp);
        // Pack the block align (ulChannels * 2)
        pack16LE((unsigned short) lBlockAlign, &pTmp, &ulTmp);
        // Pack the bits per sample
        pack16LE((unsigned short) bitsPerSample, &pTmp, &ulTmp);
        // Pack the 'data' subchunk four cc
        pack32BE(TAG_FOURCC_data, &pTmp, &ulTmp);
        // Pack the data subchunk size (0 for now - update in UpdateWavHeader)
        pack32LE(0, &pTmp, &ulTmp);
        // Write out the wav header
        fwrite(&ucTmp[0], 1, 44, fp);
    }
}

void UpdateWAVHeader(char *pFileName)
{
    if (pFileName && strcmp(pFileName, "-")) {
        /* Re-open the file for updating */
        FILE *fp = fopen((const char *) pFileName, "r+b");
        if (fp) {
            unsigned char ucTmp[4];
            unsigned long ulFileSize = 0;
            unsigned long ulRIFFSize = 0;
            unsigned long ulDataSize = 0;
            unsigned char *pTmp = NULL;
            unsigned long ulLen = 0;
            /*
             * Compute the RIFF chunk size and the
             * data chunk size from the size of the file.
             */
            fseek(fp, 0, SEEK_END);
            ulFileSize = (unsigned long) ftell(fp);
            ulRIFFSize = ulFileSize - 8;
            ulDataSize = ulFileSize - 44;
            /* Seek to the RIFF chunk size */
            fseek(fp, 4, SEEK_SET);
            /* Set up the packing buffer */
            pTmp = &ucTmp[0];
            ulLen = sizeof(ucTmp);
            /* Pack the RIFF chunk size */
            pack32LE(ulRIFFSize, &pTmp, &ulLen);
            /* Write out the buffer */
            fwrite(&ucTmp[0], 1, 4, fp);
            /* Seek to the beginning of the data chunk size */
            fseek(fp, 40, SEEK_SET);
            /* Set up the packing buffer */
            pTmp = &ucTmp[0];
            ulLen = sizeof(ucTmp);
            /* Pack the data chunk size */
            pack32LE(ulDataSize, &pTmp, &ulLen);
            /* Write out the buffer */
            fwrite(&ucTmp[0], 1, 4, fp);
            /* Close the file */
            fclose(fp);
        }
    }
}

int ReadWAVHeader(FILE * fp, int *sampRate, int *nChans ,int *bitsPerSample)
{
    char  b_val[4];
    short s_val;
    int   l_val;
    unsigned long ulFileSize = 0;
    
    fseek(fp, 0, SEEK_END);
    ulFileSize = (unsigned long) ftell(fp);

    *sampRate = *nChans = 0;

    fseek(fp, 0, SEEK_SET);
    if (fp) {
        fread(b_val, 1, 4, fp);
        if (strncmp(b_val, "RIFF", 4)) {
            printf("ReadWAVHeader(): not wav format file\n");
            return -1;
        }

        fseek(fp, 8, SEEK_SET);
        fread(b_val, 1, 4, fp);
        if (strncmp(b_val, "WAVE", 4)) {
            printf("ReadWAVHeader(): not wav format file\n");
            return -1;
        }

        fread(b_val, 1, 4, fp);
        if (strncmp(b_val, "fmt ", 4)) {
            printf("ReadWAVHeader(): not wav format file\n");
            return -1;
        }


        fseek(fp, 22, SEEK_SET);
        fread(&s_val, 2, 1, fp);
        *nChans = (int)s_val;

        fread(&l_val, 4, 1, fp);
        *sampRate = l_val;

        fseek(fp, 34, SEEK_SET);
        fread(&s_val, 2, 1, fp);
        *bitsPerSample = (int)s_val;
        
        // set to the end of wave header
        fseek(fp, 44, SEEK_SET);

        printf("ReadWAVHeader %d %d %d %d\n",*nChans,*sampRate,*bitsPerSample,ulFileSize);
        
    } else {
        return -1;
    }

    return 0;
}

#endif // defined(WIN32) || defined(__CYGWIN__)


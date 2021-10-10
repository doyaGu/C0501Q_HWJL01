#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libamr.h"

#define MAX_SERIAL_SIZE     (244)
#define MAX_PACKED_SIZE     (MAX_SERIAL_SIZE / 8 + 2)
#define SERIAL_FRAMESIZE    (1+MAX_SERIAL_SIZE+5)
#define L_FRAME             (160)

int main (int argc, char *argv[])
{
    FILE            *file_serial;
    char            *serialFileName = NULL;
    FILE            *file_syn;
    char            *fileName = NULL;

    unsigned char   q, ft;
    unsigned char   packed_bits[MAX_PACKED_SIZE];

    short           synth[L_FRAME];
    int             iOutSize;

    Word16 packed_size[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

    // OPEN FILE
    serialFileName = argv[1];
    if ((file_serial = fopen (serialFileName, "rb")) == NULL) {
        fprintf (stderr, "Input file '%s' does not exist !!\n", serialFileName);
        exit(0);
    }

    fileName = argv[2];
    if ((file_syn = fopen (fileName, "wb")) == NULL) {
        fprintf (stderr, "Cannot create output file '%s' !!\n", fileName);
        exit(0);
    }

    // INIT
    AMR_initDecode();

    // DEOCDE
    while (fread (&packed_bits[0], sizeof(UWord8), 1, file_serial) == 1) {
        q  = (packed_bits[0] >> 2) & 0x01;
        ft = (packed_bits[0] >> 3) & 0x0F;
        fread (&packed_bits[1], sizeof(UWord8), packed_size[ft], file_serial);

        //AMR_decode(Word8* pIn, Word32 iInSize, Word16* pOut, Word32 *iOutSize);
        iOutSize = L_FRAME * 2;
        if (AMR_decode(packed_bits, packed_size[ft] + 1, (Word8*)synth, &iOutSize) == false) {
            fprintf (stderr, "\nDecode FAIL!!\n");
            break;
        }

        fwrite (synth, sizeof (Word16), iOutSize / 2, file_syn);
        fflush(file_syn);
        fprintf (stdout, "\rframe=%d  ", AMR_getDecodeFrame());
    }

    // END
    AMR_finalizeDecode();

    // CLOSE FILE
    fclose(file_serial);
    fclose(file_syn);

    fprintf (stdout, "\n");

    return 0;
}

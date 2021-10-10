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

    short           synth[L_FRAME];
    UWord8          serial[SERIAL_FRAMESIZE];
    int             iOutSize;

    MODE_BITRATE    bitrate;
    MODE_MODE       vbr;

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
    bitrate = MODE_BR122;
    vbr = MODE_CBR;

    AMR_initEncode(bitrate, vbr);

    // Encode
    while (!feof(file_serial)) {
        fread (synth, sizeof(Word16), 160, file_serial);

        iOutSize = SERIAL_FRAMESIZE;
        if (AMR_encode((UWord8*)synth, L_FRAME * 2, serial, &iOutSize) == false) {
            fprintf (stderr, "\nEncode FAIL!!\n");
            break;
        }

        fwrite (serial, sizeof(UWord8), iOutSize, file_syn);
        fflush(file_syn);
        fprintf (stdout, "\rframe=%d  ", AMR_getEncodeFrame());
    }

    // END
    AMR_finalizeEncode();

    // CLOSE FILE
    fclose(file_serial);
    fclose(file_syn);

    fprintf (stdout, "\n");

    return 0;
}

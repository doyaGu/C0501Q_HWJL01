#include <stdio.h>
#include <stdlib.h>

int WinAmp[]     = {0, 60, 170, 310, 600, 1000, 3000, 6000, 12000, 14000, 16000, 25000}; 
int BandCenter[] = {0, 82, 251, 404, 557, 732, 907, 1098, 1328, 1600, 3300, 6000, 13000, 16000, 25000, -1};

int main(int argc, char **argv) {
    FILE *fp;
    char eqtable[11];
    int  i, j;
    int  lo, hi, f;
    int  left, right;
    char filename[128];
    char name[128];

    if (argc!=2) {
        printf("Usage: readeq filename.eqf\n");
        exit(-1);
    }

    strcpy(filename, argv[1]);
    strcpy(name, argv[1]);

    i=strlen(name);
    while(i>0 && name[i--] != '.');
    name[i+1]=0;

    if ((fp=fopen(filename, "rb"))==NULL) {
        printf("Can not open file %s\n", filename);
        exit(-1);
    }

    fseek(fp, 0x120, SEEK_SET);
    fread(eqtable, sizeof(char), 10, fp);
    eqtable[10] = eqtable[9];

/*
    for(i=0; i<sizeof(WinAmp)/sizeof(int)-1; i++) {
        printf("%d %d\n", WinAmp[i+1], (((0x1f - eqtable[i])<<16)*12/0x1f)>>16);
    }
*/

    printf("\n");
    printf("  // %s\n",name);
    printf("  {      1,     1,\n");
    printf("    {");
    for(i=0; i<sizeof(BandCenter)/sizeof(int)-1; i++) {
        printf("%5d, ", BandCenter[i]);
    }

    printf("%5d},\n    {",-1);

    for(i=0; i<sizeof(BandCenter)/sizeof(int)-1; i++) {
        for(j=0; BandCenter[i] > WinAmp[j] && j < 11; j++);

        if (j-1>0) {
            lo = ((0x1f - eqtable[j-2])<<16)*12/0x1f;
            hi = ((0x1f - eqtable[j-1])<<16)*12/0x1f;
            left = WinAmp[j-1];
            right = WinAmp[j];
        } else {
            lo = hi = ((0x1f - eqtable[0])<<16)*12/0x1f;
            left = right = WinAmp[0];
        }

        if (right == left) {
            f = lo>>16;
        } else {
            f = (((hi-lo)*(BandCenter[i]-left)/(right-left)+lo))>>16;
        }

//      printf("(%d) %d(%d) < %d(%d) < %d(%d)\n", i, left, lo>>16, BandCenter[i], f, right, hi>>16);
        printf("%5d, ", f);
    }
    printf("%5d},\n",0);
    printf("  },\n");

    fclose(fp);
}


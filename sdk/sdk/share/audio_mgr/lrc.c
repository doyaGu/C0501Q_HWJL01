
#include <stdio.h>		//for printf,fgets...

#include "audio_mgr.h"

#include <string.h>		//for strlen

#ifdef CFG_AUDIO_MGR_LRC

//#define LRC_DEBUG

#define GB2312_TO_UTF8

#define MAXLINE 256

#define MAXLENGTH 256


#ifndef NULL
#define NULL 0
#endif


static SMTK_AUDIO_LRC_INFO gNumber[MAXLENGTH];
static SMTK_AUDIO_LRC_INFO gNumberSort;
static MMP_WCHAR  gpDestText[MAXLENGTH];
static char pUTF8[MAXLINE];

static int gLrcData=0;

static int LINE = 0;//??歌?所在的行

static int gSort = 0;

int LRCPrase(char *str, int *songTime);
int strtoint(char *str);

//把char???int
int chartoint(char ch){
    return ch - '0';
}

int strtoint(char *str){//?算??，微秒
    if(isdigit(str[0]) && isdigit(str[1])
    && isdigit(str[0]) && isdigit(str[0])
    && isdigit(str[0]) && isdigit(str[0])){
        int mintue = chartoint(str[0]) * 10 + chartoint(str[1]);
        int second = chartoint(str[3]) * 10 + chartoint(str[4]);
        int microsecond = chartoint(str[6]) * 10 + chartoint(str[7]);
        return (mintue * 60 + second) * 1000 + microsecond * 10;
    }
    return -1;
}


#if 0
int
gb2312ToUtf8(
    char*  ptDestText,
    int  nDestLength,
    char*  ptSrcText,
    int  nSrcLength
    )
{
    int  nResult = 0;
    int  i;
    int  nTemp,nTemp1;
    char* buf= 0;
    char* bufDest= 0;
    unsigned int ucs;
    unsigned char b[2];

    if (   !ptDestText
        || !ptSrcText
        || nDestLength <=0
        || nSrcLength <= 0
        )
        return -1;

    nTemp = 0;
    nTemp1 = 0;
    memset(gpDestText,0,sizeof(gpDestText));

//    buf = (char*)ptSrcText;
    bufDest = (char*)gpDestText;
    for(i=0;i<(nSrcLength);)
    {
        b[0] = (unsigned char) ptSrcText[i];
        b[1] = (unsigned char) ptSrcText[i + 1];
        if ( b[0] < 0x80 ) {
          bufDest[nTemp1] = b[0];
          i++;
          nTemp1++;
          nTemp++;
        } else if (gb2312_mbtowc(NULL, &ucs, b, 2) == 2) {
          if (i%2==0){

          } else {
              i++;
              nTemp1++;
              nTemp++;
          }
          gpDestText[i/2] = (unsigned short)ucs;
          i += 2;
          nTemp1+=2;
          nTemp+=2;
        } else {
//          ucs = '?';
//          bufDest[nTemp1] = '?';

          i++;
          nTemp1++;
          nTemp++;
        }


#if 0
        nResult = gb2312_mbtowc(0, &gpDestText[nTemp1], &ptSrcText[i], 2);

        if(nResult == 2)
        {
            i++;
            nTemp++;
        }
        else if((ptSrcText[i]>=0x20) && (ptSrcText[i]<=0x80))
        {
            gpDestText[nTemp1] = ptSrcText[i];
        }
#endif

    }

    bufDest[nTemp1] = 0;
    nResult = 0;

    Unicode32ToUtf8(gpDestText,sizeof(gpDestText),ptDestText,nDestLength);
//  memcpy(ptDestText,bufDest,nTemp1);

}

#else

int
gb2312ToUtf8(
    char*  ptDestText,
    int  nDestLength,
    char*  ptSrcText,
    int  nSrcLength
    )
{
    int  nResult = 0;
    int  i;
    int  nTemp,nTemp1;

    if (   !ptDestText
        || !ptSrcText
        || nDestLength <=0
        || nSrcLength <= 0
        )
        return -1;

    nTemp = 0;
    nTemp1 = 0;
    memset(gpDestText,0,sizeof(gpDestText));

    for(i=0;i<(nSrcLength);i++,nTemp1++)
    {
        //nResult = gb2312_mbtowc(0, &gpDestText[nTemp1], &ptSrcText[i], 2);
        if((ptSrcText[i]>=0x20) && (ptSrcText[i]<=0x80))
        {
            gpDestText[nTemp1] = ptSrcText[i];
        } else  if(gb2312_mbtowc(0, &gpDestText[nTemp1], &ptSrcText[i], 2) == 2)
        {
            i++;
            nTemp++;
        }

    }
    gpDestText[(nSrcLength)-nTemp] = 0;
    nResult = 0;
    Unicode32ToUtf8(gpDestText,sizeof(gpDestText),ptDestText,nDestLength);

}

#endif


//int LRCPrase(char *str, vector<string> &sentences, vector<Number> &songTime){
int LRCPrase(char *str, int *songTime){
    SMTK_AUDIO_LRC_INFO number;
    char *p, *q, *temp;
    int gLoop = 0;
    int nStart=0;
    int i;
    int lrc =0;
    if(strlen(str) == 1){//空行
        return 0;
    }else{

    q = str;
    //?理??的
    nStart = gLrcData;
    lrc = 0;
    while((p = strchr(q, '[')) != NULL && (temp = strchr(q, ']')) != NULL){
        q = p + 1;
        q[temp - q] = '\0';
        //printf("%s\t%d\n", q);

        if((number.time = strtoint(q)) < 0){
    	    return 0;
        }
        number.line = LINE;
        q = temp + 1;
        gNumber[gLrcData].time = number.time;
        gNumber[gLrcData].line = LINE;
        gLoop++;
        gLrcData++;
        lrc = 1;

    }
    if (lrc==0)
        return 0;
    //printf("%s", temp + 1);
    //截取歌?
    p = ++temp;
    while(*temp != NULL){
        temp++;
    }
    p[temp - p] = '\0';
    //printf("%s\n", p);
    if (p[temp-p-1] == 0x0a){
        p[temp-p-1] = '\0';
#ifdef GB2312_TO_UTF8
        memset(pUTF8,0,sizeof(pUTF8));
//        printf("AnalyzeGB2312 %d  \n",AnalyzeGB2312(p,(temp-p)));
        gb2312ToUtf8(pUTF8,sizeof(pUTF8),p,(temp-p));
#endif
        }
    if ( (temp - p)>0 && (temp - p)<256) {
        if (gLoop==1){
#ifdef GB2312_TO_UTF8
            memcpy(gNumber[gLrcData-1].pLrc,pUTF8,sizeof(pUTF8));
#else
            memcpy(gNumber[gLrcData-1].pLrc,p,(temp-p));
#endif
        } else if (gLoop>1){
            for(i=0;i<gLoop;i++){
#ifdef GB2312_TO_UTF8
            memcpy(gNumber[gLrcData-1-i].pLrc,pUTF8,sizeof(pUTF8));
#else
            memcpy(gNumber[gLrcData-1-i].pLrc,p,(temp-p));
#endif
            }
        }
    }

    LINE++;

    if (gLoop>1)
        gSort++;

    return 1;
    }

}



void Swap(int x, int y)
{
  //    int temp = array[x];
    //array[x] = array[y];
    //array[y] = temp;
    memcpy(&gNumberSort,&gNumber[x],sizeof(gNumberSort));
    memcpy(&gNumber[x],&gNumber[y],sizeof(gNumberSort));
    memcpy(&gNumber[y],&gNumberSort,sizeof(gNumberSort));
}


void InsertSort(int size)
{
    int i,j;
    for(i = 0; i < size; i++){
        for(j = i; j > 0; j--){
            if(gNumber[j].time < gNumber[j - 1].time){
                Swap(j, j-1);
            }
        }
    }

}

static char gbuf[MAXLINE];

SMTK_AUDIO_LRC_INFO* smtkAudioMgrGetLrc(char* filename){

    FILE *fd= NULL;
    int i;
    int songTime;
    char *ext ;
    char filename2[1024];

    memcpy(filename2,filename,sizeof(filename2));
    gLrcData = 0;
    //fd = fopen("D:\\audio_testing\\Mp3\\Billboard Hot 100\\y.lrc", "r");

#ifdef LRC_DEBUG
    printf("smtkAudioMgrGetLrc %s #line %d \n",filename,__LINE__);
#endif
    memset(gNumber,0,sizeof(gNumber));

    if(filename == NULL){
        printf("[Audiomgr]smtkAudioMgrGetLrc not exit file %s , #line %d  \n",__FILE__,__LINE__);
        return 0;
    }

    ext = strrchr(filename2, '.');
    if (!ext) {
        printf("[Audiomgr]smtkAudioMgrGetLrc  Invalid file name: %s\n", filename2);
        return 0;
    }
    ext++;
    printf("%s \n",ext);
    memcpy(ext,"lrc",3);
    printf("%s \n",ext);


    fd = fopen(filename2,"r");

#ifdef LRC_DEBUG
    printf("smtkAudioMgrGetLrc %s , %x #line %d  \n",filename2,fd,__LINE__);
#endif

    if(fd == NULL){
        printf("[Audiomgr]smtkAudioMgrGetLrc not exit file %s , #line %d  \n",filename2,__LINE__);
        return 0;
    }

//    malloc_stats();
    while(fgets(gbuf, MAXLINE, fd) != NULL){
        //LRCPrase(buf, sentences, songTime);
        LRCPrase(gbuf, &songTime);

    }
    /*
    for (i=0;i<=gLrcData;i++)
        printf("%d %s ",gNumber[i].time,gNumber[i].pLrc);
    */

    //ptLrc = gNumber;

    if (gSort!=0){
        printf("\n smtkAudioMgrGetLrc need to sort \n");

        InsertSort(gLrcData);

#ifdef LRC_DEBUG
        for (i=0;i<=gLrcData;i++)
            printf("%d %s \n",gNumber[i].time,gNumber[i].pLrc);
#endif

    } else {

#ifdef LRC_DEBUG
        printf("\n smtkAudioMgrGetLrc don't sort \n");
#endif
    }

    gNumber[0].line = gLrcData;

    fclose(fd);

    return gNumber;
}

#else

SMTK_AUDIO_LRC_INFO* smtkAudioMgrGetLrc(char* filename){

    return NULL;
}

#endif



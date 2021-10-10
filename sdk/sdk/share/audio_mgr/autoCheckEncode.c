
#include <stdio.h>		//for printf,fgets...
#include <stdlib.h>		//for exit

typedef struct CheckEnc{
	int ascii;
	int symbol;
    int common;
    int rare;
    int unknow;
};

static struct CheckEnc gRes;

//統計解讀為ASCII、符號、常用字、次常用字及亂碼(非有效字元)字數
static int Ascii, Symbol, Common, Rare, Unknow;

/// 亂碼指標(數值愈大，不是該編碼的機率愈高)
/// </summary>
// 1: guess yes , 0: guess no
int guessEncode(int length)
{
    int total;

    if (gRes.common*2 > length)
        return 1;

    if ( (gRes.common + gRes.rare)*2 > length)
        return 1;

/*
    total = gRes.ascii + gRes.symbol + gRes.common + gRes.rare + gRes.unknow;

    if (total == 0)
        return 0;

    return (float)(gRes.rare + gRes.unknow * 3) / total;*/

    return 0;

}

//試著分析成GB2312，取得分析報告
int AnalyzeGB2312(char* data,int length)
{
    int isDblBytes = 0;
    char dblByteHi;
    int i;
    memset(&gRes,0,sizeof(gRes));

    for (i=0;i<length;i++)
    {

        if (isDblBytes)
        {
            if (data[i] >= (char)0xa1 && data[i] <= (char)0xfe)
            {
                if (dblByteHi >= (char)0xa1 && dblByteHi <= (char)0xa9)
                {
                    gRes.symbol++; //符號
                }
                else if (dblByteHi >= (char)0xb0 && dblByteHi <= (char)0xd7)
                {
                    gRes.common++; //一級漢字(常用字)
                }
                else if (dblByteHi >= (char)0xd8 && dblByteHi <= (char)0xf7)
                {
                    gRes.rare++; //二級漢字(次常用字)
                }
                else
                {
                    gRes.unknow++; //無效字元
                }
            }
            else
            {
                gRes.unknow++; //無效字元
                isDblBytes = 0;
            }
        }
        else if ( data[i] >= (char)0xa1 && data[i] <= (char)0xf7)
        {
            isDblBytes = 1;
            dblByteHi = data[i];
        }
        else if (data[i] < (char)0x80)
        {
            gRes.ascii++;
        }

    }

    return guessEncode(length);

}
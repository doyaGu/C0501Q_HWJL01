/*****************************************************************************
 * 
 * Copyright (c) 2008-2010, CoreCodec, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of CoreCodec, Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CoreCodec, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CoreCodec, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "corec/str.h"

static const uint16_t utf_sort_en[] = {
0x0000,0x0020,0x0030,0x0040,0x0050,0x0060,0x0070,0x0080,0x0090,0x05e0,0x05f0,0x0600,0x0610,0x0620,0x00a0,0x00b0,
0x00c0,0x00d0,0x00e0,0x00f0,0x0100,0x0110,0x0120,0x0130,0x0140,0x0150,0x0160,0x0170,0x0180,0x0190,0x01a0,0x01b0,
0x05c0,0x0630,0x0640,0x0650,0x0660,0x0670,0x0680,0x03d0,0x0690,0x06a0,0x06b0,0x09c0,0x06c0,0x03e0,0x06d0,0x06e0,
0x0b80,0x0bc0,0x0be0,0x0c00,0x0c20,0x0c30,0x0c40,0x0c50,0x0c60,0x0c70,0x06f0,0x0700,0x09d0,0x09e0,0x09f0,0x0710,
0x0720,0x0c81,0x0d41,0x0d51,0x0d91,0x0dd1,0x0e61,0x0e81,0x0eb1,0x0ec1,0x0f51,0x0f61,0x0f81,0x0fd1,0x0fe1,0x1041,
0x10f1,0x1101,0x1111,0x1151,0x11a1,0x11f1,0x1281,0x1291,0x12a1,0x12b1,0x12e1,0x0730,0x0740,0x0750,0x0760,0x0780,
0x0790,0x0c80,0x0d40,0x0d50,0x0d90,0x0dd0,0x0e60,0x0e80,0x0eb0,0x0ec0,0x0f50,0x0f60,0x0f80,0x0fd0,0x0fe0,0x1040,
0x10f0,0x1100,0x1110,0x1150,0x11a0,0x11f0,0x1280,0x1290,0x12a0,0x12b0,0x12e0,0x07a0,0x07b0,0x07c0,0x07d0,0x01c0,
0x01d0,0x01e0,0x01f0,0x0200,0x0210,0x0220,0x0230,0x0240,0x0250,0x0260,0x0270,0x0280,0x0290,0x02a0,0x02b0,0x02c0,
0x02d0,0x02e0,0x02f0,0x0300,0x0310,0x0320,0x0330,0x0340,0x0350,0x0360,0x0370,0x0380,0x0390,0x03a0,0x03b0,0x03c0,
0x05d0,0x07e0,0x0a50,0x0a60,0x0a70,0x0a80,0x07f0,0x0a90,0x0800,0x0aa0,0x0c90,0x0a10,0x0ab0,0x03f0,0x0ac0,0x0810,
0x0ad0,0x0a00,0x0bf0,0x0c10,0x0820,0x0ae0,0x0af0,0x0b00,0x0830,0x0bd0,0x1050,0x0a20,0x0b90,0x0ba0,0x0bb0,0x0840,
0x0cb1,0x0ca1,0x0cc1,0x0d01,0x0cd1,0x0d11,0x0d31,0x0d81,0x0df1,0x0de1,0x0e11,0x0e21,0x0ef1,0x0ee1,0x0f11,0x0f21,
0x0dc1,0x1011,0x1071,0x1061,0x1081,0x10b1,0x1091,0x0a30,0x10d1,0x1211,0x1201,0x1221,0x1231,0x12c1,0x11d1,0x1190,
0x0cb0,0x0ca0,0x0cc0,0x0d00,0x0cd0,0x0d10,0x0d30,0x0d80,0x0df0,0x0de0,0x0e10,0x0e20,0x0ef0,0x0ee0,0x0f10,0x0f20,
0x0dc0,0x1010,0x1070,0x1060,0x1080,0x10b0,0x1090,0x0a40,0x10d0,0x1210,0x1200,0x1220,0x1230,0x12c0,0x11d0,0x12d0,
0x0100,0x0cf1,0x0101,0x0cf0,0x0102,0x0ce1,0x0103,0x0ce0,0x0104,0x0d21,0x0105,0x0d20,0x0106,0x0d61,0x0107,0x0d60,
0x010c,0x0d71,0x010d,0x0d70,0x010e,0x0da1,0x010f,0x0da0,0x0110,0x0db1,0x0111,0x0db0,0x0112,0x0e41,0x0113,0x0e40,
0x0116,0x0e01,0x0117,0x0e00,0x0118,0x0e51,0x0119,0x0e50,0x011a,0x0e31,0x011b,0x0e30,0x011e,0x0e91,0x011f,0x0e90,
0x0122,0x0ea1,0x0123,0x0ea0,0x012a,0x0f31,0x012b,0x0f30,0x012e,0x0f41,0x012f,0x0f40,0x0130,0x0f00,0x0131,0x0ed0,
0x0136,0x0f71,0x0137,0x0f70,0x0139,0x0f91,0x013a,0x0f90,0x013b,0x0fb1,0x013c,0x0fb0,0x013d,0x0fa1,0x013e,0x0fa0,
0x0141,0x0fc1,0x0142,0x0fc0,0x0143,0x0ff1,0x0144,0x0ff0,0x0145,0x1021,0x0146,0x1020,0x0147,0x1001,0x0148,0x1000,
0x014c,0x10a1,0x014d,0x10a0,0x0150,0x10c1,0x0151,0x10c0,0x0152,0x10e1,0x0153,0x10e0,0x0154,0x1121,0x0155,0x1120,
0x0156,0x1141,0x0157,0x1140,0x0158,0x1131,0x0159,0x1130,0x015a,0x1161,0x015b,0x1160,0x015e,0x1181,0x015f,0x1180,
0x0160,0x1171,0x0161,0x1170,0x0162,0x11c1,0x0163,0x11c0,0x0164,0x11b1,0x0165,0x11b0,0x016a,0x1241,0x016b,0x1240,
0x016e,0x1251,0x016f,0x1250,0x0170,0x1271,0x0171,0x1270,0x0172,0x1261,0x0173,0x1260,0x0178,0x12d1,0x0179,0x12f1,
0x017a,0x12f0,0x017b,0x1301,0x017c,0x1300,0x017d,0x1311,0x017e,0x1310,0x0192,0x0e70,0x02c6,0x0770,0x02c7,0x0850,
0x02d8,0x0860,0x02d9,0x0870,0x02db,0x0880,0x02dc,0x0890,0x02dd,0x08a0,0x0384,0x08b0,0x0385,0x08c0,0x0386,0x1331,
0x0388,0x1381,0x0389,0x13b1,0x038a,0x13e1,0x038c,0x1471,0x038e,0x14d1,0x038f,0x1541,0x0390,0x1400,0x0391,0x1321,
0x0392,0x1341,0x0393,0x1351,0x0394,0x1361,0x0395,0x1371,0x0396,0x1391,0x0397,0x13a1,0x0398,0x13c1,0x0399,0x13d1,
0x039a,0x1411,0x039b,0x1421,0x039c,0x1431,0x039d,0x1441,0x039e,0x1451,0x039f,0x1461,0x03a0,0x1481,0x03a1,0x1491,
0x03a3,0x14a2,0x03a4,0x14b1,0x03a5,0x14c1,0x03a6,0x1501,0x03a7,0x1511,0x03a8,0x1521,0x03a9,0x1531,0x03aa,0x13f1,
0x03ab,0x14e1,0x03ac,0x1330,0x03ad,0x1380,0x03ae,0x13b0,0x03af,0x13e0,0x03b0,0x14f0,0x03b1,0x1320,0x03b2,0x1340,
0x03b3,0x1350,0x03b4,0x1360,0x03b5,0x1370,0x03b6,0x1390,0x03b7,0x13a0,0x03b8,0x13c0,0x03b9,0x13d0,0x03ba,0x1410,
0x03bb,0x1420,0x03bc,0x1430,0x03bd,0x1440,0x03be,0x1450,0x03bf,0x1460,0x03c0,0x1480,0x03c1,0x1490,0x03c2,0x14a1,
0x03c3,0x14a0,0x03c4,0x14b0,0x03c5,0x14c0,0x03c6,0x1500,0x03c7,0x1510,0x03c8,0x1520,0x03c9,0x1530,0x03ca,0x13f0,
0x03cb,0x14e0,0x03cc,0x1470,0x03cd,0x14d0,0x03ce,0x1540,0x0401,0x15e1,0x0402,0x15c1,0x0403,0x15a1,0x0404,0x15f1,
0x0405,0x1621,0x0406,0x1641,0x0407,0x1651,0x0408,0x1671,0x0409,0x16b1,0x040a,0x16e1,0x040b,0x1741,0x040c,0x1691,
0x040e,0x1761,0x040f,0x17b1,0x0410,0x1551,0x0411,0x1561,0x0412,0x1571,0x0413,0x1581,0x0414,0x15b1,0x0415,0x15d1,
0x0416,0x1601,0x0417,0x1611,0x0418,0x1631,0x0419,0x1661,0x041a,0x1681,0x041b,0x16a1,0x041c,0x16c1,0x041d,0x16d1,
0x041e,0x16f1,0x041f,0x1701,0x0420,0x1711,0x0421,0x1721,0x0422,0x1731,0x0423,0x1751,0x0424,0x1771,0x0425,0x1781,
0x0426,0x1791,0x0427,0x17a1,0x0428,0x17c1,0x0429,0x17d1,0x042a,0x17e1,0x042b,0x17f1,0x042c,0x1801,0x042d,0x1811,
0x042e,0x1821,0x042f,0x1831,0x0430,0x1550,0x0431,0x1560,0x0432,0x1570,0x0433,0x1580,0x0434,0x15b0,0x0435,0x15d0,
0x0436,0x1600,0x0437,0x1610,0x0438,0x1630,0x0439,0x1660,0x043a,0x1680,0x043b,0x16a0,0x043c,0x16c0,0x043d,0x16d0,
0x043e,0x16f0,0x043f,0x1700,0x0440,0x1710,0x0441,0x1720,0x0442,0x1730,0x0443,0x1750,0x0444,0x1770,0x0445,0x1780,
0x0446,0x1790,0x0447,0x17a0,0x0448,0x17c0,0x0449,0x17d0,0x044a,0x17e0,0x044b,0x17f0,0x044c,0x1800,0x044d,0x1810,
0x044e,0x1820,0x044f,0x1830,0x0451,0x15e0,0x0452,0x15c0,0x0453,0x15a0,0x0454,0x15f0,0x0455,0x1620,0x0456,0x1640,
0x0457,0x1650,0x0458,0x1670,0x0459,0x16b0,0x045a,0x16e0,0x045b,0x1740,0x045c,0x1690,0x045e,0x1760,0x045f,0x17b0,
0x0490,0x1591,0x0491,0x1590,0x05b0,0x04b0,0x05b1,0x04c0,0x05b2,0x04d0,0x05b3,0x04e0,0x05b4,0x04f0,0x05b5,0x0500,
0x05b6,0x0510,0x05b7,0x0520,0x05b8,0x0530,0x05b9,0x0540,0x05ba,0x0010,0x05bb,0x0550,0x05bc,0x0560,0x05bd,0x0570,
0x05be,0x08d0,0x05bf,0x0580,0x05c0,0x0590,0x05c1,0x05a0,0x05c2,0x05b0,0x05c3,0x08e0,0x05d0,0x1840,0x05d1,0x1850,
0x05d2,0x1860,0x05d3,0x1870,0x05d4,0x1880,0x05d5,0x1890,0x05d6,0x18c0,0x05d7,0x18d0,0x05d8,0x18e0,0x05d9,0x18f0,
0x05da,0x1910,0x05db,0x1911,0x05dc,0x1920,0x05dd,0x1930,0x05de,0x1931,0x05df,0x1940,0x05e0,0x1941,0x05e1,0x1950,
0x05e2,0x1960,0x05e3,0x1970,0x05e4,0x1971,0x05e5,0x1980,0x05e6,0x1981,0x05e7,0x1990,0x05e8,0x19a0,0x05e9,0x19b0,
0x05ea,0x19c0,0x05f0,0x18a0,0x05f1,0x18b0,0x05f2,0x1900,0x05f3,0x08f0,0x05f4,0x0900,0x060c,0x0910,0x061b,0x0920,
0x061f,0x0930,0x0621,0x19d0,0x0622,0x1a10,0x0623,0x1a20,0x0624,0x19e0,0x0625,0x1a30,0x0626,0x19f0,0x0627,0x1a00,
0x0628,0x1a40,0x0629,0x1a60,0x062a,0x1a61,0x062b,0x1a70,0x062c,0x1a80,0x062d,0x1aa0,0x062e,0x1ab0,0x062f,0x1ac0,
0x0630,0x1ad0,0x0631,0x1ae0,0x0632,0x1af0,0x0633,0x1b10,0x0634,0x1b20,0x0635,0x1b30,0x0636,0x1b40,0x0637,0x1b50,
0x0638,0x1b60,0x0639,0x1b70,0x063a,0x1b80,0x0640,0x0015,0x0641,0x1b90,0x0642,0x1ba0,0x0643,0x1bb0,0x0644,0x1be0,
0x0645,0x1bf0,0x0646,0x1c00,0x0647,0x1c10,0x0648,0x1c20,0x0649,0x1c30,0x064a,0x1c40,0x064b,0x0430,0x064c,0x0440,
0x064d,0x0450,0x064e,0x0460,0x064f,0x0470,0x0650,0x0480,0x0651,0x04a0,0x0652,0x0490,0x0679,0x1c50,0x067e,0x1a50,
0x0686,0x1a90,0x0688,0x1c60,0x0691,0x1c70,0x0698,0x1b00,0x06a9,0x1bc0,0x06af,0x1bd0,0x06ba,0x1c80,0x06be,0x1c90,
0x06c1,0x1ca0,0x06d2,0x1cb0,0x200c,0x0013,0x200d,0x0014,0x200e,0x0011,0x200f,0x0012,0x2013,0x0400,0x2014,0x0410,
0x2015,0x0420,0x2018,0x0940,0x2019,0x0950,0x201a,0x0960,0x201c,0x0970,0x201d,0x0980,0x201e,0x0990,0x2020,0x0b10,
0x2021,0x0b20,0x2022,0x0b30,0x2026,0x0b40,0x2030,0x0b50,0x2039,0x09a0,0x203a,0x09b0,0x20aa,0x0b60,0x20ac,0x0b70,
0x2116,0x1030,0x2122,0x11e0,0xf88d,0x1cc0,0xf88e,0x1cd0,0xf88f,0x1ce0,0xf890,0x1cf0,0xf891,0x1d00,0xf892,0x1d10,
0xf893,0x1d20,0xf894,0x1d30,0xf895,0x1d40,0xf896,0x1d50,0xf8f9,0x1d60,0xf8fa,0x1d70,0xf8fb,0x1d80,0xf8fc,0x1d90,
0xf8fd,0x1da0
};

static NOINLINE uint_fast32_t sortvalue(const tchar_t** InPtr)
{
    uint_fast32_t ch;
    const uint8_t* In=(const uint8_t*)*InPtr;

    if ((In[0]&0x80)==0) // most of the time
    {
        ++(*InPtr);
        return utf_sort_en[In[0]];
    }

    if ((In[0]&0xe0)==0xc0 && (In[1]&0xc0)==0x80)
    {
        ch=((In[0]&0x1f)<<6)+(In[1]&0x3f);
        In+=2;
    }
    else
    if ((In[0]&0xf0)==0xe0 && (In[1]&0xc0)==0x80 && (In[2]&0xc0)==0x80)
    {
        ch=((In[0]&0x0f)<<12)+((In[1]&0x3f)<<6)+(In[2]&0x3f);
        In+=3;
    }
    else
    if ((In[0]&0xf8)==0xf0 && (In[1]&0xc0)==0x80 && (In[2]&0xc0)==0x80 && (In[3]&0xc0)==0x80)
    {
        ch=((In[0]&0x07)<<18)+((In[1]&0x3f)<<12)+((In[2]&0x3f)<<6)+(In[3]&0x3f);
        In+=4;
    }
    else
    {
        ch=In[0];
        ++In;
    }
    *InPtr=(const tchar_t*)In;

    if (ch<0x100)
        return utf_sort_en[ch];
    else
    {
	    intptr_t Mid;
	    intptr_t Lower = 128;
	    intptr_t Upper = sizeof(utf_sort_en)/(sizeof(uint16_t)*2)-1;
	    while (Upper >= Lower) 
	    {
		    Mid = (Upper + Lower) >> 1;
		    if (utf_sort_en[Mid*2]>ch)
			    Upper = Mid-1;	
		    else if (utf_sort_en[Mid*2]<ch)  		
			    Lower = Mid+1;	
		    else 
                return utf_sort_en[Mid*2+1];
        }
        return 1;
    }
}

int tcscmp(const tchar_t* a,const tchar_t* b) 
{
    uint_fast32_t va,vb;
    do
    {
        va=sortvalue(&a);
        vb=sortvalue(&b);
        if (va!=vb)
            return (va>vb)?1:-1;
    } while (va);
    return 0;
}

int tcsncmp(const tchar_t* a,const tchar_t* b,size_t n) 
{
    uint_fast32_t va,vb;
    do
    {
        if (!n) break;
        --n;
        va=sortvalue(&a);
        vb=sortvalue(&b);
        if (va!=vb)
            return (va>vb)?1:-1;
    } while (va);
    return 0;
}

int tcsicmp(const tchar_t* a,const tchar_t* b) 
{
    uint_fast32_t va,vb;
    do
    {
        va=sortvalue(&a) & ~15;
        vb=sortvalue(&b) & ~15;
        if (va!=vb)
            return (va>vb)?1:-1;
    } while (va);
    return 0;
}

int tcsnicmp(const tchar_t* a,const tchar_t* b,size_t n) 
{
    uint_fast32_t va,vb;
    do
    {
        if (!n) break;
        --n;
        va=sortvalue(&a) & ~15;
        vb=sortvalue(&b) & ~15;
        if (va!=vb)
            return (va>vb)?1:-1;
    } while (va);
    return 0;
}

tchar_t* TcsToUpper(tchar_t* Out,size_t OutLen,const tchar_t* In)
{
    if (OutLen)
    {
        tcscpy_s(Out,OutLen,In);
        tcsupr(Out);  //todo: doesn't support multibyte
    }
    return Out;
}

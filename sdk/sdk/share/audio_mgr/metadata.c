
#include "audio_mgr.h"
#include <malloc.h>

//#define METADATA_DEBUG

#define SMTK_FONT_SUPPORT_ENCODING_CONVERTER

#define PalHeapAlloc(a,size)                    malloc(size)
#define PalHeapFree(a, size)                    free(size)

#define bswap_16(x) (x)//((((x) << 8) & 0xff00) | (((x) >> 8) & 0x00ff))
#define bswap_32(x) (x)/* \
              ((((x) >> 24) & 0x000000ff) | (((x) >> 8 ) & 0x0000ff00) | \
               (((x) << 8 ) & 0x00ff0000) | (((x) << 24) & 0xff000000))*/
               
// get wma info
#define MASK   0xC0 /* 11000000 */
#define COMP   0x80 /* 10x      */

extern SMTK_AUDIO_MGR  audioMgr;
static SMTK_AUDIO_TAG_INFO  gtTag;


static SMTK_WMAInfo wmaInfo;

static MMP_UINT8      utf8buf[512];



static const MMP_UINT8 asf_guid_header[16] =
{ 0x30,0x26,0xB2,0x75,0x8E, 0x66, 0xCF,0x11 ,0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };

static const MMP_UINT8 asf_guid_file_properties[16]=
{ 0xa1,0xdc,0xab,0x8c , 0x47,0xa9, 0xcf,0x11, 0x8E, 0xe4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 };

static const MMP_UINT8 asf_guid_stream_properties[16] =
{0x91,0x07,0xDC,0xB7  , 0xB7,0xA9, 0xCF,0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 };

static const MMP_UINT8 asf_guid_content_description[16] =
{0x33,0x26,0xB2,0x75  , 0x8E,0x66, 0xCF,0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };

static const MMP_UINT8 asf_guid_extended_content_description[16] =
{0x40,0xA4,0xD0,0xD2  , 0x07,0xE3, 0xD2,0x11 ,  0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 };

static const MMP_UINT8 asf_guid_content_encryption[16] =
{0xfb,0xb3,0x11,0x22  , 0x23,0xbd, 0xd2,0x11, 0xb4, 0xb7, 0x00, 0xa0, 0xc9, 0x55, 0xfc, 0x6e };

static const MMP_UINT8 asf_guid_extended_content_encryption[16] =
{0x14,0xe6,0x8a,0x29  , 0x22,0x26, 0x17,0x4c, 0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c  };


/*
static const guid_t asf_guid_header =
    { 0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C} };

static const guid_t asf_guid_file_properties = { 0x8cabdca1, 0xa947, 0x11cf, {0x8E, 0xe4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65} };

static const guid_t asf_guid_stream_properties =
{0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_content_description =
{0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_extended_content_description =
{0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}};

static const guid_t asf_guid_content_encryption =
{0x2211b3fb, 0xbd23, 0x11d2, {0xb4, 0xb7, 0x00, 0xa0, 0xc9, 0x55, 0xfc, 0x6e}};

static const guid_t asf_guid_extended_content_encryption =
{0x298ae614, 0x2622, 0x4c17, {0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c}};

static const guid_t asf_guid_stream_type_audio =
{0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}};
*/


static unsigned int
PalWcslen(
    const MMP_WCHAR* s)
{
    if (s==0){
        printf("[Audio mgr] PalWcslen null pointer %d \n",__LINE__);
        return 0;
    }
    
    return wcslen(s);
}

static unsigned int
PalMemcmp(
    const void* s1,
    const void* s2,
    unsigned int n)
{
    return memcmp(s1, s2, n);
}
#if 0
static void*
PalMemcpy(
    void* dest,
    const void* src,
    unsigned int count)
{
    return memcpy(dest, src, count);
}
#endif

static const unsigned char utf8comp[6] =
{
    0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

//#if 1

int UCS4ToUtf8 (unsigned char *s, unsigned int uc, int n)
{
    int count;
    int len=0;
    unsigned char c[6];

    if (uc < 0x80)
        count = 1;
    else if (uc < 0x800)
        count = 2;
    else if (uc < 0x10000)
        count = 3;
#if 0
    else if (uc < 0x200000)
    count = 4;
    else if (uc < 0x4000000)
    count = 5;
    else if (uc <= 0x7fffffff)
    count = 6;
#else
    else if (uc < 0x110000)
        count = 4;
#endif
    else
        return -1;

    if (n < count)
        return -2;

    switch (count) /* note: code falls through cases! */
    {
#if 0
    case 6: s[5] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x4000000;
    case 5: s[4] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x200000;
#endif
    case 4: s[3] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x10000;
    case 3: s[2] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0x800;
    case 2: s[1] = 0x80 | (uc & 0x3f); uc = uc >> 6; uc |= 0xc0;
    case 1: s[0] = uc;
    }
    
    return count;
}


int UnicodeToUtf8_char(unsigned long uni,char utf8[8]){
    int len=0;
    unsigned char c[6];

    //c1=c2=c3=c4=c5=c6=0;
    if(uni < 0x80 ){   // is a ansi code
        c[0]=(char)uni;
        len=1;
    }else if (uni >= 0x80 && uni <0x800){
        c[1]= 0x80 | (0x3f & uni);
        c[0]= 0xc0 | (uni>>=6);
        len=2;
    }else if(uni >= 0x800 && uni < 0x10000l){
        c[2]= 0x80 | (0x3f & uni);
        c[1]= 0x80 | (0x3f & (uni>>=6));
        c[0]= 0xe0 | (uni>>=6);
        len=3;
    }else if(uni >=0x10000l && uni<0x200000l){
        c[3]= 0x80 | (0x3f & uni);
        c[2]= 0x80 | (0x3f & (uni>>=6));
        c[1]= 0x80 | (0x3f & (uni>>=6));
        c[0]= 0xf0 | (uni>>=6);
        len=4;
    }else if(uni >=0x200000l && uni<0x4000000l){
        c[4]= 0x80 | (0x3f & uni);
        c[3]= 0x80 | (0x3f & (uni>>=6));
        c[2]= 0x80 | (0x3f & (uni>>=6));
        c[1]= 0x80 | (0x3f & (uni>>=6));
        c[0]= 0xf8 | (uni>>=6);
        len=5;
    }else if(uni >=0x4000000l && uni <= 0x7FFFFFFFl){
        c[5]= 0x80 | (0x3f & uni);
        c[4]= 0x80 | (0x3f & (uni>>=6));
        c[3]= 0x80 | (0x3f & (uni>>=6));
        c[2]= 0x80 | (0x3f & (uni>>=6));
        c[1]= 0x80 | (0x3f & (uni>>=6));
        c[0]= 0xfc | (uni>>=6);
        len=6;
    }

    c[len]=0;
    strcpy(utf8,(char*)c);
    return len;
}

void Unicode32ToUtf8(unsigned int *unistr,long slen,char *utf8,long utf8bufsize){
    unsigned short w;
    long count;
    char autf8[8];
    char *p;
    int len;

    utf8[0]=0;
    p=utf8;
    count=0;

    while(w=*unistr++) {
        if(slen != -1 && ++count> slen){
            break;
        }

        //len=UnicodeToUtf8_char(w,autf8);
        len = UCS4ToUtf8(autf8,w,slen);

//        printf(" w %x len %d \n",w,len);        
        
        if(p+len-utf8 >= utf8bufsize ) {
            break;
        }
        //strcpy(p,autf8);
        memcpy(p,autf8,len);
        p+=len;
    }
}


//#else
/**
 * This file implement functions of:
 *
 * 1. UTF-16 character to UTF-8 chaaracter converting.
 * 2. UTF-8 character to UTF-16 character converting.
 *
 * 3. UTF-16 string to UTF-8 string converting.
 * 4. UTF-8 string to UTF-16 string converting.
 */
/* Maximum bytes of a utf-8 character */
#define MAX_CHARACTER_SIZE    8
/**
 * UnicodeToUTF8 - convert unicode char to UTF-8 char
 * @unicode: a UNICODE(utf-16) character
 * @p: a buffer to contain a utf-8 characters
 *
 * @return: One step over the end of the utf-8 character buffer
 */
unsigned char * UnicodeToUTF8( int unicode, unsigned char *p)
{
    unsigned char *e = NULL;
    if((e = p))
    {
        if(unicode < 0x80)
        {
            *e++ = unicode;
        }
        else if(unicode < 0x800)
        {
            /* <11011111> < 000 0000 0000> */
            *e++ = ((unicode >> 6) & 0x1f)|0xc0;
            *e++ = (unicode & 0x3f)|0x80; 
        }
        else if(unicode < 0x10000)
        {
            /* <11101111> <0000 0000 0000 0000> */
            *e++ = ((unicode >> 12) & 0x0f)|0xe0; 
            *e++ = ((unicode >> 6) & 0x3f)|0x80;
            *e++ = (unicode & 0x3f)|0x80; 
        }
        else if(unicode < 0x200000)
        {
            /* <11110111> <0 0000 0000 0000 0000 0000> */
            *e++ = ((unicode >> 18) & 0x07)|0xf0; 
            *e++ = ((unicode >> 12) & 0x3f)|0x80;
            *e++ = ((unicode >> 6) & 0x3f)|0x80;
            *e++ = (unicode & 0x3f)|0x80; 
        }
        else if(unicode < 0x4000000)
        {
            /* <11111011> <00 0000 0000 0000 0000 0000 0000> */
            *e++ = ((unicode >> 24) & 0x03)|0xf8 ; 
            *e++ = ((unicode >> 18) & 0x3f)|0x80;
            *e++ = ((unicode >> 12) & 0x3f)|0x80;
            *e++ = ((unicode >> 6) & 0x3f)|0x80;
            *e++ = (unicode & 0x3f)|0x80; 
        }
        else
        {
            /* <11111101> <0000 0000 0000 0000 0000 0000 0000 0000> */
            *e++ = ((unicode >> 30) & 0x01)|0xfc; 
            *e++ = ((unicode >> 24) & 0x3f)|0x80;
            *e++ = ((unicode >> 18) & 0x3f)|0x80;
            *e++ = ((unicode >> 12) & 0x3f)|0x80;
            *e++ = ((unicode >> 6) & 0x3f)|0x80;
            *e++ = (unicode & 0x3f)|0x80; 
        }
    }
	/* Return One step over the end of the utf-8 character buffer */
    return e;
}
/**
 * UTF8ToUnicode - convert UTF-8 char to unicode char
 * @ch: A buffer contain a utf-8 character
 * @unicode: Contain the converted utf-16 character
 *
 * @return: Bytes count of the utf-8 character (1 ~ 6),
 *          can be used to step to next utf-8 character when convert a utf-8 string to a utf-16 string
 */
int UTF8ToUnicode (unsigned char *ch, int *unicode)
{
    unsigned char *p = NULL;
    int e = 0, n = 0;
    if((p = ch) && unicode)
    {
        if(*p >= 0xfc)
        {
            /* 6:<11111100> */
            e = (p[0] & 0x01) << 30;
            e |= (p[1] & 0x3f) << 24;
            e |= (p[2] & 0x3f) << 18;
            e |= (p[3] & 0x3f) << 12;
            e |= (p[4] & 0x3f) << 6;
            e |= (p[5] & 0x3f);
            n = 6;
        }
        else if(*p >= 0xf8) 
        {
            /* 5:<11111000> */
            e = (p[0] & 0x03) << 24;
            e |= (p[1] & 0x3f) << 18;
            e |= (p[2] & 0x3f) << 12;
            e |= (p[3] & 0x3f) << 6;
            e |= (p[4] & 0x3f);
            n = 5;
        }
        else if(*p >= 0xf0)
        {
            /* 4:<11110000> */
            e = (p[0] & 0x07) << 18;
            e |= (p[1] & 0x3f) << 12;
            e |= (p[2] & 0x3f) << 6;
            e |= (p[3] & 0x3f);
            n = 4;
        }
        else if(*p >= 0xe0)
        {
            /* 3:<11100000> */
            e = (p[0] & 0x0f) << 12;
            e |= (p[1] & 0x3f) << 6;
            e |= (p[2] & 0x3f);
            n = 3;
        }
        else if(*p >= 0xc0) 
        {
            /* 2:<11000000> */
            e = (p[0] & 0x1f) << 6;
            e |= (p[1] & 0x3f);
            n = 2;
        }
        else 
        {
            e = p[0];
            n = 1;
        }
        *unicode = e;
    }
	/* Return bytes count of this utf-8 character */
    return n;
}
/**
 * UnicodeStrToUTF8Str - Convert a utf-16 string to a utf-8 string
 * @unicde_str: A utf-16 string
 * @utf8_str: A buffer to contain utf-8 string
 * @utf8_str_size: Maximum size of the utf-8 string buffer
 *
 * @return: One step over the end of the last utf-8 character
 */
unsigned char * UnicodeStrToUTF8Str (unsigned short * unicode_str,
							unsigned char * utf8_str, int utf8_str_size)
{
    int unicode = 0;
    unsigned char *e = NULL, *s = NULL;
    unsigned char utf8_ch[MAX_CHARACTER_SIZE]; 
    s = utf8_str;
    if ((unicode_str) && (s))
    {
        while ((unicode = (int) (*unicode_str++)))
        {
            memset (utf8_ch, 0, sizeof (utf8_ch));

            if ((e = UnicodeToUTF8 (unicode, utf8_ch)) > utf8_ch)
            {
                //*e = '/0';

                /* Judge whether exceed the destination buffer */
                if ((s - utf8_str + strlen ((const char *) utf8_ch)) >= utf8_str_size)
                {
                    printf(" > %d  \n",utf8_str_size);
                    return s;
                }
                else
                {
                    printf("%s %d,\n",utf8_ch,strlen ((const char *) utf8_ch));
                    memcpy (s, utf8_ch, strlen ((const char *) utf8_ch));
                    s += strlen ((const char *) utf8_ch);
                    //*s = '/0';
                }
            }
            else
            {
                /* Converting error occurs */

                printf(" Converting error occurs \n");
                return s;
            }
        }
    }
    printf(" ok \n");
    return s;
}
/**
 * UTF8StrToUnicodeStr - Convert a utf-8 stirng to a utf-16 string
 * @utf8_str: A utf-8 string
 * @unicode_str: A buffer to contain utf-16 string
 * @unicode_str_size: Maximum size of the utf-16 string buffer
 *
 * @return: Number of utf-16 character
 */
int UTF8StrToUnicodeStr (unsigned char * utf8_str,
					unsigned short * unicode_str, int unicode_str_size)
{
	int unicode = 0;
	int n = 0;
	int count = 0;
	unsigned char *s = NULL;
	unsigned short *e = NULL;
	
	s = utf8_str;
	e = unicode_str;
	
	if ((utf8_str) && (unicode_str))
	{
		while (*s)
		{
			if ((n = UTF8ToUnicode (s, &unicode)) > 0)
			{
				if ((count + 1) >= unicode_str_size)
				{
					return count;
				}
				else
				{
					*e = (unsigned short) unicode;
					e++;
					*e = 0;
					
					/* Step to next utf-8 character */
					s += n;
				}
			}
			else
			{
				/* Converting error occurs */
				return count;
			}
		}
	}
	
	return count;
}
//#endif
/* Encode a UCS value as UTF-8 and return a pointer after this UTF-8 char. */
static unsigned char* utf8encode(unsigned long ucs, unsigned char *utf8)
{
    int tail = 0;

    if (ucs > 0x7F)
        while (ucs >> (5*tail + 6))
            tail++;

    *utf8++ = (ucs >> (6*tail)) | utf8comp[tail];
    while (tail--)
        *utf8++ = ((ucs >> (6*tail)) & (MASK ^ 0xFF)) | COMP;

    return utf8;
}


/* Decode a LE utf16 string from a disk buffer into a fixed-sized
   utf8 buffer.
*/

static void audio_asf_utf16LEdecode(unsigned short utf16bytes,
                              unsigned char* utf8,
                              int* utf8bytes
                             )
{
    unsigned long ucs;
    int n;
    unsigned char utf16buf[512];
    unsigned char* utf16 = utf16buf;
    unsigned char* newutf8;

   if(utf16bytes>512)
    {
        LOG_DEBUG
            "audio_asf_utf16LEdecode length %d > 512 ,#line %d \n",utf16bytes,__LINE__
        LOG_END
        return;
    }
    memcpy(utf16buf, utf8, utf16bytes);
    n = utf16bytes ;
    utf16bytes -= n;

    while (n > 0)
    {
        /* Check for a surrogate pair */
        if (utf16[1] >= 0xD8 && utf16[1] < 0xE0) 
        {
            if (n < 4)
            {
                /* Run out of utf16 bytes, read some more */
                /*
                utf16buf[0] = utf16[0];
                utf16buf[1] = utf16[1];

                n = read_filebuf(utf16buf + 2, sizeof(char)*MIN(sizeof(utf16buf)-2, utf16bytes));
                utf16 = utf16buf;
                utf16bytes -= n;
                n += 2;
                */
            }
            if (n<4) 
            {
                /* Truncated utf16 string, abort */
                break;
            }
            ucs = 0x10000 + ((utf16[0] << 10) | ((utf16[1] - 0xD8) << 18)
                             | utf16[2] | ((utf16[3] - 0xDC) << 8));
            utf16 += 4;
            n -= 4;
        }
        else
        {
            ucs = (utf16[0] | (utf16[1] << 8));
            utf16 += 2;
            n -= 2;
        }

        if (*utf8bytes > 6)
        {
            newutf8 = utf8encode(ucs, utf8);
            *utf8bytes -= (newutf8 - utf8);
            utf8 += (newutf8 - utf8);
        }

        /* We have run out of utf16 bytes, read more if available */
        if ((n == 0) && (utf16bytes > 0))
        {
        /*
            n = read_filebuf(utf16buf, sizeof(char)*MIN(sizeof(utf16buf), utf16bytes));
            utf16 = utf16buf;
            utf16bytes -= n;
            */
        }
    }

    //*utf8[0] = 0;
    --*utf8bytes;

    if (utf16bytes > 0) 
    {
     /* Skip any remaining bytes */
     //    advance_buffer(utf16bytes);
    }
    return;
}
///////////////////////////////////////////////////////
static __inline MMP_UINT64 ByteSwap64(MMP_INT64 x)
{
    union
    {
        MMP_UINT64 ll;
        MMP_UINT32 l[2];
    } w, r;

    w.ll = x;
    r.l[0] = (MMP_UINT32) bswap_32(w.l[1]);
    r.l[1] = (MMP_UINT32) bswap_32(w.l[0]);
    return r.ll;
}
//////////////////////////////////////////////////////////////////////
/*  mp3                                                                                                            */
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////porting parsing id3

static MMP_BOOL
audioConvertIsoToUcs4A(
    MMP_WCHAR* strUcs,
    MMP_UINT8* strIso,
    MMP_UINT32 strLength)
{
    MMP_UINT32 index = 0;
    // PRECONDITION    
    if ( strUcs == MMP_NULL )
    {
        LOG_ERROR
            "Input wchar string is null #line %d \n",__LINE__  
        LOG_END
        return MMP_FALSE;
    }   
    if ( strIso == MMP_FALSE )
    {
        LOG_ERROR
            " Input char string is null #line %d \n",__LINE__  
        LOG_END
        return MMP_FALSE;
    }

    if ( strLength == 0 )
    {
        LOG_ERROR
            " Input string length is 0 #line %d \n",__LINE__  
        LOG_END
        return MMP_FALSE;
    }
    for ( index = 0; index < strLength; index++ )
    {
        strUcs[index] = strIso[index];
    }
    strUcs[strLength - 1] = 0;

    return MMP_TRUE;
}

SMTK_AUDIO_ID3_UTF16_BYTEORDER
audioGetUnicodeByteOrder(
    MMP_UINT8 byte1,
    MMP_UINT8 byte2)
{
    SMTK_AUDIO_ID3_UTF16_BYTEORDER byteOrder = SMTK_AUDIO_ID3_UTF16_BYTEORDER_ERROR;    
    switch( (((MMP_UINT16)byte1) << 8) | byte2 )
    {
        case 0xFEFF:
            byteOrder = SMTK_AUDIO_ID3_UTF16_BYTEORDER_BE;
            break;
        case 0xFFFE:
            byteOrder = SMTK_AUDIO_ID3_UTF16_BYTEORDER_LE;
            break;
        default:
            //dbg_msg(DBG_MSG_TYPE_INFO, "%s[%d]: Unknow byte order, value = 0x%4X\n", __FILE__, __LINE__, (((MMP_UINT16)byte1) << 8) | byte2);
            break;
    }
    return byteOrder;
}

MMP_INT
audioConvertUtf16BeToUcs4(
    MMP_WCHAR* pUcsString,
    MMP_UINT8* strUtf16Be,
    MMP_UINT32 strLength)
{
    MMP_UINT32 byteIndex = 0;
    MMP_UINT32 wordIndex = 0;
    MMP_UINT16 word = 0;
    // PRECONDITION    
    if (strUtf16Be == MMP_NULL)
    {
        LOG_ERROR
            "Input string is null #line %d \n",__LINE__  
        LOG_END   
        return MMP_NULL;
    }
    if (strLength == 0)
    {
        LOG_ERROR
            "Input strLength is 0 #line %d \n",__LINE__  
        LOG_END   
        return MMP_NULL;
    }

    memset(pUcsString, 0x00, (strLength/2 + 1) * sizeof(MMP_WCHAR));   
    // --- Swap bytes   
    for ( byteIndex = 0; byteIndex < strLength; byteIndex += 2 )
    {
        word = strUtf16Be[byteIndex + 1] | (((MMP_UINT16)strUtf16Be[byteIndex]) << 8);
        pUcsString[wordIndex] = word;
        wordIndex++;
    }       
    return MMP_TRUE;
}


MMP_INT
audioConvertUtf16LeToUcs4(
    MMP_WCHAR* pUcsString,
    MMP_UINT8* strUtf16Le,
    MMP_UINT32 strLength)
{
    MMP_UINT32 byteIndex = 0;
    MMP_UINT32 wordIndex = 0;
    MMP_UINT16 word      = 0;

    MMP_UINT8 nTemp;
    // PRECONDITION    
    if ( strUtf16Le == MMP_NULL )
    {
        LOG_ERROR
            "Input string is null #line %d \n",__LINE__  
        LOG_END      
        return MMP_NULL;
    }

    if ( strLength == 0 )
    {
        LOG_ERROR
            "Input strLength is null #line %d \n",__LINE__  
        LOG_END          
        return MMP_NULL;
    }

    memset(pUcsString, 0x00, (strLength/2 + 1) * sizeof(MMP_WCHAR));    
    // --- Swap bytes   
    for ( byteIndex = 0; byteIndex < strLength; byteIndex += 2 )
    {
        word = (((MMP_UINT16)strUtf16Le[byteIndex + 1]) << 8) | strUtf16Le[byteIndex];
        pUcsString[wordIndex] = word;
        wordIndex++;
        nTemp = strUtf16Le[byteIndex];
        strUtf16Le[byteIndex] = strUtf16Le[byteIndex+1];
        strUtf16Le[byteIndex+1] = nTemp;
    }        
    return MMP_TRUE;
}


MMP_INT
audioConvertUnicodeToUcs4(
    MMP_WCHAR* pString,
    MMP_UINT8* pBuffer,
    MMP_UINT32 bufferLength)
{
    MMP_UINT8 unicodeBom[2];
    SMTK_AUDIO_ID3_UTF16_BYTEORDER byteOrder = SMTK_AUDIO_ID3_UTF16_BYTEORDER_ERROR;
    // PRECONDITION
    if ( pBuffer == MMP_NULL )
    {
        LOG_ERROR
            "Input string is null #line %d \n",__LINE__  
        LOG_END          
        return MMP_NULL;
    }

    if ( bufferLength == 0 )
    {
        LOG_ERROR
            "Input bufferLength is null #line %d \n",__LINE__  
        LOG_END          
        return MMP_NULL;
    }
    // Main
    unicodeBom[0] = pBuffer[0];
    unicodeBom[1] = pBuffer[1];
    byteOrder = audioGetUnicodeByteOrder(unicodeBom[0], unicodeBom[1]);
    switch( byteOrder )
    {
        case SMTK_AUDIO_ID3_UTF16_BYTEORDER_BE:
            //dbg_msg(DBG_MSG_TYPE_INFO, "id3 v2 ConvertUnicodeToUcs4 SMTK_AUDIO_ID3_UTF16_BYTEORDER_BE \n");
            audioConvertUtf16BeToUcs4(pString,&pBuffer[2], bufferLength - 2);
            break;
        case SMTK_AUDIO_ID3_UTF16_BYTEORDER_LE:
            //dbg_msg(DBG_MSG_TYPE_INFO, "id3 v2 ConvertUnicodeToUcs4 SMTK_AUDIO_ID3_UTF16_BYTEORDER_LE \n");        
            audioConvertUtf16LeToUcs4(pString,&pBuffer[2], bufferLength - 2);
            break;

        default:
            LOG_ERROR
                "Unknown byte order #line %d \n",__LINE__  
            LOG_END                      
            break;
    }

    return MMP_TRUE;
}


MMP_INT
audioConvertUtf8ToUcs4(
    MMP_WCHAR* pUcsString,    
    MMP_UINT8* strUtf8,
    MMP_UINT32 bufferLength)
{
    MMP_UINT32 stringLength = 0;   
    // PRECONDITION    
    if ( strUtf8 == MMP_NULL )
    {
        LOG_ERROR
            "Input string is null #line %d \n",__LINE__  
        LOG_END              
        return MMP_NULL;
    }

    if ( bufferLength == 0 )
    {
        LOG_ERROR
            "Input bufferLength is null #line %d \n",__LINE__  
        LOG_END              
        return MMP_NULL;
    }
    // --- Get string length
    {
        MMP_UINT8* ptrCurr  = strUtf8;
        MMP_UINT8* ptrEnd   = strUtf8 + (bufferLength - 1);

        while ( ptrCurr <= ptrEnd )
        {
            if ( (*ptrCurr & 0x80) == 0x00 )
            {
                stringLength++;
                ptrCurr++;
            }
            else if ( (*ptrCurr & 0xE0) == 0xC0 )
            {
                stringLength++;
                ptrCurr += 2;
            }
            else if ( (*ptrCurr & 0xF0) == 0xE0 )
            {
                stringLength++;
                ptrCurr += 3;
            }
            else if ( (*ptrCurr & 0xF8) == 0xF0 )
            {
                stringLength++;
                ptrCurr += 4;
            }
            else
            {
                //dbg_msg(DBG_MSG_TYPE_INFO, "audioConvertUtf8ToUcs4: Unknow byte 0x%02X\n", *ptrCurr);
                ptrCurr++;
            }
        }
    }

    // --- Allocate string buffer
    if ( stringLength > 0 )
    {
        memset(pUcsString, 0x00, (stringLength + 1) * sizeof(MMP_WCHAR));

        {
            MMP_WCHAR* ptrUcsCurr  = pUcsString;
            MMP_UINT8* ptrUtf8Curr = strUtf8;
            MMP_UINT8* ptrUtf8End  = strUtf8 + (bufferLength - 1);

            MMP_UINT8  b1, b2, b3, b4;
            MMP_UINT32 ucsChar;

            while ( ptrUtf8Curr <= ptrUtf8End )
            {
                if ( (*ptrUtf8Curr & 0x80) == 0x00 )
                {
                    *ptrUcsCurr = *ptrUtf8Curr;                    
                    ptrUtf8Curr++;
                    ptrUcsCurr++;
                }
                else if ( (*ptrUtf8Curr & 0xE0) == 0xC0 )
                {
                    b1 = (*ptrUtf8Curr) & 0x1F;
                    b2 = (*(ptrUtf8Curr + 1)) & 0x3F;

                    ucsChar = ((MMP_UINT32)b1 << 6) | b2;
                    *ptrUcsCurr = ucsChar;                    
                    ptrUtf8Curr += 2;
                    ptrUcsCurr++;
                }
                else if ( (*ptrUtf8Curr & 0xF0) == 0xE0 )
                {
                    b1 = (*ptrUtf8Curr) & 0x0F;
                    b2 = (*(ptrUtf8Curr + 1)) & 0x3F;
                    b3 = (*(ptrUtf8Curr + 2)) & 0x3F;

                    ucsChar = ((MMP_UINT32)b1 << 12) | (MMP_UINT32)b2 << 6 | b3;
                    *ptrUcsCurr = ucsChar;

                    ptrUtf8Curr += 3;
                    ptrUcsCurr++;
                }
                else if ( (*ptrUtf8Curr & 0xF8) == 0xF0 )
                {
                    b1 = (*ptrUtf8Curr) & 0x0F;
                    b2 = (*(ptrUtf8Curr + 1)) & 0x3F;
                    b3 = (*(ptrUtf8Curr + 2)) & 0x3F;
                    b4 = (*(ptrUtf8Curr + 3)) & 0x3F;

                    ucsChar =  (MMP_UINT32)b1 << 18 | ((MMP_UINT32)b3 << 12) | (MMP_UINT32)b2 << 6 | b1;
                    *ptrUcsCurr = ucsChar;                    
                    ptrUtf8Curr += 4;
                    ptrUcsCurr++;
                }
                else
                {
                    LOG_ERROR
                        "audioConvertUtf8ToUcs4: Unknow byte 0x%02X #line %d \n",*ptrUtf8Curr,__LINE__  
                    LOG_END                              
                    ptrUtf8Curr++;
                }
            }
        }
    }

    return MMP_TRUE;
}

MMP_INT
audioConvertIsoToUcs4(
    MMP_WCHAR* pUcsString,
    MMP_UINT8* strIso,
    MMP_UINT32 strLength)
{
    MMP_UINT32 index = 0;
    // PRECONDITION    
    if (strIso == MMP_NULL)
    {
        LOG_ERROR
            "Input string is null #line %d \n",__LINE__  
        LOG_END          
        return MMP_NULL;
    }
    if (strLength == 0)
    {
        LOG_ERROR
            "Input strLength 0 #line %d \n",__LINE__  
        LOG_END             
        return MMP_NULL;
    }       
    for ( index = 0; index < strLength; index++ )
    {
        pUcsString[index] = strIso[index];
    }
    pUcsString[strLength] = 0;    
    return MMP_TRUE;
#if 0
        // Please refer to http://www.unicode.org/Public/5.0.0/ucd/Blocks.txt
        if (   (word >= 0x0370 && word <= 0x03FF)  // Greek and Coptic
            || (word >= 0x218F && word <= 0x218F)  // Number Forms
            || (word >= 0x2190 && word <= 0x21FF)  // Arrows
            || (word >= 0x2200 && word <= 0x22FF)  // Mathematical Operators
            || (word >= 0x2500 && word <= 0x257F)  // Box Drawing
            || (word >= 0x2580 && word <= 0x259F)  // Block Elements
            || (word >= 0x3300 && word <= 0x33FF)  // CJK Compatibility
            || (word >= 0x4E00 && word <= 0x9F5A)  // CJK Unified Ideographs
            || (word >= 0xFE30 && word <= 0xFE4F)  // CJK Compatibility Forms
            || (word >= 0xFF00 && word <= 0xFFEF)  // Halfwidth and Fullwidth Forms
            )
        {
        }
#endif
}

MMP_BOOL
audioAddId3V2Info(
    SMTK_AUDIO_FILE_INFO* pId3,
    char*         pFrameId,
    MMP_UINT8*    pFrameBuffer,
    MMP_UINT32    frameSize)
{
    MMP_BOOL   funcResult = MMP_TRUE;
    MMP_UINT8* pBuffer    = pFrameBuffer;
    MMP_UINT32 size       = frameSize;
    MMP_UINT8  stringFormat = pFrameBuffer[0];
    MMP_UINT16 i;

    MMP_INT nResult;
    MMP_INT nTemp,nTemp1;

    // PRECONDITION
    if ( pId3 == MMP_NULL )
    {
        LOG_ERROR
            "Input (SMTK_AUDIO_FILE_INFO*) is null #line %d \n",__LINE__  
        LOG_END              
        return MMP_FALSE;
    }

    if ( pFrameId == MMP_NULL )
    {
        LOG_ERROR
            "Input Frame ID is null #line %d \n",__LINE__  
        LOG_END                  
        return MMP_FALSE;
    }

    if ( pFrameBuffer == MMP_NULL )
    {
        LOG_ERROR
            "Input Frame buffer is null #line %d \n",__LINE__  
        LOG_END                      
        return MMP_FALSE;
    }

    // Main
    if ( PalMemcmp(pFrameId, "COMM", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0 )
    {
        if (frameSize > 256)
        {
            if(pId3->ptComment)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptComment);       
                 pId3->ptComment=0;
            }
            pId3->ptComment= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }
         // Comment
        if( frameSize>1) // (pId3->nFlag & SMTK_AUDIO_FLAG_INFO_COMMENT)
        {

            if ( stringFormat == 0x01 )
            {   // Unicode
                 audioConvertUnicodeToUcs4(pId3->ptComment,&pFrameBuffer[1], frameSize - 1);
            }
            else if ( stringFormat == 0x03 )
            {
                 audioConvertUtf8ToUcs4(pId3->ptComment,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                 audioConvertIsoToUcs4(pId3->ptComment,&pFrameBuffer[1], frameSize - 1);
            }
            if(PalWcslen(pId3->ptComment)>0)
            {
                pId3->bComment= MMP_TRUE;
                AudioPrintf(pId3->ptComment);
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(pId3->ptComment, &pFrameBuffer[1], PalWcslen(pId3->ptComment));
                #endif                
            }
        }        
    }
    else if (PalMemcmp(pFrameId, "TALB", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0)
    {
        if (frameSize > 256)
        {
            if(pId3->ptAlbum)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptAlbum);       
                 pId3->ptAlbum=0;
            }
            pId3->ptAlbum= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }   
        // Album
        if( frameSize>1 ) //(pId3->nFlag & SMTK_AUDIO_FLAG_INFO_ALBUM)
        {

            if ( stringFormat == 0x01 )
            {   // Unicode
                 audioConvertUnicodeToUcs4(pId3->ptAlbum,&pFrameBuffer[1], frameSize - 1);
            }
            else if ( stringFormat == 0x03 )
            {
                 audioConvertUtf8ToUcs4(pId3->ptAlbum,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                 audioConvertIsoToUcs4(pId3->ptAlbum,&pFrameBuffer[1], frameSize - 1);
            }
            if (PalWcslen(pId3->ptAlbum)>0)
            {
                pId3->bAlbum = MMP_TRUE;
                AudioPrintf(pId3->ptAlbum);
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(pId3->ptAlbum, &pFrameBuffer[1], PalWcslen(pId3->ptAlbum));
                #endif                
            }
        }               
    }
    else if ( PalMemcmp(pFrameId, "TCON", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0 )
    {
        if (frameSize > 256)
        {
            if(pId3->ptGenre)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptGenre);       
                 pId3->ptGenre=0;
            }
            pId3->ptGenre= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }       
        // Genres
        if (frameSize>1) //(pId3->nFlag & SMTK_AUDIO_FLAG_INFO_GENRE)
        {
            if (stringFormat == 0x01)
            {   // Unicode
                 audioConvertUnicodeToUcs4(pId3->ptGenre,&pFrameBuffer[1], frameSize - 1);
            }
            else if (stringFormat == 0x03)
            {
                 audioConvertUtf8ToUcs4(pId3->ptGenre,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                 audioConvertIsoToUcs4(pId3->ptGenre,&pFrameBuffer[1], frameSize - 1);
            }
            if (PalWcslen(pId3->ptGenre)>0)
            {
                pId3->bGenre = MMP_TRUE;
                AudioPrintf(pId3->ptGenre);
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(pId3->ptGenre, &pFrameBuffer[1], PalWcslen(pId3->ptGenre));
                #endif              
            }
       }        
    }
    else if ( PalMemcmp(pFrameId, "TIT2", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0 )
    {
        if (frameSize > 256)
        {
            if(pId3->ptTitle)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptTitle);       
                 pId3->ptTitle=0;
            }
            pId3->ptTitle= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }          
        // Title
        if( frameSize>1) //(pId3->nFlag & SMTK_AUDIO_FLAG_INFO_TITLE)
        {

            if ( stringFormat == 0x01 )
            {   // Unicode
                 audioConvertUnicodeToUcs4(pId3->ptTitle,&pFrameBuffer[1], frameSize - 1);
            }
            else if ( stringFormat == 0x03 )
            {
                 audioConvertUtf8ToUcs4(pId3->ptTitle,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                 audioConvertIsoToUcs4(pId3->ptTitle,&pFrameBuffer[1], frameSize - 1);
            }
            if(PalWcslen(pId3->ptTitle)>0)
            {
                pId3->bTitle = MMP_TRUE;
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(pId3->ptTitle, &pFrameBuffer[1], PalWcslen(pId3->ptTitle));
                #endif
                //dbg_msg(DBG_MSG_TYPE_INFO, "id3 v2 title encoding complete\n");
                AudioPrintf(pId3->ptTitle);
            }

        }
    }
    else if ( PalMemcmp(pFrameId, "TPE1", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0 )
    {
        if (frameSize > 256)
        {
            if(pId3->ptArtist)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptArtist);       
                 pId3->ptArtist=0;
            }
            pId3->ptArtist= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }           
        if(frameSize>1) //(pId3->nFlag & SMTK_AUDIO_FLAG_INFO_ARTIST)
        {
            // Artist
            if ( stringFormat == 0x01 )
            {   // Unicode
                 audioConvertUnicodeToUcs4(pId3->ptArtist,&pFrameBuffer[1], frameSize - 1);
            }
            else if ( stringFormat == 0x03 )
            {
                 audioConvertUtf8ToUcs4(pId3->ptArtist,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                 audioConvertIsoToUcs4(pId3->ptArtist,&pFrameBuffer[1], frameSize - 1);
            }
            if(PalWcslen(pId3->ptArtist)>0)
            {
                pId3->bArtist= MMP_TRUE;               
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(pId3->ptArtist, &pFrameBuffer[1], PalWcslen(pId3->ptArtist));
                #endif               
                AudioPrintf(pId3->ptArtist);                
            }

        }
    }
    else if ( PalMemcmp(pFrameId, "TYER", SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ID_LENGTH) == 0 )
    {
        if (frameSize > 256)
        {
            if(pId3->ptYear)
            {
                 PalHeapFree(PAL_HEAP_DEFAULT, pId3->ptYear);       
                 pId3->ptYear=0;
            }
            pId3->ptYear= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize*sizeof(MMP_WCHAR));            
        }               
        // Year
        if(frameSize>1) //(pId3->nFlag & SMTK_AUDIO_FLAG_INFO_YEAR)
        {
            // ----------------------------------------
            // Main
            if ( stringFormat == 0x01 )
            {   // Unicode
                audioConvertUnicodeToUcs4(pId3->ptYear,&pFrameBuffer[1], frameSize - 1);
            }
            else if ( stringFormat == 0x03 )
            {
                audioConvertUtf8ToUcs4(pId3->ptYear,&pFrameBuffer[1], frameSize - 1);
            }
            else
            {   // ISO-8859-1
                MMP_CHAR*  pYearTag = &pFrameBuffer[1];
                MMP_UINT32 length   = frameSize - 1;

                if(length >256)
                {
                    LOG_DEBUG
                      " Id3 V2 year get length >256 %d #line %d \n",length,__LINE__
                    LOG_END
                    return MMP_FALSE;
                }
                if ( pId3->ptYear != MMP_NULL )
                {
                    MMP_UINT32 index = 0;

                    for ( index = 0; index < length; index++ )
                    {
                        pId3->ptYear[index] = pYearTag[index];
                    }
                    pId3->ptYear[length] = 0x00;
                }
                else
                {
                    LOG_ERROR
                        "Allocate \"Year\" tag buffer fail #line %d \n",__LINE__  
                    LOG_END                                  
                }
            }

            if(PalWcslen(pId3->ptYear)>0)
            {
                pId3->bYear= MMP_TRUE;

                AudioPrintf(pId3->ptYear);                
            }
        }       
    }
    else
    {
        funcResult = MMP_FALSE;
    }

    return funcResult;
}

static MMP_INT
audioGetMp3FileInfo(
    SMTK_AUDIO_FILE_INFO *ptFileInfo)
{
    MMP_INT nResult;
    MMP_INT nTagSize;
    MMP_ULONG nFileSize;
    char ptCtemp[20];
    LOG_ENTER
        "audioGetMp3FileInfo\n"
    LOG_END   

    nResult = fseek(audioMgr.ptNetwork.pHandle, 0, SEEK_END);
    nFileSize = ftell(audioMgr.ptNetwork.pHandle);
    //get mp3 data
    
#ifdef METADATA_DEBUG        
    printf("[Audio mgr] It is mp3 length %d #line %d \n",nFileSize,__LINE__);
#endif

    nResult = fseek(audioMgr.ptNetwork.pHandle, nFileSize-128, SEEK_SET);
    nResult = fread(gtTag.pBuffer, 1, SMTK_AUDIO_MP3_ID3_V1_HEADER_LENGTH, audioMgr.ptNetwork.pHandle);
    if(nResult != SMTK_AUDIO_MP3_ID3_V1_HEADER_LENGTH) {
        printf("[Audio mgr] fread %d != SMTK_AUDIO_MP3_ID3_V1_HEADER_LENGTH #line %d \n",nResult,__LINE__);
        return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
    }    
    if (PalMemcmp("TAG", gtTag.pBuffer, SMTK_AUDIO_MP3_ID3_V1_HEADER_LENGTH)==0) {

        LOG_DEBUG
            " It has id3 v1 tag #line %d \n",__LINE__
        LOG_END
        //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_TITLE)
        {
            nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V1_TITLE_LENGTH, audioMgr.ptNetwork.pHandle);
            if (nResult != SMTK_AUDIO_MP3_ID3_V1_TITLE_LENGTH)
            {
                LOG_ERROR
                  " PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_TITLE_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                    
            }        
            audioConvertIsoToUcs4A(ptFileInfo->ptTitle, gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V1_TITLE_LENGTH);            
            if(PalWcslen(ptFileInfo->ptTitle)>0)
            {
                ptFileInfo->bTitle = MMP_TRUE;
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(ptFileInfo->ptTitle, gtTag.pTitle, PalWcslen(ptFileInfo->ptTitle));
                #endif                
            }
        }
         //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_ARTIST)
        {
            nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V1_ARTIST_LENGTH, audioMgr.ptNetwork.pHandle);
            if(nResult != SMTK_AUDIO_MP3_ID3_V1_ARTIST_LENGTH)
            {
                LOG_ERROR
                   "PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_ARTIST_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                                    
            }
            audioConvertIsoToUcs4A(ptFileInfo->ptArtist, gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V1_ARTIST_LENGTH);            
            if(PalWcslen(ptFileInfo->ptArtist)>0)
            {
                ptFileInfo->bArtist= MMP_TRUE;
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(ptFileInfo->ptArtist, gtTag.pTitle, PalWcslen(ptFileInfo->ptArtist));
                #endif                
            }
        }
         //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_ALBUM)
        {
            nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V1_ALBUM_LENGTH, audioMgr.ptNetwork.pHandle);
            if(nResult != SMTK_AUDIO_MP3_ID3_V1_ALBUM_LENGTH)
            {
                LOG_ERROR
                   "PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_ALBUM_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                                    
            }
            audioConvertIsoToUcs4A(ptFileInfo->ptAlbum, gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V1_ALBUM_LENGTH);             
            AudioPrintf(ptFileInfo->ptAlbum);             
            if(PalWcslen(ptFileInfo->ptAlbum)>0)
            {
                ptFileInfo->bAlbum= MMP_TRUE;
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(ptFileInfo->ptAlbum, gtTag.pTitle, PalWcslen(ptFileInfo->ptAlbum));
                #endif                                
            }
        }
         //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_YEAR)
        {
            nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V1_YEAR_LENGTH, audioMgr.ptNetwork.pHandle);
            if(nResult != SMTK_AUDIO_MP3_ID3_V1_YEAR_LENGTH)
            {
                LOG_ERROR
                   "PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_YEAR_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                                    
            }
            audioConvertIsoToUcs4A(ptFileInfo->ptYear, gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V1_YEAR_LENGTH);            
            if(PalWcslen(ptFileInfo->ptYear)>0)
            {
                ptFileInfo->bYear= MMP_TRUE;
            }
        }
         //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_COMMENT)
        {
            nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V1_COMMENT_LENGTH, audioMgr.ptNetwork.pHandle);
            if(nResult != SMTK_AUDIO_MP3_ID3_V1_COMMENT_LENGTH)
            {
                LOG_ERROR
                   "PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_COMMENT_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                                    
            }
            audioConvertIsoToUcs4A(ptFileInfo->ptComment, gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V1_COMMENT_LENGTH);
            AudioPrintf(ptFileInfo->ptComment);                          
            if(PalWcslen(ptFileInfo->ptComment)>0)
            {
                ptFileInfo->bComment= MMP_TRUE;
                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                    smtkFontMgrEncodingConverter(ptFileInfo->ptComment, gtTag.pTitle, PalWcslen(ptFileInfo->ptComment));
                #endif                
            }
        }         
         //if(ptFileInfo->nFlag & SMTK_AUDIO_FLAG_INFO_GENRE)
        {
            nResult = fread(ptFileInfo->ptGenre, 1, SMTK_AUDIO_MP3_ID3_V1_GENRE_LENGTH, audioMgr.ptNetwork.pHandle);
            if (nResult != SMTK_AUDIO_MP3_ID3_V1_GENRE_LENGTH)
            {
                LOG_ERROR
                   "PalTFileRead %d != SMTK_AUDIO_MP3_ID3_V1_GENRE_LENGTH #line %d \n",nResult,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;                                    
            }             
            if (PalWcslen(ptFileInfo->ptGenre)>0)
            {
                ptFileInfo->bGenre= MMP_TRUE;
            }            
        }       
        // get id3 v1 return, not get id3 v2
        if (ptFileInfo->bSkipId3V2 == MMP_TRUE)
        {
            return SMTK_AUDIO_ERROR_NO_ERROR;
        }
       
    }    
  
    // get id3 v2
    nResult = fseek(audioMgr.ptNetwork.pHandle, 0, SEEK_SET);
    nResult = fread(gtTag.pTitle, 1, SMTK_AUDIO_MP3_ID3_V2_3_HEADER_SIZE, audioMgr.ptNetwork.pHandle);
    if (PalMemcmp("ID3", gtTag.pTitle, SMTK_AUDIO_MP3_ID3_V2_3_HEADER_LENGTH) == 0)
    {
        /* high bit is not used */
        nTagSize = (gtTag.pTitle[6] << 21) | (gtTag.pTitle[7] << 14) | (gtTag.pTitle[8] <<  7) | (gtTag.pTitle[9] << 0);
        //nTagSize += 10;
        LOG_DEBUG
            "It has id3 v2 tag , tag size %d #line %d \n",nTagSize,__LINE__
        LOG_END
        // --- Read header
        {
            SMTK_AUDIO_MP3_ID3_V2_3_HEADER id3V2_3_Header;
            long                bytesToRead           = 0;
            MMP_BOOL            bUseUnsynchronisation = MMP_FALSE;
            MMP_BOOL            bUseExtendedHeader    = MMP_FALSE;
            MMP_BOOL            bExpertimentalHeader  = MMP_FALSE;
            MMP_UINT32          tagSize               = 0;
            MMP_UINT32          accessTagSize         = 0;

            SMTK_AUDIO_MP3_ID3_V2_3_FRAME frame;
            MMP_UINT32         frameSize        = 0;
            MMP_BOOL           bDiscardFrame    = MMP_FALSE;
            MMP_BOOL           bReadOnly        = MMP_FALSE;
            MMP_BOOL           bCompressedFrame = MMP_FALSE;
            MMP_BOOL           bEncryptedFrame  = MMP_FALSE;
            MMP_BOOL           bGroupFrame      = MMP_FALSE;
            MMP_UINT8         *pFrameBuffer;

            nResult = fseek(audioMgr.ptNetwork.pHandle, 0, SEEK_SET);
            nResult = fread(&id3V2_3_Header, 1, SMTK_AUDIO_MP3_ID3_V2_3_HEADER_SIZE, audioMgr.ptNetwork.pHandle);            
            if ( nResult != SMTK_AUDIO_MP3_ID3_V2_3_HEADER_SIZE )
            {
                LOG_ERROR
                    "Read ID3 v2.3 header fail! bytesToRead = %u #line %d \n",bytesToRead,__LINE__
                LOG_END
                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
            }

            // If a valid headr ?
            if (id3V2_3_Header.flag & 0x1F )
            {
                LOG_ERROR
                    "Not a valid flag, flag = 0x%02X #line %d \n",id3V2_3_Header,__LINE__
                LOG_END           
                return SMTK_AUDIO_ERROR_ID3_V2_FAIL;
            }

            if (id3V2_3_Header.flag & SMTK_AUDIO_MP3_ID3_V2_3_UNSYNC_BIT )
            {
                bUseUnsynchronisation = MMP_TRUE;
                LOG_DEBUG
                    "This file contains unsynchronization header #line %d \n",__LINE__
                LOG_END           
            }

            if (id3V2_3_Header.flag & SMTK_AUDIO_MP3_ID3_V2_3_EXTEND_HEADER_BIT)
            {
                bUseExtendedHeader = MMP_TRUE;
                LOG_DEBUG
                    "This file contains extended header #line %d \n",__LINE__
                LOG_END           
            }

            if (id3V2_3_Header.flag & SMTK_AUDIO_MP3_ID3_V2_3_EXPERTIMENTAL_BIT)
            {
                bExpertimentalHeader = MMP_TRUE;
                LOG_DEBUG
                    "This is an expertimental header #line %d \n",__LINE__
                LOG_END                           
            }
            tagSize = (((MMP_UINT32)(id3V2_3_Header.size[0] & 0x7F)) << 21) | (((MMP_UINT32)(id3V2_3_Header.size[1] & 0x7F)) << 14)
                      | (((MMP_UINT32)(id3V2_3_Header.size[2] & 0x7F)) << 7)  | (id3V2_3_Header.size[3] & 0x7F);
            accessTagSize = tagSize;
            LOG_DEBUG
              "ID3 V2 tag size %d #line %d \n",tagSize,__LINE__
            LOG_END           
            
            while (accessTagSize)
            {
                nResult =fread(&frame, 1, SMTK_AUDIO_MP3_ID3_V2_3_FRAME_SIZE, audioMgr.ptNetwork.pHandle);                
                if ( nResult != SMTK_AUDIO_MP3_ID3_V2_3_FRAME_SIZE )
                {
                    LOG_ERROR
                        "Read ID3 v2.3 frame fail #line %d \n",__LINE__
                    LOG_END           
                    return SMTK_AUDIO_ERROR_ID3_V2_FRAME_FAIL;
                }
                if ( (frame.id[0] == '\0') && (frame.id[1] == '\0') && (frame.id[2] == '\0') && (frame.id[3] == '\0') )
                {   /* padding */
                    //dbg_msg(DBG_MSG_TYPE_INFO, "[%d]: End of frame, stop search.\n", __LINE__);
                    return SMTK_AUDIO_ERROR_NO_ERROR;
                }

                if ( (frame.id[0] == 'A') && (frame.id[1] == 'P') && (frame.id[2] == 'I') && (frame.id[3] == 'C') )
                {   /* APIC */
                    frameSize = (MMP_UINT32) (
                                   (((MMP_UINT8)frame.size[0]) << 24)
                                | (((MMP_UINT8)frame.size[1]) << 16)
                                | (((MMP_UINT8)frame.size[2]) << 8)
                                | (MMP_UINT8)frame.size[3]);
                    LOG_DEBUG
                        "Id3V2 APIC pic size %d #line %d \n",frameSize,__LINE__
                    LOG_END                   
                    nResult = fread(ptCtemp, 1, SMTK_AUDIO_MP3_PASS_MIME_TYPE, audioMgr.ptNetwork.pHandle);                        
                    if (((ptCtemp[7]=='j') && (ptCtemp[8]=='p') && (ptCtemp[9]=='g')) ||((ptCtemp[7]=='j') && (ptCtemp[8]=='p') && (ptCtemp[9]=='e') && (ptCtemp[10]=='g')))
                    {
                        LOG_DEBUG
                            "It has jpeg attatch picture #line %d \n",__LINE__
                        LOG_END                    
                        if (frameSize>0)
                        {
                            //dbg_msg(DBG_MSG_TYPE_INFO, " %d,%d,%d,%d %d\n",(MMP_UINT8)frame.size[0],(MMP_UINT8)frame.size[1],(MMP_UINT8)frame.size[2],(MMP_UINT8)frame.size[3],frameSize);
                            //nResult = PalTFileSeek(ptFile, 14, SEEK_CUR, MMP_NULL);
                            if (frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE > SMTK_AUDIO_LIMIT_ATTATCHED_PICTURE_SIZE)                            
                            {
                                if (ptFileInfo->bUnLimitAttatchedPictureSize)
                                {
                                    ptFileInfo->ptAttatchedPicture = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE);
                                    if (!ptFileInfo->ptAttatchedPicture)
                                    {
                                        return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
                                    }                                                               
                                    ptFileInfo->bAttatchedPicture = MMP_TRUE;                                                              
                                }
                                else
                                {
                                    ptFileInfo->bAttatchedPicture = MMP_FALSE;                                                            
                                    LOG_DEBUG
                                        "attatch picture size %d > limit size %d #line %d \n",frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE,SMTK_AUDIO_LIMIT_ATTATCHED_PICTURE_SIZE,__LINE__
                                    LOG_END                    
                                }
                            }
                            else if (frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE>0)
                            {
                                ptFileInfo->ptAttatchedPicture = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE);
                                if (!ptFileInfo->ptAttatchedPicture)
                                {
                                    return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
                                }                                                           
                                ptFileInfo->bAttatchedPicture = MMP_TRUE;                                                        
                            }
                            if (frameSize>SMTK_AUDIO_MP3_PASS_MIME_TYPE && ptFileInfo->ptAttatchedPicture && ptFileInfo->bAttatchedPicture == MMP_TRUE)
                            {
                                ptFileInfo->nAttatchedPictureSize = frameSize-SMTK_AUDIO_MP3_PASS_MIME_TYPE;
                                //dbg_msg(DBG_MSG_TYPE_INFO, " attatched picture sizes %d \n",ptFileInfo->nAttatchedPictureSize);
                                nResult = fread(ptFileInfo->ptAttatchedPicture, 1, ptFileInfo->nAttatchedPictureSize, audioMgr.ptNetwork.pHandle);                        
                            }
                            if (nResult != ptFileInfo->nAttatchedPictureSize)
                            {
                                LOG_ERROR
                                    "Read Size wrong #line %d \n",__LINE__
                                LOG_END
                                return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
                            }
                            //dbg_msg(DBG_MSG_TYPE_INFO, " %d,%d,%d,%d,%d\n",ptFileInfo->ptAttatchedPicture[0],ptFileInfo->ptAttatchedPicture[1],ptFileInfo->ptAttatchedPicture[2],ptFileInfo->ptAttatchedPicture[3],ptFileInfo->ptAttatchedPicture[4]);                        
                        }
                    }
                    accessTagSize -= SMTK_AUDIO_MP3_ID3_V2_3_FRAME_SIZE;
                    accessTagSize -= frameSize;                    
                    continue;

                }               
                frameSize =  (((MMP_UINT32)frame.size[0]) << 24) | (((MMP_UINT32)frame.size[1]) << 16)
                            | (((MMP_UINT32)frame.size[2]) << 8) | (MMP_UINT8)frame.size[3];
                
                if (frameSize > tagSize)
                {
                    LOG_ERROR
                        "Found unreasonable frame size(%d) tagSize %d #line %d \n",frameSize,tagSize,__LINE__
                    LOG_END                
                    //return SMTK_AUDIO_ERROR_ID3_V2_UNREASONABLE_FRAME_SIZE_FAIL;
                    return SMTK_AUDIO_ERROR_NO_ERROR;
                }

                if (frame.flag[0] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_TAG_ALTER_BIT
                    || frame.flag[0] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_FILE_ALTER_BIT )
                {
                    bDiscardFrame = MMP_TRUE;
                }
                if (frame.flag[0] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_READONLY_BIT )
                {
                    bReadOnly = MMP_TRUE;
                }
                if (frame.flag[1] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_COMPRESS_BIT )
                {
                    bCompressedFrame = MMP_TRUE;
                }
                if (frame.flag[1] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_ENCRYPTION_BIT )
                {
                    bEncryptedFrame = MMP_TRUE;
                }
                if (frame.flag[1] & SMTK_AUDIO_MP3_ID3_V2_3_FRAME_GROUP_BIT)
                {
                    bGroupFrame = MMP_TRUE;
                }
                if ( bDiscardFrame == MMP_TRUE )
                {
                    nResult = fseek(audioMgr.ptNetwork.pHandle, frameSize, SEEK_CUR);
                }
                else
                {
                    if (frameSize==0)
                    {
                        accessTagSize -= SMTK_AUDIO_MP3_ID3_V2_3_FRAME_SIZE;
                        accessTagSize -= frameSize;                                        
                        continue;
                    }
                    pFrameBuffer = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, frameSize);
                    if (pFrameBuffer == MMP_NULL )
                    {
                        LOG_ERROR
                            "Allocate buffer for ID3 frame fail. frameSize = %u #line %d \n",frameSize,__LINE__
                        LOG_END                
                        return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
                    }
                    nResult = fread(pFrameBuffer, 1, frameSize, audioMgr.ptNetwork.pHandle);                  
                    if (nResult != frameSize )
                    {
                        LOG_ERROR
                            "Read ID3 frame fail! bytesToRead = %u, frameSize = %u #line %d \n",bytesToRead, frameSize,__LINE__
                        LOG_END                                    
                        PalHeapFree(PAL_HEAP_DEFAULT,pFrameBuffer);                        
                        pFrameBuffer = MMP_NULL;
                        return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
                    }
                     //dbg_msg(DBG_MSG_TYPE_INFO, "  %c%c%c%c \n",frame.id[0],frame.id[1],frame.id[2],frame.id[3] );
                    audioAddId3V2Info(ptFileInfo, frame.id, pFrameBuffer, frameSize);
                    PalHeapFree(PAL_HEAP_DEFAULT,pFrameBuffer);                        
                    pFrameBuffer = MMP_NULL;
                }
                accessTagSize -= SMTK_AUDIO_MP3_ID3_V2_3_FRAME_SIZE;
                accessTagSize -= frameSize;
            } // while accessTagSize
        }

    }
    
    LOG_LEAVE
        "audioGetMp3FileInfo\n"
    LOG_END
    return SMTK_AUDIO_ERROR_NO_ERROR;

}

static MMP_INT
audioGetWmaFileInfo(
    SMTK_AUDIO_FILE_INFO *ptFileInfo)
{
    MMP_INT nResult;
    MMP_ULONG nOffset = 24;
    MMP_UINT16 nTemp16;    
    MMP_UINT32 nTemp32;
    MMP_UINT64 nTemp64;    
    MMP_UINT i,j=0;  // index 
    MMP_UINT8 nData[16];
    MMP_UINT8 nPictureTag[30];    
    MMP_UINT16 strlength[5];
    MMP_UINT32 nTemp2;
    MMP_UINT64 nSize;
    MMP_UINT nTemp;
    int nTempSeek=0;
    MMP_CHAR* tempPtr;       

    LOG_ENTER
        "audioGetWmaFileInfo\n"
    LOG_END
    nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
    nResult = fread(&nTemp32, 1, 4, audioMgr.ptNetwork.pHandle);
    wmaInfo.subObjects = bswap_32(nTemp32);   
    LOG_DEBUG
        "audioGetWmaFileInfo subObjects %d %d #line %d \n",wmaInfo.subObjects,nTemp32,__LINE__
    LOG_END
    if( wmaInfo.subObjects<=0 )
    {
        LOG_ERROR
            "wma subObject Error %d  #line %d \n",wmaInfo.subObjects,__LINE__
        LOG_END
        return SMTK_AUDIO_ERROR_WMA_SUBOBJECT_FAIL;
    }
    nOffset = 30;//+audioMgr.nOffset;
    for(j=0;j<wmaInfo.subObjects;j++)
    {
        // parsing wma 
        nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
        nResult = fread(nData, 1, 16, audioMgr.ptNetwork.pHandle);
        nResult = fread(&nTemp32,1,4,audioMgr.ptNetwork.pHandle);
        nTemp = bswap_32(nTemp32);
        nOffset+=24;
        if (PalMemcmp(asf_guid_file_properties, nData, 16) == 0)
        {
            nOffset +=40;
            LOG_DEBUG
                "audioGetWmaFileInfo asf_guid_file_properties nOffset %d \n",nOffset
            LOG_END
            nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
            nResult = fread(&nTemp32, 1, 4, audioMgr.ptNetwork.pHandle);
            nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset+4, SEEK_SET);
            nResult = fread(&nTemp2, 1, 4, audioMgr.ptNetwork.pHandle);
            nOffset+=8;
            nTemp64 = nTemp32;
            wmaInfo.duration = bswap_32(nTemp64)/10000;
            LOG_DEBUG
                "audioGetWmaFileInfo wmaInfo.duration %d %d  \n",wmaInfo.duration,wmaInfo.duration/10000
            LOG_END
            wmaInfo.duration += (bswap_32( nTemp2)/10000);
            //audioSetTotoalTime(wmaInfo.duration/1000);
            #ifdef ParsingMP3TotolTime
                //fileSecPosComplete = MMP_TRUE;
            #endif
            nOffset +=20;
            nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
            nResult = fread(&nTemp32, 1, 4, audioMgr.ptNetwork.pHandle);
            nOffset+=4;
            wmaInfo.nPacket = bswap_32(nTemp32);
            nOffset +=(nTemp -24-72);
            LOG_DEBUG
                "audioGetWmaFileInfo nOffset %d wma duration %d %d #line %d\n",nOffset, wmaInfo.duration, bswap_32(nTemp32),__LINE__
            LOG_END
        }
        else if (PalMemcmp(asf_guid_content_description, nData, 16) == 0)
        {
            LOG_DEBUG
                "asf_guid_content_description nOffset %d #line %d \n",nOffset,__LINE__
            LOG_END
            for (i=0; i<5; i++) 
            {
                nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
                nResult = fread(&strlength[i], 1, 2, audioMgr.ptNetwork.pHandle);
                strlength[i] = bswap_16(strlength[i]);
                nOffset+=2;
            }
            nOffset += strlength[0]+strlength[1]+strlength[2]+strlength[3]+strlength[4];
            if (strlength[0] > 0)
            {   /* 0 - Title */
                nResult = fread(gtTag.pWmaBuf, 1, strlength[0], audioMgr.ptNetwork.pHandle);
                if (ptFileInfo->bTitle == MMP_FALSE ){
                    if (strlength[0] > SMTK_AUDIO_FILE_INFO_DEFAULT_SIZE)
                    {
                        if(ptFileInfo->ptTitle)
                        {
                             PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptTitle);       
                             ptFileInfo->ptTitle=0;
                        }
                        ptFileInfo->ptTitle= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, strlength[0]*sizeof(MMP_WCHAR));            
                    }                           
                    audio_asf_utf16LEdecode(strlength[0],gtTag.pWmaBuf,&nResult);
                    audioConvertIsoToUcs4A(ptFileInfo->ptTitle, gtTag.pWmaBuf, strlength[0]);              
                    if(PalWcslen(ptFileInfo->ptTitle)>0)
                    {
                        ptFileInfo->bTitle = MMP_TRUE;
                        LOG_DEBUG
                            "wma title length %d #line %d \n",PalWcslen(ptFileInfo->ptTitle),__LINE__
                        LOG_END
                        #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                            smtkFontMgrEncodingConverter(ptFileInfo->ptTitle, gtTag.pWmaBuf, PalWcslen(ptFileInfo->ptTitle));
                        #endif                  
                    }
                    AudioPrintf(ptFileInfo->ptTitle);     
                }          
            }
            if (strlength[1] > 0) 
            {  /* 1 - Artist */
                nResult = fread(gtTag.pWmaBuf, 1, strlength[1], audioMgr.ptNetwork.pHandle);
                if ( ptFileInfo->bArtist == MMP_FALSE ){
                    if (strlength[1] > SMTK_AUDIO_FILE_INFO_DEFAULT_SIZE)
                    {
                        if(ptFileInfo->ptArtist)
                        {
                             PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptArtist);       
                             ptFileInfo->ptArtist=0;
                        }
                        ptFileInfo->ptArtist= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, strlength[1]*sizeof(MMP_WCHAR));            
                    }                                           
                    audio_asf_utf16LEdecode(strlength[1],gtTag.pWmaBuf,&nResult);
                    audioConvertIsoToUcs4A(ptFileInfo->ptArtist, gtTag.pWmaBuf, strlength[1]);              
                    if(PalWcslen(ptFileInfo->ptArtist)>0)
                    {
                        ptFileInfo->bArtist= MMP_TRUE;
                        LOG_DEBUG
                            "wma artist length %d #line %d \n",PalWcslen(ptFileInfo->ptArtist),__LINE__
                        LOG_END
                        #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                            smtkFontMgrEncodingConverter(ptFileInfo->ptArtist, gtTag.pWmaBuf, PalWcslen(ptFileInfo->ptArtist));
                        #endif
                    }
                    AudioPrintf(ptFileInfo->ptArtist);
                }
            }      
            /* 2 - copyright */
            /* 3 - description */
            /* 4 - rating */
             LOG_DEBUG
                "asf_guid_content_description after nOffset %d #line %d \n",nOffset,__LINE__
             LOG_END
        }
        else if (PalMemcmp(asf_guid_stream_properties, nData, 16) == 0)
        {
            LOG_DEBUG
                "asf_guid_stream_properties #line %d \n",__LINE__
            LOG_END           
        }
        else if (PalMemcmp(asf_guid_extended_content_description, nData, 16) == 0)
        {
            nTempSeek = 0;
            nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset, SEEK_SET);
            {
                MMP_UINT16 count;
                int k;
                int bytesleft = nTemp- 24;
                MMP_UINT16 length, type;
                unsigned char* utf8 = utf8buf;
                int utf8length = 512;

                nTempSeek = 0;
                nResult = fread(&nTemp16,1,2,audioMgr.ptNetwork.pHandle);        
                count = bswap_16(nTemp16);
                nTempSeek+=2;
                bytesleft -= 2;
                LOG_DEBUG
                    "extended content count %d bytesleft %d #line %d \n",count,bytesleft,__LINE__
                LOG_END
                
                for (k=0; k < count; k++) 
                {
                    nResult = fseek(audioMgr.ptNetwork.pHandle, nOffset+nTempSeek, SEEK_SET);   
                    nResult = fread(&nTemp16,1,2,audioMgr.ptNetwork.pHandle);        
                    length = bswap_16(nTemp16);
                    nTempSeek+=2;
                    nResult = fread(utf8buf, 1, length, audioMgr.ptNetwork.pHandle); 
                    nTempSeek += length;
                    nResult = 512;
                    audio_asf_utf16LEdecode(length, utf8buf, &nResult);
                    bytesleft -= 2 + length;
                    nResult = fread(&nTemp16,1,2,audioMgr.ptNetwork.pHandle);        
                    type = bswap_16(nTemp16);
                    nTempSeek+=2;
                    nResult = fread(&nTemp16,1,2,audioMgr.ptNetwork.pHandle);        
                    length = bswap_16(nTemp16);
                    nTempSeek+=2;
                    if (!strcmp("WM/TrackNumber",utf8buf)) 
                    {
                       nResult = fread(utf8buf, 1, length, audioMgr.ptNetwork.pHandle);
                       nTempSeek+=length;                                              
                    }
                    else if ((!strcmp("WM/Genre",utf8buf)) && (type == 0)) 
                    {
                        nResult = fread(utf8buf, 1, length, audioMgr.ptNetwork.pHandle);
                        if (length > SMTK_AUDIO_FILE_INFO_DEFAULT_SIZE)
                        {
                            if(ptFileInfo->ptGenre)
                            {
                                 PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptGenre);       
                                 ptFileInfo->ptGenre=0;
                            }
                            ptFileInfo->ptGenre= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, length*sizeof(MMP_WCHAR));            
                        }                                                                  
                        nTempSeek+=length;                       
                        audio_asf_utf16LEdecode(length,gtTag.pWmaBuf,&nResult);
                        audioConvertIsoToUcs4A(ptFileInfo->ptGenre, gtTag.pWmaBuf, length);
                        if(PalWcslen(ptFileInfo->ptGenre)>0)
                        {
                            ptFileInfo->bGenre= MMP_TRUE;
                            LOG_DEBUG
                                "WM/Genre length %d #line %d \n",PalWcslen(ptFileInfo->ptGenre),__LINE__
                            LOG_END
                        }
                        AudioPrintf(ptFileInfo->ptGenre);                        
                    } 
                    else if ((!strcmp("WM/AlbumTitle",utf8buf)) && (type == 0)) 
                    {
                        nResult = fread(gtTag.pWmaBuf, 1, length, audioMgr.ptNetwork.pHandle);
                        if (length > SMTK_AUDIO_FILE_INFO_DEFAULT_SIZE)
                        {
                            if(ptFileInfo->ptTitle)
                            {
                                 PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptTitle);       
                                 ptFileInfo->ptTitle=0;
                            }
                            ptFileInfo->ptTitle= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, length*sizeof(MMP_WCHAR));            
                        }                                                                   
                        nTempSeek += length;
                        if (AnalyzeGB2312(gtTag.pWmaBuf,length)){
                            audio_asf_utf16LEdecode(length,gtTag.pWmaBuf,&nResult);
                            audioConvertIsoToUcs4A(ptFileInfo->ptTitle, gtTag.pWmaBuf, length);
                            if(PalWcslen(ptFileInfo->ptTitle)>0)
                            {
                                ptFileInfo->bTitle = MMP_TRUE;
                                LOG_DEBUG
                                    "WM/AlbumTitle length %d #line %d \n",PalWcslen(ptFileInfo->ptTitle),__LINE__
                                LOG_END
                                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                                    smtkFontMgrEncodingConverter(ptFileInfo->ptTitle, gtTag.pWmaBuf, PalWcslen(ptFileInfo->ptTitle));
                                #endif                            
                            }
                            AudioPrintf(ptFileInfo->ptTitle);
                        } else {
                        
                 /*   for (i=0;i<length;i++)
                            printf("0x%x ",gtTag.pWmaBuf[i]);
                        printf("\n\n");*/
                            ptFileInfo->bTitle = MMP_TRUE;
                            gtTag.pWmaBuf[length] = '\0' ;
                            UnicodeStrToUTF8Str(gtTag.pWmaBuf,ptFileInfo->ptTitleUtf8,256);
               /*         for (i=0;i<length;i++)
                            printf("0x%x ",ptFileInfo->ptTitleUtf8[i]);
                        printf("\n\n");*/

                        }
                        
                        
                    } 
                    else if ((!strcmp("WM/AlbumArtist",utf8buf)) && (type == 0)) 
                    {                   
                        nResult = fread(gtTag.pWmaBuf, 1, length, audioMgr.ptNetwork.pHandle);
                        if (length > SMTK_AUDIO_FILE_INFO_DEFAULT_SIZE)
                        {
                            if(ptFileInfo->ptArtist)
                            {
                                 PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptArtist);       
                                 ptFileInfo->ptArtist=0;
                            }
                            ptFileInfo->ptArtist= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, length*sizeof(MMP_WCHAR));            
                        }                                                                   
                        nTempSeek += length; 
                        
/*                        for (i=0;i<length;i++)
                            printf("0x%x ",gtTag.pWmaBuf[i]);
                        printf("\n\n");*/
                        if (AnalyzeGB2312(gtTag.pWmaBuf,length)){                                                
                            audio_asf_utf16LEdecode(length,gtTag.pWmaBuf,&nResult);                        
                            audioConvertIsoToUcs4A(ptFileInfo->ptArtist, gtTag.pWmaBuf, length);
                            if(PalWcslen(ptFileInfo->ptArtist)>0)
                            {
                                ptFileInfo->bArtist= MMP_TRUE;
                                LOG_DEBUG
                                " WM/AlbumArtist length %d #line %d \n",PalWcslen(ptFileInfo->ptArtist),__LINE__
                                LOG_END
                                #ifdef SMTK_FONT_SUPPORT_ENCODING_CONVERTER
                                    smtkFontMgrEncodingConverter(ptFileInfo->ptArtist, gtTag.pWmaBuf, PalWcslen(ptFileInfo->ptArtist));
                                #endif
                            }
                            AudioPrintf(ptFileInfo->ptArtist);
                        } else {

                            gtTag.pWmaBuf[length] = '\0' ;
                            ptFileInfo->bTitle = MMP_TRUE;                            
                            UnicodeStrToUTF8Str(gtTag.pWmaBuf,ptFileInfo->ptArtistUtf8,256);
                        }
/*                        printf("artist \n");
                        for (i=0;i<PalWcslen(ptFileInfo->ptTitle);i++)
                            printf("0x%x ",ptFileInfo->ptTitle[i]);
                        printf("\n\n");*/


                    } 
                    else if ((!strcmp("WM/Composer",utf8buf)) && (type == 0)) 
                    {
                        nResult = fread(utf8buf, 1, length, audioMgr.ptNetwork.pHandle);
                        nTempSeek+=length;                       
                    }
                    else if (!strcmp("WM/Year",utf8buf)) 
                    {
                        nResult = fread(utf8buf, 1, length, audioMgr.ptNetwork.pHandle);
                        nTempSeek+=length;                       
                        if (type == 0) 
                        {
                            audio_asf_utf16LEdecode(length,gtTag.pWmaBuf,&nResult);
                            audioConvertIsoToUcs4A(ptFileInfo->ptYear, gtTag.pWmaBuf, length);
                            if(PalWcslen(ptFileInfo->ptYear)>0)
                            {
                                ptFileInfo->bYear= MMP_TRUE;
                                LOG_DEBUG
                                   "WM/Year length %d #line %d \n",PalWcslen(ptFileInfo->ptYear),__LINE__
                                LOG_END
                            }
                            AudioPrintf(ptFileInfo->ptYear);
                        }
                    }
                    else if (!strcmp("WM/Picture",utf8buf)) 
                    {
                        // pass MIME TYPE, need checking different MIME type
                        nResult = fread(nPictureTag, 1, SMTK_AUDIO_WMA_PASS_MIME_TYPE, audioMgr.ptNetwork.pHandle);      
                        if(nResult != SMTK_AUDIO_WMA_PASS_MIME_TYPE)
                        {
                            LOG_ERROR
                                "Read Size wrong #line %d \n",__LINE__
                            LOG_END
                            return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
                        }
                        if( ((nPictureTag[17]=='j') && (nPictureTag[19]=='p') && (nPictureTag[21]=='g')) || ((nPictureTag[17]=='j') && (nPictureTag[19]=='p') && (nPictureTag[21]=='e') && (nPictureTag[23]=='g'))  )
                        {
                            LOG_ERROR
                                "it has a jpeg attatch picture  #line %d \n",__LINE__
                            LOG_END
                            if (length-SMTK_AUDIO_WMA_PASS_MIME_TYPE > SMTK_AUDIO_LIMIT_ATTATCHED_PICTURE_SIZE)                            
                            {
                                if (ptFileInfo->bUnLimitAttatchedPictureSize)
                                {
                                    ptFileInfo->ptAttatchedPicture = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, length-SMTK_AUDIO_WMA_PASS_MIME_TYPE);
                                    if (!ptFileInfo->ptAttatchedPicture)
                                    {
                                        return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
                                    }                                                               
                                    ptFileInfo->bAttatchedPicture = MMP_TRUE;                                                              
                                }
                                else
                                {
                                    ptFileInfo->bAttatchedPicture = MMP_FALSE;                                                            
                                    LOG_DEBUG
                                        "attatch picture size %d > limit size %d #line %d \n",length-SMTK_AUDIO_WMA_PASS_MIME_TYPE,SMTK_AUDIO_LIMIT_ATTATCHED_PICTURE_SIZE,__LINE__
                                    LOG_END                    
                                }
                            }
                            else if(length > SMTK_AUDIO_WMA_PASS_MIME_TYPE )
                            {
                                ptFileInfo->ptAttatchedPicture = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, length-SMTK_AUDIO_WMA_PASS_MIME_TYPE);
                                if (!ptFileInfo->ptAttatchedPicture)
                                {
                                    return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
                                }                                                           
                                ptFileInfo->bAttatchedPicture = MMP_TRUE;                                                        
                            }
                            if (ptFileInfo->bAttatchedPicture == MMP_TRUE)
                            {
                                nResult = fread(ptFileInfo->ptAttatchedPicture, 1, length-SMTK_AUDIO_WMA_PASS_MIME_TYPE, audioMgr.ptNetwork.pHandle);      
                                if(nResult != length-SMTK_AUDIO_WMA_PASS_MIME_TYPE)
                                {
                                    LOG_ERROR
                                        "Read Size wrong #line %d \n",__LINE__
                                    LOG_END
                                    return SMTK_AUDIO_ERROR_FILE_READ_FAIL;
                                }
                            }
                            if(length > SMTK_AUDIO_WMA_PASS_MIME_TYPE )
                            {
                                ptFileInfo->bAttatchedPicture = MMP_TRUE;
                                ptFileInfo->nAttatchedPictureSize = length -SMTK_AUDIO_WMA_PASS_MIME_TYPE;
                            }
                            //dbg_msg(DBG_MSG_TYPE_INFO, "WM/Picture %d %d %d %d %d \n",ptFileInfo->ptAttatchedPicture[0],ptFileInfo->ptAttatchedPicture[1],ptFileInfo->ptAttatchedPicture[2],ptFileInfo->ptAttatchedPicture[3],ptFileInfo->ptAttatchedPicture[4]);
                            
                        }                        
                        nTempSeek+=length; 
                    }
                    else
                    {
                         //dbg_msg(DBG_MSG_TYPE_INFO, " no this type %d\n",length);
                        tempPtr = utf8buf;
                        //while(*tempPtr)
                        //    dbg_msg(DBG_MSG_TYPE_INFO, "%c", *tempPtr++);

                        //dbg_msg(DBG_MSG_TYPE_INFO, "\n");
                        nTempSeek+=length;                       
                        //dbg_msg(DBG_MSG_TYPE_INFO, " no this type %d %d nTempSeek %d\n",length,nResult,nTempSeek);
                    }
                    bytesleft -= 4 + length;
                }
               // advance_buffer(bytesleft);
            }       
            nOffset += nTemp-24;
            LOG_DEBUG
             " asf_guid_extended_content_descriptionn Offset %d nSize %d #line %d \n",nOffset,(int)nTemp,__LINE__
            LOG_END                                
        }
        else if (PalMemcmp(asf_guid_content_encryption, nData, 16) == 0 
                || PalMemcmp(asf_guid_extended_content_encryption, nData, 16) == 0) 
        {
            nOffset += nTemp ;        
            LOG_DEBUG
                "asf_guid_content_encryption nSize %d \n",nTemp
            LOG_END                   
        }
        else
        {
            nOffset +=nTemp- 24;
            LOG_DEBUG
                "Wma skip %d #line %d \n",(nTemp- 24),__LINE__
            LOG_END                               
        }      
    }

    LOG_LEAVE
        "audioGetWmaFileInfo\n"
    LOG_END
    return SMTK_AUDIO_ERROR_NO_ERROR;
}


MMP_INT
smtkAudioMgrGetFileInfo(
   SMTK_AUDIO_FILE_INFO *ptFileInfo)
{
    MMP_INT nResult=0;    
    MMP_ULONG nFileSize;
    LOG_ENTER
      "smtkAudioMgrGetFileInfo\n"
    LOG_END
    if(audioMgr.ptNetwork.bLocalPlay==0) {
        printf("[Audio mgr] smtkAudioMgrGetFileInfo must local play #line %d \n",__LINE__);
        return SMTK_AUDIO_ERROR_FILE_NOT_OPEN;
    }
    if (audioMgr.ptNetwork.nType==SMTK_AUDIO_WAV) {
        return 0;
    }
    // release memory
    smtkAudioMgrGetFileInfoClose(ptFileInfo);
    // allocate memory
    ptFileInfo->ptTitle =(MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));    
    ptFileInfo->ptArtist=(MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));    
    ptFileInfo->ptAlbum=(MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));    
    ptFileInfo->ptYear= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));   
    ptFileInfo->ptComment= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));            
    ptFileInfo->ptGenre= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));        
    ptFileInfo->ptTools= (MMP_WCHAR*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(MMP_WCHAR));
    ptFileInfo->ptTitleUtf8=(unsigned char*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(unsigned char));    
    ptFileInfo->ptArtistUtf8=(unsigned char*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(unsigned char));    
    ptFileInfo->ptAlbumUtf8=(unsigned char*)PalHeapAlloc(PAL_HEAP_DEFAULT, 256*sizeof(unsigned char));    
    
    if (!ptFileInfo->ptTitle || !ptFileInfo->ptArtist || !ptFileInfo->ptAlbum || !ptFileInfo->ptYear || !ptFileInfo->ptComment ||
         !ptFileInfo->ptGenre || !ptFileInfo->ptTools ||!ptFileInfo->ptTitleUtf8 ||!ptFileInfo->ptArtistUtf8 ||! ptFileInfo->ptAlbumUtf8)
    {
        printf("[Audio mgr]smtkAudioMgrGetFileInfo SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL #line %d\n",__LINE__);    
        return SMTK_AUDIO_ERROR_NOT_ALLOCATE_MEMORY_FAIL;
    }
    memset(ptFileInfo->ptTitle, 0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptArtist, 0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptAlbum, 0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptYear,  0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptComment,0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptGenre, 0, 256*sizeof(MMP_WCHAR));
    memset(ptFileInfo->ptTools, 0, 256*sizeof(MMP_WCHAR)); 
    memset(ptFileInfo->ptTitleUtf8, 0, 256*sizeof(unsigned char));
    memset(ptFileInfo->ptArtistUtf8, 0, 256*sizeof(unsigned char));
    memset(ptFileInfo->ptAlbumUtf8, 0, 256*sizeof(unsigned char));    
    nResult = 0;
    if (audioMgr.ptNetwork.nType==SMTK_AUDIO_MP3) {
        nResult = audioGetMp3FileInfo(ptFileInfo);
    }else if (audioMgr.ptNetwork.nType==SMTK_AUDIO_WMA) {
        nResult = audioGetWmaFileInfo(ptFileInfo);            
    }
    nResult = fseek(audioMgr.ptNetwork.pHandle, 0, SEEK_SET);

    #ifdef DUMP_AUDIO_ATTATCH_PICTURE
        if(ptFileInfo->bAttatchedPicture == MMP_TRUE) {
            //dbg_msg(DBG_MSG_TYPE_INFO, "picture size %d \n",ptFileInfo->nAttatchedPictureSize);       
            if(ptAttatchPictureFile==MMP_NULL) {
                 sprintf(ptFileName, "C:/dump%d.jpg", nFileCount++);
                 AudioPrintf(ptFileName);
                 ptAttatchPictureFile = fopen(ptFileName, "wb");
            }        
            if (ptAttatchPictureFile == MMP_NULL){
                LOG_DEBUG
                  " ptAttatchPictureFile not open success #line %d %c%c%c%c%c%c%c%c%c%c%c%c\n",__LINE__,ptFileName[0],ptFileName[1],ptFileName[2],ptFileName[3],ptFileName[4],ptFileName[5],ptFileName[6],ptFileName[7],ptFileName[8],ptFileName[9],ptFileName[10],ptFileName[11]
                LOG_END
            } else {
                PalFileWrite(ptFileInfo->ptAttatchedPicture, sizeof(char), ptFileInfo->nAttatchedPictureSize, ptAttatchPictureFile, MMP_NULL);
                PalFileClose(ptAttatchPictureFile, MMP_NULL);
                ptAttatchPictureFile = MMP_NULL;
            }

        }
    #endif // #ifdef DUMP_AUDIO_ATTATCH_PICTURE       

    if (ptFileInfo->bAlbum && audioMgr.ptNetwork.nType==SMTK_AUDIO_MP3){
        //UnicodeStrToUTF8Str(ptFileInfo->ptAlbum,ptFileInfo->ptAlbumUtf8,256);
        Unicode32ToUtf8(ptFileInfo->ptAlbum,128,ptFileInfo->ptAlbumUtf8,256);
    }

    if (ptFileInfo->bArtist && audioMgr.ptNetwork.nType==SMTK_AUDIO_MP3){
        //UnicodeStrToUTF8Str(ptFileInfo->ptArtist,ptFileInfo->ptArtistUtf8,256);
        Unicode32ToUtf8(ptFileInfo->ptArtist,128,ptFileInfo->ptArtistUtf8,256);
    }

    if (ptFileInfo->bTitle && audioMgr.ptNetwork.nType==SMTK_AUDIO_MP3){
        //UnicodeStrToUTF8Str(ptFileInfo->ptTitle,ptFileInfo->ptTitleUtf8,256);
        Unicode32ToUtf8(ptFileInfo->ptTitle,128,ptFileInfo->ptTitleUtf8,256);
    }


    if (nResult) {
        LOG_ERROR
            "smtkAudioMgrGetFileInfo error %d #line %d\n",nResult,__LINE__
        LOG_END    
    }   
    LOG_LEAVE
       "smtkAudioMgrGetFileInfo \n"
    LOG_END

    return nResult;
}

// Close file Info
MMP_INT
smtkAudioMgrGetFileInfoClose(
   SMTK_AUDIO_FILE_INFO *ptFileInfo)
{
    LOG_ENTER
       " audio get FileInfoClose #line %d \n",__LINE__
    LOG_END
    
    if (ptFileInfo->ptTitle) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptTitle);
         ptFileInfo->ptTitle=0;
    }
    if (ptFileInfo->ptArtist) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptArtist);
         ptFileInfo->ptArtist=0;
    }
    if (ptFileInfo->ptAlbum) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptAlbum);
         ptFileInfo->ptAlbum=0;
    }
    if (ptFileInfo->ptYear) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptYear);
         ptFileInfo->ptYear=0;
    }
    if (ptFileInfo->ptComment) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptComment);
         ptFileInfo->ptComment=0;
    }
    if (ptFileInfo->ptGenre) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptGenre);
         ptFileInfo->ptGenre=0;
    }
    if (ptFileInfo->ptTools) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptTools);
         ptFileInfo->ptTools=0;
    }
    if (ptFileInfo->ptTitleUtf8) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptTitleUtf8);
         ptFileInfo->ptTitleUtf8=0;
    }
    if (ptFileInfo->ptAlbumUtf8) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptAlbumUtf8);
         ptFileInfo->ptAlbumUtf8=0;
    }
    if (ptFileInfo->ptArtistUtf8) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptArtistUtf8);
         ptFileInfo->ptArtistUtf8=0;
    }
    
    if (ptFileInfo->ptAttatchedPicture) {
         PalHeapFree(PAL_HEAP_DEFAULT, ptFileInfo->ptAttatchedPicture);
         ptFileInfo->nAttatchedPictureSize = 0;
    }
    ptFileInfo->ptAttatchedPicture=0;
    ptFileInfo->bAlbum = MMP_FALSE;
    ptFileInfo->bArtist = MMP_FALSE;
    ptFileInfo->bAttatchedPicture = MMP_FALSE;
    ptFileInfo->bComment = MMP_FALSE;
    ptFileInfo->bGenre = MMP_FALSE;
    ptFileInfo->bTitle = MMP_FALSE;
    ptFileInfo->bTools = MMP_FALSE;
    ptFileInfo->ptTitleUtf8 = MMP_FALSE;
    ptFileInfo->ptAlbumUtf8 = MMP_FALSE;
    ptFileInfo->ptArtistUtf8 = MMP_FALSE;    
    ptFileInfo->bYear = MMP_FALSE;

    LOG_LEAVE
       " audio get FileInfoClose complete #line %d \n",__LINE__
    LOG_END

    return 0;
}



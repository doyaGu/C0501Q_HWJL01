#include "globle.h"
#include "test_items.h"
#include "io_api.h"
#include "ite/itv.h"
#include "ite/ith_video.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#if (_MSC_VER)
    #include <io.h>
    #include <direct.h>
    #define _INIT_UNION(a, b)      {b}
    #define __byte_align4 __attribute__ ((aligned(4)))
#else
    #define _INIT_UNION(a, b)      {.a = b}
    #define _INIT_UNION_FUNC(a, b) { ## a ## b}
    #define __byte_align4 __attribute__ ((aligned(4)))
#endif

//#define _f_toupper(ch)             (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _OptionDef_TAG
{
    const char *name;
    int        flags;
#define OPT_STRING 0x0008
#define OPT_INT    0x0080
#define OPT_FUNC2  0x2000
    union
    {
        int  value;
        char *str_ptr;
        void *dst_ptr;
        int (*func_arg)(void *, void *);
    } u;

    int (*func2_arg)(void *, void *, char *);
} OptionDef;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static int
opt_load_one_bmp_file(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define BMP_FILE_EXIST 0
#define BMP_FILE_DATA  "test_800x600_422.bmp.data"
    static BASE_STRM_INFO bmpStreamInfo = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    bmpStreamInfo.u.filename       = filename;
    return (int)(&bmpStreamInfo);
#else
    #if (BMP_FILE_EXIST)
    static uint8_t __byte_align4 BmpFileName[] = {
        #include BMP_FILE_DATA
    };

    printf("\n\tload data: %s\n", BMP_FILE_DATA);

    bmpStreamInfo.u.strm.startAddr = BmpFileName;
    bmpStreamInfo.u.strm.length    = sizeof(BmpFileName);

    return (int)(&bmpStreamInfo);
    #else
    return 0;
    #endif
#endif
}

static int
opt_load_one_jpg_file_00(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define JPG_FILE_00_EXIST 0
#define JPG_FILE_DATA_00  "800x600_422.jpg.data"
    static BASE_STRM_INFO jpgStreamInfo_00 = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    jpgStreamInfo_00.u.filename    = filename;
    return (int)(&jpgStreamInfo_00);
#else
    #if (JPG_FILE_00_EXIST)
    static uint8_t __byte_align4 JpgFileName_00[] = {
        #include JPG_FILE_DATA_00
    };

    printf("\n\tload data: %s\n", JPG_FILE_DATA_00);

    jpgStreamInfo_00.u.strm.startAddr = JpgFileName_00;
    jpgStreamInfo_00.u.strm.length    = sizeof(JpgFileName_00);

    return (int)(&jpgStreamInfo_00);
    #else
    return 0;
    #endif
#endif
}

static int
opt_load_one_jpg_file_01(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define JPG_FILE_01_EXIST 1
#define JPG_FILE_DATA_01  "Canon_S2IS_007_LB.jpg.data"
    static BASE_STRM_INFO jpgStreamInfo_01 = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    jpgStreamInfo_01.u.filename    = filename;
    return (int)(&jpgStreamInfo_01);
#else
    #if (JPG_FILE_01_EXIST)
    static uint8_t __byte_align4 JpgFileName_01[] = {
        #include JPG_FILE_DATA_01
    };

    printf("\n\tload data: %s\n", JPG_FILE_DATA_01);

    jpgStreamInfo_01.u.strm.startAddr = JpgFileName_01;
    jpgStreamInfo_01.u.strm.length    = sizeof(JpgFileName_01);

    return (int)(&jpgStreamInfo_01);
    #else
    return 0;
    #endif
#endif
}

static int
opt_dump_one_jpg_file(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define JPG_DUMP_DATA "test_enc.jpg"
    static BASE_STRM_INFO jpgStreamInfo = {0};

#if (HAS_FILE_SYS)
    jpgStreamInfo.u.filename = filename;
    return (int)(&jpgStreamInfo);
#else
    static uint8_t JpgFileName[300 << 10] = {0};

    jpgStreamInfo.u.strm.startAddr = JpgFileName;
    jpgStreamInfo.u.strm.length    = sizeof(JpgFileName);

    return (int)(&jpgStreamInfo);
#endif
}

static int
opt_load_one_yuv_file(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define YUV_FILE_EXIST 1
#define YUV_FILE_DATA  "d:/qcif_420_3s.yuv.data"     //"qcif_420_3s.yuv.data"
    static BASE_STRM_INFO yuvStreamInfo = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    yuvStreamInfo.u.filename       = filename;
    return (int)(&yuvStreamInfo);
#else
    #if (YUV_FILE_EXIST)
    static uint8_t __byte_align4 YuvFileName[] = {
        #include YUV_FILE_DATA
    };

    printf("\n\tload data: %s\n", YUV_FILE_DATA);
    yuvStreamInfo.u.strm.startAddr = YuvFileName;
    yuvStreamInfo.u.strm.length    = sizeof(YuvFileName);

    return (int)(&yuvStreamInfo);
    #else
    return 0;
    #endif
#endif
}

static int
opt_load_one_raw_file_00(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define RAW_FILE_00_EXIST      0
#define RAW_FILE_COLOR_TYPE_00 2
#if (RAW_FILE_COLOR_TYPE_00 == 1)
    // yuv444
    #define RAW_FILE_DATA_00   "yuv444_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 2)
    // yuv422
    #define RAW_FILE_DATA_00   "yuv422_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 3)
    // yuv422r
    #define RAW_FILE_DATA_00   "yuv422r_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 4)
    //yuv420
    #define RAW_FILE_DATA_00   "yuv420_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 5)
    //rgb565
    #define RAW_FILE_DATA_00   "rgb565_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 6)
    // argb8888
    #define RAW_FILE_DATA_00   "argb8888_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 7)
    // nv12
    #define RAW_FILE_DATA_00   "NV12_800x600_01.raw.data"
#elif (RAW_FILE_COLOR_TYPE_00 == 8)
    // nv21
    #define RAW_FILE_DATA_00   "NV21_800x600_01.raw.data"
#else

#endif

    static BASE_STRM_INFO rawStreamInf_00 = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    rawStreamInf_00.u.filename     = filename;
    return (int)(&rawStreamInf_00);
#else
    #if (RAW_FILE_00_EXIST)
    static uint8_t __byte_align4 RawFileName_00[] = {
        #include RAW_FILE_DATA_00
    };

    printf("\n\tload data: %s\n", RAW_FILE_DATA_00);

    rawStreamInf_00.u.strm.startAddr = RawFileName_00;
    rawStreamInf_00.u.strm.length    = sizeof(RawFileName_00);
    printf(".......... startAddr= 0x%x\r\n", rawStreamInf_00.u.strm.startAddr);
    return (int)(&rawStreamInf_00);
    #else
    return 0;
    #endif
#endif
}

static int
opt_load_one_raw_file_01(
    void *ptr,
    void *ptr2,
    char *filename)
{
#define RAW_FILE_01_EXIST      0
#define RAW_FILE_COLOR_TYPE_01 4
#if (RAW_FILE_COLOR_TYPE_01 == 1)
    // yuv444
    #define RAW_FILE_DATA_01   "yuv444_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 2)
    // yuv422
    #define RAW_FILE_DATA_01   "yuv422_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 3)
    // yuv422r
    #define RAW_FILE_DATA_01   "yuv422r_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 4)
    // yuv420
    #define RAW_FILE_DATA_01   "yuv420_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 5)
    // rgb565
    #define RAW_FILE_DATA_01   "rgb565_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 6)
    // argb8888
    #define RAW_FILE_DATA_01   "argb8888_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 7)
    // nv12
    #define RAW_FILE_DATA_01   "NV12_800x600_02.raw.data"
#elif (RAW_FILE_COLOR_TYPE_01 == 8)
    // nv21
    #define RAW_FILE_DATA_01   "NV21_800x600_02.raw.data"
#else

#endif
    static BASE_STRM_INFO rawStreamInf_01 = {0};

#if (HAS_FILE_SYS)
    filename[strlen(filename) - 5] = '\0';
    rawStreamInf_01.u.filename     = filename;
    return (int)(&rawStreamInf_01);
#else
    #if (RAW_FILE_01_EXIST)
    static uint8_t __byte_align4 RawFileName_01[] = {
        #include RAW_FILE_DATA_01
    };

    printf("\n\tload data: %s\n", RAW_FILE_DATA_01);

    rawStreamInf_01.u.strm.startAddr = RawFileName_01;
    rawStreamInf_01.u.strm.length    = sizeof(RawFileName_01);

    return (int)(&rawStreamInf_01);
    #else
    return 0;
    #endif
#endif
}

////////////////////////////////////////////////
int
parse_option(
    OptionDef *options,
    char      *atomType)
{
    int ret = 0;
    int i   = 0;

    while (options[i].name)
    {
        if (strcmp(options[i].name, atomType) == 0)
        {
            if (options[i].flags & OPT_INT)
                ret = options[i].u.value;
            else if (options[i].flags & OPT_FUNC2)
                ret = options[i].func2_arg(0, 0, options[i].u.str_ptr);
            else if (options[i].flags & OPT_STRING)
                ret = (int)options[i].u.str_ptr;

            return ret;
        }

        i++;
    }

    return 0xdeadbeaf;
}

static OptionDef cfgParam[] =
{
    //---------------------------
    // isp test item (Just enable one test item in running time)
    {"Test_Lcd",            OPT_INT,                                                    0},
    {"Test_Bmp",            OPT_INT,                                                    0},
    {"Test_JPEG",           OPT_INT,                                                    1}, // Now it work
    {"Test_PlayVideo",      OPT_INT,                                                    0}, // only input avi YU12/I420 format
    {"Test_Onfly",          OPT_INT,                                                    0}, // work on ROM code emummp_p9070_ddrii_2011_0726_ISP_LCD_UIENC.bit
    {"Test_colorTrans",     OPT_INT,                                                    0}, // test input YUV444/420/422/422R/NV12/RGB, and output rgb565
    {"Test_UiEnc",          OPT_INT,                                                    0},

    //---------------------------
    // Test_Bmp
    {"OnMultiBmpDec",       OPT_INT,                                                    0},
    {"BmpFileName",         OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, BMP_FILE_DATA), opt_load_one_bmp_file}, // signle bmp decode
    {"BmpDirPath",          OPT_STRING,                          _INIT_UNION(str_ptr, "./")}, // if (OnMultiBmpDec,, 1)

    //---------------------------
    // Test_JPEG
    {"JpgFileName_00",      OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, JPG_FILE_DATA_00), opt_load_one_jpg_file_00}, // signle jpg decode
    {"JpgFileName_01",      OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, JPG_FILE_DATA_01), opt_load_one_jpg_file_01}, // signle jpg decode
    {"JpgEncode",           OPT_INT,                                                    0},
    {"JpgOutName",          OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, JPG_DUMP_DATA), opt_dump_one_jpg_file},
    {"MultiJpgDecOn",       OPT_INT,                                                    1},
#if !(_MSC_VER)
	{"JpgDirPath",			OPT_STRING, 						 _INIT_UNION(str_ptr, "A:/")}, //"."  // if (MultiJpgDecOn,, 1)
#else
	{"JpgDirPath",			OPT_STRING, 						 _INIT_UNION(str_ptr, "D:/")},
#endif
    //---------------------------
    // Test_PlayVideo
    {"AviFileName",         OPT_STRING,                          _INIT_UNION(str_ptr, "Gee_320x240_I420.avi")},
    {"YuvFileName",         OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, YUV_FILE_DATA), opt_load_one_yuv_file},
    {"YuvSpace",            OPT_INT,                                                    0}, // 0: 420, 1:422
    {"YuvFile_W",           OPT_INT,                                                  176},
    {"YuvFile_H",           OPT_INT,                                                  144},

    //---------------------------
    // Test color transform
    {"ColorTestFillPpm_01", OPT_STRING,                          _INIT_UNION(str_ptr, "./CIMG9710.ppm")},
    {"ColorTestFillPpm_02", OPT_STRING,                          _INIT_UNION(str_ptr, "./P1000693.ppm")},
    {"ColorTestRawData_00", OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, RAW_FILE_DATA_00), opt_load_one_raw_file_00},
    {"ColorTestRawData_01", OPT_STRING | OPT_FUNC2,              _INIT_UNION(str_ptr, RAW_FILE_DATA_01), opt_load_one_raw_file_01},
    {"RawData_W",           OPT_INT,                                                  800},
    {"RawData_H",           OPT_INT,                                                  600},

    //---------------------------
    // Test UiEnc
    //{"UiTestRgb_01",            OPT_STRING | OPT_FUNC2, _INIT_UNION(str_ptr, UIENC_RGB_FILE_00_DATA), opt_load_uiEnc_rgb_file_00},
    //{"UiTestRgb_02",            OPT_STRING | OPT_FUNC2, _INIT_UNION(str_ptr, UIENC_RGB_FILE_01_DATA), opt_load_uiEnc_rgb_file_01},
    {                    0,                                   0,                        0},
};

void
_color_trans(
    void)
{
    void            *inData00       = (void *)parse_option(cfgParam, "ColorTestRawData_00");
    void            *inData01       = (void *)parse_option(cfgParam, "ColorTestRawData_01");
    DATA_COLOR_TYPE srcColorType_00 = 0xF;
    DATA_COLOR_TYPE srcColorType_01 = 0xF;
    int             srcWidth        = parse_option(cfgParam, "RawData_W");
    int             srcHeight       = parse_option(cfgParam, "RawData_H");

    while (1)
    {
#if (RAW_FILE_COLOR_TYPE_00 == 1)
        srcColorType_00 = DATA_COLOR_YUV444;
        printf(" test 1 %s\n", "DATA_COLOR_YUV444");
#elif (RAW_FILE_COLOR_TYPE_00 == 2)
        srcColorType_00 = DATA_COLOR_YUV422;
        printf(" test 1 %s\n", "DATA_COLOR_YUV422");
#elif (RAW_FILE_COLOR_TYPE_00 == 3)
        srcColorType_00 = DATA_COLOR_YUV422R;
        printf(" test 1 %s\n", "DATA_COLOR_YUV422R");
#elif (RAW_FILE_COLOR_TYPE_00 == 4)
        srcColorType_00 = DATA_COLOR_YUV420;
        printf(" test 1 %s\n", "DATA_COLOR_YUV420");
#elif (RAW_FILE_COLOR_TYPE_00 == 5)
        srcColorType_00 = DATA_COLOR_RGB565;
        printf(" test 1 %s\n", "DATA_COLOR_RGB565");
#elif (RAW_FILE_COLOR_TYPE_00 == 6)
        srcColorType_00 = DATA_COLOR_ARGB8888;
        printf(" test 1 %s\n", "DATA_COLOR_ARGB8888");
#elif (RAW_FILE_COLOR_TYPE_00 == 7)
        srcColorType_00 = DATA_COLOR_NV12;
        printf(" test 1 %s\n", "DATA_COLOR_NV12");
#elif (RAW_FILE_COLOR_TYPE_00 == 8)
        srcColorType_00 = DATA_COLOR_NV21;
        printf(" test 1 %s\n", "DATA_COLOR_NV21");
#else
        _err(" no input !!");
#endif

        item_isp_color_trans(inData00, srcColorType_00, srcWidth, srcHeight);
        _Sleep(1000);

#if (RAW_FILE_COLOR_TYPE_01 == 1)
        srcColorType_01 = DATA_COLOR_YUV444;
        printf(" test 2 %s\n", "DATA_COLOR_YUV444");
#elif (RAW_FILE_COLOR_TYPE_01 == 2)
        srcColorType_01 = DATA_COLOR_YUV422;
        printf(" test 2 %s\n", "DATA_COLOR_YUV422");
#elif (RAW_FILE_COLOR_TYPE_01 == 3)
        srcColorType_01 = DATA_COLOR_YUV422R;
        printf(" test 2 %s\n", "DATA_COLOR_YUV422R");
#elif (RAW_FILE_COLOR_TYPE_01 == 4)
        srcColorType_01 = DATA_COLOR_YUV420;
        printf(" test 2 %s\n", "DATA_COLOR_YUV420");
#elif (RAW_FILE_COLOR_TYPE_01 == 5)
        srcColorType_01 = DATA_COLOR_RGB565;
        printf(" test 2 %s\n", "DATA_COLOR_RGB565");
#elif (RAW_FILE_COLOR_TYPE_01 == 6)
        srcColorType_01 = DATA_COLOR_ARGB8888;
        printf(" test 2 %s\n", "DATA_COLOR_ARGB8888");
#elif (RAW_FILE_COLOR_TYPE_01 == 7)
        srcColorType_01 = DATA_COLOR_NV12;
        printf(" test 2 %s\n", "DATA_COLOR_NV12");
#elif (RAW_FILE_COLOR_TYPE_01 == 8)
        srcColorType_01 = DATA_COLOR_NV21;
        printf(" test 2 %s\n", "DATA_COLOR_NV21");
#else
        _err(" no input !!");
#endif

        item_isp_color_trans(inData01, srcColorType_01, srcWidth, srcHeight);
        _Sleep(1000);
    }

    return;
}

static int
_checkFileExtention(
    char *ext,
    char *filename)
{
    uint32_t extLen   = strlen(ext);
    uint32_t nameLeng = strlen(filename);
    int      i;

    for (i = 0; i < extLen; i++)
    {
        if (ext[extLen - i] == '*')
            break;

        if (_f_toupper(filename[nameLeng - i]) != _f_toupper(ext[extLen - i]) )
            return 0;
    }

    return 1;
}

//=============================================================================
//                Public Function Definition
//=============================================================================

static void
VideoInit(
    void)
{
    ithVideoInit(NULL);
    itv_init();
}

static void
VideoExit(
    void)
{
    /* release dbuf & itv */
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
}

void Test_Proc(void)
{
    uint32_t       reservedBuf = 0;
    BASE_RECT      lcdRect     = {0};
    struct timeval currT       = {0};

    itpInit();
    ioctl(ITP_DEVICE_SCREEN,    ITP_IOCTL_POST_RESET, NULL);
    //ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_ON, NULL);

    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET,      NULL);
    VideoInit();

    reservedBuf = itpVmemAlloc(0x80);  // for win32 case

	usleep(1000*1000*1); //for freertos waiting for USB ready.
	
    gettimeofday(&currT, NULL);
    srand(currT.tv_usec);

    lcdRect.w = _Get_Lcd_Width();
    lcdRect.h = _Get_Lcd_Height();

    if (parse_option(cfgParam, "Test_Lcd") )
    {
        printf("------- start test lcd\n");
        item_lcd();
    }
    else if (parse_option(cfgParam, "Test_Bmp") )
    {
        printf("------- start test bmp\n");
        _err("No implement !!");
    }
    else if (parse_option(cfgParam, "Test_colorTrans") )
    {
        printf("------- start test color trans\n");
        _color_trans();
    }
    else if (parse_option(cfgParam, "Test_PlayVideo") )
    {
        void *inData    = (void *)parse_option(cfgParam, "YuvFileName");
        void *uiData_00 = (void *)parse_option(cfgParam, "UiTestRgb_01");
        void *uiData_01 = (void *)parse_option(cfgParam, "UiTestRgb_02");
        int  srcWidth   = parse_option(cfgParam, "YuvFile_W");
        int  srcHeight  = parse_option(cfgParam, "YuvFile_H");

        printf("------- start test play video\n");
        item_play_video(inData, uiData_00, uiData_01, srcWidth, srcHeight);
    }
    else if (parse_option(cfgParam, "Test_JPEG") )
    {
        void *inData[2] = {0};

        if (parse_option(cfgParam, "JpgEncode") )
        {
            char outName[128] = {0};
            char inName[128]  = {0};
            int  srcWidth     = parse_option(cfgParam, "YuvFile_W");
            int  srcHeight    = parse_option(cfgParam, "YuvFile_H");
            int  bYuv422      = parse_option(cfgParam, "YuvSpace");

            inData[0] = (void *)parse_option(cfgParam, "YuvFileName");

            printf("------- start test jpg encode\n");
#if !(_MSC_VER)
            sprintf(inName,  "%s/%s", (char *)parse_option(cfgParam, "JpgDirPath"), "Canon_S2IS_007_LB.jpg"); //"pic_001.jpg");
            sprintf(outName, "%s/%s", (char *)parse_option(cfgParam, "JpgDirPath"), "test_jpg_encode.jpg");
#else
            sprintf(inName,  "D:/%s", "qcif_420_3s.yuv");
            sprintf(outName, "D:/%s", "test_jpg_encode.jpg");
#endif
            Draw_Rect((uint8_t *)_Get_Lcd_Addr_A(), _Get_Lcd_Pitch(), &lcdRect, ((rand() >> 3) & 0xFFFF));
            //Draw_Rect((uint8_t*)_Get_Lcd_Addr_B(), _Get_Lcd_Pitch(), &lcdRect, 0xFFFF);

            //item_jpeg_isp_dec_fileName(inName);

            //-------------------------------------
            // input yuv raw data
            printf("inName = %s ,srcWidth = %d ,srcHeight =%d,bYuv422=%d ,outName =%s\n", inName, srcWidth, srcHeight, bYuv422, outName);
            item_jpeg_encoder(inData[0], srcWidth, srcHeight,
                              (bYuv422) ? JPEG_ENC_YUV_422 : JPEG_ENC_YUV_420, outName);

            printf("encode OK!\n");
            printf("\n");
            usleep(1000 * 1000 * 1);
            item_jpeg_isp_dec_fileName("d:/test_jpg_encode.jpg.0.jpg");
        }
        else if (parse_option(cfgParam, "MultiJpgDecOn") )
        {
            int     ret          = 0;
            char    dirPath[128] = {0};
            bool    bFindEnd     = false, bReset = true;

#if !(_MSC_VER)
            FN_FIND Hfind        = {0};

            printf("------- start test jpg/isp dir decode\n");

            sprintf(dirPath, "%s/*.*", (char *)parse_option(cfgParam, "JpgDirPath"));
            printf("path: %s\r\n", dirPath);

            while (1) //bFindEnd == false )
            {
                if (bReset == true)
                {
                    ret    = f_findfirst(dirPath, &Hfind);
                    if (ret)
                        _err(" f_findfirst(%s) fail  (%d)!! ", dirPath, ret);
                    bReset = false;
                }

                if (!(Hfind.attr & F_ATTR_DIR) &&
                    strcmp(Hfind.filename, "..") &&
                    strcmp(Hfind.filename, ".") )
                {
                    F_FILE *fout = 0;

                    printf("\tF:%s\r\n", Hfind.filename);
                    if (_checkFileExtention("*.jpg", Hfind.filename) ||
                        _checkFileExtention("*.jpeg", Hfind.filename) )
                    {
                        char fileName[256] = {0};
                        sprintf(fileName, "%s/%s", (char *)parse_option(cfgParam, "JpgDirPath"), Hfind.filename);
                        //sprintf(fileName, "%s/%s", (char*)parse_option(cfgParam, "JpgDirPath"), "Canon_IXUS300_1600x1200_010.jpg");
                        Draw_Rect((uint8_t *)_Get_Lcd_Addr_A(), _Get_Lcd_Pitch(), &lcdRect, ((rand() >> 3) & 0xFFFF));
    #if 1
                        item_jpeg_isp_dec_fileName(fileName);
    #else
                        item_jpeg_yuv_fileName(fileName);
    #endif
                        //bFindEnd = true;
                    }
                }
                malloc_stats();
                _Sleep(1000);

                ret = f_findnext(&Hfind);
                if (ret)
                {
                    bReset = bFindEnd = true;
                }
            }
#else
            struct  _finddata_t c_file;
            long                hFile;

            printf("------- start test jpg/isp dir decode\n");

            sprintf(dirPath, "%s/*.*", (char *)parse_option(cfgParam, "JpgDirPath"));
            printf("path: %s\r\n", dirPath);

            while (1) //bFindEnd == false )
            {
                if (bReset == true)
                {
                    hFile  = _findfirst(dirPath, &c_file);
                    if (hFile < 0)
                        _err(" _findfirst(%s) fail  (%d)!! ", dirPath, hFile);
                    bReset = false;
                }

                if (!(c_file.attrib & _A_SUBDIR) )
                {
                    // file case
                    if (_checkFileExtention("*.jpg", c_file.name) ||
                        _checkFileExtention("*.jpeg", c_file.name) )
                    {
                        char fileName[128] = {0};

                        //sprintf(fileName, "%s/%s", (char*)parse_option(cfgParam, "JpgDirPath"), c_file.name);
                        sprintf(fileName, "%s%s", (char *)parse_option(cfgParam, "JpgDirPath"), "test.jpg");
                        //sprintf(outName, "D:/%s", "test_jpg_encode.jpg");
                        Draw_Rect((uint8_t *)_Get_Lcd_Addr_A(), _Get_Lcd_Pitch(), &lcdRect, ((rand() >> 3) & 0xFFFF));

    #if 1
                        item_jpeg_isp_dec_fileName(fileName);
    #else
                        item_jpeg_yuv_fileName(fileName);
    #endif

                        //bFindEnd = true;
                    }
                }

                ret = _findnext(hFile, &c_file);
                if (ret)
                {
                    bReset = bFindEnd = true; _findclose(hFile);
                }
            }
#endif
        }
        else
        {
            uint32_t cnt = 0;
            printf("------- start test jpg/isp decode\n");
            inData[0] = (void *)parse_option(cfgParam, "JpgFileName_00");
            inData[1] = (void *)parse_option(cfgParam, "JpgFileName_01");

            //while( 1 )
            {
                item_jpeg_decoder(inData[cnt & 0x1]);
                cnt++;
                //dbg_heap_dump("");
                _Sleep(1000);
                printf("cnt=%d\n", cnt);
            }
        }
    }

    if (reservedBuf)
        itpVmemFree(reservedBuf);

    return;
}

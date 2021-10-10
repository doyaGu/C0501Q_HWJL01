#include "test_items.h"
#include "test_item_isp.h"
#include "io_api.h"
//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
void
_copy_to_varm(
    unsigned char *sysRam,
    unsigned int  byteSize,
    unsigned char **vramAddr)
{
    unsigned int tmp = 0;

    tmp = _Get_Lcd_Addr_B();

    if (*vramAddr == 0)
        *vramAddr = (unsigned char *)tmp;
    else
        tmp = (unsigned int)(*vramAddr);

    _Vram_WriteBlkMem(tmp, (unsigned int)sysRam, byteSize);
    return;
}

#if 0 // difference isp version
void
_set_isp_frmFunc(
    unsigned char         *uiVramBuf_0,
    unsigned int          uiVramBufPitch_0,
    unsigned char         *uiVramBuf_1,
    unsigned int          uiVramBufPitch_1,
    MMP_ISP_FRM_FUNC_INFO *frmFunInfo)
{
    unsigned int currUiIdx = 0;

    //----------------------------
    // set frame function info
    srand(clock());
    currUiIdx              = (rand() & 0x800) >> 11;

    frmFunInfo->frmFunId   = (int)currUiIdx;
    frmFunInfo->vramAddr   = (currUiIdx) ? (unsigned int)uiVramBuf_1 : (unsigned int)uiVramBuf_0;
    frmFunInfo->startX     = 0; //g_rect.x;
    frmFunInfo->startY     = 0; //g_rect.y;
    frmFunInfo->width      = _Get_Lcd_Width();
    frmFunInfo->height     = _Get_Lcd_Height();
    frmFunInfo->pitch      = (currUiIdx) ? (unsigned int)uiVramBufPitch_1 : (unsigned int)uiVramBufPitch_0;
    frmFunInfo->colorKeyR  = 144; //0xff;
    frmFunInfo->colorKeyG  = 148; //0x0;
    frmFunInfo->colorKeyB  = 176; //0xff;
    frmFunInfo->constAlpha = 0;
    frmFunInfo->format     = MMP_ISP_FFUNC_RGB565;

    // enable ui decompression
    frmFunInfo->bUiDecMode = ISP_FALSE;
    // frmFunInfo->lineBytes  = (currUiIdx) ? lineBytes_1 : lineBytes_0;
    // frmFunInfo->totalBytes = uiTotalBytes[currUiIdx];

    // enable feild frmFunc
    frmFunInfo->bFeildMode   = ISP_FALSE;
    frmFunInfo->vramAddr_2nd = ((currUiIdx - 1) & 0x1) ? (unsigned int)uiVramBuf_1 : (unsigned int)uiVramBuf_0;
    // frmFunInfo->totalBytes_2nd = uiTotalBytes[(currUiIdx-1) & 0x1];
}
#endif

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
item_lcd(
    void)
{
    uint16_t       *addr    = (uint16_t *) _Get_Lcd_Addr_A();
    uint32_t       col      = _Get_Lcd_Pitch() / 2;
    uint32_t       row      = _Get_Lcd_Height();
    uint32_t       x, y, i = 0;
    const uint16_t colors[] = { ITH_RGB565(255, 0, 0), ITH_RGB565(0, 255, 0), ITH_RGB565(0, 0, 255) };

    for (;;)
    {
        uint16_t *base = ithMapVram((uint32_t) addr, _Get_Lcd_Pitch() * _Get_Lcd_Height(), ITH_VRAM_WRITE);
        uint16_t color = colors[i++ % ITH_COUNT_OF(colors)];
        uint16_t *ptr  = base;

        for (y = 0; y < row; y++)
            for (x = 0; x < col; x++)
                *ptr++ = color;

        ithFlushDCacheRange(base, row * col * 2);
        sleep(1);
        printf("go=%d\n", i);
    }
}

void
item_isp_color_trans(
    void            *inInfo,
    DATA_COLOR_TYPE colorType,
    int             rawWidth,
    int             rawHeight)
{
    IO_ERR result    = IO_ERR_OK;
    IO_MGR *pInIoMgr = 0;

#if (HAS_FILE_SYS)
    pInIoMgr = ioA_Create_Stream(IO_FILE_RB, 0);
#else
    pInIoMgr = ioA_Create_Stream(IO_MEM_RB, 0);
#endif
    if (pInIoMgr)
    {
        unsigned char *inBuf   = 0;
        unsigned int  fileSize = 0;
        unsigned char *vramBuf = 0;
        unsigned char *srcAddr_rgby;
        unsigned char *srcAddr_u;
        unsigned char *srcAddr_v;
        CLIP_WND_INFO clipInfo = {0};
        BASE_RECT     srcRect  = {0};

        result = ioA_Stream_Open(pInIoMgr, inInfo);
        if (result != IO_ERR_OK)
            _err(" open fail !!");

        fileSize = pInIoMgr->length;
        inBuf    = malloc(fileSize);
        if (!inBuf)
            _err(" malloc fail (size=%d) !!", fileSize);

        memset(inBuf, 0, fileSize);
        ioA_Stream_FillBuf(pInIoMgr, inBuf, 1, fileSize);
        ioA_Stream_Close(pInIoMgr);

        vramBuf      = 0;
        _copy_to_varm(inBuf, fileSize, &vramBuf);

        srcRect.w    = rawWidth;
        srcRect.h    = rawHeight;

        srcAddr_rgby = vramBuf;
        switch (colorType)
        {
        case DATA_COLOR_YUV444:
            srcAddr_u = srcAddr_rgby + (srcRect.w * srcRect.h);
            srcAddr_v = srcAddr_u + (srcRect.w * srcRect.h);
            break;

        case DATA_COLOR_YUV422:
        case DATA_COLOR_YUV422R:
            srcAddr_u = srcAddr_rgby + (srcRect.w * srcRect.h);
            srcAddr_v = srcAddr_u + ((srcRect.w * srcRect.h) >> 1);
            break;

        case DATA_COLOR_YUV420:
            srcAddr_u = srcAddr_rgby + (srcRect.w * srcRect.h);
            srcAddr_v = srcAddr_u + ((srcRect.w * srcRect.h) >> 2);
            break;

        case DATA_COLOR_NV12:
        case DATA_COLOR_NV21:
            srcAddr_u = srcAddr_rgby + (srcRect.w * srcRect.h);
            break;
        }
        srand(clock());

        clipInfo.bClipEnable  = 0;
        clipInfo.bClipOutside = 0;
        clipInfo.clipWndId    = 0; // ((rand() >> 4) % 3);
        clipInfo.clipRect.w   = ((rand() >> 4) % _Get_Lcd_Width());
        clipInfo.clipRect.h   = ((rand() >> 4) % _Get_Lcd_Height());
        clipInfo.clipRect.w   = (clipInfo.clipRect.w < 64) ? 64 : clipInfo.clipRect.w;
        clipInfo.clipRect.h   = (clipInfo.clipRect.h < 64) ? 64 : clipInfo.clipRect.h;

        clipInfo.clipRect.x   = ((rand() >> 5) % _Get_Lcd_Width());
        clipInfo.clipRect.y   = ((rand() >> 3) % _Get_Lcd_Height());
        if (clipInfo.clipRect.x + clipInfo.clipRect.w >= _Get_Lcd_Width() )
            clipInfo.clipRect.x = (_Get_Lcd_Width() - clipInfo.clipRect.w) - 1;

        if (clipInfo.clipRect.y + clipInfo.clipRect.h >= _Get_Lcd_Height() )
            clipInfo.clipRect.y = (_Get_Lcd_Height() - clipInfo.clipRect.h) - 1;

        test_isp_colorTrans(srcAddr_rgby, srcAddr_u, srcAddr_v, colorType, &clipInfo, &srcRect);

        ioA_Destroy_Stream(pInIoMgr);

        if (inBuf)
            free(inBuf);
    }

    return;
}

void
item_jpeg_decoder(void *inInfo)
{
    IO_ERR result  = IO_ERR_OK;
    IO_MGR *pIoMgr = 0;

#if (JPEG_INTERNAL_FILE_IO)
    pIoMgr = ioA_Create_Stream(IO_FILE_RB, 0);
    ioA_Stream_Open(pIoMgr, inInfo);
    ioA_Stream_Close(pIoMgr);

    test_jpeg_dec_withIsp(0, 0, pIoMgr->filePath);
    ioA_Destroy_Stream(pIoMgr);

#else

    #if (HAS_FILE_SYS)
    pIoMgr = ioA_Create_Stream(IO_FILE_RB, 0);
    #else
    pIoMgr = ioA_Create_Stream(IO_MEM_RB, 0);
    #endif

    if (pIoMgr)
    {
        unsigned char *inBuf   = 0;
        unsigned int  fileSize = 0;
        unsigned char *vramBuf = 0;

        result = ioA_Stream_Open(pIoMgr, inInfo);
        if (result != IO_ERR_OK)
            _err(" open fail !!");

        fileSize = pIoMgr->length;
        inBuf    = malloc(fileSize);
        if (!inBuf)
            _err(" malloc fail !!");

        memset(inBuf, 0, fileSize);
        ioA_Stream_FillBuf(pIoMgr, inBuf, 1, fileSize);
        ioA_Stream_Close(pIoMgr);

        //_copy_to_varm(inBuf, fileSize, &vramBuf);
        vramBuf = inBuf;
        test_jpeg_dec_withIsp(vramBuf, fileSize, 0);

        ioA_Destroy_Stream(pIoMgr);

        if (inBuf)
        {
            free(inBuf);
            inBuf = 0;
        }
    }
#endif
}

void
item_jpeg_isp_dec_fileName(
    char *path)
{
    printf("%s():path= %s \r\n", __FUNCTION__, path);
    test_jpeg_dec_withIsp(0, 0, path);
}

void
item_jpeg_yuv_fileName(
    char *path)
{
    FILE     *fp          = 0;
    uint8_t  *pJpgSrc     = 0;
    uint32_t jpg_filesize = 0;

    if (!(fp = fopen(path, "r")))
    {
        printf("open %s fail !!", path);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    jpg_filesize = ftell(fp);
	jpg_filesize += 5000;  //still don`t know why.
    fseek(fp, 0L, SEEK_SET);
    if (!(pJpgSrc = malloc(jpg_filesize)) )
    {
        printf("malloc src fail !!\n");
        return;
    }
    memset(pJpgSrc, 0x0, jpg_filesize);
    fread(pJpgSrc, 1, jpg_filesize, fp);
    fclose(fp);

    printf("\n============= \n %s():path= %s \r\n", __FUNCTION__, path);

    test_jpeg_dec_to_yuv(pJpgSrc, jpg_filesize, 0);

    if (pJpgSrc)
        free(pJpgSrc);

    return;
}

void
item_jpeg_encoder(
    void                 *inInfo,
    int                  rawWidth,
    int                  rawHeight,
    JPEG_ENC_COLOR_SPACE colorSpace,
    char                 *encName)
{
    IO_ERR result    = IO_ERR_OK;
    IO_MGR *pInIoMgr = 0;

#if (HAS_FILE_SYS)
    pInIoMgr = ioA_Create_Stream(IO_FILE_RB, 0);
#else
    pInIoMgr = ioA_Create_Stream(IO_MEM_RB, 0);
#endif

    printf("%s(): encName = %s \r\n", __FUNCTION__, encName);

    if (pInIoMgr)
    {
        uint32_t fileSize = 0;
        uint32_t curIdx   = 0;
        uint32_t frmSize  = 0;
        uint32_t totalFrm = 0;
        uint8_t  *pInBuf  = 0;

        // ready input yuv420/422
        result = ioA_Stream_Open(pInIoMgr, inInfo);
        if (result != IO_ERR_OK)
            _err(" open fail !!");

        fileSize = pInIoMgr->length;
        switch (colorSpace)
        {
        case JPEG_ENC_YUV_420:
            frmSize = ((3 * rawWidth * rawHeight) >> 1);
            break;
        case JPEG_ENC_YUV_422:
            frmSize = 2 * rawWidth * rawHeight;
            break;
        }

        totalFrm = fileSize / frmSize;
        printf(" fileSize= %d, frmSize = %d, totalFrm = %d\r\n", fileSize, frmSize, totalFrm);

        pInBuf   = malloc(frmSize);
        if (!pInBuf)
            _err(" malloc fail !!");

        // ----------------
        //  action
        curIdx = 0;
        while (curIdx < totalFrm)
        {
            uint8_t *pAddr_y     = 0, *pAddr_u = 0, *pAddr_v = 0;
            char    outName[128] = {0};

            memset(pInBuf, 0, frmSize);
            ioA_Stream_FillBuf(pInIoMgr, pInBuf, 1, frmSize);

            pAddr_y = pInBuf;
            pAddr_u = pInBuf + (rawWidth * rawHeight);
            switch (colorSpace)
            {
            case JPEG_ENC_YUV_420:
                pAddr_v = pAddr_u + ((rawWidth * rawHeight) >> 2);
                break;

            case JPEG_ENC_YUV_422:
                pAddr_v = pAddr_u + ((rawWidth * rawHeight) >> 1);
                break;
            }

            sprintf(outName, "%s.%d.jpg", encName, curIdx);
            test_jpeg_enc(rawWidth, rawHeight, pAddr_y, pAddr_u, pAddr_v, (colorSpace == JPEG_ENC_YUV_422), outName);

            _Sleep(28);

            curIdx++;
            //if( curIdx == totalFrm )
            //if( curIdx > 2 )
            {
                //ioA_Stream_SeekData(pInIoMgr, IO_SEEK_SET, 0);
                //curIdx = 0;
                break;
            }
        }

        ioA_Stream_Close(pInIoMgr);
        ioA_Destroy_Stream(pInIoMgr);

        if (pInBuf)
            free(pInBuf);
    }
    else
        _err(" null pointer !!");
}

void
item_jpeg_codec(
    char *decName,
    char *encName)
{
    printf("%s():decName= %s, encName = %s \r\n", __FUNCTION__, decName, encName);
    test_jpeg_dec_enc(decName, encName);
}

void
item_play_video(
    void *inInfo,
    void *uiInfo_00,
    void *uiInfo_01,
    int  rawWidth,
    int  rawHeight)
{
    IO_ERR result     = IO_ERR_OK;
    IO_MGR *pInIoMgr  = 0;
    IO_MGR *pUiMgr[2] = {0};
    void   *uiInfo[2] = {0};

    uiInfo[0] = uiInfo_00;
    uiInfo[1] = uiInfo_01;

#if (HAS_FILE_SYS)
    pInIoMgr  = ioA_Create_Stream(IO_FILE_RB, 0);
    pUiMgr[0] = ioA_Create_Stream(IO_FILE_RB, 0);
    pUiMgr[1] = ioA_Create_Stream(IO_FILE_RB, 0);
#else
    pInIoMgr  = ioA_Create_Stream(IO_MEM_RB, 0);
    pUiMgr[0] = ioA_Create_Stream(IO_MEM_RB, 0);
    pUiMgr[1] = ioA_Create_Stream(IO_MEM_RB, 0);
#endif
    if (pInIoMgr)
    {
        int           i;
        unsigned int  curIdx             = 0;
        unsigned char *inBuf             = 0;
        unsigned int  fileSize           = 0;
        unsigned char *vramBuf           = 0;
        unsigned char *uiAddr[2]         = {0};
        unsigned int  uiVramBuf_pitch[2] = {0};
        unsigned char *uiVramBuf[2]      = {0};
        unsigned char *uiVramBuf_org[2]  = {0};
        unsigned int  lineBytes[2]       = {0};
        unsigned int  frmSize            = 0;
        unsigned int  totalFrm           = 0;
        unsigned char *vramFrmBuf[2]     = {0};
        unsigned char *srcAddr_y         = 0, *srcAddr_u = 0, *srcAddr_v = 0;
        BASE_RECT     srcRect            = {0};

        // ready input yuv420
        result = ioA_Stream_Open(pInIoMgr, inInfo);
        if (result != IO_ERR_OK)
            _err(" open fail !!");

        fileSize = pInIoMgr->length;
        frmSize  = ((3 * rawWidth * rawHeight) >> 1);
        totalFrm = fileSize / frmSize;
        printf(" fileSize= %d, frmSize = %d, totalFrm = %d\r\n", fileSize, frmSize, totalFrm);

        inBuf    = malloc(frmSize);
        if (!inBuf)
            _err(" malloc fail !!");

        // ready ui data
        for (i = 0; i < 2; i++)
        {
            result = ioA_Stream_Open(pUiMgr[i], uiInfo[i]);
            if (result != IO_ERR_OK)
                _err(" open fail !!");

            fileSize  = pUiMgr[i]->length;

            uiAddr[i] = malloc(fileSize);
            if (!uiAddr[i])
                _err(" malloc fail %d-th !!", i);

            memset(uiAddr[i], 0, fileSize);
            ioA_Stream_FillBuf(pUiMgr[i], uiAddr[i], 1, fileSize);

            uiVramBuf_org[i]   = _Allocat_vram(fileSize);
            uiVramBuf_pitch[i] = _Get_Lcd_Width() * 2;
            vramBuf            = (unsigned char *)uiVramBuf_org[i];
            _copy_to_varm(uiAddr[i], fileSize, &vramBuf);

            // alloc frame buf in vram
            vramFrmBuf[i] = _Allocat_vram(frmSize);
            printf(" vramFrmBuf[%d] = 0x%x\r\n", i, vramFrmBuf[i]);
            if (!vramFrmBuf[i])
                _err(" malloc fail %d-th !!", i);
        }

        //-----------
        // init isp
#if 0   // difference isp version
      //test_isp_ui_frmFunc(ISP_ACT_CMD_INIT, 0, 0, 0, 0, 0, 0, 0, &frmFunInfo);
        test_isp_ui_frmFunc(ISP_ACT_CMD_INIT, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

        // ----------------
        // isp action
        curIdx = 0;
        while (curIdx < totalFrm)
        {
            memset(inBuf, 0, frmSize);
            ioA_Stream_FillBuf(pInIoMgr, inBuf, 1, frmSize);

            vramBuf   = vramFrmBuf[curIdx & 0x1];
            _copy_to_varm(inBuf, frmSize, &vramBuf);

            srcRect.w = rawWidth;
            srcRect.h = rawHeight;

            srcAddr_y = vramBuf;
            srcAddr_u = srcAddr_y + (srcRect.w * srcRect.h);
            srcAddr_v = srcAddr_u + ((srcRect.w * srcRect.h) >> 2);
#if 0       // difference isp version
            {
                MMP_ISP_FRM_FUNC_INFO frmFunInfo = {0};
                _set_isp_frmFunc(
                    uiVramBuf_org[0], uiVramBuf_pitch[0],
                    uiVramBuf_org[1], uiVramBuf_pitch[1],
                    &frmFunInfo);//*/

                test_isp_ui_frmFunc(ISP_ACT_CMD_PROC, uiVramBuf_org[0], uiVramBuf_org[1], srcAddr_y, srcAddr_u, srcAddr_v, srcRect.w, srcRect.h, &frmFunInfo);
                // test_isp_ui_frmFunc(ISP_ACT_CMD_PROC, uiVramBuf_org[0], uiVramBuf_org[1], srcAddr_y, srcAddr_u, srcAddr_v, srcRect.w, srcRect.h, 0);
            }
#endif
            _Sleep(28);

            curIdx++;
            if (curIdx == totalFrm)
            {
                ioA_Stream_SeekData(pInIoMgr, IO_SEEK_SET, 0);
                curIdx = 0;
            }
        }

#if 0   // difference isp version
        test_isp_ui_frmFunc(ISP_ACT_CMD_TERMINATE, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
        ioA_Stream_Close(pInIoMgr);
        ioA_Destroy_Stream(pInIoMgr);

        if (inBuf)
            free(inBuf);
        if (uiAddr[0])
            free(uiAddr[0]);
        if (uiAddr[1])
            free(uiAddr[1]);
        if (uiVramBuf_org[0])
            _Free_vram(uiVramBuf_org[0]);
        if (uiVramBuf_org[1])
            _Free_vram(uiVramBuf_org[1]);
        if (vramFrmBuf[0])
            _Free_vram(vramFrmBuf[0]);
        if (vramFrmBuf[1])
            _Free_vram(vramFrmBuf[0]);
    }
}
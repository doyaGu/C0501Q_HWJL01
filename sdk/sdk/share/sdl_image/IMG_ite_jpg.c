/*
  SDL_image:  An example image loading library for use with SDL
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* This is a generic "format not supported" image framework */

#include <stdio.h>

#include "SDL_image.h"

#if (CFG_JPEG_HW_ENABLE)

#include "jpg/ite_jpg.h"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    #define MASK_32_BITS_R  0x00FF0000
    #define MASK_32_BITS_G  0x0000FF00
    #define MASK_32_BITS_B  0x000000FF
    #define MASK_32_BITS_A  0xFF000000

    #define MASK_24_BITS_R  0x0000FF
    #define MASK_24_BITS_G  0x00FF00
    #define MASK_24_BITS_B  0xFF0000
    #define MASK_24_BITS_A  0x000000

    #define MASK_16_BITS_R  0xF800
    #define MASK_16_BITS_G  0x07E0
    #define MASK_16_BITS_B  0x001F
    #define MASK_16_BITS_A  0x0000
#else
    #define MASK_32_BITS_R  0x0000FF00
    #define MASK_32_BITS_G  0x00FF0000
    #define MASK_32_BITS_B  0xFF000000
    #define MASK_32_BITS_A  0x000000FF

    #define MASK_24_BITS_R  0xFF0000
    #define MASK_24_BITS_G  0x00FF00
    #define MASK_24_BITS_B  0x0000FF
    #define MASK_24_BITS_A  0x000000

    #define MASK_16_BITS_R  0x001F
    #define MASK_16_BITS_G  0x07E0
    #define MASK_16_BITS_B  0xF800
    #define MASK_16_BITS_A  0x0000
#endif

static JPG_ERR
_sdl_File_open(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        SDL_RWops   *pSdlRWops = 0;

        pHJStream->privData[0] = 0;

        // fopen
        switch( pHJStream->jStreamInfo.streamIOType )
        {
            case JPG_STREAM_IO_WRITE:
            case JPG_STREAM_IO_READ:
                pHJStream->privData[0] = (void*)pHJStream->jStreamInfo.jstream.extraData;
                if( pHJStream->privData[0] == 0 )
                    printf(" sdl open jpg fail (0x%x) !!", (char*)pHJStream->jStreamInfo.jstream.extraData);
                break;
        }

        do{
            if( pHJStream->privData[0] == 0 )       break;

            //printf(" pHJStream->privData[0] = 0x%x\n", pHJStream->privData[0]);
            // get total file size
            pHJStream->curBsPos = 0;

            if( pHJStream->jStreamInfo.streamIOType == JPG_STREAM_IO_READ )
            {
                SDL_RWseek((SDL_RWops*)pHJStream->privData[0], 0, RW_SEEK_END);
                pHJStream->streamSize = SDL_RWtell((SDL_RWops*)pHJStream->privData[0]);
                SDL_RWseek((SDL_RWops*)pHJStream->privData[0], 0, RW_SEEK_SET);
            }

        }while(0);
    }

    return result;
}

static JPG_ERR
_sdl_File_close(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        pHJStream->privData[0] = 0;
    }

    return result;
}

static JPG_ERR
_sdl_File_seek(
    JPG_STREAM_HANDLE   *pHJStream,
    int                 offset,
    JPG_SEEK_TYPE       seekType,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            int         rst = 0;
            uint32_t    type = 0;

            switch( seekType )
            {
                case JPG_SEEK_SET:
                    type = RW_SEEK_SET;
                    pHJStream->curBsPos = offset;
                    break;
                case JPG_SEEK_CUR:
                    type = RW_SEEK_CUR;
                    pHJStream->curBsPos += offset;
                    break;
                case JPG_SEEK_END:
                    type = RW_SEEK_END;
                    pHJStream->curBsPos = (pHJStream->streamSize - pHJStream->curBsPos);
                    break;
            }

            rst = SDL_RWseek((SDL_RWops*)pHJStream->privData[0], offset, type);
            if( rst )   printf(" seek fail !! %s[%d]\n", __FILE__, __LINE__);

            if( pHJStream->curBsPos > pHJStream->streamSize )
                pHJStream->curBsPos = pHJStream->streamSize;

            if( (int)pHJStream->curBsPos < 0 )
                pHJStream->curBsPos = 0;
        }
    }

    return result;
}

static JPG_ERR
_sdl_File_tell(
    JPG_STREAM_HANDLE   *pHJStream,
    uint32_t            *pCurPos,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            uint32_t    curr = 0;
            curr = SDL_RWtell((SDL_RWops*)pHJStream->privData[0]);
            if( pCurPos )       *pCurPos = curr;
        }
    }

    return result;
}

static JPG_ERR
_sdl_File_read(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *srcBuf,
    uint32_t            requestSize,
    uint32_t            *realSize,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            SDL_RWops   *pSdlRWops = (SDL_RWops*)pHJStream->privData[0];
            uint32_t    size = 0;

            size = SDL_RWread(pSdlRWops, srcBuf, 1, requestSize);
            if( realSize )      *realSize = size;
            pHJStream->curBsPos += size;
        }
    }

    return result;
}

static JPG_ERR
_sdl_File_write(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *destBuf,
    uint32_t            length,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            SDL_RWops   *pSdlRWops = (SDL_RWops*)pHJStream->privData[0];
            uint32_t    size = 0;

            size = SDL_RWwrite(pSdlRWops, destBuf, 1, length);
            pHJStream->curBsPos += size;
        }
    }

    return result;
}

static int
_sdl_reset_jstream_info(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *extraData)
{
    if( pHJStream )
    {
        switch (pHJStream->jStreamInfo.streamType )
        {
            case JPG_STREAM_FILE:
                pHJStream->jStreamDesc.jOpen_stream  = _sdl_File_open;
                pHJStream->jStreamDesc.jClose_stream = _sdl_File_close;
                pHJStream->jStreamDesc.jSeek_stream  = _sdl_File_seek;
                pHJStream->jStreamDesc.jTell_stream  = _sdl_File_tell;
                pHJStream->jStreamDesc.jFull_buf     = _sdl_File_read;
                pHJStream->jStreamDesc.jOut_buf      = _sdl_File_write;
                break;

            case JPG_STREAM_MEM:
                break;
        }
    }
    return 0;
}

/* See if an image is contained in a data source */
int IMG_is_Ite_Jpg(SDL_RWops *src)
{
    /* This detection code is by Steaphan Greene <stea@cs.binghamton.edu> */
    /* Blame me, not Sam, if this doesn't work right. */
    /* And don't forget to report the problem to the the sdl list too! */

    int         start;
    int         is_JPG;
    uint8_t     magic[4];
    do{
        if( !src )      break;

        start = SDL_RWtell(src);
        is_JPG = 0;

        if( !SDL_RWread(src, magic, 2, 1) )     break;

        if( (magic[0] != 0xFF) || (magic[1] != 0xD8) )  break;

        SDL_RWseek(src, -2, RW_SEEK_END);

        if( !SDL_RWread(src, magic, 2, 1) )     break;

        if( (magic[0] != 0xFF) || (magic[1] != 0xD9) )  break;

        is_JPG = 1;

    }while(0);

    SDL_RWseek(src, start, RW_SEEK_SET);
    return(is_JPG);
}

/* Load a XXX type image from an SDL datasource */
SDL_Surface *IMG_Load_Ite_Jpg(SDL_RWops *src)
{
#define SIMULATION_BRANCH   1

    JPG_ERR             result = JPG_ERR_OK;
    int                 start;
    SDL_Surface         *surface = NULL;
    HJPG                *pHJpeg = 0;
    JPG_USER_INFO       jpgUserInfo = {0};
    
    bool        bArgb8888 = true;
    uint8_t     *pArgb8888Buf = 0;

    do{
        JPG_INIT_PARAM      initParam = {0};
        JPG_STREAM_INFO     inStreamInfo = {0};
        JPG_STREAM_INFO     outStreamInfo = {0};
        JPG_BUF_INFO        entropyBufInfo = {0};

        if( !src )      break;

        start = SDL_RWtell(src);

        /* Load the image here */
        initParam.codecType     = JPG_CODEC_DEC_JPG;
        initParam.decType       = JPG_DEC_PRIMARY;
        initParam.outColorSpace = (bArgb8888) ? JPG_COLOR_SPACE_ARGB8888 : JPG_COLOR_SPACE_RGB565;
        initParam.width         = 0;
        initParam.height        = 0;
        initParam.dispMode      = JPG_DISP_FIT;

        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        // set input info
        inStreamInfo.streamType        = JPG_STREAM_FILE;
        inStreamInfo.jstream.extraData = (void*)src;

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.jpg_reset_stream_info =  _sdl_reset_jstream_info;

        result = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        if( result != JPG_ERR_OK )
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            break;
        }

        iteJpg_Parsing(pHJpeg, &entropyBufInfo, 0);

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
                    jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
                    jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
                    initParam.dispMode);

#if (SIMULATION_BRANCH)
        pArgb8888Buf = (uint8_t*)malloc(4*jpgUserInfo.jpgRect.w*jpgUserInfo.jpgRect.h);
        if( !pArgb8888Buf )
        {
            printf("malloc fail ! %s[%d]\n", __FILE__, __LINE__);
            break;
        }
        memset(pArgb8888Buf, 0xaa, 4*jpgUserInfo.jpgRect.w*jpgUserInfo.jpgRect.h);
        // set output info
        outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType         = JPG_STREAM_MEM;
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pArgb8888Buf;
        outStreamInfo.jstream.mem[0].pitch  = 4*jpgUserInfo.jpgRect.w;
        outStreamInfo.jstream.mem[0].length = outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
        outStreamInfo.validCompCnt = 1;

        //printf("pArgb8888Buf=0x%x\n", pArgb8888Buf);
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		                             jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h, 
                                       24,
                                       MASK_24_BITS_R, MASK_24_BITS_G, MASK_24_BITS_B, 0);
        if( surface == NULL )
        {
            SDL_RWseek(src, start, RW_SEEK_SET);
            IMG_SetError("create surface fail ! %s[%d]\n", __FILE__, __LINE__);
            break;
        }
#else
        // Allocate an output surface to hold the image
        switch( initParam.outColorSpace )
        {
            case JPG_COLOR_SPACE_ARGB8888:
                surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                               jpgUserInfo.jpgRect.w,
                                               jpgUserInfo.jpgRect.h,
                                               32, 
                                               MASK_32_BITS_R, MASK_32_BITS_G, MASK_32_BITS_B, MASK_32_BITS_A);
                break;

            case JPG_COLOR_SPACE_RGB565:
                surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                               jpgUserInfo.jpgRect.w,
                                               jpgUserInfo.jpgRect.h,
                                               16, 
                                               MASK_16_BITS_R, MASK_16_BITS_G, MASK_16_BITS_B, MASK_16_BITS_A);
                break;
        }

        if( surface == NULL )
        {
            SDL_RWseek(src, start, RW_SEEK_SET);
            IMG_SetError("create surface fail ! %s[%d]\n", __FILE__, __LINE__);
            break;
        }

        // set output info
        outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType         = JPG_STREAM_MEM;
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)surface->pixels;
        outStreamInfo.jstream.mem[0].pitch  = surface->pitch;
        outStreamInfo.jstream.mem[0].length = surface->pitch * surface->h;
        outStreamInfo.validCompCnt = 1;
#endif
        //printf(" ----- addr =0x%x, pitch=%d, height=%d\n", surface->pixels, surface->pitch, surface->h);
        result = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
        if( result != JPG_ERR_OK )
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            break;
        }

        result = iteJpg_Setup(pHJpeg, 0);
        if( result != JPG_ERR_OK )
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            break;
        }

        result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
        if( result != JPG_ERR_OK )
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            break;
        }

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("\n\tresult = %d\n", jpgUserInfo.status);
    }while(0);

    iteJpg_DestroyHandle(&pHJpeg, 0);
    if( result != JPG_ERR_OK )
    {
        SDL_RWseek(src, start, RW_SEEK_SET);
        if( surface )
        {
            SDL_FreeSurface(surface);
            surface = NULL;
        }
    }
#if (SIMULATION_BRANCH)
    else if( surface != NULL )
    {
        //------------------------------
        // RGB565 not ready, so use RGB 24 bits
        int     x, y;
        uint8_t *pCur = (uint8_t*)surface->pixels;

        // isp output GBAR
        for(y = 0; y < surface->h; y++)
            for(x = 0; x < surface->w; x++)
            {
                *pCur++ = pArgb8888Buf[y * 4 * surface->w + x * 4 + 2];
                *pCur++ = pArgb8888Buf[y * 4 * surface->w + x * 4 + 0];
                *pCur++ = pArgb8888Buf[y * 4 * surface->w + x * 4 + 4];
            }  
    }

    if( pArgb8888Buf )      free(pArgb8888Buf);
#endif

    return surface;
}

#else

/* See if an image is contained in a data source */
int IMG_is_Ite_Jpg(SDL_RWops *src)
{
    return(0);
}

/* Load a XXX type image from an SDL datasource */
SDL_Surface *IMG_Load_Ite_Jpg(SDL_RWops *src)
{
    return(NULL);
}

#endif /* LOAD_XXX */

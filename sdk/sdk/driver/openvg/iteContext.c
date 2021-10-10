/*
 * Copyright (c) 2007 Ivan Leben
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file COPYING;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define VG_API_EXPORT
#include "egl.h"
#include "openvg.h"
#include "iteContext.h"
#include <string.h>
#include <stdio.h>
#include "iteHardware.h"
#include "iteImage.h"
#include "iteUtility.h"
#include "ite/itv.h"

/* Array for Path command array */
#define _ITEM_T  ITEPathCommandArray *
#define _ARRAY_T ITEPathCommandPool
#define _FUNC_T  itePathCommandPool
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

#ifdef _WIN32
/* Array for Path command array */
    #define _ITEM_T  ITEuint8 *
    #define _ARRAY_T ITEPathCommandPoolHW
    #define _FUNC_T  itePathCommandPoolHW
    #define _ARRAY_DEFINE
    #include "iteArrayBase.h"
#endif

/*-----------------------------------------------------
 * Simple functions to create a VG context instance
 * on top of an existing OpenGL context.
 * TODO: There is no mechanics yet to asure the OpenGL
 * context exists and to choose which context / window
 * to bind to.
 *-----------------------------------------------------*/

static VGContext *g_context = NULL;

/*-----------------------------------------------------
 * VGContext constructor
 *-----------------------------------------------------*/
static void VGContext_ctor(VGContext *c)
{
    ITEint i;

    /* GetString info */
    strncpy(c->vendor, "ITE", sizeof(c->vendor));
    strncpy(c->renderer, "ITEVG 0.1.0", sizeof(c->renderer));
    strncpy(c->version, "1.1", sizeof(c->version));
    strncpy(c->extensions, "", sizeof(c->extensions));

    /* Mode settings */
    c->matrixMode       = VG_MATRIX_PATH_USER_TO_SURFACE;
    c->fillRule         = VG_EVEN_ODD;
    c->imageQuality     = VG_IMAGE_QUALITY_FASTER;
    //  c->renderingQuality = VG_RENDERING_QUALITY_NONANTIALIASED;
    //  c->renderingQuality = VG_RENDERING_QUALITY_FASTER;
    c->renderingQuality = VG_RENDERING_QUALITY_BETTER;
    c->blendMode        = VG_BLEND_SRC_OVER;
    c->imageMode        = VG_DRAW_IMAGE_NORMAL;
    c->tilingMode       = VG_TILE_FILL;
    //c->paintMode = VG_FILL_PATH;

    /* Scissor rectangles */
    ITE_INITOBJ(ITERectArray, c->scissor);
    c->scissoring          = VG_FALSE;
    c->masking             = VG_FALSE;
    c->scissorImage.width  = 0;
    c->scissorImage.height = 0;
    c->scissorImage.pitch  = 0;
    c->scissorImage.data   = NULL;

    /* Stroke parameters */
    c->strokeLineWidth     = 1.0f;
    c->strokeCapStyle      = VG_CAP_BUTT;
    c->strokeJoinStyle     = VG_JOIN_MITER;
    //  c->strokeJoinStyle = VG_JOIN_ROUND;
    //  c->strokeJoinStyle = VG_JOIN_BEVEL;
    //c->strokeMiterLimit = 100.0f;
    c->strokeMiterLimit_input = 4.0f;
    c->strokeMiterLimit       = 4.0f;
    c->strokeDashPhase        = 0.0f;
    c->strokeDashPhaseReset   = VG_FALSE;
    ITE_INITOBJ(ITEFloatArray, c->strokeDashPattern);

    /* Edge fill color for vgConvolve and pattern paint */
    CSET(c->tileFillColor_input, 0.0f, 0.0f, 0.0f, 0.0f);
    CSET(c->tileFillColor, 0, 0, 0, 0);

    /* Color for vgClear */
    CSET(c->clearColor_input, 0.0f, 0.0f, 0.0f, 0.0f);
    CSET(c->clearColor, 0, 0, 0, 0);

    /* Color components layout inside pixel */
    c->pixelLayout               = VG_PIXEL_LAYOUT_UNKNOWN;

    /* Source format for image filters */
    c->filterFormatLinear        = VG_FALSE;
    c->filterFormatPremultiplied = VG_FALSE;
    c->filterChannelMask         = VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA;

    /* Matrices */
    ITE_INITOBJ(ITEMatrix3x3, c->pathTransform);
    ITE_INITOBJ(ITEMatrix3x3, c->imageTransform);
    ITE_INITOBJ(ITEMatrix3x3, c->fillTransform);
    ITE_INITOBJ(ITEMatrix3x3, c->strokeTransform);
    ITE_INITOBJ(ITEMatrix3x3, c->glyphTransform);

    /* Paints */
    c->fillPaint   = NULL;
    c->strokePaint = NULL;
    ITE_INITOBJ(ITEPaint, c->defaultPaint);

    /* Color Transform */
    c->enColorTransform = VG_FALSE;
    for (i = 0; i < 8; i++)
    {
        c->colorTransform[i] = (i < 4) ? 1.0f : 0.0f;
    }

    /* Error */
    c->error = VG_NO_ERROR;

    /* Resources */
    ITE_INITOBJ(ITEPathArray, c->paths);
    ITE_INITOBJ(ITEPaintArray, c->paints);
    ITE_INITOBJ(ITEImageArray, c->images);
    ITE_INITOBJ(ITEFontArray, c->fonts);
    ITE_INITOBJ(ITEImageArray, c->maskLayers);

    /* Surface */

    /* update flag */
    VG_Memset(&c->updateFlag, 0, sizeof(ITEUpdateFlag));
    c->updateFlag.paintBaseFlag = VG_TRUE;

    /* Print number */
    c->printnum                 = 0;

    /* Glyph */
    c->glyphOrigin_input[0]     = 0.0f;
    c->glyphOrigin_input[1]     = 0.0f;
    c->glyphOrigin[0]           = 0.0f;
    c->glyphOrigin[1]           = 0.0f;
}

/*-----------------------------------------------------
 * VGContext constructor
 *-----------------------------------------------------*/

static void VGContext_dtor(VGContext *c)
{
    int i;

    ITE_DEINITOBJ(ITERectArray, c->scissor);
    ITE_DEINITOBJ(ITEFloatArray, c->strokeDashPattern);

    /* Destroy resources */
    for (i = 0; i < c->paths.size; ++i)
    {
        ITE_DELETEOBJ(ITEPath, c->paths.items[i]);
    }
    ITE_DEINITOBJ(ITEPathArray, c->paths);

    for (i = 0; i < c->paints.size; ++i)
    {
        ITE_DELETEOBJ(ITEPaint, c->paints.items[i]);
    }
    ITE_DEINITOBJ(ITEPaintArray, c->paints);

    for (i = 0; i < c->images.size; ++i)
    {
        //ITE_DELETEOBJ(ITEImage, c->images.items[i]);
    }
    ITE_DEINITOBJ(ITEImageArray, c->images);

    for (i = 0; i < c->fonts.size; ++i)
    {
        ITE_DELETEOBJ(ITEFont, c->fonts.items[i]);
    }
    ITE_DEINITOBJ(ITEFontArray, c->fonts);

    for (i = 0; i < c->images.size; ++i)
    {
        //ITE_DELETEOBJ(ITEImage, c->maskLayers.items[i]);
    }
    ITE_DEINITOBJ(ITEImageArray, c->maskLayers);
}

VGboolean
iteCreateContext()
{
    /* return if already created */
    if (g_context)
    {
        return VG_TRUE;
    }

    /* create new context */
    ITE_NEWOBJ(VGContext, g_context);
    if (!g_context)
    {
        return VG_FALSE;
    }

    iteCreateHardware(&g_context->hardware);

    /*
       g_context->hardware = iteCreateHardware();
       if(!g_context->hardware)
       {
            iteDestroyContext();
            return VG_FALSE;
       }
     */

    return VG_TRUE;
}

ITESurface *
iteContextAllocSurface(
    void *OSWindowContext,
    int renderBuffer,
    VGImageFormat surfaceFormat,
    int windowWidth,
    int windowHeight,
    ITEboolean allocMask)
{
    ITESurface *pSurface     = NULL;
    VGbitfield imageQualilty = VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER;

    pSurface = (ITESurface *)malloc(sizeof(ITESurface));
    if (pSurface)
    {
        ITEColor color = {0};

        VG_Memset(pSurface, 0x00, sizeof(ITESurface));

        pSurface->OSWindowContext = OSWindowContext;
        pSurface->renderBuffer    = renderBuffer;

        /* Allocate/Set color image */
        pSurface->colorImage      = (ITEImage *)vgCreateImage(surfaceFormat, windowWidth, windowHeight, imageQualilty);
        if (pSurface->colorImage == NULL)
        {
            goto clean;
        }
        pSurface->colorImage->data = (void *)ithLcdGetBaseAddrA();

        CSET(color, 0, 0, 0, 0);
        iteSetImage(ITE_FALSE, pSurface->colorImage, 0, 0, windowWidth, windowHeight, color);

        /* Allocate/Set mask image */
        if (allocMask)
        {
            pSurface->maskImage = (ITEImage *)vgCreateImage(VG_A_8, windowWidth, windowHeight, imageQualilty);
            if (pSurface->maskImage == NULL)
            {
                goto clean;
            }

            pSurface->maskImage->objType = ITE_VG_OBJ_MASK;
            CSET(color, 255, 255, 255, 255);
            iteSetImage(ITE_FALSE, pSurface->maskImage, 0, 0, windowWidth, windowHeight, color);
        }

        /* Create coverage image for hardware */
        pSurface->coverageImageA = iteCreateImage(VG_sRGB_565, windowWidth, windowHeight, 0, VG_TRUE, VG_TRUE);
        pSurface->coverageImageB = iteCreateImage(VG_sRGB_565, windowWidth, windowHeight, 0, VG_TRUE, VG_TRUE);
        if (pSurface->coverageImageA == NULL
            || pSurface->coverageImageB == NULL)
        {
            goto clean;
        }

        pSurface->coverageIndex = 0;

        /* Create valid image for hardware */
        pSurface->validImageA   = iteCreateImage(VG_A_1, windowWidth, windowHeight, 0, VG_TRUE, VG_TRUE);
        pSurface->validImageB   = iteCreateImage(VG_A_1, windowWidth, windowHeight, 0, VG_TRUE, VG_TRUE);
        if (pSurface->validImageA == NULL
            || pSurface->validImageB == NULL)
        {
            goto clean;
        }

        return pSurface;
    }

clean:
    if (pSurface->colorImage)
    {
        vgDestroyImage((VGImage)pSurface->colorImage);
        pSurface->colorImage = NULL;
    }

    if (pSurface->maskImage)
    {
        vgDestroyImage((VGImage)pSurface->colorImage);
        pSurface->maskImage = NULL;
    }

    if (pSurface->coverageImageA)
    {
        vgDestroyImage((VGImage)pSurface->coverageImageA);
        pSurface->coverageImageA = NULL;
    }

    if (pSurface->coverageImageB)
    {
        vgDestroyImage((VGImage)pSurface->coverageImageB);
        pSurface->coverageImageB = NULL;
    }

    if (pSurface->validImageA)
    {
        vgDestroyImage((VGImage)pSurface->validImageA);
        pSurface->validImageA = NULL;
    }

    if (pSurface->validImageB)
    {
        vgDestroyImage((VGImage)pSurface->validImageB);
        pSurface->validImageB = NULL;
    }

    if (pSurface)
    {
        free((ITESurface *)pSurface);
    }

    return EGL_NO_SURFACE;
}

void
iteDestroyContext(
    VGContext *context)
{
    /* return if already released */
    if (!g_context)
    {
        return;
    }

    /* delete context object */
    ITE_DELETEOBJ(VGContext, g_context);
    g_context = NULL;
    iteDestroyHardware();
}

VGContext *iteGetContext()
{
    ITE_ASSERT(g_context);
    return g_context;
}

/*--------------------------------------------------
 * Tries to find resources in this context
 *--------------------------------------------------*/

ITEint iteIsCurrentRenderTarget(VGContext *c, VGImage image)
{
    return ((ITEImage *)image == c->surface->colorImage || (ITEImage *)image == c->surface->maskImage);
}

ITEint iteIsValidPath(VGContext *c, VGHandle h)
{
    int index = itePathArrayFind(&c->paths, (ITEPath *)h);
    return (index == -1) ? 0 : 1;
}

ITEint iteIsValidPaint(VGContext *c, VGHandle h)
{
    int index = itePaintArrayFind(&c->paints, (ITEPaint *)h);
    return (index == -1) ? 0 : 1;
}

ITEint iteIsValidImage(VGContext *c, VGHandle h)
{
    int index = iteImageArrayFind(&c->images, (ITEImage *)h);
    return (index == -1) ? 0 : 1;
}

ITEint iteIsValidFont(VGContext *c, VGHandle h)
{
    int index = iteFontArrayFind(&c->fonts, (ITEFont *)h);
    return (index == -1) ? 0 : 1;
}

ITEint iteIsValidMaskLayer(VGContext *c, VGHandle h)
{
    int index = iteImageArrayFind(&c->maskLayers, (ITEImage *)h);
    return (index == -1) ? 0 : 1;
}

/*--------------------------------------------------
 * Tries to find a resources in this context and
 * return its type or invalid flag.
 *--------------------------------------------------*/

ITEResourceType iteGetResourceType(VGContext *c, VGHandle h)
{
    if (iteIsValidPath(c, h))
    {
        return ITE_RESOURCE_PATH;
    }
    else if (iteIsValidPaint(c, h))
    {
        return ITE_RESOURCE_PAINT;
    }
    else if (iteIsValidImage(c, h))
    {
        return ITE_RESOURCE_IMAGE;
    }
    else if (iteIsValidFont(c, h))
    {
        return ITE_RESOURCE_FONT;
    }
    else
    {
        return ITE_RESOURCE_INVALID;
    }
}

/*-----------------------------------------------------
 * Sets the specified error on the given context if
 * there is no pending error yet
 *-----------------------------------------------------*/

void iteSetError(VGContext *c, VGErrorCode e)
{
    if (c->error == VG_NO_ERROR)
    {
        c->error = e;
    }
}

/*-----------------------------------------------------------
 * Returns the matrix currently selected via VG_MATRIX_MODE
 *-----------------------------------------------------------*/
static ITEMatrix3x3 *iteCurrentMatrix(VGContext *c)
{
    switch (c->matrixMode)
    {
    case VG_MATRIX_PATH_USER_TO_SURFACE:  return &c->pathTransform;
    case VG_MATRIX_IMAGE_USER_TO_SURFACE: return &c->imageTransform;
    case VG_MATRIX_FILL_PAINT_TO_USER:    return &c->fillTransform;
    default:                              return &c->strokeTransform;
    }
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*--------------------------------------------------
 * Returns the oldest error pending on the current
 * context and clears its error code
 *--------------------------------------------------*/

VG_API_CALL VGErrorCode vgGetError(void)
{
    VGErrorCode error;
    VG_GETCONTEXT(VG_NO_CONTEXT_ERROR);
    error          = context->error;
    context->error = VG_NO_ERROR;
    VG_RETURN(error);
}

VG_API_CALL void vgFlush(void)
{
    ITEuint32 frameID = 0;
    ITEint    index   = 0;
    VG_GETCONTEXT(VG_NO_RETVAL);

    frameID = iteHardwareGenFrameID();
    iteHardwareNullFire(frameID);
    iteHardwareWaitFrameID(frameID);

#if 0
    /* Delete image which deleteCount > 0 */
    for (index = 0; index < context->images.size; index++)
    {
        ITEImage *pImage = iteImageArrayAt(&context->images, index);
        if (pImage && pImage->deleteCount)
        {
            iteImageArrayRemoveAt(&context->images, index);

            /* Delete object if none reference to it*/
            if (ITEImage_RemoveReference(pImage) == 0)
            {
                ITE_DELETEOBJ(ITEImage, pImage);
            }

            index--;
        }
    }

    /* Decrement drawCount of image which in list */
    for (index = 0; index < context->images.size; index++)
    {
        ITEImage *pImage = iteImageArrayAt(&context->images, index);
        if (pImage)
        {
            pImage->drawCount = 0;
        }
    }
#endif

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgFinish(void)
{
    VG_GETCONTEXT(VG_NO_RETVAL);
    //we doesn't cache anything, so this is a no-op
    VG_RETURN(VG_NO_RETVAL);
}

/*--------------------------------------
 * Sets the current matrix to identity
 *--------------------------------------*/

VG_API_CALL void vgLoadIdentity(void)
{
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    m = iteCurrentMatrix(context);
    IDMAT((*m));

    VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------
 * Loads values into the current matrix from the given array.
 * Matrix affinity is preserved if an affine matrix is loaded.
 *-------------------------------------------------------------*/

VG_API_CALL void vgLoadMatrix(const VGfloat *mm)
{
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    VG_RETURN_ERR_IF(!mm || !CheckAlignment(mm, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

    m = iteCurrentMatrix(context);

    if (context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
    {
        SETMAT((*m),
               mm[0], mm[3], mm[6],
               mm[1], mm[4], mm[7],
               mm[2], mm[5], mm[8]);
    }
    else
    {
        SETMAT((*m),
               mm[0], mm[3], mm[6],
               mm[1], mm[4], mm[7],
               0.0f,  0.0f,  1.0f);
    }

    VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------------
 * Outputs the values of the current matrix into the given array
 *---------------------------------------------------------------*/

VG_API_CALL void vgGetMatrix(VGfloat *mm)
{
    ITEMatrix3x3 *m;
    int          i, j, k = 0;
    VG_GETCONTEXT(VG_NO_RETVAL);

    VG_RETURN_ERR_IF(!mm || !CheckAlignment(mm, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

    m = iteCurrentMatrix(context);

    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
        {
            mm[k++] = m->m[j][i];
        }
    }

    VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------
 * Right-multiplies the current matrix with the one specified
 * in the given array. Matrix affinity is preserved if an
 * affine matrix is begin multiplied.
 *-------------------------------------------------------------*/

VG_API_CALL void vgMultMatrix(const VGfloat *mm)
{
    ITEMatrix3x3 *m, mul, temp;
    VG_GETCONTEXT(VG_NO_RETVAL);

    VG_RETURN_ERR_IF(!mm || !CheckAlignment(mm, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

    m = iteCurrentMatrix(context);

    if (context->matrixMode == VG_MATRIX_IMAGE_USER_TO_SURFACE)
    {
        SETMAT(mul,
               mm[0], mm[3], mm[6],
               mm[1], mm[4], mm[7],
               mm[2], mm[5], mm[8]);
    }
    else
    {
        SETMAT(mul,
               mm[0], mm[3], mm[6],
               mm[1], mm[4], mm[7],
               0.0f,  0.0f,  1.0f);
    }

    MULMATMAT((*m), mul, temp);
    SETMATMAT((*m), temp);

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgTranslate(VGfloat tx, VGfloat ty)
{
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    m = iteCurrentMatrix(context);
    TRANSLATEMATR((*m), tx, ty);

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgScale(VGfloat sx, VGfloat sy)
{
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    m = iteCurrentMatrix(context);
    SCALEMATR((*m), sx, sy);

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgShear(VGfloat shx, VGfloat shy)
{
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    m = iteCurrentMatrix(context);
    ITEEARMATR((*m), shx, shy);

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgRotate(VGfloat angle)
{
    ITEfloat     a;
    ITEMatrix3x3 *m;
    VG_GETCONTEXT(VG_NO_RETVAL);

    a = ITE_DEG2RAD(angle);
    m = iteCurrentMatrix(context);
    ROTATEMATR((*m), a);

    VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL VGHardwareQueryResult
vgHardwareQuery(
    VGHardwareQueryType key,
    VGint setting)
{
    VG_GETCONTEXT(VG_HARDWARE_UNACCELERATED);

    /* VG_ILLEGAL_ARGUMENT_ERROR
            �V if key is not one of the values from the VGHardwareQueryType enumeration
            �V if setting is not one of the values from the enumeration associated with key*/
    VG_RETURN_ERR_IF(key != VG_IMAGE_FORMAT_QUERY && key != VG_PATH_DATATYPE_QUERY,
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_HARDWARE_UNACCELERATED);
    VG_RETURN_ERR_IF(key == VG_IMAGE_FORMAT_QUERY && !iteIsValidImageFormat(setting),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_HARDWARE_UNACCELERATED);
    VG_RETURN_ERR_IF(key == VG_PATH_DATATYPE_QUERY && (setting < VG_PATH_DATATYPE_S_8 || setting > VG_PATH_DATATYPE_F),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_HARDWARE_UNACCELERATED);
    return VG_HARDWARE_ACCELERATED;
}
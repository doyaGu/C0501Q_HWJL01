/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Used as implementatoin template file.
 *
 * @author James Lin
 * @version 0.1
 */
#define VG_API_EXPORT
#include "openvg.h"
#include "iteHardware.h"
#include "iteImage.h"
#include "iteVectors.h"
#include "iteGeometry.h"
#include "iteUtility.h"
#include "vgmem.h"
#include "iteDefs.h"


void
iteM2dAlphaBlend(
    ITEImage*         destSurface,
    ITEint			  destX,
    ITEint			  destY,
    ITEuint		      destWidth,
    ITEuint		      destHeight,
    ITEImage*         srcSurface,
    ITEint			  srcX,
    ITEint			  srcY,
    VGBlendMode       blendMode,
    ITEint			  srcConstAlpha)
{
	ITEHardwareRegister        hwReg				 = {0};
	ITEboolean			       srcPreMultiFormat	 = ITE_FALSE;
	ITEboolean			       dstPreMultiFormat	 = ITE_FALSE;
	ITEboolean			       EnPreMultiForTexture  = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean			       EnUnpreMultiForDst	 = ITE_FALSE; // Reg 0x0AC, Bit 31
	ITEboolean			       EnPreMultiForBlend	 = ITE_FALSE; // Reg 0x0AC, Bit 29
	ITEboolean			       EnUnpreMultiForCT	 = ITE_FALSE; // Reg 0x0AC, Bit 30
	HWMatrix3x3 		       transMat			     = {0};
	ITEboolean			       enBlend 			     = ITE_TRUE;
	HWImageFormat              destImageFormat       = HW_sRGB_565;
	HWImageFormat              srcImageFormat        = HW_sRGB_565;
	ITEMatrix3x3               rotateMat             = {0};
	ITEMatrix3x3               rotateinverseMat      = {0};
    HWMatrix3x3                transinverseMat       = {0};

    VG_GETCONTEXT(VG_NO_RETVAL);

	ITE_INITOBJ(ITEHardware, hwReg);
	
	SETMAT( transMat, 
        	0x10000, 0,       (ITEs15p16)((srcX - destX) << 16), 
        	0,       0x10000, (ITEs15p16)((srcY - destY) << 16), 
        	0,       0,       0x10000);
	
	SETMAT(rotateMat, 
			1.0f,    0, 	  0, 
			0,		 1.0f,    0, 
			0,		 0, 	  1.0f);

	
	iteInvertMatrix(&rotateMat,&rotateinverseMat);
	
	MULMATMAT(rotateinverseMat, transMat, transinverseMat);

	srcPreMultiFormat = ITE_FALSE;

	dstPreMultiFormat = ITE_FALSE;

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING |
	                     ITE_VG_CMD_FULLRDY_EN |
	                     ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
						 (EnPreMultiForTexture ? ITE_VG_CMD_TEX_PRE_EN : 0) |
						 ITE_VG_CMD_DESTINATION_EN |
						 (enBlend ? ITE_VG_CMD_BLEND_EN : 0)|
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
						 ITE_VG_CMD_RENDERMODE_1;
	
	// Render Mode Register
	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
						 (((ITEuint32)blendMode & 0x1F) << ITE_VG_CMDSHIFT_BLENDMODE) |						 
						 ITE_VG_CMD_MASKMODE_INTERSECT; 

    destImageFormat = destSurface->vgformat;
    srcImageFormat = srcSurface->vgformat;

	hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;

	hwReg.REG_CAR_BASE = 0xff | (srcConstAlpha << 8) | (srcConstAlpha << 16) | (srcConstAlpha << 24);  
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
						 ((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)destSurface->height<< ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
						  ((ITEint16)destSurface->width& ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(destSurface->data)& ITE_VG_CMDMASK_SRCBASE;

	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)srcSurface->pitch<< ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
						  (destSurface->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)0 << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
						 ((ITEs12p3)0 & ITE_VG_CMDMASK_DSTX);
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)srcSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
						  ((ITEint16)srcSurface->width& ITE_VG_CMDMASK_DSTWIDTH);
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(srcSurface->data)& ITE_VG_CMDMASK_SRCBASE;

	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	hwReg.REG_PITR00_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR01_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR02_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR10_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR11_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR12_BASE = (ITEs15p16)(0 * 0x10000);

	/* Set image object ID */
	iteHardwareGenObjectID();
	hwReg.REG_BID2_BASE = iteHardwareGetCurrentObjectID();
	destSurface->objectID= hwReg.REG_BID2_BASE;
	srcSurface->objectID= hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);

}


void
iteM2dSourceCopy(
    ITEImage*        destSurface,
    ITEint		     destX,
    ITEint		     destY,
    ITEuint			 destWidth,
    ITEuint			 destHeight,
    ITEImage*        srcSurface,
    ITEint		     srcX,
    ITEint			 srcY)
{
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enScissor            = ITE_FALSE;
	HWImageFormat       destImageFormat      = HW_sRGB_565;
	HWImageFormat       srcImageFormat       = HW_sRGB_565;
	ITEMatrix3x3        rotateMat            = {0};
	ITEMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3         transinverseMat      = {0};
	

    VG_GETCONTEXT(VG_NO_RETVAL);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		0x10000, 0,       (ITEs15p16)((srcX - destX) << 16), 
		0,       0x10000, (ITEs15p16)((srcY - destY) << 16), 
		0,       0,       0x10000);

	SETMAT(rotateMat, 
		1.0f,    0,       0, 
		0,       1.0f,    0, 
		0,       0,       1.0f);


	iteInvertMatrix(&rotateMat,&rotateinverseMat);

	MULMATMAT(rotateinverseMat, transMat, transinverseMat);


	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING |
	                     ITE_VG_CMD_FULLRDY_EN |
	                     ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnUnpreMultiForDst ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     ITE_VG_CMD_DESTINATION_EN |
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
						 ITE_VG_CMD_SRCCOPY_EN |
	                     ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
	                     ITE_VG_CMD_RENDERMODE_1;

	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN;
	
    destImageFormat = destSurface->vgformat;
    srcImageFormat = srcSurface->vgformat;


    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = ((destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 (destX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register

	hwReg.REG_DHWR_BASE = (((ITEint16)destSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)destSurface->width & ITE_VG_CMDMASK_DSTWIDTH);

	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(destSurface->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)srcSurface->pitch<< ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (destSurface->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = ((0 << ITE_VG_CMDSHIFT_SRCY) & ITE_VG_CMDMASK_SRCY) |
		                 (0 & ITE_VG_CMDMASK_SRCX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)srcSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)srcSurface->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(srcSurface->data) & ITE_VG_CMDMASK_SRCBASE;	
	
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	/* Set image object ID */
	iteHardwareGenObjectID();
	hwReg.REG_BID2_BASE = iteHardwareGetCurrentObjectID();
	srcSurface->objectID= hwReg.REG_BID2_BASE;
	destSurface->objectID= hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);


}


void
iteM2dTransferBlock(
    ITEImage*           destSurface,
    ITEint				destX,
    ITEint				destY,
    ITEuint			    destWidth,
    ITEuint			    destHeight,
    ITEImage*           srcSurface,
    ITEint				srcX,
    ITEint				srcY,
    ITEint				rop)
{
    ITEHardwareRegister hwReg = {0};
    ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	HWImageFormat       destImageFormat      = HW_sRGB_565;
	HWImageFormat       srcImageFormat       = HW_sRGB_565;
	ITEboolean          enScissor            = ITE_FALSE;
	ITEColor	        color;
	ITEMatrix3x3        rotateMat            = {0};
	ITEMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3         transinverseMat      = {0};
	ITEImage*           hwCoverageImage      = NULL;
	ITEImage*           hwValidImage		 = NULL;
	ITEboolean          textureEnable        = ITE_FALSE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	color = context->clearColor;

	/* Get coverage image parameter */
	if ( context->surface->coverageIndex )
	{
		hwCoverageImage = context->surface->coverageImageB;
		hwValidImage = context->surface->validImageB;
		context->surface->coverageIndex = 0;
	}
	else
	{
		hwCoverageImage = context->surface->coverageImageA;
		hwValidImage = context->surface->validImageA;
		context->surface->coverageIndex = 1;
	}

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		0x10000, 0,       (ITEs15p16)((srcX - destX) << 16), 
		0,       0x10000, (ITEs15p16)((srcY - destY) << 16), 
		0,       0,       0x10000);

	SETMAT(rotateMat, 
			1.0f,    0, 	  0, 
			0,		 1.0f,    0, 
			0,		 0, 	  1.0f);

	
	iteInvertMatrix(&rotateMat,&rotateinverseMat);

	MULMATMAT(rotateinverseMat, transMat, transinverseMat);

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN |
				         ITE_VG_CMD_READMEM |
			             ITE_VG_CMD_TELWORK;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING |
						 ITE_VG_CMD_RENDERQ_BETTER |
						 ITE_VG_CMD_PLANFORMAT_TWOBYTES |
						 ITE_VG_CMD_FULLRDY_EN |
	                     ITE_VG_CMD_TIMERDY_EN;
											 
	hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;										 

	hwReg.REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
	hwReg.REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
		                   (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
	hwReg.REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
	hwReg.REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
	hwReg.REG_ACR_BASE |= ITE_VG_CMD_CLRCOVCACHE | ITE_VG_CMD_CLRRDRCACHE;

    //need source 
    if((rop ^ (rop>>2)) & 0x33)
	{
      textureEnable = ITE_TRUE;
	}


	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = ITE_VG_CMD_DST_PRE_EN |
						 ITE_VG_CMD_SRC_NONPRE_EN |
			             ITE_VG_CMD_SRC_PRE_EN |
			             ITE_VG_CMD_TEX_PRE_EN |
			             ITE_VG_CMD_WR_NONPRE_EN |
                   		 ITE_VG_CMD_DITHER_EN | // always enable dither 
                   		 ITE_VG_CMD_DESTINATION_EN |
                         ITE_VG_CMD_DSTCOLOR_EN |
						 ITE_VG_CMD_TEXCACHE_EN |
						 ((textureEnable == ITE_TRUE) ? ITE_VG_CMD_TEXTURE_EN : 0) | 
						 ITE_VG_CMD_ROP3_EN|
                         ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
                         ITE_VG_CMD_RDPLN_VLD_EN |
                         ITE_VG_CMD_RENDERMODE_1;
	
	hwReg.REG_CAR_BASE = rop | (rop << 8) | (rop << 16) | (rop << 24); 


    // ROP3 pattern paint color
    hwReg.REG_PPCR0_BASE = (color.b << 20 | 0xFF); 
    hwReg.REG_PPCR1_BASE = (color.r << 20 | color.g << 4); 

	
	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
						 ITE_VG_CMD_PAINTTYPE_COLOR;


    destImageFormat = destSurface->vgformat;
    srcImageFormat = srcSurface->vgformat;

	hwReg.REG_RFR_BASE |= (context->surface->maskImage->vgformat << 16)|(destImageFormat << 8) | srcImageFormat;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)destSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)destSurface->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(destSurface->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)srcSurface->pitch<< ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (destSurface->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)0 << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)0 & ITE_VG_CMDMASK_DSTX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)srcSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)srcSurface->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(srcSurface->data) & ITE_VG_CMDMASK_SRCBASE;
	
	
	hwReg.REG_UTR00_BASE = (ITEs15p16)(1.0f * 0x10000);
	hwReg.REG_UTR01_BASE = (ITEs15p16)(0    * 0x10000);
	hwReg.REG_UTR02_BASE = (ITEs15p16)(0    * 0x10000);
    hwReg.REG_UTR10_BASE = (ITEs15p16)(0    * 0x10000);
    hwReg.REG_UTR11_BASE = (ITEs15p16)(1.0f * 0x10000);
    hwReg.REG_UTR12_BASE = (ITEs15p16)(0    * 0x10000);
	hwReg.REG_UTR20_BASE = (ITEs15p16)(0    * 0x10000);
	hwReg.REG_UTR21_BASE = (ITEs15p16)(0    * 0x10000);
	hwReg.REG_UTR22_BASE = (ITEs15p16)(1.0f * 0x10000);

	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	hwReg.REG_MBR_BASE = (ITEuint32)context->surface->maskImage->data;

	hwReg.REG_SMPR_BASE = context->surface->maskImage->pitch;

	hwReg.REG_CTBR0_BASE = (ITEint32)(0 * 0x100);
	hwReg.REG_CTBR1_BASE = (ITEint32)(0 * 0x100);
	hwReg.REG_CTBR2_BASE = (ITEint32)(0 * 0x100);
    hwReg.REG_CTBR3_BASE = (ITEint32)(0 * 0x100);

	hwReg.REG_CTSR0_BASE = (((ITEint16)(0 * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
	                       (ITEint16)(1 * 0x100);

	hwReg.REG_CTSR1_BASE = (((ITEint16)(0 * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
	                       (ITEint16)(1 * 0x100);

	/* Set image object ID */
	iteHardwareGenObjectID();
	hwReg.REG_BID2_BASE = iteHardwareGetCurrentObjectID();
	srcSurface->objectID= hwReg.REG_BID2_BASE;
	destSurface->objectID= hwReg.REG_BID2_BASE;

	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);

}

void
iteM2dDrawTransparentBlock(
	    ITEImage* 	    destSurface,
		ITEint 			destX,
		ITEint 			destY,
		ITEuint			destWidth,
		ITEuint			destHeight,
		ITEImage*    	srcSurface,
		ITEint 			srcX,
		ITEint 			srcY,
		ITEColor		destHighColor,
		ITEColor		destLowColor,
		ITEColor		srcHighColor,
		ITEColor		srcLowColor,
		ITEint		    transparentRop
	)
{
	ITEHardwareRegister hwReg = {0};
    ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	HWImageFormat       destImageFormat      = HW_sRGB_565;
	HWImageFormat       srcImageFormat       = HW_sRGB_565;
	ITEboolean          enScissor            = ITE_FALSE;
	ITEMatrix3x3        rotateMat            = {0};
	ITEMatrix3x3        rotateinverseMat     = {0};
    HWMatrix3x3         transinverseMat      = {0};	
	ITEuint8            ropinverse           = 0;
	ITEMatrix3x3        imageMatrix          = {0};
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	ropinverse = ((transparentRop << 3)& 0x8) | ((transparentRop << 1)&0x4) | ((transparentRop >> 1)&0x2) | ((transparentRop >> 3)&0x1);

	ITE_INITOBJ(ITEHardware, hwReg);

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN |
	                     ITE_VG_CMD_READMEM |
			             ITE_VG_CMD_TELWORK;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING |
	                     ITE_VG_CMD_FULLRDY_EN |
	                     ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (destX + destWidth - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (destY + destHeight - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = ITE_VG_CMD_DST_PRE_EN |
						 ITE_VG_CMD_SRC_NONPRE_EN |
			             ITE_VG_CMD_SRC_PRE_EN |
			             ITE_VG_CMD_TEX_PRE_EN |
			             ITE_VG_CMD_WR_NONPRE_EN |
                         ITE_VG_CMD_DITHER_EN | // always enable dither 
                         ITE_VG_CMD_DESTINATION_EN |
                         ITE_VG_CMD_TEXCACHE_EN |
				         ITE_VG_CMD_TEXTURE_EN |
				         ITE_VG_CMD_TBITBLT_EN |
		                 ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
		                 ITE_VG_CMD_RDPLN_VLD_EN |
		                 ITE_VG_CMD_RENDERMODE_1;
	
	hwReg.REG_CAR_BASE = ropinverse;


	// tBitBlt Source HIGH color key
	hwReg.REG_RCR10_BASE = (srcHighColor.b  << 20 | 0xFF); 
	hwReg.REG_RCR11_BASE = (srcHighColor.r  << 20 | srcHighColor.g <<4); 

	//tBitBlt Source LOW color key
	hwReg.REG_RCR20_BASE = (srcLowColor.b  << 20 | 0xFF); 
    hwReg.REG_RCR21_BASE = (srcLowColor.r << 20 | srcLowColor.g <<4); 

    //tBitBlt Destination HIGH color key
	hwReg.REG_RCR30_BASE = (destHighColor.b << 20 | 0xFF); 
    hwReg.REG_RCR31_BASE = (destHighColor.r << 20 | destHighColor.g <<4); 

	//tBitBlt Destination LOW color key
	hwReg.REG_RCR40_BASE = (destLowColor.b << 20 | 0xFF); 
    hwReg.REG_RCR41_BASE = (destLowColor.r << 20 | destLowColor.g <<4); 


	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN;

    destImageFormat = destSurface->vgformat;
    srcImageFormat = srcSurface->vgformat;	

    hwReg.REG_RFR_BASE |= (destImageFormat << 8) | srcImageFormat;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)destY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)destX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)destSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)destSurface->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(destSurface->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)srcSurface->pitch<< ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (destSurface->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)0 << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)0 & ITE_VG_CMDMASK_DSTX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)srcSurface->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)srcSurface->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(srcSurface->data) & ITE_VG_CMDMASK_SRCBASE;
		

    SETMAT(transMat, 
          	0x10000, 0,       (ITEs15p16)((srcX - destX) << 16), 
          	0,       0x10000, (ITEs15p16)((srcY - destY) << 16), 
          	0,       0,       0x10000);

	SETMAT(rotateMat, 
			1.0f,    0, 	  0, 
			0,		 1.0f,    0, 
			0,		 0, 	  1.0f);  
			

    iteInvertMatrix(&rotateMat,&rotateinverseMat);

    MULMATMAT(rotateinverseMat, transMat, transinverseMat);			

    hwReg.REG_UITR00_BASE = (transinverseMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transinverseMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transinverseMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transinverseMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transinverseMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transinverseMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transinverseMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transinverseMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transinverseMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;


	/* Set image object ID */
	iteHardwareGenObjectID();
	hwReg.REG_BID2_BASE = iteHardwareGetCurrentObjectID();
	srcSurface->objectID= hwReg.REG_BID2_BASE;
	destSurface->objectID= hwReg.REG_BID2_BASE;

	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);

}


/*---------------------------------------------------
 * m2d extend API for alphablend
 * 
 *---------------------------------------------------*/

VG_API_CALL void 
vgM2dAlphaBlend(
	VGImage           image,
	VGint             x, 
	VGint             y, 
	VGint             width, 
	VGint             height,
	VGImage           srcimage,
	VGint             srcx, 
	VGint             srcy,
	VGBlendMode       blendMode,
    VGint			  srcConstAlpha)
{
	ITEImage* im          = NULL;
	ITEImage* srcim       = NULL;
	VGint     boundWidth  = width;
	VGint     boundHeight = height;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current contex */
	VG_RETURN_ERR_IF((ITEImage*)image == NULL, 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image),
                     VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	im = (ITEImage*)image;
	srcim = (ITEImage*)srcimage;

	/* Nothing to do if target rectangle out of bounds */
	if (   (x >= im->width)
		|| (y >= im->height) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}
	if (   ((x + im->width) < 0)
		|| ((y + im->height) < 0) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}

	/* Check copy boundary */
	if ( (x + width) > im->width )
	{
		boundWidth = im->width - x;
	}
	if ( (y + height) > im->height )
	{
		boundHeight = im->height - y;
	}


    iteM2dAlphaBlend(im,
				    x,
				    y,
				    boundWidth,
				    boundHeight,
				    srcim,
				    srcx,
				    srcy,
				    blendMode,
				    srcConstAlpha);

	ITEImage_AddDrawCount(im);

	VG_RETURN(VG_NO_RETVAL);
}



/*---------------------------------------------------
 * m2d extend API for transferblock
 * 
 *---------------------------------------------------*/

VG_API_CALL void 
vgM2dTransferBlock(
	VGint   x, 
	VGint   y, 
	VGint   width, 
	VGint   height,
	VGImage srcimage,
	VGint   srcx, 
	VGint   srcy,
	VGint   rop)
{
	ITEImage* srcim       = NULL;
	VGint     boundWidth  = width;
	VGint     boundHeight = height;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	srcim = (ITEImage*)srcimage;

    if(srcim != NULL)
    {
		/* Nothing to do if target rectangle out of bounds */
		if (   (srcx >= srcim->width)
			|| (srcy >= srcim->height) )
		{
			VG_RETURN(VG_NO_RETVAL);
		}
		if (   ((srcx + srcim->width) < 0)
			|| ((srcy + srcim->height) < 0) )
		{
			VG_RETURN(VG_NO_RETVAL);
		}

		/* Check copy boundary */
		if ( (srcx + width) > srcim->width )
		{
			boundWidth = srcim->width - srcx;
		}
		if ( (srcy + height) > srcim->height )
		{
			boundHeight = srcim->height - srcy;
		}
    }


    iteM2dTransferBlock(context->surface->colorImage,
					    x,
					    y,
					    boundWidth,
					    boundHeight,
					    srcim,
					    srcx,
					    srcy,
					    rop);

	ITEImage_AddDrawCount(srcim);

	VG_RETURN(VG_NO_RETVAL);
}


/*---------------------------------------------------
 * m2d extend API for fast source copy
 * 
 *---------------------------------------------------*/

VG_API_CALL void 
vgM2dSourceCopy(
	VGint   x, 
	VGint   y, 
	VGint   width, 
	VGint   height,
	VGImage srcimage,
	VGint   srcx, 
	VGint   srcy)
{
	ITEImage* srcim       = NULL;
	VGint     boundWidth  = width;
	VGint     boundHeight = height;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current contex */
	VG_RETURN_ERR_IF((ITEImage*)srcimage == NULL, 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, srcimage),
                     VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	srcim = (ITEImage*)srcimage;

	/* Nothing to do if target rectangle out of bounds */
	if (   (srcx >= srcim->width)
		|| (srcy >= srcim->height) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}
	if (   ((srcx + srcim->width) < 0)
		|| ((srcy + srcim->height) < 0) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}

	/* Check copy boundary */
	if ( (srcx + width) > srcim->width )
	{
		boundWidth = srcim->width - x;
	}
	if ( (srcy + height) > srcim->height )
	{
		boundHeight = srcim->height - y;
	}


    iteM2dSourceCopy(context->surface->colorImage,
				    x,
				    y,
				    boundWidth,
				    boundHeight,
				    srcim,
				    srcx,
				    srcy);

	ITEImage_AddDrawCount(srcim);

	VG_RETURN(VG_NO_RETVAL);
}


/*---------------------------------------------------
 * m2d extend API for transparentblock
 * 
 *---------------------------------------------------*/

VG_API_CALL void 
vgM2dDrawTransparentblock(
	VGImage           image,
	VGint             x, 
	VGint             y, 
	VGint             width, 
	VGint             height,
	VGImage           srcimage,
	VGint             srcx, 
	VGint             srcy,
	VGuint		      destHighColor,
	VGuint		      destLowColor,
	VGuint		      srcHighColor,
	VGuint		      srcLowColor,
	VGint		      transparentRop)
{
	ITEImage* im          = NULL;
	ITEImage* srcim       = NULL;
	VGint     boundWidth  = width;
	VGint     boundHeight = height;
	ITEColor  itedestHighColor;
	ITEColor  itedestLowColor;
	ITEColor  itesrcHighColor;
	ITEColor  itesrcLowColor;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current contex */
	VG_RETURN_ERR_IF((ITEImage*)image == NULL, 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image),
                     VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	im = (ITEImage*)image;
	srcim = (ITEImage*)srcimage;

	/* Nothing to do if target rectangle out of bounds */
	if (   (x >= im->width)
		|| (y >= im->height) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}
	if (   ((x + im->width) < 0)
		|| ((y + im->height) < 0) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}

	/* Check copy boundary */
	if ( (x + width) > im->width )
	{
		boundWidth = im->width - x;
	}
	if ( (y + height) > im->height )
	{
		boundHeight = im->height - y;
	}
	
	CSET(itedestHighColor,((((destHighColor & 0x0000F800) >> 11)*255)/31),((((destHighColor & 0x000007E0) >> 5)*255)/63),(((destHighColor & 0x0000001F)*255)/31),0xFF);
    CSET(itedestLowColor,((((destLowColor & 0x0000F800) >> 11)*255)/31),((((destLowColor & 0x000007E0) >> 5)*255)/63),(((destLowColor & 0x0000001F)*255)/31),0xFF);
	CSET(itesrcHighColor,((((srcHighColor & 0x0000F800) >> 11)*255)/31),((((srcHighColor & 0x000007E0) >> 5)*255)/63),(((srcHighColor & 0x0000001F)*255)/31),0xFF);
	CSET(itesrcHighColor,((((srcLowColor & 0x0000F800) >> 11)*255)/31),((((srcLowColor & 0x000007E0) >> 5)*255)/63),(((srcLowColor & 0x0000001F)*255)/31),0xFF);

    iteM2dDrawTransparentBlock(im,
							    x,
							    y,
							    boundWidth,
							    boundHeight,
							    srcim,
							    srcx,
							    srcy,
							    itedestHighColor,
								itedestLowColor,
								itesrcHighColor,
								itesrcLowColor,
								transparentRop);

	ITEImage_AddDrawCount(im);

	VG_RETURN(VG_NO_RETVAL);
}





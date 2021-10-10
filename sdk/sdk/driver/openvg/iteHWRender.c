/*
 */

#include "stdio.h"
#include "openvg.h"
#include "iteDefs.h"
#include "iteContext.h"
#include "itePath.h"
#include "iteImage.h"
#include "iteGeometry.h"
#include "itePaint.h"
#include "iteVectors.h"
#include "iteHardware.h"

unsigned int RenderDraw = 0;

/*-----------------------------------------------------------
 * Look up table
 *-----------------------------------------------------------*/
static void iteLookup(ITEColor* c)
{
	ITEHardware *h;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	c->r = h->lookupTable[0][c->r];
	c->g = h->lookupTable[1][c->g];
	c->b = h->lookupTable[2][c->b];
	c->a = h->lookupTable[3][c->a];

}

/*-----------------------------------------------------------
 * Get format bytes
 *-----------------------------------------------------------*/
static ITEuint iteGetFormatLogBits(HWImageFormat vg)
{
	ITEuint logBits = 5;
	switch(vg & 0x1F)
	{
		case 3: /* HW_sRGB_565 */
		case 4: /* HW_sRGBA_5551 */
		case 5: /* HW_sRGBA_4444 */
			logBits = 4;
			break;
		case 6: /* HW_sL_8 */
		case 10: /* HW_lL_8 */
		case 11: /* HW_A_8 */
			logBits = 3;
			break;
		case 12: /* HW_BW_1 */
		case 13: /* HW_A_1 */
			logBits = 0;
			break;
		case 14: /* HW_A_4 */
			logBits = 2;
			break;
		case 15: /* HW_RGBA_16 */
			logBits = 6;
			break;
		default:
			logBits = 5;
			break;
	}
	return logBits;
}

/*--------------------------------------------------------
 * Packs the pixel color components into memory at given
 * address according to given format
 *--------------------------------------------------------*/

void iteStoreColor(ITEColor *c, ITEHColor *hc, void *data, HWImageFormat vg, ITEuint8 bit)
{
	ITEuint8 in,src;
	ITEuint amsbBit = ((vg & (1 << 6)) >> 6);
	ITEuint bgrBit = ((vg & (1 << 7)) >> 7);

	if( (vg&0x1f) == 0xf )
	{
		*((ITEuint32*)data) = ((ITEuint32)hc->b<<16) | ((ITEuint32)hc->a&0xffff);
		*((ITEuint32*)data+1) = ((ITEuint32)hc->r<<16) | ((ITEuint32)hc->g&0xffff);
	}

	switch( vg & 0x1f )
	{
	case 12: // HW_BW_1
		src  = (ITEuint8)(0.2126f * c->r + 0.7152f * c->g + 0.0722f * c->b);
		src = src>>7;  // select bit7
		in = (ITEuint8) *((ITEuint8*)data); 
		*((ITEuint8*)data) = (in & (~(1<<bit))) | (src<<bit);
		break;
	case 13: // HW_A_1
		src = c->a>>7;
		in = (ITEuint8) *((ITEuint8*)data); 
		*((ITEuint8*)data) = (in & (~(1<<bit))) | (src<<bit);		
		break;
	case 14: // HW_A_4
		src = c->a>>4;
		bit = (bit) ? 4 : 0;
		in = (ITEuint8) *((ITEuint8*)data); 
		*((ITEuint8*)data) = (in & (~(0xf<<bit))) | (src<<bit);		
		break;
	case 15: // HW_RGBA_16
		break;
	case 6: // HW_sL_8
	case 10: // VG_lL_8
		*((ITEuint8*)data)  = (ITEuint8)(0.2126f * c->r + 0.7152f * c->g + 0.0722f * c->b);
		break;
	case 11: // HW_A_8
		*((ITEuint8*)data)  = (ITEuint8)c->a;
		break;
	case 3: // HW_sRGB_565
		if( bgrBit )
			*((ITEuint16*)data) = (((ITEuint16)c->b>>3)<<11) | (((ITEuint16)c->g>>2)<<5) | ((ITEuint16)c->r>>3);
		else
			*((ITEuint16*)data) = (((ITEuint16)c->r>>3)<<11) | (((ITEuint16)c->g>>2)<<5) | ((ITEuint16)c->b>>3);
		break;
	case 4: //HW_sRGBA_5551
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: *((ITEuint16*)data) = (((ITEuint16)c->r>>3)<<11) | (((ITEuint16)c->g>>3)<<6) | (((ITEuint16)c->b>>3)<<1) | ((ITEuint16)c->a>>7);
				break;
			case 1: *((ITEuint16*)data) = (((ITEuint16)c->b>>3)<<11) | (((ITEuint16)c->g>>3)<<6) | (((ITEuint16)c->r>>3)<<1) | ((ITEuint16)c->a>>7);
				break;
			case 2: *((ITEuint16*)data) = (((ITEuint16)c->a>>7)<<15) | (((ITEuint16)c->r>>3)<<10) | (((ITEuint16)c->g>>3)<<5) | ((ITEuint16)c->b>>3);
				break;
			case 3: *((ITEuint16*)data) = (((ITEuint16)c->a>>7)<<15) | (((ITEuint16)c->b>>3)<<10) | (((ITEuint16)c->g>>3)<<5) | ((ITEuint16)c->r>>3);
				break;
		}
		break;
	case 5: //HW_sRGBA_4444
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: *((ITEuint16*)data) = (((ITEuint16)c->r>>4)<<12) | (((ITEuint16)c->g>>4)<<8) | (((ITEuint16)c->b>>4)<<4) | ((ITEuint16)c->a>>4);
				break;
			case 1: *((ITEuint16*)data) = (((ITEuint16)c->b>>4)<<12) | (((ITEuint16)c->g>>4)<<8) | (((ITEuint16)c->r>>4)<<4) | ((ITEuint16)c->a>>4);
				break;
			case 2: *((ITEuint16*)data) = (((ITEuint16)c->a>>4)<<12) | (((ITEuint16)c->r>>4)<<8) | (((ITEuint16)c->g>>4)<<4) | ((ITEuint16)c->b>>4);
				break;
			case 3: *((ITEuint16*)data) = (((ITEuint16)c->a>>4)<<12) | (((ITEuint16)c->b>>4)<<8) | (((ITEuint16)c->g>>4)<<4) | ((ITEuint16)c->r>>4);
				break;
		}
		break; 
	default: // RGBA8888, RGBX888....
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: *((ITEuint32*)data) = ((ITEuint32)c->r<<24) | ((ITEuint32)c->g<<16) | ((ITEuint32)c->b<<8) | (ITEuint32)c->a;
				break;
			case 1: *((ITEuint32*)data) = ((ITEuint32)c->b<<24) | ((ITEuint32)c->g<<16) | ((ITEuint32)c->r<<8) | (ITEuint32)c->a;
				break;
			case 2: *((ITEuint32*)data) = ((ITEuint32)c->a<<24) | ((ITEuint32)c->r<<16) | ((ITEuint32)c->g<<8) | (ITEuint32)c->b;
				break;
			case 3: *((ITEuint32*)data) = ((ITEuint32)c->a<<24) | ((ITEuint32)c->b<<16) | ((ITEuint32)c->g<<8) | (ITEuint32)c->r;
				break;
		}
		break;
	}
}

/*---------------------------------------------------------
 * Unpacks the pixel color components from memory at given
 * address according to the given format
 *---------------------------------------------------------*/
void iteLoadColor(ITEColor *c, ITEHColor *hc, const void *data, HWImageFormat vg, ITEuint8 bit)
{
	ITEuint32 in = 0x0;
	ITEuint32 in1 = 0x0;
	ITEuint amsbBit = ((vg & (1 << 6)) >> 6);
	ITEuint bgrBit = ((vg & (1 << 7)) >> 7);
	ITEuint logBits = iteGetFormatLogBits(vg);

	/* Load from buffer */
	switch (logBits) 
	{
	case 6: 
		in = (ITEuint32) *((ITEuint32*)data); 
		in1 = (ITEuint32) *((ITEuint32*)data+1); 
		break;
	case 5: 
		in = (ITEuint32) *((ITEuint32*)data); 
		break;
	case 4: 
		in = (ITEuint32) *((ITEuint16*)data); 
		break;
	case 3: 
		in = (ITEuint32) *((ITEuint8*)data); 
		break;
	case 2: 
		in = (ITEuint32) *((ITEuint8*)data);
		if(bit)
			in = (in>>4)&0xf;
		else
			in = in&0xf;
		break;
	default: //0 
		in = (ITEuint32) *((ITEuint8*)data); 
		in = (in>>bit) & 0x1;
		break;
	}

	switch( vg & 0x1f )
	{
	case 12: // HW_BW_1
		c->r = c->g = c->b = (in) ? 0xff : 0;
		c->a = 0xff;
		break;
	case 13: // HW_A_1
		c->r = c->g = c->b = 0xff;
		c->a = (in) ? 0xff : 0;
		break;
	case 14: // HW_A_4
		c->r = c->g = c->b = 0xff;
		c->a =  in<<4;
		break;
	case 15: // HW_RGBA_16
		break;
	case 6: // HW_sL_8
	case 10: // HW_lL_8
		c->r = c->g = c->b = in;
		c->a = 0xff;
		break;
	case 11: // HW_A_8
		c->r = c->g = c->b = 0xff;
		c->a = in;
		break;
	case 3: // HW_sRGB_565
		if( bgrBit )
		{
			c->r = ((in&0x1f)<<3) | ((in&0x1f)>>2);
			c->g = ((in&0x7e0)>>3) | ((in&0x7e0)>>9);
			c->b = ((in&0xf800)>>8) | ((in&0xf800)>>13);
			c->a = 0xff;
		}
		else
		{
			c->r = ((in&0xf800)>>8) | ((in&0xf800)>>13);
			c->g = ((in&0x7e0)>>3) | ((in&0x7e0)>>9);
			c->b = ((in&0x1f)<<3) | ((in&0x1f)>>2);
			c->a = 0xff;
		}
		break;
	case 4: //HW_sRGBA_5551
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: 
				c->r = ((in&0xf800)>>8) | ((in&0xf800)>>13);
				c->g = ((in&0x7c0)>>3) | ((in&0x7e0)>>8);
				c->b = ((in&0x3e)<<2) | ((in&0x3e)>>3);
				c->a = (in&0x1) ? 0xff : 0;
				break;
			case 1:
				c->r = ((in&0x3e)<<2) | ((in&0x3e)>>3);
				c->g = ((in&0x7c0)>>3) | ((in&0x7e0)>>8);
				c->b = ((in&0xf800)>>8) | ((in&0xf800)>>13);
				c->a = (in&0x1) ? 0xff : 0;
				break;
			case 2:
				c->r = ((in&0x7c00)>>7) | ((in&0x7c00)>>12);
				c->g = ((in&0x3e0)>>2) | ((in&0x3e0)>>7);
				c->b = ((in&0x1f)<<3) | ((in&0x1f)>>2);
				c->a = (in&0x8000) ? 0xff : 0;
				break;
			case 3:
				c->r = ((in&0x1f)<<3) | ((in&0x1f)>>2);
				c->g = ((in&0x3e0)>>2) | ((in&0x3e0)>>7);
				c->b = ((in&0x7c00)>>7) | ((in&0x7c00)>>12);
				c->a = (in&0x8000) ? 0xff : 0;
				break;
		}
		break;
	case 5: //HW_sRGBA_4444
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: 
				c->r = ((in&0xf000)>>8) | ((in&0xf000)>>12);
				c->g = ((in&0xf00)>>4) | ((in&0xf00)>>8);
				c->b = (in&0xf0) | ((in&0xf0)>>4);
				c->a = ((in&0xf)<<4) | (in&0xf);
				break;
			case 1:
				c->b = ((in&0xf000)>>8) | ((in&0xf000)>>12);
				c->g = ((in&0xf00)>>4) | ((in&0xf00)>>8);
				c->r = (in&0xf0) | ((in&0xf0)>>4);
				c->a = ((in&0xf)<<4) | (in&0xf);
				break;
			case 2:
				c->a = ((in&0xf000)>>8) | ((in&0xf000)>>12);
				c->r = ((in&0xf00)>>4) | ((in&0xf00)>>8);
				c->g = (in&0xf0) | ((in&0xf0)>>4);
				c->b = ((in&0xf)<<4) | (in&0xf);
				break;
			case 3:
				c->a = ((in&0xf000)>>8) | ((in&0xf000)>>12);
				c->b = ((in&0xf00)>>4) | ((in&0xf00)>>8);
				c->g = (in&0xf0) | ((in&0xf0)>>4);
				c->r = ((in&0xf)<<4) | (in&0xf);
				break;
		}
		break;
	default: //HW_sRGBA_8888, HW_lRGBA_8888, HW_sRGBA_8888_PRE, HW_lRGBA_8888_PRE,
		switch( (amsbBit<<1) | bgrBit)
		{
			case 0: 
				c->r = (in&0xff000000)>>24;
				c->g = (in&0xff0000)>>16;
				c->b = (in&0xff00)>>8;
				c->a = ((vg&0x1f)==0 || (vg&0x1f)==7) ? 0xff : in&0xff;
				break;
			case 1:
				c->b = (in&0xff000000)>>24;
				c->g = (in&0xff0000)>>16;
				c->r = (in&0xff00)>>8;
				c->a = ((vg&0x1f)==0 || (vg&0x1f)==7) ? 0xff : in&0xff;
				break;
			case 2:
				c->a = ((vg&0x1f)==0 || (vg&0x1f)==7) ? 0xff : (in&0xff000000)>>24;
				c->r = (in&0xff0000)>>16;
				c->g = (in&0xff00)>>8;
				c->b = in&0xff;
				break;
			case 3:
				c->a = ((vg&0x1f)==0 || (vg&0x1f)==7) ? 0xff : (in&0xff000000)>>24;
				c->b = (in&0xff0000)>>16;
				c->g = (in&0xff00)>>8;
				c->r = in&0xff;
				break;
		}
		break;
	}

	if( (vg&0x1f) == 0xf )
	{
		hc->r = in1>>16;
		hc->g = in1&0xffff;
		hc->b = in>>16;
		hc->a = in&0xffff;
	}
	else
	{
		hc->r = c->r;
		hc->g = c->g;
		hc->b = c->b;
		hc->a = c->a;
	}

}

/*-------------------------------------------------------------------*//*!
* \brief	Reads a texel (u,v) at the given mipmap level. Tiling modes and
*			color space conversion are applied. Outputs color in premultiplied
*			format.
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/
static void readTexel(ITEColor *c, ITEHColor *hc, ITEs12p3 u, ITEs12p3 v, ITEuint8 *image, ITEint16 pitch, ITEint16 width, ITEint16 height, ITEuint8 vg, ITEint8 level,
					  VGTilingMode tilingMode, ITEColor tileFillColor, HWboolean enLookup)
{
	ITEint offset;
	ITEuint bit;
	ITEint uu,vv;
	ITEuint logBits = iteGetFormatLogBits(vg);
	ITEHardware *h;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	ITE_ASSERT(image);
	ITE_ASSERT(pitch);
	ITE_ASSERT(height);

	uu = u>>3;
	vv = v>>3;
	if(tilingMode == HW_TILE_FILL)
	{
		if(uu < 0 || vv < 0 || uu >= width || vv >= height)
			*c =tileFillColor;
		else
		{
			offset = vv*pitch + ((uu<<logBits)>>3);
			bit = uu&(0x7>>logBits);
			iteLoadColor(c, hc, &image[offset], vg, bit);
		}
	}
	else if(tilingMode == HW_TILE_PAD)
	{
		uu = ITE_MIN(ITE_MAX(uu,0),width-1);
		vv = ITE_MIN(ITE_MAX(vv,0),height-1);
		offset = vv*pitch + ((uu<<logBits)>>3);
		bit = uu&(0x7>>logBits);
		iteLoadColor(c, hc, &image[offset], vg, bit);
	}
	else if(tilingMode == HW_TILE_REPEAT)
	{
		uu = HW_MOD(uu, width);
		vv = HW_MOD(vv, height);
		offset = vv*pitch + ((uu<<logBits)>>3);
		bit = uu&(0x7>>logBits);
		iteLoadColor(c, hc, &image[offset], vg, bit);
	}
	else  //HW_TILE_REFLECT
	{
		uu = HW_MOD(uu, width*2);
		vv = HW_MOD(vv, height*2);
		if( uu >= width ) uu = width*2-1 - uu;
		if( vv >= height ) vv = height*2-1 - vv;
		offset = vv*pitch + ((uu<<logBits)>>3);
		bit = uu&(0x7>>logBits);
		iteLoadColor(c, hc, &image[offset], vg, bit);
	}

	// output is pre-multiplied
	if ( ~enLookup && (vg & 0xf)!=2 && (vg & 0xf)!=7 && h->enSrcMultiply) 
	{
		c->r = (ITEuint8)(((ITEuint)c->r * c->a)/255);
		c->g = (ITEuint8)(((ITEuint)c->g * c->a)/255);
		c->b = (ITEuint8)(((ITEuint)c->b * c->a)/255);
	}
}

/*-------------------------------------------------------------------*//*!
* \brief	Image resample with bilinear and nearest
* \param	Interpolation in pre-multiplied
* \return	Output is pre-multiplied color (expect:lookup)
* \note		
*//*-------------------------------------------------------------------*/
static void iteImageResample(ITEColor* c, ITEHColor* hc, ITEs12p3 x, ITEs12p3 y, HWMatrix3x3 *m, 
					  ITEuint8* image, ITEint pitch, ITEint width, ITEint height, HWImageFormat vg, 
					  HWImageQuality imageQuality, HWTilingMode tilingMode, ITEColor tileFillColor,
					  HWboolean enLookup)
{
	HWVector3 uvw0;
	HWTXVector3 uvw;
	ITEHColor h;
	ITEs15p16 oow;  // s12.16

	SET3(uvw0, x, y, 8);  // s12.3
    HW0TRANSFORM3TO(uvw0, (*m), uvw);	// output s12.16
	oow = (1<<28) / (uvw.z>>4);			// s12.16 = s1.28 / s1.12
	uvw.x = (ITEint16)(((ITEint64)uvw.x * oow)>>29);	// s12.3 = (s12.16 * s12.16)>>29
	uvw.y = (ITEint16)(((ITEint64)uvw.y * oow)>>29);
	uvw.z = (ITEint16)(((ITEint64)uvw.z * oow)>>29);

	if(imageQuality == HW_IMAGE_QUALITY_NONANTIALIASED)
	{	//point sampling
		ITEs12p3 u, v;
		u = (ITEint)HW_FLOOR(uvw.x);
		v = (ITEint)HW_FLOOR(uvw.y);
		readTexel(c, hc, u, v, image, pitch, width, height, vg, 0, tilingMode, tileFillColor, enLookup);
	}
	else
	{	//bilinear
		ITEs12p3 u, v;
		ITEs12p3 fu, fv;
		ITEColor c0, c1, c00, c10, c01, c11;
		uvw.x -= 0x4;  //0.5f
		uvw.y -= 0x4;  //0.5f
		u = (ITEint)HW_FLOOR(uvw.x);
		v = (ITEint)HW_FLOOR(uvw.y);
		readTexel(&c00, hc, u, v, image, pitch, width, height, vg, 0, tilingMode, tileFillColor, enLookup);
		readTexel(&c10, &h, u+1, v, image, pitch, width, height, vg, 0, tilingMode, tileFillColor, enLookup);
		readTexel(&c01, &h, u, v+1, image, pitch, width, height, vg, 0, tilingMode, tileFillColor, enLookup);
		readTexel(&c11, &h, u+1, v+1, image, pitch, width, height, vg, 0, tilingMode, tileFillColor, enLookup);
		fu = uvw.x - u;
		fv = uvw.y - v;
		c0.r = ((ITEint)c00.r*(8-fu) + (ITEint)c10.r*fu)>>3;
		c0.g = ((ITEint)c00.g*(8-fu) + (ITEint)c10.g*fu)>>3;
		c0.b = ((ITEint)c00.b*(8-fu) + (ITEint)c10.b*fu)>>3;
		c0.a = ((ITEint)c00.a*(8-fu) + (ITEint)c10.a*fu)>>3;
		c1.r = ((ITEint)c01.r*(8-fu) + (ITEint)c11.r*fu)>>3;
		c1.g = ((ITEint)c01.g*(8-fu) + (ITEint)c11.g*fu)>>3;
		c1.b = ((ITEint)c01.b*(8-fu) + (ITEint)c11.b*fu)>>3;
		c1.a = ((ITEint)c01.a*(8-fu) + (ITEint)c11.a*fu)>>3;
		c->r = ((ITEint)c0.r*(8-fv) + (ITEint)c1.r*fv)>>3;
		c->g = ((ITEint)c0.g*(8-fv) + (ITEint)c1.g*fv)>>3;
		c->b = ((ITEint)c0.b*(8-fv) + (ITEint)c1.b*fv)>>3;
		c->a = ((ITEint)c0.a*(8-fv) + (ITEint)c1.a*fv)>>3;
	}
}

static ITEuint8 iteReadMask(ITEint x, ITEint y, ITEuint8* data, ITEint pitch, ITEint height, ITEuint format)
{
	ITEuint logBits = iteGetFormatLogBits(format);
	ITEuint bit = x&(0x7>>logBits);
	ITEuint8 mask = data[y*pitch + ((x<<logBits)>>3)];

	if (logBits == 2) 
	{
		if(bit)
			mask = mask&0xf0;
		else
			mask = (mask&0xf)<<4;
	}
	else if(logBits == 0)
	{
		mask = ((mask>>bit) & 0x1) ? 0xff : 0;
	}
	return mask;
}

static void iteBlend(ITEColor *r, ITEColor *s, ITEColor *d, 
					 ITEHColor *hr, ITEHColor *hs, ITEHColor *hd, 
					 ITEuint8 ar, ITEuint8 ag, ITEuint8 ab)
{
	ITEHardware *h;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	hr->r = hr->g = hr->b = hr->a = 0;
    switch(h->blendMode)
    {
    case HW_BLEND_SRC:
        CSETC((*r),(*s));
        CSETC((*hr),(*hs));
        break;

    case HW_BLEND_SRC_OVER:
        r->r = (ITEuint)s->r + ((ITEuint)d->r * (255 - ar))/255;
        r->g = (ITEuint)s->g + ((ITEuint)d->g * (255 - ag))/255;
        r->b = (ITEuint)s->b + ((ITEuint)d->b * (255 - ab))/255;
        r->a = (ITEuint)s->a + ((ITEuint)d->a * (255 - s->a))/255;
        break;

    case HW_BLEND_DST_OVER:
        r->r = ((ITEuint)s->r * (255 - d->a))/255 + (ITEuint)d->r;
        r->g = ((ITEuint)s->g * (255 - d->a))/255 + (ITEuint)d->g;
        r->b = ((ITEuint)s->b * (255 - d->a))/255 + (ITEuint)d->b;
        r->a = ((ITEuint)s->a * (255 - d->a))/255 + (ITEuint)d->a;
        break;

    case HW_BLEND_SRC_IN:
        r->r = ((ITEuint)s->r * d->a)/255;
        r->g = ((ITEuint)s->g * d->a)/255;
        r->b = ((ITEuint)s->b * d->a)/255;
        r->a = ((ITEuint)s->a * d->a)/255;
        break;

    case HW_BLEND_DST_IN:
        r->r = ((ITEuint)d->r * ar)/255;
        r->g = ((ITEuint)d->g * ag)/255;
        r->b = ((ITEuint)d->b * ab)/255;
        r->a = ((ITEuint)d->a * s->a)/255;
        break;

    case HW_BLEND_MULTIPLY:
        r->r = ((ITEuint)s->r * (255 - d->a + d->r) + d->r * (255 - ar))/255;
        r->g = ((ITEuint)s->g * (255 - d->a + d->g) + d->g * (255 - ag))/255;
        r->b = ((ITEuint)s->b * (255 - d->a + d->b) + d->b * (255 - ab))/255;
        r->a = (ITEuint)s->a + (d->a * (255 - s->a))/255;
        break;

    case HW_BLEND_SCREEN:
        r->r = (ITEuint)s->r + ((ITEuint)d->r * (255 - s->r))/255;
        r->g = (ITEuint)s->g + ((ITEuint)d->g * (255 - s->g))/255;
        r->b = (ITEuint)s->b + ((ITEuint)d->b * (255 - s->b))/255;
        r->a = (ITEuint)s->a + ((ITEuint)d->a * (255 - s->a))/255;
        break;

    case HW_BLEND_DARKEN:
        r->r = ITE_MIN((ITEuint)s->r + ((ITEuint)d->r * (255 - ar))/255, (ITEuint)d->r + ((ITEuint)s->r * (255 - d->a))/255);
        r->g = ITE_MIN((ITEuint)s->g + ((ITEuint)d->g * (255 - ag))/255, (ITEuint)d->g + ((ITEuint)s->g * (255 - d->a))/255);
        r->b = ITE_MIN((ITEuint)s->b + ((ITEuint)d->b * (255 - ab))/255, (ITEuint)d->b + ((ITEuint)s->b * (255 - d->a))/255);
        r->a = (ITEuint)s->a + ((ITEuint)d->a * (255 - s->a))/255;
        break;

    case HW_BLEND_LIGHTEN:
        r->r = ITE_MAX((ITEuint)s->r + ((ITEuint)d->r * (255 - ar))/255, (ITEuint)d->r + ((ITEuint)s->r * (255 - d->a))/255);
        r->g = ITE_MAX((ITEuint)s->g + ((ITEuint)d->g * (255 - ag))/255, (ITEuint)d->g + ((ITEuint)s->g * (255 - d->a))/255);
        r->b = ITE_MAX((ITEuint)s->b + ((ITEuint)d->b * (255 - ab))/255, (ITEuint)d->b + ((ITEuint)s->b * (255 - d->a))/255);
        //although the statement below is equivalent to r.a = s.a + d.a * (1.0f - s.a)
        //in practice there can be a very slight difference because
        //of the max operation in the blending formula that may cause color to exceed alpha.
        //Because of this, we compute the result both ways and return the maximum.
        r->a = ITE_MAX((ITEuint)s->a + ((ITEuint)d->a * (255 - s->a))/255, (ITEuint)d->a + ((ITEuint)s->a * (255 - d->a))/255);
        break;

	case HW_BLEND_ADDITIVE:
        r->r = ITE_MIN(s->r + d->r, 255);
        r->g = ITE_MIN(s->g + d->g, 255);
        r->b = ITE_MIN(s->b + d->b, 255);
        r->a = ITE_MIN(s->a + d->a, 255);
		hr->r = hs->r + hd->r;
		hr->g = hs->g + hd->g;
		hr->b = hs->b + hd->b;
		hr->a = hs->a + hd->a;

        break;

	case HW_UNION_MASK:
		r->r = r->g = r->b = r->a = 255 - ((ITEuint)(255 - s->a) * (255 - d->a))/255;
		break;

	case HW_INTERSECT_MASK:
		r->r = r->g = r->b = r->a =  ((ITEuint)s->a * d->a)/255;
		break;

	case HW_SUBTRACT_MASK:
		r->r = r->g = r->b = r->a =  ((ITEuint)d->a * (255 - s->a))/255;
		break;

	default:
        CSETC((*r),(*s));
        break;

    }
}

static ITEint16 iteMask (ITEint16 cov, ITEuint8 mask, HWMaskMode maskMode)
{
	ITEint16 output;

    switch(maskMode)
    {
	case HW_UNION_RENDERMASK:
		output = (ITEint16)(255 - (((ITEuint)(0x20 - cov) * (255 - mask) + (1<<4) )>>5) );
		break;

	case HW_INTERSECT_RENDERMASK:
		output = (ITEint16)(((ITEuint)cov * mask  + (1<<4) )>>5);
		break;

	case HW_SUBTRACT_RENDERMASK:
		output = (ITEint16)(((ITEuint)cov * (255 - mask)  + (1<<4) )>>5);
		break;

	default:
		output = (ITEint16)(((ITEuint)cov * 255 + (1<<4) )>>5);
        break;
    }
	return output;
}


static ITEuint8 iteColorBound(ITEint16 hc)
{
	ITEuint8 c;
	if(hc<0)
		c = 0;
	else if(hc>255)
		c = 0xff;
	else
		c = (ITEuint8)hc;
	return c;
}

static void iteColorTransform(ITEColor* c, ITEHColor *hc)
{
	ITEHardware *h;
	ITEColor c0 = *c;
	ITEHColor hc0 = *hc;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	// colorTransform parameter is s7.8
	hc->r = (((ITEint)hc0.r*h->colorTransform[0][0] + 
		      (ITEint)c0.g*h->colorTransform[0][1] +
			  (ITEint)c0.b*h->colorTransform[0][2] + 
			  (ITEint)c0.a*h->colorTransform[0][3])>>8) + h->colorBias[0];
	hc->g = (((ITEint)c0.r*h->colorTransform[1][0] + 
		      (ITEint)hc0.g*h->colorTransform[1][1] +
			  (ITEint)c0.b*h->colorTransform[1][2] + 
			  (ITEint)c0.a*h->colorTransform[1][3])>>8) + h->colorBias[1];
	hc->b = (((ITEint)c0.r*h->colorTransform[2][0] + 
		      (ITEint)c0.g*h->colorTransform[2][1] +
			  (ITEint)hc0.b*h->colorTransform[2][2] + 
			  (ITEint)c0.a*h->colorTransform[2][3])>>8) + h->colorBias[2];
	hc->a = (((ITEint)c0.r*h->colorTransform[3][0] + 
		      (ITEint)c0.g*h->colorTransform[3][1] +
			  (ITEint)c0.b*h->colorTransform[3][2] + 
			  (ITEint)hc0.a*h->colorTransform[3][3])>>8) + h->colorBias[3];
	c->r = iteColorBound(hc->r);
	c->g = iteColorBound(hc->g);
	c->b = iteColorBound(hc->b);
	c->a = iteColorBound(hc->a);

}

/*-------------------------------------------------------------------*//*!
* \brief    Maps a gradient function value to a color.
* \param    
* \return   
* \note     
*//*-------------------------------------------------------------------*/
static ITEColor iteColorRamp(ITEs15p16 gradient)
{
	ITEColor c = {0};
	ITEHColor hc;
	ITEs12p3 x;
	HWTilingMode tilingMode;
	HWMatrix3x3 m;
	ITEHardware *h;

	VG_GETCONTEXT(c);
	h = context->hardware;

	SETMAT(m, 0x10000,0,0, 0,0x10000,0, 0,0,0x10000);
	x = (ITEs12p3)(gradient>>(16-3-h->gradientLen));
	switch(h->spreadMode)
	{
        case HW_COLOR_RAMP_SPREAD_REPEAT:
            tilingMode = HW_TILE_REPEAT;
            break;
        case HW_COLOR_RAMP_SPREAD_REFLECT:
        {
            tilingMode = HW_TILE_REFLECT;
            break;
        }
        default: // HW_COLOR_RAMP_SPREAD_PAD
            tilingMode = HW_TILE_PAD;
            break;
	}

	if(h->patternData)
		iteImageResample(&c, &hc, x, 0, &m, h->patternData, h->patternPitch, h->patternWidth, 1, h->patternFormat,
						 HW_IMAGE_QUALITY_NONANTIALIASED, tilingMode, h->tileFillColor, h->enLookup);
	else
		c = h->tileFillColor;
	return c;
}

/*-------------------------------------------------------------------*//*!
* \brief    Computes the linear gradient function at (x,y).
* \param    
* \return   
* \note     g(x,y) = Ax + By + C + SQRT(Dx^2 + Ey^2 + Fxy + Gx + Hy + I)
*			R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
*			A = (fx-cx) * R
*			B = (fy-cy) * R
*			C = - (fx(fx-cx) + fy(fy-cy)) * R
*			D = (r^2 + (fy-cy)^2) * R^2;
*			E = (r^2 + (fx-cx)^2) * R^2;
*			F = 2*(fx-cx)(fy-cy) * R^2
*			G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
*			H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
*			I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
*//*-------------------------------------------------------------------*/

static ITEs15p16 iteRadialGradient(ITEs12p3 x, ITEs12p3 y, HWMatrix3x3 *m)
{
	ITEs15p16 g, g0;
	HWVector2 pi;
	HWTXVector2 po;
	ITEHardware *h;

	VG_GETCONTEXT(0);
	h = context->hardware;
	
	SET2(pi, x, y);
	HW0TRANSFORM2TO(pi, (*m), po);
	g0 = HW_SQRT((((ITEint64)h->radialGradientD*po.x*po.x)>>32) + 
				 (((ITEint64)h->radialGradientE*po.y*po.y)>>32) + 
		         (((ITEint64)h->radialGradientF*po.x*po.y)>>32) + 
				 (((ITEint64)h->radialGradientG*po.x)>>16) + 
				 (((ITEint64)h->radialGradientH*po.y)>>16) + 
				 (ITEint64)h->radialGradientI );
	g = (ITEs15p16)((((ITEint64)h->radialGradientA*po.x)>>16) + (((ITEint64)h->radialGradientB*po.y)>>16) + h->radialGradientC + g0);
	return g;
}

/*-------------------------------------------------------------------*//*!
* \brief    Computes the linear gradient function at (x,y).
* \param    
* \return   
* \note     g(x,y) = Ax + By + C
*           A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
*           B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
*           C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
*//*-------------------------------------------------------------------*/

static ITEs15p16 iteLinearGradient(ITEs12p3 x, ITEs12p3 y, HWMatrix3x3 *m)
{
	ITEs15p16 g;
	HWVector2 pi;
	HWTXVector2 po;
	ITEHardware *h;

	VG_GETCONTEXT(0);
	h = context->hardware;

	SET2(pi, x, y);
	HW0TRANSFORM2TO(pi, (*m), po);
	g = (ITEs15p16)((((ITEint64)h->linearGradientA*po.x)>>16) + (((ITEint64)h->linearGradientB*po.y)>>16) + h->linearGradientC);
	return g;
}

/*--------------------------------------------------------------
 * pixle pipe
 *--------------------------------------------------------------*/

void iteRenderEngine()
{
	ITEColor s, d, r, im; 
	ITEHColor hs, hd, hr;
	ITEuint8 ar = 0, ag = 0, ab = 0, valid, scissor, mask;
	ITEint16 cov = 0x20, center;  // s10.5
	ITEint x,y;
	ITEint xmin, xmax, ymin, ymax;
	ITEint offset;
	ITEuint bit;
	ITEuint logBits;
	ITEHardware *h;
	HWMatrix3x3 *m;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	xmin = ITE_MAX((ITEint)(h->dstX>>3), 0);
	xmax = ITE_MIN((ITEint)(h->dstX>>3)+h->dstWidth, h->surfaceWidth);
	ymin = ITE_MAX((ITEint)(h->dstY>>3), 0);
	ymax = ITE_MIN((ITEint)(h->dstY>>3)+h->dstHeight, h->surfaceHeight);

	if( h->paintMode == HW_STROKE_PATH )
		m = &h->strokeTransform;
	else
		m = &h->fillTransform;

	for(y = ymin; y < ymax ; y++)
	{
		for(x = xmin; x < xmax ; x++)
		{
			// 0) check scissoring value
			if (h->enScissor && context->scissorImage.data)
			{
				ITEuint addroffset = (x>>3) + y*context->scissorImage.pitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;	

				scissor = *(context->scissorImage.data + addroffset);

				if ( (scissor & (1 << bitoffset)) == 0)	continue;
			}

			// 1) Check coverage value
			if(h->enCoverage && h->coverageData)
			{
				ITEuint covoffset = x-(ITEint)(h->coverageX>>3) + (y-(ITEint)(h->coverageY>>3))*h->coverageWidth;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(h->coverageX>>3)>>5) )  + (y-(int)(h->coverageY>>3))*h->coveragevalidpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(h->coverageX>>3)>>3) )  + (y-(int)(h->coverageY>>3))*h->coveragevalidpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;

				valid = *(h->coverageValid + validoffset);

				if ( (valid & (1 << bitoffset)) == 0) {
					if (h->maskMode == HW_INTERSECT_RENDERMASK)
						continue;
					else
						cov = 0x0;
				} else 
					cov = *(h->coverageData + covoffset);
	

				if (h->fillRule == HW_EVEN_ODD) 
				{
					cov = (cov>0) ? cov : - cov;
					center = (((cov>>1) + 0x10)>>5)<<5; // floor(cov/2 + 0.5)
					cov = cov - (center<<1);
					cov = (cov>0) ? cov : - cov;
				} 
				else	//HW_NON_ZERO
				{
					cov = (cov>0) ? cov : - cov;
					if (cov > 0x20 ) cov = 0x20;
				}
			} else
				cov = 0x20;

			// 2) Get paint color
			switch(h->paintType)
			{
				case HW_PAINT_TYPE_COLOR:
					s = h->paintColor;
					break;

				case HW_PAINT_TYPE_LINEAR_GRADIENT:
				{
					ITEs15p16 g;
					g = iteLinearGradient((ITEs12p3)((x<<3)+4), (ITEs12p3)((y<<3)+4), m);
					s = iteColorRamp(g);
					break;
				}

				case HW_PAINT_TYPE_RADIAL_GRADIENT:
				{
					ITEs15p16 g;
					g = iteRadialGradient((ITEs12p3)((x<<3)+4), (ITEs12p3)((y<<3)+4), m);
					s = iteColorRamp(g);
					break;
				}

				default:  // HW_PAINT_TYPE_PATTERN
					if(h->patternData)
						iteImageResample(&s, &hs, (ITEs12p3)((x<<3)+4), (ITEs12p3)((y<<3)+4),m, h->patternData, h->patternPitch, h->patternWidth, h->patternHeight, h->patternFormat,
										 h->imageQuality, h->tilingMode, h->tileFillColor, h->enLookup);
					else
						s = h->paintColor;
					break;
			}

			// 3) read Texture image
			if(h->enTexture)
			{
				ITEColor tileColor;
				CSET(tileColor, 0, 0, 0, 0);
				iteImageResample(&im, &hs, (ITEs12p3)((x<<3)+4), (ITEs12p3)((y<<3)+4), &h->imageTransform, h->textureData, h->texturePitch, h->textureWidth, h->textureHeight, h->textureFormat,
								 h->imageQuality, HW_TILE_PAD, tileColor, h->enLookup);
				// 3.1 enable lookup table
				if(h->enLookup)
					iteLookup(&im);
			
			}

			// 4) ImageMode operation
			if( h->enTexture )
			{
				if(h->imageMode == HW_DRAW_IMAGE_NORMAL)
					s = im;
				else if (h->imageMode == HW_DRAW_IMAGE_MULTIPLY)
				{
					s.r = (((ITEuint)s.r*im.r) + (((ITEuint)s.r*im.r)>>8))>>8; //(s.r * im.r)/255;
					s.g = (((ITEuint)s.g*im.g) + (((ITEuint)s.g*im.g)>>8))>>8;
					s.b = (((ITEuint)s.b*im.b) + (((ITEuint)s.b*im.b)>>8))>>8;
					s.a = (((ITEuint)s.a*im.a) + (((ITEuint)s.a*im.a)>>8))>>8;
				}
				else	// h->imageMode == HW_DRAW_IMAGE_STENCIL
					s = s;
			}

			// 5) Color Transform (input: non-premultiplied)
			if(h->enSrcUnMultiply)
			{
				ITEuint16 ooa;
				ooa = (s.a != 0) ? 0xff00/s.a : 0;
				s.r = ((ITEuint)s.r*ooa)>>8;
				s.g = ((ITEuint)s.g*ooa)>>8;
				s.b = ((ITEuint)s.b*ooa)>>8;
			}
	
			if(h->enColorTransform)
				iteColorTransform(&s, &hs);
			else
			{
				hs.r = (ITEint16)s.r;
				hs.g = (ITEint16)s.g;
				hs.b = (ITEint16)s.b;
				hs.a = (ITEint16)s.a;
			}

			// 6) source pre-multiply
			if(h->enSrcMultiply)
			{
				s.r = (((ITEuint)hs.r*s.a) + (((ITEuint)hs.r*s.a)>>8))>>8; //(s.r*s.a)/255;
				s.g = (((ITEuint)hs.g*s.a) + (((ITEuint)hs.g*s.a)>>8))>>8;
				s.b = (((ITEuint)hs.b*s.a) + (((ITEuint)hs.b*s.a)>>8))>>8;
			}

			// 7) Stencil alpha multiply
			if( h->enTexture && h->imageMode == HW_DRAW_IMAGE_STENCIL)
			{
				ar = (((ITEuint)s.a*im.r) + (((ITEuint)s.a*im.r)>>8))>>8; //(s.a * im.r)/255;
				ag = (((ITEuint)s.a*im.g) + (((ITEuint)s.a*im.g)>>8))>>8;
				ab = (((ITEuint)s.a*im.b) + (((ITEuint)s.a*im.b)>>8))>>8;
			}
			else
			{
				ar = s.a;
				ag = s.a;
				ab = s.a;
			}
			
			// 8) Blending
			logBits = iteGetFormatLogBits(h->surfaceFormat);
			offset = y*h->surfacePitch + ((x<<logBits)>>3);
			bit = x&(0x7>>logBits);

			if( h->enBlend || h->enCoverage || h->enMask)
			{
				// 8.1) read destination
				iteLoadColor(&d, &hd, &h->surfaceData[offset], h->surfaceFormat, bit);

				// 8.2) destination pre-multiply
				if(h->enDstMultiply)
				{
					d.r = (((ITEuint)d.r*d.a) + (((ITEuint)d.r*d.a)>>8))>>8; //(d.r*d.a)/255;
					d.g = (((ITEuint)d.g*d.a) + (((ITEuint)d.g*d.a)>>8))>>8;
					d.b = (((ITEuint)d.b*d.a) + (((ITEuint)d.b*d.a)>>8))>>8;
				}

				// 8.3) blending
				iteBlend(&r, &s, &d, &hr, &hs, &hd, ar, ag, ab); 
			}
			else
			{
				CSETC(r, s);
				CSET(d, 0, 0, 0, 0);  // not important
				CSETC(hr, hs);
				CSET(hd, 0, 0, 0, 0);
			}
			
			// 9) mask and coverage multiply
			if( h->enMask && h->maskData )
				mask = iteReadMask(x, y, h->maskData, h->maskPitch, h->maskHeight, h->maskFormat);
			else
				mask = 0xff;

			// cov: 0.8
			cov = iteMask(cov, mask, h->maskMode);

			// 10) Coverage blending
			r.r = (ITEuint8)(((ITEuint)r.r*cov + (ITEuint)d.r*(0xff - cov))/255);
			r.g = (ITEuint8)(((ITEuint)r.g*cov + (ITEuint)d.g*(0xff - cov))/255);
			r.b = (ITEuint8)(((ITEuint)r.b*cov + (ITEuint)d.b*(0xff - cov))/255);
			r.a = (ITEuint8)(((ITEuint)r.a*cov + (ITEuint)d.a*(0xff - cov))/255);
/*
			r.r = (ITEuint8)(((ITEuint)r.r*cov + d.r*(0x20 - cov))>>5);
			r.g = (ITEuint8)(((ITEuint)r.g*cov + d.g*(0x20 - cov))>>5);
			r.b = (ITEuint8)(((ITEuint)r.b*cov + d.b*(0x20 - cov))>>5);
			r.a = (ITEuint8)(((ITEuint)r.a*cov + d.a*(0x20 - cov))>>5);
*/		

			// 11) destination un-pre-multiply
			if(h->enDstMultiply)
			{
				ITEuint16 ooa;
				ooa = (r.a != 0) ? 0xff00/r.a : 0;
				r.r = ((ITEuint)r.r*ooa)>>8;
				r.g = ((ITEuint)r.g*ooa)>>8;
				r.b = ((ITEuint)r.b*ooa)>>8;
			}

			// 12) write back (ToDo: dither)
			if (cov != 0) {
				iteStoreColor(&r, &hr, &h->surfaceData[offset], h->surfaceFormat, bit);
				if (RENDERDRAW && h->enCoverage) RenderDraw++;
			}
		}
	}
	if (RENDERDRAW) printf("RenderDraw = %d\n",RenderDraw);

	if (DUMPDATA && context->printnum!=0) {
		char	filename[] = "dumprdr000.hex";
		char	fileraw[] = "dumprdr000.bin";
		int		i, j, dump_pitch;
		char    prtnum;
		FILE	*out_file;
		FILE	*raw_file;

		prtnum = context->printnum-1;

		filename[7] = '0' + (prtnum/100)%10;
		filename[8] = '0' + (prtnum/10)%10;
		filename[9] = '0' + prtnum%10;

		fileraw[7] = filename[7];
		fileraw[8] = filename[8];
		fileraw[9] = filename[9];

		// write file
		out_file = fopen(filename, "wb");
		raw_file = fopen(fileraw, "wb");

/*
		for (i=0; i<h->surfaceHeight; i++) {
			for (j=0; j<h->surfacePitch; j++) {
				fprintf(out_file,"%02x ", h->surfaceData[i*h->surfacePitch + j]);
				fprintf(raw_file,"%c", h->surfaceData[i*h->surfacePitch + j]);
			}
			fprintf(out_file,"\n");
		}
*/
		switch( h->surfaceFormat & 0x1f )
		{
		case 12: // HW_BW_1
		case 13: // HW_A_1
			dump_pitch	= (DUMPWIDTH + 7) >> 3;
			break;

		case 14: // HW_A_4
			dump_pitch	= (DUMPWIDTH + 1) >> 1;
			break;

		case 6: // HW_sL_8
		case 10: // VG_lL_8
		case 11: // HW_A_8
			dump_pitch	= DUMPWIDTH;
			break;

		case 3: // HW_sRGB_565
		case 4: //HW_sRGBA_5551
		case 5: //HW_sRGBA_4444
		case 15: // HW_RGBA_16
			dump_pitch	= DUMPWIDTH << 1;
			break;

		default: // RGBA8888, RGBX888....
			dump_pitch	= DUMPWIDTH << 2;
			break;
		}

		/*
		for (i=0; i<DUMPHEIGHT; i++) {
			for (j=0; j<dump_pitch; j++) {
				fprintf(out_file,"%02x ", h->surfaceData[i*h->surfacePitch + j]);
				fprintf(raw_file,"%c", h->surfaceData[i*h->surfacePitch + j]);
			}
			fprintf(out_file,"\n");	
		}
		*/

		fclose(out_file);
		fclose(raw_file);
		printf("\nDump Render No. %d file ok!\n", prtnum);

	}
}

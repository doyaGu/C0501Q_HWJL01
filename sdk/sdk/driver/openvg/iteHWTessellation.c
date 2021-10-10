/*
 */
#include "openvg.h"
#include "iteDefs.h"
#include "iteContext.h"
#include "iteHardware.h"

static HWTXVector2 start;
static HWTXVector2 last;

// for debug length
ITEint64 totallength = 0;

// Inverse square root
static ITEint64 iteInvSqurt (ITEint64 x, ITEint shift)
{
	float r = (float)x/((ITEint64)1<<shift);										// s24.shift
	float tmp = (float)x/((ITEint64)1<<shift);
	float x0f, x1f;
	float r_tmp, tmp_tmp;															// for HW debug
	int i = *(int*)&tmp;
	i = 0x5F375A86 - (i>>1);
	tmp = *(float*)&i;

	r_tmp = r*tmp;
	tmp_tmp = tmp*tmp;

	x0f = 3*tmp;
	x1f = r*tmp*tmp*tmp;  

	x = ( (int)(x0f*(1<<POINTPREC)) - (int)(x1f*(1<<POINTPREC)) +1 )>>1;			// s12.11 Newton method with rounding
	//x = (int)(tmp*(1<<POINTPREC));												// s12.11 

	return x;
}

static ITEint64 iteSqrt(ITEint64 num) 
{

  ITEint64 op = num;
  ITEint64 res = 0;
  ITEint64 tmp = 0;

  op	= iteInvSqurt(num, POINTPREC*2);					// s12.11
 
  if ( (num>>(POINTPREC*3-1))!=0 ) {						// For unit cycle, this case is imposseible
		res	= (num>>(POINTPREC*2)) * op;					// ((s24.22>>22)*s12.11) = s12.11
  } else if ( (num>>(POINTPREC*2+1))!=0 ) {
		res	= ((num>>POINTPREC) * op)>>POINTPREC;			// ((s24.22>>11)*s12.11)>>11 = s12.11
  } else {
		res	= (num * op)>>(POINTPREC*2);					// ((s1.22*s12.11)>>22 = s12.11
  }

  /*
  ITEint64 one = (ITEint64)1 << 62;	
  // For int64, it should be <<62. However, the most bit shift is 24 because of input s3.20 . 
  // The second-to-top bit is set: 1L<<30 for long

  // "one" starts at the highest power of four <= the argument.
  while (one > op)
      one >>= 2;

  while (one != 0) {
      if (op >= res + one) {
          op -= res + one;
          res = (res >> 1) + one;
      }
      else {
          res >>= 1;
      }
      one >>= 2;
  }
  */

  return res;
  
}


static void iteDivideCircle(HWTXVector2 u0, HWTXVector2 u1, ITEuint16 lines, ITEint i,
							ITEuint8 numofaxis, ITEuint8 section, ITEuint8 ccw, ITEuint8 u0quad,
							ITEint* shift, ITEuint16* divider, ITEuint16* n, HWTXVector2* vec0, HWTXVector2* vec1)
{
	HWTXVector2 axis[4];

	if (ccw!=1) {
		axis[0].x = 1<<(POINTPREC*2);	// s1.22
		axis[0].y = 0;					// s1.22
		axis[1].x = 0;					// s1.22
		axis[1].y = -1<<(POINTPREC*2);	// s1.22
		axis[2].x = -1<<(POINTPREC*2);	// s1.22
		axis[2].y = 0;					// s1.22		
		axis[3].x = 0;					// s1.22
		axis[3].y = 1<<(POINTPREC*2);	// s1.22
	} else {
		axis[0].x = 0;					// s1.22
		axis[0].y = 1<<(POINTPREC*2);	// s1.22
		axis[1].x = -1<<(POINTPREC*2);	// s1.22
		axis[1].y = 0;					// s1.22
		axis[2].x = 0;					// s1.22
		axis[2].y = -1<<(POINTPREC*2);	// s1.22
		axis[3].x = 1<<(POINTPREC*2);	// s1.22
		axis[3].y = 0;					// s1.22
	}

	if (section == 1) 
	{
		//SET2V(vec0,u0);
		//SET2V(vec1,u1);
		SET2VP(vec0,u0);
		SET2VP(vec1,u1);
	
		*divider = lines;
		*n = i;
	} 
	else if (section == 2) 
	{
		if (i < (lines>>1)) 
		{
			//SET2V(vec0,u0);
			//SET2V(vec1,axis[u0quad & 0x3]);
			SET2VP(vec0,u0);
			SET2VP(vec1,axis[u0quad & 0x3]);
			*divider = lines >> 1;
			*n = i;
		}
		else
		{
			//SET2V(vec0,axis[u0quad & 0x3]);
			//SET2V(vec1,u1);
			SET2VP(vec0,axis[u0quad & 0x3]);
			SET2VP(vec1,u1);
			*divider = lines >> 1;
			*n = i - (lines>>1);
		}
	}
	else if (section == 3)
	{
		if (i < (lines>>2)) 
		{
			//SET2V(vec0,u0);
			//SET2V(vec1,axis[u0quad & 0x3]);
			SET2VP(vec0,u0);
			SET2VP(vec1,axis[u0quad & 0x3]);		
			*divider = lines >> 2;
			*n = i;
		}
		else if (i < (lines>>1)+(lines>>2))
		{
			//SET2V(vec0,axis[u0quad & 0x3]);
			//SET2V(vec1,axis[(u0quad+1) & 0x3]);
			SET2VP(vec0,axis[u0quad & 0x3]);
			SET2VP(vec1,axis[(u0quad+1) & 0x3]);
			*divider = lines >> 1;
			*n = i - (lines>>2);
		}
		else
		{
			//SET2V(vec0,axis[(u0quad+1) & 0x3]);
			//SET2V(vec1,u1);
			SET2VP(vec0,axis[(u0quad+1) & 0x3]);
			SET2VP(vec1,u1);
			*divider = lines >> 2;
			*n = i - (lines>>1) - (lines>>2);
		}
	}
	else if (section == 4)
	{
		if (i < (lines>>2)) 
		{
			//SET2V(vec0,u0);
			//SET2V(vec1,axis[u0quad & 0x3]);
			SET2VP(vec0,u0);
			SET2VP(vec1,axis[u0quad & 0x3]);
			*divider = lines >> 2;
			*n = i;
		}
		else if (i < (lines>>1))
		{
			//SET2V(vec0,axis[u0quad & 0x3]);
			//SET2V(vec1,axis[(u0quad+1) & 0x3]);
			SET2VP(vec0,axis[u0quad & 0x3]);
			SET2VP(vec1,axis[(u0quad+1) & 0x3]);
			*divider = lines >> 2;
			*n = i - (lines>>2);
		}
		else if (i < (lines>>1)+(lines>>2))
		{
			//SET2V(vec0,axis[(u0quad+1) & 0x3]);
			//SET2V(vec1,axis[(u0quad+2) & 0x3]);
			SET2VP(vec0,axis[(u0quad+1) & 0x3]);
			SET2VP(vec1,axis[(u0quad+2) & 0x3]);
			*divider = lines >> 2;
			*n = i - (lines>>1);
		}
		else
		{
			//SET2V(vec0,axis[(u0quad+2) & 0x3]);
			//SET2V(vec1,u1);
			SET2VP(vec0,axis[(u0quad+2) & 0x3]);
			SET2VP(vec1,u1);
			*divider = lines >> 2;
			*n = i - (lines>>1) - (lines>>2);
		}
	
	}
	else if (section == 5)
	{
		if (i < (lines>>3)) 
		{
			//SET2V(vec0,u0);
			//SET2V(vec1,axis[u0quad & 0x3]);
			SET2VP(vec0,u0);
			SET2VP(vec1,axis[u0quad & 0x3]);
			*divider = lines >> 3;
			*n = i;
		}
		else if (i < (lines>>2)+(lines>>3))
		{
			//SET2V(vec0,axis[u0quad & 0x3]);
			//SET2V(vec1,axis[(u0quad+1) & 0x3]);
			SET2VP(vec0,axis[u0quad & 0x3]);
			SET2VP(vec1,axis[(u0quad+1) & 0x3]);
			*divider = lines >> 2;
			*n = i - (lines>>3);
		}
		else if (i < (lines>>1)+(lines>>3))
		{
			//SET2V(vec0,axis[(u0quad+1) & 0x3]);
			//SET2V(vec1,axis[(u0quad+2) & 0x3]);
			SET2VP(vec0,axis[(u0quad+1) & 0x3]);
			SET2VP(vec1,axis[(u0quad+2) & 0x3]);
			*divider = lines >> 2;
			*n = i - (lines>>2) - (lines>>3);
		}
		else if (i < lines-(lines>>3))
		{
			//SET2V(vec0,axis[(u0quad+2) & 0x3]);
			//SET2V(vec1,axis[(u0quad+3) & 0x3]);
			SET2VP(vec0,axis[(u0quad+2) & 0x3]);
			SET2VP(vec1,axis[(u0quad+3) & 0x3]);
			*divider = lines >> 2;
			*n = i - (lines>>1) - (lines>>3);
		}		
		else
		{
			//SET2V(vec0,axis[(u0quad+3) & 0x3]);
			//SET2V(vec1,u1);
			SET2VP(vec0,axis[(u0quad+3) & 0x3]);
			SET2VP(vec1,u1);
			*divider = lines >> 3;
			*n = i - lines + (lines>>3);
		}
	}

	if (*divider == 2)		*shift=1;
	if (*divider == 4)		*shift=2;
	if (*divider == 8)		*shift=3;
	if (*divider == 16)		*shift=4;
	if (*divider == 32)		*shift=5;
	if (*divider == 64)		*shift=6;
	if (*divider == 128)	*shift=7;
	if (*divider == 256)	*shift=8;
	if (*divider == 512)	*shift=9;
	if (*divider == 1024)	*shift=10;
	if (*divider == 2048)	*shift=11;
	if (*divider == 4096)	*shift=12;

	return;
}


static void iteDoCap(ITEuint* outCount, HWTXVector2 p, HWTXVector2 r, HWTXVector2 c, HWTXVector2 u0)
{
	ITEHardware* h;
	HWTXVector2 outdata;
	HWTXVector2	u1, m;
	ITEuint8 su0x, su0y, su1x, su1y;	// signed of unit vector
	ITEuint8 numofaxis, section, u0quad;
	ITEint i;
	
	// For caculate circule points
	HWTXVector2 interm, vec0, vec1;
	//HWTXVector2 unitm;					// s1.22
	HWTXVector2 arcm;					// s12.11
	ITEint64 squre;
	ITEuint16 divider, n;
	ITEint shift;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if (EQ2V(p, r) || h->strokeCapStyle==0) return;

	h->cmdData[(*outCount)++] = 0xc0000001;
	HWAFFINETRANSFORM2TO(p, h->pathTransform, outdata);
	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);

	h->cmdData[(*outCount)++] = 0xe0000000 | 0x1;
	HWAFFINETRANSFORM2TO(r, h->pathTransform, outdata);
	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);

	switch(h->strokeCapStyle)
	{
	case 0: //HW_CAP_BUTT	// impossible reach here
		break;

	case 1: //HW_CAP_ROUND
		h->cmdData[(*outCount)++] = 0xe0000000 | (h->strokeRoundLines-1);

		u1.x = -u0.x;
		u1.y = -u0.y;

		// determine how many axises between u0 and u1 and start quadrant of u0
		su0x = (u0.x>>(POINTPREC*2+1))&0x1;
		su0y = (u0.y>>(POINTPREC*2+1))&0x1;
		su1x = (u1.x>>(POINTPREC*2+1))&0x1;
		su1y = (u1.y>>(POINTPREC*2+1))&0x1;

		if (u0.x==0 && su0y==1)		su0x=1;
		if (u0.y==0 && su0x==0)		su0y=1;
		if (u1.x==0 && su1y==0)		su1x=1;
		if (u1.y==0 && su1x==1)		su1y=1;

		if (su0x == su1x && su0y == su1y) {			// pass 0 axis
			numofaxis	= 0;
			section		= 1;
		} else if (su0x != su1x && su0y != su1y) {	// pass 2 axises
			numofaxis	= 2;
			section		= 3;
		} else {									// pass 1 axises
			numofaxis	= 1;
			section		= 2;
		}

		// ccw = -1 => Quadrant clock wise
		if (su0x==0 && su0y==0)
			u0quad = 0;
		else if (su0x==1 && su0y==0)
			u0quad = 3;
		else if (su0x==1 && su0y==1)
			u0quad = 2;
		else
			u0quad = 1;

		for(i=1;i<h->strokeRoundLines;i++)
		{
			iteDivideCircle(u0, u1, h->strokeRoundLines, i, numofaxis, section, -1, u0quad,
							&shift, &divider, &n, &vec0, &vec1);			

			interm.x = (ITEs15p16)( ( (ITEint64)vec0.x*(divider-n) + (ITEint64)vec1.x*n )>>shift );		// s1.22
			interm.y = (ITEs15p16)( ( (ITEint64)vec0.y*(divider-n) + (ITEint64)vec1.y*n )>>shift );		// s1.22

			squre = ( (ITEint64)interm.x*interm.x + (ITEint64)interm.y*interm.y ) >> (POINTPREC*2);		// s3.22
/*
			// original version
			// s1.22 = (s1.22*s12.11)>>11
			unitm.x = (ITEs15p16)( ( (ITEint64)interm.x * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );
			unitm.y = (ITEs15p16)( ( (ITEint64)interm.y * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );	

			// (s1.22 * s12.11/2)>>22 + s12.11 = s12.11
			m.x = (ITEs15p16)( ((ITEint64)unitm.x*h->lineWidth>>(POINTPREC*2+1)) + c.x);	
			m.y = (ITEs15p16)( ((ITEint64)unitm.y*h->lineWidth>>(POINTPREC*2+1)) + c.y);	
*/
			// s12.11 = (s1.22*s12.11/2)>>22
			arcm.x = (ITEs15p16)( ( (ITEint64)interm.x * h->lineWidth ) >> (POINTPREC*2+1) );
			arcm.y = (ITEs15p16)( ( (ITEint64)interm.y * h->lineWidth ) >> (POINTPREC*2+1) );	

			// (s12.11 * s12.11)>>11 + s12.11 = s12.11
			m.x = (ITEs15p16)( ( ((ITEint64)arcm.x*iteInvSqurt(squre, POINTPREC*2))>>POINTPREC ) + c.x);	
			m.y = (ITEs15p16)( ( ((ITEint64)arcm.y*iteInvSqurt(squre, POINTPREC*2))>>POINTPREC ) + c.y);	

			HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
			h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
			MIN2V(h->min, outdata);
			MAX2V(h->max, outdata);
		}
		break;
	
	case 2: //HW_CAP_SQUARE
		h->cmdData[(*outCount)++] = 0xe0000000 | 0x2;

		//u1.x = u0.y;
		//u1.y = -u0.x;

		//m.x = r.x + (ITEs15p16)( ((ITEint64)u1.x*h->lineWidth)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
		//m.y = r.y + (ITEs15p16)( ((ITEint64)u1.y*h->lineWidth)>>(POINTPREC*2+1) );
		m.x = r.x + (ITEs15p16)( ((ITEint64)u0.y*h->lineWidth)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
		m.y = r.y - (ITEs15p16)( ((ITEint64)u0.x*h->lineWidth)>>(POINTPREC*2+1) );
		HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);

		//m.x = p.x + (ITEs15p16)( ((ITEint64)u1.x*h->lineWidth)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
		//m.y = p.y + (ITEs15p16)( ((ITEint64)u1.y*h->lineWidth)>>(POINTPREC*2+1) );
		m.x = p.x + (ITEs15p16)( ((ITEint64)u0.y*h->lineWidth)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
		m.y = p.y - (ITEs15p16)( ((ITEint64)u0.x*h->lineWidth)>>(POINTPREC*2+1) );
		HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);
		break;
	}
}


static void iteDoJoin(ITEuint* outCount)
{
	ITEHardware* h;
	HWTXVector2 p, r, s, t;
	HWTXVector2 aplusb, unitvec;
	HWTXVector2 tr;
	HWTXVector2 outdata;
	ITEs15p16 w, w2limint;
	HWTXVector2	c, m, u0, u1;
	ITEuint8 su0x, su0y, su1x, su1y;	// signed of unit vector
	ITEuint8 numofaxis, section, u0quad;
	ITEint i;

	// For caculate circule points
	HWTXVector2 interm, vec0, vec1;
	//HWTXVector2 unitm;					// s1.22
	HWTXVector2 arcm;					// s12.11
	ITEint64 squre;
	ITEuint16 divider, n;
	ITEint shift;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	if (EQ2V(h->strokerecm0 ,h->strokerecm3) || (!h->strokelastdash1 && !h->strokedash0)) return;

	if (!h->strokedash0) {
		// do end cap when no join
		SET2V(p, h->strokelastrecm2);
		SET2V(r, h->strokelastrecm1);
		SET2V(c, last);
		u0.x = -h->strokelastUnitV1.y;
		u0.y = h->strokelastUnitV1.x;

		iteDoCap(outCount, p, r, c, u0);
		return;
	}

	if (!h->strokelastdash1) {
		// do start cap when no join
		SET2V(p, h->strokerecm0);
		SET2V(r, h->strokerecm3);
		SET2V(c, h->strokeRoundC);
		u0.x = h->strokeUnitV0.y;
		u0.y = -h->strokeUnitV0.x;

		iteDoCap(outCount, p, r, c, u0);
		return;
	}

	if ( (ITEint64)h->strokelastVec1.x*h->strokeVec0.y - (ITEint64)h->strokelastVec1.y*h->strokeVec0.x >= 0)
	{
		SET2V(p, h->strokerecm0);
		SET2V(r, h->strokerecm3);
		SET2V(t, h->strokelastrecm2);
		SET2V(unitvec, h->strokelastUnitV1);

		u0.x = h->strokeUnitV0.y;
		u0.y = -h->strokeUnitV0.x;
		u1.x = h->strokelastUnitV1.y;
		u1.y = -h->strokelastUnitV1.x;
	} 
	else 
	{
		SET2V(p, h->strokelastrecm2);
		SET2V(r, h->strokelastrecm1);
		SET2V(t, h->strokerecm0);
		SET2V(unitvec, h->strokeUnitV0);

		u0.x = -h->strokelastUnitV1.y;
		u0.y = h->strokelastUnitV1.x;
		u1.x = -h->strokeUnitV0.y;
		u1.y = h->strokeUnitV0.x;
	}	

	h->cmdData[(*outCount)++] = 0xc0000001;
	HWAFFINETRANSFORM2TO(p, h->pathTransform, outdata);
	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);

	// parameter p is no more to use.
	h->cmdData[(*outCount)++] = 0xe0000000 | 0x1;
	HWAFFINETRANSFORM2TO(r, h->pathTransform, outdata);
	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);

	switch(h->strokeJoinStyle)
	{
	case 0: //HW_JOIN_MITER
		SET2V(aplusb, h->strokelastUnitV1);
		ADD2V(aplusb, h->strokeUnitV0);			// s1.22

		SET2V(tr, r);
		SUB2V(tr, t);							// s12.11

		// parameter r is no more to use.
		if ( (aplusb.x>>(POINTPREC*2-15)) != 0 )
			w = (ITEs15p16)( ((ITEint64)tr.x<<(21-POINTPREC))/(aplusb.x>>(POINTPREC*2-15)) );	// (s12.11<<10)/(s1.22>>7) = s12.6
		else
			w = (ITEs15p16)( ((ITEint64)tr.y<<(21-POINTPREC))/(aplusb.y>>(POINTPREC*2-15)) );	// (s12.11<<10)/(s1.22>>7) = s12.6

		w2limint = ((h->strokeMiterLimit*h->strokeMiterLimit)>>2) - ((h->lineWidth*h->lineWidth)>>(POINTPREC*2-6 +2));	
		// ( s12.3*s12.3 - (s12.11*s12.11>>16) ) = s12.6

		if (w2limint - w*w >= 0) 
		{
			s.x = t.x + (w*unitvec.x>>(POINTPREC+6));	// s12.11 + (s12.6*s1.22)>>17 = s12.11
			s.y = t.y + (w*unitvec.y>>(POINTPREC+6));	// s12.11 + (s12.6*s1.22)>>17 = s12.11

			h->cmdData[(*outCount)++] = 0xe0000000 | 0x1;
			HWAFFINETRANSFORM2TO(s, h->pathTransform, outdata);
			h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
			
			MIN2V(h->min, outdata);
			MAX2V(h->max, outdata);
		} 

		break;

	case 1: //HW_JOIN_ROUND

		h->cmdData[(*outCount)++] = 0xe0000000 | (h->strokeRoundLines-1);

		// parameter r is no more to use.
		c.x = h->strokeRoundC.x;
		c.y = h->strokeRoundC.y;

		// determine how many axises between u0 and u1 and start quadrant of u0
		su0x = (u0.x>>(POINTPREC*2+1))&0x1;
		su0y = (u0.y>>(POINTPREC*2+1))&0x1;
		su1x = (u1.x>>(POINTPREC*2+1))&0x1;
		su1y = (u1.y>>(POINTPREC*2+1))&0x1;

		if (u0.x==0 && su0y==1)		su0x=1;
		if (u0.y==0 && su0x==0)		su0y=1;
		if (u1.x==0 && su1y==0)		su1x=1;
		if (u1.y==0 && su1x==1)		su1y=1;

		if (su0x == su1x && su0y == su1y) {			// pass 0 axis
			numofaxis	= 0;
			section		= 1;
		} else if (su0x != su1x && su0y != su1y) {	// pass 2 axises
			numofaxis	= 2;
			section		= 3;
		} else {									// pass 1 axises
			numofaxis	= 1;
			section		= 2;
		}

		// ccw = -1 => Quadrant clock wise
		if (su0x==0 && su0y==0)
			u0quad = 0;
		else if (su0x==1 && su0y==0)
			u0quad = 3;
		else if (su0x==1 && su0y==1)
			u0quad = 2;
		else
			u0quad = 1;

		for(i=1;i<h->strokeRoundLines;i++)
		{
			iteDivideCircle(u0, u1, h->strokeRoundLines, i, numofaxis, section, -1, u0quad,
							&shift, &divider, &n, &vec0, &vec1);			

			interm.x = (ITEs15p16)( ( (ITEint64)vec0.x*(divider-n) + (ITEint64)vec1.x*n )>>shift );		// s1.22
			interm.y = (ITEs15p16)( ( (ITEint64)vec0.y*(divider-n) + (ITEint64)vec1.y*n )>>shift );		// s1.22

			squre = ( (ITEint64)interm.x*interm.x + (ITEint64)interm.y*interm.y) >> (POINTPREC*2);		// s3.22
/*
			// original version
			// s1.22 = (s1.22*s12.11)>>11
			unitm.x = (ITEs15p16)( ( (ITEint64)interm.x * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );
			unitm.y = (ITEs15p16)( ( (ITEint64)interm.y * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );	

			// (s1.22 * s12.11/2)>>22 + s12.11 = s12.11
			m.x = (ITEs15p16)( ((ITEint64)unitm.x*h->lineWidth>>(POINTPREC*2+1)) + c.x);	
			m.y = (ITEs15p16)( ((ITEint64)unitm.y*h->lineWidth>>(POINTPREC*2+1)) + c.y);	
*/
			// s12.11 = (s1.22*s12.11/2)>>22
			arcm.x = (ITEs15p16)( ( (ITEint64)interm.x * h->lineWidth ) >> (POINTPREC*2+1) );
			arcm.y = (ITEs15p16)( ( (ITEint64)interm.y * h->lineWidth ) >> (POINTPREC*2+1) );	

			// (s12.11 * s12.11)>>11 + s12.11 = s12.11
			m.x = (ITEs15p16)( ( ((ITEint64)arcm.x*iteInvSqurt(squre, POINTPREC*2))>>POINTPREC ) + c.x);	
			m.y = (ITEs15p16)( ( ((ITEint64)arcm.y*iteInvSqurt(squre, POINTPREC*2))>>POINTPREC ) + c.y);	

			HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
			h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
			MIN2V(h->min, outdata);
			MAX2V(h->max, outdata);
		}
		break;
	
	case 2: //HW_JOIN_BEVEL
		break;
	}
	
	h->cmdData[(*outCount)++] = 0xe0000000 | 0x1;
	HWAFFINETRANSFORM2TO(t, h->pathTransform, outdata);
	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		
}


static void iteStrokeLine(HWTXVector2* p0, HWTXVector2* p1, ITEuint* outCount, HWTXVector2* vec,
						  HWTXVector2* recm0, HWTXVector2* recm1, HWTXVector2* recm2, HWTXVector2* recm3, 
						  ITEint j, HWboolean* dash0, HWboolean* dash1)
{
/*   m0 ---------------------------- m1   */
/*		^ normalv		   		   |	  */
/*	 p0	|------------------------->| p1   */
/*		|		 vector			   |	  */
/*	 m3 ---------------------------- m2   */
	ITEHardware* h;
	ITEs15p16 width;
	ITEs15p16 dashRLength, helflength, linelength, dashlength, dashCount;
	HWboolean dashon=1, dashstartcap=0;
	HWTXVector2 m0, m1, m2, m3, mm0, mm3, u0;	
	HWTXVector2 vector, unitv;
	// HWTXVector2 normalv;
	HWTXVector2 outdata;
	ITEint64 xsqure, ysqure, square;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	width = (ITEs15p16)(h->lineWidth);	// s12.11
	
	vector.x = p1->x - p0->x;	// s12.11
	vector.y = p1->y - p0->y;	// s12.11

	xsqure = (ITEint64)vector.x*vector.x;	// 24.22
	ysqure = (ITEint64)vector.y*vector.y;	// 24.22
	square  = xsqure + ysqure;

	unitv.x = (ITEs15p16)( (ITEint64)vector.x * iteInvSqurt(square, POINTPREC*2) );
	unitv.y = (ITEs15p16)( (ITEint64)vector.y * iteInvSqurt(square, POINTPREC*2) );		// s1.22 = (s12.11*s12.11)

	linelength = (ITEs15p16)iteSqrt(square);											// s12.11 = (24.22)^0.5

	// for debug length
	if (DEBUGLENGTH)
		totallength = totallength + linelength;
	
	/*
	normalv.x = -unitv.y;	// s1.22
	normalv.y = unitv.x;	// s1.22

	m0.x = p0->x + (ITEs15p16)( ((ITEint64)normalv.x * width)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
	m0.y = p0->y + (ITEs15p16)( ((ITEint64)normalv.y * width)>>(POINTPREC*2+1) );
	m1.x = p1->x + (ITEs15p16)( ((ITEint64)normalv.x * width)>>(POINTPREC*2+1) );
	m1.y = p1->y + (ITEs15p16)( ((ITEint64)normalv.y * width)>>(POINTPREC*2+1) );
	m2.x = p1->x - (ITEs15p16)( ((ITEint64)normalv.x * width)>>(POINTPREC*2+1) );
	m2.y = p1->y - (ITEs15p16)( ((ITEint64)normalv.y * width)>>(POINTPREC*2+1) );
	m3.x = p0->x - (ITEs15p16)( ((ITEint64)normalv.x * width)>>(POINTPREC*2+1) );
	m3.y = p0->y - (ITEs15p16)( ((ITEint64)normalv.y * width)>>(POINTPREC*2+1) );
	*/

	m0.x = p0->x - (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );		// s12.11 + (s1.22 * s12.11)>>22
	m0.y = p0->y + (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );
	m3.x = p0->x + (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
	m3.y = p0->y - (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );

	if (j==1) {
		h->strokeRoundC.x = p0->x;
		h->strokeRoundC.y = p0->y;
	}

	SET2V(mm0, m0);
	SET2V(mm3, m3);

	if (h->enDashLine) {

		dashCount = h->dashCount;
		dashRLength = h->dashRLength;										// s12.11
		helflength = h->dashRLength;										// s12.11

		dashRLength = dashRLength - linelength;								// s12.11

		if (h->dashPattern[dashCount]>>(POINTPREC+12)) {
			dashon = 1;
			dashstartcap = 0;
			*dash0 = HW_TRUE;
			*dash1 = HW_TRUE;
		} else { 
			dashon = 0;
			dashstartcap = 0;
			*dash0 = HW_FALSE;
			*dash1 = HW_FALSE;
		}

		if (dashRLength<0) {		// ready to next dash line

			if (helflength!=0) {		

				if (dashCount==h->dashMaxCount)
					dashCount = 1;
				else
					dashCount++;

				p0->x = p0->x + (ITEs15p16)( ((ITEint64)unitv.x * helflength)>>(POINTPREC*2) );
				p0->y = p0->y + (ITEs15p16)( ((ITEint64)unitv.y * helflength)>>(POINTPREC*2) );

				m1.x = p0->x - (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
				m1.y = p0->y + (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );
				m2.x = p0->x + (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
				m2.y = p0->y - (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );

				if (dashon && ~EQ2V(mm0,mm3)) {
					h->cmdData[(*outCount)++] = 0xc0000001;
					HWAFFINETRANSFORM2TO(mm0, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm0
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					h->cmdData[(*outCount)++] = 0xe0000000 | 0x3;	// 0x4

					HWAFFINETRANSFORM2TO(m1, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m1
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					HWAFFINETRANSFORM2TO(m2, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m2
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					HWAFFINETRANSFORM2TO(mm3, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm3
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);
			
					SET2V(mm0, m1);
					SET2V(mm3, m2);
	
					// m1 and m2 are no more to use.
					// do end cap 
					h->dashRLength = dashRLength;
					h->dashCount = dashCount;

					u0.x = -unitv.y;		// s1.22
					u0.y = unitv.x;			// s1.22

					iteDoCap(outCount, mm3, mm0, *p0, u0);
					
					dashCount = h->dashCount;
					dashRLength = h->dashRLength;
				} else {
					// m1 and m2 are no more to use.
					SET2V(mm0, m1);
					SET2V(mm3, m2);
				}

				if (h->dashPattern[dashCount]>>(POINTPREC+12)) {
					dashon = 1;
					dashstartcap = 1;
					*dash1 = HW_TRUE;
				} else {
					dashon = 0;
					dashstartcap = 0;
					*dash1 = HW_FALSE;
				}
			}

			dashlength = h->dashPattern[dashCount] & ((1<<(POINTPREC+12))-1);
			dashRLength = dashRLength + dashlength;

			if (dashRLength<0) {

				while (dashRLength<0) {	// ready to next dash line

					if (dashon && ~EQ2V(mm0,mm3) && dashstartcap) {
				
						h->dashRLength = dashRLength;
						h->dashCount = dashCount;

						// do start cap
						u0.x = unitv.y;		// s1.22
						u0.y = -unitv.x;	// s1.22

						iteDoCap(outCount, mm0, mm3, *p0, u0);	
						
						dashCount = h->dashCount;
						dashRLength = h->dashRLength;
					}

					p0->x = p0->x + (ITEs15p16)( ((ITEint64)unitv.x * dashlength)>>(POINTPREC*2) );
					p0->y = p0->y + (ITEs15p16)( ((ITEint64)unitv.y * dashlength)>>(POINTPREC*2) );

					m1.x = p0->x - (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
					m1.y = p0->y + (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );
					m2.x = p0->x + (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
					m2.y = p0->y - (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );

					if (dashon && ~EQ2V(mm0,mm3)) {
						h->cmdData[(*outCount)++] = 0xc0000001;
						HWAFFINETRANSFORM2TO(mm0, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm0
						MIN2V(h->min, outdata);
						MAX2V(h->max, outdata);

						h->cmdData[(*outCount)++] = 0xe0000000 | 0x3;	// 0x4

						HWAFFINETRANSFORM2TO(m1, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m1
						MIN2V(h->min, outdata);
						MAX2V(h->max, outdata);

						HWAFFINETRANSFORM2TO(m2, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m2
						MIN2V(h->min, outdata);
						MAX2V(h->max, outdata);

						HWAFFINETRANSFORM2TO(mm3, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm3
						MIN2V(h->min, outdata);
						MAX2V(h->max, outdata);
		
						SET2V(mm0, m1);
						SET2V(mm3, m2);
			
						// m1 and m2 are no more to use.
						// do end cap
						h->dashRLength = dashRLength;
						h->dashCount = dashCount;
						
						u0.x = -unitv.y;		// s1.22
						u0.y = unitv.x;			// s1.22

						iteDoCap(outCount, mm3, mm0, *p0, u0);
						
						dashCount = h->dashCount;
						dashRLength = h->dashRLength;				
					} else {
						// m1 and m2 are no more to use.
						SET2V(mm0, m1);
						SET2V(mm3, m2);
					}

					if (dashCount==h->dashMaxCount)
						dashCount = 1;
					else
						dashCount++;		

					if (h->dashPattern[dashCount]>>(POINTPREC+12)) {
						dashon = 1;
						dashstartcap = 1;
						*dash1 = HW_TRUE;
					} else {
						dashon = 0;
						dashstartcap = 0;
						*dash1 = HW_FALSE;
					}

					dashlength = h->dashPattern[dashCount] & ((1<<(POINTPREC+12))-1);
					dashRLength = dashRLength + dashlength;
				}
				
			}
		}
		h->dashRLength = dashRLength;
		h->dashCount = dashCount;
	}

	m1.x = p1->x - (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
	m1.y = p1->y + (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );
	m2.x = p1->x + (ITEs15p16)( ((ITEint64)unitv.y * width)>>(POINTPREC*2+1) );
	m2.y = p1->y - (ITEs15p16)( ((ITEint64)unitv.x * width)>>(POINTPREC*2+1) );

	// for cap and join
	if (j==1) {					
		SET2V(h->strokelastrecm1, h->strokerecm1);
		SET2V(h->strokelastrecm2, h->strokerecm2);
		SET2V(h->strokerecm0, m0);
		SET2V(h->strokerecm3, m3);	

		SET2V(h->strokelastUnitV1, h->strokeUnitV1);
		SET2V(h->strokelastVec1, h->strokeVec1);
		SET2V(h->strokeUnitV0, unitv);
		SET2V(h->strokeVec0, vector);
		h->strokelastdash1 = h->strokedash1;
		h->strokedash0 = *dash0;

		// To remember first two point to close
		/*
		if(h->LastisMove) {
			SET2V(h->strokestartrecm0, m0);
			SET2V(h->strokestartrecm3, m3);
			SET2V(h->strokestartUnitV0, unitv);
			SET2V(h->strokestartVec0, vector);
			h->strokestartdash0 = *dash0;
			h->LastisMove = 0;
		}	
		*/
		if(h->LastisMove) {
			SET2V(h->strokestartrecm0, h->strokerecm0);
			SET2V(h->strokestartrecm3, h->strokerecm3);
			SET2V(h->strokestartUnitV0, h->strokeUnitV0);
			SET2V(h->strokestartVec0, h->strokeVec0);
			h->strokestartdash0 = h->strokedash0;
			h->LastisMove = 0;
		}
	}
	SET2V(h->strokerecm1, m1);
	SET2V(h->strokerecm2, m2);

	SET2V(h->strokeUnitV1, unitv);
	SET2V(h->strokeVec1, vector);
	h->strokedash1 = *dash1;

	// dashon default value is 1 for no dash line case.
	if (dashon && ~EQ2V(mm0,mm3)) {
	
		h->cmdData[(*outCount)++] = 0xc0000001;
		HWAFFINETRANSFORM2TO(mm0, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm0
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);

		h->cmdData[(*outCount)++] = 0xe0000000 | 0x3;	// 0x4

		HWAFFINETRANSFORM2TO(m1, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m1
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);

		HWAFFINETRANSFORM2TO(m2, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// m2
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);

		HWAFFINETRANSFORM2TO(mm3, h->pathTransform, outdata);
		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);		// mm3
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);

		// dashstartcap default value is 0 for no dash line case.
		if (dashstartcap) {
			// do start cap
			u0.x = unitv.y;		// s1.22
			u0.y = -unitv.x;	// s1.22

			iteDoCap(outCount, mm0, mm3, *p0, u0);
		}
	}

	recm0->x = m0.x;	
	recm0->y = m0.y;
	recm1->x = m1.x;	
	recm1->y = m1.y;
	recm2->x = m2.x;	
	recm2->y = m2.y;
	recm3->x = m3.x;	
	recm3->y = m3.y;
	vec->x = vector.x;			// s12.11	protect error term of unit vector to fill wrong triangle
	vec->y = vector.y;			// s12.11

}

static void iteMove(HWTXVector2* p0, ITEuint* outCount)
{
	ITEHardware* h;
	HWTXVector2 tmp, outdata;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	h->cmdData[(*outCount)++] = 0xc0000001;
	tmp.x = p0->x;
	tmp.y = p0->y;

	if (h->enPerspective) {
		HWPERSPECTIVETRANSFORM3TO(tmp, h->pathTransform, outdata);
	}
	else {
		HWAFFINETRANSFORM2TO(tmp, h->pathTransform, outdata);
	}

	h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
	MIN2V(h->min, outdata);
	MAX2V(h->max, outdata);

}

static void iteClose(ITEuint* outCount)
{
	ITEHardware* h;
	ITEint j;
	HWTXVector2 recm0, recm1, recm2, recm3;	// s12.11
	HWTXVector2 vec;						// s12.11
	HWTXVector2 premm, mm;
	HWboolean dash0=1, dash1=1;
	//HWTXVector2 outdata;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	SET2V(premm, last);
	SET2V(mm, start);
	j = 1;

	iteStrokeLine(&premm, &mm, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
				  j, &dash0, &dash1);

}

static void iteLine(HWTXVector2* p0, HWTXVector2* p1, ITEuint* outCount)
{
	ITEHardware* h;
	ITEint j;
	HWTXVector2 recm0, recm1, recm2, recm3;		// s12.11
	HWTXVector2 vec;							// s12.11
	HWTXVector2 premm, mm;
	HWboolean dash0=1, dash1=1;
	HWTXVector2 tmp, outdata;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	premm.x = p0->x;
	premm.y = p0->y;
	mm.x = p1->x;
	mm.y = p1->y;

	j = 1;
	
	if(h->paintMode)  // HW_STROKE_PATH
	{
		iteStrokeLine(&premm, &mm, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
					  j, &dash0, &dash1);
	}
	else
	{
		h->cmdData[(*outCount)++] = 0xe0000001;
		tmp.x = p1->x;
		tmp.y = p1->y;
		
		if (h->enPerspective) {
			HWPERSPECTIVETRANSFORM3TO(tmp, h->pathTransform, outdata);
		}
		else {
			HWAFFINETRANSFORM2TO(tmp, h->pathTransform, outdata);
		}

		h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
		MIN2V(h->min, outdata);
		MAX2V(h->max, outdata);
	}
}

static void iteQuad(HWTXVector2* p0, HWTXVector2* p1, HWTXVector2* p2,
					ITEuint16 lines, ITEuint* outCount)
{
	ITEHardware* h;
	ITEint i,j,l,s;
	ITEuint32 pointer;
	HWTXVector2 m0, m1, premm, mm;
	HWTXVector2 prevec, vec;
	HWboolean dash0=1, dash1=1, predash=1;
	HWTXVector2 outdata;
	HWTXVector2 recm0, recm1, recm2, recm3, prerecm1, prerecm2;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if(!h->paintMode)  // HW_FILL_PATH
	{
		h->cmdData[(*outCount)++] = 0xe0000000 | lines;
		pointer = (*outCount)-1;
	}

	premm.x = p0->x;
	premm.y = p0->y;
	j = 1;
	l = 0;

	if (lines == 2)			s=1;
	if (lines == 4)			s=2;
	if (lines == 8)			s=3;
	if (lines == 16)		s=4;
	if (lines == 32)		s=5;
	if (lines == 64)		s=6;
	if (lines == 128)		s=7;
	if (lines == 256)		s=8;
	if (lines == 512)		s=9;
	if (lines == 1024)		s=10;
	if (lines == 2048)		s=11;
	if (lines == 4096)		s=12;

	for(i=1;i<=lines;i++)
	{
//		premm = mm;
		m0.x = (ITEs15p16)((( ((ITEint)p1->x-p0->x)*i )>>s) + p0->x);		// s12.11
		m0.y = (ITEs15p16)((( ((ITEint)p1->y-p0->y)*i )>>s) + p0->y);
		m1.x = (ITEs15p16)((( ((ITEint)p2->x-p1->x)*i )>>s) + p1->x);
		m1.y = (ITEs15p16)((( ((ITEint)p2->y-p1->y)*i )>>s) + p1->y);
		mm.x = (ITEs15p16)((( ((ITEint)m1.x-m0.x)*i )>>s) + m0.x);
		mm.y = (ITEs15p16)((( ((ITEint)m1.y-m0.y)*i )>>s) + m0.y);

		if (premm.x!=mm.x || premm.y!=mm.y)
		{	
			if(h->paintMode)  // HW_STROKE_PATH
			{
				iteStrokeLine(&premm, &mm, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
							  j, &dash0, &dash1);

				if (j==0 && ~EQ2V(recm0,recm3) && predash && dash0)
				{
					h->cmdData[(*outCount)++] = 0xc0000001;
					HWAFFINETRANSFORM2TO(recm0, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					h->cmdData[(*outCount)++] = 0xe0000000 | 0x2;
					HWAFFINETRANSFORM2TO(recm3, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);
				
					if ( (ITEint64)prevec.x*vec.y-(ITEint64)prevec.y*vec.x >= 0 )
					{
						HWAFFINETRANSFORM2TO(prerecm2, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					} else {
						HWAFFINETRANSFORM2TO(prerecm1, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					}
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);
				}
				SET2V(premm,mm);
				SET2V(prevec,vec);
				SET2V(prerecm1,recm1);
				SET2V(prerecm2,recm2);
				predash = dash1;
				j = 0;
			}
			else
			{
				HWAFFINETRANSFORM2TO(mm, h->pathTransform, outdata);
				h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
				SET2V(premm,mm);
				l++;
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);
			}
		}
	}

	if(!h->paintMode)  // HW_FILL_PATH
		h->cmdData[pointer] = 0xe0000000 | l;

}

static void iteCubic(HWTXVector2* p0, HWTXVector2* p1, HWTXVector2* p2, HWTXVector2* p3,
					 ITEuint16 lines, ITEuint* outCount)
{
	ITEHardware* h;
	ITEint i,j,l,s;
	ITEuint32 pointer;
	HWTXVector2 m0, m1, m2, mm0, mm1, premmm, mmm;
	HWTXVector2 prevec, vec;
	HWboolean dash0=1, dash1=1, predash=1;
	HWTXVector2 recm0, recm1, recm2, recm3, prerecm1, prerecm2;
	HWTXVector2 outdata;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if(!h->paintMode)  // HW_FILL_PATH
	{
		h->cmdData[(*outCount)++] = 0xe0000000 | lines;
		pointer = (*outCount)-1;
	}

	premmm.x = p0->x;
	premmm.y = p0->y;
	j = 1;
	l = 0;

	if (lines == 2)			s=1;
	if (lines == 4)			s=2;
	if (lines == 8)			s=3;
	if (lines == 16)		s=4;
	if (lines == 32)		s=5;
	if (lines == 64)		s=6;
	if (lines == 128)		s=7;
	if (lines == 256)		s=8;
	if (lines == 512)		s=9;
	if (lines == 1024)		s=10;
	if (lines == 2048)		s=11;
	if (lines == 4096)		s=12;

	for(i=1;i<=lines;i++)
	{
//		premmm = mmm;
		m0.x  = (ITEs15p16)((( ((ITEint)p1->x-p0->x)*i )>>s) + p0->x);
		m0.y  = (ITEs15p16)((( ((ITEint)p1->y-p0->y)*i )>>s) + p0->y);
		m1.x  = (ITEs15p16)((( ((ITEint)p2->x-p1->x)*i )>>s) + p1->x);
		m1.y  = (ITEs15p16)((( ((ITEint)p2->y-p1->y)*i )>>s) + p1->y);
		m2.x  = (ITEs15p16)((( ((ITEint)p3->x-p2->x)*i )>>s) + p2->x);
		m2.y  = (ITEs15p16)((( ((ITEint)p3->y-p2->y)*i )>>s) + p2->y);
		mm0.x = (ITEs15p16)((( ((ITEint)m1.x-m0.x)*i )>>s) + m0.x);
		mm0.y = (ITEs15p16)((( ((ITEint)m1.y-m0.y)*i )>>s) + m0.y);
		mm1.x = (ITEs15p16)((( ((ITEint)m2.x-m1.x)*i )>>s) + m1.x);
		mm1.y = (ITEs15p16)((( ((ITEint)m2.y-m1.y)*i )>>s) + m1.y);
		mmm.x = (ITEs15p16)((( ((ITEint)mm1.x-mm0.x)*i )>>s) + mm0.x);
		mmm.y = (ITEs15p16)((( ((ITEint)mm1.y-mm0.y)*i )>>s) + mm0.y);

		if (premmm.x!=mmm.x || premmm.y!=mmm.y)
		{

			if(h->paintMode)  // HW_STROKE_PATH
			{
				iteStrokeLine(&premmm, &mmm, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
						      j, &dash0, &dash1);
				
				if (j==0 && ~EQ2V(recm0,recm3) && predash && dash0)
				{
					h->cmdData[(*outCount)++] = 0xc0000001;
					HWAFFINETRANSFORM2TO(recm0, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);		

					h->cmdData[(*outCount)++] = 0xe0000000 | 0x2;
					HWAFFINETRANSFORM2TO(recm3, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);		
	
					if ( (ITEint64)prevec.x*vec.y-(ITEint64)prevec.y*vec.x >= 0 )
					{
						HWAFFINETRANSFORM2TO(prerecm2, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					} else {
						HWAFFINETRANSFORM2TO(prerecm1, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					}
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);		
				}
				SET2V(premmm,mmm);
				SET2V(prevec,vec);
				SET2V(prerecm1,recm1);
				SET2V(prerecm2,recm2);
				predash = dash1;
				j = 0;
			}
			else
			{
				HWAFFINETRANSFORM2TO(mmm, h->pathTransform, outdata);
				h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
				SET2V(premmm,mmm);
				l++;
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);
			}
		}
	}

	if(!h->paintMode)  // HW_FILL_PATH
		h->cmdData[pointer] = 0xe0000000 | l;
}

static char itefindUnitCircles(ITEs15p16 x0p, ITEs15p16 y0p, ITEs15p16 x1p, ITEs15p16 y1p, ITEuint segment, 
							   HWTXVector2* cp, HWTXVector2* u0, HWTXVector2* u1)
{
	ITEs15p16 dx = x0p - x1p;			// 0<= dx <=2, s2.11
	ITEs15p16 dy = y0p - y1p;			// 0<= dy <=2, s2.11
	ITEs15p16 xm = (x0p + x1p) >> 1;	// s12.11
	ITEs15p16 ym = (y0p + y1p) >> 1;	// s12.11
	ITEs15p16 dsq, s, sdx, sdy;
	ITEint64 disc;

	dsq = (ITEs15p16)( ((ITEint64)dx*dx + (ITEint64)dy*dy)>>12 );	// s3.10

	if(dsq <= 0)
		return 0;	//the points are coincident

	disc = (ITEint64)((ITEint64)1<<32)/dsq - ((ITEint64)1<<(22-2));	// (s1<<32)/(s3.10) - s3.22 = s11.22

	if (disc < 0)
		return 0;	//the points are too far apart
	
	s = (ITEs15p16)iteSqrt(disc);			// s6.11

    sdx = (ITEs15p16)( (ITEint64)dx*s );	// (s12.11*s6.11) = s1.22
	sdy = (ITEs15p16)( (ITEint64)dy*s );	// (s12.11*s6.11) = s1.22

	//choose the center point and direction
	//	VG_SCCWARC_TO	4
	//	VG_SCWARC_TO	5
	//	VG_LCCWARC_TO	6
	//	VG_LCWARC_TO	7
	
	//move the unit circle origin to the chosen center point
	//	u0->x = x0p - cp->x;
	//	u0->y = y0p - cp->y;
	//	u1->x = x1p - cp->x;
	//	u1->y = y1p - cp->y;
	if(segment == (VG_SCCWARC_TO/2-5) || segment == (VG_LCWARC_TO/2-5))
	{
		cp->x = xm + (( sdy)>>POINTPREC);				// s12.11
		cp->y = ym + ((-sdx)>>POINTPREC);

		u0->x = ((x0p-xm)<<POINTPREC) - sdy;			// s1.22
		u0->y = ((y0p-ym)<<POINTPREC) + sdx;			// s1.22
		u1->x = ((x1p-xm)<<POINTPREC) - sdy;			// s1.22
		u1->y = ((y1p-ym)<<POINTPREC) + sdx;			// s1.22

	}

	if(segment == (VG_SCWARC_TO/2-5) || segment == (VG_LCCWARC_TO/2-5))
	{
		cp->x = xm + ((-sdy)>>POINTPREC);				// s12.11
		cp->y = ym + (( sdx)>>POINTPREC);
	
		u0->x = ((x0p-xm)<<POINTPREC) + sdy;			// s1.22
		u0->y = ((y0p-ym)<<POINTPREC) - sdx;			// s1.22
		u1->x = ((x1p-xm)<<POINTPREC) + sdy;			// s1.22
		u1->y = ((y1p-ym)<<POINTPREC) - sdx;			// s1.22
	}

	if( u0->x==u1->x && u0->y==u1->y )
		return 0;

	return 1;
}


static void iteArc(HWTXVector2 cp, HWTXVector2* p0, HWTXVector2* p1, HWTXVector2 u0, HWTXVector2 u1, ITEuint segment,
				   ITEs15p16 rhsinrot, ITEs15p16 rhcosrot, ITEs15p16 rvsinrotN, ITEs15p16 rvcosrot,
				   ITEuint16 lines, ITEuint* outCount)
				   
{
	ITEHardware* h;
	ITEint i,j,l;
	ITEuint32 pointer;

	HWTXVector2 prem, m;
	HWTXVector2 prevec, vec;
	HWboolean dash0=1, dash1=1, predash=1;
	HWTXVector2 recm0, recm1, recm2, recm3,  prerecm1, prerecm2;
	HWTXVector2 outdata;
	ITEuint8 su0x, su0y, su1x, su1y;	// signed of unit vector
	ITEuint8 numofaxis, section, ccw, u0quad;

	// For caculate circule points
	HWTXVector2 vec0, vec1;
	HWTXVector2 interm;
	//HWTXVector2 unitm;					// s1.22
	HWTXVector2 arcm1, arcm2;			// s12.11
	ITEint64 squre;
	ITEuint16 divider, n;
	ITEint shift;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if(!h->paintMode)  // HW_FILL_PATH
	{
		h->cmdData[(*outCount)++] = 0xe0000000 | lines;
		pointer = (*outCount)-1;
	}

	prem.x = p0->x;
	prem.y = p0->y;
	// no more to use p0
	j = 1;
	l = 0;

	//	VG_SCCWARC_TO	4
	//	VG_SCWARC_TO	5
	//	VG_LCCWARC_TO	6
	//	VG_LCWARC_TO	7
	if (segment==(VG_SCWARC_TO/2-5) || segment==(VG_LCWARC_TO/2-5))
		ccw = -1;
	else
		ccw = 1;

	// determine how many axises between u0 and u1 and start quadrant of u0
	su0x = (u0.x>>(POINTPREC*2+1))&0x1;
	su0y = (u0.y>>(POINTPREC*2+1))&0x1;
	su1x = (u1.x>>(POINTPREC*2+1))&0x1;
	su1y = (u1.y>>(POINTPREC*2+1))&0x1;

	if (u0.x==0 && su0y==0 && ccw==1)		su0x=1;
	if (u0.y==0 && su0x==1 && ccw==1)		su0y=1;
	if (u1.x==0 && su1y==1 && ccw==1)		su1x=1;
	if (u1.y==0 && su1x==0 && ccw==1)		su1y=1;

	if (u0.x==0 && su0y==1 && ccw!=1)		su0x=1;
	if (u0.y==0 && su0x==0 && ccw!=1)		su0y=1;
	if (u1.x==0 && su1y==0 && ccw!=1)		su1x=1;
	if (u1.y==0 && su1x==1 && ccw!=1)		su1y=1;

	//	VG_SCCWARC_TO	4
	//	VG_SCWARC_TO	5
	//	VG_LCCWARC_TO	6
	//	VG_LCWARC_TO	7
	if (segment==(VG_SCWARC_TO/2-5) || segment==(VG_SCCWARC_TO/2-5)) 
	{
		if (su0x == su1x && su0y == su1y) {			// pass 0 axis
			numofaxis	= 0;
			section		= 1;
		} else if (su0x != su1x && su0y != su1y) {	// pass 2 axises
			numofaxis	= 2;
			section		= 3;
		} else {									// pass 1 axises
			numofaxis	= 1;
			section		= 2;
		}
	}
	else 
	{
		if (su0x == su1x && su0y == su1y) {			// pass 4 axises
			numofaxis	= 4;
			section		= 5;
		} else if (su0x != su1x && su0y != su1y) {	// pass 2 axises
			numofaxis	= 2;
			section		= 3;
		} else {									// pass 3 axises
			numofaxis	= 3;
			section		= 4;
		}
	}

	// ccw= 1	=> Quadrant clock counter wise
	// ccw=-1	=> Quadrant clock wise
	if (ccw==1) {
		if (su0x==0 && su0y==0)
			u0quad = 0;
		else if (su0x==1 && su0y==0)
			u0quad = 1;
		else if (su0x==1 && su0y==1)
			u0quad = 2;
		else
			u0quad = 3;
	} else {
		if (su0x==0 && su0y==0)
			u0quad = 0;
		else if (su0x==1 && su0y==0)
			u0quad = 3;
		else if (su0x==1 && su0y==1)
			u0quad = 2;
		else
			u0quad = 1;
	}

	// Start divide
	for(i=1;i<lines;i++)
	{
		iteDivideCircle(u0, u1, lines, i, numofaxis, section, ccw, u0quad,
						&shift, &divider, &n, &vec0, &vec1);			

		// caculate unit vector
		interm.x = (ITEs15p16)( ( (ITEint64)vec0.x*(divider-n) + (ITEint64)vec1.x*n )>>shift );		// s1.22
		interm.y = (ITEs15p16)( ( (ITEint64)vec0.y*(divider-n) + (ITEint64)vec1.y*n )>>shift );		// s1.22

		squre = ( (ITEint64)interm.x*interm.x + (ITEint64)interm.y*interm.y ) >> (POINTPREC*2);		// s3.22
/*
		// original version
		// s1.22 = (s1.22*s12.11)>>11
		unitm.x = (ITEs15p16)( ( (ITEint64)interm.x * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );
		unitm.y = (ITEs15p16)( ( (ITEint64)interm.y * iteInvSqurt(squre, POINTPREC*2) ) >> POINTPREC );	

		//(s12.11*s1.22)>>22 + (s12.11*s1.22)>>22 + s12.11
		m.x = (ITEs15p16)( ( (ITEint64)rhcosrot*unitm.x + (ITEint64)rvsinrotN*unitm.y )>>(POINTPREC*2) ) + cp.x;	
		m.y = (ITEs15p16)( ( (ITEint64)rhsinrot*unitm.x + (ITEint64)rvcosrot*unitm.y )>>(POINTPREC*2) ) + cp.y;		
*/
		// s12.11 = (s1.22*s12.11)>>22  (should be s14.11. However, interm.x <= 1)
		arcm1.x = (ITEs15p16)( ( (ITEint64)interm.x * rhcosrot ) >> (POINTPREC*2) );
		arcm2.x = (ITEs15p16)( ( (ITEint64)interm.y * rvsinrotN) >> (POINTPREC*2) );
		arcm1.y = (ITEs15p16)( ( (ITEint64)interm.x * rhsinrot ) >> (POINTPREC*2) );	
		arcm2.y = (ITEs15p16)( ( (ITEint64)interm.y * rvcosrot ) >> (POINTPREC*2) );	

		//(s12.11*s12.11)>>11 + (s12.11*s12.11)>>11 + s12.11
		m.x = (ITEs15p16)( ( (ITEint64)arcm1.x*iteInvSqurt(squre, POINTPREC*2) + 
						     (ITEint64)arcm2.x*iteInvSqurt(squre, POINTPREC*2) )>>POINTPREC ) + cp.x;	
		m.y = (ITEs15p16)( ( (ITEint64)arcm1.y*iteInvSqurt(squre, POINTPREC*2) + 
						     (ITEint64)arcm2.y*iteInvSqurt(squre, POINTPREC*2) )>>POINTPREC ) + cp.y;	

		if (prem.x!=m.x || prem.y!=m.y)
		{

			if(h->paintMode)  // HW_STROKE_PATH
			{
				iteStrokeLine(&prem, &m, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
							  j, &dash0, &dash1);

				if (j==0 && ~EQ2V(recm0,recm3) && predash && dash0)
				{
					h->cmdData[(*outCount)++] = 0xc0000001;
					HWAFFINETRANSFORM2TO(recm0, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					h->cmdData[(*outCount)++] = 0xe0000000 | 0x2;
					HWAFFINETRANSFORM2TO(recm3, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);

					if ((ITEint64)prevec.x*vec.y-(ITEint64)prevec.y*vec.x >= 0)
					{
						HWAFFINETRANSFORM2TO(prerecm2, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
					} else {
						HWAFFINETRANSFORM2TO(prerecm1, h->pathTransform, outdata);
						h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
					}
					MIN2V(h->min, outdata);
					MAX2V(h->max, outdata);
				}
				SET2V(prem,m);
				SET2V(prevec,vec);
				SET2V(prerecm1,recm1);
				SET2V(prerecm2,recm2);
				predash = dash1;
				j = 0;
			}
			else
			{
				HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
				h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
				SET2V(prem,m);
				l++;
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);
			}
		}
	}

	// i==lines.  In order to prevent calculation error.
	m.x = p1->x;
	m.y = p1->y;
	
	if (prem.x!=m.x || prem.y!=m.y)
	{

		if(h->paintMode)  // HW_STROKE_PATH
		{
			iteStrokeLine(&prem, &m, outCount, &vec, &recm0, &recm1, &recm2, &recm3, 
						  j, &dash0, &dash1);

			if (j==0 && ~EQ2V(recm0,recm3) && predash && dash0)
			{
				h->cmdData[(*outCount)++] = 0xc0000001;
				HWAFFINETRANSFORM2TO(recm0, h->pathTransform, outdata);
				h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);

				h->cmdData[(*outCount)++] = 0xe0000000 | 0x2;
				HWAFFINETRANSFORM2TO(recm3, h->pathTransform, outdata);
				h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);

				if ((ITEint64)prevec.x*vec.y-(ITEint64)prevec.y*vec.x >= 0)
				{
					HWAFFINETRANSFORM2TO(prerecm2, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
				} else {
					HWAFFINETRANSFORM2TO(prerecm1, h->pathTransform, outdata);
					h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);	
				}
				MIN2V(h->min, outdata);
				MAX2V(h->max, outdata);
			}
		}
		else
		{
			HWAFFINETRANSFORM2TO(m, h->pathTransform, outdata);
			h->cmdData[(*outCount)++] = ((ITEuint)outdata.y<<16) | (outdata.x);
			l++;
			MIN2V(h->min, outdata);
			MAX2V(h->max, outdata);
		}
	}

	if(!h->paintMode)  // HW_FILL_PATH
		h->cmdData[pointer] = 0xe0000000 | l;
}


void iteTessllationEngine()
{
	
	ITEHardware* h;
	HWTXVector2 p0, p1, p2, p3;
	HWTXVector2 u0, u1, cp, c, p, r;
	ITEs15p16 x0p, y0p, x1p, y1p;
	ITEs15p16 rhsinrot, rhcosrot, rvsinrotN, rvcosrot;
	ITEint	inCount = 0;
	ITEint	outCount = 0;
	HWboolean  doCap, doJoin, dashon;
	ITEint  lastCmdMove = 0;
	ITEint  segment;
	ITEuint16 lines;
	char	i;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
  
	// for debug length
	if (DEBUGLENGTH)
		totallength = 0;

	h->dashRLength = 0;
	h->dashCount = 0;
	h->LastisMove = 1;
	doCap = 1 & h->paintMode;
	doJoin = 0 & h->paintMode;
	start.x = 0;
	start.y = 0;
	last = start;

	if (h->dashMaxCount!=0)
		h->enDashLine = 1;
	else
		h->enDashLine = 0;

	while((h->tessellateCmd[inCount]>>30) != 0x2)
	{
		segment = (h->tessellateCmd[inCount]>>24)&0xf;
		lines = h->tessellateCmd[inCount]&0xfff;

		switch(segment)
		{
		case 0: // MOVE_TO
			if(doCap && !h->LastisMove)		// Only HW_STROKE_PATH and avoid double move.
			{
				dashon = h->strokedash1;
				if (dashon) {
					// do end cap
					SET2V(p, h->strokerecm2);
					SET2V(r, h->strokerecm1);
					SET2V(c, last);
					u0.x = -h->strokeUnitV1.y;		// s1.22
					u0.y = h->strokeUnitV1.x;		// s1.22

					iteDoCap(&outCount, p, r, c, u0);
				}

				dashon = h->strokestartdash0;
				if (dashon) {
					// do start cap
					SET2V(p, h->strokestartrecm0);
					SET2V(r, h->strokestartrecm3);
					SET2V(c, start);
					u0.x = h->strokestartUnitV0.y;		// s1.22
					u0.y = -h->strokestartUnitV0.x;		// s1.22
		
					iteDoCap(&outCount, p, r, c, u0);
				}
			}
			// For HW_STROKE_PATH
			doCap = 1 & h->paintMode;
			doJoin = 0 & h->paintMode;
			h->LastisMove = 1;
			
			if (h->dashPhaseReset) {
				h->dashCount = 0;
				h->dashRLength = 0;
			}

			start.x = (ITEs15p16)h->tessellateCmd[inCount+1] >> (16-POINTPREC);
			start.y = (ITEs15p16)h->tessellateCmd[inCount+2] >> (16-POINTPREC);
			last = start;

			if (!h->paintMode)		// HW_FILL_PATH
				iteMove(&start, &outCount);

			inCount += 3;
			break;
		case 1: // LINE_TO
			p0 = last;
			p1.x = (ITEs15p16)h->tessellateCmd[inCount+1] >> (16-POINTPREC);
			p1.y = (ITEs15p16)h->tessellateCmd[inCount+2] >> (16-POINTPREC);
			last = p1;
			inCount += 3;
			
			if (p0.x!=p1.x || p0.y!=p1.y) {

				iteLine(&p0, &p1, &outCount);

				if(doJoin)			// Only HW_STROKE_PATH
					iteDoJoin(&outCount);
				
				// For HW_STROKE_PATH
				doCap = 1 & h->paintMode;
				doJoin = 1 & h->paintMode;
			}
			break;
		case 2: // QUAD_TO
			p0 = last;
			p1.x = (ITEs15p16)h->tessellateCmd[inCount+1] >> (16-POINTPREC);
			p1.y = (ITEs15p16)h->tessellateCmd[inCount+2] >> (16-POINTPREC);
			p2.x = (ITEs15p16)h->tessellateCmd[inCount+3] >> (16-POINTPREC);
			p2.y = (ITEs15p16)h->tessellateCmd[inCount+4] >> (16-POINTPREC);
			last = p2;
			inCount += 5;
			
			if ( (p0.x!=p1.x || p0.y!=p1.y) || (p1.x!=p2.x || p1.y!=p2.y) ) {
				
				iteQuad(&p0, &p1, &p2, lines, &outCount);

				if(doJoin)			// Only HW_STROKE_PATH
					iteDoJoin(&outCount);

				// For HW_STROKE_PATH
				doCap = 1 & h->paintMode;
				doJoin = 1 & h->paintMode;
			}
			break;
		case 3: // CUBIC_TO
			p0 = last;
			p1.x = (ITEs15p16)h->tessellateCmd[inCount+1] >> (16-POINTPREC);  
			p1.y = (ITEs15p16)h->tessellateCmd[inCount+2] >> (16-POINTPREC);  
			p2.x = (ITEs15p16)h->tessellateCmd[inCount+3] >> (16-POINTPREC);
			p2.y = (ITEs15p16)h->tessellateCmd[inCount+4] >> (16-POINTPREC);
			p3.x = (ITEs15p16)h->tessellateCmd[inCount+5] >> (16-POINTPREC);
			p3.y = (ITEs15p16)h->tessellateCmd[inCount+6] >> (16-POINTPREC);
			last = p3;
			inCount += 7;
		
			if ( (p0.x!=p1.x || p0.y!=p1.y) || (p1.x!=p2.x || p1.y!=p2.y) || (p2.x!=p3.x || p2.y!=p3.y) ) {

				iteCubic(&p0, &p1, &p2, &p3, lines, &outCount);

				if(doJoin)			// Only HW_STROKE_PATH
					iteDoJoin(&outCount);

				// For HW_STROKE_PATH
				doCap = 1 & h->paintMode;
				doJoin = 1 & h->paintMode;
			}
			break;
		case 4: // SCWARC_TO
		case 5: // SCCWARC_TO
		case 6: // LCWARC_TO
		case 7: // LCCWARC_TO
			p0 = last;	
			p1.x = (ITEs15p16)h->tessellateCmd[inCount+1] >> (16-POINTPREC);		//p1.x			s12.11
			p1.y = (ITEs15p16)h->tessellateCmd[inCount+2] >> (16-POINTPREC);		//p1.y			s12.11
			x0p = (ITEs15p16)h->tessellateCmd[inCount+3] >> (16-POINTPREC);			//x0p			s12.11
			y0p = (ITEs15p16)h->tessellateCmd[inCount+4] >> (16-POINTPREC);			//y0p			s12.11
			x1p = (ITEs15p16)h->tessellateCmd[inCount+5] >> (16-POINTPREC);			//x1p			s12.11
			y1p = (ITEs15p16)h->tessellateCmd[inCount+6] >> (16-POINTPREC);			//y1p			s12.11
			rhcosrot = (ITEs15p16)h->tessellateCmd[inCount+7] >> (16-POINTPREC);	//rh*cos(rot)	s12.11
			rvsinrotN = (ITEs15p16)h->tessellateCmd[inCount+8] >> (16-POINTPREC);	//-rv*sin(rot)	s12.11
			rhsinrot = (ITEs15p16)h->tessellateCmd[inCount+9] >> (16-POINTPREC);	//rh*sin(rot)	s12.11
			rvcosrot = (ITEs15p16)h->tessellateCmd[inCount+10] >> (16-POINTPREC);	//rv*cos(rot)	s12.11
			last = p1;
			inCount += 11;

			if (p0.x!=p1.x || p0.y!=p1.y) {

				i = itefindUnitCircles(x0p, y0p, x1p, y1p, segment, &cp, &u0, &u1);
				if (i==0) break;

				// no more to use x0p, y0p, x1p, y1p
				// (s12.11*s12.11)>>11 + (s12.11*s12.11)>>11
				c.x = (ITEs15p16)( ( (ITEint64)rhcosrot*cp.x + (ITEint64)rvsinrotN*cp.y )>>(POINTPREC) );		
				c.y = (ITEs15p16)( ( (ITEint64)rhsinrot*cp.x + (ITEint64)rvcosrot*cp.y )>>(POINTPREC) );

				iteArc(c, &p0, &p1, u0, u1, segment, rhsinrot, rhcosrot, rvsinrotN, rvcosrot,
					   lines, &outCount);

				if (doJoin)			// Only HW_STROKE_PATH
					iteDoJoin(&outCount);

				// For HW_STROKE_PATH
				doCap = 1 & h->paintMode;
				doJoin = 1 & h->paintMode;
			}
			break;
		case 8: // CLOSE
			inCount += 1;

			if(h->paintMode && (last.x!=start.x || last.y!=start.y))  // HW_STROKE_PATH
			{
				iteClose(&outCount);
		
				if (doJoin)		// do join
					iteDoJoin(&outCount);
			}

			// For HW_STROKE_PATH
			doCap = 0 & h->paintMode;
			doJoin = 1 & h->paintMode;
			break;
		}
	}
	// for rom code version.
	h->strokeRoundC.x = start.x;
	h->strokeRoundC.y = start.y;

	if (doCap) {	// Only HW_STROKE_PATH	
		dashon = h->strokedash1;
		if (dashon) {
			// do end cap
			SET2V(p, h->strokerecm2);
			SET2V(r, h->strokerecm1);
			SET2V(c, last);
			u0.x = -h->strokeUnitV1.y;			// s1.22
			u0.y = h->strokeUnitV1.x;			// s1.22

			iteDoCap(&outCount, p, r, c, u0);
		}

		dashon = h->strokestartdash0;
		if (dashon) {
			// do start cap
			SET2V(p, h->strokestartrecm0);
			SET2V(r, h->strokestartrecm3);
			SET2V(c, start);
			u0.x = h->strokestartUnitV0.y;		// s1.22
			u0.y = -h->strokestartUnitV0.x;		// s1.22

			iteDoCap(&outCount, p, r, c, u0);
		}
	} else if (doJoin) {	// Only HW_STROKE_PATH
		SET2V(h->strokelastrecm1, h->strokerecm1);
		SET2V(h->strokelastrecm2, h->strokerecm2);
		SET2V(h->strokerecm0, h->strokestartrecm0);
		SET2V(h->strokerecm3, h->strokestartrecm3);		

		// original version.
		//h->strokeRoundC.x = start.x;
		//h->strokeRoundC.y = start.y;

		SET2V(h->strokelastUnitV1, h->strokeUnitV1);
		SET2V(h->strokelastVec1, h->strokeVec1);
		SET2V(h->strokeUnitV0, h->strokestartUnitV0);
		SET2V(h->strokeVec0, h->strokestartVec0);
		h->strokelastdash1 = h->strokedash1;
		h->strokedash0 = h->strokestartdash0;

		// do join
		iteDoJoin(&outCount);
	}

	h->cmdData[outCount] = 0x80000000; // list end;
}

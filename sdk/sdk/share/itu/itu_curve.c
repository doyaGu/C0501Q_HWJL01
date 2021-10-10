#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char curveName[] = "ITUCurve";

void ituCurveExit(ITUWidget* widget)
{
    ITUCurve* curve = (ITUCurve*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (curve->ctrlPointCount > 0)
    {
        free(curve->secondCtrlPoints);
        curve->secondCtrlPoints = NULL;

        free(curve->firstCtrlPoints);
        curve->firstCtrlPoints = NULL;

        curve->ctrlPointCount = 0;
    }

    ituIconExit(widget);
}

static void CurveDraw(ITUSurface* dest, int destx, int desty, ITUCurve* curve, int x, int y, uint8_t desta)
{
    ITUWidget* widget = (ITUWidget*)curve;
    ITUBackground* bg = (ITUBackground*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITUGradientMode gfMode = bg->graidentMode;
    ITURectangle prevClip;
    int height = rect->height - y;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (bg->icon.surf)
    {
        if (height > 0)
        {
            if (desta > 0)
            {
                if (desta == 255)
                {
                    ituBitBlt(dest, destx + x, desty + y, 1, height, bg->icon.surf, x, y);
                }
                else
                {
                    ituAlphaBlend(dest, destx + x, desty + y, 1, height, bg->icon.surf, x, y, desta);
                }
            }
        }
    }
    else
    {
        if (desta == 255)
        {
            if (height > 0)
            {
                int h = desty + rect->height - height;

                if (h < 0)
                    h = 0;

                ituSurfaceSetClipping(dest, destx + x, h, prevClip.width, prevClip.height);

                if (bg->graidentMode == ITU_GF_NONE)
                    ituColorFill(dest, destx + x, desty + rect->height - height, 1, height, &widget->color);
                else
                    ituGradientFill(dest, destx + x, desty, 1, rect->height, &widget->color, &bg->graidentColor, ITU_GF_VERTICAL);
            }
        }
        else if (desta > 0)
        {
            ITUSurface* surf = ituCreateSurface(1, rect->height, 0, dest->format, NULL, 0);
            if (surf)
            {
                if (height > 0)
                {
                    int h = desty + rect->height - height;

                    if (h < 0)
                        h = 0;

                    ituSurfaceSetClipping(dest, prevClip.x, h, prevClip.width, prevClip.height);

                    if (bg->graidentMode == ITU_GF_NONE)
                        ituColorFill(surf, 0, 0, 1, height, &widget->color);
                    else
                        ituGradientFill(surf, 0, 0, 1, rect->height, &widget->color, &bg->graidentColor, ITU_GF_VERTICAL);
                }
                ituAlphaBlend(dest, destx + x, desty, 1, rect->height, surf, 0, 0, desta);                
                ituDestroySurface(surf);
            }
        }
    }
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

static void plotQuadBezierSeg(int x0, int y0, int x1, int y1, int x2, int y2, ITUSurface* dest, int destx, int desty, ITUCurve* curve, uint8_t alpha)
{                            
    int sx = x2-x1, sy = y2-y1;
    long xx = x0-x1, yy = y0-y1, xy;         /* relative values for checks */
    float dx, dy, err, cur = (float)xx*sy-yy*sx;                    /* curvature */

    assert(xx*sx <= 0 && yy*sy <= 0);  /* sign of gradient must not change */

    if (sx*(long)sx+sy*(long)sy > xx*xx+yy*yy) { /* begin with longer part */ 
        x2 = x0; x0 = sx+x1; y2 = y0; y0 = sy+y1; cur = -cur;  /* swap P0 P2 */
    }  
    if (cur != 0) {                                    /* no straight line */
        xx += sx; xx *= sx = x0 < x2 ? 1 : -1;           /* x step direction */
        yy += sy; yy *= sy = y0 < y2 ? 1 : -1;           /* y step direction */
        xy = 2*xx*yy; xx *= xx; yy *= yy;          /* differences 2nd degree */
        if (cur*sx*sy < 0) {                           /* negated curvature? */
            xx = -xx; yy = -yy; xy = -xy; cur = -cur;
        }
        dx = 4.0f*sy*cur*(x1-x0)+xx-xy;             /* differences 1st degree */
        dy = 4.0f*sx*cur*(y0-y1)+yy-xy;
        xx += xx; yy += yy; err = dx+dy+xy;                /* error 1st step */    
        do {                              
            CurveDraw(dest, destx, desty, curve, x0, y0, alpha);                  /* plot curve */
            if (x0 == x2 && y0 == y2) return;  /* last pixel -> curve finished */
            y1 = 2*err < dx;                  /* save value for test of y step */
            if (2*err > dy) { x0 += sx; dx -= xy; err += dy += yy; } /* x step */
            if (    y1    ) { y0 += sy; dy -= xy; err += dx += xx; } /* y step */
        } while (dy < dx );           /* gradient negates -> algorithm fails */
  }
  do {
      CurveDraw(dest, destx, desty, curve, x0, y0, alpha);       /* plot remaining part to end */
  } while (x0++ < x2 );
}  

static void plotCubicBezierSeg(int x0, int y0, float x1, float y1,
 float x2, float y2, int x3, int y3, ITUSurface* dest, int destx, int desty, ITUCurve* curve, uint8_t alpha)
{   /* plot limited cubic Bezier segment */
    int f, fx, fy, leg = 1;
    int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1; /* step direction */
    float xc = -fabsf(x0+x1-x2-x3), xa = xc-4*sx*(x1-x2), xb = sx*(x0-x1-x2+x3);
    float yc = -fabsf(y0+y1-y2-y3), ya = yc-4*sy*(y1-y2), yb = sy*(y0-y1-y2+y3);
    float ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01f;

    /* check for curve restrains */
    /* slope P0-P1 == P2-P3 and (P0-P3 == P1-P2 or no slope change) */
    assert((x1-x0)*(x2-x3) < EP && ((x3-x0)*(x1-x2) < EP || xb*xb < xa*xc+EP));
    assert((y1-y0)*(y2-y3) < EP && ((y3-y0)*(y1-y2) < EP || yb*yb < ya*yc+EP));
    if (xa == 0 && ya == 0) { /* quadratic Bezier */
        sx = (int)floorf((3*x1-x0+1)/2); sy = (int)floorf((3*y1-y0+1)/2); /* new midpoint */
        plotQuadBezierSeg(x0,y0, sx,sy, x3,y3,dest,destx,desty,curve,alpha);
        return;
    }
    x1 = (x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+1; /* line lengths */
    x2 = (x2-x3)*(x2-x3)+(y2-y3)*(y2-y3)+1;
    do { /* loop over both ends */
        ab = xa*yb-xb*ya; ac = xa*yc-xc*ya; bc = xb*yc-xc*yb;
        ex = ab*(ab+ac-3*bc)+ac*ac; /* P0 part of self-intersection loop? */
        f = (int)(ex > 0.0f ? 1.0f : sqrtf(1.0f+1024.0f/x1)); /* calculate resolution */
        ab *= f; ac *= f; bc *= f; ex *= f*f; /* increase resolution */
        xy = 9*(ab+ac+bc)/8; cb = 8*(xa-ya);/* init differences of 1st degree */
        dx = 27*(8*ab*(yb*yb-ya*yc)+ex*(ya+2*yb+yc))/64-ya*ya*(xy-ya);
        dy = 27*(8*ab*(xb*xb-xa*xc)-ex*(xa+2*xb+xc))/64-xa*xa*(xy+xa);
        /* init differences of 2nd degree */
        xx = 3*(3*ab*(3*yb*yb-ya*ya-2*ya*yc)-ya*(3*ac*(ya+yb)+ya*cb))/4;
        yy = 3*(3*ab*(3*xb*xb-xa*xa-2*xa*xc)-xa*(3*ac*(xa+xb)+xa*cb))/4;
        xy = xa*ya*(6*ab+6*ac-3*bc+cb); ac = ya*ya; cb = xa*xa;
        xy = 3*(xy+9*f*(cb*yb*yc-xb*xc*ac)-18*xb*yb*ab)/8;
        if (ex < 0) { /* negate values if inside self-intersection loop */
            dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; cb = -cb;
        } /* init differences of 3rd degree */
        ab = 6*ya*ac; ac = -6*xa*ac; bc = 6*ya*cb; cb = -6*xa*cb;
        dx += xy; ex = dx+dy; dy += xy; /* error of 1st step */
        for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3; ) {
            CurveDraw(dest, destx, desty,  curve, x0, y0, alpha); /* plot curve */
            do { /* move sub-steps of one pixel */
                if (dx > *pxy || dy < *pxy) goto exit; /* confusing values */
                y1 = 2*ex-dy; /* save value for test of y step */
                if (2*ex >= dx) { /* x sub-step */
                    fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab;
                }
                if (y1 <= 0) { /* y sub-step */
                    fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += cb;
                }
            } while (fx > 0 && fy > 0); /* pixel complete? */
            if (2*fx <= f) { x0 += sx; fx += f; } /* x step */
            if (2*fy <= f) { y0 += sy; fy += f; } /* y step */
            if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;/* pixel ahead valid */
        }
exit:   xx = (float)x0; x0 = x3; x3 = (int)xx; sx = -sx; xb = -xb; /* swap legs */
        yy = (float)y0; y0 = y3; y3 = (int)yy; sy = -sy; yb = -yb; x1 = x2;
    } while (leg--); /* try other end */
    do {
        CurveDraw(dest, destx, desty, curve, x0, y0, alpha); /* remaining part in case of cusp or crunode */
    } while (x0++ < x3 );
}


static void plotCubicBezier(int x0, int y0, int x1, int y1,
 int x2, int y2, int x3, int y3, ITUSurface* dest, int destx, int desty, ITUCurve* curve, uint8_t alpha)
{   /* plot any cubic Bezier curve */
    int n = 0, i = 0;
    long xc = x0+x1-x2-x3, xa = xc-4*(x1-x2);
    long xb = x0-x1-x2+x3, xd = xb+4*(x1+x2);
    long yc = y0+y1-y2-y3, ya = yc-4*(y1-y2);
    long yb = y0-y1-y2+y3, yd = yb+4*(y1+y2);
    float fx0 = (float)x0, fx1, fx2, fx3, fy0 = (float)y0, fy1, fy2, fy3;
    float t1 = (float)xb*xb-xa*xc, t2, t[5];
    /* sub-divide curve at gradient sign changes */
    if (xa == 0) { /* horizontal */
        if (fabsf(xc) < 2*fabsf(xb)) t[n++] = xc/(2.0f*xb); /* one change */
    } else if (t1 > 0.0) { /* two changes */
        t2 = sqrtf(t1);
        t1 = (xb-t2)/xa; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (xb+t2)/xa; if (fabs(t1) < 1.0) t[n++] = t1;
    }
    t1 = (float)(yb*yb-ya*yc);
    if (ya == 0) { /* vertical */
        if (fabsf(yc) < 2*fabsf(yb)) t[n++] = yc/(2.0f*yb); /* one change */
    } else if (t1 > 0.0f) { /* two changes */
        t2 = sqrtf(t1);
        t1 = (yb-t2)/ya; if (fabs(t1) < 1.0f) t[n++] = t1;
        t1 = (yb+t2)/ya; if (fabs(t1) < 1.0f) t[n++] = t1;
    }
    for (i = 1; i < n; i++) /* bubble sort of 4 points */
        if ((t1 = t[i-1]) > t[i]) { t[i-1] = t[i]; t[i] = t1; i = 0; }
    t1 = -1.0f; t[n] = 1.0f; /* begin / end point */
    for (i = 0; i <= n; i++) { /* plot each segment separately */
        t2 = t[i]; /* sub-divide at t[i-1], t[i] */
        fx1 = (t1*(t1*xb-2*xc)-t2*(t1*(t1*xa-2*xb)+xc)+xd)/8-fx0;
        fy1 = (t1*(t1*yb-2*yc)-t2*(t1*(t1*ya-2*yb)+yc)+yd)/8-fy0;
        fx2 = (t2*(t2*xb-2*xc)-t1*(t2*(t2*xa-2*xb)+xc)+xd)/8-fx0;
        fy2 = (t2*(t2*yb-2*yc)-t1*(t2*(t2*ya-2*yb)+yc)+yd)/8-fy0;
        fx0 -= fx3 = (t2*(t2*(3*xb-t2*xa)-3*xc)+xd)/8;
        fy0 -= fy3 = (t2*(t2*(3*yb-t2*ya)-3*yc)+yd)/8;
        x3 = (int)floorf(fx3+0.5f); y3 = (int)floorf(fy3+0.5f); /* scale bounds to int */
        if (fx0 != 0.0f) { fx1 *= fx0 = (x0-x3)/fx0; fx2 *= fx0; }
        if (fy0 != 0.0f) { fy1 *= fy0 = (y0-y3)/fy0; fy2 *= fy0; }
        if (x0 != x3 || y0 != y3) /* segment t1 - t2 */
        plotCubicBezierSeg(x0,y0, x0+fx1,y0+fy1, x0+fx2,y0+fy2, x3,y3, dest, destx, desty, curve, alpha);
        x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
    }
}

static float* GetFirstControlPoints(float rhs[], int len)
{
	int i, n = len;
	float* x = malloc(sizeof(float)*n); // Solution vector.
	float* tmp = alloca(sizeof(float)*n); // Temp workspace.

	float b = 2.0f;
	x[0] = rhs[0] / b;
	for (i = 1; i < n; i++) // Decomposition and forward substitution.
	{
		tmp[i] = 1 / b;
		b = (i < n - 1 ? 4.0f : 3.5f) - tmp[i];
		x[i] = (rhs[i] - x[i - 1]) / b;
	}
	for (i = 1; i < n; i++)
		x[n - i - 1] -= tmp[n - i] * x[n - i]; // Backsubstitution.

	return x;
}

static void GetCurveControlPoints(ITUPoint knots[], int len, ITUPoint** firstControlPoints, ITUPoint** secondControlPoints, int* outlen)
{
    int i, n;
    float *rhs, *x, *y;

	assert(knots);

	n = len - 1;
    assert(n >= 1);

	if (n == 1)
	{ // Special case: Bezier curve should be a straight line.
		*firstControlPoints = malloc(sizeof(ITUPoint) * 1);
		// 3P1 = 2P0 + P3
		(*firstControlPoints)[0].x = (2 * knots[0].x + knots[1].x) / 3;
		(*firstControlPoints)[0].y = (2 * knots[0].y + knots[1].y) / 3;

		*secondControlPoints = malloc(sizeof(ITUPoint) * 1);
		// P2 = 2P1 – P0
		(*secondControlPoints)[0].x = 2 * (*firstControlPoints)[0].x - knots[0].x;
		(*secondControlPoints)[0].y = 2 * (*firstControlPoints)[0].y - knots[0].y;
        *outlen = 1;
		return;
	}

	// Calculate first Bezier control points
	// Right hand side vector
	rhs = alloca(sizeof(float) * n);

	// Set right hand side X values
	for (i = 1; i < n - 1; ++i)
		rhs[i] = (float)(4 * knots[i].x + 2 * knots[i + 1].x);
	rhs[0] = (float)(knots[0].x + 2 * knots[1].x);
	rhs[n - 1] = (8 * knots[n - 1].x + knots[n].x) / 2.0f;
	// Get first control points X-values
	x = GetFirstControlPoints(rhs, n);

	// Set right hand side Y values
	for (i = 1; i < n - 1; ++i)
		rhs[i] = (float)(4 * knots[i].y + 2 * knots[i + 1].y);
	rhs[0] = (float)(knots[0].y + 2 * knots[1].y);
	rhs[n - 1] = (8 * knots[n - 1].y + knots[n].y) / 2.0f;
	// Get first control points Y-values
	y = GetFirstControlPoints(rhs, n);

	// Fill output arrays.
	*firstControlPoints = malloc(sizeof(ITUPoint) * n);
	*secondControlPoints = malloc(sizeof(ITUPoint) * n);
	for (i = 0; i < n; ++i)
	{
		// First control point
        (*firstControlPoints)[i].x = (int)x[i];
        (*firstControlPoints)[i].y = (int)y[i];
		// Second control point
		if (i < n - 1)
        {
            (*secondControlPoints)[i].x = (int)(2 * knots[i + 1].x - x[i + 1]);
            (*secondControlPoints)[i].y = (int)(2 * knots[i + 1].y - y[i + 1]);
        }
		else
        {
            (*secondControlPoints)[i].x = (int)((knots[n].x + x[n - 1]) / 2.0f);
            (*secondControlPoints)[i].y = (int)((knots[n].y + y[n - 1]) / 2.0f);
        }
	}

    free(y);
    free(x);
    *outlen = n;
}

bool ituCurveUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUCurve* curve = (ITUCurve*) widget;
    assert(curve);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);
    if (ev == ITU_EVENT_LAYOUT)
    {
        if (curve->ctrlPointCount > 0)
        {
            free(curve->secondCtrlPoints);
            curve->secondCtrlPoints = NULL;

            free(curve->firstCtrlPoints);
            curve->firstCtrlPoints = NULL;

            curve->ctrlPointCount = 0;
        }
        if (curve->pointCount > 0)
            GetCurveControlPoints(curve->points, curve->pointCount, &curve->firstCtrlPoints, &curve->secondCtrlPoints, &curve->ctrlPointCount);
    }
    return widget->visible ? result : false;
}

void ituCurveDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty, i;
    uint8_t desta;
    ITUCurve* curve = (ITUCurve*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(curve);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;

    for (i = 0; i < curve->ctrlPointCount; ++i)
    {
        plotCubicBezier(
            curve->points[i].x, curve->points[i].y,
            curve->firstCtrlPoints[i].x, curve->firstCtrlPoints[i].y,
            curve->secondCtrlPoints[i].x, curve->secondCtrlPoints[i].y,
            curve->points[i+1].x, curve->points[i+1].y,
            dest, destx, desty, curve, desta);
    }
}

void ituCurveInit(ITUCurve* curve)
{
    assert(curve);
    ITU_ASSERT_THREAD();

    memset(curve, 0, sizeof (ITUCurve));

    ituBackgroundInit(&curve->bg);

    ituWidgetSetType(curve, ITU_CURVE);
    ituWidgetSetName(curve, curveName);
    ituWidgetSetExit(curve, ituCurveExit);
    ituWidgetSetUpdate(curve, ituCurveUpdate);
    ituWidgetSetDraw(curve, ituCurveDraw);
}

void ituCurveLoad(ITUCurve* curve, uint32_t base)
{
    assert(curve);

    ituBackgroundLoad(&curve->bg, base);
    ituWidgetSetExit(curve, ituCurveExit);
    ituWidgetSetUpdate(curve, ituCurveUpdate);
    ituWidgetSetDraw(curve, ituCurveDraw);
}

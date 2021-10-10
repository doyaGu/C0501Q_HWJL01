#include "itu_cfg.h"
#include "ite/itu.h"
#include "itu_private.h"

ITUScene* ituScene = NULL;

static ITUSurface* GetDisplaySurfaceDefault(void)
{
    // DO NOTHING
    return NULL;
}
ITUSurface* (*ituGetDisplaySurface)(void) = GetDisplaySurfaceDefault;

static ITUSurface* CreateSurfaceDefault(int w, int h, int pitch, ITUPixelFormat format, const uint8_t* bitmap, unsigned int flags)
{
    // DO NOTHING
    return NULL;
}
ITUSurface* (*ituCreateSurface)(int w, int h, int pitch, ITUPixelFormat format, const uint8_t* bitmap, unsigned int flags) = CreateSurfaceDefault;

static void DestroySurfaceDefault(ITUSurface* surf)
{
    // DO NOTHING
}
void (*ituDestroySurface)(ITUSurface* surf) = DestroySurfaceDefault;

static ITUMaskSurface* CreateMaskSurfaceDefault(int w, int h, int pitch, ITUMaskFormat format, uint8_t* buffer, uint32_t bufferLength)
{
    // DO NOTHING
    return NULL;
}
ITUMaskSurface* (*ituCreateMaskSurface)(int w, int h, int pitch, ITUMaskFormat format, uint8_t* buffer, uint32_t bufferLength) = CreateMaskSurfaceDefault;

static void DestroyMaskSurfaceDefault(ITUMaskSurface* surf)
{
    // DO NOTHING
}
void (*ituDestroyMaskSurface)(ITUMaskSurface* surf) = DestroyMaskSurfaceDefault;

static void SetMaskSurfaceDefault(ITUSurface* surface, ITUMaskSurface* maskSurface, bool enable)
{
    // DO NOTHING
}
void (*ituSetMaskSurface)(ITUSurface* surface, ITUMaskSurface* maskSurface, bool enable) = SetMaskSurfaceDefault;


static uint8_t* LockSurfaceDefault(ITUSurface* surf, int x, int y, int w, int h)
{
    // DO NOTHING
    return NULL;
}

uint8_t* (*ituLockSurface)(ITUSurface* surf, int x, int y, int w, int h) = LockSurfaceDefault;

static void UnlockSurfaceDefault(ITUSurface* surf)
{
    // DO NOTHING
}
void (*ituUnlockSurface)(ITUSurface* surf) = UnlockSurfaceDefault;

static void SetRotationDefault(ITURotation rot)
{
    // DO NOTHING
}
void (*ituSetRotation)(ITURotation rot) = SetRotationDefault;

static void ituDrawGlyphDefault(ITUSurface* surf, int x, int y, ITUGlyphFormat format, const uint8_t* bitmap, int w, int h)
{
    // DO NOTHING
}
void (*ituDrawGlyph)(ITUSurface* surf, int x, int y, ITUGlyphFormat format, const uint8_t* bitmap, int w, int h) = ituDrawGlyphDefault;

static void BitBltDefault(ITUSurface* dest, int dx, int dy, int w, int h, ITUSurface* src, int sx, int sy)
{
    // DO NOTHING
}
void (*ituBitBlt)(ITUSurface* dest, int dx, int dy, int w, int h, ITUSurface* src, int sx, int sy) = BitBltDefault;

static void StretchBltDefault(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh)
{
    // DO NOTHING
}
void (*ituStretchBlt)(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh) = StretchBltDefault;

static void AlphaBlendDefault(ITUSurface* dest, int dx, int dy, int w, int h, ITUSurface* src, int sx, int sy, uint8_t alpha)
{
    // DO NOTHING
}
void (*ituAlphaBlend)(ITUSurface* dest, int dx, int dy, int w, int h, ITUSurface* src, int sx, int sy, uint8_t alpha) = AlphaBlendDefault;

static void ColorFillDefault(ITUSurface* surf, int x, int y, int w, int h, ITUColor* color)
{
    // DO NOTHING
}
void (*ituColorFill)(ITUSurface* surf, int x, int y, int w, int h, ITUColor* color) = ColorFillDefault;

static void ColorFillBlendDefault(ITUSurface* surf, int x, int y, int w, int h, ITUColor* color, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue)
{
    // DO NOTHING
}
void (*ituColorFillBlend)(ITUSurface* surf, int x, int y, int w, int h, ITUColor* color, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue) = ColorFillBlendDefault;

static void GradientFillDefault(ITUSurface* surf, int x, int y, int w, int h, ITUColor* startColor, ITUColor* endColor, ITUGradientMode mode)
{
    // DO NOTHING
}
void (*ituGradientFill)(ITUSurface* surf, int x, int y, int w, int h, ITUColor* startColor, ITUColor* endColor, ITUGradientMode mode) = GradientFillDefault;

static void GradientFillBlendDefault(ITUSurface* surf, int x, int y, int w, int h, ITUColor* startColor, ITUColor* endColor, ITUGradientMode mode, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue)
{
    // DO NOTHING
}
void (*ituGradientFillBlend)(ITUSurface* surf, int x, int y, int w, int h, ITUColor* startColor, ITUColor* endColor, ITUGradientMode mode, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue) = GradientFillBlendDefault;

static void RotateDefault(ITUSurface* dest, int dx, int dy, ITUSurface* src, int cx, int cy, float angle, float scaleX, float scaleY)
{
    // DO NOTHING
}
void (*ituRotate)(ITUSurface* dest, int dx, int dy, ITUSurface* src, int cx, int cy, float angle, float scaleX, float scaleY) = RotateDefault;

static void TransformDefault(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh, int cx, int cy, float scaleWidth, float scaleHeight, float angle, ITUTileMode tilemode, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue)
{
    // DO NOTHING
}
void (*ituTransform)(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh, int cx, int cy, float scaleWidth, float scaleHeight, float angle, ITUTileMode tilemode, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue) = TransformDefault;

static void FlipDefault(ITUSurface* surf)
{
    // DO NOTHING
}
void (*ituFlip)(ITUSurface* surf) = FlipDefault;

static void ProjectionDefault(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh, float scaleWidth, float scaleHeight, float degree, float FOV, float pivotX, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue)
{
    // DO NOTHING
}
void (*ituProjection)(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh, float scaleWidth, float scaleHeight, float degree, float FOV, float pivotX, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue) = ProjectionDefault;

static void DrawLineDefault(ITUSurface* surf, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, ITUColor* lineColor, int32_t lineWidth)
{
    // DO NOTHING
}
void (*ituDrawLine)(ITUSurface* surf, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, ITUColor* lineColor, int32_t lineWidth) = DrawLineDefault;

static void DrawCurveDefault(ITUSurface* surf, ITUPoint* point1, ITUPoint* point2, ITUPoint* point3, ITUPoint* point4, ITUColor* lineColor, int32_t lineWidth)
{
    // DO NOTHING
}
void (*ituDrawCurve)(ITUSurface* surf, ITUPoint* point1, ITUPoint* point2, ITUPoint* point3, ITUPoint* point4, ITUColor* lineColor, int32_t lineWidth) = DrawCurveDefault;

static void TransformBltDefault(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int sw, int sh, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, bool bInverse, ITUPageFlowType type)
{
    // DO NOTHING
}
void(*ituTransformBlt)(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int sw, int sh, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, bool bInverse, ITUPageFlowType type, ITUTransformType transformType) = TransformBltDefault;

static void ReflectedDefault(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int reflectedWidth, int reflectedHeight, ITUSurface* masksurf)
{
    // DO NOTHING
}
void(*ituReflected)(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int reflectedWidth, int reflectedHeight, ITUSurface* masksurf) = ReflectedDefault;

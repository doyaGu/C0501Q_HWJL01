#ifndef ITU_PRIVATE_H
#define ITU_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    ITUSurface surf;
#ifdef CFG_M2D_ENABLE    
    GFXSurface *m2dSurf;
#endif    
} M2dSurface;

#define LcdSurface M2dSurface

#ifdef __cplusplus
}
#endif

#endif // ITU_PRIVATE_H

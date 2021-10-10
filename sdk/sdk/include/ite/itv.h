/*
 * Copyright (c) 2015 ITE technology Corp. All Rights Reserved.
 */
/** @file itv.h
 * Used to do H/W video overlay
 *
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef ITV_H
#define ITV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/ith.h"
#include "ite/itu.h"
#if (CFG_CHIP_FAMILY == 9920)
	#include "isp/mmp_isp_9920.h"
#else
	#include "isp/mmp_isp.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ITV_MAX_NDBUF    3
#define ITV_MAX_DISP_BUF 2

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint8_t  *ya;       /// address of Y decoded video buffer
    uint8_t  *ua;       /// address of U decoded video buffer
    uint8_t  *va;       /// address of V decoded video buffer
    uint32_t src_w;     /// width of decoded video buffer
    uint32_t src_h;     /// height of decoded video buffer
    int      bidx;
    uint32_t pitch_y;   /// pitch of Y
    uint32_t pitch_uv;  /// pitch of UV
    uint32_t format;    /// YUV format. see MMP_ISP_INFORMAT.
} ITV_DBUF_PROPERTY;

typedef struct
{
    uint32_t startX;
    uint32_t startY;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;

    /// color key value ([A]lpha, [R]ed, [G]reen, [B]lue)
    uint32_t colorKeyR;
    uint32_t colorKeyG;
    uint32_t colorKeyB;
#if (CFG_CHIP_FAMILY == 9920)
    uint32_t EnAlphaBlend;
#endif
    uint32_t constAlpha;
} ITV_UI_PROPERTY;

//=============================================================================
//                Public Function Definition
//=============================================================================
int      itv_init(void);
int      itv_deinit(void);
int      itv_set_pb_mode(int pb_mode);

/* FRAME FUNCTION */
int      itv_ff_setup_base(int id, int bid, uint8_t *base);
int      itv_ff_enable(int id, int enable);

uint8_t *itv_get_uibuf_anchor(int id);
int      itv_update_uibuf_anchor(int id, ITV_UI_PROPERTY *uiprop);
void     itv_flush_uibuf(int id);

uint8_t *itv_get_dbuf_anchor(void);
int      itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop);
void     itv_flush_dbuf(void);

void     itv_set_rotation(ITURotation rot);
void
itv_set_video_window(
    uint32_t  startX,
    uint32_t  startY,
    uint32_t  width,
    uint32_t  height);

ISP_RESULT
itv_enable_isp_feature(
    MMP_ISP_CAPS cap);

#if (CFG_CHIP_FAMILY == 9850)

void itv_set_vidSurf_buf(uint8_t* addr, uint8_t index);
int itv_get_vidSurf_index(void);
int itv_update_vidSurf_anchor(void);
void itv_stop_vidSurf_anchor(void);
ITURotation itv_get_rotation(void);
bool itv_get_new_video(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // ITV_H
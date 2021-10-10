#ifndef __ts_recv_camera_H_PkzkKlh6_zx13_r8Q4_GC8U_ACxuASoUWQs8__
#define __ts_recv_camera_H_PkzkKlh6_zx13_r8Q4_GC8U_ACxuASoUWQs8__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "tsi/mmp_tsi.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * ts receiver operator (tsi/usb/file/...)
 **/
typedef struct TS_RECV_OPR_T
{
    uint32_t    (*ts_recv_init)(struct TS_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_deinit)(struct TS_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_turn_on)(struct TS_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_turn_off)(struct TS_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_get_data)(struct TS_RECV_OPR_T *pTs_recv_opr,
                                    uint8_t **ppSampleAddr, uint32_t *pSampleSize, void *extraData);

    // tsi
    uint32_t    tsi_index;
    void        *data;
}TS_RECV_OPR;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint32_t
_ts_recv_init(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        tsi_idx = pTs_recv_opr->tsi_index;

        mmpTsiInitialize(tsi_idx);
    }
    return result;
}

static uint32_t
_ts_recv_deinit(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        tsi_idx = pTs_recv_opr->tsi_index;

        mmpTsiDisable(tsi_idx);
        mmpTsiTerminate(tsi_idx);
    }
    return result;
}

static uint32_t
_ts_recv_turn_on(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr )
    {
        uint32_t    tsi_idx = pTs_recv_opr->tsi_index;

        mmpTsiEnable(tsi_idx);
    }
    return result;
}

static uint32_t
_ts_recv_turn_off(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr )
    {
        uint32_t    tsi_idx = pTs_recv_opr->tsi_index;

        mmpTsiDisable(tsi_idx);
    }
    return result;
}

static uint32_t
_ts_recv_get_data(
    TS_RECV_OPR     *pTs_recv_opr,
    uint8_t         **ppSampleAddr,
    uint32_t        *pSampleSize,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr && ppSampleAddr && pSampleSize )
    {
        uint32_t    tsi_idx = pTs_recv_opr->tsi_index;
        uint8_t     *pSampleBuf = 0;
        uint32_t    sampleLength = 0;

        mmpTsiReceive(tsi_idx, &pSampleBuf, &sampleLength);
        //printv("tsi dataSize = %d\n", sampleLength);
        (*ppSampleAddr) = pSampleBuf;
        (*pSampleSize)  = sampleLength;
    }
    return result;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
static TS_RECV_OPR ts_recv_opr =
{
    _ts_recv_init,
    _ts_recv_deinit,
    _ts_recv_turn_on,
    _ts_recv_turn_off,
    _ts_recv_get_data,
};


#ifdef __cplusplus
}
#endif

#endif

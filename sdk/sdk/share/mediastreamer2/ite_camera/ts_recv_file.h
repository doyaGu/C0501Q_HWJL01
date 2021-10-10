#ifndef __ts_recv_file_H_G6RQWHde_ryJ3_PAUF_IQ2o_CCPUAGBy6BXI__
#define __ts_recv_file_H_G6RQWHde_ryJ3_PAUF_IQ2o_CCPUAGBy6BXI__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#undef _err
#if (_MSC_VER)
    #define _err(string, args, ...)           do{ printf(string, __VA_ARGS__); \
                                                  printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                  while(1); \
                                                }while(0)
#else
    #define _err(string, args...)           do{ printf(string, ## args); \
                                                  printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                  while(1); \
                                                }while(0)
#endif
#define INPUT_FILE	"A:/test.ts"
//=============================================================================
//                Structure Definition
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

    // file
    uint32_t    tsi_index;
    uint8_t     *pTsi_buf[2];
    void        *fin[2];
}TS_RECV_OPR;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
#define CACHE_BUF_SIZE      (300 << 10)
static uint32_t
_file_ts_recv_init(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        uint32_t    index = 0;
        uint8_t     *pTsi_buf = 0;
        FILE        *fp = 0;

        if( !(fp = fopen(INPUT_FILE, "rb")) )  _err("open file fail !!");
        //if( !(fp = fopen("./aggre_ts_tag1-2-3-4_length_1.ts", "rb")) ) _err("open file fail !!");

        index = 0;
        if( !(pTsi_buf = malloc(CACHE_BUF_SIZE)) )     _err("malloc fail !!");

        pTs_recv_opr->tsi_index       = index;
        pTs_recv_opr->fin[index]      = (void*)fp;
        pTs_recv_opr->pTsi_buf[index] = pTsi_buf;
    }
    return result;
}

static uint32_t
_file_ts_recv_deinit(
    TS_RECV_OPR     *pTs_recv_opr,
    void            *extraData)
{
    uint32_t    result = 0;
    uint32_t    tsi_idx = 0;

    if( pTs_recv_opr )
    {
        uint32_t    index = pTs_recv_opr->tsi_index;
        uint8_t     *pTsi_buf = 0;
        FILE        *fp = 0;

        pTsi_buf = pTs_recv_opr->pTsi_buf[index];
        if( pTsi_buf )
        {
            free(pTsi_buf);
            pTs_recv_opr->pTsi_buf[index] = 0;
        }

        fp = (FILE*)pTs_recv_opr->fin[index];
        fclose(fp);
    }
    return result;
}

static uint32_t
_file_ts_recv_get_data(
    TS_RECV_OPR     *pTs_recv_opr,
    uint8_t         **ppSampleAddr,
    uint32_t        *pSampleSize,
    void            *extraData)
{
    uint32_t    result = 0;

    if( pTs_recv_opr && ppSampleAddr && pSampleSize )
    {
        uint32_t    index = pTs_recv_opr->tsi_index;
        uint8_t     *pTsi_buf = 0;
        uint32_t    stream_data_size = (2 * 1024); // ((rand() >> 3) % CACHE_BUF_SIZE);
        uint32_t    valid_size = 0;
        FILE        *fp = 0;

        fp = (FILE*)pTs_recv_opr->fin[index];
        pTsi_buf = pTs_recv_opr->pTsi_buf[index];

        valid_size = fread(pTsi_buf, 1, stream_data_size, fp);
        if( !valid_size )
        {
            fseek(fp, 0, SEEK_SET);
            valid_size = fread(pTsi_buf, 1, stream_data_size, fp);
        }

        (*ppSampleAddr) = pTsi_buf;
        (*pSampleSize)  = valid_size;
    }
    return result;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
static TS_RECV_OPR ts_recv_opr =
{
    _file_ts_recv_init,
    _file_ts_recv_deinit,
    0,
    0,
    _file_ts_recv_get_data,
};


#ifdef __cplusplus
}
#endif

#endif

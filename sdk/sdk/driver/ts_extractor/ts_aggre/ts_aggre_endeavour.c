

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_aggre_desc.h"
#include <pthread.h>

#if (CONFIG_TSA_DESC_ENDEAVOUR_DESC)

#include "IT9300.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#if defined(CFG_TSI_PARALLEL_MODE)
    #define ACT_OUTPUT_DATA_TYPE    OUT_DATA_TS_PARALLEL
#else
    #define ACT_OUTPUT_DATA_TYPE    OUT_DATA_TS_SERIAL
#endif
//=============================================================================
//                Macro Definition
//=============================================================================
#define _STR(s)     #s
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct AGGRE_ENDEAVOUR_INFO_T
{
    Endeavour       hEndeavour;

    uint32_t        reserved;

}AGGRE_ENDEAVOUR_INFO;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
//=============================================================================
/**
 * Lock I2C module for internal demod usage
 * return none.
 */
//=============================================================================
static void
_AGGRECTRL_Lock(
    AGGRE_ENDEAVOUR_INFO    *pAggre_info)
{
    Endeavour *pEndeavour = &pAggre_info->hEndeavour;

    if( pEndeavour->ctrlBus == BUS_I2C )
    {
        //mmpIicLockModule();
    }
    return;
}

//=============================================================================
/**
 * Release I2C module for other module usage
 * return none.
 */
//=============================================================================
static void
_AGGRECTRL_unLock(
    AGGRE_ENDEAVOUR_INFO    *pAggre_info)
{
    Endeavour *pEndeavour = &pAggre_info->hEndeavour;

    if( pEndeavour->ctrlBus == BUS_I2C )
    {
        //mmpIicReleaseModule();
    }
}


static uint32_t
endeavour_init(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;

    do{
        uint32_t        i = 0;
        Endeavour       *pEndeavour = 0;

        if( pTsaDev->privData )        break;

        pAggre_info = tsa_malloc(sizeof(AGGRE_ENDEAVOUR_INFO));
        if( !pAggre_info )
        {
            tsa_msg_ex(TSA_MSG_ERR, "err, malloc endeavour handle fail !");
            result = TSA_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pAggre_info, 0x0, sizeof(AGGRE_ENDEAVOUR_INFO));

        pEndeavour = &pAggre_info->hEndeavour;

        //-----------------------------
        // init param
        switch( pTsaDev->bus_type )
        {
            case TSA_BUS_I2C:  pEndeavour->ctrlBus = BUS_I2C; break;
            case TSA_BUS_USB:  pEndeavour->ctrlBus = BUS_USB; break;
        }

        pEndeavour->maxBusTxSize          = 255;
        pEndeavour->chipCount             = 1;
        pEndeavour->gator[0].existed      = True;
        pEndeavour->gator[0].i2cAddr      = pTsaDev->aggre_i2c_addr;
        pEndeavour->gator[0].outDataType  = ACT_OUTPUT_DATA_TYPE;
        pEndeavour->gator[0].outTsPktLen  = PKT_LEN_192; //PKT_LEN_188;

        pEndeavour->bypassBoot            = True;
        pEndeavour->bypassScript          = True;

        tsa_msg(1, "\n\ni2c_addr: aggre = 0x%x, ", pEndeavour->gator[0].i2cAddr);
        switch( pEndeavour->gator[0].outDataType )
        {
            case OUT_DATA_TS_SERIAL:    tsa_msg(1, "interface = serial, "); break;
            case OUT_DATA_TS_PARALLEL:  tsa_msg(1, "interface = parallel, "); break;
        }

        for(i = 0; i < pTsaDev->total_demod_port; i++)
        {
            pEndeavour->tsSource[0][i].existed  = True;
            pEndeavour->tsSource[0][i].tsType   = TS_SERIAL;
            pEndeavour->tsSource[0][i].i2cAddr  = pTsaDev->demod_i2c_addr[i];
            pEndeavour->tsSource[0][i].i2cBus   = 3;
            pEndeavour->tsSource[0][i].syncByte = 0x47;
            pEndeavour->tsSource[0][i].tsPktLen = PKT_LEN_188;

            switch( pTsaDev->linked_aggre_port_idx[i] )
            {
                case 0:  pEndeavour->tsSource[0][i].tsPort = TS_PORT_0; break;
                case 1:  pEndeavour->tsSource[0][i].tsPort = TS_PORT_1; break;
                case 2:  pEndeavour->tsSource[0][i].tsPort = TS_PORT_2; break;
                case 3:  pEndeavour->tsSource[0][i].tsPort = TS_PORT_3; break;
                case 4:  pEndeavour->tsSource[0][i].tsPort = TS_PORT_4; break;
            }

            tsa_msg(1, "demoe_%d(tsPort=%d)=0x%x, ", i, pEndeavour->tsSource[0][i].tsPort, pEndeavour->tsSource[0][i].i2cAddr);
        }

        tsa_msg(1, "\n");

        _AGGRECTRL_Lock(pAggre_info);
        //-----------------------------
        // init chip
        aggre_rst = IT9300_initialize(pEndeavour, 0);
        if( aggre_rst != BR_ERR_NO_ERROR )
        {
            result = aggre_rst;
            tsa_msg_ex(TSA_MSG_ERR, "[it9300_INIT] ERROR (0x%x) !", aggre_rst);
            break;
        }
        else
            tsa_msg(1, "[it9300_INIT] ok !\n");

        //--------------------------
        // modify PLL to 80M
        IT9300_writeRegister (pEndeavour, 0, 0xd813 , 0x0a);
        IT9300_writeRegister (pEndeavour, 0, 0xd830 , 0x01);
        IT9300_writeRegister (pEndeavour, 0, 0xd831 , 0x01);
        IT9300_writeRegister (pEndeavour, 0, 0xd832 , 0x01);
        IT9300_writeRegister (pEndeavour, 0, 0xf491 , 0x18);
        IT9300_writeRegister (pEndeavour, 0, 0xf491 , 0x1b);
        IT9300_writeRegister (pEndeavour, 0, 0xf496 , 0x89);
        IT9300_writeRegister (pEndeavour, 0, 0xf48C , 0x0e);
        IT9300_writeRegister (pEndeavour, 0, 0xf48F , 0x08);
        IT9300_writeRegister (pEndeavour, 0, 0xf48F , 0x28);
        IT9300_writeRegister (pEndeavour, 0, 0xf48F , 0x08);
        //--------------------------//*/

        aggre_rst = IT9300_setOutTsPktLen(pEndeavour, 0);
        if( aggre_rst != BR_ERR_NO_ERROR )
        {
            result = aggre_rst;
            tsa_msg_ex(TSA_MSG_ERR, "[IT9300_setOutTsPktLen] err, (0x%x) !", aggre_rst);
            break;
        }

        _AGGRECTRL_unLock(pAggre_info);
        pTsaDev->privData = (void*)pAggre_info;

    }while(0);

    if( result )
    {
        if( pAggre_info )        free(pAggre_info);
        pTsaDev->privData = 0;
    }
    return result;
}

static uint32_t
endeavour_deinit(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;
    Endeavour               *pEndeavour = 0;

    if( pTsaDev->privData )
    {
        pAggre_info = (AGGRE_ENDEAVOUR_INFO*)pTsaDev->privData;

        _AGGRECTRL_Lock(pAggre_info);

        pEndeavour = &pAggre_info->hEndeavour;

        // endeavour_terminate()
        // To Do:

        _AGGRECTRL_unLock(pAggre_info);

        free(pAggre_info);
        pTsaDev->privData = 0;
    }

    return result;
}

static uint32_t
endeavour_enable_port(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;

    if( pTsaDev->privData )
    {
        do{
            uint32_t    act_port_index = 0;
            Endeavour   *pEndeavour = 0;

            if( !pTsaArg )
            {
                tsa_msg_ex(TSA_MSG_ERR, "err, invalid parameter !");
                result = TSA_ERR_INVALID_PARAMETER;
                break;
            }

            pAggre_info = (AGGRE_ENDEAVOUR_INFO*)pTsaDev->privData;
            pEndeavour  = &pAggre_info->hEndeavour;

            act_port_index = pTsaArg->arg.set_mode.port_index;

            _AGGRECTRL_Lock(pAggre_info);

            //trace(" enable port, user_port=%d, aggre_port=%d, ",
            //        act_port_index, pEndeavour->tsSource[0][act_port_index].tsPort);

            aggre_rst = IT9300_enableTsPort(pEndeavour, 0, act_port_index);
            if( aggre_rst != BR_ERR_NO_ERROR )
            {
                result = -1;
                tsa_msg_ex(TSA_MSG_ERR, "err, enable port (%d) fail (rst=0x%x) !", act_port_index, aggre_rst);
                break;
            }

            _AGGRECTRL_unLock(pAggre_info);
        }while(0);
    }

    return result;
}

static uint32_t
endeavour_disable_port(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;

    if( pTsaDev->privData )
    {
        do{
            uint32_t    act_port_index = 0;
            Endeavour   *pEndeavour = 0;

            if( !pTsaArg )
            {
                tsa_msg_ex(TSA_MSG_ERR, "err, invalid parameter !");
                result = TSA_ERR_INVALID_PARAMETER;
                break;
            }

            pAggre_info = (AGGRE_ENDEAVOUR_INFO*)pTsaDev->privData;
            pEndeavour  = &pAggre_info->hEndeavour;

            act_port_index = pTsaArg->arg.set_mode.port_index;

            _AGGRECTRL_Lock(pAggre_info);

            //trace(" disable port, user_port=%d, aggre_port=%d, ",
            //        act_port_index, pEndeavour->tsSource[0][act_port_index].tsPort);

            aggre_rst = IT9300_disableTsPort(pEndeavour, 0, act_port_index);
            if( aggre_rst != BR_ERR_NO_ERROR )
            {
                result = -1;
                tsa_msg_ex(TSA_MSG_ERR, "err, disable port (%d) fail (rst=0x%x) !", act_port_index, aggre_rst);
                break;
            }

            _AGGRECTRL_unLock(pAggre_info);
        }while(0);
    }


    return result;
}

static uint32_t
endeavour_set_aggre_mode(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;

    if( pTsaDev->privData )
    {
        do{
            uint32_t    act_port_index = 0;
            uint32_t    tag_value = 0;
            Endeavour   *pEndeavour = 0;

            if( !pTsaArg )
            {
                tsa_msg_ex(TSA_MSG_ERR, "err, invalid parameter !");
                result = TSA_ERR_INVALID_PARAMETER;
                break;
            }

            pAggre_info = (AGGRE_ENDEAVOUR_INFO*)pTsaDev->privData;
            pEndeavour  = &pAggre_info->hEndeavour;

            act_port_index = pTsaArg->arg.set_mode.port_index;
            tag_value      = pTsaArg->arg.set_mode.tag_value;

            _AGGRECTRL_Lock(pAggre_info);

            switch( pTsaArg->arg.set_mode.aggre_mode )
            {
                case TSA_MODE_TAG:
                    //pEndeavour->tsSource[0][act_port_index].tsPktLen = PKT_LEN_188;
                    IT9300_setInTsPktLen(pEndeavour, 0, act_port_index);

                    //pEndeavour->tsSource[0][act_port_index].tsType = TS_SERIAL;
                    IT9300_setInTsType(pEndeavour, 0, act_port_index);

                    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
                    pEndeavour->tsSource[0][act_port_index].tag[0] =  (tag_value & 0x000000FF);
                    pEndeavour->tsSource[0][act_port_index].tag[1] = ((tag_value & 0x0000FF00) >> 8);
                    pEndeavour->tsSource[0][act_port_index].tag[2] = ((tag_value & 0x00FF0000) >> 16);
                    pEndeavour->tsSource[0][act_port_index].tag[3] = ((tag_value & 0xFF000000) >> 24);

                    //trace(" user_port=%d, aggre_port=%d, tag=0x%x, ",
                    //        act_port_index, pEndeavour->tsSource[0][act_port_index].tsPort, tag_value);
                    aggre_rst = IT9300_setTagMode(pEndeavour, 0, act_port_index);
                    if( aggre_rst != BR_ERR_NO_ERROR )
                    {
                        result = -1;
                        tsa_msg_ex(TSA_MSG_ERR, "err, set mode port (%d) fail (rst=0x%x) !", act_port_index, aggre_rst);
                        break;
                    }
                    break;

                case TSA_MODE_SYNC_BYTE:
                case TSA_MODE_PID_REMAP:
                    tsa_msg_ex(TSA_MSG_ERR, "err, %s,%s NO implement !", _STR(TSA_MODE_SYNC_BYTE), _STR(TSA_MODE_PID_REMAP));
                    result = TSA_ERR_NO_IMPLEMENT;
                default:     break;
            }

            _AGGRECTRL_unLock(pAggre_info);

        }while(0);
    }

    return result;
}

static uint32_t
endeavour_ctrl(
    TSA_DEV     *pTsaDev,
    TSA_AGR     *pTsaArg,
    void        *extraData)
{
    uint32_t                result = 0, aggre_rst = BR_ERR_NO_ERROR;
    AGGRE_ENDEAVOUR_INFO    *pAggre_info = 0;

    if( pTsaDev->privData )
    {
        Endeavour               *pEndeavour = 0;

        pAggre_info = (AGGRE_ENDEAVOUR_INFO*)pTsaDev->privData;
        pEndeavour  = &pAggre_info->hEndeavour;

    }

    return result;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
TSA_DESC TSA_DESC_endeavour_desc =
{
    "ts aggr endeavour",     // char        *name;
    0,                       // struct TS_AGGRE_DESC_T  *next;
    TS_AGGR_ID_ENDEAVOUR,    // TS_AGGR_ID              id;
    0,                       // void        *privInfo;
    endeavour_init,          // uint32_t    (*init)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    endeavour_deinit,        // uint32_t    (*deinit)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    endeavour_enable_port,   // uint32_t    (*enable_port)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    endeavour_disable_port,  //uint32_t    (*disable_port)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    endeavour_set_aggre_mode,//uint32_t    (*set_aggre_mode)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    endeavour_ctrl,          // uint32_t    (*control)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
};
#else
TSA_DESC TSA_DESC_endeavour_desc =
{
    "ts aggr endeavour",     // char        *name;
    0,                       // struct TS_AGGRE_DESC_T  *next;
    TS_AGGR_ID_ENDEAVOUR,    // TS_AGGR_ID              id;
    0,                       // void        *privInfo;
};
#endif


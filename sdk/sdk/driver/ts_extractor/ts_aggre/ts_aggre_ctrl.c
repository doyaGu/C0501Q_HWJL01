

#include "register_template.h"
#include "ts_aggre_desc.h"
#include "ts_aggre_ctrl.h"

//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum TSA_STATUS_T
{
    TSA_STATUS_IDLE = 0x11,
    TSA_STATUS_BUSY = 0xBB,
    TSA_STATUS_FAIL = 0xFF,

}TSA_STATUS;
//=============================================================================
//                Macro Definition
//=============================================================================
DEFINE_REGISTER_TEMPLATE(TSA_DESC, TS_AGGR_ID);
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct TSA_MGR_T
{
    TSA_HANDLE          hTsa;

    TSA_STATUS          tsaStatus;

    TSA_DEV             dev;
    TSA_DESC            *pTsaDesc;

}TSA_MGR;
//=============================================================================
//                Global Data Definition
//=============================================================================
#if !defined(TSA_LOCAL_MACRO_DISABLE)
uint32_t  tsaMsgOnFlag = 0x1;
#endif
//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
tsa_Register_all(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(TSA_DESC, ENDEAVOUR, endeavour);
}


TSA_ERR
tsa_CreateHandle(
    TSA_HANDLE      **ppHTsa,
    TSA_SETUP_INFO  *pSetup_info,
    void            *extraData)
{
    TSA_ERR     result = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = 0;

    do{
        if( !ppHTsa || (*ppHTsa) || !pSetup_info )
        {
            tsa_msg_ex(TSA_MSG_ERR, " err, wrong paraments !!");
            result = TSA_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pTsaMgr = tsa_malloc(sizeof(TSA_MGR));
        if( !pTsaMgr )
        {
            tsa_msg_ex(TSA_MSG_ERR, " err, malloc fail !!");
            result = TSA_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pTsaMgr, 0x0, sizeof(TSA_MGR));

        // ------------------------
        // regist all
        tsa_Register_all();

        // ------------------------
        // find action descriptor
        pTsaMgr->pTsaDesc = FIND_DESC_ITEM(TSA_DESC, pSetup_info->aggre_id);
        if( !pTsaMgr->pTsaDesc )
            tsa_msg_ex(TSA_MSG_ERR, " Can't find action tsa descriptor !!");
        else
        {
            pTsaMgr->hTsa.tsa_type = pSetup_info->aggre_id;
            tsa_msg(1, " action tsa descriptor = %s !!\n", (pTsaMgr->pTsaDesc->name) ? pTsaMgr->pTsaDesc->name : "unknow");
        }

        // ------------------------
        // init param
        pTsaMgr->dev.aggre_index = pSetup_info->tsa_idx;
        pTsaMgr->dev.tsa_tag_len = pSetup_info->tsa_tag_len;

        switch( pSetup_info->tsa_tag_len )
        {
            case TSA_TAG_LEN_0:      pTsaMgr->hTsa.tag_length = 0;   break;
            case TSA_TAG_LEN_4:      pTsaMgr->hTsa.tag_length = 4;   break;
            case TSA_TAG_LEN_16:     pTsaMgr->hTsa.tag_length = 16;  break;
            case TSA_TAG_LEN_20:     pTsaMgr->hTsa.tag_length = 20;  break;
        }

        //-------------------------------------
        (*ppHTsa) = &pTsaMgr->hTsa;
    }while(0);

    if( result != TSA_ERR_OK )
    {
        if( pTsaMgr )
        {
            TSA_HANDLE   *pHTmp = &pTsaMgr->hTsa;
            tsa_DestroyHandle(&pHTmp, extraData);
        }
        tsa_msg_ex(TSA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

TSA_ERR
tsa_DestroyHandle(
    TSA_HANDLE     **ppHTsa,
    void           *extraData)
{
    TSA_ERR     result = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, (*ppHTsa), hTsa);

    if( pTsaMgr )
    {
        if( pTsaMgr->dev.privData )
        {
            free(pTsaMgr->dev.privData);
            pTsaMgr->dev.privData = 0;
        }

        // free handle
        free(pTsaMgr);
        *ppHTsa = 0;
    }
    return result;
}

TSA_ERR
tsa_Init(
    TSA_HANDLE      *pHTsa,
    TSA_INIT_PARAM  *pInit_param,
    void            *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        uint32_t    i = 0;
        TSA_AGR     tsaArg = {0};

        do{
            if( pInit_param->total_demod_port > TSA_MAX_PORT_NUM )
                break;

            pTsaMgr->dev.total_demod_port = pInit_param->total_demod_port;
            pTsaMgr->dev.tsa_mode         = pInit_param->tsa_mode;
            pTsaMgr->dev.bus_type         = pInit_param->bus_type;
            pTsaMgr->dev.aggre_i2c_addr   = pInit_param->aggre_i2c_addr;

            for(i = 0; i < pInit_param->total_demod_port; i++)
            {
                pTsaMgr->dev.tag_value[i]             = pInit_param->tag_value[i];
                pTsaMgr->dev.demod_i2c_addr[i]        = pInit_param->demod_i2c_addr[i];
                pTsaMgr->dev.linked_aggre_port_idx[i] = pInit_param->linked_aggre_port_idx[i];
            }

            if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->init )
                rst = pTsaMgr->pTsaDesc->init(&pTsaMgr->dev, &tsaArg, extraData);
        }while(0);
    }
    return rst;
}


TSA_ERR
tsa_Deinit(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->deinit )
            rst = pTsaMgr->pTsaDesc->deinit(&pTsaMgr->dev, pTsaArg, extraData);
    }
    return rst;
}

TSA_ERR
tsa_Enable_Port(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->enable_port )
            rst = pTsaMgr->pTsaDesc->enable_port(&pTsaMgr->dev, pTsaArg, extraData);
    }
    return rst;
}

TSA_ERR
tsa_Disable_Port(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->disable_port )
            rst = pTsaMgr->pTsaDesc->disable_port(&pTsaMgr->dev, pTsaArg, extraData);
    }
    return rst;
}

TSA_ERR
tsa_Set_Aggre_Mode(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->set_aggre_mode )
            rst = pTsaMgr->pTsaDesc->set_aggre_mode(&pTsaMgr->dev, pTsaArg, extraData);
    }
    return rst;
}


TSA_ERR
tsa_Control(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData)
{
    TSA_ERR     rst = TSA_ERR_OK;
    TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);

    if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
    {
        if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->control )
            rst = pTsaMgr->pTsaDesc->control(&pTsaMgr->dev, pTsaArg, extraData);
    }
    return rst;
}


// TSA_ERR
// tsa_template(
//     TSA_HANDLE     *pHTsa,
//     TSA_AGR        *pTsaArg,
//     void           *extraData)
// {
//     TSA_ERR     rst = TSA_ERR_OK;
//     TSA_MGR     *pTsaMgr = DOWN_CAST(TSA_MGR, pHTsa, hTsa);
//
//     if( pTsaMgr && pTsaMgr->tsaStatus != TSA_STATUS_FAIL )
//     {
//         if( pTsaMgr->pTsaDesc && pTsaMgr->pTsaDesc->template_func )
//             rst = pTsaMgr->pTsaDesc->template_func(&pTsaMgr->dev, pTsaArg, extraData);
//     }
//     return rst;
// }






#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list_template.h"
#include "ts_demuxer_defs.h"
#include "ts_channel_info.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum _CHNL_STATUS_TAG
{
    CHNL_STATUS_IDLE = 0x11,
    CHNL_STATUS_BUSY = 0xBB,
    CHNL_STATUS_FAIL = 0xFF, 

}CHNL_STATUS;

typedef enum _CHNL_FIND_CMP_TYPE_TAG
{
    CHNL_FIND_CMP_ORDER_NUM, 
    CHNL_FIND_CMP_FREQ,
    CHNL_FIND_CMP_CUSTOMIZE_0, 
    
}CHNL_FIND_CMP_TYPE;
//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _chnl_cmp_data_tag
{
    uint32_t    frequency;
    uint32_t    bandwidth;
    
}chnl_cmp_data_t;

typedef struct _TS_CHNL_DB_TAG
{
    TS_CHNL_HANDLE      hChnl;
    
    CHNL_STATUS         chnlStatus;

    TS_CHNL_INFO        *pStartChnlInfo;
    TS_CHNL_INFO        *pCurChnlInfo;
    
    uint32_t            totalChnls;
    uint32_t            order_num;

}TS_CHNL_DB;
//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================
DEFINE_LIST_TEMPLATE(TS_CHNL_INFO);

static int
_Search_Func(
    int             cmpMode,
    TS_CHNL_INFO    *pCur, 
    void            *pattern)
{
    int             rst = 0;
    chnl_cmp_data_t *pCmpData = 0;
    
    switch( cmpMode )
    {
        case CHNL_FIND_CMP_ORDER_NUM:
            rst = (pCur->order_num == (uint32_t)pattern);
            break;

        case CHNL_FIND_CMP_FREQ:
            pCmpData = (chnl_cmp_data_t*)pattern;
            
            rst = ((pCur->userInfo.frequency == pCmpData->frequency) && 
                   (pCur->userInfo.bandwidth == pCmpData->bandwidth));
            break;
    }
    return rst;
}

static uint32_t
_Add_to_Channel_List(
    TS_CHNL_DB       *pChnlDb,
    TS_CHNL_INFO     *pInsChnlInfo)
{
    TSD_ERR             result = CHNL_ERR_OK;
    TS_CHNL_INFO        *pNewChnlInfo = tsd_malloc(sizeof(TS_CHNL_INFO));
    
    do{
        if( !pNewChnlInfo )
        {
            chnl_msg_ex(CHNL_MSG_TYPE_ERR, " error, allocate fail !!");
            result = CHNL_ERR_ALLOCATE_FAIL;
            break;
        }
        
        memcpy(pNewChnlInfo, pInsChnlInfo, sizeof(TS_CHNL_INFO));
    
        if( !pChnlDb->pStartChnlInfo )    
        {
            pChnlDb->pStartChnlInfo = pChnlDb->pCurChnlInfo = pNewChnlInfo;
            LIST_INIT(pChnlDb->pCurChnlInfo);
        }
        else
        {
            pNewChnlInfo->order_num = pChnlDb->order_num;
            pChnlDb->pCurChnlInfo = LIST_ADD(TS_CHNL_INFO, pChnlDb->pCurChnlInfo, pNewChnlInfo); 
        }
    
        pChnlDb->totalChnls++; // maybe plus/minus servicie
        pChnlDb->order_num++;  // for order number, conter always plus. 
    
    }while(0);

    return result;
}

static void
_Del_Channel_List(
    TS_CHNL_DB      *pChnlDb)
{
    TS_CHNL_INFO    *tmp_chnl_info = 0;
    TS_CHNL_INFO    *pCurChnlInfo = pChnlDb->pCurChnlInfo;
    uint32_t        i;

    // need mutex
    // To Do:

    for(i = 0; i < pChnlDb->totalChnls; i++)
    {
        tmp_chnl_info = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                  CHNL_FIND_CMP_ORDER_NUM, pCurChnlInfo, i);
    
        if( tmp_chnl_info )  
        {
            pCurChnlInfo = LIST_DEL(TS_CHNL_INFO, tmp_chnl_info);
            free(tmp_chnl_info);
        }
    }

    pChnlDb->totalChnls = 0;
    pChnlDb->order_num = 0;
    pChnlDb->pStartChnlInfo = pChnlDb->pCurChnlInfo = 0;

}

static uint32_t
_Merge_Channel_List(
    TS_CHNL_DB          *pChnlDb_master,
    TS_CHNL_DB          *pChnlDb_slave,
    TS_CHNL_MERGE_TYPE  type)
{
    uint32_t            result = 0;
    chnl_cmp_data_t     cmpData = {0};
    uint32_t            i;

    do{
        switch( type )
        {
            default:        break;

            case TS_CHNL_MERGE_2_HANDLE:
                do{
                    TS_CHNL_INFO     *pCur_Master_chnl = 0;
                    TS_CHNL_INFO     *pCur_Slave_chnl = 0;

                    if( !pChnlDb_slave->totalChnls )    break;

                    pCur_Master_chnl = pChnlDb_master->hChnl.pStartChnlInfo;
                    pCur_Slave_chnl  = pChnlDb_slave->hChnl.pStartChnlInfo;

                    for(i = 0; i < pChnlDb_slave->totalChnls; i++)
                    {
                        pCur_Slave_chnl = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                                    CHNL_FIND_CMP_ORDER_NUM, 
                                                    pCur_Slave_chnl, i);

                        if( !pCur_Slave_chnl )    
                        {
                            pCur_Slave_chnl  = pChnlDb_slave->hChnl.pStartChnlInfo;
                            continue; 
                        }

                        cmpData.frequency = pCur_Slave_chnl->userInfo.frequency;
                        cmpData.bandwidth = pCur_Slave_chnl->userInfo.bandwidth;
                        
                        pCur_Master_chnl = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                                     CHNL_FIND_CMP_FREQ, 
                                                     pCur_Master_chnl, &cmpData);

                        if( !pCur_Master_chnl )
                        {
                            // add channel info node
                            result = _Add_to_Channel_List(pChnlDb_master, pCur_Slave_chnl);
                            if( result )    break;
                            
                            pCur_Master_chnl = pChnlDb_master->hChnl.pStartChnlInfo;
                        }
                    }
                }while(0);
                break;
        }

        pChnlDb_master->hChnl.pStartChnlInfo = pChnlDb_master->pStartChnlInfo;
        pChnlDb_master->hChnl.pCurChnlInfo   = pChnlDb_master->pCurChnlInfo;
        pChnlDb_master->hChnl.totalChnls     = pChnlDb_master->totalChnls; 
    }while(0);

    return result;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsChnl_CreateHandle(
    TS_CHNL_HANDLE  **pHChnl,
    void            *extraData)
{
    TSD_ERR         result = CHNL_ERR_OK;
    TS_CHNL_DB      *pChnlDb = 0; 
    
    do{
        if( *pHChnl != 0 )
        {
            chnl_msg_ex(CHNL_MSG_TYPE_ERR, " error, Exist service DB handle !!");
            result = CHNL_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete servic database handle
        pChnlDb = (TS_CHNL_DB*)tsd_malloc(sizeof(TS_CHNL_DB));
        if( !pChnlDb )
        {
            chnl_msg_ex(CHNL_MSG_TYPE_ERR, " error, allocate fail !!");
            result = CHNL_ERR_ALLOCATE_FAIL;
            break;
        }

        memset((void*)pChnlDb, 0x0, sizeof(TS_CHNL_DB));
        pChnlDb->chnlStatus = CHNL_STATUS_IDLE;
        
        // if not error
        (*pHChnl) = &pChnlDb->hChnl;
        
    }while(0);
    
    if( result != CHNL_ERR_OK )
    {
        pChnlDb->chnlStatus = CHNL_STATUS_FAIL;
        chnl_msg_ex(CHNL_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

TSD_ERR
tsChnl_DestroyHandle(
    TS_CHNL_HANDLE  **pHChnl)
{
    TSD_ERR         result = CHNL_ERR_OK;
    TS_CHNL_DB      *pChnlDb = DOWN_CAST(TS_CHNL_DB, (*pHChnl), hChnl);
    
    if( pChnlDb )
    {
        TS_CHNL_INFO    *tmp_chnl_info = 0;
        TS_CHNL_INFO    *pCurChnlInfo = pChnlDb->hChnl.pCurChnlInfo;
        uint32_t        i;
        
        // --------------------------
        // reset globle variable
        for(i = 0; i < pChnlDb->totalChnls; i++)
        {
            tmp_chnl_info = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                      CHNL_FIND_CMP_ORDER_NUM, pCurChnlInfo, i);

            if( tmp_chnl_info )  
            {
                pCurChnlInfo = LIST_DEL(TS_CHNL_INFO, tmp_chnl_info);
                free(tmp_chnl_info);
            }
        }
        
        // free handle
        free(pChnlDb);
        *pHChnl = 0;    
    }
    
    return result;
}

TSD_ERR
tsChnl_GetChannelInfo(
    TS_CHNL_HANDLE      *pHChnl,
    uint32_t            orderIdx,
    TS_CHNL_INFO        *pChnl_info_start,
    TS_CHNL_INFO        **pChnl_info,
    void                *extraData)
{
    TSD_ERR         result = CHNL_ERR_OK;
    TS_CHNL_DB      *pChnlDb = DOWN_CAST(TS_CHNL_DB, pHChnl, hChnl);
    
    if( pChnlDb && pChnlDb->chnlStatus != CHNL_STATUS_FAIL )
    {
        TS_CHNL_INFO    *pCurChnlInfo = pChnlDb->hChnl.pCurChnlInfo;
        
        if( !pChnl_info_start )
            *pChnl_info = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                    CHNL_FIND_CMP_ORDER_NUM, pCurChnlInfo, orderIdx);
        else
            *pChnl_info = LIST_FIND(TS_CHNL_INFO, _Search_Func, 
                                    CHNL_FIND_CMP_ORDER_NUM, pChnl_info_start, orderIdx);        
        
    }
    
    if( result != CHNL_ERR_OK )
    {
        pChnlDb->chnlStatus = CHNL_STATUS_FAIL;
        chnl_msg_ex(CHNL_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }     
    return result;
}

TSD_ERR
tsChnl_AddChannel(
    TS_CHNL_HANDLE  *pHChnl,
    TS_CHNL_INFO    *pInsChnlInfo,
    uint32_t        *curChnlIdx,
    void            *extraData)
{
    TSD_ERR         result = CHNL_ERR_OK;
    TS_CHNL_DB      *pChnlDb = DOWN_CAST(TS_CHNL_DB, pHChnl, hChnl);
    
    if( pChnlDb && pChnlDb->chnlStatus != CHNL_STATUS_FAIL )
    {
        if( curChnlIdx )    *curChnlIdx = pChnlDb->order_num;
        
        result = _Add_to_Channel_List(pChnlDb, pInsChnlInfo);

        pChnlDb->hChnl.pStartChnlInfo = pChnlDb->pStartChnlInfo;
        pChnlDb->hChnl.pCurChnlInfo   = pChnlDb->pCurChnlInfo;
        pChnlDb->hChnl.totalChnls     = pChnlDb->totalChnls;
    }
    
    if( result != CHNL_ERR_OK )
    {
        pChnlDb->chnlStatus = CHNL_STATUS_FAIL;
        chnl_msg_ex(CHNL_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }     
    return result;
}


TSD_ERR
tsChnl_Control(
    TS_CHNL_HANDLE  *pHChnl,
    TS_CHNL_CTL_CMD cmd,
    uint32_t        *value,
    void            *extraData)
{
    TSD_ERR         result = CHNL_ERR_OK;
    TS_CHNL_DB      *pChnlDb = DOWN_CAST(TS_CHNL_DB, pHChnl, hChnl);
    
    if( pChnlDb && pChnlDb->chnlStatus != CHNL_STATUS_FAIL )
    {
        switch( cmd )
        {
            case TS_CHNL_CTL_DEL_ALL_CHNL_INFO:
                _Del_Channel_List(pChnlDb);
                
                pChnlDb->hChnl.pStartChnlInfo = pChnlDb->pStartChnlInfo;
                pChnlDb->hChnl.pCurChnlInfo   = pChnlDb->pCurChnlInfo;
                pChnlDb->hChnl.totalChnls     = pChnlDb->totalChnls;
                break;

            case TS_CHNL_CTL_MERGE_CHNL_INFO:
                _Merge_Channel_List(pChnlDb, (TS_CHNL_DB*)extraData, (TS_CHNL_MERGE_TYPE)value);
                
                pChnlDb->hChnl.pStartChnlInfo = pChnlDb->pStartChnlInfo;
                pChnlDb->hChnl.pCurChnlInfo   = pChnlDb->pCurChnlInfo;
                pChnlDb->hChnl.totalChnls     = pChnlDb->totalChnls;                
                break;
                
            default:
                result = TSP_ERR_NO_IMPLEMENT;
                break;
        }
    }
    
    if( result != CHNL_ERR_OK &&
        result != TSP_ERR_NO_IMPLEMENT )
    {
        pChnlDb->chnlStatus = CHNL_STATUS_FAIL;
        chnl_msg_ex(CHNL_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }     
    return result;
}

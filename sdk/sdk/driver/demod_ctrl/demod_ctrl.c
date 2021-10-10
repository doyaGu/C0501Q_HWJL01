
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "register_template.h"
#include "demod_config.h"
#include "demod.h"
#include "demod_ctrl.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #define DEMODCTRL_MAX_SUPPORT_CNT         CFG_DEMOD_SUPPORT_COUNT // 4
#else
    #define DEMODCTRL_MAX_SUPPORT_CNT         1
#endif


typedef enum _DEM_STATUS_TAG
{
    DEM_STATUS_IDLE = 0x11,
    DEM_STATUS_BUSY = 0xBB,
    DEM_STATUS_FAIL = 0xFF,

}DEM_STATUS;

/**
 * record demod used index.
 *               demod_(n-1)      demod_1  demod_0
 *    |-------|-------|-- .......... --|-------|
 *   MSB                                       LSB
 **/
#define DEMOD_BIT_FIELD_SIZE                 (DEMODCTRL_MAX_SUPPORT_CNT+1)
typedef struct DEMOD_MAPPING_T
{
    uint16_t    bits_field[((DEMOD_BIT_FIELD_SIZE)+0xF)>>4];
}DEMOD_MAPPING;
#define DEMOD_BIT_SET(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] |=  (1<<((bit_order)&0xF)))
#define DEMOD_BIT_CLR(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] &= ~(1<<((bit_order)&0xF)))
#define DEMOD_BIT_IS_SET(pZone_set_member, bit_order)  ((pZone_set_member)->bits_field[(bit_order)>>4] &   (1<<((bit_order)&0xF)))
#define DEMOD_BIT_ZERO(pZone_set_member)               memset((void*)(pZone_set_member),0,sizeof(DEMOD_MAPPING))

//=============================================================================
//                Macro Definition
//=============================================================================

DEFINE_REGISTER_TEMPLATE(DEMOD_DESC, DEMOD_TYPE_ID);
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _DEMOD_CTRL_MGR_TAG
{
    DEMOD_CTRL_HANDLE   hDemodCtrl;

    DEM_STATUS          demStatus;

    DEMOD_DEV           dev;
    DEMOD_DESC          *demodDesc;

}DEMOD_CTRL_MGR;


/**
 * for the damn demod H/W architecture
 **/
typedef struct DEMOD_COMP_T
{
    DEMOD_MAPPING       used_flag;
    DEMOD_MAPPING       init_flag; // bInitialized

    DEMOD_MAPPING       verified_flag;

    DEMOD_ATTR          demod_attr[DEMODCTRL_MAX_SUPPORT_CNT];
}DEMOD_COMP;

//=============================================================================
//                Global Data Definition
//=============================================================================
uint32_t  demMsgOnFlag = 0x1;

static DEMOD_COMP    g_demod_comp = {0};
//=============================================================================
//                Private Function Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 1)
static uint32_t
_Check_Demod_Eng_Status(
    DEMOD_CTRL_MGR      **ppDemCtl)
{
    bool            bRepeat = false;
    DEMOD_CTRL_MGR  *pDemCtl = (*ppDemCtl);
    uint32_t        demod_idx = 0;

    if( pDemCtl )
    {
        DEMOD_ATTR          *pCur_demod_attr = 0;

        for(demod_idx = 0; demod_idx < DEMODCTRL_MAX_SUPPORT_CNT; demod_idx++)
        {
            if( DEMOD_BIT_IS_SET(&g_demod_comp.verified_flag, demod_idx) )
                continue;

            DEMOD_BIT_SET(&g_demod_comp.verified_flag, demod_idx);

            pCur_demod_attr = &g_demod_comp.demod_attr[demod_idx];
            if( pCur_demod_attr->bus_type != DEMOD_BUS_I2C )
                continue;

            if( !(DEMOD_BIT_IS_SET(&g_demod_comp.init_flag, demod_idx)) )
            {
                dem_msg_ex(1, " %3d-th demod not be Initialized\n");
                dem_msg(0, "     bus_type=0x%x, demod_chip=0x%x, i2c_addr=0x%x\n",
                                 pCur_demod_attr->bus_type,
                                 pCur_demod_attr->demod_type,
                                 pCur_demod_attr->demod_i2c_addr);
                continue;
            }

            if( !(DEMOD_BIT_IS_SET(&g_demod_comp.used_flag, demod_idx)) )
            {
                // idle state => need to sync others
                *ppDemCtl = (DEMOD_CTRL_MGR*)g_demod_comp.demod_attr[demod_idx].privData;
                bRepeat = true;
                break;
            }
        }
    }

    return (uint32_t)bRepeat;
}
#else
    #define _Check_Demod_Eng_Status(a)      0
#endif

//=============================================================================
//                Public Function Definition
//=============================================================================
void
DemodCtrl_Register_all(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(DEMOD_DESC, OMEGA, omega);
    REGISTER_ITEM(DEMOD_DESC, IT9135, it9135);
    REGISTER_ITEM(DEMOD_DESC, IT9137, it9137);
    REGISTER_ITEM(DEMOD_DESC, IT9137_USB, it9137_usb);
}

uint32_t
DemodCtrl_CreateHandle(
    DEMOD_CTRL_HANDLE   **pHDemodCtrl,
    DEMOD_ATTR          *pDemodAttr)
{
    DEM_ERR         result = DEM_ERR_OK;
    DEMOD_CTRL_MGR  *pDemCtl = 0;
    uint32_t        demod_idx = 0;

    do{
        if( !pHDemodCtrl || (*pHDemodCtrl) || !pDemodAttr )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, " error, invalid parameter !!");
            result = DEM_ERR_INVALID_PARAMETER;
            break;
        }

        if( pDemodAttr->demod_idx >= DEMODCTRL_MAX_SUPPORT_CNT )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, " err, wrong demod index (max=%d) !!", DEMODCTRL_MAX_SUPPORT_CNT - 1);
            result = DEM_ERR_INVALID_PARAMETER;
            break;
        }

        demod_idx = pDemodAttr->demod_idx;
        if( DEMOD_BIT_IS_SET(&g_demod_comp.init_flag, demod_idx) )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, " error, the demode index already be initialized !!");
            result = DEM_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pDemCtl = dem_malloc(sizeof(DEMOD_CTRL_MGR));
        if( !pDemCtl )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, " error, allocate fail !!");
            result = DEM_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pDemCtl, 0, sizeof(DEMOD_CTRL_MGR));

        pDemCtl->hDemodCtrl.demodId = demod_idx;
        pDemCtl->dev.demodId        = demod_idx;

        // ------------------------
        // regist demod
        DemodCtrl_Register_all();
        pDemCtl->demodDesc = FIND_DESC_ITEM(DEMOD_DESC, pDemodAttr->demod_type);
        if( !pDemCtl->demodDesc )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, " error, can't find demod description (id=%d)!!", pDemodAttr->demod_type);
        }
        //------------------------------
        // set demod attribute
        g_demod_comp.demod_attr[demod_idx]        = (*pDemodAttr);
        pDemCtl->dev.demod_attr                   = (*pDemodAttr);
        pDemCtl->hDemodCtrl.linked_aggre_port_idx = pDemodAttr->linked_aggre_port_idx;
        printf(" ---- demod_attr: idx=%d, bus=%d, chip_type=%d, i2cAddr=0x%x, linked_aggre_port=0x%x\n",
                pDemCtl->dev.demod_attr.demod_idx,
                pDemCtl->dev.demod_attr.bus_type,
                pDemCtl->dev.demod_attr.demod_type,
                pDemCtl->dev.demod_attr.demod_i2c_addr,
                pDemCtl->hDemodCtrl.linked_aggre_port_idx);

        //------------------------------
        (*pHDemodCtrl) = &pDemCtl->hDemodCtrl;

        DEMOD_BIT_SET(&g_demod_comp.init_flag, demod_idx);
        g_demod_comp.demod_attr[pDemCtl->hDemodCtrl.demodId].privData = (void*)pDemCtl;
    }while(0);

    if( result != DEM_ERR_OK )
    {
        if( pDemCtl )
        {
            DEMOD_CTRL_HANDLE   *pHTmp = &pDemCtl->hDemodCtrl;
            DemodCtrl_DestroyHandle(&pHTmp);
        }
        dem_msg_ex(DEM_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

uint32_t
DemodCtrl_DestroyHandle(
    DEMOD_CTRL_HANDLE   **pHDemodCtrl)
{
    DEM_ERR         result = DEM_ERR_OK;
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, (*pHDemodCtrl), hDemodCtrl);

    if( pHDemodCtrl && (*pHDemodCtrl) && pDemCtl )
    {
        if( pDemCtl->dev.privData )
        {
            free(pDemCtl->dev.privData);
            pDemCtl->dev.privData = 0;
        }

        DEMOD_BIT_CLR(&g_demod_comp.used_flag, pDemCtl->hDemodCtrl.demodId);
        DEMOD_BIT_CLR(&g_demod_comp.init_flag, pDemCtl->hDemodCtrl.demodId);
        g_demod_comp.demod_attr[pDemCtl->hDemodCtrl.demodId].privData = 0;

        // free handle
        free(pDemCtl);
        *pHDemodCtrl = 0;
    }

    return result;
}
//=============================================================================
/**
 * Used to Init the front end demodulator and turner.
 * @return none.
 */
//=============================================================================
uint32_t
DemodCtrl_Init(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_SETUP_INFO    *pSetupInfo)
{
    DEM_ERR         result = DEM_ERR_OK;
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);

    if( pHDemodCtrl && pDemCtl && pDemCtl->demStatus != DEM_STATUS_FAIL )
    {
        pDemCtl->dev.supportType  = pSetupInfo->supportType;
        pDemCtl->dev.architecture = pSetupInfo->architecture;

        if( pDemCtl->demodDesc && pDemCtl->demodDesc->DemodCtrl_Init )
            pDemCtl->demodDesc->DemodCtrl_Init(&pDemCtl->dev, 0);

        DEMOD_BIT_CLR(&g_demod_comp.used_flag, pDemCtl->dev.demodId);
    }
    return result;
}

//=============================================================================
/**
 * Used to Terminate the inited front end demodulator and turner.
 * @return none.
 */
//=============================================================================
uint32_t
DemodCtrl_Terminate(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    DEM_ERR         result = DEM_ERR_OK;
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);

    // it need to sync with other demod ???????

    if( pHDemodCtrl && pDemCtl && pDemCtl->demStatus != DEM_STATUS_FAIL )
    {
        if( pDemCtl->demodDesc && pDemCtl->demodDesc->DemodCtrl_Terminate )
            pDemCtl->demodDesc->DemodCtrl_Terminate(&pDemCtl->dev, 0);
    }
    return result;
}

//=============================================================================
/**
 * By specifying the frequency and bandwith to detech if any channel locked or
 * existed through the setting parameters.
 * @param frequency The desired checking frequency by unit of kHz.
 * @param bandwith  The bandwith of the specific frequency channel. Usually,
 *                  the bandwith is diverse by different countries.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
uint32_t
DemodCtrl_AcquireChannel(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    uint32_t        frequency = 0;
    uint32_t        bandwith  = 0;
    int             i = 0;

    if( pHDemodCtrl && pDemCtl && pDemCtl->demStatus != DEM_STATUS_FAIL )
    {
        DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

        DEMOD_BIT_SET(&g_demod_comp.used_flag, pDemCtl->dev.demodId);

        frequency = pDemCtl->hDemodCtrl.frequency;
        bandwith  = pDemCtl->hDemodCtrl.bandwith;

        pCurDemCtl = pDemCtl;

        do{
            if( pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
            {
                pCurDemCtl->dev.frequency = frequency;
                pCurDemCtl->dev.bandwith  = bandwith;

                if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_AcquireChannel )
                {
                    rst[i] = pCurDemCtl->demodDesc->DemodCtrl_AcquireChannel(&pCurDemCtl->dev, 0);
                    DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
                }
            }

            if( rst[i] )
            {
                pCurDemCtl->demStatus = DEM_STATUS_FAIL;
                dem_msg_ex(DEM_MSG_TYPE_ERR, "demod(%d-th) %s() err 0x%x !", pCurDemCtl->dev.demodId, __FUNCTION__, rst[i]);
            }
            i++;
        }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));
    }
    return rst[0];
}

//=============================================================================
/**
 * Check if the demod is locked after channel acquisition step.
 * @return MMP_BOOL If the demod is locked then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
bool
DemodCtrl_IsChannelLock(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_IsChannelLock )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_IsChannelLock(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Read TS stream Datagram from USB interface
 * @param demodId   control a specific demod and tuner.
 * @return MMP_BOOL If got Data then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
bool
DemodCtrl_ReadDataStream(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    uint8_t              *buffer,
    uint32_t             bufferlength,
    void                 *pfCallback,
    void                 *pCtrlParam)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_ReadDataStream )
            {
                pCurDemCtl->dev.pApCtrlParam = pCtrlParam;

                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_ReadDataStream(&pCurDemCtl->dev, buffer, bufferlength, pfCallback);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0);

    return rst[0];
}

//=============================================================================
/**
 * Check if TPS locked after channel acquisition step.
 * @return MMP_BOOL If the TPS is locked then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
bool
DemodCtrl_IsTpsLock(
     DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_IsTpsLock )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_IsTpsLock(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}


//=============================================================================
/**
 * Check if MPEG2 locked after channel acquisition step.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
bool
DemodCtrl_IsMpeg2Lock(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_IsMpeg2Lock )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_IsMpeg2Lock(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Retrieve current signal statistic from demodulator.
 * @param ptDemodStatistic  The output of current signal status if return
 *                          is MMP_TRUE.
 * @return Whether the operation is success or not.
 */
//=============================================================================
bool
DemodCtrl_GetSignalStatus(
    DEMOD_CTRL_HANDLE       *pHDemodCtrl,
    DEMOD_SIGNAL_STATISTIC  *ptDemodStatistic)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_GetSignalStatus )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_GetSignalStatus(&pCurDemCtl->dev, ptDemodStatistic, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Update the PID filter table by specified PID. For a new PID just inserts it
 * into the PID filter table, update the table otherwise.
 * @param pid       The desired filtering PID.
 * @param pidType   The desired PID Type.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
uint32_t
DemodCtrl_UpdatePidTable(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    uint32_t            pid,
    PID_FILTER_INDEX    pidIndex)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_UpdatePidTable )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_UpdatePidTable(&pCurDemCtl->dev, pid, pidIndex, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Reset the PID filter table
 * @param  bFilterOn    The desired filtering PID.
 * @return uint32_t   the error code from demod control API if any.
 */
//=============================================================================
uint32_t
DemodCtrl_ResetPidTable(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    bool                bFilterOn)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_ResetPidTable )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_ResetPidTable(&pCurDemCtl->dev, bFilterOn, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while( _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * To setup whether the demond enter the suspend state or not.
 * @param bEnableSuspend    if MMP_TRUE, the demod will enter the suspend state.
 * @return MMP_BOOL         Whether the mode change success.
 */
//=============================================================================
bool
DemodCtrl_SetSuspend(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    bool                bSuspend)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_SetSuspend )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_SetSuspend(&pCurDemCtl->dev, bSuspend, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Open Tuner
 * @return MMP_BOOL Whether the action success.
 */
//=============================================================================
bool
DemodCtrl_OpenTuner(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_OpenTuner )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_OpenTuner(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Close Tuner
 * @return MMP_BOOL Whether the action success.
 */
//=============================================================================
bool
DemodCtrl_CloseTuner(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    bool            rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_CloseTuner )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_CloseTuner(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Adjust Orion DCXO
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_GetDemodFwVersion(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_GetDemodFwVersion )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_GetDemodFwVersion(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}


//=============================================================================
/**
 * Enable DEMOD Retrain
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_EnableRetrain(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_EnableRetrain )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_EnableRetrain(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Disable DEMOD Retrain
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_DisableRetrain(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_DisableRetrain )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_DisableRetrain(&pCurDemCtl->dev, 0);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

//=============================================================================
/**
 * Used to get demodulator information.
 * @return none.
 */
//=============================================================================
void
DemodCtrl_GetBer(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    uint32_t            *pErrorCount,
    uint32_t            *pBitCount,
    uint16_t            *pAbortCount,
    uint8_t             *pExtData)
{
    //uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    uint32_t        errorCount[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    uint32_t        bitCount[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    uint16_t        abortCount[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_GetBer )
            {
                pCurDemCtl->demodDesc->DemodCtrl_GetBer(&pCurDemCtl->dev, &errorCount[i], &bitCount[i], &abortCount[i], pExtData);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    if( pErrorCount )   *pErrorCount = errorCount[0];
    if( pBitCount )     *pBitCount   = bitCount[0];
    if( pAbortCount )   *pAbortCount = abortCount[0];

    return ;
}

//=============================================================================
/**
 * To Power Down the demond.
 * note iic address is 0x20
 */
//=============================================================================
uint32_t
DemodCtrl_SetPowerDown(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    bool                bEnable)
{
    uint32_t        rst = 0;
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);

    if( pHDemodCtrl && pDemCtl->demStatus != DEM_STATUS_FAIL )
    {
        if( pDemCtl->demodDesc && pDemCtl->demodDesc->DemodCtrl_SetPowerDown )
            rst = pDemCtl->demodDesc->DemodCtrl_SetPowerDown(&pDemCtl->dev, bEnable, 0);
    }
    return rst;
}


uint32_t
DemodCtrl_Control(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_CTRL_CMD      cmd,
    uint32_t            *value,
    void                *extraData)
{
    uint32_t        rst[DEMODCTRL_MAX_SUPPORT_CNT] = {0};
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);
    DEMOD_CTRL_MGR  *pCurDemCtl = 0;
    int             i = 0;

    DEMOD_BIT_ZERO(&g_demod_comp.verified_flag);

    pCurDemCtl = pDemCtl;
    do{
        if( pHDemodCtrl && pCurDemCtl && pCurDemCtl->demStatus != DEM_STATUS_FAIL )
        {
            if( pCurDemCtl->demodDesc && pCurDemCtl->demodDesc->DemodCtrl_Control )
            {
                rst[i] = pCurDemCtl->demodDesc->DemodCtrl_Control(&pCurDemCtl->dev, cmd, value, extraData);
                DEMOD_BIT_SET(&g_demod_comp.verified_flag, pCurDemCtl->dev.demodId);
            }
        }
        i++;
    }while(0 && _Check_Demod_Eng_Status(&pCurDemCtl) && (i < DEMODCTRL_MAX_SUPPORT_CNT));

    return rst[0];
}

uint32_t
DemodCtrl_Set_Engine_Status(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_ENG_STATUS    status)
{
    uint32_t        rst = 0;
    DEMOD_CTRL_MGR  *pDemCtl = DOWN_CAST(DEMOD_CTRL_MGR, pHDemodCtrl, hDemodCtrl);

    if( pHDemodCtrl && pDemCtl->demStatus != DEM_STATUS_FAIL )
    {
        uint32_t    demodId = (pDemCtl->dev.demodId % DEMODCTRL_MAX_SUPPORT_CNT);

        if( DEMOD_BIT_IS_SET(&g_demod_comp.init_flag, demodId) )
        {
            switch( status )
            {
                case DEMOD_ENG_STATUS_IDLE:     DEMOD_BIT_CLR(&g_demod_comp.used_flag, demodId); break;
                case DEMOD_ENG_STATUS_RUNNING:  DEMOD_BIT_SET(&g_demod_comp.used_flag, demodId); break;
            }
        }
        else
            dem_msg_ex(DEM_MSG_TYPE_ERR, "err, demod (%d-th) not be initialized !", demodId);
    }
    return rst;
}

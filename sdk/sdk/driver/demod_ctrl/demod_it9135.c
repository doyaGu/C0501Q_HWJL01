
#include <stdio.h>
#include <string.h>
#include "demod.h"

#if CONFIG_DEMOD_DESC_IT9135_DESC
#include "./it9135/it9130.h" // lazy demod guy
#include "./it9135/user.h"
#include "iic/mmp_iic.h"
#include "./it9135/tuner/tuner.h"
#include "./standard.h"
#include "ite/itp.h"
#include "ite/ite_usbex.h"
#include "./usb_demod/usb_demod.h"
#include "./usb_demod/usb_demod_transport.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define DEM_9135_MSG_INFO       1 // shwo msg

#define CORE_DCA
#ifdef CORE_DCA
    #define TOTAL_DEMOD_CHIP_NUMBER         (2) // diversity or Dual-Channel
#else
    #define TOTAL_DEMOD_CHIP_NUMBER         (1) // single
#endif
#define ACTIVE_DEMOD_CHIP_ID            (0)
#define SAW_INIT_BANDWIDTH              (8000)

#if !defined(CFG_AGGRE_ENABLE) && defined(CFG_TSI_PARALLEL_MODE) 
    #define DEMOD_INIT_STREAM_TYPE          (StreamType_DVBT_PARALLEL)
#else
    #define DEMOD_INIT_STREAM_TYPE          (StreamType_DVBT_SERIAL)
#endif

#define DEMOD_INIT_ARCHITECTURE         (Architecture_DCA)



#define PAL_HEAP_DEFAULT        0
#define GPIO_PADSEL_REG         0x90

// AMBA
#define AHB_ReadRegister(add, data)                 (*data = ithReadRegA(add))
#define AHB_WriteRegister(add, data)                ithWriteRegA(add, data)
#define AHB_WriteRegisterMask(add, data, mark)      ithWriteRegMaskA(add, data, mark)

typedef void (*DEMODCALLBACK)(void* pCallbackData);
typedef void (*UPPERCALLBACK)(uint32_t index, void *extraData);

#if defined(CFG_GPIO_DEMOD_SUSPEND) && (CFG_GPIO_DEMOD_SUSPEND >= 0)

    #define REG_BASE_GPIO_SEL_MODE      (ITH_GPIO_BASE + 0x90)
    #define REG_GPIO_DATA_OUT           (ITH_GPIO_BASE + 0x40)
    #define REG_GPIO_PIN_DIRECTION      (ITH_GPIO_BASE + 0x48)
    
#if (CFG_GPIO_DEMOD_SUSPEND > 47)
    // 48 ~ 63
    #define GPIO_SEL_MODE_SHIFT_BITS    ((CFG_GPIO_DEMOD_SUSPEND-48) << 1)
    #define GPIO_PIN_SEL_SHIFT_BITS     (CFG_GPIO_DEMOD_SUSPEND-32)
#elif (CFG_GPIO_DEMOD_SUSPEND > 31)
    // 32 ~ 47
    #define GPIO_SEL_MODE_SHIFT_BITS    ((CFG_GPIO_DEMOD_SUSPEND-32) << 1)
    #define GPIO_PIN_SEL_SHIFT_BITS     (CFG_GPIO_DEMOD_SUSPEND-32)
#elif (CFG_GPIO_DEMOD_SUSPEND > 15)
    // 16 ~ 31
    #define GPIO_SEL_MODE_SHIFT_BITS    ((CFG_GPIO_DEMOD_SUSPEND-16) << 1)
    #define GPIO_PIN_SEL_SHIFT_BITS     CFG_GPIO_DEMOD_SUSPEND
#else
    // 0 ~ 15
    #define GPIO_SEL_MODE_SHIFT_BITS    (CFG_GPIO_DEMOD_SUSPEND << 1)
    #define GPIO_PIN_SEL_SHIFT_BITS     CFG_GPIO_DEMOD_SUSPEND
#endif

#endif

// for switch demod i2c addr
#define DEMOD_ACT_SWITCH_GPIO_0             CFG_DEMOD_SWITCH_GPIO_0
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct DEMOD_IT9135_INFO_TAG
{
    // usb demod (can't change member order)
    USB_CALLBACK_CONTEXT  usbCbCtxt;

    UPPERCALLBACK         pfUpperCallback;
    void*                 pCallbackData;

    IT9130                it9130Demodulator;
    uint8_t               supportType; // turner support type
    bool                  bEnableSuspend;
    bool                  bFilterOn;
    uint32_t              totalPmtCount;
    uint32_t              filterTable[DEMOD_TOTAL_FILTER_COUNT];
    uint32_t              shareBuffer;
} DEMOD_IT9135_INFO;

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
_DEMODCTRL_Lock(
    DEMOD_ATTR  *pDemod_attr)
{
    if( pDemod_attr->bus_type == DEMOD_BUS_I2C )
    {
        mmpIicLockModule(IIC_PORT_0);
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
_DEMODCTRL_unLock(
    DEMOD_ATTR  *pDemod_attr)
{
    if( pDemod_attr->bus_type == DEMOD_BUS_I2C )
    {
        mmpIicReleaseModule(IIC_PORT_0);
    }
}

static void
_it9135_Usb_ReadCallback(
    void* pContext)
{
    DEMOD_IT9135_INFO *pIt9135Info = DOWN_CAST(DEMOD_IT9135_INFO, pContext, usbCbCtxt);
    DEMOD_DEV         *dev = (DEMOD_DEV*)pIt9135Info->pCallbackData;

    //printf("callback!!\n");
    if( pIt9135Info->pfUpperCallback )
    {
        (*pIt9135Info->pfUpperCallback)((dev->demodId & 0x1), dev->pApCtrlParam);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Used to Init the front end demodulator and turner.
 * @return none.
 */
//=============================================================================
static void
Demod_9135_Init(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    uint32_t            result = 0, it9135_result = 0;
    DEMOD_IT9135_INFO   *pIt9135Info = 0;

    _DEMODCTRL_Lock(&dev->demod_attr);

    if( 0 == dev->privData )
    {
        do{
            DEMOD_ATTR           *pDemod_attr = &dev->demod_attr;

            // create private info
            pIt9135Info = (DEMOD_IT9135_INFO*)dem_malloc(sizeof(DEMOD_IT9135_INFO));
            if( !pIt9135Info )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Memory allocation is failed for Demod omega info\n");
                break;
            }
            memset((void*)pIt9135Info, 0x0, sizeof(DEMOD_IT9135_INFO));

            switch( pDemod_attr->bus_type )
            {
                case DEMOD_BUS_I2C:  
                    pIt9135Info->it9130Demodulator.busId        = Bus_I2C;
                    pIt9135Info->it9130Demodulator.demodAddr    = pDemod_attr->demod_i2c_addr;
                    pIt9135Info->it9130Demodulator.streamType   = DEMOD_INIT_STREAM_TYPE; 
                    pIt9135Info->supportType                    = OMEGA_LNA_Config_2;
                    break;

                case DEMOD_BUS_USB:  
                    pIt9135Info->it9130Demodulator.busId        = Bus_USB;
                    pIt9135Info->it9130Demodulator.demodAddr    = pDemod_attr->demod_i2c_addr;
                    pIt9135Info->it9130Demodulator.streamType   = StreamType_DVBT_DATAGRAM; 
                    pIt9135Info->supportType                    = OMEGA_NORMAL;
                    break;
            }
            
            pIt9135Info->it9130Demodulator.architecture = Architecture_PIP;

            switch( pIt9135Info->it9130Demodulator.busId )
            {
                case Bus_USB:
                    {
                        ITPUsbInfo      itpUsbInfo = {0};
                        int             index = (dev->demodId & 0x1);
                        struct timeval  startT;

                        itpUsbInfo.host     = true;
                        itpUsbInfo.usbIndex = index;
                        dem_get_clock(&startT);

                        do{
                            ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, &itpUsbInfo);

                            usleep(50000);
                            if( dem_get_duration(&startT) > 2500 )
                            {
                                dem_msg_ex(1, "Wait IT9135 Usb Deomd (%d-th) insert time out !! ", dev->demodId);
                                result = 1;
                                break;
                            }
                        }while( !itpUsbInfo.ctxt );

                        pIt9135Info->it9130Demodulator.usb_info = itpUsbInfo.ctxt;
                    }
                    break;

                default:
                    pIt9135Info->it9130Demodulator.usb_info = 0;
                    break;
            }
            if( result )    break;


dem_msg(1, "\n\ndemodAddr=0x%x, supportType=%d, streamType=%d (%s), ",
    pIt9135Info->it9130Demodulator.demodAddr, pIt9135Info->supportType, pIt9135Info->it9130Demodulator.streamType,
    (pIt9135Info->it9130Demodulator.streamType == StreamType_DVBT_DATAGRAM)? "usb" : "onBoard");
    switch( pIt9135Info->it9130Demodulator.streamType )
    {
        case StreamType_DVBT_SERIAL:    dem_msg(1, "interface = serial\n"); break;
        case StreamType_DVBT_PARALLEL:  dem_msg(1, "interface = parallel\n"); break;
        case StreamType_DVBT_DATAGRAM:  dem_msg(1, "interface = datagram\n"); break;
        default:    dem_msg(1, "\n"); break;
    }

#if (CFG_DEMOD_SUPPORT_COUNT > 2) && (CFG_DEMOD_SWITCH_GPIO_0 >= 0)
            // set gpio for switch demod 
            ithGpioSetOut(DEMOD_ACT_SWITCH_GPIO_0);
            ithGpioEnable(DEMOD_ACT_SWITCH_GPIO_0); 
#endif    
            User_createCriticalSection();

            it9135_result = OMEGA_supportLNA((Demodulator*)&(pIt9135Info->it9130Demodulator), pIt9135Info->supportType);
            if( it9135_result )  dem_msg_ex(DEM_MSG_TYPE_ERR,"[it9135_INIT] ERROR = 0x%x \n", it9135_result);

            result = Demodulator_initialize((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                            1,//(pIt9135Info->it9130Demodulator.busId == Bus_USB)? 2 : 1,
                                            SAW_INIT_BANDWIDTH,
                                            pIt9135Info->it9130Demodulator.streamType,
                                            pIt9135Info->it9130Demodulator.architecture);
            if( result != Error_NO_ERROR )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod init is failed: reason - 0x%X\n", result);
                break;
            }
            else
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod init is successfully!!\n");

          #if 1
            // ------------------------------------------
            // enable suspend
            it9135_result = Demodulator_writeRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                      0, Processor_LINK, p_reg_top_pwrdw_hwen, 1);
            if( it9135_result )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                //break;
            }

            it9135_result = Demodulator_writeRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                      0, Processor_LINK, p_reg_top_pwrdw, 1);
            if( it9135_result )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                //break;
            }
          #endif

            memset(pIt9135Info->filterTable, 0xFF, sizeof(pIt9135Info->filterTable));
            pIt9135Info->totalPmtCount = 0;
            dev->privData = (void*)pIt9135Info;

        }while(0);
    }
    _DEMODCTRL_unLock(&dev->demod_attr);

    if( result )
    {
        if( pIt9135Info )       free(pIt9135Info);
        dev->privData = 0;
    }
}

//=============================================================================
/**
 * Used to Terminate the inited front end demodulator and turner.
 * @return none.
 */
//=============================================================================
static void
Demod_9135_Terminate(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    Dword               result = Error_NO_ERROR;
    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        do{
            if( pIt9135Info->bEnableSuspend == true )
            {
                dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                break;
            }

            result = Demodulator_finalize((Demodulator*)&(pIt9135Info->it9130Demodulator));
            if( result != Error_NO_ERROR )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod reset is failed: reason - %d\n", result);
            }
        }while(0);

        _DEMODCTRL_unLock(&dev->demod_attr);

        if( pIt9135Info->shareBuffer )   free((void*)pIt9135Info->shareBuffer);

        free(pIt9135Info);
        dev->privData = 0;
    }

    User_deleteCriticalSection();
}

//=============================================================================
/**
 * By specifying the frequency and bandwith to detech if any channel locked or
 * existed through the setting parameters.
 * @param demodId   control a specific demod and tuner.
 * @param frequency The desired checking frequency by unit of kHz.
 * @param bandwith  The bandwith of the specific frequency channel. Usually,
 *                  the bandwith is diverse by different countries.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
static uint32_t
Demod_9135_AcquireChannel(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    uint32_t            result = 0;
    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        do{
            if( pIt9135Info->bEnableSuspend == true )
            {
                dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                break;
            }

            result = Demodulator_acquireChannel((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                0,
                                                (Word)dev->bandwith, (Dword)dev->frequency);
        }while(0);

        _DEMODCTRL_unLock(&dev->demod_attr);
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }

    return result;
}

//=============================================================================
/**
 * Check if the demod is locked after channel acquisition step.
 * @param demodId   control a specific demod and tuner.
 * @return bool If the demod is locked then return true, false
 *                  otherwise.
 */
//=============================================================================
static bool
Demod_9135_IsChannelLock(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    bool                result = false;
    Bool                bLock = False;
    DEMOD_IT9135_INFO   *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        do{
            if( pIt9135Info->bEnableSuspend == true )
            {
                dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                break;
            }

            Demodulator_isLocked((Demodulator*)&(pIt9135Info->it9130Demodulator), 0,
                                 &bLock);

        }while(0);

        _DEMODCTRL_unLock(&dev->demod_attr);

        result = (bLock) ? true : false;
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }

    return result;
}

//=============================================================================
/**
 * Read TS stream Datagram from USB interface
 * @param demodId   control a specific demod and tuner.
 * @return MMP_BOOL If got Data then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
static uint32_t
Demod_9135_Usb_ReadDataStream(
    DEMOD_DEV   *dev,
    uint8_t*     buffer,
    uint32_t     bufferlength,
    void*        pfCallback)
{
    uint32_t result = 0;
    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        // need to check suspend ??
        // To Do:

        if (pfCallback)
        {
            pIt9135Info->usbCbCtxt.pfSelfCallback = _it9135_Usb_ReadCallback;
            pIt9135Info->pfUpperCallback          = pfCallback;
            pIt9135Info->pCallbackData            = dev;

            result = User_busRx_Data((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                      (Dword) bufferlength,
                                      (Byte*) buffer,
                                      &pIt9135Info->usbCbCtxt);
        }
        else
        {
            pIt9135Info->usbCbCtxt.pfSelfCallback = 0;
            pIt9135Info->pfUpperCallback          = 0;
            pIt9135Info->pCallbackData            = 0;
            result = User_busRx_Data((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                      (Dword) bufferlength,
                                      (Byte*) buffer,
                                      0);
        }
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }

    return result;
}
//=============================================================================
/**
 * Retrieve current signal statistic from demodulator.
 * @param demodId           control a specific demod and tuner.
 * @param ptDemodStatistic  The output of current signal status if return
 *                          is true.
 * @return Whether the operation is success or not.
 */
//=============================================================================
static bool
Demod_9135_GetSignalStatus(
    DEMOD_DEV               *dev,
    DEMOD_SIGNAL_STATISTIC  *ptDemodStatistic,
    void                    *extraData)
{
    bool     bResult = true;
// #define GET_INFO_BY_STATISTIC
    uint8_t  temp = 0;
    Bool     bLock = False;
    #ifndef GET_INFO_BY_STATISTIC
        Byte    strengh = 0;
        Byte    quality = 0;
    #else
        Statistic tStatistic = {0};
    #endif

    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( !ptDemodStatistic || !pIt9135Info || pIt9135Info->bEnableSuspend )     return false;

    _DEMODCTRL_Lock(&dev->demod_attr);

#ifndef GET_INFO_BY_STATISTIC
    if( Error_NO_ERROR !=
        Demodulator_getSignalStrength((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                       0, &strengh) )
    {
        bResult = false;
        goto exit;
    }

    if( Error_NO_ERROR !=
        Demodulator_getSignalQuality((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                      0, &quality) )
    {
        bResult = false;
        goto exit;
    }

    if( Error_NO_ERROR !=
        Standard_isMpeg2Locked((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &bLock) )
    {
        bResult = false;
        goto exit;
    }

    ptDemodStatistic->signalStrength = (uint32_t)strengh;
    ptDemodStatistic->signalQuality  = (uint32_t)quality;
    ptDemodStatistic->bMpg2Lock      = (bool)bLock;

#else
    if( Error_NO_ERROR !=
        Demodulator_getStatistic((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &tStatistic) )
    {
        bResult = false;
        goto exit;
    }

    if( Error_NO_ERROR !=
        Standard_isMpeg2Locked((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &bLock) )
    {
        bResult = false;
        goto exit;
    }

    ptDemodStatistic->signalStrength = (uint32_t)tStatistic.signalStrength;
    ptDemodStatistic->signalQuality  = (uint32_t)tStatistic.signalQuality;
    ptDemodStatistic->bMpg2Lock      = (bool)bLock;
#endif

    //------------------------------
    // Get Hierarcical TX flag
    if( Error_NO_ERROR !=
        Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                 0, Processor_OFDM, 0xF4C6, &temp) )
    {
        bResult = false;
        goto exit;
    }

    dev->bHierarchicalTx = (bool)temp;
    if( temp )      dem_msg(1, "hierarchical mode: 0x%x\n", temp);

exit:
    _DEMODCTRL_unLock(&dev->demod_attr);

    return bResult;
}

//=============================================================================
/**
 * Update the PID filter table by specified PID. For a new PID just inserts it
 * into the PID filter table, update the table otherwise.
 * @param demodId   control a specific demod and tuner.
 * @param pid       The desired filtering PID.
 * @param pidType   The desired PID Type.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
static uint32_t
Demod_9135_UpdatePidTable(
    DEMOD_DEV         *dev,
    uint32_t          pid,
    PID_FILTER_INDEX  pidIndex,
    void              *extraData)
{
    Dword       result = Error_NO_ERROR;
    bool        bInsert = true, bUpdate = true;
    uint32_t    i = 0;
    Pid         tPid = {0};
    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        // software filter instead of hardware filter.
        if( false == pIt9135Info->bFilterOn )    return result;

        i = pidIndex;
        // Only the PMT table owns more than one index.
        if( PID_PMT_INDEX == pidIndex )
        {
#if 1
            i = PID_PMT_INDEX;
#else
            for(i; i < (PID_PMT_INDEX + pIt9135Info->totalPmtCount); i++)
            {
                if( pid == pIt9135Info->filterTable[i] )
                {
                    bInsert = false;
                    break;
                }
            }

            if( bInsert )
            {
                if( pIt9135Info->totalPmtCount < DEMOD_MAX_PMT_FILTER_COUNT )
                    pIt9135Info->totalPmtCount++;
                else
                {
                    dem_msg(DEM_MSG_TYPE_ERR, "no more available PMT pid filter\n");
                    bUpdate = false;
                }
            }
#endif
        }

        // If the pid is added to table already, ignore it.
        if( (pid != pIt9135Info->filterTable[i]) && bUpdate )
        {
            tPid.value = (Word)pid;
            _DEMODCTRL_Lock(&dev->demod_attr);

            do{
                if( pIt9135Info->bEnableSuspend == true )
                {
                    dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                    break;
                }

                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, (uint8_t)i, tPid);
                dem_msg(1, "demodulator (%d-th), add pid result: %d, index: %2d, pid: 0x%X (%d)\n",
                        dev->demodId, result, i, pid, pid);

            }while(0);

            _DEMODCTRL_unLock(&dev->demod_attr);
        }

        if( result == Error_NO_ERROR )      pIt9135Info->filterTable[i] = pid;
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }


    return result;
}

//=============================================================================
/**
 * Reset the PID filter table
 * @param demodId   control a specific demod and tuner.
 * @param  bFilterOn    The desired filtering PID.
 * @return uint32_t   the error code from demod control API if any.
 */
//=============================================================================
static uint32_t
Demod_9135_ResetPidTable(
    DEMOD_DEV   *dev,
    bool        bFilterOn,
    void        *extraData)
{
    Dword               result = Error_NO_ERROR;
    Pid                 tPid = {0};
    DEMOD_IT9135_INFO   *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        do{
            if( pIt9135Info->bEnableSuspend == true )
            {
                dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                break;
            }

            if( bFilterOn )
            {
                result = Demodulator_resetPidFilter((Demodulator*)&(pIt9135Info->it9130Demodulator), 0);

                result = Demodulator_controlPidFilter((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, 1);
                //dem_msg(1, "hardware demod PID filter is ON\n");

                tPid.value = 0x0; // PAT
                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                    (Byte)0, (uint8_t)PID_PAT_INDEX, tPid);

                tPid.value = 0x10; // NIT
                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                    (Byte)0, (uint8_t)PID_NIT_INDEX, tPid);
                tPid.value = 0x11; // SDT
                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                    (Byte)0, (uint8_t)PID_SDT_INDEX, tPid);
                tPid.value = 0x12; // EIT
                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                    (Byte)0, (uint8_t)PID_EIT_INDEX, tPid);
                tPid.value = 0x14; // TOT_TDT
                result = Demodulator_addPidToFilter((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                                    (Byte)0, (uint8_t)PID_TOT_TDT_INDEX, tPid);
            }
            else
            {
                result = Demodulator_controlPidFilter((Demodulator*) &(pIt9135Info->it9130Demodulator), 0, 0);
                //dem_msg(1, "hardware demod PID filter is OFF\n");
            }

            pIt9135Info->totalPmtCount = 0;
            memset((void*)pIt9135Info->filterTable, 0xFF, sizeof(pIt9135Info->filterTable));
            pIt9135Info->bFilterOn = bFilterOn;

        }while(0);

        _DEMODCTRL_unLock(&dev->demod_attr);

    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }

    return result;
}

//=============================================================================
/**
 * To setup whether the demond enter the suspend state or not.
 * @param demodId           control a specific demod and tuner.
 * @param bEnableSuspend    if true, the demod will enter the suspend state.
 * @return bool         Whether the mode change success.
 */
//=============================================================================
static bool
Demod_9135_SetSuspend(
    DEMOD_DEV   *dev,
    bool        bSuspend,
    void        *extraData)
{
    uint32_t            result = 0;
    uint32_t            it9135_result = Error_NO_ERROR;
    DEMOD_IT9135_INFO   *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;
    uint8_t             control = 0;        // 1: Power up, 0: Power down

#if defined(CFG_GPIO_DEMOD_SUSPEND) && (CFG_GPIO_DEMOD_SUSPEND >= 0)

    if( pIt9135Info )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        pIt9135Info->bEnableSuspend = bSuspend;

        switch( pIt9135Info->bEnableSuspend )
        {
            case true:  // set suspend
              #if 0
                //----------------------------
                // power off by setting GPIO
                it9135_result = Demodulator_writeRegister((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                          0, Processor_LINK, p_reg_top_pwrdw_hwen, 1);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }

                it9135_result = Demodulator_writeRegister((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                          0, Processor_LINK, p_reg_top_pwrdw, 1);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }
              #endif

                control = !(pIt9135Info->bEnableSuspend); // 1: Power up, 0: Power down
                it9135_result = Demodulator_controlPowerSaving((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                                0, control);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }
                break;

            case false: // wake up
                control = !(pIt9135Info->bEnableSuspend); // 1: Power up, 0: Power down
                it9135_result = Demodulator_controlPowerSaving((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                               0, control);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }

              #if 0
                it9135_result = Demodulator_writeRegister((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                          0, Processor_LINK, p_reg_top_pwrdw_hwen, 0);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }

                it9135_result = Demodulator_writeRegister((Demodulator*) &(pIt9135Info->it9130Demodulator),
                                                          0, Processor_LINK, p_reg_top_pwrdw, 0);
                if( it9135_result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, " Demod set suspend fail !!\n");
                    break;
                }
              #endif

                usleep(50000);
                break;
        }

        memset(pIt9135Info->filterTable, 0xFF, sizeof(pIt9135Info->filterTable));
        pIt9135Info->totalPmtCount = 0;

        _DEMODCTRL_unLock(&dev->demod_attr);
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
        result = Error_NULL_PTR;
    }
#endif

    return result;
}

//=============================================================================
/**
 * Get Demodulator FW Version
 * @param demodId   control a specific demod and tuner.
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
static uint32_t
Demod_9135_GetDemodFwVersion(
    DEMOD_DEV  *dev,
    void       *extraData)
{
#if 0
    uint32_t  version = 0;

    if( 1 == dev->tsiId )
    {
        return (0x09090900);
    }
    else
    {
        return (version);
    }
#endif
}

//=============================================================================
/**
 * Enable DEMOD Retrain
 * @param demodId   control a specific demod and tuner.
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
static uint32_t
Demod_9135_EnableRetrain(
    DEMOD_DEV  *dev,
    void       *extraData)
{
    uint32_t  error = 0;

    return (error);
}

//=============================================================================
/**
 * Disable DEMOD Retrain
 * @param demodId   control a specific demod and tuner.
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
static uint32_t
Demod_9135_DisableRetrain(
    DEMOD_DEV  *dev,
    void       *extraData)
{
    uint32_t  error = 0;

    return (error);
}

//=============================================================================
/**
 * Used to get demodulator information.
 * @param demodId   control a specific demod and tuner.
 * @return none.
 */
//=============================================================================
static void
Demod_9135_GetBer(
    DEMOD_DEV  *dev,
    uint32_t   *pErrorCount,
    uint32_t   *pBitCount,
    uint16_t   *pAbortCount,
    uint8_t    *extraData)
{
    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

#if 0
    if( pIt9135Info && dev->tsiId == 0 )
    {
        _DEMODCTRL_Lock(&dev->demod_attr);

        Demodulator_getPostVitBer((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                  pErrorCount, pBitCount, pAbortCount);

        Demodulator_readRegisters((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                  Processor_OFDM, 0xA4, 4, extraData);

        Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                 Processor_OFDM, 0x011c, extraData+4);

        _DEMODCTRL_unLock(&dev->demod_attr);
    }
#endif
}

//=============================================================================
/**
 * Used to get demodulator information.
 * @param demodId   control a specific demod and tuner.
 * @return none.
 */
//=============================================================================
static void
Demod_9135_GetInfo(
    DEMOD_DEV  *dev,
    DEMOD_INFO *ptDemodInfo,
    void       *extraData)
{
    uint8_t   temp = 0, temp1 = 0;
    uint32_t  temp_postErrorCount = 0;
    uint32_t  temp_postBitCount = 0;
    uint32_t  preErrorCount = 0;
    uint32_t  preBitCount = 0;
    uint16_t  temp_abortCount = 0;
    ChannelModulation   channelModulation;
    Bool      Locked;
    double    ElementaryPeriod;
    uint32_t  NormalizedOffset;
    uint32_t  Offset = 0;
    double    AdcFrequency = 0.0;
    double    offset, offsetPpm;
    uint16_t  api_version_number;
    uint32_t  api_version_date;
    uint8_t   api_version_build;
    double    snr;
    int32_t   tempLong;
    uint32_t  fwVersion;

    DEMOD_IT9135_INFO    *pIt9135Info = (DEMOD_IT9135_INFO*)dev->privData;

    if( pIt9135Info )
    {
        do{
            _DEMODCTRL_Lock(&dev->demod_attr);

            if( pIt9135Info->bEnableSuspend == true )
            {
                dem_msg_ex(DEM_9135_MSG_INFO, "Suspend mode => skip handle\n");
                break;
            }

            // 1. Get Post_VTB_BER
            // 2. Get RSD_abort_count

            Demodulator_getPostVitBer((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                      0,
                                      &temp_postErrorCount,   /** 24 bits */
                                      &temp_postBitCount,     /** 16 bits */
                                      &temp_abortCount);

            ptDemodInfo->Post_VTB_BER   = ((double)temp_postErrorCount / (double)temp_postBitCount);
            ptDemodInfo->postErrorCount = temp_postErrorCount;
            ptDemodInfo->postBitCount   = temp_postBitCount;
            ptDemodInfo->RSD_abort_count = temp_abortCount;

            // 3. Get Soft Bit Quality (%)

            // 4. Get Mobile ch
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, p_reg_aagc_mobile_sel,
                                         reg_aagc_mobile_sel_pos, reg_aagc_mobile_sel_len,
                                         &temp);
            ptDemodInfo->MCH = temp;

            // 4.5 Get Channel No
            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, ChannelNo, &temp);
            ptDemodInfo->CHNo = temp;

            // 5. Get FDI Type
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, r_reg_ce_var_hw,
                                         reg_ce_var_hw_pos, reg_ce_var_hw_len,
                                         &temp);
            ptDemodInfo->IDF_Type = temp;

            // 6. Get FDI Forced Enable
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, p_reg_ce_var_forced_en,
                                         reg_ce_var_forced_en_pos, reg_ce_var_forced_en_len,
                                         &temp);
            ptDemodInfo->IDF_Forced_Enable = temp;

            // 7. Get FDI Forced Type
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, p_reg_ce_var_forced_value,
                                         reg_ce_var_forced_value_pos, reg_ce_var_forced_value_len,
                                         &temp);
            ptDemodInfo->IDF_Forced_Type = temp;

            // 8. Get AAGC Signal Level
            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, signal_level, &temp);
            ptDemodInfo->AAGC_Signal_Level = temp;


            //9. Get Near ACI 0
            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, aci_0, &temp);
            ptDemodInfo->N_ICA_0 = temp;

            // 10. Get Near ACI 1
            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, aci_1, &temp);
            ptDemodInfo->N_ICA_1 = temp;

            // 10.5 Get Near ACI 2
            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, aci_2, &temp);
            ptDemodInfo->N_ICA_2 = temp;

            // 11. Get CCI0
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, p_reg_mccid_ccif0_state,
                                         reg_mccid_ccif0_state_pos, reg_mccid_ccif0_state_len,
                                         &temp);
            ptDemodInfo->ICC_0 = temp;

            // 12. Get CCI1
            Demodulator_readRegisterBits((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                         0, Processor_OFDM, p_reg_mccid_ccif1_state,
                                         reg_mccid_ccif1_state_pos, reg_mccid_ccif1_state_len,
                                         &temp);
            ptDemodInfo->ICC_1 = temp;

            // 13. Get Signal Quality
            Demodulator_getSignalQualityIndication((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &temp);
            ptDemodInfo->Signal_Quality = temp;

            // 14. Get Signal Strength
            //Demodulator_getSignalStrength((Demodulator*)&(pIt9135Info->it9130Demodulator), &temp);
            //ptDemodInfo->Signal_Strength = temp;

            // 14.5 Get Signal Strength (dbm)
            Demodulator_getSignalStrengthDbm((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &tempLong);
            ptDemodInfo->Signal_Strength = tempLong;

            // 15. Get More OFDM Information
            /* 15.1 Constellation      : Signal uses QPSK/16QAM/64QAM constellation
               15.2 Frequency         : Channel frequency in KHz.
               15.3 Bandwidth         : Signal bandwidth is 6MHz/7MHz/8MHz/5MHz
               15.4 HighCodeRate    : Signal uses FEC coding ratio of 1/2, 2/3, 3/4, 5/6, 7/8
               15.5 GuardInterval    : Guard interval is 1/32 or 1/16 or 1/8 or 1/4  of symbol length
            */
            Demodulator_getChannelModulation((Demodulator*)&(pIt9135Info->it9130Demodulator), 0, &channelModulation);

            ptDemodInfo->Constellation = channelModulation.constellation;
            ptDemodInfo->Frequency     = channelModulation.frequency;
            ptDemodInfo->Bandwidth     = channelModulation.bandwidth;
            ptDemodInfo->HighCodeRate  = channelModulation.highCodeRate;
            ptDemodInfo->GuardInterval = channelModulation.interval;

            // 16. Lock status : TPS Locked
            //Demodulator_isTpsLocked((Demodulator*) &(pIt9135Info->it9130Demodulator), &Locked);
            //ptDemodInfo->TpsLocked = Locked;

            // 17. Lock status : MPEG2 Locked
            //Demodulator_isMpeg2Locked ( (Demodulator*) &(pIt9135Info->it9130Demodulator), &Locked);
            //ptDemodInfo->Mpeg2Locked = Locked;

            // 18. Freq Offset
            // 19. Clock Offset
            switch (ptDemodInfo->Bandwidth)
            {
                case 0:   ElementaryPeriod = (7.0/48.0);  break;  // Bandwidth = 6M
                case 1:   ElementaryPeriod = (1.0/8.0);   break;  // Bandwidth = 7M
                case 2:   ElementaryPeriod = (7.0/64.0);  break;  // Bandwidth = 8M
                case 3:   ElementaryPeriod = (7.0/40.0);  break;  // Bandwidth = 5M
                default:  break;
            }

            //Demodulator_getFrequencyOffset((Demodulator*)&(pIt9135Info->it9130Demodulator),
            //                               ElementaryPeriod, &NormalizedOffset,
            //                               &Offset);
            //ptDemodInfo->FrequencyOffset = Offset;

            //Demodulator_getTimeOffset((Demodulator*)&(pIt9135Info->it9130Demodulator),
            //                          0, AdcFrequency, ElementaryPeriod,
            //                          &offset, &offsetPpm);
            //ptDemodInfo->TimeOffset = offsetPpm;

            //Demodulator_getPreVitBer((Demodulator*)&(pIt9135Info->it9130Demodulator),
            //                         0,
            //                         &preErrorCount, &preBitCount, &snr);
            //ptDemodInfo->snr = snr;


            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, r_reg_r_rssi_ori_7_0,
                                     &temp);

            Demodulator_readRegister((Demodulator*)&(pIt9135Info->it9130Demodulator),
                                     0, Processor_OFDM, r_reg_r_rssi_ori_9_8,
                                     &temp1);
            ptDemodInfo->rssi = (uint16_t)((temp1 << 8) + temp);

            ptDemodInfo->IsHierarchical = (channelModulation.hierarchy) ? 1 : 0;

            //Demodulator_getFWVersion((Demodulator*)&(pIt9135Info->it9130Demodulator), &fwVersion);
            //sprintf(ptDemodInfo->DemodFwVersion, "%08x", fwVersion);

            //Demodulator_getOFDMFwVersion((Demodulator*)&(pIt9135Info->it9130Demodulator), (char*)&(ptDemodInfo->DemodOFDMFwVersion));
            //Demodulator_getLINKFwVersion((Demodulator*)&(pIt9135Info->it9130Demodulator), (char*)&(ptDemodInfo->DemodLINKFwVersion));
            //Demodulator_getTunerID((Demodulator*)&(pIt9135Info->it9130Demodulator), (char*)&(ptDemodInfo->DemodTunerID));
        }while(0);

        _DEMODCTRL_unLock(&dev->demod_attr);

        api_version_number = Version_NUMBER;
        api_version_date   = Version_DATE;
        api_version_build  = Version_BUILD;
        sprintf((char*) &(ptDemodInfo->DemodApiVersion), "%04x.%08x.%02x", api_version_number, api_version_date, api_version_build);
    }
    else
    {
        dem_msg_ex(DEM_MSG_TYPE_ERR, "Null Pointer !!\n");
    }

    return;
}

//=============================================================================
/**
 * To Power Down the demond.
 * note iic address is 0x20
 */
//=============================================================================
static uint32_t
Demod_9135_SetPowerDown(
    DEMOD_DEV   *dev,
    bool        bEnable,
    void        *extraData)
{
    uint32_t    result = 0;
#if defined(CFG_GPIO_DEMOD_SUSPEND) && (CFG_GPIO_DEMOD_SUSPEND >= 0)

    static int  bCurStat = -1;  // two chips use one GPIO
    int         bPullHigth = !!bEnable;
    if( bCurStat != bPullHigth )
    {
        if( bPullHigth )
        {
            ithWriteRegMaskA(REG_BASE_GPIO_SEL_MODE,
                             ((0x1 << GPIO_SEL_MODE_SHIFT_BITS) | (0x1 << (GPIO_SEL_MODE_SHIFT_BITS+1))),
                             ((0x1 << GPIO_SEL_MODE_SHIFT_BITS) | (0x1 << (GPIO_SEL_MODE_SHIFT_BITS+1))));
            ithWriteRegMaskA(REG_GPIO_DATA_OUT, (0x1 << GPIO_PIN_SEL_SHIFT_BITS), (0x1 << GPIO_PIN_SEL_SHIFT_BITS));
            ithWriteRegMaskA(REG_GPIO_PIN_DIRECTION, (0x1 << GPIO_PIN_SEL_SHIFT_BITS), (0x1 << GPIO_PIN_SEL_SHIFT_BITS));
            printf("\t 1.reg[0x%x]=0x%x\n", REG_BASE_GPIO_SEL_MODE, ithReadRegA(REG_BASE_GPIO_SEL_MODE));
            printf("\t 1.reg[0x%x]=0x%x\n", REG_GPIO_DATA_OUT, ithReadRegA(REG_GPIO_DATA_OUT));
            printf("\t 1.reg[0x%x]=0x%x\n", REG_GPIO_PIN_DIRECTION, ithReadRegA(REG_GPIO_PIN_DIRECTION));
        }
        else
        {
            ithWriteRegMaskA(REG_BASE_GPIO_SEL_MODE,
                             ((0x0 << GPIO_SEL_MODE_SHIFT_BITS) | (0x0 << (GPIO_SEL_MODE_SHIFT_BITS+1))),
                             ((0x1 << GPIO_SEL_MODE_SHIFT_BITS) | (0x1 << (GPIO_SEL_MODE_SHIFT_BITS+1))));
            ithWriteRegMaskA(REG_GPIO_DATA_OUT, (0x0 << GPIO_PIN_SEL_SHIFT_BITS), (0x1 << GPIO_PIN_SEL_SHIFT_BITS));
            ithWriteRegMaskA(REG_GPIO_PIN_DIRECTION, (0x0 << GPIO_PIN_SEL_SHIFT_BITS), (0x1 << GPIO_PIN_SEL_SHIFT_BITS));
            usleep(50000);
        }
        bCurStat = bPullHigth;
    }
#endif
    return result;
}

static uint32_t
Demod_9135_Control(
    DEMOD_DEV       *dev,
    DEMOD_CTRL_CMD  cmd,
    uint32_t        *value,
    void            *extraData)
{
    uint32_t result = 0;

    switch( cmd )
    {
        default:        break;
    }
    return result;
}


DEMOD_DESC  DEMOD_DESC_it9135_desc =
{
    "demod it9135",         // char        *name;
    0,                      // struct _DEMOD_DESC_TAG  *next;
    DEMOD_ID_IT9135,        // DEMOD_TYPE_ID           id;
    0,                      // void        *privInfo;
    Demod_9135_Init,             // void (*DemodCtrl_Init)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_Terminate,        // void (*DemodCtrl_Terminate)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_AcquireChannel,   // uint32_t (*DemodCtrl_AcquireChannel)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_IsChannelLock,    // bool (*DemodCtrl_IsChannelLock)(DEMOD_DEV *dev, void *extraData);
    0,                      // bool (*DemodCtrl_IsTpsLock)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    0,                      // bool (*DemodCtrl_IsMpeg2Lock)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_9135_GetSignalStatus,  // bool (*DemodCtrl_GetSignalStatus)(DEMOD_DEV *dev,uint32_t  demodId,DEMOD_SIGNAL_STATISTIC *ptDemodStatistic,void      *extraData);
    Demod_9135_Usb_ReadDataStream,
    Demod_9135_UpdatePidTable,   // uint32_t (*DemodCtrl_UpdatePidTable)(DEMOD_DEV  *dev, uint32_t   demodId, uint32_t   pid, PID_FILTER_INDEX  pidIndex,void   *extraData);
    Demod_9135_ResetPidTable,    // uint32_t (*DemodCtrl_ResetPidTable)(DEMOD_DEV   *dev, uint32_t    demodId, bool        bFilterOn, void        *extraData);
    Demod_9135_SetSuspend,       // bool (*DemodCtrl_SetSuspend)(DEMOD_DEV *dev, uint32_t demodId, bool bEnableSuspend, void *extraData);
    0,                      // bool (*DemodCtrl_OpenTuner)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    0,                      // bool (*DemodCtrl_CloseTuner)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_9135_GetDemodFwVersion, // uint32_t (*DemodCtrl_GetDemodFwVersion)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_9135_EnableRetrain,    // uint32_t (*DemodCtrl_EnableRetrain)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_DisableRetrain,   // uint32_t (*DemodCtrl_DisableRetrain)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_GetBer,           // void (*DemodCtrl_GetBer)(DEMOD_DEV *dev, uint32_t  demodId,uint32_t  *pErrorCount,uint32_t  *pBitCount,uint16_t  *pAbortCount,uint8_t   *pExtData,void      *extraData);
    Demod_9135_GetInfo,          // void (*DemodCtrl_GetInfo)(DEMOD_DEV *dev, uint32_T demodId, DEMOD_INFO* ptDemodInfo, void *extraData);
    Demod_9135_SetPowerDown,     // int (*DemodCtrl_SetPowerDown)(DEMOD_DEV *dev, void *extraData);
    Demod_9135_Control,
};

#else

DEMOD_DESC  DEMOD_DESC_it9135_desc =
{
    "demod it9135",             // char        *name;
    0,                          // struct _DEMOD_DESC_TAG  *next;
    DEMOD_ID_IT9135,            // DEMOD_TYPE_ID           id;
};

#endif

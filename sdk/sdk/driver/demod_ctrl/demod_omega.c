
#include <stdio.h>
#include <string.h>
#include "demod.h"

#if CONFIG_DEMOD_DESC_OMEGA_DESC

#include "./IT9133.h"
#include "./user.h"
#include "iic/mmp_iic.h"
#include "./tuner.h"
#include "./standard.h"
#include "ite/itp.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#ifdef CORE_DCA
    #define TOTAL_DEMOD_CHIP_NUMBER         (2) // diversity
#else
    #define TOTAL_DEMOD_CHIP_NUMBER         (1) // single
#endif
#define ACTIVE_DEMOD_CHIP_ID            (0)
//#define SAW_INIT_BANDWIDTH              (8000)

#if !defined(CFG_AGGRE_ENABLE) && defined(CFG_TSI_PARALLEL_MODE) 
    #define DEMOD_INIT_STREAM_TYPE          (StreamType_DVBT_PARALLEL)
#else
    #define DEMOD_INIT_STREAM_TYPE          (StreamType_DVBT_SERIAL)
#endif
//#define DEMOD_INIT_ARCHITECTURE         (Architecture_DCA)



#define PAL_HEAP_DEFAULT        0
#define GPIO_PADSEL_REG         0x90

// AMBA
#define AHB_ReadRegister(add, data)                 (*data = ithReadRegA(add))
#define AHB_WriteRegister(add, data)                ithWriteRegA(add, data)
#define AHB_WriteRegisterMask(add, data, mark)      ithWriteRegMaskA(add, data, mark)



#define OMEGA_9133_INDEX_0_ADDR     0x38
#define OMEGA_9133_INDEX_1_ADDR     0x3C

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct DEMOD_OMEGA_INFO_TAG
{
    DefaultDemodulator    tGanymede;
    bool                  bEnableSuspend;
    bool                  bFilterOn;
    uint32_t              totalPmtCount;
    uint32_t              filterTable[DEMOD_TOTAL_FILTER_COUNT];
    uint32_t              shareBuffer;
} DEMOD_OMEGA_INFO;

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
_DEMODCTRL_LockIic(
    uint32_t    demodId)
{
    // Switch PAD_Sel Bit 5 to switch IIC mode to Keyboard mode to prevent
    // internal IIC signal spill out.
    mmpIicLockModule(IIC_PORT_0);
    return;
}

//=============================================================================
/**
 * Release I2C module for other module usage
 * return none.
 */
//=============================================================================
static void
_DEMODCTRL_ReleaseIic(
    void)
{
    mmpIicReleaseModule(IIC_PORT_0);
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
Demod_Init(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    uint32_t            result, omega_result = 0;
    DEMOD_OMEGA_INFO    *pOmegaInfo = 0;
    uint8_t             supportType = (uint8_t)(-1);

    _DEMODCTRL_LockIic(dev->demodId);

    if( 0 == dev->privData )
    {
        do{
            // create private info
            pOmegaInfo = (DEMOD_OMEGA_INFO*)dem_malloc(sizeof(DEMOD_OMEGA_INFO));
            if( !pOmegaInfo )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Memory allocation is failed for Demod omega info\n");
                break;
            }
            memset((void*)pOmegaInfo, 0x0, sizeof(DEMOD_OMEGA_INFO));
            dev->privData = (void*)pOmegaInfo;

            switch( dev->demodId )
            {
                default:
                case 0: pOmegaInfo->tGanymede.demodAddr = OMEGA_9133_INDEX_0_ADDR; break;
                case 1: pOmegaInfo->tGanymede.demodAddr = OMEGA_9133_INDEX_1_ADDR; break;
            }

            switch( dev->supportType )
            {
                case 0: supportType = OMEGA_NORMAL;       break;
                case 1: supportType = OMEGA_LNA_Config_1; break;
                case 2: supportType = OMEGA_LNA_Config_2; break;
            }
            pOmegaInfo->tGanymede.streamType = DEMOD_INIT_STREAM_TYPE;

printf("\n\ndemodAddr=0x%x, supportType=%d, streamType=%d, ",
pOmegaInfo->tGanymede.demodAddr, supportType, pOmegaInfo->tGanymede.streamType);
    switch( pOmegaInfo->tGanymede.streamType )
    {
        case StreamType_DVBT_SERIAL:    dem_msg(1, "interface = serial\n"); break;
        case StreamType_DVBT_PARALLEL:  dem_msg(1, "interface = parallel\n"); break;
        case StreamType_DVBT_DATAGRAM:  dem_msg(1, "interface = datagram\n"); break;
        default:    dem_msg(1, "\n"); break;
    }

            User_createCriticalSection();

            omega_result = OMEGA_supportLNA((Demodulator*)&(pOmegaInfo->tGanymede), supportType);
            if( omega_result )  dem_msg_ex(DEM_MSG_TYPE_ERR,"[OMEGA_INIT] ERROR = %x \n", omega_result);

            result = Demodulator_initialize((Demodulator*)&(pOmegaInfo->tGanymede), pOmegaInfo->tGanymede.streamType); //DEMOD_INIT_STREAM_TYPE);

            if( result != Error_NO_ERROR )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod init is failed: reason - 0x%X\n", result);
                break;
            }
/*
            result = Demodulator_setMultiplier((Demodulator*)&(pOmegaInfo->tGanymede), Multiplier_2X); //ste ADCx2
            if( result != Error_NO_ERROR )
            {
                dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod set Multiplier failed: reason - 0x%X\n", result);
                break;
            }
//*/
            memset(pOmegaInfo->filterTable, 0xFF, sizeof(pOmegaInfo->filterTable));
            pOmegaInfo->totalPmtCount = 0;

        }while(0);
    }

    _DEMODCTRL_ReleaseIic();
}

//=============================================================================
/**
 * Used to Terminate the inited front end demodulator and turner.
 * @return none.
 */
//=============================================================================
static void
Demod_Terminate(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    Dword               result = Error_NO_ERROR;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        _DEMODCTRL_LockIic(dev->demodId);

        result = Demodulator_finalize((Demodulator*)&(pOmegaInfo->tGanymede));

        _DEMODCTRL_ReleaseIic();

        if( result != Error_NO_ERROR )
        {
            dem_msg_ex(DEM_MSG_TYPE_ERR, "Demod reset is failed: reason - %d\n", result);
        }

        if( pOmegaInfo->shareBuffer )   free((void*)pOmegaInfo->shareBuffer);

        free(pOmegaInfo);
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
Demod_AcquireChannel(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    uint32_t            result = 0;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        _DEMODCTRL_LockIic(dev->demodId);

        result = Demodulator_acquireChannel((Demodulator*)&(pOmegaInfo->tGanymede),
                                            (Word)dev->bandwith,
                                            (Dword)dev->frequency);
        _DEMODCTRL_ReleaseIic();
    }
    else
        result = Error_NULL_PTR;

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
Demod_IsChannelLock(
    DEMOD_DEV   *dev,
    void        *extraData)
{
    bool                result = false;
    Bool                bLock = False;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        _DEMODCTRL_LockIic(dev->demodId);

        Demodulator_isLocked((Demodulator*)&(pOmegaInfo->tGanymede), &bLock);

        _DEMODCTRL_ReleaseIic();

        result = (bLock) ? true : false;
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
Demod_GetSignalStatus(
    DEMOD_DEV               *dev,
    DEMOD_SIGNAL_STATISTIC  *ptDemodStatistic,
    void                    *extraData)
{
#define GET_INFO_BY_STATISTIC

    bool     bResult = true;
    uint8_t  temp = 0;
    Bool     bLock = False;
    #ifndef GET_INFO_BY_STATISTIC
        BYTE    strengh = 0;
        BYTE    uality = 0;
    #else
        Statistic tStatistic = {0};
    #endif

    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( !ptDemodStatistic || !pOmegaInfo )     return false;

    _DEMODCTRL_LockIic(dev->demodId);

#ifndef GET_INFO_BY_STATISTIC
    if( Error_NO_ERROR !=
        Demodulator_getSignalStrength((Demodulator*)&(pOmegaInfo->tGanymede),
                                       dev->demodId,
                                       &strengh) )
    {
        bResult = false;
        goto exit;
    }

    if( Error_NO_ERROR !=
        Demodulator_getSignalQuality((Demodulator*)&(pOmegaInfo->tGanymede),
                                      dev->demodId,
                                      &quality) )
    {
        bResult = false;
        goto exit;
    }

    ptDemodStatistic->signalStrength = (uint32_t)strengh;
    ptDemodStatistic->signalQuality  = (uint32_t)quality;
    ptDemodStatistic->bMpg2Lock      = (bool)bLock;

#else
    if( Error_NO_ERROR !=
        Demodulator_getStatistic((Demodulator*)&(pOmegaInfo->tGanymede), &tStatistic) )
    {
        bResult = false;
        goto exit;
    }

    if( Error_NO_ERROR !=
        Standard_isMpeg2Locked((Demodulator*)&(pOmegaInfo->tGanymede), &bLock) )
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
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, 0xF4C6, &temp) )
    {
        bResult = false;
        goto exit;
    }

    dev->bHierarchicalTx = (bool)temp;
    if( temp )      dem_msg(1, "hierarchical mode: 0x%x\n", temp);

exit:
    _DEMODCTRL_ReleaseIic();

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
Demod_UpdatePidTable(
    DEMOD_DEV         *dev,
    uint32_t          pid,
    PID_FILTER_INDEX  pidIndex,
    void              *extraData)
{
    Dword       result = Error_NO_ERROR;
    bool        bInsert = true, bUpdate = true;
    uint32_t    i = 0;
    Word        tPid = 0;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        // software filter instead of hardware filter.
        if( false == pOmegaInfo->bFilterOn )    return result;

        i = pidIndex;
        // Only the PMT table owns more than one index.
        if( PID_PMT_INDEX == pidIndex )
        {
#if 1
            i = PID_PMT_INDEX;
#else
            for(i; i < (PID_PMT_INDEX + pOmegaInfo->totalPmtCount); i++)
            {
                if( pid == pOmegaInfo->filterTable[i] )
                {
                    bInsert = false;
                    break;
                }
            }

            if( bInsert )
            {
                if( pOmegaInfo->totalPmtCount < DEMOD_MAX_PMT_FILTER_COUNT )
                    pOmegaInfo->totalPmtCount++;
                else
                {
                    dem_msg(DEM_MSG_TYPE_ERR, "no more available PMT pid filter\n");
                    bUpdate = false;
                }
            }
#endif
        }

        // If the pid is added to table already, ignore it.
        if( (pid != pOmegaInfo->filterTable[i]) && bUpdate )
        {
            tPid = (Word) pid;
            _DEMODCTRL_LockIic(dev->demodId);

            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede), (uint8_t)i, tPid);

            _DEMODCTRL_ReleaseIic();

            dem_msg(1, "demodulator, add pid result: %d, index: %2d, pid: 0x%X (%d)\n",
                    result, i, pid, pid);
        }

        if( result == Error_NO_ERROR )      pOmegaInfo->filterTable[i] = pid;
    }
    else
        result = Error_NULL_PTR;

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
Demod_ResetPidTable(
    DEMOD_DEV   *dev,
    bool        bFilterOn,
    void        *extraData)
{
    Dword               result = Error_NO_ERROR;
    Word                tPid = 0;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        _DEMODCTRL_LockIic(dev->demodId);

        if( bFilterOn )
        {
            result = Demodulator_resetPidFilter((Demodulator*)&(pOmegaInfo->tGanymede));

            result = Demodulator_controlPidFilter((Demodulator*)&(pOmegaInfo->tGanymede), 1);
            //dem_msg(1, "hardware demod PID filter is ON\n");

            tPid = 0x0; // PAT
            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede),
                                                (uint8_t)PID_PAT_INDEX, tPid);

            tPid = 0x10; // NIT
            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede),
                                                (uint8_t)PID_NIT_INDEX, tPid);
            tPid = 0x11; // SDT
            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede),
                                                (uint8_t)PID_SDT_INDEX, tPid);
            tPid = 0x12; // EIT
            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede),
                                                (uint8_t)PID_EIT_INDEX, tPid);
            tPid = 0x14; // TOT_TDT
            result = Demodulator_addPidToFilter((Demodulator*)&(pOmegaInfo->tGanymede),
                                                (uint8_t)PID_TOT_TDT_INDEX, tPid);
        }
        else
        {
            result = Demodulator_controlPidFilter((Demodulator*) &(pOmegaInfo->tGanymede), 0);
            //dem_msg(1, "hardware demod PID filter is OFF\n");
        }

        _DEMODCTRL_ReleaseIic();

        pOmegaInfo->totalPmtCount = 0;
        memset((void*)pOmegaInfo->filterTable, 0xFF, sizeof(pOmegaInfo->filterTable));
        pOmegaInfo->bFilterOn = bFilterOn;
    }
    else
        result = Error_NULL_PTR;

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
Demod_SetSuspend(
    DEMOD_DEV   *dev,
    bool        bSuspend,
    void        *extraData)
{
    bool                result = false;
    uint8_t             buffer[16];
    uint32_t            error  = 0;
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

#if (0) && defined(CFG_GPIO_DEMOD_SUSPEND) && (CFG_GPIO_DEMOD_SUSPEND >= 0)
// Now, not support
    if( pOmegaInfo )
    {
        if( pOmegaInfo->bEnableSuspend == bSuspend )    return false;

        _DEMODCTRL_LockIic(dev->demodId);

        do{
            if( bSuspend )
            {
                // tuner suspend
                error = Demodulator_writeRegister((Demodulator*)&(pOmegaInfo->tGanymede), Processor_OFDM, 0, 0xFF);
                if( error )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, "error = 0x%08x\n", error);
                    break;
                }

                dem_msg(1, "\tSet Tuner Suspned\n");
            }
            else
            {
                // tuner Resume
                result = mmpIicReceiveDataEx(IIC_MASTER_MODE, 0x1C, 0, 0, buffer, 5);
                if( result )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, "IIC Error: 0x%08x\n", result);
                    break;
                }

                error = Demodulator_writeRegister((Demodulator*)&(pOmegaInfo->tGanymede), Processor_OFDM, 0, 0);
                if( error )
                {
                    dem_msg_ex(DEM_MSG_TYPE_ERR, "error = 0x%08x\n", error);
                    break;
                }

                dem_msg(1, "Set Tuner Resume\n");
            }

            pOmegaInfo->bEnableSuspend = bSuspend;
            result = true;

        }while(0);

        _DEMODCTRL_ReleaseIic();
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
Demod_GetDemodFwVersion(
    DEMOD_DEV  *dev,
    void       *extraData)
{
    uint32_t  version = 0;

    if( 1 == dev->tsiId )
    {
        return (0x09090900);
    }
    else
    {
        return (version);
    }
}

//=============================================================================
/**
 * Enable DEMOD Retrain
 * @param demodId   control a specific demod and tuner.
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
static uint32_t
Demod_EnableRetrain(
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
Demod_DisableRetrain(
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
Demod_GetBer(
    DEMOD_DEV  *dev,
    uint32_t   *pErrorCount,
    uint32_t   *pBitCount,
    uint16_t   *pAbortCount,
    uint8_t    *extraData)
{
    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo && dev->tsiId == 0 )
    {
        _DEMODCTRL_LockIic(dev->demodId);

        Demodulator_getPostVitBer((Demodulator*)&(pOmegaInfo->tGanymede),
                                  pErrorCount, pBitCount, pAbortCount);

        Demodulator_readRegisters((Demodulator*)&(pOmegaInfo->tGanymede),
                                  Processor_OFDM, 0xA4, 4, extraData);

        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, 0x011c, extraData+4);

        _DEMODCTRL_ReleaseIic();
    }
}

//=============================================================================
/**
 * Used to get demodulator information.
 * @param demodId   control a specific demod and tuner.
 * @return none.
 */
//=============================================================================
static void
Demod_GetInfo(
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

    DEMOD_OMEGA_INFO    *pOmegaInfo = (DEMOD_OMEGA_INFO*)dev->privData;

    if( pOmegaInfo )
    {
        // 1. Get Post_VTB_BER
        // 2. Get RSD_abort_count

         _DEMODCTRL_LockIic(dev->demodId);

        Demodulator_getPostVitBer((Demodulator*)&(pOmegaInfo->tGanymede),
                                  &temp_postErrorCount,   /** 24 bits */
                                  &temp_postBitCount,     /** 16 bits */
                                  &temp_abortCount);

        ptDemodInfo->Post_VTB_BER   = ((double)temp_postErrorCount / (double)temp_postBitCount);
        ptDemodInfo->postErrorCount = temp_postErrorCount;
        ptDemodInfo->postBitCount   = temp_postBitCount;
        ptDemodInfo->RSD_abort_count = temp_abortCount;

        // 3. Get Soft Bit Quality (%)

        // 4. Get Mobile ch
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, p_reg_aagc_mobile_sel,
                                     reg_aagc_mobile_sel_pos, reg_aagc_mobile_sel_len,
                                     &temp);
        ptDemodInfo->MCH = temp;


        // 4.5 Get Channel No
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, ChannelNo, &temp);
        ptDemodInfo->CHNo = temp;



        // 5. Get FDI Type
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, r_reg_ce_var_hw,
                                     reg_ce_var_hw_pos, reg_ce_var_hw_len,
                                     &temp);
        ptDemodInfo->IDF_Type = temp;


        // 6. Get FDI Forced Enable
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, p_reg_ce_var_forced_en,
                                     reg_ce_var_forced_en_pos, reg_ce_var_forced_en_len,
                                     &temp);
        ptDemodInfo->IDF_Forced_Enable = temp;


        // 7. Get FDI Forced Type
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, p_reg_ce_var_forced_value,
                                     reg_ce_var_forced_value_pos, reg_ce_var_forced_value_len,
                                     &temp);
        ptDemodInfo->IDF_Forced_Type = temp;


        // 8. Get AAGC Signal Level
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, signal_level, &temp);
        ptDemodInfo->AAGC_Signal_Level = temp;


        //9. Get Near ACI 0
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, aci_0, &temp);
        ptDemodInfo->N_ICA_0 = temp;


        // 10. Get Near ACI 1
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, aci_1, &temp);
        ptDemodInfo->N_ICA_1 = temp;


        // 10.5 Get Near ACI 2
        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, aci_2, &temp);
        ptDemodInfo->N_ICA_2 = temp;


        // 11. Get CCI0
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, p_reg_mccid_ccif0_state,
                                     reg_mccid_ccif0_state_pos, reg_mccid_ccif0_state_len,
                                     &temp);
        ptDemodInfo->ICC_0 = temp;


        // 12. Get CCI1
        Demodulator_readRegisterBits((Demodulator*)&(pOmegaInfo->tGanymede),
                                     Processor_OFDM, p_reg_mccid_ccif1_state,
                                     reg_mccid_ccif1_state_pos, reg_mccid_ccif1_state_len,
                                     &temp);
        ptDemodInfo->ICC_1 = temp;


        // 13. Get Signal Quality
        Demodulator_getSignalQualityIndication((Demodulator*)&(pOmegaInfo->tGanymede), &temp);
        ptDemodInfo->Signal_Quality = temp;

        // 14. Get Signal Strength
        //Demodulator_getSignalStrength((Demodulator*)&(pOmegaInfo->tGanymede), &temp);
        //ptDemodInfo->Signal_Strength = temp;

        // 14.5 Get Signal Strength (dbm)
        Demodulator_getSignalStrengthDbm((Demodulator*)&(pOmegaInfo->tGanymede), &tempLong);
        ptDemodInfo->Signal_Strength = tempLong;

        // 15. Get More OFDM Information
        /* 15.1 Constellation      : Signal uses QPSK/16QAM/64QAM constellation
           15.2 Frequency         : Channel frequency in KHz.
           15.3 Bandwidth         : Signal bandwidth is 6MHz/7MHz/8MHz/5MHz
           15.4 HighCodeRate    : Signal uses FEC coding ratio of 1/2, 2/3, 3/4, 5/6, 7/8
           15.5 GuardInterval    : Guard interval is 1/32 or 1/16 or 1/8 or 1/4  of symbol length
        */
        Demodulator_getChannelModulation((Demodulator*)&(pOmegaInfo->tGanymede), &channelModulation);

        ptDemodInfo->Constellation = channelModulation.constellation;
        ptDemodInfo->Frequency     = channelModulation.frequency;
        ptDemodInfo->Bandwidth     = channelModulation.bandwidth;
        ptDemodInfo->HighCodeRate  = channelModulation.highCodeRate;
        ptDemodInfo->GuardInterval = channelModulation.interval;

        // 16. Lock status : TPS Locked
        //Demodulator_isTpsLocked((Demodulator*) &(pOmegaInfo->tGanymede), &Locked);
        //ptDemodInfo->TpsLocked = Locked;

        // 17. Lock status : MPEG2 Locked
        //Demodulator_isMpeg2Locked ( (Demodulator*) &(pOmegaInfo->tGanymede), &Locked);
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

        //Demodulator_getFrequencyOffset((Demodulator*)&(pOmegaInfo->tGanymede),
        //                               ElementaryPeriod, &NormalizedOffset,
        //                               &Offset);
        //ptDemodInfo->FrequencyOffset = Offset;

        //Demodulator_getTimeOffset((Demodulator*)&(pOmegaInfo->tGanymede),
        //                          dev->demodId, AdcFrequency, ElementaryPeriod,
        //                          &offset, &offsetPpm);
        //ptDemodInfo->TimeOffset = offsetPpm;

        //Demodulator_getPreVitBer((Demodulator*)&(pOmegaInfo->tGanymede),
        //                         dev->demodId,
        //                         &preErrorCount, &preBitCount, &snr);
        //ptDemodInfo->snr = snr;


        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, r_reg_r_rssi_ori_7_0,
                                 &temp);

        Demodulator_readRegister((Demodulator*)&(pOmegaInfo->tGanymede),
                                 Processor_OFDM, r_reg_r_rssi_ori_9_8,
                                 &temp1);
        ptDemodInfo->rssi = (uint16_t)((temp1 << 8) + temp);

        ptDemodInfo->IsHierarchical = (channelModulation.hierarchy) ? 1 : 0;

        //Demodulator_getFWVersion((Demodulator*)&(pOmegaInfo->tGanymede), &fwVersion);
        //sprintf(ptDemodInfo->DemodFwVersion, "%08x", fwVersion);

        //Demodulator_getOFDMFwVersion((Demodulator*)&(pOmegaInfo->tGanymede), (char*)&(ptDemodInfo->DemodOFDMFwVersion));
        //Demodulator_getLINKFwVersion((Demodulator*)&(pOmegaInfo->tGanymede), (char*)&(ptDemodInfo->DemodLINKFwVersion));
        //Demodulator_getTunerID((Demodulator*)&(pOmegaInfo->tGanymede), (char*)&(ptDemodInfo->DemodTunerID));

        _DEMODCTRL_ReleaseIic();

        api_version_number = Version_NUMBER;
        api_version_date   = Version_DATE;
        api_version_build  = Version_BUILD;
        sprintf((char*) &(ptDemodInfo->DemodApiVersion), "%04x.%08x.%02x", api_version_number, api_version_date, api_version_build);
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
Demod_SetPowerDown(
    DEMOD_DEV   *dev,
    bool        bEnable,
    void        *extraData)
{
    uint32_t result = 0;

    return result;
}

static uint32_t
Demod_Control(
    DEMOD_DEV       *dev,
    DEMOD_CTRL_CMD  cmd,
    uint32_t        *value,
    void            *extraData)
{
    uint32_t result = 0;

    switch( cmd )
    {
        default:
            break;
    }
    return result;
}


DEMOD_DESC  DEMOD_DESC_omega_desc =
{
    "demod omega",          // char        *name;
    0,                      // struct _DEMOD_DESC_TAG  *next;
    DEMOD_ID_OMEGA,         // DEMOD_TYPE_ID           id;
    0,                      // void        *privInfo;
    Demod_Init,             // void (*DemodCtrl_Init)(DEMOD_DEV *dev, void *extraData);
    Demod_Terminate,        // void (*DemodCtrl_Terminate)(DEMOD_DEV *dev, void *extraData);
    Demod_AcquireChannel,   // uint32_t (*DemodCtrl_AcquireChannel)(DEMOD_DEV *dev, void *extraData);
    Demod_IsChannelLock,    // bool (*DemodCtrl_IsChannelLock)(DEMOD_DEV *dev, void *extraData);
    0,                      // bool (*DemodCtrl_IsTpsLock)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    0,                      // bool (*DemodCtrl_IsMpeg2Lock)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_GetSignalStatus,  // bool (*DemodCtrl_GetSignalStatus)(DEMOD_DEV *dev,uint32_t  demodId,DEMOD_SIGNAL_STATISTIC *ptDemodStatistic,void      *extraData);
    0,                      // uint32_t (*DemodCtrl_ReadDataStream)(DEMOD_DEV *dev, uint8_t* buffer, uint32_t bufferlength);
    Demod_UpdatePidTable,   // uint32_t (*DemodCtrl_UpdatePidTable)(DEMOD_DEV  *dev, uint32_t   demodId, uint32_t   pid, PID_FILTER_INDEX  pidIndex,void   *extraData);
    Demod_ResetPidTable,    // uint32_t (*DemodCtrl_ResetPidTable)(DEMOD_DEV   *dev, uint32_t    demodId, bool        bFilterOn, void        *extraData);
    Demod_SetSuspend,       // bool (*DemodCtrl_SetSuspend)(DEMOD_DEV *dev, uint32_t demodId, bool bEnableSuspend, void *extraData);
    0,                      // bool (*DemodCtrl_OpenTuner)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    0,                      // bool (*DemodCtrl_CloseTuner)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_GetDemodFwVersion, // uint32_t (*DemodCtrl_GetDemodFwVersion)(DEMOD_DEV *dev, uint32_t demodId, void *extraData);
    Demod_EnableRetrain,    // uint32_t (*DemodCtrl_EnableRetrain)(DEMOD_DEV *dev, void *extraData);
    Demod_DisableRetrain,   // uint32_t (*DemodCtrl_DisableRetrain)(DEMOD_DEV *dev, void *extraData);
    Demod_GetBer,           // void (*DemodCtrl_GetBer)(DEMOD_DEV *dev, uint32_t  demodId,uint32_t  *pErrorCount,uint32_t  *pBitCount,uint16_t  *pAbortCount,uint8_t   *pExtData,void      *extraData);
    Demod_GetInfo,          // void (*DemodCtrl_GetInfo)(DEMOD_DEV *dev, uint32_T demodId, DEMOD_INFO* ptDemodInfo, void *extraData);
    Demod_SetPowerDown,     // int (*DemodCtrl_SetPowerDown)(DEMOD_DEV *dev, void *extraData);
    Demod_Control,
};

#else

DEMOD_DESC  DEMOD_DESC_omega_desc =
{
    "demod omega",              // char        *name;
    0,                          // struct _DEMOD_DESC_TAG  *next;
    DEMOD_ID_OMEGA,             // DEMOD_TYPE_ID           id;
};

#endif

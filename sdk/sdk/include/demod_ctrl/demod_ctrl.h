#ifndef __DEMOD_CTRL_H_WXH1X8GL_GHS6_D5QH_7XDS_XWIIT402NKYA__
#define __DEMOD_CTRL_H_WXH1X8GL_GHS6_D5QH_7XDS_XWIIT402NKYA__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include "demod_ctrl_err.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define DEMOD_TOTAL_FILTER_COUNT    (32)

/**
 * demod H/W engine status
 **/
typedef enum DEMOD_ENG_STATUS_TAG
{
    DEMOD_ENG_STATUS_UNKNOW     = 0,
    DEMOD_ENG_STATUS_IDLE       = 1,
    DEMOD_ENG_STATUS_RUNNING    = 2,

}DEMOD_ENG_STATUS;


/**
* demod control cmd
**/
typedef enum _DEMOD_CTRL_CMD_TAG
{
    DEMOD_CTRL_CMD_UNKNOW   = 0,
    DEMOD_CTRL_CMD_SET_TSI_ID,
    DEMOD_CTRL_CMD_IS_HIERARCHICAL_TX,

}DEMOD_CTRL_CMD;

/**
* demod/turn type
**/
typedef enum _DEMOD_TYPE_ID_TAG
{
    DEMOD_ID_UNKNOW     = 0,
    DEMOD_ID_OMEGA,
    DEMOD_ID_IT9135,
    DEMOD_ID_IT9137,
    DEMOD_ID_IT9137_USB,

}DEMOD_TYPE_ID;

/* The index of PID filter table. However, due to more than one active PMT pids
* in a specific frequency channel, the index of PMT here is just the first PMT
* index.
**/
typedef enum PID_FILTER_INDEX_TAG
{
    PID_PAT_INDEX       = 0,
    PID_SDT_INDEX,
    PID_EIT_INDEX,
    PID_TOT_TDT_INDEX,
    PID_NIT_INDEX,
    PID_PMT_INDEX,
    PID_AUDIO_RESERVE_0 = (DEMOD_TOTAL_FILTER_COUNT - 7),
    PID_AUDIO_RESERVE_1 = (DEMOD_TOTAL_FILTER_COUNT - 6),
    PID_AUDIO_RESERVE_2 = (DEMOD_TOTAL_FILTER_COUNT - 5),
    PID_AUDIO_INDEX     = (DEMOD_TOTAL_FILTER_COUNT - 4),
    PID_TELETEXT_INDEX  = (DEMOD_TOTAL_FILTER_COUNT - 3),
    PID_SUBTITLE_INDEX  = (DEMOD_TOTAL_FILTER_COUNT - 2),
    PID_VIDEO_INDEX     = (DEMOD_TOTAL_FILTER_COUNT - 1)
} PID_FILTER_INDEX;

/**
 *
 **/
typedef enum DEMOD_BUS_TYPE_TAG
{
    DEMOD_BUS_UNKNOW    = 0,
    DEMOD_BUS_I2C,
    DEMOD_BUS_USB,
    
}DEMOD_BUS_TYPE;
//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  demod attribute
 **/
typedef struct DEMOD_ATTR_T
{
    uint32_t            demod_idx;
    DEMOD_BUS_TYPE      bus_type; 
    DEMOD_TYPE_ID       demod_type;  // demod chip type
    uint32_t            demod_i2c_addr;

    uint32_t            linked_aggre_port_idx;

    void                *privData;
}DEMOD_ATTR;

/**
 * demod setup info for initialize
 **/
typedef struct _DEMOD_SETUP_INFO_TAG
{
    // demod module support optimal setting by using case
    // [0]: Normal, [1]: Config_1, [2]: Config_2
    uint8_t     supportType;

    // demod it9133 architecture type,
    // [0]: Inavalid, [1]: Diversity combine, [2]: Picture in picture
    uint32_t    architecture;

}DEMOD_SETUP_INFO;

/**
 *                                                                      
 **/
typedef struct DEMOD_SIGNAL_STATISTIC_TAG
{
    uint32_t    signalStrength;
    uint32_t    signalQuality;
    bool        bMpg2Lock;

}DEMOD_SIGNAL_STATISTIC;

/**
 * demode control handle
 **/
typedef struct _DEMOD_CTRL_HANDLE_TAG
{
    uint32_t    tsiId;
    uint32_t    demodId;
    uint32_t    linked_aggre_port_idx;
    uint32_t    frequency;
    uint32_t    bandwith;

}DEMOD_CTRL_HANDLE;

//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================
uint32_t
DemodCtrl_CreateHandle(
    DEMOD_CTRL_HANDLE   **pHDemodCtrl,
    DEMOD_ATTR          *pDemodAttr);


uint32_t
DemodCtrl_DestroyHandle(
    DEMOD_CTRL_HANDLE   **pHDemodCtrl);
//=============================================================================
/**
 * Used to Init the front end demodulator and turner.
 * @param setupInfo
 * @return none.
 */
//=============================================================================
uint32_t
DemodCtrl_Init(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_SETUP_INFO    *pSetupInfo);

//=============================================================================
/**
 * Used to Terminate the inited front end demodulator and turner.
 * @return none.
 */
//=============================================================================
uint32_t
DemodCtrl_Terminate(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

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
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);


//=============================================================================
/**
 * Check if the demod is locked after channel acquisition step.
 * @return MMP_BOOL If the demod is locked then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
bool
DemodCtrl_IsChannelLock(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

//=============================================================================
/**
 * Check if TPS locked after channel acquisition step.
 * @return MMP_BOOL If the TPS is locked then return MMP_TRUE, MMP_FALSE
 *                  otherwise.
 */
//=============================================================================
bool
DemodCtrl_IsTpsLock(
     DEMOD_CTRL_HANDLE   *pHDemodCtrl);

//=============================================================================
/**
 * Check if MPEG2 locked after channel acquisition step.
 * @return uint32_t the error code from demod control API if any.
 */
//=============================================================================
bool
DemodCtrl_IsMpeg2Lock(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

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
    DEMOD_SIGNAL_STATISTIC  *ptDemodStatistic);

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
    void                 *pCtrlParam);

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
    PID_FILTER_INDEX    pidIndex);

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
    bool                bFilterOn);

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
    bool                bSuspend);

//=============================================================================
/**
 * Open Tuner
 * @return MMP_BOOL Whether the action success.
 */
//=============================================================================
bool
DemodCtrl_OpenTuner(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

//=============================================================================
/**
 * Close Tuner
 * @return MMP_BOOL Whether the action success.
 */
//=============================================================================
bool
DemodCtrl_CloseTuner(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);


//=============================================================================
/**
 * Adjust Orion DCXO
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_GetDemodFwVersion(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

//=============================================================================
/**
 * Enable DEMOD Retrain
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_EnableRetrain(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

//=============================================================================
/**
 * Disable DEMOD Retrain
 * @return version number if successful, 0 if failed.
 */
//=============================================================================
uint32_t
DemodCtrl_DisableRetrain(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl);

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
    uint8_t             *pExtData);

//=============================================================================
/**
 * To Power Down the demond.
 * note iic address is 0x20
 */
//=============================================================================
uint32_t
DemodCtrl_SetPowerDown(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    bool                bEnable);


uint32_t
DemodCtrl_Control(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_CTRL_CMD      cmd,
    uint32_t            *value,
    void                *extraData);


uint32_t
DemodCtrl_Set_Engine_Status(
    DEMOD_CTRL_HANDLE   *pHDemodCtrl,
    DEMOD_ENG_STATUS    status);


#ifdef __cplusplus
}
#endif

#endif

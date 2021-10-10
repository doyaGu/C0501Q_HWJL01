#ifndef __DEMOD_H_R38YOB96_J2IB_PSV1_B6MS_K2L372YQIG5F__
#define __DEMOD_H_R38YOB96_J2IB_PSV1_B6MS_K2L372YQIG5F__

#ifdef __cplusplus
extern "C" {
#endif


#include "demod_config.h"
#include "demod_ctrl_defs.h"
#include "demod_ctrl.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define DEMOD_MAX_PMT_FILTER_COUNT  (23)

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _DEMOD_INFO_TAG
{
    double    Post_VTB_BER;
    double    TimeOffset;
    double    snr;
    uint32_t  postErrorCount;
    uint32_t  postBitCount;
    uint32_t  FrequencyOffset;
    uint32_t  Frequency;
    int32_t   Signal_Strength_dBm;
    uint16_t  RSD_abort_count;
    uint16_t  rssi;

    uint8_t   MCH;
    uint8_t   CHNo;
    uint8_t   IDF_Type;
    uint8_t   IDF_Forced_Enable;
    uint8_t   IDF_Forced_Type;
    uint8_t   N_ICA_0;
    uint8_t   N_ICA_1;
    uint8_t   N_ICA_2;
    uint8_t   ICC_0;
    uint8_t   ICC_1;
    uint8_t   AAGC_Signal_Level;
    uint8_t   Signal_Quality;
    uint8_t   Signal_Strength;

    uint8_t   Constellation;
    uint8_t   Bandwidth;
    uint8_t   HighCodeRate;
    uint8_t   GuardInterval;
    uint8_t   TpsLocked;
    uint8_t   Mpeg2Locked;
    uint8_t   IsHierarchical;
    char      DemodTunerID[20];
    char      DemodApiVersion[20];
    char      DemodFwVersion[20];
    char      DemodOFDMFwVersion[20];
    char      DemodLINKFwVersion[20];
} DEMOD_INFO;

typedef struct _DEMOD_DEV_TAG
{
    uint32_t    tsiId;

    // wl+
    // for run-time modiy demod 
    DEMOD_ATTR  demod_attr;
    // wl-

    uint32_t    demodId;

    uint8_t     supportType;  // demod module support optimal setting by using case
    uint32_t    architecture; // demod it9133 architecture type, 0: Inavalid, 1: Diversity combine, 2: Picture in picture
    
    uint32_t    frequency;
    uint32_t    bandwith;

    bool        bHierarchicalTx;
    void        *privData;

    void        *pApCtrlParam;
}DEMOD_DEV;


typedef struct _DEMOD_DESC_TAG
{
    char        *name;

    struct _DEMOD_DESC_TAG  *next;
    DEMOD_TYPE_ID           id;

    void        *privInfo;

    // operator

    /**
     * Used to Init the front end demodulator and turner.
     * @return none.
     */
    void (*DemodCtrl_Init)(DEMOD_DEV *dev, void *extraData);

    /**
     * Used to Terminate the inited front end demodulator and turner.
     * @return none.
     */
    void (*DemodCtrl_Terminate)(DEMOD_DEV *dev, void *extraData);

    /**
     * By specifying the frequency and bandwith to detech if any channel locked or
     * existed through the setting parameters.
     * @param frequency The desired checking frequency by unit of kHz.
     * @param bandwith  The bandwith of the specific frequency channel. Usually,
     *                  the bandwith is diverse by different countries.
     * @return uint32_t the error code from demod control API if any.
     */
    uint32_t (*DemodCtrl_AcquireChannel)(DEMOD_DEV *dev, void *extraData);

    /**
     * Check if the demod is locked after channel acquisition step.
     * @return bool If the demod is locked then return MMP_TRUE, MMP_FALSE
     *                  otherwise.
     */
    bool (*DemodCtrl_IsChannelLock)(DEMOD_DEV *dev, void *extraData);

    /**
     * Check if TPS locked after channel acquisition step.
     * @return bool If the TPS is locked then return MMP_TRUE, MMP_FALSE
     *                  otherwise.
     */
    bool (*DemodCtrl_IsTpsLock)(DEMOD_DEV *dev, void *extraData);

    /**
     * Check if MPEG2 locked after channel acquisition step.
     * @return uint32_t the error code from demod control API if any.
     */
    bool (*DemodCtrl_IsMpeg2Lock)(DEMOD_DEV *dev, void *extraData);

    /**
     * Retrieve current signal statistic from demodulator.
     * @param ptDemodStatistic  The output of current signal status if return
     *                          is MMP_TRUE.
     * @return Whether the operation is success or not.
     */
    bool (*DemodCtrl_GetSignalStatus)(DEMOD_DEV *dev,
                                      DEMOD_SIGNAL_STATISTIC *ptDemodStatistic,
                                      void      *extraData);

   /**
     * Read TS stream Datagram from USB interface
     * @param demodId   control a specific demod and tuner.
     * @return MMP_BOOL If got Data then return MMP_TRUE, MMP_FALSE
     *                  otherwise.
     */
    uint32_t (*DemodCtrl_ReadDataStream)(DEMOD_DEV   *dev,
                                         uint8_t*     buffer,
                                         uint32_t     bufferlength,
                                         void*        pfCallback);

    /**
     * Update the PID filter table by specified PID. For a new PID just inserts it
     * into the PID filter table, update the table otherwise.
     * @param pid       The desired filtering PID.
     * @param pidType   The desired PID Type.
     * @return uint32_t the error code from demod control API if any.
     */
    uint32_t (*DemodCtrl_UpdatePidTable)(DEMOD_DEV          *dev,
                                         uint32_t           pid,
                                         PID_FILTER_INDEX   pidIndex,
                                         void               *extraData);

    /**
     * Reset the PID filter table
     * @param  bFilterOn    The desired filtering PID.
     * @return uint32_t   the error code from demod control API if any.
     */
    uint32_t (*DemodCtrl_ResetPidTable)(DEMOD_DEV   *dev,
                                        bool        bFilterOn,
                                        void        *extraData);

    /**
     * To setup whether the demond enter the suspend state or not.
     * @param bSuspend    if MMP_TRUE, the demod will enter the suspend state.
     * @return bool         Whether the mode change success.
     */
    bool (*DemodCtrl_SetSuspend)(DEMOD_DEV *dev, bool bSuspend, void *extraData);

    /**
     * Open Tuner
     * @return bool Whether the action success.
     */
    bool (*DemodCtrl_OpenTuner)(DEMOD_DEV *dev, void *extraData);

    /**
     * Close Tuner
     * @return bool Whether the action success.
     */
    bool (*DemodCtrl_CloseTuner)(DEMOD_DEV *dev, void *extraData);

    /**
     * Adjust Orion DCXO
     * @return version number if successful, 0 if failed.
     */
    uint32_t (*DemodCtrl_GetDemodFwVersion)(DEMOD_DEV *dev, void *extraData);

    /**
     * Enable DEMOD Retrain
     * @return version number if successful, 0 if failed.
     */
    uint32_t (*DemodCtrl_EnableRetrain)(DEMOD_DEV *dev, void *extraData);

    /**
     * Disable DEMOD Retrain
     * @return version number if successful, 0 if failed.
     */
    uint32_t (*DemodCtrl_DisableRetrain)(DEMOD_DEV *dev, void *extraData);

    /**
     * Used to get demodulator information.
     * @param demodId   control a specific demod and tuner.
     * @return none.
     */
    void (*DemodCtrl_GetBer)(DEMOD_DEV *dev,
                             uint32_t  *pErrorCount,
                             uint32_t  *pBitCount,
                             uint16_t  *pAbortCount,
                             uint8_t   *extraData);

    /**
     * Used to get demodulator information.
     * @return none.
     */
    void (*DemodCtrl_GetInfo)(DEMOD_DEV *dev, DEMOD_INFO* ptDemodInfo, void *extraData);

    /**
     * To Power Down the demond.
     * note iic address is 0x20
     */
    uint32_t (*DemodCtrl_SetPowerDown)(DEMOD_DEV *dev, bool bEnable, void *extraData);

    uint32_t (*DemodCtrl_Control)(DEMOD_DEV *dev, DEMOD_CTRL_CMD cmd, uint32_t *value,void *extraData);

}DEMOD_DESC;

//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

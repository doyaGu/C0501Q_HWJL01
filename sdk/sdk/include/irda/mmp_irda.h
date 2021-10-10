#ifndef MMP_IRDA_H 
#define MMP_IRDA_H 

#ifdef __cplusplus
extern "C" {
#endif

#if 0//defined(_WIN32) || defined(_WIN32_WCE)
	#if defined(IRDA_EXPORTS)
		#define IRDA_API __declspec(dllexport)
	#else
		#define IRDA_API __declspec(dllimport)
	#endif
#else
	#define IRDA_API extern
#endif /* defined(_WIN32) || defined(_WIN32_WCE) */
//#define IRDA_API

#include <stdio.h>
#include <unistd.h>
#include "stdint.h"

typedef struct IR_PARAMEMTER_TAG
{
    uint32_t baudrate;
    uint32_t sip_pw_value;
    uint8_t bTxInv;
    uint8_t bRxInv;
    uint32_t bDmaMode;
}IR_PARAMEMTER;

/**
 * Initialize FIR module.
 *
 * @param parameter of IR, include baudrate/sip_pw_value/... .
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirInitialize(IR_PARAMEMTER *parameter);

/**
 * Terminate FIR module.
 *
 * @param none.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirTerminate(void);

/**
 * Send Data by FIR module.
 *
 * @param buffer input data buffer.
 * @param size  Number of bytes.
 * @param param  externtion parameter.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirSendData(uint8_t *buffer, uint32_t size, void* param);

/**
 * Send Frame by FIR module.
 *
 * @param buffer input data buffer.
 * @param frameSize  Frame Size
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirSendFrame(uint8_t *buffer, uint32_t frameSize);

/**
 * Check state of Frame Send.
 *
 * @param none.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirSendCheck(void);

/**
 * Close FIR Frame Send.
 *
 * @param none.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirSendClose(void);

/**
 * Receive Data by FIR module.
 *
 * @param buffer output data buffer.
 * @param size  Number of bytes.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirReceiveData(uint8_t *buffer, uint32_t size);

/**
 * Receive Frame by FIR module.
 *
 * @param buffer output data buffer.
 * @param maxSize  input the max data size.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirReceiveFrame(uint8_t *buffer, uint32_t maxSize);

/**
 * Check state of Frame Received.
 *
 * @param fsize  output the frame size.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirReceiveCheck(int32_t *fsize, uint32_t *fCount);

/**
 * Close FIR Frame Received.
 *
 * @param none.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpFirReceiveClose(void);

/**
 * Initialize SIR module.
 *
 * @param parameter of IR, include baudrate/sip_pw_value/... .
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirInitialize(IR_PARAMEMTER *parameter);

/**
 * Terminate SIR module.
 *
 * @param none.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirTerminate(void);

/**
 * Start to Send Data by SIR module.Enable Tx and Disable Rx 
 *
 * @param size  None.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirSendStart(void);

/**
 * Stop to Send Data by SIR module.Enable Rx and Disable Tx 
 *
 * @param size  None.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirSendClose(void);

/**
 * Send Data by SIR module.
 *
 * @param buffer input data buffer.
 * @param size  Number of bytes.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirSendData(uint8_t *buffer, uint32_t size);

/**
 * Send One Byte by SIR module.
 *
 * @param ch input character.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
IRDA_API int32_t mmpSirSendByte(uint8_t ch);

/**
 * Receive Data by SIR module.
 *
 * @param buffer output data buffer.
 * @param size  Number of bytes.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
 
IRDA_API int32_t mmpSirReceiveByte(uint8_t *ch);

/**
 * Receive One Byte by SIR module.
 *
 * @param ch output character.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
#endif

/*
 * Copyright (c) 2006 ITE Technology Corp. All Rights Reserved.
 */
/** @file libamr.h
 * libamr include file
 *
 * @author Cory Tong
 * @version 1.0
 */
#ifndef _LIBAMR_H_
#define _LIBAMR_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "typedef.h"
//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MODE_BITRATE_TAG {
    MODE_BR475 = 0,
    MODE_BR515,
    MODE_BR59,
    MODE_BR67,
    MODE_BR74,
    MODE_BR795,
    MODE_BR102,
    MODE_BR122
} MODE_BITRATE;

typedef enum MODE_MODE_TAG {
    MODE_CBR = 0,
    MODE_VBR
} MODE_MODE;

//=============================================================================
//                              Public Function Declaration
//=============================================================================

/*
   CodecID AMR_getCodecID(void);
   CodecType AMR_getCodecCategory(void);
   DWORD AMR_getCodecTag(void);
   DWORD AMR_getCodecType(void);
   DWORD AMR_getVersion(void);
   DWORD AMR_getAuthor(void);
   bool AMR_isInitEncode(void);
   bool AMR_isInitDecode(void);
   bool AMR_initEncode(DWORD dwData, DWORD dwData2);
   bool AMR_initDecode(DWORD dwData, DWORD dwData2);
   bool AMR_finalizeEncode(DWORD dwData);
   bool AMR_finalizeDecode(DWORD dwData);
   bool AMR_encode(BYTE *pIn, int inSize, BYTE *pOut, int *pOutSize, DWORD dwData);
   bool AMR_decode(BYTE *pIn, int inSize, BYTE *pOut, int *pOutSize, DWORD dwData);
   DWORD AMR_getFrameTime(void);
   bool AMR_getDecodeYUV(BYTE **py,BYTE **pu,BYTE **pv,int *pw,int *ph);
   bool AMR_decodeFrameHeader(BYTE *pIn, int inSize, FrameType *pType);
 */

/**
 * Initial AMR SW solution
 *
 * @param bitrate [IN]      set bitrate
 * @param cvbr [IN]         set CBR/VBR
 * @return true if succeed, false otherwise.
 * @see AMR_encode(), AMR_finalizeEncode(), AMR_getEncodeFrame(), AMR_getEncodeTime()
 * @remark Initial at just start up and after calling AMR_finalizeEncode().
 *
 */
Bool AMR_initEncode(enum MODE_BITRATE bitrate, enum MODE_MODE cvbr);

/**
 * AMR SW encode
 *
 * @param pIn [IN]          pointer of input PCM bitstram
 * @param iInSize [IN]      length of input bitstream, by 8 bits (BYTE)
 * @param pOut [OUT]        pointer of output buffer, AMR bitstream inside
 * @param iOutSize [IN/OUT] length of ouput buffer, and length of encode AMR bitstream, by 8 bits (BYTE)
 * @return true if succeed, false otherwise.
 * @see AMR_initEncode(), AMR_finalizeEncode(), AMR_getEncodeFrame(), AMR_getEncodeTime()
 * @remark
 *  (1) Be sure pOut has enough length.
 *  (2) False when remain not enough PCM sample.
 *  (3) False when output buffer is full.
 *
 */
Bool AMR_encode(Word8 *pIn, Word32 iInSize, Word8 *pOut, Word32 *iOutSize);

/**
 * Finalize AMR encode
 *
 * @return true if succeed, false otherwise.
 * @see AMR_initEncode(), AMR_encode(), AMR_getEncodeFrame(), AMR_getEncodeTime()
 * @remark Must call AMR_initEncode() after finalize.
 *
 */
Bool AMR_finalizeEncode();

/**
 * Report Encode Total Frame
 *
 * @return Count of encode frame
 * @see AMR_initEncode(), AMR_encode(), AMR_finalizeEncode(), AMR_getEncodeTime()
 * @remark Reset after AMR_initEncode() and AMR_finalizeEncode().
 *
 */
UWord32 AMR_getEncodeFrame();

/**
 * Report Encode Total Time
 *
 * @return Total record time of encode frame
 * @see AMR_initEncode(), AMR_encode(), AMR_finalizeEncode(), AMR_getEncodeFrame()
 * @remark Reset after AMR_initEncode() and AMR_finalizeEncode().
 *
 */
UWord32 AMR_getEncodeTime();

/**
 * Initial AMR SW solution
 *
 * @return true if succeed, false otherwise.
 * @see AMR_decode(), AMR_finalizeDecode(), AMR_getDecodeFrame(), AMR_getDecodeTime()
 * @remark Initial at just start up and after calling AMR_finalizeDecode().
 *
 */
Bool AMR_initDecode();

/**
 * AMR SW decode
 *
 * @param pIn [IN]          pointer of input AMR bitstream
 * @param iInSize [IN]      length of input bitstream, by 8 bits (BYTE)
 * @param pOut [OUT]        pointer of output buffer, PCM bistream inside
 * @param iOutSize [IN/OUT] length of ouput buffer, and length of decoded PCM bitstream, by 8 bits (BYTE)
 * @return true if succeed, false otherwise.
 * @see AMR_initDecode(), AMR_finalizeDecode(), AMR_getDecodeFrame(), AMR_getDecodeTime()
 * @remark
 *  (1) Be sure pOut has enough length.
 *  (2) False when remain not completed AMR frame.
 *  (3) False when output buffer is full.
 *
 */
Bool AMR_decode(Word8 *pIn, Word32 iInSize, Word8 *pOut, Word32 *iOutSize);

/**
 * Finalize AMR deocde
 *
 * @return true if succeed, false otherwise.
 * @see AMR_initDecode(), AMR_decode(), AMR_getDecodeFrame(), AMR_getDecodeTime()
 * @remark Must call AMR_initDecode() after finalize.
 *
 */
Bool AMR_finalizeDecode();

/**
 * Report Decode Total Frame
 *
 * @return Count of decoded frame
 * @see AMR_initDecode(), AMR_decode(), AMR_finalizeDecode(), AMR_getDecodeTime()
 * @remark Reset after AMR_initDecode() and AMR_finalizeDecode().
 *
 */
UWord32 AMR_getDecodeFrame();

/**
 * Report Decode Total Time
 *
 * @return Total playback time of decoded frame
 * @see AMR_initDecode(), AMR_decode(), AMR_finalizeDecode(), AMR_getDecodeFrame()
 * @remark Reset after AMR_initDecode() and AMR_finalizeDecode().
 *
 */
UWord32 AMR_getDecodeTime();

#ifdef __cplusplus
}
#endif
#endif  /* _LIBAMR_H_ */
/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file bus.c
 *
 * @version 0.1
 */
#include "bus.h"
#include "itx.h"
#include "pal.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if (ITX_BUS_TYPE == ITX_BUS_I2C)
#define IIC_MAX_SIZE            256
#endif
//=============================================================================
//                              Global Data Definition
//=============================================================================
#if (ITX_BUS_TYPE == ITX_BUS_I2C)
MMP_UINT8  gITXs_IIC_ID       = 0x58 >> 1;
#endif

// Global Mutex for critical section protection
//static pthread_mutex_t gBusCriticalSection = 0;
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Function Declaration
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

#if (ITX_BUS_TYPE == ITX_BUS_I2C)
//=============================================================================
/**
 * Use IIC port.
 */
//=============================================================================
MMP_RESULT
busI2COpen(
    void)
{
 //   if (gBusCriticalSection)
 //		return MMP_RESULT_SUCCESS;
 //
 //	pthread_mutex_init(&gBusCriticalSection, NULL);

	//if (0 == gBusCriticalSection)
 //		return MMP_RESULT_ERROR_CRITICAL_SECTION;
 		
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busI2CClose(
    void)
{
    //if (gBusCriticalSection != 0)
    //    pthread_mutex_destroy(&gBusCriticalSection);
        
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busI2CEnter(
    void)
{
  //  if (0 == gBusCriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gBusCriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_lock(&gBusCriticalSection);
    
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busI2CLeave(
    void)
{
  //  if (0 == gBusCriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gBusCriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_unlock(&gBusCriticalSection);
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busI2CRead(
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size)    
{
    MMP_RESULT  result = MMP_RESULT_SUCCESS;
    MMP_UINT8   txBuf[2];
   
    txBuf[0] = (MMP_UINT8)(startAddr & 0x00FF);
    txBuf[1] = (MMP_UINT8)((startAddr & 0xFF00) >> 8) ;

    /*
    *  ToDo:  Add code here
    *
    *  //Pseudo code
    */
    //result = mmpIicReceiveDataEx(0, slaveAddr, txBuf, 2, (MMP_UINT8 *)pdata, size); 
    
    return result;
}

MMP_RESULT
busI2CWrite(   
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size)   
{
    MMP_RESULT  result = MMP_RESULT_SUCCESS;
    MMP_UINT8   txBuf[IIC_MAX_SIZE + 2];
    MMP_UINT8   *ptxBuf = txBuf, *ptwrdata = pdata;
    MMP_UINT32  size2;
    MMP_UINT32  i2c_addr = startAddr, remainlen, writelen;

    writelen  = (size > IIC_MAX_SIZE) ? IIC_MAX_SIZE : size;
    remainlen = size;    

    while(remainlen > 0)
    {
        ptxBuf = txBuf;
         
        *ptxBuf++ = (MMP_UINT8)(i2c_addr & 0x00FF);
        *ptxBuf++ = (MMP_UINT8)((i2c_addr & 0xFF00) >> 8);

        PalMemcpy(ptxBuf, ptwrdata, writelen);
        size2 = writelen + 2;
    
        /*
        *  ToDo:  Add code here
        *
        *  //Pseudo code
        */
        //result = mmpIicSendDataEx(0, slaveAddr, txBuf, size2);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
                
        ptwrdata += writelen;
        remainlen -= writelen;
        i2c_addr = i2c_addr + (writelen >> 1);
        writelen = (remainlen > IIC_MAX_SIZE) ? IIC_MAX_SIZE : remainlen;                 
    } 
            
    return result;
}
#endif


#if (ITX_BUS_TYPE == ITX_BUS_SPI)
//=============================================================================
/**
 * Use SPI port.
 */
//=============================================================================
MMP_RESULT
busSPIOpen(
    void)
{
 //   if (gBusCriticalSection)
 //		return MMP_RESULT_SUCCESS;
 //
 //	pthread_mutex_init(&gBusCriticalSection, NULL);

	//if (0 == gBusCriticalSection)
 //		return MMP_RESULT_ERROR_CRITICAL_SECTION;
 		
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busSPIClose(
    void)
{
    //if (gBusCriticalSection != 0)
    //    pthread_mutex_destroy(&gBusCriticalSection);
    
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busSPIEnter(
    void)
{
  //  if (0 == gBusCriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gBusCriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_lock(&gBusCriticalSection);
    
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busSPILeave(
    void)
{
  //  if (0 == gBusCriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gBusCriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_unlock(&gBusCriticalSection);
    return MMP_RESULT_SUCCESS;
}

MMP_RESULT
busSPIRead(
    MMP_UINT8*  pCtrl,
    MMP_UINT32  ctrlLen,
    MMP_UINT8*  pData,
    MMP_UINT32  dataLen)
{
    MMP_RESULT result = MMP_SUCCESS;

    /*
      *  ToDo:  Add code here
      *
      *  //Pseudo code
      */
#ifdef CFG_CHIP_PKG_IT9079       
    if ( ctrlLen + dataLen > 16)
        result = mmpSpiDmaReadNoDropFirstByte(SPI_0, pCtrl, ctrlLen, pData, dataLen, 8);
    else 
    	result = mmpSpiPioRead(SPI_0, pCtrl, ctrlLen, pData, dataLen, 8);
    
    return result;
#else
 	if ( ctrlLen + dataLen > 16)
 	{ 	
        result = mmpSpiDmaRead(SPI_1, pCtrl, ctrlLen, pData, dataLen, 8);
 	}
	else    
        result = mmpSpiPioRead(SPI_1, pCtrl, ctrlLen, pData, dataLen, 8);

	if (result)
		return MMP_SUCCESS;
	else
    	return MMP_FALSE;
#endif
}

MMP_RESULT 
busSPIWrite(
    MMP_UINT8*  pCtrl,
    MMP_UINT32  ctrlLen,
    MMP_UINT8*  pData,
    MMP_UINT32  dataLen)
{
    MMP_RESULT result = MMP_SUCCESS;

     /*
      *  ToDo:  Add code here
      *
      *  //Pseudo code
      */
#ifdef CFG_CHIP_PKG_IT9079 
    if ( ctrlLen + dataLen > 16)
        result = mmpSpiDmaWrite(SPI_0, pCtrl, ctrlLen, pData, dataLen, 8);
    else    
        result = mmpSpiPioWrite(SPI_0, pCtrl, ctrlLen, pData, dataLen, 8);

    return result;
#else
    if ( ctrlLen + dataLen > 16)
        result = mmpSpiDmaWrite(SPI_1, pCtrl, ctrlLen, pData, dataLen, 8);
    else    
        result = mmpSpiPioWrite(SPI_1, pCtrl, ctrlLen, pData, dataLen, 8);

	if (result)
		return MMP_SUCCESS;
	else
    	return MMP_FALSE;
#endif
}
#endif
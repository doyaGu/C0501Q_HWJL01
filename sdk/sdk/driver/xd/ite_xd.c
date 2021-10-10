/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  XD flash extern API implementation.
 *
 * @author Joseph
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
//#include "xd/xd_mlayer.h"
#include "../include/ite/ite_xd.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define NF_API
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================


//=============================================================================
//                              Private Function Definition
//=============================================================================
////////////////////////////////////////////////////////////
//
//	Function Implementation
//
////////////////////////////////////////////////////////////

XD_API XD_RESULT iteXdInitial()
{
	unsigned int sectorNum;
	unsigned int blockLength;

	if(mmpxDInitialize()==MMP_TRUE)
	{
		return true;
	}
	else
	{
		return -1; 
	}
}

XD_API XD_RESULT iteXdTerminate(void)
{
	if(mmpxDTerminate()==MMP_TRUE)
	{
		return true;
	}
	else
	{
		return -1;
	}
}

XD_API XD_RESULT iteXdRead(uint32_t sector, uint32_t count, void* data)
{
	if(mmpxDRead(sector, count, data)==MMP_TRUE)
	{
		return true;
	}
	else
	{
		return -1;
	}
}


XD_API XD_RESULT iteXdWrite(uint32_t sector, uint32_t count, void* data)
{
	if(mmpxDWrite(sector, count, data)==MMP_TRUE)
	{
		return true;
	}
	else
	{
		return -1; 
	}
}

XD_API XD_RESULT iteXdGetCapacity(uint32_t* sectorNum, uint32_t* blockLength)
{
	if(mmpxDGetCapacity(sectorNum, blockLength)==MMP_TRUE)
	{
		return true;
	}
	else
	{
		return -1;
	}
}

XD_API XD_RESULT iteXdGetCardState(XD_CARD_STATE index)
{
	//return (uint32_t)mmpxDGetCardState(state); 
	return true;
}

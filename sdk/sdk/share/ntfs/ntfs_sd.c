#include <sys/ioctl.h>
#include "gctypes.h"
#include "disc_io.h"
#include "ite/itp.h"
#include "ite/ite_sd.h"

#define DEVICE_TYPE_WII_SD (('W'<<24)|('I'<<16)|('S'<<8)|'D')

static bool sd_Startup(int card)
{
    return true;
}
 
static bool sd_Shutdown(int card)
{
	return true;
}
 
static bool sd_ReadSectors(int card, sec_t sector, sec_t numSectors,void* buffer)
{
	s32 ret;
 
	if(buffer==NULL) return false;
 
    ret = iteSdReadMultipleSectorEx(card, sector, numSectors, (void*)buffer);
 
	return (ret==0);
}
 
static bool sd_WriteSectors(int card, sec_t sector, sec_t numSectors,const void* buffer)
{
	s32 ret;
 
	if(buffer==NULL) return false;
 
    ret = iteSdWriteMultipleSectorEx(card, sector, numSectors, (void*)buffer);
 
	return (ret==0);
}
 
static bool sd_ClearStatus(int card)
{
	return true;
}

static bool sd0_Startup(void)
{
	return sd_Startup(0);
}
 
static bool sd0_Shutdown(void)
{
	return sd_Shutdown(0);
}
 
static bool sd0_ReadSectors(sec_t sector, sec_t numSectors,void* buffer)
{
	return sd_ReadSectors(0, sector, numSectors, buffer);
}
 
static bool sd0_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer)
{
	return sd_WriteSectors(0, sector, numSectors, buffer);
}
 
static bool sd0_ClearStatus(void)
{
	return sd_ClearStatus(0);
}
 
static bool sd0_IsInserted(void)
{
	return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_SD0);
}

const DISC_INTERFACE __io_sd0 = {
	DEVICE_TYPE_WII_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_SD,
	(FN_MEDIUM_STARTUP)&sd0_Startup,
	(FN_MEDIUM_ISINSERTED)&sd0_IsInserted,
	(FN_MEDIUM_READSECTORS)&sd0_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&sd0_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&sd0_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&sd0_Shutdown
};

static bool sd1_Startup(void)
{
	return sd_Startup(1);
}
 
static bool sd1_Shutdown(void)
{
	return sd_Shutdown(1);
}
 
static bool sd1_ReadSectors(sec_t sector, sec_t numSectors,void* buffer)
{
	return sd_ReadSectors(1, sector, numSectors, buffer);
}
 
static bool sd1_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer)
{
	return sd_WriteSectors(1, sector, numSectors, buffer);
}
 
static bool sd1_ClearStatus(void)
{
	return sd_ClearStatus(1);
}
 
static bool sd1_IsInserted(void)
{
	return (bool) ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_SD1);
}

const DISC_INTERFACE __io_sd1 = {
	DEVICE_TYPE_WII_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_SD,
	(FN_MEDIUM_STARTUP)&sd1_Startup,
	(FN_MEDIUM_ISINSERTED)&sd1_IsInserted,
	(FN_MEDIUM_READSECTORS)&sd1_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&sd1_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&sd1_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&sd1_Shutdown
};

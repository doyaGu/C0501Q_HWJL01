#include "disc_io.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DEVICE_TYPE_WII_SD (('W'<<24)|('I'<<16)|('S'<<8)|'D')
 
static int __win32_initialized = 0;
static HANDLE file;

static bool win32_Startup()
{
	if(__win32_initialized==1) return true;

    file = CreateFile("D:/temp/ntfs_sd.img", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	__win32_initialized = 1;
	return true;
}
 
static bool win32_Shutdown()
{
	if(__win32_initialized==0) return false;

    CloseHandle(file);
 
	__win32_initialized = 0;
	return true;
}
 
static bool win32_ReadSectors(sec_t sector, sec_t numSectors,void* buffer)
{
	bool ret;
    DWORD value;
 
	if(buffer==NULL) return false;
 
    value = SetFilePointer(file, sector * 512, NULL, FILE_BEGIN);

    ret = ReadFile(file, buffer, numSectors * 512, &value, NULL);
 
	return ret;
}
 
static bool win32_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer)
{
	bool ret;
    DWORD value;
 
	if(buffer==NULL) return false;
 
    value = SetFilePointer(file, sector * 512, NULL, FILE_BEGIN);

    ret = WriteFile(file, buffer, numSectors * 512, &value, NULL);
 
	return ret;
}
 
static bool win32_ClearStatus()
{
	return true;
}
 
static bool win32_IsInserted()
{
    return file ? true : false;
}

static bool win32_IsInitialized()
{
    return file ? true : false;
}

const DISC_INTERFACE __io_win32 = {
	DEVICE_TYPE_WII_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_WII_SD,
	(FN_MEDIUM_STARTUP)&win32_Startup,
	(FN_MEDIUM_ISINSERTED)&win32_IsInserted,
	(FN_MEDIUM_READSECTORS)&win32_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&win32_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&win32_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&win32_Shutdown
};

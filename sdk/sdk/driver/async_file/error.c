#include "async_file/config.h"
#include "async_file/pal.h"

MMP_UINT32  enable_DbgMsgFlag = 0x3; // default: enable ERROR and INFO
MMP_UINT32  enable_SdkMsgFlag = 0x3;

void
PalAssertFail(
	const MMP_CHAR* exp,
	const MMP_CHAR* file,
	MMP_UINT line)
{
    printf("Failed assertion: %s (in %s, line %u)\r\n", exp, file, line);
    while (1);
}

MMP_CHAR*
PalGetErrorString(
    MMP_INT errnum)
{
    return MMP_NULL;
}

void
PalExit(
    MMP_INT status)
{
    printf("Program exit with status code: %d\r\n", status);
    while(1);
}

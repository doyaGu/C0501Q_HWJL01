#include "async_file/config.h"
#include "async_file/init.h"

static MMP_BOOL inited;

MMP_INT
PalInitialize(
    void)
{
    MMP_INT result = 0;

    LOG_ENTER "PalInitialize()\r\n" LOG_END

    if (inited)
        goto end; // initialized before

    //result = PalThreadInitialize();
    //if (result)
    //    goto end;

    result = PalFileInitialize();
    if (result)
        goto end;

    result = PalMsgQInitialize();
    if (result)
        goto end;


             
    inited = MMP_TRUE;

end:
    LOG_LEAVE "PalInitialize()=%d\r\n", result LOG_END
    printf("result = %d\n",result);
    return result;
}

MMP_INT
PalTerminate(
    void)
{
    MMP_INT result = 0;

    LOG_ENTER "PalTerminate()\r\n" LOG_END

    if (!inited)
        goto end; // not initialized yet

    result = PalMsgQTerminate();
    if (result)
        goto end;

    result = PalFileTerminate();
    if (result)
        goto end;
   
   
    //result = PalThreadTerminate();
    //if (result)
    //    goto end;

    inited = MMP_FALSE;

end:
    LOG_LEAVE "PalTerminate()=%d\r\n", result LOG_END
    printf("result = %d\n",result);
    return result;
}

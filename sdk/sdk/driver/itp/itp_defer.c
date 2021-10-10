/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Deferred Interrupt Handling functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "itp_cfg.h"

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif // __OPENRTOS__

void itpPendFunctionCallFromISR(ITPPendFunction func, void* arg1, uint32_t arg2)
{
#if defined(__OPENRTOS__)

    #if (INCLUDE_xTimerPendFunctionCall == 1)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* The actual processing is to be deferred to a task.  Request the
    func() callback function is executed, passing in the
    number of the interface that needs processing.  The interface to
    service is passed in the second parameter.  The first parameter is
    not used in this case. */
    xTimerPendFunctionCallFromISR( func, arg1, arg2, &xHigherPriorityTaskWoken );

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
    switch should be requested.  The macro used is port specific and will
    be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
    the documentation page for the port being used. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    #else
    func(arg1, arg2);

    #endif

#else
    func(arg1, arg2);
    
#endif // __OPENRTOS__
}

#ifndef ITE_ITH_CMDQ_H
#define ITE_ITH_CMDQ_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Initializes command queue
*
* @param cmdQ the command queue instance
*/
void ithCmdQInit(ITHCmdQ* cmdQ);

/**
* Terminates command queue
*/
void ithCmdQExit(void);

/**
* Resets command queue hardware
*/
void ithCmdQReset(void);

/**
* Waits specified command queue size.
*
* @param size the specified size to wait.
*/
uint32_t* ithCmdQWaitSize(uint32_t size);

/**
* Flushes command queue buffer.
*
* @param ptr the end of command queue buffer.
*/
void ithCmdQFlush(uint32_t* ptr);

/**
* Hardware flip LCD.
*
* @param index the index to flip. 0 is A, 1 is B, 2 is C.
*/
void ithCmdQFlip(unsigned int index);

/**
* Waits command queue empty.
*/
int ithCmdQWaitEmpty(void);

/**
* Waits the interrupt of command queue.
*/
int ithCmdQWaitInterrupt(void);

/**
* Enable command queue clock.
*/
void ithCmdQEnableClock(void);

/**
* Disable command queue clock.
*/
void ithCmdQDisableClock(void);

/**
* Enables specified command queue controls.
*
* @param ctrl the controls to enable.
*/
void ithCmdQCtrlEnable(ITHCmdQCtrl ctrl);

/**
* Disables specified command queue controls.
*
* @param ctrl the controls to disable.
*/
void ithCmdQCtrlDisable(ITHCmdQCtrl ctrl);

/**
* Clears the interrupt of command queue.
*/
void ithCmdQClearIntr(void);

#ifdef __cplusplus
}
#endif

#endif

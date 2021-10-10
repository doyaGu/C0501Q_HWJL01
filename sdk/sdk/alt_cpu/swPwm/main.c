#include <stdarg.h>

#include "alt_cpu/swPwm/swPwm.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

typedef struct
{
    uint32_t bValid;
    uint32_t bRun;
    uint32_t pwmGpio;
    uint32_t tickPerCycle;
    uint32_t highTickCount;
    uint32_t lowTickCount;
    uint32_t curTime;
    uint32_t checkTickCount;
    uint32_t gpioValue;
} SW_PWM_HANDLE;

static SW_PWM_HANDLE gptPwmHandle[SW_PWM_COUNT] = { 0 };

static void swPwmProcessInitCmd(void)
{
    SW_PWM_INIT_DATA* pInitData = (SW_PWM_INIT_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint8_t* pAddr = (uint8_t*)pInitData;
    uint32_t pwmId = ENDIAN_SWAP32(pInitData->pwmId);
    uint32_t cpuClk = ENDIAN_SWAP32(pInitData->cpuClock);
    uint32_t pwmClk = ENDIAN_SWAP32(pInitData->pwmClk);
    uint32_t dutyCyclye = ENDIAN_SWAP32(pInitData->pwmDutyCycle);
    uint32_t tickPerUs = 0;
    SW_PWM_HANDLE *ptSwPwmHandle = 0;


    if (pwmId >= SW_PWM0 && pwmId < SW_PWM_COUNT && cpuClk && pwmClk && dutyCyclye > 0 && dutyCyclye < 100)
    {
        ptSwPwmHandle = &gptPwmHandle[pwmId];
        tickPerUs = cpuClk / (1000 * 1000);
        ptSwPwmHandle->pwmGpio = ENDIAN_SWAP32(pInitData->pwmGpio);
        ptSwPwmHandle->tickPerCycle = ((1000 * 1000) / pwmClk) * tickPerUs;

        ptSwPwmHandle->highTickCount = ptSwPwmHandle->tickPerCycle * dutyCyclye / 100;
        ptSwPwmHandle->lowTickCount = ptSwPwmHandle->tickPerCycle - ptSwPwmHandle->highTickCount;

        ptSwPwmHandle->bValid = 1;
        ptSwPwmHandle->bRun = 0;
        ptSwPwmHandle->curTime = 0;
    }
}

static void swPwmProcessRunCmd(void)
{
    SW_PWM_RUN_CMD_DATA* pRunCmd = (SW_PWM_RUN_CMD_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t pwmId = ENDIAN_SWAP32(pRunCmd->pwmId);  
    SW_PWM_HANDLE *ptSwPwmHandle = 0;
    
    if (pwmId >= SW_PWM0 && pwmId < SW_PWM_COUNT)
    {
        ptSwPwmHandle = &gptPwmHandle[pwmId];
        if (ptSwPwmHandle->bValid)
        {
            //Set Gpio to Output mode            
            setGpioMode(ptSwPwmHandle->pwmGpio, 0);
            setGpioDir(ptSwPwmHandle->pwmGpio, 0);
            ptSwPwmHandle->curTime = ptSwPwmHandle->checkTickCount = getCurTimer(0);
            ptSwPwmHandle->gpioValue = 0;
            ptSwPwmHandle->bRun = 1;
        }
    }
}

static void swPwmProcessStopCmd(void)
{
    SW_PWM_STOP_CMD_DATA* pStopCmd = (SW_PWM_STOP_CMD_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t pwmId = ENDIAN_SWAP32(pStopCmd->pwmId);  
    SW_PWM_HANDLE *ptSwPwmHandle = 0;
    
    if (pwmId >= SW_PWM0 && pwmId < SW_PWM_COUNT)
    {
        ptSwPwmHandle = &gptPwmHandle[pwmId];
        if (ptSwPwmHandle->bValid && ptSwPwmHandle->bRun)
        {
            //Set Gpio to Input mode
            setGpioDir(ptSwPwmHandle->pwmGpio, 1);
            ptSwPwmHandle->bRun = 0;
        }
    }
}

static void swPwmProcessUpdateDutyCycleCmd(void)
{
    SW_PWM_UPDATE_DUTY_CYCLE_DATA* pDutyCcycleCmd = (SW_PWM_UPDATE_DUTY_CYCLE_DATA*) CMD_DATA_BUFFER_OFFSET;
    uint32_t pwmId = ENDIAN_SWAP32(pDutyCcycleCmd->pwmId);
    uint32_t newDuty = ENDIAN_SWAP32(pDutyCcycleCmd->pwmDutyCycle);
    SW_PWM_HANDLE *ptSwPwmHandle = 0;

    if (pwmId >= SW_PWM0 && pwmId < SW_PWM_COUNT)
    {
        ptSwPwmHandle = &gptPwmHandle[pwmId];
        if (ptSwPwmHandle->bValid)
        {
            ptSwPwmHandle->highTickCount = ptSwPwmHandle->tickPerCycle * newDuty / 100;
            ptSwPwmHandle->lowTickCount = ptSwPwmHandle->tickPerCycle - ptSwPwmHandle->highTickCount;
        }
    }
}

extern unsigned long gTickTestCounter;
static void swPwmGenerateClk(void)
{
    int pwmId = 0;
    for (pwmId = SW_PWM0; pwmId < SW_PWM_COUNT; pwmId++)
    {
        SW_PWM_HANDLE *ptSwPwmHandle = &gptPwmHandle[pwmId];
        if (ptSwPwmHandle->bRun)
        {
            if (getDuration(0, ptSwPwmHandle->curTime) >= ptSwPwmHandle->checkTickCount)
            {
                if (ptSwPwmHandle->gpioValue)
                {
                    if (ptSwPwmHandle->lowTickCount)
                    {
                        setGpioValue(ptSwPwmHandle->pwmGpio, 0);
                    }
                    ptSwPwmHandle->curTime = getCurTimer(0);
                    ptSwPwmHandle->checkTickCount = ptSwPwmHandle->lowTickCount;
                    ptSwPwmHandle->gpioValue = 0;
                }
                else
                {
                    if (ptSwPwmHandle->highTickCount)
                    {
                        setGpioValue(ptSwPwmHandle->pwmGpio, 1);
                    }
                    ptSwPwmHandle->curTime = getCurTimer(0);
                    ptSwPwmHandle->checkTickCount = ptSwPwmHandle->highTickCount;
                    ptSwPwmHandle->gpioValue = 1;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    //Set GPIO and Clock Setting
    int inputCmd = 0;

    //Start Timer
    startTimer(0);

    while(1)
    {
        inputCmd = ithReadRegH(REQUEST_CMD_REG);
        if (inputCmd && ithReadRegH(RESPONSE_CMD_REG) == 0)
        {
            switch(inputCmd)
            {
                case INIT_CMD_ID:
                    swPwmProcessInitCmd();
                    break;
                case RUN_CMD_ID:
                    swPwmProcessRunCmd();
                    break;
                case STOP_CMD_ID:
                    swPwmProcessStopCmd();
                    break;
                case UPDATE_DUTY_CYCLE_CMD_ID:
                    swPwmProcessUpdateDutyCycleCmd();
                    break;
                default:
                    break;
            }
            ithWriteRegH(RESPONSE_CMD_REG, (uint16_t) inputCmd);
        }
        swPwmGenerateClk();
    }
}

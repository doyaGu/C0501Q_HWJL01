/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL WIN32 functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <stdio.h>
#include "ith_cfg.h"
#include "FTCSPI.h"

#define VMEM_START  (64) // keep 0 for NULL

#ifdef CFG_LCD_MULTIPLE
    #define VMEM_SIZE   (CFG_RAM_SIZE - CFG_CMDQ_SIZE)
#else
    #define VMEM_SIZE   (CFG_RAM_SIZE - CFG_LCD_WIDTH * CFG_LCD_HEIGHT * CFG_LCD_BPP * 2 - CFG_CMDQ_SIZE)
#endif

static ITHVmem vmem;
static ITHCmdQ cmdQ;
static void*   dmaMutex;
void* ithStorMutex = NULL;
static CRITICAL_SECTION    CriticalSection;


void __cdecl _assert(void *, void *, unsigned);
int SpiOpen(DWORD dwClockRate);

static const uint32_t ramScript[] =
{
#include "ram.inc"
};

enum {
    WAIT_CMD      = 0xffffffff,
    DATA_CMD      = 0xfffffffe,
    DATA_WAIT0_CMD= 0xfffffffd,
    DATA_WAIT1_CMD= 0xfffffffc,
    CALL_CMD      = 0xfffffffb,
    WRITE_MASK_CMD= 0xfffffffa,
    GOTO_CMD      = 0xfffffff9,
    READ_MASK_CMD = 0xfffffff8,
    SKIP_CMD      = 0xfffffff7,
    BEQ_CMD       = 0xffffffe0,
    BNE_CMD       = 0xffffffe1,
    BGT_CMD       = 0xffffffe2,
    BGTE_CMD      = 0xffffffe3,
    BLT_CMD       = 0xffffffe4,
    BLTE_CMD      = 0xffffffe5,
};

static char* GetErrorString(int errnum)
{
    static TCHAR buf[256];

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errnum,
		0,
        buf,
        256,
        NULL);

    return buf;
}

static void RunScript(const uint32_t* cmd, unsigned int cmd_len)
{
    unsigned int idx = 0;

    while (idx < cmd_len)
    {
        int rval = 0;

        switch (cmd[idx])
        {
        case WAIT_CMD: // WAIT(n)
            {
                int n = cmd[idx+1];
                int delay;

                LOG_DBG "WAIT(%d);\n", n LOG_END

                #if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
                    delay = n;      // n is ms

                #else
                    delay = (n+999)/1000; // n is us

                #endif // (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)

                if (delay == 0)
                    delay = 1;

                Sleep(delay);   // delay ms
            }
            idx += 2;
            break;

        case DATA_CMD: // DATA(base, height, width, pitch)
            {
                int base   = cmd[idx+1];
                int height = cmd[idx+2];
                int width  = cmd[idx+3];
                int pitch  = cmd[idx+4];
                char *src = (char*)cmd[idx+5];

                // memcpy from src to base for height*width with pitch
                int i;
                for (i = 0; i < height; i++)
                    ithWriteVram(base + pitch * i, src + width * i, width);

                idx += (height*width) / sizeof(int);
            }
            idx+=5;
            break;

        case DATA_WAIT0_CMD: // DATA_WAIT0(addr, mask)
            {
                int addr = cmd[idx+1];
                int mask = cmd[idx+2];
                int data;

            #if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
                if ((addr & 0xffff0000) != 0)
                {
                     LOG_ERR "Do not support AHB address access for command DATA_WAIT0(0x%04x, 0x%04x);\n", addr, mask LOG_END
                     break;
                }

                do
                {
                    data = ithReadRegH(addr);
                } while(data & mask);

            #else
                do
                {
                    data = ithReadRegA(addr);
                } while(data & mask);

            #endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                LOG_DBG "DATA_WAIT0(0x%04x, 0x%04x);\n", addr, mask LOG_END
            }
            idx+=3;
            break;

        case DATA_WAIT1_CMD: // DATA_WAIT1(addr, mask)
            {
                int addr = cmd[idx+1];
                int mask = cmd[idx+2];
                int data;

            #if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
                if ((addr & 0xffff0000) != 0)
                {
                    LOG_ERR "Do not support AHB address access for command DATA_WAIT1(0x%04x, 0x%04x);\n", addr, mask LOG_END
                    break;
                }

                do
                {
                    data = ithReadRegH(addr);
                } while(!(data & mask));

            #else
                do
                {
                    data = ithReadRegA(addr);
                } while(!(data & mask));

            #endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                LOG_DBG "DATA_WAIT1(0x%04x, 0x%04x);\n", addr, mask LOG_END
            }
            idx+=3;
            break;

        case CALL_CMD: // CALL(addr)
            {
                int addr = cmd[idx+1];
                LOG_DBG "Do not support command CALL(0x%08x);\n", addr LOG_END
            }
            idx+=2;
            break;

        case WRITE_MASK_CMD: // WRITE_MASK(addr, data, mask)
            {
                int addr = cmd[idx+1];
                int val  = cmd[idx+2];
                int mask = cmd[idx+3];
                int data;

            #if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                if ((addr & 0xffff0000) != 0)
                {
                    LOG_ERR "Do not support AHB address access for command WRITE_MASK(0x%04x, 0x%04x, 0x%04x);\n", addr, val, mask LOG_END
                    break;
                }

                data = ithReadRegH(addr);
                ithWriteRegH(addr, (data & mask) | (val & ~mask));

            #else
                data = ithReadRegA(addr);
                ithWriteRegA(addr, (data & mask) | (val & ~mask));

            #endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                LOG_DBG "WRITE_MASK(0x%04x, 0x%04x, 0x%04x);\n", addr, val, mask LOG_END
            }
            idx+=4;
            break;

        case GOTO_CMD: // GOTO(addr)
            {
                int addr = cmd[idx+1];
                LOG_ERR "Do not support command GOTO(0x%08x);\n", addr LOG_END
            }
            idx+=2;
            break;

        case READ_MASK_CMD: // READ_MASK(addr, mask)
            {
                int addr = cmd[idx+1];
                int mask = cmd[idx+2];
                int data;

            #if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850
                if ((addr & 0xffff0000) != 0)
                {
                    LOG_ERR "Do not support AHB address access for command READ_MASK(0x%04x, 0x%04x);\n", addr, mask LOG_END
                    break;
                }

                data = ithReadRegH(addr);
                rval = data & mask;

            #else
                data = ithReadRegA(addr);
                rval = data & mask;

            #endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                LOG_DBG "READ_MASK(0x%04x, 0x%04x);\n", addr, mask LOG_END
            }
            idx+=3;
            break;

        case SKIP_CMD: // SKIP(offset)
            {
                int offset = cmd[idx+1];
                idx += offset;
                LOG_DBG "SKIP(0x%04x);\n", offset LOG_END
            }
            idx += 2;
            break;

        case BEQ_CMD: // BEQ(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval == (unsigned)val) idx += offset;
                LOG_DBG "BEQ(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        case BNE_CMD: // BNE(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval != (unsigned)val) idx += offset;
                LOG_DBG "BNE(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        case BGT_CMD: // BGT(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval > (unsigned)val) idx += offset;
                LOG_DBG "BGT(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        case BGTE_CMD: // BGTE(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval >= (unsigned)val) idx += offset;
                LOG_DBG "BGTE(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        case BLT_CMD: // BLT(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval < (unsigned)val) idx += offset;
                LOG_DBG "BLT(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        case BLTE_CMD: // BLTE(val, offset)
            {
                int val = cmd[idx+1];
                int offset = cmd[idx+2];
                if ((unsigned)rval <= (unsigned)val) idx += offset;
                LOG_DBG "BLTE(0x%04x, 0x%04x);\n", val, offset LOG_END
            }
            idx += 3;
            break;

        default: // WRITE(addr, data)
            {
                int addr=cmd[idx];
                int data=cmd[idx+1];

            #if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                if ((addr & 0xffff0000) != 0)
                {
                    LOG_ERR "Do not support AHB address access for command WRITE(0x%04x, 0x%04x);\n", addr, data LOG_END
                    break;
                }

                ithWriteRegH(addr, data);

            #else
                ithWriteRegA(addr, data);

            #endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

                LOG_DBG "WRITE(0x%04x, 0x%04x);\n", addr, data LOG_END
            }
            idx += 2;
            break;
        }
    }
}

static void GpioIntrHandler(void* arg)
{
    ithGpioDoIntr();
}

void ithInit(void)
{
    const ITHCardConfig cardCfg =
    {
        {
            // card detect pin of sd0
    #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_CARD_DETECT,
    #elif defined(CFG_SD0_STATIC)
            (uint8_t)-2,  // always insert
    #else
            (uint8_t)-1,
    #endif

            // card detect pin of sd1
    #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_CARD_DETECT,
    #elif defined(CFG_SD1_STATIC)
            (uint8_t)-2,  // always insert
    #else
            (uint8_t)-1,
    #endif
        },

        {
            // power enable pin of sd0
    #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_POWER_ENABLE,
    #else
            (uint8_t)-1,
    #endif

            // power enable pin of sd1
    #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_POWER_ENABLE,
    #else
            (uint8_t)-1,
    #endif
        },

        {
            // write protect pin of sd0
    #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_WRITE_PROTECT,
    #else
            (uint8_t)-1,
    #endif

            // write protect pin of sd1
    #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_WRITE_PROTECT,
    #else
            (uint8_t)-1,
    #endif
        },

		{
    #if defined(CFG_GPIO_SD0_IO)
			CFG_GPIO_SD0_IO,
	#else
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	#endif
		},

		{
    #if defined(CFG_GPIO_SD1_IO)
			CFG_GPIO_SD1_IO,
	#else
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	#endif
		},
    };

    // init critical section
	InitializeCriticalSection(&CriticalSection);

	// init usb-spi
	SpiOpen(2);

	// init ram setting
	RunScript(ramScript, ITH_COUNT_OF(ramScript));

    ithClockStats();

    // init video memory management
    vmem.startAddr      = VMEM_START;
    vmem.totalSize      = VMEM_SIZE;
    vmem.mutex          = CreateMutex(NULL, FALSE, NULL);
    vmem.usedMcbCount = vmem.freeSize = 0;

    ithVmemInit(&vmem);

    // disable isp clock for power saving
    ithIspDisableClock();

    // init gpio
    ithGpioInit();
    ithGpioSetDebounceClock(800);
    ithIntrDisableIrq(ITH_INTR_GPIO);
    ithIntrClearIrq(ITH_INTR_GPIO);
    ithIntrRegisterHandlerIrq(ITH_INTR_GPIO, GpioIntrHandler, NULL);
    ithGpioEnableClock();

    // init dma controller
    dmaMutex = CreateMutex(NULL, FALSE, NULL);
    ithDmaInit(dmaMutex);

    // recover AHB0 control register value
    ithAhb0SetCtrlReg();

    // init card
#if defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE)
    ithCardInit(&cardCfg);
    // storage need always power on for pin share issue
    ithCardPowerOn(ITH_CARDPIN_SD0);
    ithCardPowerOn(ITH_CARDPIN_SD1);
#endif // defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE)

    // create mutex for storage pin share 
    ithStorMutex = CreateMutex(NULL, FALSE, NULL);


#ifdef CFG_CMDQ_ENABLE
    // init command queue
    cmdQ.size   = CFG_CMDQ_SIZE;
    cmdQ.addr   = CFG_RAM_SIZE - CFG_CMDQ_SIZE;
    cmdQ.mutex  = CreateMutex(NULL, FALSE, NULL);

    ithCmdQInit(&cmdQ);
	ithCmdQReset();
#endif // CFG_CMDQ_ENABLE

    // init stc
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)    
#ifdef CFG_VIDEO_ENABLE
    ithFpcReset();
#endif
#endif
}

void ithAssertFail(const char* exp, const char* file, int line, const char* func)
{
	_assert((void*) exp, (void*) file, line);
}

void ithDelay(unsigned long us)
{
    LARGE_INTEGER freq, start, curr, setting;

    QueryPerformanceFrequency(&freq);
    setting.QuadPart = (LONGLONG) (freq.QuadPart * us * 0.000001f);
    QueryPerformanceCounter(&start);

    do
    {
        QueryPerformanceCounter(&curr);

    } while ((curr.QuadPart - start.QuadPart) < setting.QuadPart);
}

void ithLockMutex(void* mutex)
{
    if (WaitForSingleObject(mutex, INFINITE))
        LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
}

void ithUnlockMutex(void* mutex)
{
    if (ReleaseMutex(mutex) == 0)
        LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
}

#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910))
static uint16_t Ahb2HostAddr(uint32_t addr)
{
    switch (addr & 0xFFF00000)
    {
    case 0xC0000000:
        addr -= 0xC0000000;
        break;

    case 0xD0000000:
        addr -= 0xD0000000;
        addr += 0x5400;
        break;

    case 0xD0100000:
        addr -= 0xD0010000;
        addr += 0x5800;
        break;

    case 0xD0200000:
        addr -= 0xD0200000;
        addr += 0x5C00;
        break;

    case 0xD0300000:
        addr -= 0xD0300000;
        addr += 0x6000;
        break;

    case 0xD0400000:
        addr -= 0xD0400000;
        addr += 0x6400;
        break;

    case 0xD0500000:
        addr -= 0xD0500000;
        addr += 0x6800;
        break;

    case 0xD0600000:
        addr -= 0xD0600000;
        addr += 0x6C00;
        break;

    case 0xD0700000:
        addr -= 0xD0700000;
        addr += 0x7000;
        break;

    case 0xD0800000:
        addr -= 0xD0800000;
        addr += 0x7400;
        break;

    case 0xD0900000:
        addr -= 0xD0900000;
        addr += 0x7800;
        break;

    case 0xDE000000:
        addr -= 0xDE000000;
        addr += 0x7C00;
        break;

    case 0xDE100000:
        addr -= 0xDE100000;
        addr += 0x8000;
        break;

    case 0xDE200000:
        addr -= 0xDE200000;
        addr += 0x8400;
        break;

    case 0xDE300000:
        addr -= 0xDE300000;
        addr += 0x8B00;
        break;

    case 0xDE400000:
        addr -= 0xDE400000;
        addr += 0x8C00;
        break;

    case 0xDE500000:
        addr -= 0xDE500000;
        addr += 0x9000;
        break;

    case 0xDE600000:
        addr -= 0xDE600000;
        addr += 0x9400;
        break;

    case 0xDE700000:
        addr -= 0xDE700000;
        addr += 0x9B00;
        break;

    case 0xDE800000:
        addr -= 0xDE800000;
        addr += 0x9C00;
        break;

    case 0xDE900000:
        addr -= 0xDE900000;
        addr += 0xA000;
        break;

    case 0xDEA00000:
        addr -= 0xDEA00000;
        addr += 0xA400;
        break;

    case 0xDEB00000:
        addr -= 0xDEB00000;
        addr += 0xA800;
        break;

    case 0xDEC00000:
        addr -= 0xDEC00000;
        addr += 0xAC00;
        break;

    case 0xDED00000:
        addr -= 0xDED00000;
        addr += 0xB000;
        break;

    case 0xDEE00000:
        addr -= 0xDEE00000;
        addr += 0xB000;
        break;

    default:
        ASSERT(0);
    }
    return (uint16_t) addr;
}
#else
static uint16_t Ahb2HostAddr(uint32_t addr)
{
    switch (addr & 0xFFF00000)
    {
    case 0xC0000000:
        addr -= 0xC0000000;
        break;

    case 0xD0000000:
        addr -= 0xD0000000;
        addr += 0x5400;
        break;

    case 0xD0100000:
        addr -= 0xD0010000;
        addr += 0x5800;
        break;

    case 0xD0200000:
        addr -= 0xD0200000;
        addr += 0x5C00;
        break;

    case 0xD0300000:
        addr -= 0xD0300000;
        addr += 0x6000;
        break;

    case 0xD0400000:
        addr -= 0xD0400000;
        addr += 0x6400;
        break;

    case 0xD0500000:
        addr -= 0xD0500000;
        addr += 0x6800;
        break;

    case 0xD0600000:
        addr -= 0xD0600000;
        addr += 0x6C00;
        break;

    case 0xD0700000:
        addr -= 0xD0700000;
        addr += 0x7000;
        break;

    case 0xD0800000:
        addr -= 0xD0800000;
        addr += 0x7400;
        break;

    case 0xD0900000:
        addr -= 0xD0900000;
        addr += 0x7800;
        break;

    case 0xD0A00000:
        addr -= 0xD0A00000;
        addr += 0x7C00;
        break;

    case 0xDE000000:
        addr -= 0xDE000000;
        addr += 0x8000;
        break;

    case 0xDE100000:
        addr -= 0xDE100000;
        addr += 0x8400;
        break;

    case 0xDE200000:
        addr -= 0xDE200000;
        addr += 0x8800;
        break;

    case 0xDE300000:
        addr -= 0xDE300000;
        addr += 0x8C00;
        break;

    case 0xDE400000:
        addr -= 0xDE400000;
        addr += 0x9000;
        break;

    case 0xDE500000:
        addr -= 0xDE500000;
        addr += 0x9400;
        break;

    case 0xDE600000:
        addr -= 0xDE600000;
        addr += 0x9800;
        break;

    case 0xDE700000:
        addr -= 0xDE700000;
        addr += 0x9C00;
        break;

    case 0xDE800000:
        addr -= 0xDE800000;
        addr += 0xA000;
        break;

    case 0xDE900000:
        addr -= 0xDE900000;
        addr += 0xA400;
        break;

    case 0xDEA00000:
        addr -= 0xDEA00000;
        addr += 0xA800;
        break;

    case 0xDEB00000:
        addr -= 0xDEB00000;
        addr += 0xAC00;
        break;

    case 0xDEC00000:
        addr -= 0xDEC00000;
        addr += 0xB000;
        break;

    default:
        ASSERT(0);
    }
    return (uint16_t) addr;
}
#endif

#if CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

uint32_t ithReadRegA(uint32_t addr)
{
    uint16_t hostAddr = Ahb2HostAddr(addr);
    uint32_t dataHi = ithReadRegH(hostAddr + 2);
	uint32_t dataLo = ithReadRegH(hostAddr);

	return (dataHi << 16) | dataLo;
}

void ithWriteRegA(uint32_t addr, uint32_t data)
{
	uint16_t dataHi = (uint16_t) (data >> 16);
	uint16_t dataLo = (uint16_t) (data & 0xFFFF);
    uint16_t hostAddr = Ahb2HostAddr(addr);

	ithWriteRegH(hostAddr + 2, dataHi);
	ithWriteRegH(hostAddr, dataLo);
}
#endif // CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850

void ithFlushDCache(void)
{
}

void ithInvalidateDCache(void)
{
}

void ithInvalidateDCacheRange(void* addr, uint32_t size)
{
}

void ithInvalidateICache(void)
{
}

void ithCpuDoze(void)
{
}

void ithFlushMemBuffer(void)
{
}

void ithFlushAhbWrap(void)
{
}

void ithYieldFromISR(bool yield)
{
}

void ithEnterCritical(void)
{
    EnterCriticalSection(&CriticalSection);
}

void ithExitCritical(void)
{
    LeaveCriticalSection(&CriticalSection);
}

void ithTaskYield(void)
{
    Sleep(0);
}

ITHCpuMode ithGetCpuMode(void)
{
    return ITH_CPU_SYS;
}

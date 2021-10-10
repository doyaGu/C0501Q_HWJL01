//=============================================================================
//                              Include Files
//=============================================================================
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "ssp/mmp_spi.h"
#include "../../include/ssp/ssp_reg.h"
#include "../../include/ssp/ssp_error.h"
#include "ite/ith.h"
#include "../../include/ite/itp.h"
#include "ite/ith.h"
#if defined (__OPENRTOS__)
#elif defined (__FREERTOS__)
#include "or32.h"
#endif

//=============================================================================
//                              Macro
//=============================================================================
#define SWAP_ENDIAN16(value) \
    ((((value) >> 8) & 0x00FF) | \
    (((value) << 8) & 0xFF00))

#define SWAP_ENDIAN32(value) \
    ((((value) >> 24) & 0x000000FF) | \
     (((value) >>  8) & 0x0000FF00) | \
     (((value) <<  8) & 0x00FF0000) | \
     (((value) << 24) & 0xFF000000))

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SSP_POLLING_COUNT			(0x10000)
#define SSP_DMA_TEMPBUF				0x100000

#define AHB_SPI1_DATA_BASE			0xDE800018
#define AHB_SPI2_DATA_BASE			0xDE900018
#define AHB_SPI_DATA_BASE			AHB_SPI1_DATA_BASE
#define FREERUN_SHIFT				16
#define SSP_DEFAULT_DIV				0
#define SSP_DEFAULT_FIFO_LENGTH		31
#define SSP_DEFAULT_RX_THRESHOLD	8
#define SSP_DEFAULT_TX_THRESHOLD    8

//=============================================================================
//                              Global Data Definition
//=============================================================================
//static bool nor_using = false;
static SPI_CONTROL_MODE ctrl_mode = SPI_CONTROL_NOR;
static pthread_mutex_t  SwitchMutex;
//static pthread_mutex_t  SpiInternalMutex  = PTHREAD_MUTEX_INITIALIZER;
static uint8_t			g_initCount[SPI_PORT_MAX];
static unsigned int 	gSpiCtrlMethod = SPI_CONTROL_SLAVE;
static unsigned int 	gShareGpioTable[SPI_SHARE_GPIO_MAX]={0};

#ifdef SPI_USE_DMA
static int              DmaSpiChannel = 0;
static int              DmaSpiSlaveChannel = 0;
#ifdef SPI_USE_DMA_INTR
static sem_t			SpiDmaMutex;
#endif
#endif
static SPI_CONTEXT	g_SpiContext[SPI_PORT_MAX] = 
{
	{
		0,							// refCount
		PTHREAD_MUTEX_INITIALIZER	// mutex
	},
	{
		0,							// refCount
		PTHREAD_MUTEX_INITIALIZER	// mutex
	}
};
#ifdef SPI_USE_DMA
/*
static MMP_DMA_CONTEXT	g_spiTxDmaCtxt;
static MMP_DMA_CONTEXT	g_spiRxDmaCtxt;

static uint32_t spidmaReadAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE,			(uint32_t)MMP_DMA_TYPE_SPI2_TO_MEM,
    MMP_DMA_ATTRIB_SRC_ADDR,			AHB_SPI_DATA_BASE,        //FOR WIN32
    MMP_DMA_ATTRIB_DST_ADDR,			(uint32_t)0,
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE,	(uint32_t)1024,//TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING,		(uint32_t)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH,		1, //TODO
	MMP_DMA_ATTRIB_DST_TX_WIDTH,		1,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE,		1,
	MMP_DMA_ATTRIB_FIFO_TH,				4,
    MMP_DMA_ATTRIB_NONE
};

static uint32_t spidmaWriteAttrib[] =
{
    MMP_DMA_ATTRIB_DMA_TYPE, (uint32_t)MMP_DMA_TYPE_MEM_TO_SPI2,
    MMP_DMA_ATTRIB_SRC_ADDR, (uint32_t)0,
    MMP_DMA_ATTRIB_DST_ADDR, AHB_SPI_DATA_BASE,    //FOR WIN32
    MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (uint32_t)1024,//TODO
    MMP_DMA_ATTRIB_HW_HANDSHAKING, (uint32_t)MMP_TRUE,
    MMP_DMA_ATTRIB_SRC_TX_WIDTH, 1, //TODO
	MMP_DMA_ATTRIB_DST_TX_WIDTH, 1,
    MMP_DMA_ATTRIB_SRC_BURST_SIZE, 8,
    MMP_DMA_ATTRIB_NONE
};
*/
#endif

typedef enum _FIFO_READY
{
    FIFO_RX,
    FIFO_TX
}FIFO_READY;

//=============================================================================
//                              Private Function Definition
//=============================================================================
#ifdef SPI_USE_DMA_INTR
static void
SpiDmaIntr(
	int			ch,
	void*		arg,
	uint32_t	int_status)
{
	itpSemPostFromISR(&SpiDmaMutex);
}
#endif

static float
_GetWclkClock()
{
	uint16_t PLL1_numerator = ithReadRegH(0x00A4) & 0x3FF;
	uint16_t reg_1C = ithReadRegH(0x001C);
	uint16_t reg_A2 = ithReadRegH(0x00A2);
	uint16_t wclkRatio = reg_1C & 0x3FF;
	uint16_t clockSrc = (reg_1C & 0x3800) >> 11;
	uint16_t regValue, pllDivider;
	float    wclk_value = 0;

	switch ( clockSrc )
	{
	case 0:	// From PLL1 output1
		regValue = ithReadRegH(0x00A2);
		pllDivider = regValue & 0x007F;
		break;

	case 1: // From PLL1 output2
		regValue = ithReadRegH(0x00A2);
		pllDivider = (regValue & 0x7F00) >> 8;
		break;

	case 2: // From PLL2 output1
	case 3:	// From PLL2 output2
	case 4: // From PLL3 output1
	case 5: // From PLL3 output2
	case 6: // From CKSYS (12MHz)
	case 7: // From Ring OSC (200KHz)
		printf("[NOR][%d] ERROR!\n", __LINE__);
		assert(0);
		break;
	}

	//printf("PLL1_numerator = %d, wclkRatio = %d, pllDivider = %d\n", PLL1_numerator, wclkRatio, pllDivider);
	wclk_value = (float)PLL1_numerator / (wclkRatio + 1) / pllDivider;
	//printf("WCLK clock = %f\n", wclk_value);
	return wclk_value;
}

static void
_SwitchSpiGpioPin(
	SPI_CONTROL_MODE controlMode)
{
	int pin_ori = gShareGpioTable[ctrl_mode];
	int pin_dst = gShareGpioTable[controlMode];
	
   	if(pin_ori && pin_dst)
   	{
   		if(gSpiCtrlMethod==SPI_CONTROL_SLAVE)
        {
   		if(pin_ori<32)	ithWriteRegMaskA(ITH_GPIO_BASE + 0x00, (0x1 << pin_ori), (0x1 << pin_ori));
   		else			ithWriteRegMaskA(ITH_GPIO_BASE + 0x40, (0x1 << (pin_ori-32)), (0x1 << (pin_ori-32)));
   		ithGpioSetOut(pin_ori);
   		
   		ithGpioClear(pin_dst);
   		ithGpioSetIn(pin_dst);
   		
   		usleep(10);   		
   		}
   		else
   		{
	   		ithGpioSet(pin_ori);    	ithGpioSetOut(pin_ori);
	   		ithGpioClear(pin_dst);    	ithGpioSetOut(pin_dst);
	   		usleep(10);
	   		
	        while(1)
	        {
	        	if( !ithGpioGet(pin_dst) && ithGpioGet(pin_ori) )	break;
	        }	   		
   		}
    }
}

static void
Patch_Sleep()
{
	const unsigned long SLEEP_TIME = 2000;

	ithDelay(SLEEP_TIME);
}

static void
SetFifoZero(
	SPI_PORT port)
{
	// Diable SSP
	ithWriteRegMaskA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, 0, REG_BIT_SSP_EN);

	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	return;
}

static int
WaitDmaIdle(
	int dmachannel)
{
#ifdef SPI_USE_DMA_INTR
	int result = 0;

	result = itpSemWaitTimeout(&SpiDmaMutex, 5000);
	if ( result )
	{
		/* Time out, fail! */
		return 0;
	}
	else
	{
		return 1;
	}
#else
	uint32_t timeout_ms = 3000;

	while(ithDmaIsBusy(dmachannel) && --timeout_ms)
	{
        usleep(1000);
    }

    if( !timeout_ms )
    {
    	/* DMA fail */
    	return 0;
    }

    return 1;
#endif
}

static void
InitRegs(
    SPI_PORT port)
{
    int32_t format = 0;

    switch(g_SpiContext[port].format)
    {
    case CPO_0_CPH_0:
        format = REG_BIT_CPOL_LO | REG_BIT_CPHA_ACTIVE;
        break;
    case CPO_1_CPH_0:
        format = REG_BIT_CPOL_HI | REG_BIT_CPHA_ACTIVE;
        break;
    case CPO_0_CPH_1:
        format = REG_BIT_CPOL_LO | REG_BIT_CPHA_HALF_ACTIVE;
        break;
    case CPO_1_CPH_1:
        format = REG_BIT_CPOL_HI | REG_BIT_CPHA_HALF_ACTIVE;
        break;
    }

	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, REG_BIT_CS_ACTIVE_LOW | REG_BIT_FFMT_SPI | REG_BIT_MASTER_MODE | format);
    ithWriteRegA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, SSP_DEFAULT_DIV | (SSP_DEFAULT_FIFO_LENGTH << REG_SHIFT_SERIAL_DATA_LEN));
	ithWriteRegMaskA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, (1 << 2) | (1 << 3), (1 << 2) | (1 << 3));
    ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
		(1 << REG_SHIFT_TX_THRESHOLD) |
        (1 << REG_SHIFT_RX_THRESHOLD) |
        REG_BIT_RX_INTR_OR_EN |
        REG_BIT_TX_INTR_UR_EN |
        REG_BIT_RX_INTR_TH_EN |
        REG_BIT_TX_INTR_TH_EN);
}

static int32_t
WaitContollerReady(
    SPI_PORT port)
{

    uint32_t temp;
    int32_t result = ERROR_SSP_TRANSMIT_TIMEOUT;
   	uint32_t j = SSP_POLLING_COUNT;

    while(--j)
    {
        temp = ithReadRegA(REG_SSP_STATUS + port * REG_SSP_PORT_OFFSET);
        if((temp & REG_BIT_IS_BUSY) == 0)
        {
            result = 0;
            break;
        }
    }

    return result;
}


static int32_t
WaitFifoReady(
    SPI_PORT port,
    FIFO_READY fifoIndex, // 0 for RX, 1 for TX
    uint32_t count)
{

    uint32_t temp;
    int32_t result = ERROR_SSP_FIFO_READY_TIMEOUT;
   	uint32_t j = SSP_POLLING_COUNT;
   	uint32_t fifoReadyMask = (fifoIndex == FIFO_RX) ? REG_MASK_RX_FIFO_VALID_ENTRIES : REG_MASK_TX_FIFO_VALID_ENTRIES;
   	uint32_t fifoShift = (fifoIndex == FIFO_RX) ? REG_SHIFT_RX_FIFO_VALID_ENTRIES : REG_SHIFT_TX_FIFO_VALID_ENTRIES;

    if (fifoIndex == FIFO_TX)
    {
        while((ithReadRegA(REG_SSP_STATUS + port * REG_SSP_PORT_OFFSET) & (0x4 |0x7f000))!=0);
        result = 0;
        return result;
    }

    while(--j)
    {
        temp = ithReadRegA(REG_SSP_STATUS + port * REG_SSP_PORT_OFFSET);
        if(((temp & fifoReadyMask) >> fifoShift) >= count)
        {
            result = 0;
            break;
        }
    }	
    return result;
}

static bool 
_spiAssign9070Gpio(
	SPI_PORT port)
{
	int i = 0;
    int entryCount = 0;
    int mosiGpio = 0;
    int misoGpio = 0;
    int clockGpio = 0;
    int chipSelGpio = 0;

    SPI_CONFIG_MAPPING_ENTRY*    ptMappingEntry;
    typedef struct PORT_IO_MAPMING_ENTRY_TAG
    {
        SPI_PORT port;
        int gpioPin;
        int gpioMode;
    } PORT_IO_MAPMING_ENTRY;

	static PORT_IO_MAPMING_ENTRY tSpiMosiMappingTable[] =
	{
		//SPI_1 : PORT, GPIO, MODE
		{SPI_1, 10, 3 },
	};

	static PORT_IO_MAPMING_ENTRY tSpiMisoMappingTable[] =
	{
		//SPI_1 : PORT, GPIO, MODE
		{SPI_1, 0, 3 }, 
	};

	static PORT_IO_MAPMING_ENTRY tSpiClockMappingTable[] =
	{
		//SPI_1 : PORT, GPIO, MODE
		{SPI_1, 1, 3 },
	};

	static PORT_IO_MAPMING_ENTRY tSpiChipSelMappingTable[] =
	{
		//SPI_1 : PORT, GPIO, MODE
		{SPI_1, 9, 2 }, 
	};

	
	ptMappingEntry = &g_SpiContext[port].tMappingEntry;

    if (port == SPI_0)
	{		
		return true;
	}
	else if (port == SPI_1)
	{
#ifdef CFG_SPI2_ENABLE
		mosiGpio = CFG_SPI2_MOSI_GPIO;
		misoGpio = CFG_SPI2_MISO_GPIO;
		clockGpio = CFG_SPI2_CLOCK_GPIO;
		chipSelGpio = CFG_SPI2_CHIP_SEL_GPIO;
#else
		mosiGpio = 10;
		misoGpio = 0;
		clockGpio = 1;
		chipSelGpio = 9;
#endif
	}
	else
	{
		return false;
	}
	
	//MOSI
	entryCount = sizeof(tSpiMosiMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);	
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiMosiMappingTable[i].port
		&&	mosiGpio == tSpiMosiMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{		
		return false;
	}
	ptMappingEntry->spiMosi.gpioPin = tSpiMosiMappingTable[i].gpioPin;
	ptMappingEntry->spiMosi.gpioMode = tSpiMosiMappingTable[i].gpioMode;
	
	//MISO
	entryCount = sizeof(tSpiMisoMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{		
		if (port == tSpiMisoMappingTable[i].port
		&&	misoGpio == tSpiMisoMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{		
		return false;
	}
	ptMappingEntry->spiMiso.gpioPin = tSpiMisoMappingTable[i].gpioPin;
	ptMappingEntry->spiMiso.gpioMode = tSpiMisoMappingTable[i].gpioMode;
	
	//CLOCK
	entryCount = sizeof(tSpiClockMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiClockMappingTable[i].port
		&&	clockGpio == tSpiClockMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{	
		return false;
	}
	ptMappingEntry->spiClock.gpioPin = tSpiClockMappingTable[i].gpioPin;
	ptMappingEntry->spiClock.gpioMode = tSpiClockMappingTable[i].gpioMode;

	//CHIP SEL
	entryCount = sizeof(tSpiChipSelMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiChipSelMappingTable[i].port
		&&	chipSelGpio == tSpiChipSelMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{
		return false;
	}
	ptMappingEntry->spiChipSel.gpioPin = tSpiChipSelMappingTable[i].gpioPin;
	ptMappingEntry->spiChipSel.gpioMode = tSpiChipSelMappingTable[i].gpioMode;
	return true;
	    
}

static bool
_spiAssign9910Gpio(
	SPI_PORT port)
{
	int i = 0;
    int entryCount = 0;
    int mosiGpio = 0;
    int misoGpio = 0;
    int clockGpio = 0;
    int chipSelGpio = 0;

	SPI_CONFIG_MAPPING_ENTRY*    ptMappingEntry;
	typedef struct PORT_IO_MAPMING_ENTRY_TAG
	{
		SPI_PORT port;
		int gpioPin;
		int gpioMode;
	} PORT_IO_MAPMING_ENTRY;

	static PORT_IO_MAPMING_ENTRY tSpiMosiMappingTable[] =
	{
		//SPI_0
		{SPI_0, 9, 1 },
		//SPI_1
		{SPI_1, 22, 1 }, {SPI_1, 29, 1 }, {SPI_1, 40, 1 }, {SPI_1, 44, 1 },
	};

	static PORT_IO_MAPMING_ENTRY tSpiMisoMappingTable[] =
	{
		//SPI_0
		{SPI_0, 10, 1},
		//SPI_1
		{SPI_1, 23, 1 }, {SPI_1, 30, 1 }, {SPI_1, 41, 1 }, {SPI_1, 45, 1 },
	};

	static PORT_IO_MAPMING_ENTRY tSpiClockMappingTable[] =
	{
		//SPI_0
		{SPI_0, 11, 1},
		//SPI_1
		{SPI_1, 20, 1 }, {SPI_1, 27, 1 }, {SPI_1, 38, 1 }, {SPI_1, 42, 1 },
	};

	static PORT_IO_MAPMING_ENTRY tSpiChipSelMappingTable[] =
	{	
		//SPI_0
		{SPI_0, 7,  1},
		//SPI_1
		{SPI_1, 21, 1 }, {SPI_1, 28, 1 }, {SPI_1, 39, 1 }, {SPI_1, 43, 1 },
	};
	
	ptMappingEntry = &g_SpiContext[port].tMappingEntry;

	if (port == SPI_0)
	{
#if defined (CFG_SPI1_ENABLE) && defined (CFG_CHIP_PKG_IT9910)
		mosiGpio = CFG_SPI1_MOSI_GPIO;
		misoGpio = CFG_SPI1_MISO_GPIO;
		clockGpio = CFG_SPI1_CLOCK_GPIO;
		chipSelGpio = CFG_SPI1_CHIP_SEL_GPIO;
#else
		mosiGpio = 9;
		misoGpio = 10;
		clockGpio = 11;
		chipSelGpio = 7;
#endif
	}
	else if (port == SPI_1)
	{
#ifdef CFG_SPI2_ENABLE
		mosiGpio = CFG_SPI2_MOSI_GPIO;
		misoGpio = CFG_SPI2_MISO_GPIO;
		clockGpio = CFG_SPI2_CLOCK_GPIO;
		chipSelGpio = CFG_SPI2_CHIP_SEL_GPIO;
#else
		mosiGpio = 10;
		misoGpio = 0;
		clockGpio = 1;
		chipSelGpio = 9;
#endif
	}
	else
	{
		return false;
	}

	//MOSI
	entryCount = sizeof(tSpiMosiMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiMosiMappingTable[i].port
		&&	mosiGpio == tSpiMosiMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{
		return false;
	}
	ptMappingEntry->spiMosi.gpioPin = tSpiMosiMappingTable[i].gpioPin;
	ptMappingEntry->spiMosi.gpioMode = tSpiMosiMappingTable[i].gpioMode;
	
	//MISO
	entryCount = sizeof(tSpiMisoMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiMisoMappingTable[i].port
		&&	misoGpio == tSpiMisoMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{
		return false;
	}
	ptMappingEntry->spiMiso.gpioPin = tSpiMisoMappingTable[i].gpioPin;
	ptMappingEntry->spiMiso.gpioMode = tSpiMisoMappingTable[i].gpioMode;
	
	//CLOCK
	entryCount = sizeof(tSpiClockMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiClockMappingTable[i].port
		&&	clockGpio == tSpiClockMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{
		return false;
	}
	ptMappingEntry->spiClock.gpioPin = tSpiClockMappingTable[i].gpioPin;
	ptMappingEntry->spiClock.gpioMode = tSpiClockMappingTable[i].gpioMode;

	//CHIP SEL
	entryCount = sizeof(tSpiChipSelMappingTable) / sizeof(SPI_IO_MAPPING_ENTRY);
	for (i = 0; i < entryCount; i++)
	{
		if (port == tSpiChipSelMappingTable[i].port
		&&	chipSelGpio == tSpiChipSelMappingTable[i].gpioPin)
		{
			break;
		}
	}
	if (i >= entryCount)
	{
		return false;
	}
	ptMappingEntry->spiChipSel.gpioPin = tSpiChipSelMappingTable[i].gpioPin;
	ptMappingEntry->spiChipSel.gpioMode = tSpiChipSelMappingTable[i].gpioMode;
	return true;
}	

static bool
_spiSetGPIO(
	SPI_PORT port)
{
    int setGpio = 0;
    int setMode = 0;
    SPI_CONFIG_MAPPING_ENTRY*    ptMappingEntry;

    if (ithGetDeviceId() == 0x9910)
    {
        if (!_spiAssign9910Gpio(port))
        {
            return false;
        }
    }
	else if (ithGetDeviceId() == 0x9070)
	{
		if (!_spiAssign9070Gpio(port))
		{
			return false;
		}
	}		
    else
    {
        return false;
    }

    ptMappingEntry = &g_SpiContext[port].tMappingEntry;

    // Set MOSI
    setGpio = ptMappingEntry->spiMosi.gpioPin;
    setMode = ptMappingEntry->spiMosi.gpioMode;
    ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
        (setMode << ((setGpio & 0xF) * 2)),
        (0x03 << ((setGpio & 0xF) * 2)));

    // Set MISO
    setGpio = ptMappingEntry->spiMiso.gpioPin;
    setMode = ptMappingEntry->spiMiso.gpioMode;
    ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
        (setMode << ((setGpio & 0xF) * 2)),
        (0x03 << ((setGpio & 0xF) * 2)));

    // Set CLOCK
    setGpio = ptMappingEntry->spiClock.gpioPin;
    setMode = ptMappingEntry->spiClock.gpioMode;
    ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
        (setMode << ((setGpio & 0xF) * 2)),
        (0x03 << ((setGpio & 0xF) * 2)));

    // Set CHIP SEL
    setGpio = ptMappingEntry->spiChipSel.gpioPin;
    setMode = ptMappingEntry->spiChipSel.gpioMode;
    ithWriteRegMaskA(ITH_GPIO_BASE + (ITH_GPIO1_MODE_REG + (setGpio / 16) * 4),
        (setMode << ((setGpio & 0xF) * 2)),
        (0x03 << ((setGpio & 0xF) * 2)));
    return true;
}

static void
SetPadSel(
	SPI_PORT port)
{
    uint32_t data;

	//Set GPIO	
	if (_spiSetGPIO(port) == false)
		printf("_spiSetGPIO fail port[%d]\n", port);

	//Set SSP CLK SEL
    switch(port)
    {
    case SPI_0:
        data = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG);
    	// SSP CLK SEL
    	data |= 0x0080;
    	data &= ~0x07;
    	data |= 0x06;
    	ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG, data);
        break;

    case SPI_1:
		ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG, (0x4<<4) | (0x1<<27) , 0x0070 | (0x1<<27));        
        break;
    }

}

static int32_t
SPI_Initial(
	SPI_PORT port)
{
	int32_t result = 0;
		
#ifdef SPI_USE_DMA_INTR
	ITHDmaIntrHandler spiDmaIntr = SpiDmaIntr;
#else
	ITHDmaIntrHandler spiDmaIntr = NULL;
#endif
	
	g_SpiContext[port].format = CPO_0_CPH_0;
	g_SpiContext[port].mode   = SPI_MODE_0;

	if (port == SPI_0)
	{
		g_SpiContext[port].ch_name = "dma_SPI_0";
		g_SpiContext[port].slave_ch_name = "dma_SPI_0_slave";
	}
	else
	{	
		g_SpiContext[port].ch_name = "dma_SPI_1";
		g_SpiContext[port].slave_ch_name = "dma_SPI_1_slave";
	}

	
#ifdef SPI_USE_DMA
		g_SpiContext[port].dma_ch= ithDmaRequestCh(g_SpiContext[port].ch_name, ITH_DMA_CH_PRIO_HIGH_3, spiDmaIntr, NULL);
		if (g_SpiContext[port].dma_ch < 0 )
		{
			result = ERROR_SSP_CREATE_DMA_FAIL;
			goto end;
		}
		ithDmaReset(g_SpiContext[port].dma_ch);

		/* DMA channel for slave */
		g_SpiContext[port].dma_slave_ch = ithDmaRequestCh(g_SpiContext[port].slave_ch_name, ITH_DMA_CH_PRIO_HIGH_3, spiDmaIntr, NULL);
		if (g_SpiContext[port].dma_slave_ch < 0)
		{
			result = ERROR_SSP_CREATE_DMA_FAIL;
			goto end;
		}
		ithDmaReset(g_SpiContext[port].dma_slave_ch);
#ifdef SPI_USE_DMA_INTR
		sem_init(&SpiDmaMutex, 0, 0);
#endif
#endif
		InitRegs(port);

end:	
    return result;

}	

int32_t
mmpSpiInitialize(
    SPI_PORT port, SPI_OP_MODE mode, SPI_FORMAT  format, SPI_CLK_LAB spiclk)
{
    int32_t result = 0;

    /*init for IT9910 and NOR switch control mutex*/
    //pthread_mutex_init(&SwitchMutex, NULL);

    pthread_mutex_lock(&g_SpiContext[port].mutex);

    switch(port)
    {
    case SPI_0:
        if(g_initCount[SPI_0] == 0)
    	{
    		SetPadSel(SPI_0);
            result = SPI_Initial(SPI_0);
    	}
        g_initCount[SPI_0]++;
        break;

    case SPI_1:
        if(g_initCount[SPI_1] == 0)
    	{
    		SetPadSel(SPI_1);
            result = SPI_Initial(SPI_1);
    	}
        g_initCount[SPI_1]++;
        break;
    }

    // Frequency Divider
    {
    	//const float refClock = 16.875f;
    	const float refClock = 25.0f;

    	float wclk = _GetWclkClock();
		uint32_t slck_divider = ((wclk / 2) - refClock) / refClock + 0.9f;
		
		//ithWriteRegMaskA(ITH_GPIO_BASE + 0xD0, 0x03000F00, 0x0300FF00);
		//ithWriteRegMaskA(REG_SSP_CONTROL_0, 0x00000003, 0x00000003);
		
		//printf("_reg::0x1C=%x,0xA2=%x,0xA4=%x\n",ithReadRegH(0x1C),ithReadRegH(0xA2),ithReadRegH(0xA4));
		//printf("_regGPIO::0xD0=%x\n",ithReadRegA(ITH_GPIO_BASE + 0xD0) );
		//printf("_regSPI::0x00=%x, 0x04=%x\n",ithReadRegA(REG_SSP_CONTROL_0 + 0x00),ithReadRegA(REG_SSP_CONTROL_0 + 0x04) );		

		printf("[NOR]slck_divider = %d, SPI clock = %f MHz\n", slck_divider, wclk / (((float)slck_divider + 1) * 2));

		ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, slck_divider, REG_MASK_SCLK_DIV);
	}
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

int32_t
mmpSpiTerminate(
    SPI_PORT port)
{
    int32_t result = 0;

    /*destroy for IT9910 and NOR switch control mutex*/
    pthread_mutex_destroy(&SwitchMutex);

    pthread_mutex_lock(&g_SpiContext[port].mutex);

    switch(port)
    {
    case SPI_0:
        if((--g_initCount[SPI_0]) == 0)
        {
            //STOP SSP
            ithWriteRegA(REG_SSP_CONTROL_2, REG_BIT_TXFCLR_EN);
        }
        break;

    case SPI_1:
        if((--g_initCount[SPI_1]) == 0)
        {
            //STOP SSP
            ithWriteRegA(REG_SSP_CONTROL_2 + REG_SSP_PORT_OFFSET, REG_BIT_TXFCLR_EN);
        }
        break;
    }

#ifdef SPI_USE_DMA
    if (   g_initCount[SPI_0] == 0
        && g_initCount[SPI_1] == 0 )
   	{
   		ithDmaFreeCh(g_SpiContext[port].dma_ch);
   		ithDmaFreeCh(g_SpiContext[port].dma_slave_ch);
   	}
#ifdef SPI_USE_DMA_INTR
	sem_destroy(&SpiDmaMutex);
#endif
#endif

	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

#ifdef SPI_USE_DMA
int32_t
mmpSpiDmaWrite(
    SPI_PORT port,
    uint8_t* inputData,
    uint32_t  inputSize,
    uint8_t* psrc,
    uint32_t  size,
    uint8_t  dataLength)
{
    int32_t     result       = -1;
    uint8_t*    tempBuf      = NULL;
    uint32_t    totalSize    = size + inputSize;
    uint8_t     dataLen      = dataLength - 1;
	uint32_t    data         = 0;
    ITHDmaWidth srcWidth     = ITH_DMA_WIDTH_8;
    ITHDmaWidth dstWidth     = ITH_DMA_WIDTH_8;
    ITHDmaBurst burstSize    = ITH_DMA_BURST_1;
   	uint32_t    tx_threshold = 0;
    uint32_t    rx_threshold = 0;

    /* Lock */
    pthread_mutex_lock(&g_SpiContext[port].mutex);
    
	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
    	SetPadSel(port);
	}
	
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

    tempBuf = (uint8_t *)itpVmemAlloc(SSP_DMA_TEMPBUF);
	if(tempBuf == NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}

	if ( g_SpiContext[port].dma_ch >= 0 )
	{
		// TX channel
        // write control data
#if defined(_WIN32)
		{
			uint8_t* mappedSysRam = NULL;

			mappedSysRam = ithMapVram((uint32_t)tempBuf, SSP_DMA_TEMPBUF, ITH_VRAM_WRITE);
			memcpy(mappedSysRam, inputData, inputSize);
			memcpy(mappedSysRam + inputSize, psrc, size);
			ithFlushDCacheRange(mappedSysRam, SSP_DMA_TEMPBUF);
			ithUnmapVram(mappedSysRam, SSP_DMA_TEMPBUF);
        }
#else
		{
			unsigned int i = 0;

        	for(i = 0; i < inputSize; ++i)
			{
            	*(tempBuf+ i) = *(inputData + i);
			}

	        memcpy(tempBuf + inputSize, psrc, size);
        }
#endif
		// 32 bits
		if((totalSize & 0x3) == 0)
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_32;
			dstWidth     = ITH_DMA_WIDTH_32;
			burstSize    = ITH_DMA_BURST_4; // 4
			dataLen      = 31;
			
			// Big enddian, NOR CMD format
			data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
			data |= REG_BIT_TXFIFO_BIG_END;			
			ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);
		}
		else if((totalSize & 0x1) == 0)	// 16 bits
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_16;
			dstWidth     = ITH_DMA_WIDTH_16;
			burstSize    = ITH_DMA_BURST_4;
			dataLen      = 15;
			
			// Big enddian NOR CMD format
			data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
			data |= REG_BIT_TXFIFO_BIG_END;
			ithWriteRegA(REG_SSP_CONTROL_0+ port * REG_SSP_PORT_OFFSET,data);

		}
		else	// 8 bits
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth     = ITH_DMA_WIDTH_8;
			dstWidth     = ITH_DMA_WIDTH_8;
			burstSize    = ITH_DMA_BURST_1; //ITH_DMA_BURST_8; // 8
			dataLen      = 7;
		}

		ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, (uint32_t)tempBuf);
		ithDmaSetSrcParams(g_SpiContext[port].dma_ch, srcWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetDstParams(g_SpiContext[port].dma_ch, dstWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetTxSize(g_SpiContext[port].dma_ch, totalSize);
		ithDmaSetBurst(g_SpiContext[port].dma_ch, burstSize);
	    switch(port)
		{
	    case SPI_0:
	    	ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI1_DATA_BASE);
	    	ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_TX);
	        break;
        case SPI_1:
            ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI2_DATA_BASE);
            ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_TX);
	        break;
		}

		// Clear tx FIFO
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

		// Free run
		ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,REG_BIT_FR_CSN_CLR | REG_BIT_FR_RX_LOCK);


#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tempBuf, SSP_DMA_TEMPBUF);
#endif

        // Fire DMA
        ithDmaStart(g_SpiContext[port].dma_ch);

	    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

        // SSP dma enable
        ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
        	((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
            (rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_INTR_OR_EN |
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_TH_EN |
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_TX_DMA_EN));

        // Fire SSP
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
        result = WaitDmaIdle(g_SpiContext[port].dma_ch);
        if( result == 0 )
        {
        	/* DMA fail */
            goto end;
        }

        // Wait ssp ready
        result = WaitContollerReady(port);
	}

end:
    itpVmemFree((uint32_t)tempBuf);

    //SSP dma disable
    ithWriteRegA(
		REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
		((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
		(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
		REG_BIT_RX_INTR_OR_EN |
		REG_BIT_TX_INTR_UR_EN |
		REG_BIT_RX_INTR_TH_EN |
		REG_BIT_TX_INTR_TH_EN));

	//reset enddian
	data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
	data &= ~REG_BIT_TXFIFO_BIG_END;	
	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, data);

	//Free run
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0x0);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

int32_t
mmpSpiDmaWriteMulti(
    SPI_PORT port,
    uint8_t* inputData,
    int32_t  inputSize,
    uint8_t* psrc,
    int32_t  size,
    uint8_t  dataLength,
    uint32_t pageSize)
{
    int32_t     result       = -1;
    uint8_t*    tempBuf      = NULL;
    uint32_t    totalSize    = inputSize + pageSize;
    uint8_t     dataLen      = dataLength - 1;
	uint32_t    data         = 0;
    ITHDmaWidth srcWidth     = ITH_DMA_WIDTH_8;
    ITHDmaWidth dstWidth     = ITH_DMA_WIDTH_8;
    ITHDmaBurst burstSize    = ITH_DMA_BURST_1;
   	uint32_t    tx_threshold = 0;
    uint32_t    rx_threshold = 0;

    uint32_t loopCount = size / pageSize;
    uint32_t loopIndex = 0;
    uint32_t dmaBufferSize = totalSize * loopCount;

    /* Lock */
    pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
    	SetPadSel(port);
	}

    tempBuf = (uint8_t *)itpVmemAlloc(dmaBufferSize);
	if(tempBuf == NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}

	if ( g_SpiContext[port].dma_ch >= 0 )
	{
		// TX channel
        // write control data
//#if defined(_WIN32)
#if 1
		{
			uint8_t* mappedSysRam = NULL;
			uint32_t writeAddress = 0;

			if ( inputSize == 5 )
			{
				writeAddress =
					(((uint32_t)inputData[1]) << 24) |
					(((uint32_t)inputData[2]) << 16) |
					(((uint32_t)inputData[3]) << 8) |
					inputData[4];
			}
			else
			{
				writeAddress =
					(((uint32_t)inputData[1]) << 16) |
					(((uint32_t)inputData[2]) << 8) |
					inputData[3];
			}

			mappedSysRam = ithMapVram((uint32_t)tempBuf, dmaBufferSize, ITH_VRAM_WRITE);
			for ( loopIndex = 0; loopIndex < loopCount; loopIndex++ )
			{
				uint8_t norCmd[5];

				if ( inputSize == 5 )
				{
					norCmd[0] = inputData[0];
					norCmd[1] = (uint8_t)(writeAddress >> 24);
					norCmd[2] = (uint8_t)(writeAddress >> 16);
					norCmd[3] = (uint8_t)(writeAddress >> 8);
					norCmd[4] = (uint8_t)writeAddress;
				}
				else
				{
					norCmd[0] = inputData[0];
					norCmd[1] = (uint8_t)(writeAddress >> 16);
					norCmd[2] = (uint8_t)(writeAddress >> 8);
					norCmd[3] = (uint8_t)writeAddress;
				}
				memcpy(mappedSysRam + (loopIndex * totalSize), norCmd, inputSize);
				memcpy(mappedSysRam + (loopIndex * totalSize) + inputSize, psrc + (loopIndex * pageSize), pageSize);
				writeAddress += pageSize;
			}
			ithFlushDCacheRange(mappedSysRam, dmaBufferSize);
			ithUnmapVram(mappedSysRam, dmaBufferSize);
        }
#else
		{
			unsigned int i = 0;

        	for(i = 0; i < inputSize; ++i)
			{
            	*(tempBuf+ i) = *(inputData + i);
			}

	        memcpy(tempBuf + inputSize, psrc, size);
        }
#endif
		// 32 bits
		if((totalSize & 0x3) == 0)
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_32;
			dstWidth     = ITH_DMA_WIDTH_32;
			burstSize    = ITH_DMA_BURST_4; // 4
			dataLen      = 31;
		}
		else if((totalSize & 0x1) == 0)	// 16 bits
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_16;
			dstWidth     = ITH_DMA_WIDTH_16;
			burstSize    = ITH_DMA_BURST_4;
			dataLen      = 15;
		}
		else	// 8 bits
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth     = ITH_DMA_WIDTH_8;
			dstWidth     = ITH_DMA_WIDTH_8;
			burstSize    = ITH_DMA_BURST_1; //ITH_DMA_BURST_8; // 8
			dataLen      = 7;
		}

#ifdef ARM_NOR_WRITER
		printf("Writing pages......\n");
		SetArmNorWriterAppStatus(ANW_APP_STATUS_WRITING_PAGES);
		SetArmNorWriterAppProgressMin(0);
		SetArmNorWriterAppProgressMax(loopCount - 1);
#endif
		for ( loopIndex = 0; loopIndex < loopCount; loopIndex++ )
		{
#ifdef ARM_NOR_WRITER
			SetArmNorWriterAppProgressCurr(loopIndex);
#endif
			// Release CSN clear
			ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

			/* Write Enable */
			{
				uint8_t tempCmdData[1] = {0x06};
		        mmpSpiPioWriteNoLock(port, tempCmdData, 1, 0, 0, 8);
			}

			if(   (totalSize & 0x3) == 0
			   || (totalSize & 0x1) == 0 )
			{
				// Big enddian
				data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
				data |= REG_BIT_TXFIFO_BIG_END;
				ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, data);
			}

			ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, ((uint32_t)tempBuf) + (loopIndex * totalSize));
			ithDmaSetSrcParams(g_SpiContext[port].dma_ch, srcWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
			ithDmaSetDstParams(g_SpiContext[port].dma_ch, dstWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
			ithDmaSetTxSize(g_SpiContext[port].dma_ch, totalSize);
			ithDmaSetBurst(g_SpiContext[port].dma_ch, burstSize);
		    switch(port)
			{
		    case SPI_0:
		    	ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI1_DATA_BASE);
		    	ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_TX);
		        break;
	        case SPI_1:
	            ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI2_DATA_BASE);
	            ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_TX);
		        break;
			}

			// Clear tx FIFO
	        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

			// Free run
			ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, REG_BIT_FR_CSN_CLR | REG_BIT_FR_RX_LOCK);

#ifdef __OPENRTOS__
			// Flush DC
			ithFlushDCacheRange(tempBuf, dmaBufferSize);
#endif

	        // Fire DMA
	        ithDmaStart(g_SpiContext[port].dma_ch);

		    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

	        // SSP dma enable
	        ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
	        	((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
	            (rx_threshold << REG_SHIFT_RX_THRESHOLD) |
				REG_BIT_RX_INTR_OR_EN |
				REG_BIT_TX_INTR_UR_EN |
				REG_BIT_RX_INTR_TH_EN |
				REG_BIT_TX_INTR_TH_EN |
				REG_BIT_TX_DMA_EN));

	        // Fire SSP
	        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
	        result = WaitDmaIdle(g_SpiContext[port].dma_ch);
	        if( result == 0 )
	        {
	        	/* DMA fail */
	            goto end;
	        }

	        // Wait ssp ready
	        result = WaitContollerReady(port);
		}
#ifdef ARM_NOR_WRITER
		SetArmNorWriterAppProgressCurr(GetArmNorWriterAppProgressMax());
#endif
	}

end:
    itpVmemFree((uint32_t)tempBuf);

    // SSP dma disable
    ithWriteRegA(
		REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
		((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
		(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
		REG_BIT_RX_INTR_OR_EN |
		REG_BIT_TX_INTR_UR_EN |
		REG_BIT_RX_INTR_TH_EN |
		REG_BIT_TX_INTR_TH_EN));

	//reset enddian
	data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
	data &= ~REG_BIT_TXFIFO_BIG_END;
	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, data);

	//Free run
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0x0);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

#endif

#ifdef SPI_USE_DMA
int32_t
mmpSpiDmaRead(
    SPI_PORT port,
    uint8_t* inputData,
    uint32_t  inputSize,
    uint8_t* pdes,
    uint32_t  size,
    uint8_t  dataLength)
{
    int32_t     result       = -1;
    uint8_t*    tempBuf      = NULL;
    uint8_t*    tempRxBuf    = NULL;
    uint32_t    totalSize    = size + inputSize;
    uint8_t     dataLen      = dataLength - 1;
	uint32_t    data         = 0;
    uint32_t    i            = 0;
	ITHDmaWidth srcWidth     = ITH_DMA_WIDTH_8;
    ITHDmaWidth dstWidth     = ITH_DMA_WIDTH_8;
    ITHDmaBurst burstSize    = ITH_DMA_BURST_1;
   	uint32_t    tx_threshold = 1;
    uint32_t    rx_threshold = 1;
    //uint32_t c            = SSP_POLLING_COUNT;

    /* Lock */
    pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
    	SetPadSel(port);
	}
#if defined(_WIN32)
    tempRxBuf = (uint8_t *)itpVmemAlloc(SSP_DMA_TEMPBUF);
    if(tempRxBuf == NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}
#elif defined(__OPENRTOS__) || defined(__FREERTOS__)
	tempRxBuf = (uint8_t *)malloc(SSP_DMA_TEMPBUF);
    if(tempRxBuf == NULL)
	{
		result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
		goto end;
	}
#endif
    if( g_SpiContext[port].dma_ch >= 0 )
    {
    	
    	// 32b its
		if( (totalSize&0x3) == 0 )
		{
			uint32_t value = 0;

			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_32;
			dstWidth     = ITH_DMA_WIDTH_32;
			burstSize    = ITH_DMA_BURST_4; // 4
			dataLen      = 31;
			
			// TX channel
			//TODO work around clear
			SetFifoZero(port);

			ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
			ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
			// Big enddian
			data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
			data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
			data |= REG_BIT_RXFIFO_BIG_END;
#endif
			ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, data);

			for( i = 0; i < (uint32_t)(inputSize / 4); ++i )
			{
				value = (uint32_t)(*((uint32_t*)inputData + i));
				ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, value);
			}

			// Free run
			{
				int32_t freerunCount = 0;
				int32_t remaincnt = 0;
				
				if (rx_threshold != 1)
					remaincnt = rx_threshold-((totalSize/4)%rx_threshold);
				
				freerunCount = (size / ((dataLen + 1) / 8))+ remaincnt;				
				data = (freerunCount << FREERUN_SHIFT) | 0x06;
				ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);			
				data = (freerunCount << FREERUN_SHIFT) | 0x07;
				ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);
			}

		}
		else if( (totalSize&0x1) == 0 )	//16bits
		{
			uint32_t value = 0;

			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth     = ITH_DMA_WIDTH_16;
			dstWidth     = ITH_DMA_WIDTH_16;
			burstSize    = ITH_DMA_BURST_4; // 4
			dataLen      = 15;
			
			// TX channel
			//TODO work around clear
			SetFifoZero(port);

			ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
            ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
			// Big enddian
			data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
			data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
			data |= REG_BIT_RXFIFO_BIG_END;
#endif
			ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

			for( i = 0; i < (uint32_t)(inputSize / 2); ++i )
			{
				value = (uint32_t)(*((uint16_t*)inputData + i));
				ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, value);
			}

			// Free run
			{
				int32_t freerunCount = 0;
				int32_t remaincnt = 0;

				if (rx_threshold != 1)
					remaincnt = rx_threshold-((totalSize/2)%rx_threshold);
				
				freerunCount = (size / ((dataLen + 1) / 8)) + remaincnt;			
				data = (freerunCount << FREERUN_SHIFT) | 0x06;
				ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);			
				data = (freerunCount << FREERUN_SHIFT) | 0x07;
				ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);
		    }
		}
		else	// 8 bits
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth     = ITH_DMA_WIDTH_8;
			dstWidth     = ITH_DMA_WIDTH_8;
			burstSize    = ITH_DMA_BURST_1; // 1
			dataLen      = 7;
			
			//TX channel
			//clear rx tx FIFO
			SetFifoZero(port);

			ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_TXFCLR_EN | REG_BIT_RXFCLR_EN);
            ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

			data = ((size)<< FREERUN_SHIFT)  | 0x06;
			ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,data);

			for( i = 0; i < (uint32_t)inputSize; ++i )
            {
				ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(inputData + i));
            }

			data = ((size)<<FREERUN_SHIFT) | 0x07;
			ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,data);
		}

		ithDmaSetDstAddr(g_SpiContext[port].dma_ch, (uint32_t)tempRxBuf);
		ithDmaSetSrcParams(g_SpiContext[port].dma_ch, srcWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetDstParams(g_SpiContext[port].dma_ch, dstWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetTxSize(g_SpiContext[port].dma_ch, totalSize);
		ithDmaSetBurst(g_SpiContext[port].dma_ch, burstSize);
	    switch(port)
		{
	    case SPI_0:
	    	ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, AHB_SPI1_DATA_BASE);
	    	ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
        case SPI_1:
        	ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, AHB_SPI2_DATA_BASE);
            ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
		}


#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tempRxBuf, SSP_DMA_TEMPBUF);
#endif
		

		// Fire DMA
        ithDmaStart(g_SpiContext[port].dma_ch);

        // SSP dma enable
        ithWriteRegA(
			REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
			((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
			(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_DMA_EN |
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_RX_INTR_TH_EN |
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_OR_EN));
		
		// Fire SSP
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

		// DMA wait idle
        result = WaitDmaIdle(g_SpiContext[port].dma_ch);
        if( result == 0 )
        {
        	/* DMA fail */
            goto end;
        }

        // Wait ssp ready
        result = WaitContollerReady(port);
		if(result)
        {
            goto end;
        }

        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_DIS);
#if defined(_WIN32)
		{
            uint8_t* mappedSysRam = NULL;

			mappedSysRam = ithMapVram((uint32_t)tempRxBuf, SSP_DMA_TEMPBUF, ITH_VRAM_READ);
			memcpy(pdes, mappedSysRam + inputSize, totalSize - inputSize);
			ithUnmapVram(mappedSysRam, SSP_DMA_TEMPBUF);
        }
#elif defined (__OPENRTOS__)
        // Clear cache
        ithInvalidateDCacheRange(tempRxBuf + inputSize, totalSize - inputSize);
		memcpy(pdes, tempRxBuf + inputSize, totalSize - inputSize);
#elif defined (__FREERTOS__)
		or32_invalidate_cache(tempRxBuf + inputSize, totalSize - inputSize);
		memcpy(pdes, tempRxBuf + inputSize, totalSize - inputSize);
#endif
    }

end:

#if defined(_WIN32) || defined (__FREERTOS__) || defined (__OPENRTOS__)
    itpVmemFree((uint32_t)tempRxBuf);
#endif

    // SSP dma disable
    ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
    	((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
		(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
		REG_BIT_RX_INTR_OR_EN |
		REG_BIT_TX_INTR_UR_EN |
		REG_BIT_RX_INTR_TH_EN |
		REG_BIT_TX_INTR_TH_EN));

	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	// Reset enddian
	data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
	data &= ~(REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END);
	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

int32_t
mmpSpiDmaReadNoDropFirstByte(
    SPI_PORT port,
    uint8_t* inputData,
    int32_t  inputSize,
    void*    pdes,
    int32_t  size,
    uint8_t  dataLength)
{
    int32_t     result       = -1;
    uint8_t*    tempBuf      = NULL;
    uint8_t*    tempRxBuf    = NULL;
    uint32_t    totalSize    = size + inputSize;
    uint8_t     dataLen      = dataLength - 1;
    uint32_t    data         = 0;
    uint32_t    i            = 0;
    ITHDmaWidth srcWidth     = ITH_DMA_WIDTH_8;
    ITHDmaWidth dstWidth     = ITH_DMA_WIDTH_8;
    ITHDmaBurst burstSize    = ITH_DMA_BURST_1;
    uint32_t    tx_threshold = 1;
    uint32_t    rx_threshold = 1;
    //uint32_t c            = SSP_POLLING_COUNT;

    /* Lock */
    pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
    	SetPadSel(port);
	}
#if defined(_WIN32)
    tempRxBuf = (uint8_t *)itpVmemAlloc(SSP_DMA_TEMPBUF);
    if(tempRxBuf == NULL)
    {
        result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
        goto end;
    }
#elif defined(__OPENRTOS__) || defined(__FREERTOS__)
    tempRxBuf = (uint8_t *)malloc(SSP_DMA_TEMPBUF);
    if(tempRxBuf == NULL)
    {
        result = ERROR_SSP_ALLOC_DMA_BUF_FAIL;
        goto end;
    }
#endif
    if( g_SpiContext[port].dma_ch >= 0 )
    {
        if( (totalSize&0x1) == 0 )	//16bits
        {
            uint32_t value = 0;

            tx_threshold = 4;
            rx_threshold = 4;
            srcWidth     = ITH_DMA_WIDTH_16;
            dstWidth     = ITH_DMA_WIDTH_16;
            burstSize    = ITH_DMA_BURST_1; // 1
            dataLen      = 15;

            // TX channel
            //TODO work around clear
            SetFifoZero(port);

            ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
            ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
            // Big enddian
            data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            data |= REG_BIT_RXFIFO_BIG_END;
#endif
            ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

            for( i = 0; i < (uint32_t)(inputSize / 2); ++i )
            {
                value = (uint32_t)(*((uint16_t*)inputData + i));
                ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, value);
            }

            // Free run
            {
                int32_t freerunCount = 0;

                freerunCount = (size / ((dataLen + 1) / 8)) + 31;
                data = (freerunCount << FREERUN_SHIFT) | 0x06;
                ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);
                data = (freerunCount << FREERUN_SHIFT) | 0x07;
                ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);
            }
        }
        // 32b its
        else if( (totalSize&0x3) == 0 )
        {
            uint32_t value = 0;

            tx_threshold = 4;
            rx_threshold = 4;
            srcWidth     = ITH_DMA_WIDTH_32;
            dstWidth     = ITH_DMA_WIDTH_32;
            burstSize    = ITH_DMA_BURST_1; // 1
            dataLen      = 31;

            // TX channel
            //TODO work around clear
            SetFifoZero(port);

            ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);
            ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

            // Big enddian
            data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            data |= REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            data |= REG_BIT_RXFIFO_BIG_END;
#endif
            ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

            // Free run
            {
                int32_t freerunCount = 0;

                freerunCount = (size / ((dataLen + 1) / 8)) + 31;
                data = (freerunCount << FREERUN_SHIFT) | 0x06;
                ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data);

                for( i = 0; i < (uint32_t)(inputSize / 4); ++i )
                {
                    value = (uint32_t)(*((uint32_t*)inputData + i));
                    ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, value);
                }

                data = (freerunCount << FREERUN_SHIFT) | 0x07;
                ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, data); //awin
            }
        }
        else	// 8 bits
        {
            tx_threshold = 1;
            rx_threshold = 1;
            srcWidth     = ITH_DMA_WIDTH_8;
            dstWidth     = ITH_DMA_WIDTH_8;
            burstSize    = ITH_DMA_BURST_1; // 1
            dataLen      = 7;

            //TX channel
            //clear rx tx FIFO
            SetFifoZero(port);

            ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET,  REG_BIT_TXFCLR_EN | REG_BIT_RXFCLR_EN);
            ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

            data = ((size)<< FREERUN_SHIFT)  | 0x06;
            ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,data);

            for( i = 0; i < (uint32_t)inputSize; ++i )
            {
                ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(inputData + i));
            }

            data = ((size)<<FREERUN_SHIFT) | 0x07;
            ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,data);
        }

        ithDmaSetDstAddr(g_SpiContext[port].dma_ch, (uint32_t)tempRxBuf);
        ithDmaSetSrcParams(g_SpiContext[port].dma_ch, srcWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetDstParams(g_SpiContext[port].dma_ch, dstWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetTxSize(g_SpiContext[port].dma_ch, totalSize);
        ithDmaSetBurst(g_SpiContext[port].dma_ch, burstSize);
        switch(port)
        {
        case SPI_0:
            ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, AHB_SPI1_DATA_BASE);
            ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
            break;
        case SPI_1:
            ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, AHB_SPI2_DATA_BASE);
            ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
            break;
        }


#ifdef __OPENRTOS__
        // Flush DC
        ithFlushDCacheRange(tempRxBuf, SSP_DMA_TEMPBUF);
#endif

        // Fire DMA
        ithDmaStart(g_SpiContext[port].dma_ch);

        // SSP dma enable
        ithWriteRegA(
            REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
            ((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
            (rx_threshold << REG_SHIFT_RX_THRESHOLD) |
            REG_BIT_RX_DMA_EN |
            REG_BIT_TX_INTR_TH_EN |
            REG_BIT_RX_INTR_TH_EN |
            REG_BIT_TX_INTR_UR_EN |
            REG_BIT_RX_INTR_OR_EN));

        // Fire SSP
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

        // DMA wait idle
        result = WaitDmaIdle(g_SpiContext[port].dma_ch);
        if( result == 0 )
        {
            /* DMA fail */
            goto end;
        }

        // Wait ssp ready
        result = WaitContollerReady(port);
        if(result)
        {
            goto end;
        }

        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_DIS);
#if defined(_WIN32)
        {
            uint8_t* mappedSysRam = NULL;

            mappedSysRam = ithMapVram((uint32_t)tempRxBuf, SSP_DMA_TEMPBUF, ITH_VRAM_READ);
            memcpy(pdes, mappedSysRam + 3, totalSize - 3);
            ithUnmapVram(mappedSysRam, SSP_DMA_TEMPBUF);
        }
#elif defined (__OPENRTOS__)
        // Clear cache
        ithInvalidateDCacheRange(tempRxBuf + 3, totalSize - 3);
        memcpy(pdes, tempRxBuf + 3, totalSize - 3);
#elif defined (__FREERTOS__)
        or32_invalidate_cache(tempRxBuf + 3, totalSize - 3);
        memcpy(pdes, tempRxBuf + 3, totalSize - 3);
#endif
    }

end:

#if defined(_WIN32) || defined (__FREERTOS__) || defined (__OPENRTOS__)
    itpVmemFree((uint32_t)tempRxBuf);
#endif

    // SSP dma disable
    ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
        ((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
        (rx_threshold << REG_SHIFT_RX_THRESHOLD) |
        REG_BIT_RX_INTR_OR_EN |
        REG_BIT_TX_INTR_UR_EN |
        REG_BIT_RX_INTR_TH_EN |
        REG_BIT_TX_INTR_TH_EN));

    ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

    // Reset enddian
    data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
    data &= ~(REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END);
    ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

    /* Unlock */
	if (ithGetDeviceId() == 0x9070)
    ithUnlockMutex(ithStorMutex);
    pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}
#endif

int32_t
mmpSpiPioWrite(
    SPI_PORT 	port,
    uint8_t* 	inputData,
    uint32_t 	inputSize,
    uint8_t *   pbuf,
    uint32_t 	size,
    uint8_t  	dataLength)

{
    uint32_t i       = 0;
    uint8_t  dataLen = dataLength - 1;
    int32_t  result  = 0;

	/* Lock */
	pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
		SetPadSel(port);
	}
    // Clear rx tx FIFO
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 4);
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN | REG_BIT_TXDO_EN);


    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

#ifdef __OPENRTOS__
	// Flush DC
	ithFlushDCacheRange(inputData, inputSize);
	ithFlushDCacheRange(pbuf, size);
#endif

    if( dataLength == 8 ) // Datalen 8bits
    {
        uint8_t *binput = (uint8_t*)(inputData);
        uint8_t *pbBuf  = (uint8_t*)(pbuf);
				
		if((inputSize + size) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        //Write data to fifo
        for(i = 0; i < inputSize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(binput + i));
        }

        for(i = 0; i < size; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pbBuf + i));
        }
    }
    else if( dataLength == 16 ) // Datalen 16bits
    {
        uint16_t* winput     = (uint16_t*)inputData;
        uint16_t* pwbuf      = (uint16_t*)pbuf;
        uint32_t  winputSize = inputSize / 2;
        uint32_t  wsize      = size / 2;
        uint16_t  temp       = 0 ;

		if((winputSize + wsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        // Write data to fifo
        for(i = 0; i < winputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(winput + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(winput + i));
#endif
        }

		for(i = 0; i < wsize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(pwbuf + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pwbuf + i));
#endif
        }
    }
    else if(dataLength == 32) // Datalen 32bits
    {
        uint32_t *dwinput = (uint32_t*)(inputData);
        uint32_t *pdwbuf = (uint32_t*)(pbuf);
        uint32_t dwinputSize = inputSize/4;
        uint32_t dwsize = size/4;
		uint32_t temp = 0;
		
		if((dwinputSize + dwsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}

        // Write data to fifo
        for(i = 0; i < dwinputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(dwinput + i);
            temp = SWAP_ENDIAN32(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(dwinput + i));
#endif                    
        }

        for(i = 0; i < dwinputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(pdwbuf + i);
            temp = SWAP_ENDIAN32(temp);			
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)			
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pdwbuf + i));
#endif                    
        }
    }
    else
    {
        //TODO
        result = ERROR_SSP_FIFO_LENGTH_UNSUPPORT;
		goto end;
    }

    // Fire
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

    result = 0;//WaitContollerReady(port);
    WaitFifoReady(port, FIFO_TX, 0);

end:
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

int32_t
mmpSpiPioWriteNoLock(
    SPI_PORT port,
    void*    inputData,
    uint32_t inputSize,
    void*    pbuf,
    uint32_t size,
    uint8_t  dataLength)
{
    uint32_t i       = 0;
    uint8_t  dataLen = dataLength - 1;
    int32_t  result  = 0;

	/* Lock */
    //ithLockMutex(ithStorMutex);

	if (ithGetDeviceId() == 0x9070)
		SetPadSel(port);

    // Clear rx tx FIFO
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 4);
	Patch_Sleep();
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

#ifdef __OPENRTOS__
	// Flush DC
	ithFlushDCacheRange(inputData, inputSize);
	ithFlushDCacheRange(pbuf, size);
#endif

    if( dataLength == 8 ) // Datalen 8bits
    {
        uint8_t *binput = (uint8_t*)(inputData);
        uint8_t *pbBuf  = (uint8_t*)(pbuf);

		if((inputSize + size) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        // Write data to fifo
        for(i = 0; i < inputSize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(binput + i));
        }

        for(i = 0; i < size; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pbBuf + i));
        }
    }
    else if( dataLength == 16 ) // Datalen 16bits
    {
        uint16_t* winput     = (uint16_t*)inputData;
        uint16_t* pwbuf      = (uint16_t*)pbuf;
        uint32_t  winputSize = inputSize / 2;
        uint32_t  wsize      = size / 2;
        uint16_t  temp       = 0 ;

		if((winputSize + wsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        // Write data to fifo
        for(i = 0; i < winputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(winput + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(winput + i));
#endif
        }

		for(i = 0; i < wsize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(pwbuf + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pwbuf + i));
#endif
        }
    }
    else if(dataLength == 32) // Datalen 32bits
    {
        uint32_t *dwinput = (uint32_t*)(inputData);
        uint32_t *pdwbuf = (uint32_t*)(pbuf);
        uint32_t dwinputSize = inputSize/4;
        uint32_t dwsize = size/4;

		if((dwinputSize + dwsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}

        // Write data to fifo
        for(i = 0; i < dwinputSize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(dwinput + i));
        }

        for(i = 0; i < dwinputSize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pdwbuf + i));
        }
    }
    else
    {
        //TODO
        result = ERROR_SSP_FIFO_LENGTH_UNSUPPORT;
		goto end;
    }

    // Fire
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

    result = WaitContollerReady(port);
    WaitFifoReady(port, FIFO_TX, 0);

end:
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	/* Unlock */
	//ithUnlockMutex(ithStorMutex);

    return result;
}

int32_t
mmpSpiPioRead(
    SPI_PORT port,
    uint8_t *   inputData,
    uint32_t 	inputSize,
    uint8_t *   outputBuf,
    uint32_t 	size,
    uint8_t  	dataLength)

{
    uint32_t temp   = 0;
    uint32_t dummy  = -1;
    uint32_t i      = 0;
    uint8_t dataLen = dataLength - 1;
    int32_t result  = 0;
	
	//usleep(10 * 1000);
	//printf("BBBBBBBBBBBBBBBBBBBBBBBBBBBB\n");

	/* Lock */
	pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
    	SetPadSel(port);
	}
	
    // Clear rx tx FIFO
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,4);
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN | REG_BIT_TXDO_EN);

    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

#ifdef __OPENRTOS__
	// Flush DC
	ithFlushDCacheRange(inputData, inputSize);
	ithFlushDCacheRange(outputBuf, size);
#endif

    if(dataLength == 8) // Datalen 8bits
    {
        uint8_t *binput  = (uint8_t*)(inputData);
        uint8_t *boutput = (uint8_t*)(outputBuf);
		
		if((inputSize + size) > 16)
		{			
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}

        // Write data to fifo
        for(i = 0; i < inputSize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(binput + i));
        }

		for(i = 0; i < size; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, dummy);
        }

        // Fire
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

        // Wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port, FIFO_RX, inputSize + size);

        // Read dummy
        for(i = 0; i < inputSize; i++)
        {
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
        }
        for(i = 0; i < size; i++)
        {
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
            *(boutput+i) = (uint8_t)temp;			
        }
    }
    else if(dataLength == 16) // Datalen 16bits
    {
        uint16_t* winput     = (uint16_t*)inputData;
        uint16_t* woutput    = (uint16_t*)outputBuf;
        uint32_t  winputSize = inputSize / 2;
        uint32_t  wsize      = size / 2;


		if((winputSize + wsize) > 16)
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;			
			goto end;
		}

        // Write data to fifo
		for(i = 0; i < winputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(winput + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA+ port * REG_SSP_PORT_OFFSET, *(winput + i));
#endif
        }

        for(i = 0; i < wsize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, dummy);
        }

        // Fire
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

        // Wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port, FIFO_RX, winputSize+ wsize);

        // Read dummy
        for(i = 0; i < winputSize; i++)
        {
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);			
        }

		for(i = 0; i < wsize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
            temp = SWAP_ENDIAN16(temp);
            *(woutput+i) = (uint16_t)temp;			
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);			
            *(woutput+i) = (uint16_t)temp;
#endif
        }
		
    }
    else if(dataLength == 32) // Datalen 32bits
    {
        uint32_t* dwinput     = (uint32_t*)inputData;
        uint32_t* dwoutput    = (uint32_t*)outputBuf;
        uint32_t  dwinputSize = inputSize / 4;
        uint32_t  dwsize      = size / 4;
		uint32_t  temp        = 0;

		if( (dwinputSize + dwsize) > 16 )
		{
			result = ERROR_SSP_OVER_FIFO_COUNT;
			goto end;
		}
        // Write data to fifo
        for(i = 0; i < dwinputSize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(dwinput + i);
            temp = SWAP_ENDIAN32(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA+ port * REG_SSP_PORT_OFFSET, *(dwinput + i));
#endif        
        }

        for(i = 0; i < dwsize; i++)
        {
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, dummy);
        }

        // Fire
        ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

        // Wait ssp ready
        result = WaitContollerReady(port);
        result = WaitFifoReady(port, FIFO_RX, dwinputSize+ dwsize);
        // Read dummy
        for(i = 0; i < dwinputSize; i++)
        {
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);			
        }
        for(i = 0; i < dwsize; i++)
        {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
			temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
			temp = SWAP_ENDIAN32(temp);
			*(dwoutput+i) = (uint32_t)temp;			
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
			temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
			*(dwoutput+i) = (uint32_t)temp;
#endif           
        }
    }
    else
    {
        // TODO
		result = ERROR_SSP_FIFO_LENGTH_UNSUPPORT;
    }

end:
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);

    return result;
}

void
mmpSpiSetDiv(
    SPI_PORT port,
    int16_t  div)
{
	pthread_mutex_lock(&g_SpiContext[port].mutex);
    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, div, REG_MASK_SCLK_DIV);
    pthread_mutex_unlock(&g_SpiContext[port].mutex);
}

#if 0
void
mmpSpiSetMode(
    SPI_PORT port,
    SPI_MODE mode)
{
    uint32_t temp;

	pthread_mutex_lock(&SpiInternalMutex);
    temp = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
    temp &= ~(0x3);
    temp |= mode;
    ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, temp);
    pthread_mutex_unlock(&SpiInternalMutex);
}
#endif

void
mmpSpiSetMaster(
    SPI_PORT port)
{
    uint32_t temp;

	pthread_mutex_lock(&g_SpiContext[port].mutex);
    temp = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
    temp &= ~(0xC);
    temp |= 0x8;
    ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, temp);
    pthread_mutex_unlock(&g_SpiContext[port].mutex);
}

void
mmpSpiSetSlave(
    SPI_PORT port)
{
    uint32_t temp;

	pthread_mutex_lock(&g_SpiContext[port].mutex);
    temp = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
    temp &= ~(0xC);
    ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET, temp);
    pthread_mutex_unlock(&g_SpiContext[port].mutex);
}

#if 0
int
mmpSpiDmaTriggerRead(
    SPI_PORT	port,
    void*		pdes,
    int			size)
{
    int      result       = -1;
    uint8_t  dataLen      = size - 1;
	uint32_t srcWidth     = 0;
	uint32_t dstWidth     = 0;
	uint32_t burstSize    = 0;
   	uint32_t tx_threshold = 1;
	uint32_t rx_threshold = 1;

	/* Lock */
	pthread_mutex_unlock(&SpiInternalMutex);
    ithLockMutex(ithStorMutex);

	{
		uint32_t data;

		data = ithReadRegA(ITH_GPIO_BASE + 0x90);
		//GPIO9
		data &= (3<<18);
		data |= (2<<18);
		ithWriteRegA(ITH_GPIO_BASE + 0x90, data);
		//MUDEX
		data = ithReadRegA(ITH_GPIO_BASE + 0xD0);
		//GPIO9
		data &= 0x7;
		data |= 0x0086;
		ithWriteRegA(ITH_GPIO_BASE + 0xD0, data);
	}

    SetPadSel(port);

    if( DmaSpiSlaveChannel >= 0 )
    {
		/*
    	//32bits
		if((size & 0x3) == 0)
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 4;
			dstWidth = 4;
			burstSize = 1;
			dataLen = 31;
		}
		if((size & 0x1) == 0)	// 16bits
		{
			tx_threshold = 4;
			rx_threshold = 4;
			srcWidth = 2;
			dstWidth = 2;
			burstSize = 1;
			dataLen = 15;
		}
		else					// 8bits
		*/
		{
			tx_threshold = 1;
			rx_threshold = 1;
			srcWidth = ITH_DMA_WIDTH_8;
			dstWidth = ITH_DMA_WIDTH_8;
			burstSize = ITH_DMA_BURST_1; // 1
			dataLen = 7;
		}

		/* TXDOE */
		ithWriteRegMaskA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_TXDO_EN, REG_BIT_TXDO_EN);
		/* Set serial data length */
		ithWriteRegMaskA(REG_SSP_CONTROL_1 + port*REG_SSP_PORT_OFFSET, (dataLen << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
		/* SSP dma enable */
		ithWriteRegA(
			REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
			((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
			(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_DMA_EN |
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_RX_INTR_TH_EN |
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_OR_EN));

		switch ( port )
		{
		case SPI_0:
			ithDmaSetDstAddr(DmaSpiSlaveChannel, (uint32_t)pdes);
			break;

		case SPI_1:
			ithDmaSetDstAddr(DmaSpiSlaveChannel, (uint32_t)pdes);
			break;
		}

		ithDmaSetSrcParams(DmaSpiSlaveChannel, srcWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetDstParams(DmaSpiSlaveChannel, dstWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetTxSize(DmaSpiSlaveChannel, size);
		ithDmaSetBurst(DmaSpiSlaveChannel, burstSize);
	    switch(port)
		{
	    case SPI_0:
	    	ithDmaSetSrcAddr(DmaSpiSlaveChannel, AHB_SPI1_DATA_BASE);
	    	ithDmaSetRequest(DmaSpiSlaveChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
        case SPI_1:
        	ithDmaSetSrcAddr(DmaSpiSlaveChannel, AHB_SPI2_DATA_BASE);
            ithDmaSetRequest(DmaSpiSlaveChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
		}

        // Fire DMA
        ithDmaStart(DmaSpiSlaveChannel);

		//Fire SSP
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
    }

    /* Unlock */
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&SpiInternalMutex);

    return result;
}
#endif

void
mmpSpiSetControlMode(
    SPI_CONTROL_MODE controlMode)
{
    uint32_t mask = 0;
    uint32_t value = 0;

    pthread_mutex_lock(&SwitchMutex);
    //printf("SetSpiGpio:[%x,%x]\n",controlMode,ctrl_mode);
 
	if (controlMode != ctrl_mode)
	{
		_SwitchSpiGpioPin(controlMode);
		ctrl_mode = controlMode;
	}
}

void
mmpSpiResetControl(
    void)
{
    pthread_mutex_unlock(&SwitchMutex);
    //nor_using = false;
}

void
mmpSpiInitShareGpioPin(
    unsigned int *pins)
{
    unsigned int i;
    
    printf("init SPI share pin:[%x]\n",ITH_COUNT_OF(pins));
    //ithWriteRegMaskA( REG_SSP_FREERUN, (0x1 << 2), (0x1 << 2));
    pthread_mutex_init(&SwitchMutex, NULL);

    for (i = 0; i < SPI_SHARE_GPIO_MAX; i++)
	{
		if( (pins[i]<64) && (pins[i]>0) )	gShareGpioTable[i] = pins[i];
		else	gShareGpioTable[i] = 0;
		
		printf("pin[%x]=[%d,%d]\n",i,pins[i],gShareGpioTable[i]);
	}

    if(gShareGpioTable[SPI_CONTROL_NAND])	gSpiCtrlMethod = SPI_CONTROL_NAND;
	printf("gSpiCtrlMethod = %x\n",gSpiCtrlMethod,SPI_CONTROL_NAND);
}

#if 1
uint32_t mmpSpiDmaTransfer(
	SPI_PORT port,
	void* tx_buf,
	void* rx_buf,
	uint32_t buflen)
{
	uint32_t 	i       = 0;    
    int32_t  	result  = -1;
    uint8_t*	tempBuf = NULL;
    uint8_t*    tempRxBuf = NULL;
    uint32_t    data    = 0;
    uint32_t    temp   = 0;
	uint32_t    tx_threshold = 0;
    uint32_t    rx_threshold = 0;
	ITHDmaWidth srcWidth     = ITH_DMA_WIDTH_8;
    ITHDmaWidth dstWidth     = ITH_DMA_WIDTH_8;
    ITHDmaBurst burstSize    = ITH_DMA_BURST_1;
	uint8_t     dataLen      = 7;
	
	//ITHDmaWidth rxsrcWidth     = ITH_DMA_WIDTH_8;
    //ITHDmaWidth rxdstWidth     = ITH_DMA_WIDTH_8;
    //ITHDmaBurst rxburstSize    = ITH_DMA_BURST_1;
	//uint8_t     rxdataLen      = 7;
   
  
    /* Lock */
	pthread_mutex_lock(&g_SpiContext[port].mutex);
	
	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
		SetPadSel(port);
	}
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);


	if ((((uint32_t)tx_buf | buflen) & 0x3) == 0)
	{
		srcWidth     = ITH_DMA_WIDTH_32;
	    dstWidth     = ITH_DMA_WIDTH_32;
	    burstSize    = ITH_DMA_BURST_4;
		dataLen      = 31;
		tx_threshold = 4;
		rx_threshold = 1;
	}	
	else if ((((uint32_t)tx_buf | buflen) & 0x3) == 1)
	{
		srcWidth     = ITH_DMA_WIDTH_8;
	    dstWidth     = ITH_DMA_WIDTH_8;
	    burstSize    = ITH_DMA_BURST_4;
		dataLen      = 7;		
		tx_threshold = 4;
		rx_threshold = 1;
	}
	else if ((((uint32_t)tx_buf | buflen) & 0x3) == 2)
	{
		srcWidth     = ITH_DMA_WIDTH_16;
	    dstWidth     = ITH_DMA_WIDTH_16;
	    burstSize    = ITH_DMA_BURST_4;
		dataLen      = 15;
		tx_threshold = 4;
		rx_threshold = 1;
	}
	else
	{
		srcWidth     = ITH_DMA_WIDTH_8;
	    dstWidth     = ITH_DMA_WIDTH_8;
	    burstSize    = ITH_DMA_BURST_4;
		dataLen      = 7;
		tx_threshold = 4;
		rx_threshold = 1;		
	}

	//Big Enddian
	data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
	data |= REG_BIT_TXFIFO_BIG_END|REG_BIT_RXFIFO_BIG_END;			
	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);

/*	
	if ((((uint32_t)rx_buf | buflen) & 0x3) == 0)
	{
		rxsrcWidth     = ITH_DMA_WIDTH_32;
	    rxdstWidth     = ITH_DMA_WIDTH_32;
	    rxburstSize    = ITH_DMA_BURST_1;
		rxdataLen      = 31;		
        rx_threshold   = 1;
		printf("-- Rx 32bit setting --\n");
	}
	
	else if ((((uint32_t)rx_buf | buflen) & 0x3) == 1)
	{
		rxsrcWidth     = ITH_DMA_WIDTH_8;
	    rxdstWidth     = ITH_DMA_WIDTH_8;
	    rxburstSize    = ITH_DMA_BURST_1;
		rxdataLen      = 7;		
		rx_threshold   = 1;
		printf("-- Rx 8bit setting --\n");
	}
	else if ((((uint32_t)rx_buf | buflen) & 0x3) == 2)
	{
		rxsrcWidth     = ITH_DMA_WIDTH_16;
	    rxdstWidth     = ITH_DMA_WIDTH_16;
	    rxburstSize    = ITH_DMA_BURST_1;
		rxdataLen      = 15;
		rx_threshold   = 1;
		printf("-- Rx 16bit setting --\n");
	}
	else
	{
		rxsrcWidth     = ITH_DMA_WIDTH_8;
	    rxdstWidth     = ITH_DMA_WIDTH_8;
	    rxburstSize    = ITH_DMA_BURST_1;
		rxdataLen      = 7;
		
		rx_threshold   = 1;
		printf("-- Rx 8bit setting --\n");
	}
*/
	if ((g_SpiContext[port].dma_ch >= 0) && (g_SpiContext[port].dma_slave_ch >= 0))
	{
				
		ithDmaSetSrcAddr(g_SpiContext[port].dma_ch, (uint32_t)tx_buf);
		ithDmaSetSrcParams(g_SpiContext[port].dma_ch, srcWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetDstParams(g_SpiContext[port].dma_ch, dstWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetTxSize(g_SpiContext[port].dma_ch, buflen);
		ithDmaSetBurst(g_SpiContext[port].dma_ch, burstSize);

		switch(port)
		{
		case SPI_0:
			ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI1_DATA_BASE);		
			ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_TX);
			break;
		case SPI_1:
			ithDmaSetDstAddr(g_SpiContext[port].dma_ch, AHB_SPI2_DATA_BASE);
			ithDmaSetRequest(g_SpiContext[port].dma_ch, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_TX);
			break;
		}

		//SET RX DMA
		ithDmaSetDstAddr(g_SpiContext[port].dma_slave_ch, (uint32_t)rx_buf);
		ithDmaSetSrcParams(g_SpiContext[port].dma_slave_ch, srcWidth, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetDstParams(g_SpiContext[port].dma_slave_ch, dstWidth, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetTxSize(g_SpiContext[port].dma_slave_ch, buflen);
		ithDmaSetBurst(g_SpiContext[port].dma_slave_ch, ITH_DMA_BURST_1);
	    switch(port)
		{
	    case SPI_0:
	    	ithDmaSetSrcAddr(g_SpiContext[port].dma_slave_ch, AHB_SPI1_DATA_BASE);
	    	ithDmaSetRequest(g_SpiContext[port].dma_slave_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
        case SPI_1:
        	ithDmaSetSrcAddr(g_SpiContext[port].dma_slave_ch, AHB_SPI2_DATA_BASE);
            ithDmaSetRequest(g_SpiContext[port].dma_slave_ch, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SSP2_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
	        break;
		}
		
		// Clear TX/RX FIFO
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN);

		// Free run
		ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET,REG_BIT_FR_CSN_CLR | REG_BIT_FR_RX_LOCK);

#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tx_buf, buflen);
#endif

#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(rx_buf, buflen);
#endif


		// Fire DMA
		ithDmaStart(g_SpiContext[port].dma_ch);
		ithDmaStart(g_SpiContext[port].dma_slave_ch);

		ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (31 << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

		// SSP dma enable
		ithWriteRegA(REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
			((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
			(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
			REG_BIT_RX_INTR_OR_EN |
			REG_BIT_TX_INTR_UR_EN |
			REG_BIT_RX_INTR_TH_EN |
			REG_BIT_TX_INTR_TH_EN |
			REG_BIT_TX_DMA_EN |
			REG_BIT_RX_DMA_EN));

		// Fire SSP
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);
		result = WaitDmaIdle(g_SpiContext[port].dma_ch);		 
		if( result == 0 )
		{
			/* DMA fail */
			printf("TX DMA FAIL\n");
			goto dma_end;
		}

		result = WaitDmaIdle(g_SpiContext[port].dma_slave_ch);		 
		if( result == 0 )
		{
			/* DMA fail */
			printf("RX DMA FAIL\n");
			goto dma_end;
		}

		// Wait ssp ready
		result = WaitContollerReady(port);

		ithInvalidateDCacheRange(rx_buf, buflen);
    	
	}
			
	dma_end:
	
	//SSP dma disable
	ithWriteRegA(
		REG_SSP_INTR_CONTROL + port * REG_SSP_PORT_OFFSET,
		((tx_threshold << REG_SHIFT_TX_THRESHOLD) |
		(rx_threshold << REG_SHIFT_RX_THRESHOLD) |
		REG_BIT_RX_INTR_OR_EN |
		REG_BIT_TX_INTR_UR_EN |
		REG_BIT_RX_INTR_TH_EN |
		REG_BIT_TX_INTR_TH_EN));

	// Reset enddian
	data = ithReadRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET);
	data &= ~(REG_BIT_RXFIFO_BIG_END | REG_BIT_TXFIFO_BIG_END);
	ithWriteRegA(REG_SSP_CONTROL_0 + port * REG_SSP_PORT_OFFSET,data);
	
	//Free run
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0x0);
	
	/* Unlock */
	if (ithGetDeviceId() == 0x9070)
	ithUnlockMutex(ithStorMutex);
	pthread_mutex_unlock(&g_SpiContext[port].mutex);
	return result;
	    
}

uint32_t mmpSpiPioTransfer(
	SPI_PORT port,
	void* tx_buf,
	void* rx_buf,
	uint32_t buflen)
{
    uint32_t i       = 0;    
    int32_t  result  = 0;
    uint32_t temp   = 0;
	uint32_t size =0;


	/* Lock */
	pthread_mutex_lock(&g_SpiContext[port].mutex);

	if (ithGetDeviceId() == 0x9070)
	{
		ithLockMutex(ithStorMutex);
		SetPadSel(port);
	}
    // Clear rx tx FIFO
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 4);
    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_RXFCLR_EN | REG_BIT_TXFCLR_EN | REG_BIT_TXDO_EN);

	if ((((uint32_t)tx_buf | buflen)& 0x3) == 0)
	{
    	uint32_t *pbBuf  = (uint32_t*)(tx_buf);
    	uint32_t *boutput = (uint32_t*)(rx_buf);
		uint32_t temp = 0;
		size = buflen/4;
		
	    ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (31 << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);
#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tx_buf, buflen);
#endif

	    //Write data to fifo
	    for(i = 0; i < size; i++)
	    {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(pbBuf + i);
            temp = SWAP_ENDIAN32(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pbBuf + i));
#endif		    	        
	    }

		// Fire
	    ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

	    result = 0;//WaitContollerReady(port);
	    WaitFifoReady(port, FIFO_TX, 0);
		WaitFifoReady(port, FIFO_RX, size);
		
	  	for(i = 0; i < size; i++)
	    {
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
			temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
			temp = SWAP_ENDIAN32(temp);
			*(boutput+i) = (uint32_t)temp;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
			temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
			*(boutput+i) = (uint32_t)temp;			
#endif		
			
	    }
	}	
	else if ((((uint32_t)tx_buf | buflen)& 0x3) == 2)
	{
    	uint16_t *pbBuf  = (uint16_t*)(tx_buf);
    	uint16_t *boutput = (uint16_t*)(rx_buf);
		uint16_t temp = 0;
		size = buflen/2;
		
		ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (15 << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tx_buf, buflen);
#endif

		//Write data to fifo
		for(i = 0; i < size; i++)
		{
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = *(pbBuf + i);
            temp = SWAP_ENDIAN16(temp);
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, temp);
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pbBuf + i));
#endif					
		}

		// Fire
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

		result = 0;//WaitContollerReady(port);
		WaitFifoReady(port, FIFO_TX, 0);
		WaitFifoReady(port, FIFO_RX, size);

		for(i = 0; i < size; i++)
		{
#if (defined(_WIN32) || defined(__OPENRTOS__)) && !defined(CFG_CHIP_PKG_IT9910)
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
            temp = SWAP_ENDIAN16(temp);
            *(boutput+i) = (uint16_t)temp;
#elif defined(__FREERTOS__) || defined(CFG_CHIP_PKG_IT9910)
            temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
            *(boutput+i) = (uint16_t)temp;
#endif		
		}

	}
	else
	{
		uint8_t *pbBuf  = (uint8_t*)(tx_buf);
		uint8_t *boutput = (uint8_t*)(rx_buf);
		size = buflen;
		
		ithWriteRegMaskA(REG_SSP_CONTROL_1 + port * REG_SSP_PORT_OFFSET, (7 << REG_SHIFT_SERIAL_DATA_LEN), REG_MASK_SERIAL_DATA_LEN);

#ifdef __OPENRTOS__
		// Flush DC
		ithFlushDCacheRange(tx_buf, buflen);
#endif

		//Write data to fifo
		for(i = 0; i < size; i++)
		{
			ithWriteRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET, *(pbBuf + i));
		}

		// Fire
		ithWriteRegA(REG_SSP_CONTROL_2 + port * REG_SSP_PORT_OFFSET, REG_BIT_SSP_EN);

		result = 0;//WaitContollerReady(port);
		WaitFifoReady(port, FIFO_TX, 0);
		WaitFifoReady(port, FIFO_RX, size);

		for(i = 0; i < size; i++)
		{
			temp = ithReadRegA(REG_SSP_DATA + port * REG_SSP_PORT_OFFSET);
			*(boutput+i) = (uint8_t)temp;
		}


	}
//end:
	ithWriteRegA(REG_SSP_FREERUN + port * REG_SSP_PORT_OFFSET, 0);

	/* Unlock */
	if (ithGetDeviceId() == 0x9070)		
	ithUnlockMutex(ithStorMutex);	
	pthread_mutex_unlock(&g_SpiContext[port].mutex);
    return result;
}


uint32_t mmpSpiTransfer(
	SPI_PORT port,	
	void* tx_buf,
	void* rx_buf,
	uint32_t buflen
)
{
	uint32_t result;

	if (buflen > 16)	
		result = mmpSpiDmaTransfer(port, tx_buf, rx_buf, buflen);	
	else
		result = mmpSpiPioTransfer(port, tx_buf, rx_buf, buflen);
	
	return result;
}
#endif

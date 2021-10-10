#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"

#define IIC_BUFFER_LEN	1024 * 1024
static uint8_t	IicMasterWriteBuffer[IIC_BUFFER_LEN] = {0};
static uint8_t	IicSlaveReceviceBuffer[IIC_BUFFER_LEN] = {0};
static uint8_t	buf[256] = {0};
static	int       gMasterDev = 0;
static	int       gSlaveDev = 0;

static void _TestMasterWriteSlave_IicReceiveCallback(uint8_t* recvBuffer, uint32_t recvBufferSize)
{
	int i = 0;

	if (recvBufferSize != IIC_BUFFER_LEN)
	{
		printf("[ERROR] recvBufferSize(%d) != %d\n", recvBufferSize, IIC_BUFFER_LEN);
	}

	memcpy(IicSlaveReceviceBuffer, recvBuffer, recvBufferSize);
	if (memcmp(IicMasterWriteBuffer, IicSlaveReceviceBuffer, IIC_BUFFER_LEN) == 0)
	{
		printf("Data compare ok.\n");
	}
	else
	{
		int printCount = 0;

		printf("Data compare fail.\n");
		for (i = 0; i < IIC_BUFFER_LEN; i++)
		{
			if (IicMasterWriteBuffer[i] != IicSlaveReceviceBuffer[i])
			{
				printf("[%d]    M(0x%02X) <==> S(0x%02X)\n", i, IicMasterWriteBuffer[i], IicSlaveReceviceBuffer[i]);
				if (printCount++ >= 10)
				{
					break;
				}
			}
		}
	}
}

static int _TestMasterWriteSlave_IicWriteCallback()
{
	return 0;
}

static void _TestMasterReadSlave_IicReceiveCallback(uint8_t* recvBuffer, uint32_t recvBufferSize)
{
	if (recvBuffer[0] == 0xFE)
	{
		printf("IIC Slave receive write cmd(0x%02X)\n", recvBuffer[0]);
	}
}

static int _TestMasterReadSlave_IicWriteCallback(IIC_DEVICE dev)
{
	static uint32_t ii = 0;
	
	//uint8_t		buf[256] = {0};
	uint32_t	i = 0;
	bool		result = false;

	printf("IIC Slave output...\n");

	for(i = 0; i < 256; i++)
	{
		buf[i] = 'a' + i + ii;
	}
	
	result = mmpIicSlaveWrite(IIC_PORT_1, 0x77, buf, 256);
	assert(result == true);
	ii++;
}


void
TestMasterWriteSlave()
{
	bool iicResult = false;
	int loop = 5;

	mmpIicInitialize(IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 400 * 1000, 0);
	mmpIicInitialize(IIC_PORT_1, IIC_SLAVE_MODE, 0, 0, 400 * 1000, 0);

	iicResult = mmpIicSetSlaveModeCallback(IIC_PORT_1, 0x77, _TestMasterWriteSlave_IicReceiveCallback, _TestMasterWriteSlave_IicWriteCallback);
	assert(iicResult == true);

	while(loop--)
	{
		uint32_t i = 0;

		for (i = 0; i < IIC_BUFFER_LEN; i++)
		{
			IicMasterWriteBuffer[i] = '0' + loop + i;
			//IicMasterWriteBuffer[i] = i;
		}
		printf("Data out 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
				IicMasterWriteBuffer[0],
				IicMasterWriteBuffer[1],
				IicMasterWriteBuffer[2],
				IicMasterWriteBuffer[3],
				IicMasterWriteBuffer[4],
				IicMasterWriteBuffer[5],
				IicMasterWriteBuffer[6],
				IicMasterWriteBuffer[7],
				IicMasterWriteBuffer[8],
				IicMasterWriteBuffer[9]);

		iicResult= mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, 0x77, IicMasterWriteBuffer, IIC_BUFFER_LEN);
		assert(iicResult == true);
		printf("Sleep for next!\n");
		usleep(30 * 1000 * 1000);
	}
}

void
TestMasterReadSlave()
{
	bool iicResult = false;
	ITPI2cInfo evt;

	iicResult = mmpIicSetSlaveModeCallback(IIC_PORT_1, 0x77, _TestMasterReadSlave_IicReceiveCallback, _TestMasterReadSlave_IicWriteCallback);
	mmpIicSetSlaveMode(IIC_PORT_1, 0x77);
	assert(iicResult == true);

	while(1)
	{
		bool		result = false;
		uint8_t		cmd = 0xFE;
		uint8_t		recvBuffer[256] = {0};
		uint32_t	i;

		usleep(500000);
		evt.slaveAddress   = 0x77;
	    evt.cmdBuffer      = &cmd;
	    evt.cmdBufferSize  = 1;
	    evt.dataBuffer     = recvBuffer;
	    evt.dataBufferSize = 256;	
	    
	    result = read(gMasterDev, &evt, 1);
		//result = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, 0x77, &cmd, 1, recvBuffer, 256);

		printf("Master received:\n");
		for (i = 0; i < 256; i++)
		{
			printf("0x%02X ", recvBuffer[i]);
			if(buf[i] != recvBuffer[i])
			{
				printf("data compare error(0x%x!=0x%x)!!!\n", buf[i], recvBuffer[i]);
				break;
			}
			else if(i == 255)
				printf("IIC master read slave data compare ok, test success\n");
		}
		break;
	}
}

uint32_t DoTest(void)
{
	const uint8_t TOUCH_DEVICE_ID = 0x48;

    bool result;
    uint8_t cmd, measureX;

	mmpIicInitialize(IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000, 0);

	while(1)
	{
		static uint32_t	count = 0;

		//active x driver
	    cmd = 0x82;
	    result = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, TOUCH_DEVICE_ID, &cmd, 1);
		assert(result == true);

	    //measure x
	    cmd = 0xC2;
	    result = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, TOUCH_DEVICE_ID , &cmd, 1, &measureX, 1);
	    assert(result == true);

		printf("%d", measureX);
		if (count && ++count % 16 == 0)
		{
			printf("\n");
		}
		else
		{
			printf(" ");
		}
	}

    return result;
}

/* Test with touch IC(ZT2083) */
bool
DoTest2(void)
{
	const uint8_t SLAVE_ADDR = (0x98>>1);

    bool		result  = 0;
	uint8_t		regAddr = 0;
	uint8_t		buf[8]  = {0};

	mmpIicInitialize(IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000, 0);

    regAddr = 0x09;	// HDMI REG_INT_MASK1
	buf[0] = 0x09;
	result = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1);
    if(result == false)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("Write 0x%X to regAddr 0x%X\n", buf[0], regAddr);
	}

    //measure x
    regAddr = 0xC6;	// HDMI REG_INT_MASK1
    result = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1, buf, 1);
    if(result == false)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("The value read from 0x%X is 0x%X\n", regAddr, buf[0]);
	}

    return result;
}

/* Test Interrupt */
bool
DoTest3(void)
{
	const uint8_t SLAVE_ADDR = 0x48;

    bool		result  = 0;
	uint8_t  	regAddr = 0;
	uint8_t		buf[8]  = {0};

	/* Enable interrupt in AMBA */
	ithWriteRegMaskA(0xDE200000 + 0x04, (1 << 19), (1 << 19));
	ithWriteRegMaskA(0xDE200000 + 0x24, (1 << 19), (1 << 19));

	mmpIicInitialize(IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000, 0);

    regAddr = 0x82;	// HDMI REG_INT_MASK1
	buf[0] = 0xF8;
	result = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1);
    if(result == false)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("Write 0x%X to regAddr 0x%X\n", buf[0], regAddr);
	}

    //measure x
    regAddr = 0xC6;	// HDMI REG_INT_MASK1
    result = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &regAddr, 1, buf, 1);
    if(result == false)
    {
        printf("%s[%d]ERROR = %x \n", __FILE__, __LINE__, result);
    }
	else
	{
		printf("The value read from 0x%X is 0x%X\n", regAddr, buf[0]);
	}

    return result;
}

/* Test IIC slave, only work at 9910 */
#if 0
uint32_t DoTest4(void)
{
    uint32_t result = 0;

    printf("[IIC_TEST]DoTest4().\n");

	result = mmpIicInitialize(0, 0);
	if ( result )
	{
		printf("[IIC_TEST]mmpIicInitialize() fail, code = %d\n", result);
	}

	while( 1 )
	{
		#define INPUT_BUF_LENGTH	512

		uint8_t  inutBuffer[INPUT_BUF_LENGTH] = {0xFF};
		uint32_t i;

		result = mmpIicSlaveRead(IIC_PORT_0, inutBuffer, INPUT_BUF_LENGTH);
		if ( result )
		{
			printf("[IIC_TEST]Receive data......count = %d\n", result);
			for ( i = 0; i < result ; i++ )
			{
				printf("0x%02X ", inutBuffer[i]);
			}
			printf("\n");
		}
	}
}
#endif

/* Test ZT2083, include interrupt */
uint32_t DoTest5(void)
{
	const uint8_t SLAVE_ADDR = 0x48;

	uint32_t regValue;
	uint8_t  iicCmd;

	/* 1. Enable GPIO48 to input mode */
	regValue = ithReadRegA(ITH_GPIO_BASE + 0x98);
	printf("regValue = 0x%08X, GPIO44 mode select = %d\n", regValue, (regValue & 0x03000000) >> 24);
	regValue = ithReadRegA(ITH_GPIO_BASE + 0x48);
	printf("regValue = 0x%08X, GPIO44 data set = %d\n", regValue, (regValue & 0x1000) >> 12);

	/* 2. Init IIC */
	mmpIicInitialize(IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000, 0);
	
    iicCmd = 0x86;
    mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &iicCmd, 1);

	/* 3. Polling and get value */
	while(1)
	{
		uint8_t slaveReg;
		
		regValue = ithReadRegA(ITH_GPIO_BASE + 0x44);
		if ( !(regValue & 0x1000) )
		{
			uint8_t buf[4];

			/* Get X */
			iicCmd = 0x86;
		    mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &iicCmd, 1);
			slaveReg = 0xC6;
			mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &slaveReg, 1, &buf[0], 1);

			/* Get Y */
			iicCmd = 0x96;
		    mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, SLAVE_ADDR, &iicCmd, 1);
			slaveReg = 0xD6;
			mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE,  SLAVE_ADDR, &slaveReg, 1, &buf[1], 1);

			printf("(%03d, %03d)\r", buf[0], buf[1]);
		}
		else
		{
			printf("(%03d, %03d)\r", 0, 0);
		}
	}

	return 0;
}

void* TestFunc(void* arg)
{
    //itpInit();
    IIC_OP_MODE iic_port0_mode = IIC_MASTER_MODE;
	IIC_OP_MODE iic_port1_mode = IIC_SLAVE_MODE;
#ifdef CFG_I2C0_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C0, &itpDeviceI2c0);
	gMasterDev = open(":i2c0", 0);
    ioctl(ITP_DEVICE_I2C0, ITP_IOCTL_INIT, (void*)iic_port0_mode);
#endif

    // init i2c1 device
#ifdef CFG_I2C1_ENABLE
    itpRegisterDevice(ITP_DEVICE_I2C1, &itpDeviceI2c1);
	gSlaveDev = open(":i2c1", 0);
    ioctl(ITP_DEVICE_I2C1, ITP_IOCTL_INIT, (void*)iic_port1_mode);
#endif
	TestMasterReadSlave();

	return NULL;
}

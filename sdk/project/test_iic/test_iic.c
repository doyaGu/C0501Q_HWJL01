/* It's a special example for indoor machine with touch IC "IT7260". 			*/
/* You will learn how I2C itp driver read/write I2C slave device.	 		  	*/
/* You will learn I2C read operation in _readQueryBuffer() & _readPointBuffer().*/
/* You will also learn I2C write operation in _writeCommandBuffer().			*/

#include <sys/ioctl.h>
#include <stdio.h>
#include "ite/itp.h"


/********************************************************************************
 * MACRO defination
 ********************************************************************************/
const unsigned char I2C_SLAVE_ADDR = 0x46; 



/********************************************************************************
 * global variable
 ********************************************************************************/







/********************************************************************************
 * private function 
 ********************************************************************************/
int _readQueryBuffer(int fd, unsigned char *dBuf)
{
	int 			i2cret;
	ITPI2cInfo		dev;
	unsigned char	addr;

	
	addr = 0x80;	//1000 0010		
	dev.slaveAddress   = I2C_SLAVE_ADDR;
	dev.cmdBuffer      = &addr;
	dev.cmdBufferSize  = 1;
	dev.dataBuffer     = dBuf;
	dev.dataBufferSize = 1;	
	
	i2cret = read(fd, &dev, 1);
	if(i2cret<0)
	{
		printf("[IIC ERROR] iic read fail\n");
		return -1;		
	}
	
	return 0;
}

int _readPointBuffer(int fd, unsigned char *dBuf)
{
	int 			i2cret;
	ITPI2cInfo 		dev;
	unsigned char	addr;


	addr = 0xE0;
	dev.slaveAddress   = I2C_SLAVE_ADDR;
	dev.cmdBuffer      = &addr;
	dev.cmdBufferSize  = 1;
	dev.dataBuffer     = dBuf;
	dev.dataBufferSize = 14;	
	
	i2cret = read(fd, &dev, 1);
	
	if(i2cret<0)
	{
		printf("[IIC ERROR].Read point buffer fail\n");
		return -1;
	}
	
	return 0;
}

int _writeCommandBuffer(int fd, unsigned char *wBuf, int wLen)
{
	ITPI2cInfo 		dev;
	int 			i2cret;

	dev.slaveAddress   = I2C_SLAVE_ADDR;
	dev.cmdBuffer      = wBuf;
	dev.cmdBufferSize  = wLen;
		
	i2cret = write(fd, &dev, 1);
	
	usleep(200);
	
	if(i2cret<0)
	{
		printf("[IIC ERROR] write command fail\n");
		return -1;		
	}
	
	return 0;
}

/********************************************************************************
 * I2C example 
 ********************************************************************************/
/* 1.open IIC device for device ID 						                        */
/* 2.write reset command (0x07) for IIC write example 					    	*/
/* 1.read query buffer for IIC read example             						*/
void RUN_I2C_Example(void)
{
	int	i2cret;
	int	fd;
	
	/* 1. open IIC device for device ID */
	fd = open(":i2c", O_RDWR, 0);
	if ( fd == -1 )
	{
		printf("[IIC ERROR] open iic device fail!\n");
        goto end;    
	}
	
	/* 2. write reset command (0x07) for IIC write example */
	{
		unsigned char	wBuf[2];
		
		wBuf[0]=0x20;	//command buffer address 0x20 of IT7260
		wBuf[1]=0x07;	//reset command 0x07 of IT7260
		
		i2cret = _writeCommandBuffer(fd, &wBuf[0], 2);
		if(i2cret<0)
		{
			printf("[IIC ERROR] write command buffer fail!\n");
			goto end;
		}
	}
		
	/* 3. read query buffer for IIC read example */
	while(1)
	{
		unsigned char	qBuf=0xFF;
		
		i2cret = _readQueryBuffer(fd, &qBuf);
		if(i2cret<0)
		{
			printf("[IIC ERROR] Read Query buffer fail\n");
			goto end;
		}
		
		printf("Query buffer=%x:",qBuf);
		
		switch ( qBuf )
		{
			case 0x00:
				printf("(No finger touched)\n");
				break;
				
			case 0x80:
			case 0x88:
				printf("(New point information)\n");
				{
					unsigned char ptBuf[14]={0};
					i2cret = _readPointBuffer(fd, &ptBuf[0]);
					if(i2cret<0)
					{
						printf("[IIC ERROR] Read pointer buffer fail\n");
						goto end;
					}
					printf("	pointer Bufffer = [%x,%x,%x,%x]\n",ptBuf[0],ptBuf[2],ptBuf[3],ptBuf[4]);
				}
				break;
				
			case 0x08:	
				printf("(No information, but finger is still touched)\n");
				break;
				
			default:
				printf("(got other touch case)\n");
				break;			
		}
		usleep(33000);	//read query buffer each 33ms
	}

end:
	printf("END of IIC example.\n");
	return;
}

void* TestFunc(void* arg)
{
	/* TO avoid the conflict of IIC & SPI bus */
	/* Removing SPI connector before countdown if they both occupied the GPIO2&3 */
	printf("! Ready to GO, 5 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 4 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 3 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 2 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 1 ...\n"); usleep(500000); usleep(500000);
	printf("! Ready to GO, 0 ...\n"); usleep(500000); usleep(500000);
	
    itpInit();

    RUN_I2C_Example();

	return NULL;
}

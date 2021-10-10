/*
 * Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_gpio/test_gpio_ex4.c
 *
 * @author Benson Lin
 * @version 1.0.0
 *
 * example code for setting GPIO output status
 */
#include "ite/itp.h"	//for all ith driver (include GPIO) & MACRO
#include <pthread.h>
#include <time.h>

static bool RunIoExQuit;
static sem_t IoExSem;

static void IOEXCallback(void* arg1, uint32_t arg2)
{	
	//don`t add any codes here.
	sem_post(&IoExSem);
}

void* TestFunc(void* arg)
{
	printf("GPIO test start~~~\n");
	uint8_t ReadBuf = 0;
	int32_t ret;
	int32_t mappingNumber = 0;
	ITHIOEXConfig IoExConfig = {0};
	int i = 0;
	
	IoExConfig.MappingCount= 8;
	IoExConfig.MappingGPIONum[0] = ITH_GPIO_PIN74;	
	IoExConfig.MappingGPIONum[1] = ITH_GPIO_PIN75;
	IoExConfig.MappingGPIONum[2] = ITH_GPIO_PIN76;	
	IoExConfig.MappingGPIONum[3] = ITH_GPIO_PIN77;	
	IoExConfig.MappingGPIONum[4] = ITH_GPIO_PIN78;
	IoExConfig.MappingGPIONum[5] = ITH_GPIO_PIN79;	
	IoExConfig.MappingGPIONum[6] = ITH_GPIO_PIN80;	
	IoExConfig.MappingGPIONum[7] = ITH_GPIO_PIN81;
	IoExConfig.EdgeTrigger = 1;
	IoExConfig.BothEdge= 1;

	RunIoExQuit	= false;
	itpInit();
	sem_init(&IoExSem, 0, 0);

#ifdef CFG_IOEX_ENABLE
	ioctl(ITP_DEVICE_IOEX, ITP_IOCTL_INIT, (void*)&IoExConfig);
	ioctl(ITP_DEVICE_IOEX, ITP_IOCTL_REG_IOEX_CB , (void*)IOEXCallback);
#endif

	usleep(1000*1000*1);
	while (!RunIoExQuit)
	{
		sem_wait(&IoExSem);
#ifdef CFG_IOEX_ENABLE
		ret = read(ITP_DEVICE_IOEX, &ReadBuf, sizeof(ReadBuf));	
#endif
		if(ret)
			printf("ret =0x%x,ReadBuf=0x%x,len=0x%x\n",ret,ReadBuf,sizeof(ReadBuf));
	}
	return NULL;
}

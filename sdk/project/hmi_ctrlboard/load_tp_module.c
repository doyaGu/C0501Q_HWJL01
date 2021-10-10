#ifdef	CFG_DYNAMIC_LOAD_TP_MODULE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ite/itp.h"
#include "SDL/SDL_touch.h"
#include "tslib.h"
#include "iic/mmp_iic.h"

/*####################################################################*/
/*#                   Set I2C Slave Address Array                    #*/
/*####################################################################*/
//This ID lists must match the "CFG_TOUCH_MODULE" sequence
static uint8_t slvAddrLists[]={0x46,          //it7260's slave address
                               0x5D,0x14,     //gt5688's slave address
                               0x38,          //ft5316's slave address
                               0x48,          //zt2083's slave address
                               0x76,          //zet6221's slave address
                              };

/*####################################################################*/
/*#                         Global Variables                         #*/
/*####################################################################*/
static int gDev;

/*####################################################################*/
/*#                        private functions                         #*/
/*####################################################################*/
/**
 * /try I2C slave address.
 *
 * @param id the I2C slave address
 *
 * @return 1 if I2C has ACK, 0 if NO ACK.
 */
static uint8_t tryTpByI2cSlaveAddr(uint8_t id)
{
    ITPI2cInfo *evt;
	unsigned char	I2cCmd;
	unsigned char	buf[4]={0};
    int i2cret;
    uint32_t    i2cResult = 0;
    
    printf("Slave ID:%x\n",id);

    evt = alloca(sizeof(ITPI2cInfo));
	
	I2cCmd = 0x00;
	evt->slaveAddress   = id;
	evt->cmdBuffer      = &I2cCmd;
	evt->cmdBufferSize  = 0;
	evt->dataBuffer     = buf;
	evt->dataBufferSize = 1;
	
	i2cret = read(gDev, evt, 1);
	
	if(i2cret<0)
	{
		printf("itp-Incorrect ID_1:%x\n",id);
		
		return 0;
	}
	if(evt->errCode == I2C_NON_ACK)
	{
		printf("itp-Incorrect ID_2:%x\n",id);		
		return 0;
	}
	
	printf("itp-Got Correct ID: %x, %x, %x\n",id,i2cret,buf[0]);
	return 1;
}

/**
 * /To Identify the TP module by I2C ACK of trying slave address.
 *
 * @param module the TP module name
 *
 * @return none.
 *
 * NOTE: It's a rough rule for identifying the TP module.
 *       This rule could not fit all conditions.
 *       It will fail if the same I2C slave addrres or other side effect.
 *       User should modify this function by case/project.
 */
static void DoTpModuleIdentify(char **module)
{
	char *buf, *ptr;
	char* str[8];
	char *s = " ";
	int mCnt=0, i = 0;
	uint8_t *mId = (uint8_t*)&slvAddrLists[0];
	uint8_t idCnt = sizeof(mId);
	uint8_t idIndex = 0;
	
	//0.open I2C device
#if defined(CFG_TOUCH_I2C0)
    gDev = open(":i2c0",0);
#elif defined(CFG_TOUCH_I2C1)
    gDev = open(":i2c1",0);
#endif // CFG_TOUCH_I2C
	if (gDev==-1)	perror("ctrlboard ts_open");		
		
	//1.count TP modules
	buf = (char*)malloc(128);
	strcpy(buf, CFG_CTRLBOARD_TOUCH_MODULE);
	ptr = strtok(buf,s);
	
	if(ptr == NULL)
	{
		*module = NULL;
		printf("TP initial ERROR:: incorrect module\n");
		return;
	}

    while (ptr != NULL)
    {
    	str[mCnt++] = ptr;
    	ptr = strtok(NULL, s);
    }

	printf("### TOUCH_MODULE:: ");
    for(i=0; i<mCnt; i++)	printf ("%s, ",str[i]);

	printf(" ####\n module count = %d, id count = %d\n", mCnt, idCnt);
	
	
	//2.if(count == 1) then load default module
	//(But this condition doesn't exist, because SDL won't call this callback)
	if(mCnt == 1)
	{
		*module = str[0];
		printf("only one module, return module: %s\n", *module);
		return;
	}	
	
	//3.try I2C slave ID
	for(i=0; i<idCnt; i++)
	{
		if(tryTpByI2cSlaveAddr(mId[i]))
		{
			idIndex = i+1;
			break;
		}
	}
			
    //4.To get the module name by index
	switch(idIndex)
	{
		case 1://0x46(it7260)
			*module = str[0];
			break;
		case 2://0x5D(gt5688)
		case 3://0x14
			*module = str[1];
		case 4://0x38(ft5316)
			*module = str[2];
			break;
		case 5://0x48(zt2083)
			*module = str[3];
			break;
		case 6://0x76(zet6221)
			*module = str[4];
			break;
		default:
			printf("TP can not be identified\n");
			*module = NULL;
			break;	
	}

	printf("return TP Module: %s, index = %d\n", *module, idIndex);
}

/*####################################################################*/
/*#                        public functions                          #*/
/*####################################################################*/

/**
 * /To assign the identify function to the callback pointer.
 *
 * @return none.
 */
void DynamicLoadTpModule(void)
{	
	lp_tp_module_identify_callback = DoTpModuleIdentify;
}
#endif //CFG_DYNAMIC_LOAD_TP_MODULE

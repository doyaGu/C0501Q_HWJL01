#include "rtl8309n_types.h"
#include "mdcmdio.h"      /*RTL8651B file*/
#include "ite/ite_mac.h"
#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int32 smiRead(uint32 phyad, uint32 regad, uint32 * data) 
{
    struct mii_ioctl_data phy_reg = { phyad, regad, 0, 0 };

    if (ithGetCpuMode() == ITH_CPU_SYS)
        pthread_mutex_lock(&mutex);

    iteMacIoctl(&phy_reg, IOCGMIIREG);
	(*data) = phy_reg.val_read;

    if (ithGetCpuMode() == ITH_CPU_SYS)
        pthread_mutex_unlock(&mutex);

    return SUCCESS ;
}

int32 smiWrite(uint32 phyad, uint32 regad, uint32 data) 
{
    struct mii_ioctl_data phy_reg = {phyad, regad, data, 0};

    if (ithGetCpuMode() == ITH_CPU_SYS)
        pthread_mutex_lock(&mutex);
    
    iteMacIoctl(&phy_reg, IOCSMIIREG);

    if (ithGetCpuMode() == ITH_CPU_SYS)
        pthread_mutex_unlock(&mutex);

    return SUCCESS;
}



/* Function Name:
 *      smiReadBit
 * Description:
 *      Read one bit of PHY register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31) 
 *      bit       -  Register bit (0~15)   
 * Output:
 *      pdata    - the pointer of  Register bit value 
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 */

int32 smiReadBit(uint32 phyad, uint32 regad, uint32 bit, uint32 * pdata) 
{
    uint32 regData;

    if ((phyad > 31) || (regad > 31) || (bit > 15) || (pdata == NULL) ) 
        return  FAILED;
    
    if(bit>=16)
        * pdata = 0;
    else 
    {
        smiRead(phyad, regad, &regData);
        if(regData & (1<<bit)) 
            * pdata = 1;
        else
            * pdata = 0;
    }
    return SUCCESS;
}

/* Function Name:
 *      smiWriteBit
 * Description:
 *      Write one bit of PHY register
 * Input:
 *      phyad   - PHY address (0~31)
 *      regad   -  Register address (0 ~31) 
 *      bit       -  Register bit (0~15)   
 *      data     -  Bit value to be written
 * Output:
 *      none
 * Return:
 *      SUCCESS         -  Success
 *      FAILED            -  Failure
 * Note:
 */

int32 smiWriteBit(uint32 phyad, uint32 regad, uint32 bit, uint32 data) 
{
    uint32 regData;
    
    if ((phyad > 31) || (regad > 31) || (bit > 15) || (data > 1) ) 
        return  FAILED;
    smiRead(phyad, regad, &regData);
    if(data) 
        regData = regData | (1<<bit);
    else
        regData = regData & ~(1<<bit);
    smiWrite(phyad, regad, regData);
    return SUCCESS;
}





















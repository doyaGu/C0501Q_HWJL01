/*
* Copyright (C) 2012 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: v1.0.1 $
* $Date: 2012-10-23 11:18:41 +0800 $
*
* Purpose : asic-level driver implementation for RTL8309N switch
*
* Feature :  This file consists of following modules:
*              1)	Packet length
*				2)	Phy
*				3)	Port isolation
*				4)	VLAN
*				5)	CPU port
*				6)	Qos
*				7)	ACL
*				8)	MIB
*				9)	Mirror
*				10) Lookup table
*				11) Spanning tree
*				12) Dot1x
*				13) IGMP
*				14)	Trap
*				15) RMA
*				16)	Interrupt
*				17)	Storm filter
*				18)	RLDP/RLPP
*				19)	ISP
*				20) LED
*/

#include <rtl8309n_types.h>
#include <rtl8309n_asicdrv.h>
#include <rtl8309n_asicdrv_ext.h>

/*for pc cle test*/

#ifndef RTK_X86_ASICDRV
#include <mdcmdio.h>
#else
extern int r_phy(int,int,int);
extern void w_phy(int,int,int,int);

#endif

#define RTL8309N_GET_REG_ADDR(x, page, phy, reg) \
    do { (page) = ((x) & 0xFF0000) >> 16; (phy) = ((x) & 0x00FF00) >> 8; (reg) = ((x) & 0x0000FF);\
    } while(0) \

#if 1 // Irene Lin
#include "ite/ith.h"

#define rtlglue_drvMutexLock()	ithEnterCritical()
#define rtlglue_drvMutexUnlock() ithExitCritical()
#endif

/* Function Name:
 *      rtl8309n_reg_set
 * Description:
 *      Write Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~17)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
int32 rtl8309n_reg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) 
{

#ifdef RTK_X86_ASICDRV

    uint32 rdata; 

    if ((phyad > RTL8309N_MAX_PORT_ID) || (npage >= RTL8309N_PAGE_NUMBER))
        return FAILED;

    /*Switch to phy register page first*/
    w_phy(32, 8, 31, 0x8000);
    
    /* write phy page to unavailable page */
    rdata = r_phy(32, phyad, 31);
    rdata &= ~0xFF;
    rdata |= 0xa;
    w_phy(32, phyad, 31, rdata);
    
    /*write mac page number*/
    rdata = r_phy(32, 8, 31); 
	rdata &= ~(0x1 << 15 | 0xff);
	rdata |= npage;
	w_phy(32, 8, 31, rdata);
    
    /*write mac register value*/
    value = value & 0xFFFF;
    w_phy(32,phyad, regad, value);

    return SUCCESS;

#else
    uint32 rdata; 

    if ((phyad > RTL8309N_MAX_PORT_ID) || (npage >= RTL8309N_PAGE_NUMBER))
        return FAILED;
	
    /*it lock the resource to ensure that reg read/write opertion is thread-safe, 
      *if porting to other platform, please rewrite it to realize the same function
      */
    rtlglue_drvMutexLock(); 
	
    /*switch to phy rigister first*/
    smiWrite(8, 31, 0x8000);
    /*write phy page to unavailable page*/
    smiRead(phyad, 31, &rdata);
    rdata &= ~0xFF;
    rdata |= 0xa;
    smiWrite(phyad, 31, rdata);
    
    /* switch to MAC page through configuring PHY 8 Register 31 [bit15] */
	smiRead(8, 31, &rdata);
	rdata &= ~(0x1 << 15 | 0xFF);
	rdata |= npage;
	smiWrite(8, 31, rdata);
    
	/* write mac register value */
	value &= 0xFFFF;
    smiWrite(phyad, regad, value); 
	
	/*unlock the source, enable interrupt*/    
    rtlglue_drvMutexUnlock();
	
    return SUCCESS;
    
#endif
}


/* Function Name:
 *      rtl8309n_reg_get
 * Description:
 *      Read Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad   - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~ 17)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
int32 rtl8309n_reg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue)
{

#ifdef RTK_X86_ASICDRV
    uint32 rdata;

    if ((phyad > RTL8309N_MAX_PORT_ID) ||(npage >= RTL8309N_PAGE_NUMBER))
        return FAILED;


    /* Select PHY Register Page through configuring PHY 8 Register 31 [bit15] */
    rdata = r_phy(32, 8, 31); 
	rdata &= ~(0x1 << 15 | 0xFF);
	rdata |= npage;
	w_phy(32, 8, 31, rdata);

    rdata = r_phy(32, phyad, regad);
    *pvalue = rdata & 0xFFFF;

    return SUCCESS;

#else
    uint32 rdata; 

    if ((phyad > RTL8309N_MAX_PORT_ID) || (npage >= RTL8309N_PAGE_NUMBER))
        return FAILED;

	/*it lock the resource to ensure that reg read/write opertion is thread-safe, 
      *if porting to other platform, please rewrite it to realize the same function
      */
    rtlglue_drvMutexLock(); 

    /* Select MAC or PHY page, configure PHY 8 Register 31 bit[15] */
	smiRead(8, 31, &rdata);
	rdata &= ~((0x1 << 15) | 0xFF);
	/* select mac page, configure phy 8 register 31 bit[0:7]*/
	rdata |= npage;
	smiWrite(8, 31, rdata);
	/* slelect phy and reg number, write data into register */
    smiRead(phyad, regad, &rdata);

	*pvalue = rdata & 0xFFFF;
	
	/*unlock the source, enable interrupt*/    
    rtlglue_drvMutexUnlock();
	
    return SUCCESS;
#endif
}


/* Function Name:
 *      rtl8309n_regbit_set
 * Description:
 *      Write one bit of Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~ 17)
 *      value    - Value to be write(0, 1)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function  you could write each bit of  all configurable registers of RTL8309N.
 */
int32 rtl8309n_regbit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value) 
{
    uint32 rdata;
    
    if ((phyad > RTL8309N_MAX_PORT_ID) || (npage >= RTL8309N_PAGE_NUMBER) ||
        (bit > 15) || (value > 1))
        return FAILED;
    
    rtl8309n_reg_get(phyad, regad,  npage, &rdata);
    if (value) 
        rtl8309n_reg_set(phyad, regad, npage, rdata | (1 << bit));
    else
        rtl8309n_reg_set(phyad, regad, npage, rdata & (~(1 << bit)));
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_regbit_get
 * Description:
 *      Read one bit of Asic  PHY Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~17)
 * Output:
 *      pvalue  - The pointer of value read back
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could read each bit of  all configurable registers of RTL8309N
 */
int32 rtl8309n_regbit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) 
{
    uint32 rdata;

    if ((phyad > RTL8309N_MAX_PORT_ID) || (npage >= RTL8309N_PAGE_NUMBER) || 
        (bit > 15) || (pvalue == NULL))
        return FAILED;
    
    rtl8309n_reg_get(phyad, regad, npage, &rdata);
    if (rdata & (0x1 << bit))
        *pvalue =1;
    else 
        *pvalue =0;

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_phyReg_set
 * Description:
 *      Write PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 7)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~ 17)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
int32 rtl8309n_phyReg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) 
{

#ifdef RTK_X86_ASICDRV
    uint32 regval; 

    if ((phyad > RTL8309N_MAX_PHY_ID) || (npage > RTL8309N_PAGE_NUMBER))
        return FAILED;

    /*switch to phy register first*/
    w_phy(32, 8, 31, 0x8000);
    
    /* write phy pagenumber */
    regval = r_phy(32, phyad, 31);
    regval &= ~0xFF;
    regval |= npage;
    w_phy(32, phyad, 31, regval);
    
    /* write phy register value */
    w_phy(32, phyad, regad, value);

    /*switch back to mac register*/
	regval = r_phy(32, 8, 31); 
	regval &= ~(0x1 << 15);
    w_phy(32, 8, 31, regval);
    
    return SUCCESS;

#else
    uint32 regval; 

    if ((phyad > RTL8309N_MAX_PHY_ID) || (npage > RTL8309N_PAGE_NUMBER ))
        return FAILED;

	/*it lock the resource to ensure that reg read/write opertion is thread-safe, 
      *if porting to other platform, please rewrite it to realize the same function
      */
    rtlglue_drvMutexLock();   

    /*choose PHY page through phy 8 reg31 bit 15 = 1*/
	smiWrite(8, 31, 0x8000);
    
	/*set phy page number*/
    smiRead(phyad, 31, &regval);
    regval &= ~0xff;
    regval |= npage;
    smiWrite(phyad, 31, regval);

    /*write register*/
	value &= 0xFFFF;
	smiWrite(phyad, regad, value);

    /*switch back to mac register*/
	smiRead(8, 31, &regval);
	regval &= ~(0x1 << 15);
    smiWrite(8, 31, regval);

    /*unlock the source, enable interrupt*/    
    rtlglue_drvMutexUnlock();
	
    return SUCCESS;
    
#endif
}

/* Function Name:
 *      rtl8309n_phyReg_get
 * Description:
 *      Read PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~7)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~17)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
int32 rtl8309n_phyReg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue)
{

#ifdef RTK_X86_ASICDRV
    uint32 regval;

    if ((phyad >= RTL8309N_MAX_PHY_ID) ||(npage > RTL8309N_PAGE_NUMBER))
        return FAILED;

    /*write/read pcs register*/
    w_phy(32, 8, 31, 0x8000);

    
    /* Set PHY Register Page number */
    regval = r_phy(32, phyad, 31);
    regval &= ~0xFF;
    regval |= npage;
    w_phy(32, phyad, 31, regval);

    *pvalue = r_phy(32, phyad, regad);
    *pvalue = *pvalue & 0xFFFF;

    regval = r_phy(32, 8, 31); 
	regval &= ~(0x1 << 15);
    w_phy(32, 8, 31, regval);

    
    return SUCCESS;

#else
    uint32 regval; 

    if ((phyad > RTL8309N_MAX_PHY_ID) || (npage > RTL8309N_PAGE_NUMBER ) ||
		(NULL == pvalue))
        return FAILED;
	
	/*it lock the resource to ensure that reg read/write opertion is thread-safe, 
      *if porting to other platform, please rewrite it to realize the same function
      */
    rtlglue_drvMutexLock();   

    /*choose PHY page through phy 8 reg31 bit 15 = 1*/
	smiWrite(8, 31, 0x8000);
    
	/*set phy page number*/
    smiRead(phyad, 31, &regval);
    regval &= ~0xff;
    regval |= npage;
    smiWrite(phyad, 31, regval);
    
    /*read phy register*/
	smiRead(phyad, regad, &regval);
	*pvalue = regval & 0xFFFF;
    
    /*switch back to mac register*/
    smiRead(8, 31, &regval);
	regval &= ~(0x1 << 15);
    smiWrite(8, 31, regval);

	/*unlock the source, enable interrupt*/    
    rtlglue_drvMutexUnlock();
	
	return SUCCESS;
  
#endif
}

int32 rtl8309n_phyRegBit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage, uint32 value)
{
    uint32 regVal;
    
    if ((phyad >= RTL8309N_MAX_PHY_ID) || (npage > RTL8309N_PAGE_NUMBER))
        return FAILED;    

    rtl8309n_phyReg_get(phyad, regad, npage, &regVal);
    
    rtl8309n_phyReg_set(phyad, regad, npage, value ? (regVal | (0x1 << bit)) : (regVal & ~(0x1 << bit)));

    return SUCCESS;
}

int32 rtl8309n_phyRegBit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage, uint32 *pvalue)
{
    uint32 regVal;

    if ((phyad >= RTL8309N_MAX_PHY_ID) || (npage > RTL8309N_PAGE_NUMBER) || (NULL == pvalue))
        return FAILED;

    rtl8309n_phyReg_get(phyad, regad, npage, &regVal);
    *pvalue = (regVal >> bit) & 0x1;

    return SUCCESS;
}

int32 rtl8309n_asic_init(void)
{
	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_phy_reset
 * Description:
 *      Reset the phy
 * Input:
 *      phy   - Specify Phy address (0 ~ 7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		RTL8309N has 7 PHYs from PHY 0 - 7.
 */
int32 rtl8309n_phy_reset(uint32 phy)
{ 
	uint32 bitVal,i;
    
	if(phy > RTL8309N_MAX_PHY_ID)
		return FAILED;

    rtl8309n_phyRegBit_set(phy, 0, 15, 0, 1);
	i = 0;
	do{
		rtl8309n_phyRegBit_get(phy, 0, 15, 0, &bitVal);
	}while(bitVal && (i++ < 100));
	if(i == 100)
		return FAILED;

	return SUCCESS;	
}


/*
int32 rtl8309n_phy_reset1(uint32 phy)
{
    uint32 regval, regval2;
    uint32 nway, pause, dupSpd;
    if (phy > 4)
        return FAILED;
    
    rtl8309n_regbit_set(0, 16, 11, 0, 1);
    rtl8309n_regbit_set(4, 23, 5, 0, 1); 
    rtl8309n_reg_get(4, 30, 0, &regval);
    rtl8309n_reg_get(4, 26, 0, &regval2);
    rtl8309n_regbit_set(0, 16, 11, 0, 0);
    rtl8309n_regbit_set(4, 23, 5, 0, 0); 
    if ((0x6167 == regval) && ((regval2 & (0x7<<13)) >> 13 == 1))
    {
        rtl8309n_regbit_get(phy, 22, 6, 0, &nway);
        rtl8309n_regbit_get(phy, 22, 3, 0, &pause);
        rtl8309n_reg_get(phy, 22, 0, &dupSpd);
          
        rtl8309n_regbit_set(phy, 0, 15, 0, 1);
        if (nway)
        {
            switch((dupSpd & (0x3 << 4)) >> 4)
            {
                case 0x3:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 1);
                    break;
                case 0x2:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 1);
                    break;
                case 0x1:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 1);
                    break;
                case 0x0:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 1);
                    break;
                default:
                   break;
            } 
            rtl8309n_regbit_set(phy, 0, 12, 0, 1);
            rtl8309n_regbit_set(phy, 0, 9, 0, 1);
        }
        else
        {
            rtl8309n_regbit_set(phy, 0, 12, 0, 0);
            switch((dupSpd & (0x3 << 4)) >> 4)
            {
                case 0x3:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 0);
                    break;
                case 0x2:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 0);
                    break;
                case 0x1:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 1);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 1);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 0);
                    break;
                case 0x0:
                    rtl8309n_regbit_set(phy, 0, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 0, 13, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 8, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 7, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 6, 0, 0);
                    rtl8309n_regbit_set(phy, 4, 5, 0, 1);
                    break;
                default:
                   break;
            }  
        }
        
        rtl8309n_regbit_set(phy, 4, 10, 0, pause);
        
    }
    else
        rtl8309n_regbit_set(phy, 0, 15, 0, 1);

    return SUCCESS;
}
*/

/* Function Name:
 *      rtl8309n_switch_maxPktLen_set
 * Description:
 *      set Max packet length which could be forwarded
 * Input:
 *		type		-  max packet length type
 *      maxLen      -  user defined max packet length
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8309N_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8309N_MAX_PKT_LEN_2048 - 2048 bytes (all tags counted);
 *      RTL8309N_MAX_PKT_LEN_USER - user defined (64~2048 bytes); 
 *      RTL8309N_MAX_PKT_LEN_2000 - 16k bytes (all tags counted) 
 *              
 */ 
int32 rtl8309n_switch_maxPktLen_set(uint32 type, uint32 maxLen)
{
	uint32 regVal;
	
    switch(type)
    {	
        case RTL8309N_MAX_PKTLEN_1518:
			rtl8309n_reg_get(7, 16, 17, &regVal);
			regVal &= ~(0x3 << 6);
			regVal |= 0x0 << 6;
			rtl8309n_reg_set(7, 16, 17, regVal);
            break;

        case RTL8309N_MAX_PKTLEN_2048:
			rtl8309n_reg_get(7, 16, 17, &regVal);
			regVal &= ~(0x3 << 6);
			regVal |= 0x1 << 6;
			rtl8309n_reg_set(7, 16, 17, regVal);          
            break;

		case RTL8309N_MAX_PKTLEN_USER:
			rtl8309n_reg_get(7, 16, 17, &regVal);
			regVal &= ~(0x3 << 6);
			regVal |= 0x2 << 6;
			rtl8309n_reg_set(7, 16, 17, regVal);
			rtl8309n_reg_set(7, 17, 17, maxLen & 0xFFF);
            break;
			
        case RTL8309N_MAX_PKTLEN_16k:
			rtl8309n_reg_get(7, 16, 17, &regVal);
			regVal &= ~(0x3 << 6);
			regVal |= 0x3 << 6;
			rtl8309n_reg_set(7, 16, 17, regVal);
            break;
            
        default:
            return FAILED;

    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_switch_maxPktLen_get
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      none
 * Output:
 *		pType	-	max packet length type
 *      pMaxLen         -  max packet length
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8309N_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8309N_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8309N_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8309N_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 
int32 rtl8309n_switch_maxPktLen_get(uint32 *pType, uint32 *pMaxLen)
{
    uint32 regVal, type;

    if ((NULL == pType) || (NULL == pMaxLen))
        return FAILED;
    
	rtl8309n_reg_get(7, 16, 17, &regVal);
	type = (regVal >> 6) & 0x3;
	switch(type)
	{
		case 0:
			*pType = RTL8309N_MAX_PKTLEN_1518;
			*pMaxLen = 1518;
			break;
			
		case 1:
			*pType = RTL8309N_MAX_PKTLEN_2048;
			*pMaxLen = 2048;
			break;

		case 2:
			*pType = RTL8309N_MAX_PKTLEN_USER;
			rtl8309n_reg_get(7, 17, 17, &regVal);
			*pMaxLen = regVal & 0xFFF;
			break;
			
		case 3:
			*pType = RTL8309N_MAX_PKTLEN_16k;
			*pMaxLen = 16000;
			break;

		default:
			break;
	}
   
    return SUCCESS;
}

#if 0
/* Function Name:
 *      rtl8309n_port_etherPhy_set
 * Description:
 *      Configure PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 *      autoNegotiation    - Specify whether enable auto-negotiation
 *      advCapability       - When auto-negotiation is enabled, specify the advertised capability
 *      speed                 - When auto-negotiation is disabled, specify the force mode speed
 *      fullDuplex            - When auto-negotiatoin is disabled, specify the force mode duplex mode
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode ,and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8309N_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8309N_ETHER_AUTO_100FULL and RTL8309N_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8309N_ETHER_SPEED_100 or RTL8309N_ETHER_SPEED_10.
 */
int32 rtl8309n_port_etherPhy_set(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) 
{
    uint32 ctrlReg;

    if(phy >= RTL8309N_MAX_PHY_ID || 
       advCapability < RTL8309N_ETHER_AUTO_100FULL ||
       advCapability > RTL8309N_ETHER_AUTO_10HALF ||
       (speed != 100 && speed != 10))
           return FAILED;

	/*set autonegotiation register, reg 4*/
    if(RTL8309N_ETHER_AUTO_100FULL == advCapability)
        rtl8309n_phyReg_set(phy, 4, 0, RTL8309N_CAPABLE_PAUSE | RTL8309N_CAPABLE_100BASE_TX_FD 
                                  | RTL8309N_CAPABLE_100BASE_TX_HD | RTL8309N_CAPABLE_10BASE_TX_FD 
                                  | RTL8309N_CAPABLE_10BASE_TX_HD | 0x1);
    else if(RTL8309N_ETHER_AUTO_100HALF == advCapability)
        rtl8309n_phyReg_set(phy, 4, 0, RTL8309N_CAPABLE_PAUSE | RTL8309N_CAPABLE_100BASE_TX_HD
                                  | RTL8309N_CAPABLE_10BASE_TX_FD | RTL8309N_CAPABLE_10BASE_TX_HD | 0x1);
    else if( RTL8309N_ETHER_AUTO_10FULL == advCapability)
        rtl8309n_phyReg_set(phy, 4, 0, RTL8309N_CAPABLE_PAUSE | RTL8309N_CAPABLE_10BASE_TX_FD 
                                  | RTL8309N_CAPABLE_10BASE_TX_HD | 0x1);
    else if(RTL8309N_ETHER_AUTO_10HALF == advCapability)
        rtl8309n_phyReg_set(phy, 4, 0, RTL8309N_CAPABLE_PAUSE | RTL8309N_CAPABLE_10BASE_TX_HD | 0x1);

    /*Each time the link ability of the RTL8309N is reconfigured, 
     *the auto-negotiation process should be executed to allow
     *the configuration to take effect. 
     */
    ctrlReg = 0;
    if(TRUE == autoNegotiation) 
        ctrlReg |= RTL8309N_ENABLE_AUTONEGO | RTL8309N_RESTART_AUTONEGO; 
    else
        ctrlReg |= 0;
    if(100 == speed) 
        ctrlReg |= RTL8309N_SPEED_SELECT_100M;
	else ctrlReg |= 0;
    if(TRUE == fullDuplex)
        ctrlReg |= RTL8309N_SELECT_FULL_DUPLEX;
	else ctrlReg |= 0;
    rtl8309n_phyReg_set(phy, 0, 0, ctrlReg);


    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_etherPhy_get
 * Description:
 *       Get PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 * Output:
 *      pAutoNegotiation    - Get whether auto-negotiation is enabled
 *      pAdvCapability       - When auto-negotiation is enabled, Get the advertised capability
 *      pSpeed                 - When auto-negotiation is disabled, Get the force mode speed
 *      pFullDuplex            - When auto-negotiatoin is disabled, Get the force mode duplex mode

 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode with the speed and duplex  
 *      provided.Port number should be smaller than RTL8309N_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8309N_ETHER_AUTO_100FULL and RTL8309N_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8309N_ETHER_SPEED_100 or RTL8309N_ETHER_SPEED_10.
 */
int32 rtl8309n_port_etherPhy_get(uint32 phy, uint32 *pAutoNegotiation, uint32 *pAdvCapability, uint32 *pSpeed, uint32 *pFullDuplex)
{
    uint32 regData;

    if((phy > RTL8309N_MAX_PHY_ID) || (NULL == pAutoNegotiation) || (NULL == pAdvCapability)
        || (NULL ==  pSpeed) || (NULL == pFullDuplex))
        return FAILED;

    rtl8309n_phyReg_get(phy, 0, 0, &regData);
    *pAutoNegotiation = ((regData >> 12) & 0x1) ? TRUE: FALSE;
    *pSpeed = ((regData >> 13) & 0x1) ? 100: 10;
    *pFullDuplex = ((regData >> 8) & 0x1) ? TRUE: FALSE;

    rtl8309n_phyReg_get(phy, 4, 0, &regData);
    if(regData & RTL8309N_CAPABLE_100BASE_TX_FD)
        *pAdvCapability = RTL8309N_ETHER_AUTO_100FULL;
    else if(regData & RTL8309N_CAPABLE_100BASE_TX_HD)
        *pAdvCapability = RTL8309N_ETHER_AUTO_100HALF;
    else if(regData & RTL8309N_CAPABLE_10BASE_TX_FD)
        *pAdvCapability = RTL8309N_ETHER_AUTO_10FULL;
    else if(regData & RTL8309N_CAPABLE_10BASE_TX_HD)
        *pAdvCapability = RTL8309N_ETHER_AUTO_10HALF;

	
    return SUCCESS;
}
#endif


/* Function Name:
 *      rtl8309n_port_phyLinkStatus_get
 * Description:
 *      Get PHY Link Status
 * Input:
*      phy        - Specify the phy 
 * Output:
*      plinkUp   - Describe whether link status is up or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       Read the link status of PHY register 1, phy could be 0~7
 */
int32 rtl8309n_port_phyLinkStatus_get(uint32 phy, uint32 *plinkUp) 
{
    uint32 bitValue;
	uint32 i;

	if(phy > RTL8309N_MAX_PHY_ID)
		return FAILED;
	
    if (NULL == plinkUp)
        return FAILED;
    
    rtl8309n_phyRegBit_get(phy, 1, 2, 0, &bitValue);
	i = 0;
	while(i++ < 100);
    rtl8309n_phyRegBit_get(phy, 1, 2, 0, &bitValue);    
    *plinkUp = (bitValue ? TRUE: FALSE);

    return SUCCESS;
}


/* Function Name:
 *		rtl8309n_port_macForceLinkExt0_set
 * Description:
 *		Set MAC 8 force mode ability
 * Input:
 *		mode	-	MAC interface mode(TMII/MII/RMII, MAC/PHY)
 *		pPortAbility	-	pointer point to the struct describing MAC ability
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_port_macForceLinkExt0_set(rtl8309n_mode_ext_t mode, rtl8309n_port_mac_ability_t *pPortAbility)
{
	uint32 regVal;
	
	if (mode >= RTL8309N_MODE_EXT_END)
		return FAILED;
	
	if (NULL == pPortAbility)
		return FAILED;
	if ((pPortAbility->nway >= RTL8309N_ENABLE_END) ||
		(pPortAbility->speed >= RTL8309N_PORT_SPEED_END) ||(pPortAbility->duplex >= RTL8309N_PORT_DUPLEX_END) ||
		(pPortAbility->rxpause >= RTL8309N_ENABLE_END) || (pPortAbility->txpause >= RTL8309N_ENABLE_END))
		return FAILED;

	/*set MAC mode*/
	switch(mode)
	{
		case RTL8309N_MODE_EXT_TMII_MAC:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x7;
			regVal |= (0 << 2) | 0x2;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;
			
		case RTL8309N_MODE_EXT_TMII_PHY:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x7;
			regVal |= (1 << 2) | 0x2;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;
			
		case RTL8309N_MODE_EXT_MII_MAC:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x7;
			regVal |= (0 << 2) | 0x1;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;

		case RTL8309N_MODE_EXT_MII_PHY:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x7;
			regVal |= (1 << 2) | 0x1;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;

		case RTL8309N_MODE_EXT_RMII:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x7;
			regVal |=  0x3;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;

		case RTL8309N_MODE_EXT_DISABLED:
			rtl8309n_reg_get(3, 16, 17, &regVal);
			regVal &= ~0x3;
			regVal |=  0x0;
			rtl8309n_reg_set(3, 16, 17, regVal);
			break;			

		default:
			break;
	}
	
	/*set mac ability*/
	rtl8309n_reg_get(8, 16, 0, &regVal);
	regVal &= ~0xFF;
	regVal |= (RTL8309N_ENABLED == pPortAbility->nway) ? 0 : 1;
	regVal |= (RTL8309N_PORT_LINK_UP == pPortAbility->link) ? (1 << 1) : (0 << 1);
	regVal |= (RTL8309N_ENABLED == pPortAbility->txpause) ? (1 << 5) : (0 << 5);
	regVal |= (RTL8309N_ENABLED == pPortAbility->rxpause) ? (1 << 6) : (0 << 6);
	/*speed*/
	switch(pPortAbility->speed)
	{
		case RTL8309N_PORT_SPEED_10M:
		case RTL8309N_PORT_SPEED_20M:
			regVal |= 0x0 << 3;
			break;
			
		case RTL8309N_PORT_SPEED_100M:
		case RTL8309N_PORT_SPEED_200M:
			regVal |= 0x1 << 3;
			break;
		default:
			break;
	}
	/*duplex*/
	switch(pPortAbility->duplex)
	{
		case RTL8309N_PORT_HALF_DUPLEX:
			regVal |= 0x0 << 2;
			break;
			
		case RTL8309N_PORT_FULL_DUPLEX:
			regVal |= 0x1 << 2;
			break;
		default:
			break;
	}
	rtl8309n_reg_set(8, 16, 0, regVal);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_port_macForceLinkExt0_get
 * Description:
 *		Get MAC 8 force mode ability configuration
 * Input:
 *		none
 * Output:
 *		pMode	-	MAC interface mode(TMII/MII/RMII, MAC/PHY)
 *		pPortAbility	-	pointer point to the struct describing MAC ability
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_port_macForceLinkExt0_get(rtl8309n_mode_ext_t *pMode, rtl8309n_port_mac_ability_t *pPortAbility)
{
	uint32 regVal;
	uint32 tmp, hitTmii;

	if (NULL == pMode)
		return FAILED;
	if (NULL == pPortAbility)
		return FAILED;

	/*get MAC mode*/
	hitTmii = FALSE;
	rtl8309n_reg_get(3, 16, 17, &regVal);
	switch(regVal & 0x7)
	{
		case (0x0 << 2) | 0x0:
		case (0x1 << 2) | 0x0:
			*pMode = RTL8309N_MODE_EXT_DISABLED;
			break;
			
		case (0x0 << 2) | 0x1:
			*pMode = RTL8309N_MODE_EXT_MII_MAC;
			break;

		case (0x1 << 2) | 0x1:
			*pMode = RTL8309N_MODE_EXT_MII_PHY;
			break;
			
		case (0x0 << 2) | 0x2:
			*pMode = RTL8309N_MODE_EXT_TMII_MAC;
			hitTmii = TRUE;
			break;
			
		case (0x1 << 2) | 0x2:
			*pMode = RTL8309N_MODE_EXT_TMII_PHY;
			hitTmii = TRUE;
			break;

		case (0x0 << 2) | 0x3:
        case (0x1 << 2) | 0x3:
			*pMode = RTL8309N_MODE_EXT_RMII;
			break;

		default:
			break;
	}
	
	/*get MAC ability*/
	rtl8309n_reg_get(8, 16, 0, &regVal);
	pPortAbility->nway = (regVal & 0x1) ? RTL8309N_DISABLED : RTL8309N_ENABLED;
	tmp = (regVal >> 1) & 0x1;
	pPortAbility->link = tmp ? RTL8309N_PORT_LINK_UP : RTL8309N_PORT_LINK_DOWN;
	tmp = (regVal >> 5) & 0x1;
	pPortAbility->txpause = tmp ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	tmp = (regVal >> 6) & 0x1;	
	pPortAbility->rxpause = tmp ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	tmp = (regVal >> 3) & 0x1;
	switch(tmp)
	{
		case 0:
			if(TRUE == hitTmii)
				pPortAbility->speed = RTL8309N_PORT_SPEED_20M;
			else
				pPortAbility->speed = RTL8309N_PORT_SPEED_10M;				
			break;
			
		case 1:
			if(TRUE == hitTmii)
				pPortAbility->speed = RTL8309N_PORT_SPEED_200M;
			else
				pPortAbility->speed = RTL8309N_PORT_SPEED_100M;				
			break;
			
		default:
			break;
	}
	tmp = (regVal >> 2) & 0x1;
	if (tmp)
		pPortAbility->duplex = RTL8309N_PORT_FULL_DUPLEX;
	else
		pPortAbility->duplex = RTL8309N_PORT_HALF_DUPLEX;
	
	return SUCCESS;
}

int32 rtl8309n_port_macAbilityExt0_get(rtl8309n_mode_ext_t *pMode, rtl8309n_port_mac_ability_t *pPortAbility)
{
	uint32 regVal;
	uint32 hitTmii;

	if(NULL == pMode || NULL == pPortAbility)
		return FAILED;

	/*get MAC mode*/
	hitTmii = FALSE;
	rtl8309n_reg_get(3, 16, 17, &regVal);
	switch(regVal & 0x7)
	{
		case (0x0 << 2) | 0x0:
		case (0x1 << 2) | 0x0:
			*pMode = RTL8309N_MODE_EXT_DISABLED;
			break;
			
		case (0x0 << 2) | 0x1:
			*pMode = RTL8309N_MODE_EXT_MII_MAC;
			break;

		case (0x1 << 2) | 0x1:
			*pMode = RTL8309N_MODE_EXT_MII_PHY;
			break;
			
		case (0x0 << 2) | 0x2:
			*pMode = RTL8309N_MODE_EXT_TMII_MAC;
			hitTmii = TRUE;
			break;
			
		case (0x1 << 2) | 0x2:
			*pMode = RTL8309N_MODE_EXT_TMII_PHY;
			hitTmii = TRUE;
			break;

		case (0x0 << 2) | 0x3:
        case (0x1 << 2) | 0x3:
			*pMode = RTL8309N_MODE_EXT_RMII;
			break;

		default:
			break;
	}	

	/*get mac 8 ability*/
	rtl8309n_reg_get(2, 18, 14, &regVal);
	pPortAbility->link = (regVal >> 8) ? RTL8309N_PORT_LINK_UP : RTL8309N_PORT_LINK_DOWN;
	rtl8309n_reg_get(2, 19, 14, &regVal);
	pPortAbility->speed = (regVal >> 8) ? RTL8309N_PORT_SPEED_100M : RTL8309N_PORT_SPEED_10M;
	if(hitTmii)
		pPortAbility->speed = (regVal >> 8) ? RTL8309N_PORT_SPEED_200M : RTL8309N_PORT_SPEED_20M;
	rtl8309n_reg_get(2, 20, 14, &regVal);
	pPortAbility->duplex = (regVal >> 8) ? RTL8309N_PORT_FULL_DUPLEX : RTL8309N_PORT_HALF_DUPLEX;
	rtl8309n_reg_get(2, 21, 14, &regVal);
	pPortAbility->txpause = (regVal >> 8) ? TRUE : FALSE;
	rtl8309n_reg_get(2, 22, 14, &regVal);
	pPortAbility->rxpause = (regVal >> 8) ? TRUE : FALSE;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_phyAutoNegoDone_get
 * Description:
 *      Get PHY auto-negotiation result status
 * Input:
 *      phy      -	Specify the phy to get status
 * Output:
*       pDone	 -  Describe whether auto-negotiation is done or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Read the auto-negotiation complete of PHY register 1.
 */
int32 rtl8309n_port_phyAutoNegoDone_get(uint32 phy, uint32 *pDone) 
{
    uint32 bitValue, regVal;

    if (NULL == pDone)
        return FAILED;

	rtl8309n_phyReg_get(phy, 1, 0, &regVal);
	bitValue = (regVal >> 5) & 0x1;
    *pDone = (bitValue ? TRUE: FALSE);
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_phyLoopback_set
 * Description:
 *       Set PHY loopback
 * Input:
 *      phy         - 	Specify the phy to configure
 *      enabled   	- 	Enable phy loopback
 * Output:
 *      none      
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only phy 0~7 could be configured the phy loopback
 */
int32 rtl8309n_port_phyLoopback_set(uint32 phy, uint32 enabled) 
{
	uint32 regVal;
	
    if(phy > RTL8309N_MAX_PHY_ID)
        return FAILED;

	rtl8309n_phyReg_get(phy, 0, 0, &regVal);
	regVal &= ~(1 << 14);
	regVal |= (RTL8309N_ENABLED == enabled) ? (1 << 14) : (0 << 14);
	rtl8309n_phyReg_set(phy, 0, 0, regVal);
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_phyLoopback_get
 * Description:
 *      Get PHY loopback setting
 * Input:
 *      phy         - Specify the phy to get status
 * Output:
 *      pEnabled  -  phy loopback setting
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8309n_port_phyLoopback_get(uint32 phy, uint32 *pEnabled) 
{
    uint32 regVal, bitVal;;

    if(phy > RTL8309N_MAX_PHY_ID)
        return FAILED;

    if (NULL == pEnabled)
        return FAILED;

	rtl8309n_phyReg_get(phy, 0, 0, &regVal);
	bitVal = (regVal >> 14) & 0x1;
    *pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_MacLearnEnable_set
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        - Specify port number (0 ~ 8)
 * Output:
 *      enabled -  enable or disable
 * Return:
 *      SUCCESS
 * Note:
 *
 */
int32 rtl8309n_port_MacLearnEnable_set(uint32 port, uint32 enabled)
{
    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;

    rtl8309n_regbit_set(port, 16, 0, 5, (RTL8309N_ENABLED == enabled) ? 1 : 0);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_port_MacLearnEnable_get
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        -	Specify port number (0 ~ 8)
 * Output:
 *      pEnabled 	-	pointer point to the enabled status of port mac learning ability 
 * Return:
 *      SUCCESS
 * Note:
 *
 */
int32 rtl8309n_port_MacLearnEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;
	
    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
	if (NULL == pEnabled)
		return FAILED;
	
    rtl8309n_regbit_get(port, 16, 0, 5, &bitVal);
	*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);

    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *		port	-	port number(0 - 8)
 *      isomsk    - port isolation port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      In RTL8309N, there are 9 ports. Every port has its port isolation mask which has 9 bits.
 *		When the port isolation mask bit n of port m is 1, it means port m can
 *		forword packets to port n. When the bit n is 0 of port m, portm can not forward 
 *		packets to port n.
 */
int32 rtl8309n_port_isolation_set(uint32 port, uint32 isomsk)
{
	int32 retVal;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (isomsk > RTL8309N_MAX_PORTMASK)
		return FAILED;
    if((retVal = rtl8309n_reg_set(port, 16, 4, isomsk)) != SUCCESS)
		return FAILED;
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_port_isolation_get
 * Description:
 *      get the status of port isolation 
 * Input:
 *      port	-	port number
 * Output:
 *      pIsomsk    -  the pointer of port isolation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * Note:
 *      In RTL8309N, there are 9 ports. Every port has its port isolation mask which has 9 bits.
 *		When the port isolation mask bit n of port m is 1, it means port m can
 *		forword packets to port n. Otherwise when the bit n is 0 of port m, portm can not forward 
 *		packets to port n.
 */
int32 rtl8309n_port_isolation_get(uint32 port, uint32 *pIsomsk)
{
    uint32 regVal;

    if (port > RTL8309N_MAX_PORT_ID || NULL == pIsomsk)
        return FAILED;
	
     rtl8309n_reg_get(port, 16, 4, &regVal);
     *pIsomsk = regVal & 0x1FF;

    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_init
 * Description:
 *		Init vlan
 * Input:
 *		none
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set 
 *      all ports PVID to the default VLAN.
 */
int32 rtl8309n_vlan_init(void)
{
    uint32 i;
    uint32 port;

    /*clear vlan table*/
    for(i = 0; i < 16; i++)
        rtl8309n_vlan_entry_set(i, 0, 0, 0, 0);

    /*set switch default configuration:
     *enable tag aware, disable ingress filter,
     *admit all packet
     */
    rtl8309n_vlan_igrFilterEnable_set(RTL8309N_DISABLED);  
    for (port = 0; port < RTL8309N_PORT_NUMBER; port++)
    {
        rtl8309n_vlan_portAcceptFrameType_set(port, RTL8309N_ACCEPT_ALL);
		rtl8309n_vlan_tagAwareEnable_set(port, RTL8309N_ENABLED);  
    }

    /*add a default vlan 1 containing all ports, vid=0,memmsk=0x1ff,untagmsk=0,fid=0 */
    rtl8309n_vlan_entry_set(0, 1, 0x1FF, 0, 0);
    
    /*set all ports' vid to vlan 1*/
    for(port = 0; port < RTL8309N_PORT_NUMBER; port++)
        rtl8309n_vlan_portPvidIndex_set(port, 0);

    /*set vlan enabled*/
	rtl8309n_vlan_enable_set(RTL8309N_ENABLED);
#ifdef RTL8309N_TBLBAK
    rtl8309_TblBak.vlanConfig.enVlan = TRUE; 
#endif
    /*disable trunk*/
   // rtl8309n_regbit_set(0, 19, 11, 0, 1);
    
    return SUCCESS;  	
}

/* Function Name:
 *		rtl8309n_vlan_enable_set
 * Description:
 *		Enable vlan
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		
 */
int32 rtl8309n_vlan_enable_set(uint32 enabled)
{
	rtl8309n_regbit_set(0, 16, 0, 16, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_enable_get
 * Description:
 *		Enable vlan
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *		
 */
int32 rtl8309n_vlan_enable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(0, 16, 0, 16, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_discardNonPvidPktEnable_set
 * Description:
 *		Enable discarding tagged frame whose VID does not match PVID 
 * Input:
 *		port	-	port number(0 - 8)
 *		discard_enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		When tagged frame's VID doesn't equal to PVID, this function can be called to discard
 *		or forward this frame.
 */
int32 rtl8309n_vlan_discardNonPvidPktEnable_set(uint32 port, uint32 discard_enabled)
{
	uint32 regNum;
	
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;

	regNum = 17 + port;
	rtl8309n_regbit_set(0, regNum, 3, 16, (RTL8309N_ENABLED == discard_enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_discardNonPvidPktEnable_get
 * Description:
 *		Get discarding status of tagged frame whose VID does not match PVID 
 * Input:
 *		port	-	port number(0 - 8)
 * Output:
 *		pDiscard_enabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		When tagged frame's VID doesn't equal to PVID, this function can be called to discard
 *		or normal forward the frame.
 */
int32 rtl8309n_vlan_discardNonPvidPktEnable_get(uint32 port, uint32 *pDiscard_enabled)
{
	uint32 bitVal;
	uint32 regNum;

	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if (NULL == pDiscard_enabled)
        return FAILED;
    
	regNum= 17 + port;
	rtl8309n_regbit_get(0, regNum, 3, 16, &bitVal);
	*pDiscard_enabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_tagAwareEnable_set
 * Description:
 *      Enable ASIC tag aware
 * Input:
 *		port     - port number(0 - 8)
 *      enabled  - Configure RTL8309N VLAN tag awared
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Enable ASIC to parse the tag when it receives a frame.
 */
int32 rtl8309n_vlan_tagAwareEnable_set(uint32 port, uint32 enabled)
{
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
    rtl8309n_regbit_set(0, 17 + port, 2, 16, (RTL8309N_ENABLED == enabled)? 1 : 0);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_tagAwareEnable_get
 * Description:
 *      Get tag aware enabled status
 * Input:
 *      port      - port number
 * Output:
 *      pEnabled  - the pointer of RTL8309N VLAN tag awared status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Enable ASIC to parse the tag when it receives a frame.
 */
int32 rtl8309n_vlan_tagAwareEnable_get(uint32 port, uint32 * pEnabled) 
{
    uint32 bitValue;

    if ((port > RTL8309N_MAX_PORT_ID) || (NULL == pEnabled))
        return FAILED;	
    
    rtl8309n_regbit_get(0, 17 + port, 2, 16, &bitValue);
    *pEnabled = (bitValue ? RTL8309N_ENABLED : RTL8309N_DISABLED);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_igrFilterEnable_set
 * Description:
 *      Enable VLAN ingress filter
 * Input:
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8309n_vlan_igrFilterEnable_set(uint32 enabled) 
{
    rtl8309n_regbit_set(0, 16, 1, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);    

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_igrFilterEnable_get
 * Description:
 *      Get VLAN ingress filter enabled status
 * Input:
 *      none
 * Output:
 *      pEnabled  - enable or disable
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8309n_vlan_igrFilterEnable_get(uint32 *pEnabled) 
{
    uint32 bitValue;

    if (NULL == pEnabled)
        return FAILED;
    
    rtl8309n_regbit_get(0, 16, 1, 16, &bitValue);
    *pEnabled = (bitValue ? RTL8309N_DISABLED : RTL8309N_ENABLED);
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                          - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    	RTL8309N_ACCEPT_ALL             -   accept vlan tagged, prority tagged(vid=0) and untagged packets
 *    	RTL8309N_ACCEPT_TAG_ONLY        -   accpet vlan tagged packets
 *    	RTL8309N_ACCEPT_UNTAG_ONLY      -   accept prority tagged and untagged packets
 */
int32 rtl8309n_vlan_portAcceptFrameType_set(uint32 port, rtl8309n_acceptFrameType_t accept_frame_type)
{
    uint32 regNum, regval;
    
    if(port > RTL8309N_MAX_PORT_ID)
        return FAILED;
	if(accept_frame_type >= RTL8309N_ACCEPT_TYPE_END)
		return FAILED;
	
	regNum = 17 + port;
    rtl8309n_reg_get(0, regNum, 16, &regval);
    regval &= ~0x3;
	switch(accept_frame_type)
	{
		case RTL8309N_ACCEPT_UNTAG_ONLY:
			regval |= 1;
			break;

		case RTL8309N_ACCEPT_TAG_ONLY:
			regval |= 2;
			break;

		case RTL8309N_ACCEPT_ALL:
			regval |= 3;
			break;

		default:
            regval |= 0;
			break;
	}
    rtl8309n_reg_set(0, regNum, 16, regval);        

    return SUCCESS;
}



/* Function Name:
 *      rtl8309n_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 * Output:
 *      pAccept_frame_type             - accept frame type pointer
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    	RTL8309N_ACCEPT_ALL             -   accept vlan tagged, prority tagged(vid=0) and untagged packets
 *    	RTL8309N_ACCEPT_TAG_ONLY        -   accpet vlan tagged packets
 *    	RTL8309N_ACCEPT_UNTAG_ONLY      -   accept prority tagged and untagged packets
 */
int32 rtl8309n_vlan_portAcceptFrameType_get(uint32 port, rtl8309n_acceptFrameType_t *pAccept_frame_type)
{
    uint32 regval, regNum;
    
    if((port > RTL8309N_MAX_PORT_ID) || (pAccept_frame_type == NULL))
        return FAILED;

	regNum = 17 + port;
    rtl8309n_reg_get(0, regNum, 16, &regval);
    switch(regval & 0x3)
    {
    	case 1:
			*pAccept_frame_type = RTL8309N_ACCEPT_UNTAG_ONLY;
			break;

		case 2:
			*pAccept_frame_type = RTL8309N_ACCEPT_TAG_ONLY;
			break;

		case 3:
			*pAccept_frame_type = RTL8309N_ACCEPT_ALL;
			break;

		default:
			break;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_portPvidIndex_set
 * Description:
 *      Configure port PVID index 
 * Input:
 *      port           -   Specify the port(port 0 ~ port 8) to configure VLAN index
 *      vlanIndex      -   Specify the VLAN index
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by Port index is port's PVID 
 */
int32 rtl8309n_vlan_portPvidIndex_set(uint32 port, uint32 vlanIndex)
{
    uint32 regValue;
	uint32 regNum;

    if((port > RTL8309N_MAX_PORT_ID) || vlanIndex > RTL8309N_MAX_VLANINDEX)
        return FAILED;

  	regNum = 17 + port;
    rtl8309n_reg_get(0, regNum, 16, &regValue);
    regValue &= ~(0xf << 4);
    regValue |= (vlanIndex << 4);
    rtl8309n_reg_set(0, regNum, 16, regValue);
       
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_vlan_portPvidIndex_get
 * Description:
 *      Get port PVID index 
 * Input:
 *      port            -   Specify the port(port 0 ~ port 8) to configure VLAN index
 * Output:
 *      pVlanIndex   -   pointer of VLAN index number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */
int32 rtl8309n_vlan_portPvidIndex_get(uint32 port, uint32 *pVlanIndex) 
{
	uint32 regNum, regVal;
	
    if((port > RTL8309N_MAX_PORT_ID) || pVlanIndex == NULL)
        return FAILED;

	regNum = 17 + port;
    rtl8309n_reg_get(0, regNum, 16, &regVal);
    *pVlanIndex = (regVal >> 4) & 0xF;
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_vlan_leakyEnable_set
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      type   -  vlan leaky type
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    (1)type coulde be:
 *          RTL8309N_VLAN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8309N_VLAN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8309N_VLAN_LEAKY_BRDCAST   - VLan leaky for broadcast pkt
 *          RTL8309N_VLAN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8309N_VLAN_LEAKY_MIRROR - Vlan leaky for mirror function
 *    (2)When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *       That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *       to the egress port ignoring the egress rule.
 *    (3)When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *       group member set, ignoring the VLAN member set domain limitation.
 *    (4)when Vlan leaky for broadcast pkt is enabled, broadcast packet may be flood to all ports, ignoring the 
 *       VLAN memeber set domain limitation.
 *    (5)When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *       ports ignoring the egress rule.
 *    (6)When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *       ignoring the VLAN member set domain limitation.
 */
int32 rtl8309n_vlan_leakyEnable_set(uint32 type, uint32 enabled)
{
    if (type > RTL8309N_VLAN_LEAKY_END)
        return FAILED;
    
    switch(type) 
    {
        case RTL8309N_VLAN_LEAKY_UNICAST:
            rtl8309n_regbit_set(0, 16, 3, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);
            break;
        case RTL8309N_VLAN_LEAKY_MULTICAST:
            rtl8309n_regbit_set(0, 16, 4, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);
            break;
		case RTL8309N_VLAN_LEAKY_BRDCAST:
			rtl8309n_regbit_set(0, 16, 5, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);
			break;
        case RTL8309N_VLAN_LEAKY_ARP:
            rtl8309n_regbit_set(0, 16, 6, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);
            break;
        case RTL8309N_VLAN_LEAKY_MIRROR:
            rtl8309n_regbit_set(0, 16, 7, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);
            break;
        default:
            break;
    } 
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_leakyEnable_get
 * Description:
 *      Get switch whether forwards unicast frames to other VLANs
 * Input:
 *      type   -  vlan leaky type
 * Output:
 *      pEnabled  - the pointer of Vlan Leaky status(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    (1)type coulde be:
 *          RTL8309N_VLAN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8309N_VLAN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8309N_VLAN_LEAKY_BRDCAST   - VLan leaky for broadcast pkt
 *          RTL8309N_VLAN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8309N_VLAN_LEAKY_MIRROR - Vlan leaky for mirror function
 *    (2)When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *       That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *       to the egress port ignoring the egress rule.
 *    (3)When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *       group member set, ignoring the VLAN member set domain limitation.
 *    (4)when Vlan leaky for broadcast pkt is enabled, broadcast packet may be flood to all ports, ignoring the 
 *       VLAN memeber set domain limitation.
 *    (5)When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *       ports ignoring the egress rule.
 *    (6)When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *       ignoring the VLAN member set domain limitation.
 */
int32 rtl8309n_vlan_leakyEnable_get(uint32 type, uint32 *pEnabled)
{
    uint32 bitValue;
    
    if (type > RTL8309N_VLAN_LEAKY_END)
        return FAILED;
    if(NULL == pEnabled)
        return FAILED;

    switch(type) 
    {
        case RTL8309N_VLAN_LEAKY_UNICAST:
            rtl8309n_regbit_get(0, 16, 3, 16, &bitValue);
            break;
        case RTL8309N_VLAN_LEAKY_MULTICAST:
            rtl8309n_regbit_get(0, 16, 4, 16, &bitValue);
            break;
		case RTL8309N_VLAN_LEAKY_BRDCAST:
			rtl8309n_regbit_get(0, 16, 5, 16, &bitValue);
			break;
        case RTL8309N_VLAN_LEAKY_ARP:
            rtl8309n_regbit_get(0, 16, 6, 16, &bitValue);
            break;
        case RTL8309N_VLAN_LEAKY_MIRROR:
            rtl8309n_regbit_get(0, 16, 7, 16, &bitValue);
            break;
        default:
            break;
    }
    *pEnabled = (bitValue ?  RTL8309N_DISABLED : RTL8309N_ENABLED);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_nullVidReplaceEnable_set
 * Description:
 *      Configure switch to replace Null VID tagged frame by PVID if it is tag aware
 * Input:
 *      port   -  port number
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8309n only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
int32 rtl8309n_vlan_nullVidReplaceEnable_set(uint32 port, uint32 enabled)
{
    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;

    rtl8309n_regbit_set(port, 16, 0, 6, (RTL8309N_ENABLED == enabled) ? 1 : 0);  
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_nullVidReplaceEnable_get
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      port   -  port number
 * Output:
 *      pEnabled  - the pointer of Null VID replacement ability(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
int32 rtl8309n_vlan_nullVidReplaceEnable_get(uint32 port, uint32 *pEnabled)
{
    uint32 bitValue;
    
    if ((port > RTL8309N_MAX_PORT_ID) || (NULL == pEnabled ))
        return FAILED;
    
    rtl8309n_regbit_get(port, 16, 0, 6, &bitValue);
    *pEnabled = (bitValue ? RTL8309N_ENABLED : RTL8309N_DISABLED);

    return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_vlan_tagInsertRemove_set
 * Description:
 *		configure switch to insert or remove tag when TX frames out from a port
 * Input:
 *		port	-	port number
 *		mode	-	replace, insert, remove, or don't touch
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      This API could be called to set per egress port action for vlan tag.
 *      The action could be:
 *          RTL8309N_VLAN_TAG_REPALCE   -   replace, which means that remove tag from tagged packets
 *                  add new tag(vid=pvid) to it, and just add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAG_REMOVE    -   remove, which means that remove tag from tagged pacets
 *                  and don't touch untagged packets
 *          RTL8309N_VLAN_TAG_INSERT    -   insert, which means that add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAG_DONTTOUCH     -   do nothing for frames' tag
 */
int32 rtl8309n_vlan_tagInsertRemove_set(uint32 port, uint32 mode)
{
	uint32 regVal, mode_val;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (mode >= RTL8309N_VLAN_TAG_MODE_END)
		return FAILED;

	switch(mode)
	{
		case RTL8309N_VLAN_TAG_REPALCE:
			mode_val = 0;	
			break;

		case RTL8309N_VLAN_TAG_REMOVE:
			mode_val = 1;			
			break;	

		case RTL8309N_VLAN_TAG_INSERT:
			mode_val = 2;		
			break;

		case RTL8309N_VLAN_TAG_DONTTOUCH:
			mode_val = 3;
			break;	

		default:
			break;
	}
	rtl8309n_reg_get(port, 16, 6, &regVal);
	regVal &= ~(0x3 << 1);
	regVal |= mode_val << 1;
	rtl8309n_reg_set(port, 16, 6, regVal);

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_vlan_tagInsertRemove_get
 * Description:
 *		Get switch status whether to insert or remove tag when TX frames out from a port
 * Input:
 *		port	-	port number
 * Output:
 *		pMode	-	replace, insert, remove, or don't touch
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      This API could be called to set per egress port action for vlan tag.
 *      The action could be:
 *          RTL8309N_VLAN_TAG_REPALCE   -   replace, which means that remove tag from tagged packets
 *                  add new tag(vid=pvid) to it, and just add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAG_REMOVE    -   remove, which means that remove tag from tagged pacets
 *                  and don't touch untagged packets
 *          RTL8309N_VLAN_TAG_INSERT    -   insert, which means that add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAG_DONTTOUCH     -   do nothing for frames' tag
 */
int32 rtl8309n_vlan_tagInsertRemove_get(uint32 port, uint32 *pMode)
{
	uint32 regVal;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (NULL == pMode)
		return FAILED;

	rtl8309n_reg_get(port, 16, 6, &regVal);
	switch((regVal >> 1) & 0x3)
	{
		case 0:
			*pMode = RTL8309N_VLAN_TAG_REPALCE;
			break;

		case 1:
			*pMode = RTL8309N_VLAN_TAG_REMOVE;
			break;
			
		case 2:
			*pMode = RTL8309N_VLAN_TAG_INSERT;
			break;

		case 3:
			*pMode = RTL8309N_VLAN_TAG_DONTTOUCH;
			break;			

		default:
			break;
	}

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_igrPortTagInsertEnable_set
 * Description:
 *		Enable switch to insert vlan tag based on ingress port
 * Input:
 *		port	-	port number	(0 - 8)
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_vlan_igrPortTagInsertEnable_set(uint32 port, uint32 enabled)
{
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;

	/*enable insert vlan tag based on ingress port*/
	rtl8309n_regbit_set(port, 16, 3, 6, (RTL8309N_ENABLED == enabled) ? 1 : 0);

    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_igrPortTagInsertEnable_get
 * Description:
 *		Enable switch to insert vlan tag based on ingress port
 * Input:
 *		port	-	port number	(0 - 8)
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_vlan_igrPortTagInsertEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;

	rtl8309n_regbit_get(port, 16, 3, 6, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_tagInsert_set
 * Description:
 *      Insert VLAN tag to untag packet by ingress port
 * Input:
 *      egPort              - egress port number 0~8
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the rxPortMsk, it will be inserted with an VLAN tag.
 */
int32 rtl8309n_vlan_igrPortTagInsert_set(uint32 egPort, uint32 rxPortMsk)
{
    uint32 regval;
    
    if((egPort > RTL8309N_MAX_PORT_ID) || (rxPortMsk > RTL8309N_MAX_PORTMASK))
        return FAILED;
    
    rtl8309n_reg_get(egPort, 16, 6, &regval);
    regval &= ~(RTL8309N_MAX_PORTMASK << 7);
    regval |= (rxPortMsk << 7);
    rtl8309n_reg_set(egPort, 16, 6, regval);
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_vlan_tagInsert_get
 * Description:
 *      get  ingress port mask of VLAN tag insertion for untagged packet
 * Input:
 *      egPort               - egress port number 0~5
 * Output:
 *      pRxPortMsk           - ingress port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
int32 rtl8309n_vlan_igrPortTagInsert_get(uint32 egPort, uint32 * pRxPortMsk)
{
    uint32 regval ;
    
    if ((egPort > RTL8309N_MAX_PORT_ID) || (NULL == pRxPortMsk))
        return FAILED;

    rtl8309n_reg_get(egPort, 16, 6, &regval);
    *pRxPortMsk = (regval >> 7) & RTL8309N_MAX_PORTMASK;
    return SUCCESS;
}

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vlanIndex     - VLAN entry index
 *      vid           - VLAN ID to configure
 *      mbrmsk        - VLAN member set portmask
 *      untagmsk      - VLAN untag set portmask
 *      fid           - VLAN IVL ID
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     fid can be any value from 0 - 3.It means that RTL8309N maxismly supports 4 IVLs.
 */
int32 rtl8309n_vlan_entry_set(uint32 vlanIndex, uint32 vid, uint32 mbrmsk, uint32 untagmsk, uint32 fid)
{
    uint32 regValue;
	uint32 regNum;

    if ((vlanIndex > RTL8309N_MAX_VLANINDEX) || (vid > RTL8309N_VIDMAX) || (mbrmsk > RTL8309N_MAX_PORTMASK) || (untagmsk > RTL8309N_MAX_PORTMASK) || (fid > RTL8309N_FID3))
        return FAILED;
	
    /*set vlan entry*/
	regNum = 16 + vlanIndex;
	regValue = (vid & RTL8309N_VIDMAX) | ((fid & 0x3) << 12);
	rtl8309n_reg_set(1, regNum, 16, regValue);
	//set vlan membership
	regValue = mbrmsk & RTL8309N_MAX_PORTMASK;
	rtl8309n_reg_set(2, regNum, 16, regValue);
	//set vlan untag mask
	regValue = untagmsk & RTL8309N_MAX_PORTMASK;
	rtl8309n_reg_set(3, regNum, 16, regValue);	

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_entry_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 * Output:
 *      pVid           -  the pointer of VLAN ID 
 *      pMbrmsk     -  the pointer of VLAN member set portmask
 *      pUntagmsk  -  the pointer of VLAN untag set portmask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
int32 rtl8309n_vlan_entry_get(uint32 vlanIndex, uint32 *pVid, uint32 *pMbrmsk, uint32 *pUntagmsk, uint32 *pFid)
{
	uint32 regVal;
	uint32 regNum;
	
    if((vlanIndex > RTL8309N_MAX_VLANINDEX) || (NULL == pVid) ||  (NULL == pMbrmsk) || (NULL == pUntagmsk) || (NULL == pFid))
        return FAILED;

	regNum = 16 + vlanIndex;
    rtl8309n_reg_get(1, regNum, 16, &regVal);
	*pVid = regVal & RTL8309N_VIDMAX;
	*pFid = (regVal >> 12) & 0x3;
    rtl8309n_reg_get(2, regNum, 16, &regVal);
	*pMbrmsk  = regVal & RTL8309N_MAX_PORTMASK;
    rtl8309n_reg_get(3, regNum, 16, &regVal); 
    *pUntagmsk = regVal & RTL8309N_MAX_PORTMASK;
	
    return  SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_fixMcastFidEnable_set
 * Description:
 *		Enable fixing fid for multicast packets
 * Input:
 *		enabled		-	Enable or Disable
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	success
 * Note:
 *		
 */
int32 rtl8309n_vlan_fixMcastFidEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(0, 16, 8, 16, (RTL8309N_ENABLED == enabled) ? 1 : 0);
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_fixMcastFidEnable_get
 * Description:
 *		Get Enabled status of fixing fid for multicast packets
 * Input:
 *		none
 * Output:
 *		pEnabled		-	Enable or Disable
 * Return:
 *		SUCCESS		-	Success
 *		FAILED		-	Failure
 * Note:
 *		
 */
int32 rtl8309n_vlan_fixMcastFidEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;

	rtl8309n_regbit_get(0, 16, 8, 16, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_fixMcastFid_set
 * Description:
 *		Set fixed fid for multicast packets
 * Input:
 *		fid
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	Success
 *		FAILED		-	Failure
 * Note:
 *		
 */
int32 rtl8309n_vlan_fixMcastFid_set(uint32 fid)
{
	uint32 regVal;
	
	if (fid > RTL8309N_FID3)
		return FAILED;

	rtl8309n_reg_get(0, 16, 16, &regVal);
	regVal &= ~(0x3 << 9);
	regVal |= (fid << 9);
	rtl8309n_reg_set(0, 16, 16, regVal);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_vlan_fixMcastFid_get
 * Description:
 *		Get fixed fid for multicast packets
 * Input:
 *		none
 * Output:
 *		pFid	-	pointer point to fid
 * Return:
 *		SUCCESS		-	success
 *		FAILED		-	failure
 */
int32 rtl8309n_vlan_fixMcastFid_get(uint32 *pFid)
{
	uint32 regVal;

	if (NULL == pFid)
		return FAILED;

	rtl8309n_reg_get(0, 16, 16, &regVal);
	*pFid = (regVal >> 9) & 0x3;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_vlan_unmatchVidAction_set
 * Description:
 *      set action for unmatch vid packets
 * Input:
 *      action
 * Output:
 *      none
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 *      This API can be called to set action for vlan tagged packets whose
 *      tag vid can't be found in vlan table.
*/
int32 rtl8309n_vlan_unmatchVidAction_set(uint32 action)
{
    uint32 bitVal;

    if (RTL8309N_ACT_DROP != action && RTL8309N_ACT_TRAP2CPU != action)
        return FAILED;

    bitVal = (RTL8309N_ACT_TRAP2CPU == action)? 1:0;
    rtl8309n_regbit_set(0, 16, 2, 16, bitVal);

    return SUCCESS; 
}

/* Function Name:
 *      rtl8309n_vlan_unmatchVidAction_get
 * Description:
 *      get action for unmatch vid packets
 * Input:
 *      none
 * Output:
 *      pAction     -   pointer pointed to action
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 *      This API can be called to get action for vlan tagged packets whose
 *      tag vid can't be found in vlan table.
*/
int32 rtl8309n_vlan_unmatchVidAction_get(uint32 *pAction)
{
    uint32 bitVal;

    if (NULL == pAction)
        return FAILED;

    rtl8309n_regbit_get(0, 16, 2, 16, &bitVal); 
    *pAction = bitVal? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;

    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_enable_set
 * Description:
 *		Enable CPU port ability
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	success
 * Note:
 */
int32 rtl8309n_cpu_enable_set(uint32 enabled)
{
    /*enable or disable CPU port Function */
  	rtl8309n_regbit_set(2, 16, 0, 17, (RTL8309N_ENABLED == enabled) ? 0 : 1); 	

	return SUCCESS;
	
}

/* Function Name:
 *		rtl8309n_cpu_enable_set
 * Description:
 *		Get status of CPU port ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS		-	success
 * Note:
 */
int32 rtl8309n_cpu_enable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;

  	rtl8309n_regbit_get(2, 16, 0, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED; 

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_cpu_portNumber_set
 * Description:
 *		Set cpu port number
 * Input:
 *		port	-	port id(0 - 8), 9 means no port is set as cpu port
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 */
int32 rtl8309n_cpu_portNumber_set(uint32 port)
{
	uint32 regValue;
	
    if (port > RTL8309N_NOCPUPORT)
        return FAILED;
	
	/*set cpu port number*/
    rtl8309n_reg_get(2, 16, 17, &regValue);
	regValue &= ~(0xF << 1);
    regValue |= (port << 1);
    rtl8309n_reg_set(2, 16, 17, regValue);	

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_cpu_portNumber_set
 * Description:
 *		Get cpu port number
 * Input:
 *		none
 * Output:
 *		pPort	-	port id(0 - 8)
 * Return:
 *		FAILED
 *		SUCCESS
 */
int32 rtl8309n_cpu_portNumber_get(uint32 *pPort)
{
	uint32 regValue;
	
    if (NULL == pPort)
        return FAILED;
	
	/*set cpu port number*/
    rtl8309n_reg_get(2, 16, 17, &regValue);
	*pPort = (regValue >> 1) & 0xF;

	return SUCCESS;	
}

/* Function Name:
 *		rtl8309n_cpu_tagInsertEnable_set
 * Description:
 *		Enable insert cpu tag to packets trapped to CPU
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_tagInsertEnable_set(uint32 enabled)
{
  	rtl8309n_regbit_set(2, 16, 6, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);	

  	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_tagInsertEnable_get
 * Description:
 *		Get enbaled status of CPU tag insert ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_cpu_tagInsertEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(2, 16, 6, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
  	return SUCCESS;	
}

/* Function Name:
 *		rtl8309n_cpu_tagRemoveEnable_set
 * Description:
 *		Enable remove CPU tag when receive a packet from cpu port
 * Input:
 *		enabled
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_cpu_tagRemoveEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(2, 16, 7, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_tagRemoveEnable_get
 * Description:
 *		Get status of removing cpu tag
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_tagRemoveEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(2, 16, 7, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
  	return SUCCESS;		
}

/* Function Name:
 *		rtl8309n_cpu_cpuTagAwareEnable_set
 * Description:
 *		Enable CPU tag aware ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_cpuTagAwareEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(2, 16, 5, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_cpu_cpuTagAwareEnable_get
 * Description:
 *		Get status of CPU tag aware ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 */
int32 rtl8309n_cpu_cpuTagAwareEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(2, 16, 5, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
  	return SUCCESS;		
}

/* Function Name:
 *		rtl8309n_cpu_learnEnable_set
 * Description:
 *		Enable CPU port learning ability
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_learnEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(2, 16, 9, 17, (RTL8309N_ENABLED==enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_learnEnable_set
 * Description:
 *		Enable CPU port learning ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_learnEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(2, 16, 9, 17, &bitVal);

	*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_checkCrcEnable_set
 * Description:
 *		Enable checking CRC for CPU tagged packets
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_checkCrcEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(2, 16, 10, 17, (RTL8309N_ENABLED == enabled) ? 0 : 1);

	return SUCCESS;

}

/* Function:
 *		rtl8309n_cpu_checkCrcEnable_get
 * Description:
 *		Get status of checking CRC ability
 * Input:
 *		none
 * Output:
 *		pEnabled
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_cpu_checkCrcEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(2, 16, 10, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;
	
  	return SUCCESS;		
}

/* Function Name:
 *		rtl8309n_cpu_nonCpuPortRxCpuTag_set
 * Description:
 *		Set action for none cpu port receive a cpu tagged frame
 * Input:
 *		action	-	action, drop or normal forward
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This Api can be called to set the action when port which is not cpu 
 *      port receives a cpu tagged packets.
 *      The action could be:
 *          RTL8309N_ACT_DROP   -   drop
 *          RTL8309N_ACT_PERMIT     -  forward as normal
 */
int32 rtl8309n_cpu_nonCpuPortRxCpuTag_set(uint32 action)
{
	if ((action != RTL8309N_ACT_DROP) && (action != RTL8309N_ACT_PERMIT))
		return FAILED;

	switch(action)
	{
		case RTL8309N_ACT_DROP:
			rtl8309n_regbit_set(2, 16, 8, 17, 0);
			break;

		case RTL8309N_ACT_PERMIT:
			rtl8309n_regbit_set(2, 16, 8, 17, 1);
			break;

		default:
			break;
	}

	return SUCCESS;
		
}

/* Function Name:
 *		rtl8309n_cpu_nonCpuPortRxCpuTag_get
 * Description:
 *		Get action for none cpu port receive a cpu tagged frame
 * Input:
 *		none
 * Output:
 *		pAction	-	action, drop or normal forward
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This Api can be called to get the action when port which is not cpu 
 *      port receives a cpu tagged packets.
 *      The action could be:
 *          RTL8309N_ACT_DROP   -   drop
 *          RTL8309N_ACT_PERMIT     -  forward as normal
 */
int32 rtl8309n_cpu_nonCpuPortRxCpuTag_get(uint32 *pAction)
{
	uint32 bitVal;
	
	if (NULL == pAction)
		return FAILED;

	rtl8309n_regbit_get(2, 16, 8, 17, &bitVal);
	switch(bitVal)
	{
		case 0:
			*pAction = RTL8309N_ACT_DROP;
			break;

		case 1:
			*pAction = RTL8309N_ACT_PERMIT;
			break;

		default:
			break;
	}

	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_cpu_port_set
 * Description:
 *      Specify Asic CPU port 
 * Input:
 *      port       -   Specify the port
 *      enTag      -   CPU tag insert or not
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8309N_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
int32 rtl8309n_cpu_port_set(uint32 port, uint32 enTag) 
{

    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
    
	/*enable cpu port function*/
    rtl8309n_cpu_enable_set(RTL8309N_ENABLED);    
    
    /*enable cpu tag aware*/
    rtl8309n_cpu_cpuTagAwareEnable_set(RTL8309N_ENABLED);
    /*whether inserting CPU tag*/
    rtl8309n_cpu_tagInsertEnable_set(enTag ? RTL8309N_ENABLED : RTL8309N_DISABLED);
    /*enable removing CPU tag*/
    rtl8309n_cpu_tagRemoveEnable_set(RTL8309N_ENABLED);
	/*set cpu port number*/
    rtl8309n_cpu_portNumber_set(port);

    /*enable asic recaculate crc for pkt with cpu tag*/
    rtl8309n_cpu_checkCrcEnable_set(RTL8309N_ENABLED);
    /*enable cpu port learn ability*/
	rtl8309n_cpu_learnEnable_set(RTL8309N_ENABLED);
    /*disable IEEE802.1x function of CPU Port*/
    rtl8309n_dot1x_portBasedEnable_set(port, RTL8309N_DISABLED);
    rtl8309n_dot1x_macBasedEnable_set(port, RTL8309N_DISABLED);
  
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_cpu_port_get
 * Description:
 *       Get Asic CPU port number
 * Input:
 *      none
 * Output:
 *      pPort     - the pointer of CPU port number
 *      pEnTag    - the pointer of CPU tag insert or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8309N_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
int32 rtl8309n_cpu_port_get(uint32 *pPort, uint32 *pEnTag) 
{

    if ((NULL == pPort ) || (NULL == pEnTag))
        return FAILED;
 
    rtl8309n_cpu_portNumber_get(pPort);
    rtl8309n_cpu_tagInsertEnable_get(pEnTag);
    
    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_tagInsertMask_set
 * Description:
 *		Enable inserting CPU tag for special packets
 * Input:
 *		type    -   special packet type
 *      enabled -   enable,disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_cpu_tagInsertMask_set(rtl8309n_cpu_trapPkt_t type, uint32 enabled)
{
	if (type >= RTL8309N_CPU_TRAP_END)
		return FAILED;

    switch(type)
    {
        case RTL8309N_CPU_TRAP_DOT1X:
            rtl8309n_regbit_set(2, 17, 0, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
        case RTL8309N_CPU_TRAP_VID:
            rtl8309n_regbit_set(2, 17, 1, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;  
        case RTL8309N_CPU_TRAP_ISPMAC:
            rtl8309n_regbit_set(2, 17, 2, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
        case RTL8309N_CPU_TRAP_ACL:
            rtl8309n_regbit_set(2, 17, 3, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;   
        case RTL8309N_CPU_TRAP_IGMPMLD:
            rtl8309n_regbit_set(2, 17, 4, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
        case RTL8309N_CPU_TRAP_RMA:
            rtl8309n_regbit_set(2, 17, 5, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;   
        case RTL8309N_CPU_TRAP_EXMACLIMIT:
            rtl8309n_regbit_set(2, 17, 6, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
        case RTL8309N_CPU_TRAP_RLPP:
            rtl8309n_regbit_set(2, 17, 7, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;  
        case RTL8309N_CPU_TRAP_LUTMACBLOCK:
            rtl8309n_regbit_set(2, 17, 8, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;            
    }
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_cpu_tagInsertMask_get
 * Description:
 *		Get enabled status of inserting CPU tag for special packets
 * Input:
 *		type    -   special packet type
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_cpu_tagInsertMask_get(rtl8309n_cpu_trapPkt_t type, uint32 *pEnabled)
{
	uint32 bitVal;
    
    if (type >= RTL8309N_CPU_TRAP_END)
            return FAILED;
    
    switch(type)
    {
        case RTL8309N_CPU_TRAP_DOT1X:
            rtl8309n_regbit_get(2, 17, 0, 17, &bitVal);
            break;
        case RTL8309N_CPU_TRAP_VID:
            rtl8309n_regbit_get(2, 17, 1, 17, &bitVal);
            break;  
        case RTL8309N_CPU_TRAP_ISPMAC:
            rtl8309n_regbit_get(2, 17, 2, 17, &bitVal);
            break;
        case RTL8309N_CPU_TRAP_ACL:
            rtl8309n_regbit_get(2, 17, 3, 17, &bitVal);
            break;   
        case RTL8309N_CPU_TRAP_IGMPMLD:
            rtl8309n_regbit_get(2, 17, 4, 17, &bitVal);
            break;
        case RTL8309N_CPU_TRAP_RMA:
            rtl8309n_regbit_get(2, 17, 5, 17, &bitVal);
            break;   
        case RTL8309N_CPU_TRAP_EXMACLIMIT:
            rtl8309n_regbit_get(2, 17, 6, 17, &bitVal);
            break;
        case RTL8309N_CPU_TRAP_RLPP:
            rtl8309n_regbit_get(2, 17, 7, 17, &bitVal);
            break;  
        case RTL8309N_CPU_TRAP_LUTMACBLOCK:
            rtl8309n_regbit_get(2, 17, 8, 17, &bitVal);
            break; 
        default:
            break;
    }

    *pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
    
	return SUCCESS;
}



/* Function Name:
 *      rtl8309n_qos_softReset_set
 * Description:
 *      Software reset the asic
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Set switch to software reset.
 */
 
int32 rtl8309n_qos_softReset_set(void) 
{    
    uint32 bitVal;
    /*software reset*/
    rtl8309n_regbit_set(0, 16, 1, 14, 1); 
    while (1)
    {
        rtl8309n_regbit_get(0, 16, 1, 14, &bitVal);
        if (!bitVal)
            break;
    }
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_queueNum_set
 * Description:
 *      Set egress port queue number 
 * Input:
 *		num		-	queue number(1 ~4)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch. It's value is from 1 to 4.
 */
int32 rtl8309n_qos_queueNum_set(uint32 num)
{
    uint32 regValue;
    
    if ((num < 1) ||(num > RTL8309N_MAX_QUEUE_NUM))
        return FAILED;
    
    rtl8309n_reg_get(5, 16, 16, &regValue);
    regValue &= ~0x3;
	regValue |= (num - 1);
    rtl8309n_reg_set(5, 16, 16, regValue);
    
    /*A soft-reset is required after configuring queue num*/
    rtl8309n_qos_softReset_set();
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_queueNum_get
 * Description:
 *      Get egress port queue number 
 * Input:
 *      pNum	-	pointer point to queue number(1 ~4)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.It's value is from 1 to 4.
 */
int32 rtl8309n_qos_queueNum_get(uint32 *pNum) 
{
    uint32 regValue;

    if (NULL == pNum) 
        return FAILED;
    
    rtl8309n_reg_get(5, 16, 16, &regValue);
    *pNum = (regValue & 0x3) + 1;
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_priToQueMap_set
 * Description:
 *     Set priority to Queue ID mapping
 * Input:
 *      port       -  port id (0 - 8)
 *      priority   -  priority value (0 ~ 3)
 *      qid        -  Queue id (0 ~ 3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Set 2 bit user priority to queue id mapping. It's set for per port.
 */
int32 rtl8309n_qos_priToQueMap_set(uint32 port, uint32 priority, uint32 qid) 
{
    uint32 regValue, regNum;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if ((qid > RTL8309N_MAX_QUEUE_ID) || (priority > 3)) 
        return FAILED;
	
    regNum = 17 + port;
    rtl8309n_reg_get(6, regNum, 16, &regValue);
    switch(priority) 
    {
        case 0:
			regValue &= ~(0x3 << 6);
            regValue |= (qid << 6);
            break;
        case 1:
			regValue &= ~(0x3 << 4);
            regValue |= (qid << 4);
            break;
        case 2:
			regValue &= ~(0x3 << 2);
            regValue |= (qid << 2);
            break;
        case 3:
			regValue &= ~0x3;
            regValue |= qid;
            break;
        default:
            return FAILED;
    }
    rtl8309n_reg_set(6, regNum, 16, regValue);
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_priToQueMap_get
 * Description:
 *      Get priority to Queue ID mapping
 * Input:
 *      port       -  port id (0 - 8)
 *      priority   -  priority value (0 ~ 3)
 * Output:
 *      pQid      -  pointer of Queue id (0~3)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * Note:
 *		Set 2 bit user priority to queue id mapping. It's set for per port.
 */
int32 rtl8309n_qos_priToQueMap_get(uint32 port, uint32 priority, uint32 *pQid) 
{
    uint32 regValue, regNum;
	
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if ((priority > 3) || (NULL == pQid))
        return FAILED;

	regNum = 17 + port;
    rtl8309n_reg_get(6, regNum, 16, &regValue);
    switch(priority) 
    {
        case 0:
            *pQid = (regValue >> 6) & 0x3;
            break;
        case 1:
            *pQid = (regValue >> 4) & 0x3;
            break;
        case 2:
            *pQid = (regValue >> 2) & 0x3;
            break;
        case 3:
            *pQid = regValue & 0x3;
            break;
        default:
            return FAILED;
    }
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_portRate_set
 * Description:
 *      Set port bandwidth control
 * Input:
 *      port            -  port number (0 ~ 8)
 *      n64Kbps         -  Port rate, unit 64Kbps ;
 *						   0 - 0x640, port 0 - 7,  100Mps, UTP mode; 
 *                         0 - 0xc80, port 0/8, 200Mbps, Fiber mode;				   
 *      direction       -  Ingress or Egress bandwidth control
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured.
 *      The direction could be:
 *          RTL8309N_PORT_RX    -  port input bandwidth control. 
 *          RTL8309N_PORT_TX    -  port output bandwidth control.
 *      port rate unit is 64Kbps. 
 */
int32 rtl8309n_qos_portRate_set(uint32 port, uint32 n64Kbps, uint32 direction)
{
    uint32 regValue;
    
    if ((port > RTL8309N_MAX_PORT_ID) || (direction > RTL8309N_PORT_TX))
        return FAILED;
	if(((RTL8309N_PORT0 == port) || (RTL8309N_PORT8 == port)) && (n64Kbps > 0xC80))
		return FAILED;
	else if(((RTL8309N_PORT1 <= port) && (RTL8309N_PORT7 >= port)) && (n64Kbps > 0x640))
		return FAILED;
	
    if (RTL8309N_PORT_RX == direction) 
    {  
        /*configure port Rx rate*/
		rtl8309n_reg_get(port, 28, 9, &regValue);
		regValue &= ~0xFFF;
		regValue |= n64Kbps;
		rtl8309n_reg_set(port, 28, 9, regValue);
		
    } 
    else if(RTL8309N_PORT_TX == direction)
    {  
        /*configure port Tx rate*/
		rtl8309n_reg_get(port, 26, 9, &regValue);
		regValue &= ~0xFFF;
		regValue |= n64Kbps;
		rtl8309n_reg_set(port, 26, 9, regValue);
    }
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_portRate_get
 * Description:
 *      Get port bandwidth control rate
 * Input:
 *      port                 -  Port number (0~5)
 * Output:
 *      *pN64Kbps        -  Port rate (0~1526), unit 64Kbps
 *      direction           -  Input or output bandwidth control
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured.
 *      The direction could be:
 *          RTL8309N_PORT_RX    -  port input bandwidth control. 
 *          RTL8309N_PORT_TX    -  port output bandwidth control.
 *      port rate unit is 64Kbps. 
 */
int32 rtl8309n_qos_portRate_get(uint32 port, uint32 *pN64Kbps, uint32 direction) 
{
    uint32 regValue;
	
    if ((port > RTL8309N_MAX_PORT_ID) || (direction > RTL8309N_PORT_TX))
        return FAILED;
	if (NULL == pN64Kbps)
		return FAILED;
    
    if (RTL8309N_PORT_RX == direction)
    {
        /*Get port Rx rate*/
        rtl8309n_reg_get(port, 28, 9, &regValue);
        *pN64Kbps = regValue & 0xFFF;
    } 
    else 
    { 
        /*Get port Tx rate*/
        rtl8309n_reg_get(port, 26, 9, &regValue);
        *pN64Kbps = regValue & 0xFFF;
    }
	
    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_qos_queueRate_set
 * Description:
 *		Set queue rate
 * Input:
 *		port	-	port number(0 - 8)
 *		queue	-	queue number(2, 3)
 *		n64Kbps	-	rate value
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)For RTL8309N, only queue 2 and 3 per port have leaky bucket. So only queue 2 and 3's rate can be set.
 *		    Port rate unit is 64Kbps.
 *      (2)The queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
int32 rtl8309n_qos_queueRate_set(uint32 port, uint32 queue, uint32 n64Kbps)
{
	if((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_QUEUE3) || (queue < RTL8309N_QUEUE2))
		return FAILED;

	if(((RTL8309N_PORT0 == port) || (RTL8309N_PORT8 == port)) && (n64Kbps > 0xC80))
		return FAILED;
	else if(((RTL8309N_PORT1 <= port) && (RTL8309N_PORT7 >= port)) && (n64Kbps > 0x640))
		return FAILED;

	if(RTL8309N_QUEUE2 == queue)
		rtl8309n_reg_set(port, 21, 9, n64Kbps);
	else if(RTL8309N_QUEUE3 == queue)
		rtl8309n_reg_set(port, 22, 9, n64Kbps);
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_qos_queueRate_set
 * Description:
 *		Set queue rate
 * Input:
 *		port	-	port number(0 - 8)
 *		queue	-	queue number(2, 3)
 * Output:
 *		pN64Kbps	-	rate value
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)For RTL8309N, only queue 2 and 3 per port have leaky bucket. So only queue 2 and 3's rate can be set.
 *		    Port rate unit is 64Kbps.
 *      (2)The queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
int32 rtl8309n_qos_queueRate_get(uint32 port, uint32 queue, uint32 *pN64Kbps)
{
	uint32 regVal;
	
	if((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_QUEUE3) || (queue < RTL8309N_QUEUE2))
		return FAILED;
	if(NULL == pN64Kbps)
		return FAILED;

	if(RTL8309N_QUEUE2 == queue)
		rtl8309n_reg_get(port, 21, 9, &regVal);
	else if(RTL8309N_QUEUE3 == queue)
		rtl8309n_reg_get(port, 22, 9, &regVal);
	*pN64Kbps = regVal & 0xFFF;

	return SUCCESS;
}


/* Function Name:
 *		rtl8309n_qos_portRate_IfgIncludeEnable_set
 * Description:
 *		Enable including IFG when set port and queue rate 
 * Input:
 *		direction	-	direction of port, rx or tx
 *		port	-	port number (0 - 8)
 *		enabled	-	enable or disable	
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
int32 rtl8309n_qos_portRate_IfgIncludeEnable_set(uint32 direction, uint32 port, uint32 enabled)
{
	if(direction > RTL8309N_PORT_TX)
		return FAILED;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
	//calculate rate with or without ipg 
	if (RTL8309N_PORT_RX == direction) /*ingress*/
    	rtl8309n_regbit_set(port, 27, 1, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
	else if(RTL8309N_PORT_TX == direction) /*egress*/
		rtl8309n_regbit_set(port, 16, 3, 9, (RTL8309N_ENABLED == enabled) ? 0 : 1);
	else 
		return FAILED;
	
	return SUCCESS;

}

/* Function Name:
 *		rtl8309n_qos_portRate_IfgIncludeEnable_get
 * Description:
 *		Enable including IFG when set port and queue rate 
 * Input:
 *		direction	-	direction of port, rx or tx
 *		port	-	port number (0 - 8)
 * Output:
 *		pEnabled	-	enable or disable	
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
int32 rtl8309n_qos_portRate_IfgIncludeEnable_get(uint32 direction, uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;

	if(direction > RTL8309N_PORT_TX)
		return FAILED;
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if(NULL == pEnabled)
		return FAILED;
	
	if (RTL8309N_PORT_RX == direction)/*ingress*/
	{
		rtl8309n_regbit_get(port, 27, 1, 9, &bitVal);
		*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	}
	else if(RTL8309N_PORT_TX == direction)/*egress*/
	{
		rtl8309n_regbit_get(port, 16, 3, 9, &bitVal);
		*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;
	}
	else 
		return FAILED;
	
	

	return SUCCESS;
	
}


/* Function Name:
 *      rtl8309n_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~8)
 *      enabled    -  enable or disable
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For RTL8309N, it could use 1P remark ability to map 2 bit internal 
 *		priority to 3 bit 1Q tag priority.
 */
int32 rtl8309n_qos_1pRemarkEnable_set(uint32 port, uint32 enabled)
{

    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
 
    /*enable new 1p remarking function*/
    rtl8309n_regbit_set(port, 16, 5, 10, (RTL8309N_ENABLED == enabled) ? 1 : 0);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1pRemarkEnable_set
 * Description:
 *      Get 802.1P remarking ability status
 * Input:
 *      port        -  port number (0~8)
 * Output:
 *      pEnabled  -  pointer of the ability status, enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For RTL8309N, it could use 1P remark ability to map 2 bit internal 
 *		priority to 3 bit 1Q tag priority.
 */
int32 rtl8309n_qos_1pRemarkEnable_get(uint32 port, uint32 *pEnabled) 
{
    uint32 bitValue;
    
    if (port > RTL8309N_MAX_PORT_ID|| (NULL == pEnabled))
        return FAILED;
    
    rtl8309n_regbit_get(port, 16, 5, 10, &bitValue);
    *pEnabled = (bitValue ? RTL8309N_ENABLED : RTL8309N_DISABLED);
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 *      priority1p     -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		
 */
int32 rtl8309n_qos_1pRemark_set(uint32 priority, uint32 priority1p)
{
    uint32 regValue;

    if ( (priority > 3) || (priority1p > 7) ) 
        return FAILED;
    
    rtl8309n_reg_get(6, 16, 16, &regValue);
    switch(priority) 
    {
        case 0:
            regValue = (regValue & ~(0x7 << 0)) | (priority1p << 0);
            break;
        case 1:
            regValue = (regValue & ~(0x7 << 3)) | (priority1p << 3);
            break;
        case 2:
            regValue = (regValue & ~(0x7 << 6)) | (priority1p << 6);
            break;
        case 3:
            regValue = (regValue & ~(0x7 << 9)) | (priority1p << 9);
            break;
        default:
            return FAILED;
    }    
    rtl8309n_reg_set(6, 16, 16, regValue);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1pRemark_get
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 * Output:
 *      pPriority1p  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
int32 rtl8309n_qos_1pRemark_get(uint32 priority, uint32 *pPriority1p) 
{
    uint32 regValue;

    if ( (priority > 3) || (NULL == pPriority1p) ) 
        return FAILED;
    
    rtl8309n_reg_get(6, 16, 16, &regValue);
    switch(priority)
    {
        case 0:
            *pPriority1p = regValue & 0x7;
            break;
        case 1:
            *pPriority1p = (regValue >> 3) & 0x7;
            break;
        case 2:
            *pPriority1p = (regValue >> 6) & 0x7;
            break;
        case 3:
            *pPriority1p = (regValue >> 9) & 0x7;
            break;
        default:
            break;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_portPri_set
 * Description:
 *      Set port-based priority
 * Input:
 *      port          -  port number (0~8)
 *      priority      -  Packet port-based priority(0~3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */
int32 rtl8309n_qos_portPri_set(uint32 port, uint32 priority)
{
    uint32 regValue;

    if ((port > RTL8309N_MAX_PORT_ID) ||(priority > 3))
        return FAILED;
       
    rtl8309n_reg_get(port, 17, 10, &regValue);
    regValue &= ~0x3;
    regValue |= priority;
    rtl8309n_reg_set(port, 17, 10, regValue);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_portPri_get
 * Description:
 *      Get port-based priority
 * Input:
 *      port          -  port number (0~5)
 * Output:
 *      pPriority    -   pointer of packet port-based priority(0~4)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */ 
int32 rtl8309n_qos_portPri_get(uint32 port, uint32 *pPriority) 
{
    uint32 regVal;

    if ((port > RTL8309N_MAX_PORT_ID) ||(NULL == pPriority))
        return FAILED;
	
    rtl8309n_reg_get(port, 17, 10, &regVal);
    *pPriority = regVal & 0x3;

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1QPortPri_set
 * Description:
 *      Set 1Q port-based priority
 * Input:
 *      port          -  port number (0~8)
 *      priority      -  Packet port-based priority(0~3)
 * Output:
 *       none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When 1Q based priority source is enabled, a untag packet will be assigned a 
 *		1Q port-based priority correspond to the 1Q port based priority. A tagged packet will be 
 *		assigned the priority from its VLAN tag or priority tag.
 */
int32 rtl8309n_qos_1QPortPri_set(uint32 port, uint32 priority)
{
	uint32 regVal;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
	/*enable 1Q port based priority */
	rtl8309n_regbit_set(port, 18, 2, 10, 0);
	/*set 1Q port based priority*/
	rtl8309n_reg_get(port, 18, 10, &regVal);
    regVal &= ~0x3;
	regVal |= priority;
	rtl8309n_reg_set(port, 18, 10, regVal);
	
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1QPortPri_get
 * Description:
 *      Set 1Q port-based priority
 * Input:
 *      port          -  port number (0~8)
 * Output:
 *      pPriority	  -  pointer point to the 1Q port based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When 1Q based priority source is enabled, a untag packet will be assigned a 
 *		1Q port-based priority correspond to the 1Q port based priority. A tagged packet will be 
 *		assigned the priority from its VLAN tag or priority tag.
 */
int32 rtl8309n_qos_1QPortPri_get(uint32 port, uint32 *pPriority)
{
	uint32 regVal, regBitVal;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if (NULL == pPriority)
        return FAILED;
    
	rtl8309n_regbit_get(port, 18, 2, 10, &regBitVal);
	if(regBitVal)
		return FAILED;
	else
	{
		rtl8309n_reg_get(port, 18, 10, &regVal);
		*pPriority = regVal & 0x3;
	}
	
	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_1pPriRemap_set
 * Description:
 *      Set Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio   -  1Q-tag proirty (0~7, 3 bit value)
 *      prio      -  internal use 2-bit priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1Q tag priority has 3 bits, but switch internal use 2-bit priority. So it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
int32 rtl8309n_qos_1pPriRemap_set(uint32 tagprio, uint32 prio) 
{
    uint32 regValue;

    if ((tagprio > RTL8309N_MAX_1QTAG_PRIO_ID) || (prio >= RTL8309N_PRI_END))
        return FAILED;
    
    rtl8309n_reg_get(5, 18, 16, &regValue);
    switch(tagprio) 
    {
        case 0:
            regValue &= ~0x3;
            regValue |= prio;
            break;
        case 1:
            regValue &= ~(0x3 << 2);
            regValue |= (prio << 2);
            break;
        case 2:
            regValue &= ~(0x3 << 4);
            regValue |= (prio << 4);
            break;
        case 3:
            regValue &= ~(0x3 << 6);
            regValue |= (prio << 6);
            break;
        case 4:
            regValue &= ~(0x3 << 8);
            regValue |= (prio << 8);
            break;
        case 5:
            regValue &= ~(0x3 << 10);
            regValue |= (prio << 10);
            break;
        case 6:
            regValue &= ~(0x3 << 12);
            regValue |= (prio << 12);
            break;
        case 7:
            regValue &= ~(0x3 << 14);
            regValue |= (prio << 14);
            break;
        default:
            break;
    }    

    rtl8309n_reg_set(5, 18, 16, regValue);   
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_1pPriRemap_get
 * Description:
 *      Get Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio   -  1Q-tag proirty (0~7, 3 bit value)
 * Output:
 *      pPrio     -  pointer of  internal use 2-bit priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
int32 rtl8309n_qos_1pPriRemap_get(uint32 tagprio, uint32 *pPrio) 
{
    uint32 regValue;
    
    if ((tagprio > RTL8309N_MAX_1QTAG_PRIO_ID) || (NULL == pPrio))
        return FAILED;
    
    rtl8309n_reg_get(5, 18, 16, &regValue);
    switch(tagprio) 
    {
        case 0:
            *pPrio = regValue & 0x3;
            break;
        case 1:
            *pPrio = (regValue >> 2) & 0x3;
            break;
        case 2:
            *pPrio = (regValue >> 4) & 0x3;
            break;
        case 3:
            *pPrio = (regValue >> 6) & 0x3;
            break;
        case 4:
            *pPrio = (regValue >> 8) & 0x3;
            break;
        case 5:
            *pPrio = (regValue >> 10) & 0x3;
            break;
        case 6:
            *pPrio = (regValue >> 12) & 0x3;
            break;
        case 7:
            *pPrio = (regValue >> 14) & 0x3;
            break;
        default:
            break;
    }

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      value      -  dscp code
 *      priority   -  dscp-based priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:  
 *		DSCP has 64 value from 0 - 63.
 */ 
int32 rtl8309n_qos_dscpPriRemap_set(uint32 value, uint32 priority) 
{
    uint32 regValue;
	uint32 regNum, fieldNum;

    if ((value > 0x3F) ||(priority > RTL8309N_PRIO3))
        return FAILED;

	regNum = (value >> 3) + 19;
	fieldNum = (value - ((value >> 3) << 3)) << 1;
	rtl8309n_reg_get(5, regNum, 16, &regValue);
	regValue &= ~(0x3 << fieldNum);
	regValue |= (priority << fieldNum);
	rtl8309n_reg_set(5, regNum, 16, regValue);
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_dscpPriRemap_set
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      value      -  dscp code
 * Output:
 *      pPriority  -  the pointer of dscp-based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     
 */ 
int32 rtl8309n_qos_dscpPriRemap_get(uint32 value, uint32 *pPriority) 
{
    uint32  regValue;
	uint32  regNum, fieldNum;

    if ((value > 0x3F) || (NULL == pPriority))
        return FAILED;
    
	regNum = (value >> 3) + 19;
	fieldNum = (value - ((value >> 3) << 3)) << 1;
	rtl8309n_reg_get(5, regNum, 16, &regValue);
	*pPriority = (regValue >> fieldNum) & 0x3;
	
    return SUCCESS;
} 


/* Function Name:
 *      rtl8309n_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration weight
 * Input:
 *      pPriArbit  - pointer point to the structure describe weight of 4 priority sources
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 8 types of priority source at most, 
 *      and a packet properly has all of them. Among them, there 
 *      are 4 type priorities could be set priority weight, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority. Each one could be set weight from 0 to 4, 
 *      arbitration module will decide their sequece to take based on 
 *      their weight, the highest weight priority will be adopted at first, 
 *      then  priority type of the sencond highest weight. 
 */
int32 rtl8309n_qos_priSrcArbit_set(rtl8309n_qos_priArbitPara_t *pPriArbit) 
{
    uint32 regval;

    if ((pPriArbit->acl_pri_weight> 3) || (pPriArbit->dscp_pri_weight> 3) ||
        (pPriArbit->dot1q_pri_weight> 3) || (pPriArbit->port_pri_weight> 3))
        return FAILED;
	
    rtl8309n_reg_get(5, 17, 16, &regval);
	 
    /*acl based priority*/	
    regval &= ~0x3;
	regval |= pPriArbit->acl_pri_weight;   
    /*DSCP based priority*/
	regval &= ~(0x3 << 2);
	regval |= pPriArbit->dscp_pri_weight << 2;
	/*1Q based priority*/
	regval &= ~(0x3 << 4);
	regval |= pPriArbit->dot1q_pri_weight << 4;
    /*port based priority*/
    regval &= ~(0x3 << 6);
	regval |= pPriArbit->port_pri_weight << 6;

    rtl8309n_reg_set(5, 17, 16, regval);
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration weight
 * Input:
 *      none
 * Output:
 *      pPriArbit  - The poniter point to the structure describe weight of 4 priority sources
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 8 types of priority source at most, 
 *      and a packet properly has all of them. Among them, there 
 *      are 4 type priorities could be set priority weight, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority. Each one could be set weight from 0 to 4, 
 *      arbitration module will decide their sequece to take based on 
 *      their weight, the highest weight priority will be adopted at first, 
 *      then  priority type of the sencond highest weight. 
 */
int32 rtl8309n_qos_priSrcArbit_get(rtl8309n_qos_priArbitPara_t *pPriArbit)
{
    uint32 regval;
        
    if (NULL == pPriArbit)
        return FAILED;

    pPriArbit->acl_pri_weight= 0;
	pPriArbit->dscp_pri_weight= 0;
    pPriArbit->dot1q_pri_weight= 0;
    pPriArbit->port_pri_weight= 0;

	rtl8309n_reg_get(5, 17, 16, &regval);
    /*acl based priority*/
	pPriArbit->acl_pri_weight = regval & 0x3;
    /*dscp based priority*/
	pPriArbit->dscp_pri_weight = (regval >> 2) & 0x3;
    /*1Q based priority*/
	pPriArbit->dot1q_pri_weight = (regval >> 4) & 0x3;
    /*port based priority*/
	pPriArbit->port_pri_weight = (regval >> 6) & 0x3;
        
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for ingress port
 * Input:
 *      port      -  Specify port number ,0~8
 *      priSrc    -  Specify priority source  
 *      enabled   -  DISABLED or ENABLED 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 5 kind of priority source for each port which could
 *     be enabled or disabled:
 *		RTL8309N_CPUTAG_PRI     -   cpu tag based priority
 *		RTL8309N_IP_PRI        -   IP address based priority
 *		RTL8309N_DSCP_PRI      -   DSCP based priority
 *		RTL8309N_1Q_PRI        -   DOT1Q based priority
 *		RTL8309N_PORT_PRI      -   port based priority
 */
int32 rtl8309n_qos_priSrcEnable_set(uint32 port, uint32 priSrc, uint32 enabled) 
{
      
    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
	if (priSrc >= RTL8309N_QOS_PRISRC_END)
		return FAILED;
	
	switch(priSrc)
	{
    	case RTL8309N_CPUTAG_PRI:
    		rtl8309n_regbit_set(port, 16, 0, 10, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    		break;
    	case RTL8309N_IP_PRI:
    		rtl8309n_regbit_set(port, 16, 1, 10, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    		break;
    	case RTL8309N_DSCP_PRI:
    		rtl8309n_regbit_set(port, 16, 2, 10, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    		break;
    	case RTL8309N_1Q_PRI:
    		rtl8309n_regbit_set(port, 16, 3, 10, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    		break;
    	case RTL8309N_PORT_PRI:
    		rtl8309n_regbit_set(port, 16, 4, 10, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    		break;
    	default:
    		break;
	}
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port       -  Specify port number (0 ~5)
 *      priSrc     -  Specify priority source  
 * Output:
 *      pEnabled   -  the pointer of priority source status  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 5 kind of priority source for each port which could
 *     be enabled or disabled:
 *		RTL8309N_CPUTAG_PRI     -   cpu tag based priority
 *		RTL8309N_IP_PRI        -   IP address based priority
 *		RTL8309N_DSCP_PRI      -   DSCP based priority
 *		RTL8309N_1Q_PRI        -   DOT1Q based priority
 *		RTL8309N_PORT_PRI      -   port based priority
 */
int32 rtl8309n_qos_priSrcEnable_get(uint32 port, uint32 priSrc, uint32 *pEnabled) 
{
    uint32 bitValue;

    if ((port > RTL8309N_MAX_PORT_ID) || (priSrc >= RTL8309N_QOS_PRISRC_END) || (NULL == pEnabled))
        return FAILED; 
    
	switch(priSrc)
	{
	case RTL8309N_CPUTAG_PRI:
		rtl8309n_regbit_get(port, 16, 0, 10, &bitValue);
		break;
	case RTL8309N_IP_PRI:
		rtl8309n_regbit_get(port, 16, 1, 10, &bitValue);
		break;
	case RTL8309N_DSCP_PRI:
		rtl8309n_regbit_get(port, 16, 2, 10, &bitValue);
		break;
	case RTL8309N_1Q_PRI:
		rtl8309n_regbit_get(port, 16, 3, 10, &bitValue);
		break;
	case RTL8309N_PORT_PRI:
		rtl8309n_regbit_get(port, 16, 4, 10, &bitValue);
		break;
	default:
		break;
	}

	*pEnabled = (bitValue ? RTL8309N_DISABLED : RTL8309N_ENABLED);
	
    return SUCCESS;

}

/* Function Name:
 *		rtl8309n_qos_ipAddrPriEnable_set
 * Description:
 *		Enable IP address based priority
 * Input:
 *		entry	-	entry type
 *		enabled	-	enable or disable
 * Output:
 * 		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
int32 rtl8309n_qos_ipAddrPriEnable_set(uint32 entry, uint32 enabled)
{
    if (entry > RTL8309N_IPADD_B)
        return FAILED;
    
	switch(entry)
	{
		case RTL8309N_IPADD_A:
			rtl8309n_regbit_set(4, 22, 2, 16, (RTL8309N_ENABLED == enabled) ? 1: 0);
			break;

		case RTL8309N_IPADD_B:
			rtl8309n_regbit_set(4, 22, 6, 16, (RTL8309N_ENABLED == enabled) ? 1: 0);
			break;	

		default:
			break;
	}
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_qos_ipAddrPriEnable_get
 * Description:
 *		Get IP address based priority ability status
 * Input:
 *		entry	-	entry type
 * Output:
 * 		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
int32 rtl8309n_qos_ipAddrPriEnable_get(uint32 entry, uint32 *pEnabled)
{
	uint32 bitVal;
	
    if (entry > RTL8309N_IPADD_B)
        return FAILED;
	if (NULL == pEnabled)
		return FAILED;
	
	switch(entry)
	{
		case RTL8309N_IPADD_A:
			rtl8309n_regbit_get(4, 22, 2, 16, &bitVal);
			break;

		case RTL8309N_IPADD_B:
			rtl8309n_regbit_get(4, 22, 6, 16, &bitVal);
			break;	

		default:
			break;
	}
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_ipAddrPri_set
 * Description:
 *      Set IP address priority
 * Input:
 *		entry	-	indicate the ip address A or B
 *      priority  -  internal use 2-bit priority value (0 ~ 3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */

int32 rtl8309n_qos_ipAddrPri_set(uint32 entry, int32 priority) 
{
    uint32 regValue;

    if ((entry > RTL8309N_IPADD_B) || (priority > 3))
        return FAILED;
	
    switch(entry)
    {
    	case RTL8309N_IPADD_A:
    		rtl8309n_reg_get(4, 22, 16, &regValue);
            regValue &= ~0x3;
            regValue |= priority;
    		rtl8309n_reg_set(4, 22, 16, regValue);
			break;
			
		case RTL8309N_IPADD_B:				
    		rtl8309n_reg_get(4, 22, 16, &regValue);
            regValue &= ~(0x3 << 4);
            regValue |= (priority << 4);
    		rtl8309n_reg_set(4, 22, 16, regValue);			
			break;	
			
		default:
			break;
    }
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_ipAddrPri_get
 * Description:
 *      Get IP address priority
 * Input:
 *		entry	-	indicate the ip address A or B
 * Output:
 *      pPriority  -  internal use 2-bit priority value (0~3)  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
int32 rtl8309n_qos_ipAddrPri_get(uint32 entry, uint32 *pPriority)
{
    uint32 regValue;
	
	if (entry > RTL8309N_IPADD_B)
		return FAILED;
    if (NULL == pPriority)
        return FAILED;
    switch(entry)
    {
        case RTL8309N_IPADD_A:
    		rtl8309n_reg_get(4, 22, 16, &regValue);
    		*pPriority =  regValue & 0x3;
			break;
			
		case RTL8309N_IPADD_B:
    		rtl8309n_reg_get(4, 22, 16, &regValue);
    		*pPriority =  (regValue & 0x30) >> 4;
			break;	
			
		default:
			break;
    }
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_ipAddr_set
 * Description:
 *      Set IP address
 * Input:
 *      entry        -   specify entry
 *      ip           -   ip address
 *      mask         -  ip mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are two entries RTL8309N_IPADD_A and RTL8309N_IPADD_B
 *          for user setting ip address, if ip address of packet matches
 *          the entry, the packet will be assign the priority of ip address
 *          priority which is configured by rtl8309n_qos_ipAddrPri_set.
 *      (2)The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
int32 rtl8309n_qos_ipAddr_set(uint32 entry, uint32 ip, uint32 mask) 
{
    uint32 regValue;

    if (entry > 1) 
        return FAILED;

    switch(entry) 
    {
        case RTL8309N_IPADD_A:        
             regValue = ip & 0xFFFF;
             rtl8309n_reg_set(4, 23, 16, regValue);
             regValue = (ip & 0xFFFF0000) >> 16;
             rtl8309n_reg_set(4, 24, 16, regValue);
             regValue = mask & 0xFFFF;
             rtl8309n_reg_set(4, 25, 16, regValue);
             regValue = (mask & 0xFFFF0000) >> 16;
             rtl8309n_reg_set(4, 26, 16, regValue);
            break;
        case RTL8309N_IPADD_B:        
             regValue = ip & 0xFFFF;
             rtl8309n_reg_set(4, 27, 16, regValue);
             regValue = (ip & 0xFFFF0000) >> 16;
             rtl8309n_reg_set(4, 28, 16, regValue);
             regValue = mask & 0xFFFF;
             rtl8309n_reg_set(4, 29, 16, regValue);
             regValue = (mask & 0xFFFF0000) >> 16;
             rtl8309n_reg_set(4, 30, 16, regValue);   
            break;
        default:
            break;
    }

    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_ipAddr_get
 * Description:
 *      Get IP address user seting
 * Input:
 *      entry       -   specify entry
 * Output:
 *      pIp            -   ip address
 *      pMask        -   ip mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are two entries RTL8309N_IPADD_A and RTL8309N_IPADD_B
 *          for user setting ip address, if ip address of packet matches
 *          the entry, the packet will be assign the priority of ip address
 *          priority which is configured by rtl8309n_qos_ipAddrPri_set.
 *      (2)The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
int32 rtl8309n_qos_ipAddr_get(uint32 entry, uint32 *pIp, uint32 *pMask) 
{
    uint32 hi, lo;

    if ((entry > 1) || (pIp == NULL) || (pMask == NULL))
        return FAILED;
    
    switch (entry) 
    {
        case RTL8309N_IPADD_A :
            rtl8309n_reg_get(4, 23, 16, &lo);
            rtl8309n_reg_get(4, 24, 16, &hi);
            *pIp = lo + (hi << 16);
            rtl8309n_reg_get(4, 25, 16, &lo);
            rtl8309n_reg_get(4, 26, 16, &hi);
            *pMask = lo + (hi << 16);
            break;
        case RTL8309N_IPADD_B :
            rtl8309n_reg_get(4, 27, 16, &lo);
            rtl8309n_reg_get(4, 28, 16, &hi);
            *pIp = lo + (hi << 16);
            rtl8309n_reg_get(4, 29, 16, &lo);
            rtl8309n_reg_get(4, 30, 16, &hi);
            *pMask = lo + (hi << 16); 
            break;
        default :
            break;
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_qos_queueWrr_set
 * Description:
 *      Set qos scheduling parameter(WRR)
 * Input:
 *      port         -  port id
 *      sch_para     -  The structure describe queue scheduling parameter
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Either WRR or SP can be used for each of 4 queues. SP has higher priority than WRR,
 *		so queues with SP will be scanned firt when 4 queues are scheduled.I n rtl8309n_qos_schPara_t, q0_wt 
 *		and q1_wt, q2_wt and  q3_wt could be 0~127. 
 */
int32 rtl8309n_qos_queueWrr_set(uint32 port, rtl8309n_qos_schPara_t sch_para)
{
    uint32 regValue;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if ((sch_para.q0_wt > 127) || (sch_para.q1_wt > 127) || (sch_para.q2_wt > 127) ||
         (sch_para.q3_wt > 127))
         return FAILED;
	
	/*set weight of queue 0 and 1*/
	rtl8309n_reg_get(port, 17, 9, &regValue);
	regValue &= ~0x7F;
	regValue |= sch_para.q0_wt;
	regValue &= ~(0x7F << 8);
	regValue |= sch_para.q1_wt << 8;
    rtl8309n_reg_set(port, 17, 9, regValue); 

	/*set weight of queue 2 and 3*/
	rtl8309n_reg_get(port, 18, 9, &regValue);
	regValue &= ~0x7F;
	regValue |= sch_para.q2_wt;
	regValue &= ~(0x7F << 8);
	regValue |= sch_para.q3_wt << 8;
	rtl8309n_reg_set(port, 18, 9, regValue);
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_qos_queueWrr_get
 * Description:
 *      Set qos scheduling parameter(WRR)
 * Input:
 *      port           -  port id
 * Output:
 *      pSch_para      - the pointer of struct describing the schedule parameter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Either WRR or SP can be used for each of 4 queues. SP has higher priority than WRR,
 *		so queues with SP will be scanned firt when 4 queues are scheduled.In rtl8309n_qos_schPara_t, q0_wt 
 *		and q1_wt, q2_wt and  q3_wt could be 0~127. 
 */
int32 rtl8309n_qos_queueWrr_get(uint32 port, rtl8309n_qos_schPara_t *pSch_para)
{
    uint32 regValue;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
    if (NULL == pSch_para)
        return FAILED;

	/*get weight of queue 0 and 1*/
	rtl8309n_reg_get(port, 17, 9, &regValue);
	pSch_para->q0_wt = regValue & 0x7F;
	pSch_para->q1_wt = (regValue >> 8) & 0x7F;

	/*get weight of queue 2 and 3*/
	rtl8309n_reg_get(port, 18, 9, &regValue);
	pSch_para->q2_wt = regValue & 0x7F;
	pSch_para->q3_wt = (regValue >> 8) & 0x7F;
  
    return SUCCESS;
}

/*Function Name:
 *		rtl8309n_qos_queueStrictPriEnable_set
 *Description:
 *      Enable one queue's stric priority
 *Input:
 *      port    -   port number
 *      queue   -   queue number
 *      enabled   -  ENABLED or DISABLED
 *Output:
 *      none
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      (1)Every port of RTL8309N have 4 queue. Strict priority ability can be enable or disable for each queue.
 *          They can be enable or disable independently.
 *      (2)The queue could be:
 *              RTL8309N_QUEUE0     -   queue 0
 *              RTL8309N_QUEUE1     -   queue 1
 *              RTL8309N_QUEUE2     -   queue 2
 *              RTL8309N_QUEUE3     -   queue 3
 */
int32 rtl8309n_qos_queueStrictPriEnable_set(uint32 port, uint32 queue, uint32 enabled)
{
	if ((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_MAX_QUEUE_ID))
		return FAILED;

	switch(queue)
	{
		case RTL8309N_QUEUE0:
			rtl8309n_regbit_set(port, 17, 7, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
		case RTL8309N_QUEUE1:
			rtl8309n_regbit_set(port, 17, 15, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
		case RTL8309N_QUEUE2:
			rtl8309n_regbit_set(port, 18, 7, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
		case RTL8309N_QUEUE3:
			rtl8309n_regbit_set(port, 18, 15, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
		default:
			break;		
	}

	return SUCCESS;
	
}

/*Function Name:
 *		rtl8309n_qos_queueStrictPriEnable_get
 *Description:
 *      Enable one queue's stric priority
 *Input:
 *      port      -  port number
 *      queue     -  queue number
 *Output:
 *      pEnabled   -  pointer point to ENABLED or DISABLED
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      (1)Every port of RTL8309N have 4 queue. Strict priority ability can be enable or disable for each queue.
 *          They can be enable or disable independently.
 *      (2)The queue could be:
 *              RTL8309N_QUEUE0     -   queue 0
 *              RTL8309N_QUEUE1     -   queue 1
 *              RTL8309N_QUEUE2     -   queue 2
 *              RTL8309N_QUEUE3     -   queue 3
 */
int32 rtl8309n_qos_queueStrictPriEnable_get(uint32 port, uint32 queue, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if ((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_QUEUE3))
		return FAILED;
	if(NULL == pEnabled)
		return FAILED;

	switch (queue)
	{
		case RTL8309N_QUEUE0:
			rtl8309n_regbit_get(port, 17, 7, 9, &bitVal);
			break;
		case RTL8309N_QUEUE1:
			rtl8309n_regbit_get(port, 17, 15, 9, &bitVal);
			break;
		case RTL8309N_QUEUE2:
			rtl8309n_regbit_get(port, 18, 7, 9, &bitVal);
			break;
		case RTL8309N_QUEUE3:
			rtl8309n_regbit_get(port, 18, 15, 9, &bitVal);
			break;
			
		default:
			break;
	}

	*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);
	
	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_portLeakyBktEnable_set
 *Description:
 *     Enable port Tx or Rx leaky bucket
 *Input:
 *     port        -  port number(0 - 8)
 *     direction   -  direction, rx or tx
 *     enabled     -  enable or disable the leaky bucket
 *Output:
 *     none
 *Return:
 *	   SUCCESS
 *	   FAILED
 *Note:
 *	   (1)Every port of RTL8309N have rx or tx leaky bucket. Each port's Tx or Rx leaky bucket can be enabled 
 *	        or disabled independently.
 *     (2)The direction could be:
 *          RTL8309N_PORT_TX    -   ouput
 *          RTL8309N_PORT_RX    -   input        
 */
int32 rtl8309n_qos_portLeakyBktEnable_set(uint32 port, uint32 direction, uint32 enabled)
{
	if((port > RTL8309N_MAX_PORT_ID) || (direction > RTL8309N_PORT_TX))
		return FAILED;

	if(RTL8309N_PORT_TX == direction)
		rtl8309n_regbit_set(port, 16, 2, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
	else if (RTL8309N_PORT_RX == direction)
		rtl8309n_regbit_set(port, 27, 0, 9, (RTL8309N_ENABLED == enabled) ? 0 : 1);

	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_portLeakyBktEnable_set
 *Description:
 *     Enable port Tx or Rx leaky bucket
 *Input:
 *     port         -  port number(0 - 8)
 *     direction    -  direction, rx or tx
 *Output:
 *     pEnabled     -  pointer point to the enabling status of leaky bucket
 *Return:
 *	   SUCCESS
 *	   FAILED
 *Note:
 *	   (1)Every port of RTL8309N have rx or tx leaky bucket. Each port's Tx or Rx leaky bucket can be enabled 
 *	        or disabled independently.
 *     (2)The direction could be:
 *          RTL8309N_PORT_TX    -   ouput
 *          RTL8309N_PORT_RX    -   input         
 */
int32 rtl8309n_qos_portLeakyBktEnable_get(uint32 port, uint32 direction, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if((port > RTL8309N_PORT_NUMBER - 1) || (direction > RTL8309N_PORT_TX))
		return FAILED;

	if(NULL == pEnabled)
		return FAILED;
	
	if(RTL8309N_PORT_TX == direction)
	{
	    rtl8309n_regbit_get(port, 16, 2, 9, &bitVal);
		*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);
	}
	else if (RTL8309N_PORT_RX == direction)
	{
		rtl8309n_regbit_get(port, 27, 0, 9, &bitVal);
		*pEnabled = (bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED);
	}

	
	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_queueLeakyBktEnable_set
 *Description:
 *     Enable output leaky buckt of a queue in a port
 *Input:
 *     port    -  port number(0 - 8)
 *     queue   -  queue number(2 - 3)
 *     enabled   -  ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)Every output port of RTL8309N have 4 queues. Only queue 3 and queue 2 have leaky bucket.
 *          They can be enable or disable independently.
 *     (2)queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
int32 rtl8309n_qos_queueLeakyBktEnable_set(uint32 port, uint32 queue, uint32 enabled)
{
	if((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_QUEUE3) || (queue < RTL8309N_QUEUE2))
		return FAILED;

	if(RTL8309N_QUEUE2 == queue)
		rtl8309n_regbit_set(port, 16, 0, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);
	else if(RTL8309N_QUEUE3 == queue)
		rtl8309n_regbit_set(port, 16, 1, 9, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_queueLeakyBktEnable_get
 *Description:
 *     Enable output leaky buckt of a queue in a port
 *Input:
 *     port    -  port number(0 - 8)
 *     queue   -  queue number(2 - 3)
 *Output:
 *     pEnabled   -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     Every output port of RTL8309N have 4 queues. Only queue 3 and queue 2 have leaky bucket.
 *     They can be enable or disable independently.
 */
int32 rtl8309n_qos_queueLeakyBktEnable_get(uint32 port, uint32 queue, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_QUEUE3) || (queue < RTL8309N_QUEUE2))
		return FAILED;

	if(NULL == pEnabled)
		return FAILED;
	
	if(RTL8309N_QUEUE2 == queue)
	    rtl8309n_regbit_get(port, 16, 0, 9, &bitVal);	
	else if(RTL8309N_QUEUE3 == queue)
		rtl8309n_regbit_get(port, 16, 1, 9, &bitVal);

	*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);

	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_queueFlcEnable_set
 *Description:
 *     Enable flow control ability of one queue
 *Input:
 *     port      -   port number (0 - 8)
 *     queue     -   queue number (0 - 3)
 *     enabled   -   ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *	   (1)Enble one queue's flow control ability. every port have 4 queues for RTL8309N.
 *        Each queue's flow control ability can be enabled or disabled independently.	
 *     (2)The port could be:
 *          RTL8309N_PORT0, RTL8309N_PORT1, RTL8309N_PORT2, RTL8309N_PORT3, 
 *          RTL8309N_PORT4, RTL8309N_PORT5, RTL8309N_PORT5, RTL8309N_PORT6, 
 *          RTL8309N_PORT7, RTL8309N_PORT8,
 */
int32 rtl8309n_qos_queueFlcEnable_set(uint32 port, uint32 queue, uint32 enabled) 
{

    if ((port > RTL8309N_MAX_PORT_ID) ||(queue > RTL8309N_MAX_QUEUE_ID))
        return FAILED;
    
    /*Enable/Disable Flow control ability of the specified queue*/
    switch (port) 
    {
        case RTL8309N_PORT0://phy7, page16, reg 25, bit 0~3 
            rtl8309n_regbit_set(7, 25, queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break;
        case RTL8309N_PORT1://phy7, page16, reg 25, bit 4~7 
            rtl8309n_regbit_set(7, 25, 4 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break;
        case RTL8309N_PORT2://phy7, page16, reg 25, bit 8~11 
            rtl8309n_regbit_set(7, 25, 8 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break;
        case RTL8309N_PORT3://phy7, page16, reg 25, bit 12~15 
            rtl8309n_regbit_set(7, 25, 12 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break;
        case RTL8309N_PORT4://phy7, page16, reg 26, bit 0~3 
            rtl8309n_regbit_set(7, 26, queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break;
        case RTL8309N_PORT5://phy7, page16, reg 26, bit 4~7 
            rtl8309n_regbit_set(7, 26, 4 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
            break; 
		case RTL8309N_PORT6://phy7, page16, reg 26, bit 8~11 
            rtl8309n_regbit_set(7, 26, 8 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
			break;
		case RTL8309N_PORT7://phy7, page16, reg 26, bit 12~15 
            rtl8309n_regbit_set(7, 26, 12 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
			break;
		case RTL8309N_PORT8://phy7, page16, reg 24, bit 12~15 
            rtl8309n_regbit_set(7, 24, 12 + queue, 16, (RTL8309N_ENABLED== enabled) ? 0:1);
			break;
        default:
            break;
    }
    
    return SUCCESS;

}

/*Function Name:
 *     rtl8309n_qos_queueFlcEnable_get
 *Description:
 *     Enable flow control ability of one queue
 *Input:
 *     port      -   port number (0 - 8)
 *     queue     -   queue number (0 - 3)
 *Output:
 *     pEnabled   -   pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *	   (1)Enble one queue's flow control ability. every port have 4 queues for RTL8309N.
 *        Each queue's flow control ability can be enabled or disabled independently.	
 *     (2)The port could be:
 *          RTL8309N_PORT0, RTL8309N_PORT1, RTL8309N_PORT2, RTL8309N_PORT3, 
 *          RTL8309N_PORT4, RTL8309N_PORT5, RTL8309N_PORT5, RTL8309N_PORT6, 
 *          RTL8309N_PORT7, RTL8309N_PORT8,
 */
int32 rtl8309n_qos_queueFlcEnable_get(uint32 port, uint32 queue, uint32 *pEnabled) 
{
    uint32 bitValue;

    if ((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_MAX_QUEUE_ID) || (NULL == pEnabled))
        return FAILED;
    
    switch (port) 
    {
        case RTL8309N_PORT0://phy7, page16, reg 25, bit 0~3 
            rtl8309n_regbit_get(7, 25, queue, 16, &bitValue);
            break;
        case RTL8309N_PORT1://phy7, page16, reg 25, bit 4~7 
            rtl8309n_regbit_get(7, 25, 4 + queue, 16, &bitValue);
            break;
        case RTL8309N_PORT2://phy7, page16, reg 25, bit 8~11 
            rtl8309n_regbit_get(7, 25, 8 + queue, 16, &bitValue);
            break;
        case RTL8309N_PORT3://phy7, page16, reg 25, bit 12~15 
            rtl8309n_regbit_get(7, 25, 12 + queue, 16, &bitValue);
            break;
        case RTL8309N_PORT4://phy7, page16, reg 26, bit 0~3 
            rtl8309n_regbit_get(7, 26, queue, 16, &bitValue);
            break;
        case RTL8309N_PORT5://phy7, page16, reg 26, bit 4~7 
            rtl8309n_regbit_get(7, 26, 4 + queue, 16, &bitValue);
            break; 
		case RTL8309N_PORT6://phy7, page16, reg 26, bit 8~11 
            rtl8309n_regbit_get(7, 26, 8 + queue, 16, &bitValue);
			break;
		case RTL8309N_PORT7://phy7, page16, reg 26, bit 12~15 
            rtl8309n_regbit_get(7, 26, 12 + queue, 16, &bitValue);
			break;
		case RTL8309N_PORT8://phy7, page16, reg 24, bit 12~15 
            rtl8309n_regbit_get(7, 24, 12 + queue, 16, &bitValue);
			break;
        default:
            break;
    }

    *pEnabled = (bitValue ? RTL8309N_DISABLED : RTL8309N_ENABLED);
    
    return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_queueFlcThr_set
 *Description:
 *     Set one queue's flow control threshold, include packet based and descriptor based types.
 *Input:
 *     port     -    port number (0 - 8)
 *     queue    -    queue number (0 - 3)
 *     type     -    indicate the queue flow controling threshold type
 *     value    -    value for the dedicated threshold
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)There are two flow controling types for queue. One is descriptor based and the other is packet based.
 *      Each type have flow control on and off threshold. The on and off threshold can be set independently 
 *      for each queue.
 *     (2)The queue flow control threshold type could be:
 *          RTL8309N_FLC_QUEUEDSCTHRLD_ON      -    queue descriptor flow control on threshold
 *          RTL8309N_FLC_QUEUEDSCTHRLD_OFF     -    queue descriptor flow control off threshold
 *          RTL8309N_FLC_QUEUEPKTTHRLD_ON      -    queue packet flow control on threshold 
 *          RTL8309N_FLC_QUEUEPKTTHRLD_OFF     -    queue packet flow control off threshold   
 *
 */
int32 rtl8309n_qos_queueFlcThr_set(uint32 port, uint32 queue, uint32 type, uint32 value)
{
    uint32 regNum;

    if ((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_MAX_QUEUE_ID) || (type >= RTL8309N_FLC_QUEUETHRLD_END) || (value > 127))
        return FAILED;
    
    switch (type) 
    {
        case RTL8309N_FLC_QUEUEDSCTHRLD_ON :  /*DSC , ON*/
			regNum = 16 + (queue << 1);
			rtl8309n_reg_set(port, regNum, 11, value);
			break;
		case RTL8309N_FLC_QUEUEDSCTHRLD_OFF :  /*DSC , OFF*/
			regNum = 17 + (queue << 1);
			rtl8309n_reg_set(port, regNum, 11, value);
			break;
		case RTL8309N_FLC_QUEUEPKTTHRLD_ON :  /*PACKET, ON*/
			regNum = 16 + (queue << 1);
			rtl8309n_reg_set(port, regNum, 12, value);
			break;
		case RTL8309N_FLC_QUEUEPKTTHRLD_OFF :  /*PACKET, OFF*/
			regNum = 17 + (queue << 1);
			rtl8309n_reg_set(port, regNum, 12, value);
			break;
            
        default:
            break;
     }

    return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_queueFlcThr_get
 *Description:
 *     Get one queue's flow control threshold, include packet based and descriptor based types.
 *Input:
 *     port     -   port number (0 - 8)
 *     queue    -   queue number (0 - 3)
 *     type     -   indicate the queue flow controling threshold type
 *Output:
 *     pValue   -   pointer point to the value for the dedicated threshold
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)There are two flow controling types for queue. One is descriptor based and the other is packet based.
 *      Each type have flow control on and off threshold. The on and off threshold can be set independently 
 *      for each queue.
 *     (2)The queue flow control threshold type could be:
 *          RTL8309N_FLC_QUEUEDSCTHRLD_ON      -    queue descriptor flow control on threshold
 *          RTL8309N_FLC_QUEUEDSCTHRLD_OFF     -    queue descriptor flow control off threshold
 *          RTL8309N_FLC_QUEUEPKTTHRLD_ON      -    queue packet flow control on threshold 
 *          RTL8309N_FLC_QUEUEPKTTHRLD_OFF     -    queue packet flow control off threshold   
 *
 */
int32 rtl8309n_qos_queueFlcThr_get(uint32 port, uint32 queue, uint32 type, uint32* pValue) 
{
    uint32 regNum;
    
    if ((port > RTL8309N_MAX_PORT_ID) || (queue > RTL8309N_MAX_QUEUE_ID) || (type >= RTL8309N_FLC_QUEUETHRLD_END) 
		|| (NULL == pValue))
        return FAILED;

    *pValue = 0;
    switch (type) 
    {
        case RTL8309N_FLC_QUEUEDSCTHRLD_ON :    /*DSC, ON*/
            regNum = 16 + (queue << 1);
			rtl8309n_reg_get(port, regNum, 11, pValue);
            break;
        case RTL8309N_FLC_QUEUEDSCTHRLD_OFF:    /*DSC, OFF*/
            regNum = 17 + (queue << 1);
			rtl8309n_reg_get(port, regNum, 11, pValue);
       		break;
        
        case RTL8309N_FLC_QUEUEPKTTHRLD_ON:    /*PACKET, ON*/
            regNum = 16 + (queue << 1);
			rtl8309n_reg_get(port, regNum, 12, pValue);         
            break; 
			
        case RTL8309N_FLC_QUEUEPKTTHRLD_OFF:    /*PACKET, OFF*/
            regNum = 17 + (queue << 1);
			rtl8309n_reg_get(port, regNum, 12, pValue);         
            break; 
			
        default:
            break;
    }

    return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_portDscFlcEnable_set
 *Description
 *     Enable port descriptor flow control ability
 *Input:
 *     port    -  port number(0 - 8)
 *     enabled -  ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The port descriptor based flow control will not be turn on untill the system descriptor based flow control 
 *     is turn on. And at this time port descriptor flow control ability must be enabled.
 */
int32 rtl8309n_qos_portDscFlcEnable_set(uint32 port, uint32 enabled)
{
    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
    
    /*Enable or Disable port discriptor based flow control*/
	/*phy7, page16, reg 23*/
    rtl8309n_regbit_set(7, 23, port, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);

    return SUCCESS;

}

/*Function Name:
 *     rtl8309n_qos_portDscFlcEnable_get
 *Description
 *     Enable port descriptor flow control ability
 *Input:
 *     port    -  port number(0 - 8)
 *Output:
 *     pEnabled  -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The port descriptor based flow control will not be turn on untill the system descriptor based flow control 
 *     is turn on. And at this time port descriptor flow control ability must be enabled.
 */
int32 rtl8309n_qos_portDscFlcEnable_get(uint32 port, uint32 *pEnabled)
{
    uint32 bitValue;

    if ((port > RTL8309N_MAX_PORT_ID) || (NULL == pEnabled))
        return FAILED;
    
    /*Enable/Disable port discriptor based flow control*/
	/*phy7, page16, reg 23*/
     rtl8309n_regbit_get(7, 23, port, 16, &bitValue);

    *pEnabled = (bitValue ? RTL8309N_DISABLED : RTL8309N_ENABLED);

    return SUCCESS;

}

/*Function Name:
 *     rtl8309n_qos_egrFlcEnable_get
 *Description
 *     Enable output flow control ability , include port and queue based flow control.
 *Input:
 *     port     -  port number(0 - 8)
 *     enabled  -  ENABLED or DISABLED
 *Output:
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The output flow control ability include port descriptor based, queue descriptor
 *     based and queue packet based type. This API can be called to enable the whole output flow control
 *     ability. Only after this ability is enabled, RTL8309N can really start to output flow control.
 */
int32 rtl8309n_qos_egrFlcEnable_set(uint32 port, uint32 enabled)
{   
   if (port > RTL8309N_MAX_PORT_ID)
		   return FAILED;
	   
   /*Enable/Disable output port  based flow control*/
	/*phy7, page16, reg 24*/
   rtl8309n_regbit_set(7, 24, port, 16, (RTL8309N_ENABLED == enabled) ? 0 : 1);

   return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_egrFlcEnable_get
 *Description
 *     Enable output flow control ability , include port and queue based flow control.
 *Input:
 *     port    -  port number(0 - 8)
 *Output:
 *     pEnabled  -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The output flow control ability include port descriptor based, queue descriptor
 *     based and queue packet based type. This API can be called to enable the whole output flow control
 *     ability. Only after this ability is enabled, RTL8309N can really start to output flow control.
 */
int32 rtl8309n_qos_egrFlcEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitValue;
	
	if (port > RTL8309N_MAX_PORT_ID)
			return FAILED;
		
	/*Enable/Disable output port based flow control*/
	/*phy7, page16, reg 24, bit 8*/
	rtl8309n_regbit_get(7, 24, port, 16, &bitValue);

	*pEnabled = (bitValue ? RTL8309N_DISABLED : RTL8309N_ENABLED);
	
	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_portFlcThr_set
 *Description
 *     Set the port descriptor based flow control turn on and turn off threshold, include RX and TX.
 *Input:
 *     port   -  port number
 *     onthr   -  turn on threshold
 *     offthr  -  turn off threshold
 *     direction  -  input or output
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *Note:
 *     (1)Set the port descriptor base flow control turn on and turn off threshold. There are input and output 
 *        port descriptor based flow control, so this API can be called to set the port descriptor based flow control
 *        threshold both for rx and tx, turn on and turn off.
 *     (2)The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
int32 rtl8309n_qos_portFlcThr_set(uint32 port, uint32 onthr, uint32 offthr, uint32 direction ) 
{

    if ((port > RTL8309N_MAX_PORT_ID) || (direction > RTL8309N_PORT_TX))
        return FAILED;
    
	switch(direction)
	{
    	case RTL8309N_PORT_RX:
    		rtl8309n_reg_set(port, 17, 13, onthr);
    		rtl8309n_reg_set(port, 18, 13, offthr);
    		break;
    	case RTL8309N_PORT_TX:
    		rtl8309n_reg_set(port, 24, 11, onthr);
    		rtl8309n_reg_set(port, 25, 11, offthr);
    		break;
    	default:
    		break;
	}
    
    return SUCCESS;
}

/*Function Name:
 *     rtl8309n_qos_portFlcThr_get
 *Description
 *     Set the port descriptor based flow control turn on and turn off threshold, include RX and TX.
 *Input:
 *     port   -  port number
 *     direction  -  input or output
 *Output:
 *     pOnthr   -  pointer point to turn on threshold
 *     pOffthr  -  pointer point to turn off threshold
 *Return:
 *     SUCCESS
 *Note:
 *     (1)Get the port descriptor base flow control turn on and turn off threshold. There are input and output 
 *        port descriptor based flow control, so this API can be called to set the port descriptor based flow control
 *        threshold both for rx and tx, turn on and turn off.
 *     (2)The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
int32 rtl8309n_qos_portFlcThr_get(uint32 port, uint32 *pOnthr, uint32 *pOffthr, uint32 direction) 
{

    if ((port > RTL8309N_MAX_PORT_ID) || (NULL == pOnthr) || (NULL == pOffthr) || (direction > RTL8309N_PORT_TX))
        return FAILED;
    
    switch(direction)
    {
        case RTL8309N_PORT_RX:
		    rtl8309n_reg_get(port, 17, 13, pOnthr);
		    rtl8309n_reg_get(port, 18, 13, pOffthr);
		    break;
	    case RTL8309N_PORT_TX:
		    rtl8309n_reg_get(port, 24, 11, pOnthr);
		    rtl8309n_reg_get(port, 25, 11, pOffthr);
		    break;		
	    default:
		    break;
    }
	
    return SUCCESS;
}

/*Function Name:
 *		rtl8309n_qos_igrFlcEnable_set
 *Description:
 *      Enable input flow control ability
 *Input:
 *      enabled
 *Output:
 *      none
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      none
 */
int32 rtl8309n_qos_igrFlcEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(7, 18, 0, 16, (RTL8309N_ENABLED == enabled) ? 1 : 0);
	
    return SUCCESS;

}

/*Function Name:
 *		rtl8309n_qos_igrFlcEnable_set
 *Description:
 *      Enable input flow control ability
 *Input:
 *      none
 *Output:
 *      pEnabled  - enable or disable
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      none
 */
int32 rtl8309n_qos_igrFlcEnable_get(uint32 *pEnabled)
{ 
	uint32 bitVal;

	rtl8309n_regbit_get(7, 18, 0, 16, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_acl_entry_set
 * Description:
 *      Set Asic ACL table
 * Input:
 *      entryAdd    -  Acl entry address (0~15)
 *      pAclData     -  struct describes the acl entry data
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *          0~8: port number, 
 *          RTL8309N_ACL_INVALID: invalid entry,
 *          RTL8309N_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8309N_ACT_DROP,
 *          RTL8309N_ACT_PERMIT, 
 *          RTL8309N_ACT_TRAP2CPU, 
 *          RTL8309N_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8309N_ACL_ETHER(ether type), 
 *          RTL8309N_ACL_TCP(TCP), 
 *          RTL8309N_ACL_UDP(UDP),
 *          RTL8309N_ACL_TCPUDP(TCP or UDP)
 *      Acl priority:
 *          RTL8309N_PRIO0 ~ RTL8309N_PRIO3     
 */
int32 rtl8309n_acl_entry_set(uint32 entryAdd, rtl8309n_acl_entry_t *pAclData) 
{
    uint32 regValue;
    uint32 pollcnt;
    uint32 bitValue;
	uint32 act, pro;
	
    if ((entryAdd > RTL8309N_ACL_ENTRYNUM - 1) || (pAclData->phyPort> RTL8309N_ACL_ANYPORT) || 
        (pAclData->action > RTL8309N_ACT_MIRROR) ||(pAclData->protocol > RTL8309N_ACL_TCPUDP) ||
        (pAclData->priority > RTL8309N_PRIO3))
        return FAILED;
	
	switch(pAclData->action)
	{
		case RTL8309N_ACT_DROP:
			act = 0;
			break;
		case RTL8309N_ACT_PERMIT:
			act = 1;
			break;
		case RTL8309N_ACT_TRAP2CPU:
			act = 2;
			break;
		case RTL8309N_ACT_MIRROR:
			act = 3;
			break;
		default:
			break;
	}
	switch(pAclData->protocol)
	{
		case RTL8309N_ACL_ETHER:
			pro = 0;
			break;
		case RTL8309N_ACL_TCP:
			pro = 1;
			break;
		case RTL8309N_ACL_UDP:
			pro = 2;
			break;
		case RTL8309N_ACL_TCPUDP:
			pro = 3;
			break;
		default:
			break;
	}

	/*Enable ACL access function*/
	rtl8309n_regbit_set(3, 16, 0, 15, 1);
	
    /*Enable CPU port function, Enable CPU TAG aware, Enable inserting CPU TAG, Enable removing CPU TAG */
	rtl8309n_reg_get(2, 16, 17, &regValue);
	regValue |= (1 << 7) | (1 << 6) | (1 << 5);
	regValue &= ~0x01;
	rtl8309n_reg_set(2, 16, 17, regValue);
    
    /*write a acl entry */
	/*entry address*/
	rtl8309n_reg_get(3, 17, 15, &regValue);
	regValue &= ~0xF;
	regValue |= entryAdd;
	rtl8309n_reg_set(3, 17, 15, regValue);
	/*data[0 - 15]*/
	regValue = pAclData->data & 0xFFFF;
	rtl8309n_reg_set(3, 18, 15, regValue);
	/*data[16 - 25]*/
    rtl8309n_reg_get(3, 19, 15, &regValue);
	regValue &= ~0x3FF;
    regValue |= (pAclData->priority << 8) | (act << 6) | (pAclData->phyPort << 2) | pro;
    rtl8309n_reg_set(3, 19, 15, regValue);
	/*triggle a command to write*/
	rtl8309n_regbit_set(3, 17, 14, 15, 0);
    rtl8309n_regbit_set(3, 17, 15, 15, 1);
    /*Polling whether the command is done*/
    for (pollcnt = 0; pollcnt < RTL8309N_IDLE_TIMEOUT; pollcnt++) 
    {
        rtl8309n_regbit_get(3, 17, 15, 15, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8309N_IDLE_TIMEOUT)
        return FAILED; 

	/*Disable ACL access function*/
	//rtl8309n_regbit_set(3, 16, 0, 15, 0);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_acl_entry_get
 * Description:
 *      Get Asic ACL entry
 * Input:
 *      entryAddr   -  Acl entry address (0 ~ 15)
 * Output:
 *      pAclData   -  pointer point to struct decribing the acl entry data
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *          0~8:       port number, 
 *          RTL8309N_ACL_INVALID: invalid entry,
 *          RTL8309N_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8309N_ACT_DROP,
 *          RTL8309N_ACT_PERMIT, 
 *          RTL8309N_ACT_TRAP2CPU, 
 *          RTL8309N_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8309N_ACL_ETHER(ether type), 
 *          RTL8309N_ACL_TCP(TCP), 
 *          RTL8309N_ACL_UDP(UDP),
 *          RTL8309N_ACL_TCPUDP(TCP or UDP)
 *      Acl priority:
 *          RTL8309N_PRIO0~RTL8309N_PRIO3    
 */
int32 rtl8309n_acl_entry_get(uint32 entryAddr, rtl8309n_acl_entry_t *pAclData)
{
    uint32 regValue;
    uint32 pollcnt;
    uint32 bitValue;
	uint32 act, pro;
	
    if ((entryAddr > (RTL8309N_ACL_ENTRYNUM - 1)) || (NULL == pAclData))
        return FAILED;

	/*Enable ACL access function*/
	rtl8309n_regbit_set(3, 16, 0, 15, 1);

    /*trigger a command to read ACL entry*/
	/*entry address*/
	rtl8309n_reg_get(3, 17, 15, &regValue);
	regValue &= ~0xF;
	regValue |= entryAddr;
	rtl8309n_reg_set(3, 17, 15, regValue);
	/*triggle a command to read*/
	rtl8309n_regbit_set(3, 17, 14, 15, 1);
	rtl8309n_regbit_set(3, 17, 15, 15, 1);
    /*Polling whether the command is done*/
    for (pollcnt = 0; pollcnt < RTL8309N_IDLE_TIMEOUT ; pollcnt++) 
    {
        rtl8309n_regbit_get(3, 17, 15, 15, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8309N_IDLE_TIMEOUT)
        return FAILED;
    /*read data*/
    rtl8309n_reg_get(3, 19, 15, &regValue);
	pAclData->priority = (regValue >> 8) & 0x3;
	act = (regValue >> 6) & 0x3;
	pAclData->phyPort = (regValue >> 2) & 0xF;
	pro = regValue & 0x3;
    rtl8309n_reg_get(3, 18, 15, &regValue);
    pAclData->data = regValue & 0xFFFF;

	switch(act)
	{
		case 0:
			pAclData->action = RTL8309N_ACT_DROP;
			break;
		case 1:
			pAclData->action = RTL8309N_ACT_PERMIT;
			break;
		case 2:
			pAclData->action = RTL8309N_ACT_TRAP2CPU;
			break;
		case 3:
			pAclData->action = RTL8309N_ACT_MIRROR;
			break;
		default:
			break;
	}
	switch(pro)
	{
		case 0:
			pAclData->protocol = RTL8309N_ACL_ETHER;
			break;
		case 1:
			pAclData->protocol = RTL8309N_ACL_TCP;
			break;
		case 2:
			pAclData->protocol = RTL8309N_ACL_UDP;
			break;
		case 3:
			pAclData->protocol = RTL8309N_ACL_TCPUDP;
			break;
		default:
			break;	
	}

	/*Disable ACL access function*/
	//rtl8309n_regbit_set(3, 16, 0, 15, 0);
	
    return SUCCESS;
}

int32 rtl8309n_mib_enable_set(uint32 enabled) 
{
    if (enabled > RTL8309N_ENABLED)
        return FAILED;

    rtl8309n_regbit_set(4, 16, 15, 15, enabled ? 1:0);

    return SUCCESS;
}

int32 rtl8309n_mib_enable_get(uint32 *pEnabled) 
{
    uint32 bitVal;
    
    if (pEnabled == NULL)
        return FAILED;

    rtl8309n_regbit_get(4, 16, 15, 15, &bitVal);
    *pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

    return FAILED;
}


int32 rtl8309n_mib_byteCnt_get(uint32 port, uint32 counter, uint32 *pValue) 
{
    uint32 regValue0, regValue1, regValue2, regValue3;
	uint32 regVal, bitVal;
	int32 i;

    if ((port > RTL8309N_MAX_PORT_ID) 
        || ((counter != RTL8309N_MIB_RXBYTECNT) &&(counter != RTL8309N_MIB_TXBYTECNT)) 
        || (NULL == pValue))
        return FAILED;
    
	/*read MIB counter*/
	regVal = (1 << 15) | (port << 4) | counter ;
    rtl8309n_reg_set(4, 18, 15, regVal);
    
	for(i = 0; i< RTL8309N_IDLE_TIMEOUT; i++)
	{
		rtl8309n_regbit_get(4, 18, 15, 15, &bitVal);
		if(!bitVal) 
            break;
	}
	if(RTL8309N_IDLE_TIMEOUT == i)
		return FAILED;

    rtl8309n_reg_get(4, 19, 15, &regValue0); //bit 0-15
    rtl8309n_reg_get(4, 20, 15, &regValue1); //bit 16-31
    

	/*read MIB counter*/
	regVal = (1 << 15) | (port << 4) | (counter+1) ;
    rtl8309n_reg_set(4, 18, 15, regVal);
    
	for(i = 0; i< RTL8309N_IDLE_TIMEOUT; i++)
	{
		rtl8309n_regbit_get(4, 18, 15, 15, &bitVal);
		if(!bitVal) 
            break;
	}
	if(RTL8309N_IDLE_TIMEOUT == i)
		return FAILED; 
    
    rtl8309n_reg_get(4, 21, 15, &regValue2); //bit 32-47
    rtl8309n_reg_get(4, 22, 15, &regValue3); //bit 48-63
    
    pValue[0] = regValue0 ;
    pValue[1] = regValue1 ;
    pValue[2] = regValue2 ;
    pValue[3] = regValue3 ;
    
    return SUCCESS;
}

int32 rtl8309n_mib_pktCnt_get(uint32 port, uint32 counter, uint32 *pValue) 
{
    uint32 regValue0, regValue1;
	uint32 regVal, bitVal;
	int32 i;

    if ((port > RTL8309N_MAX_PORT_ID) || (counter < RTL8309N_MIB_RXBYTECNT || counter > RTL8309N_MIB_RXSYMBLCNT) ||
        (NULL == pValue))
        return FAILED;
    
	/*read MIB counter*/
	regVal = (1 << 15) | (port << 4) | counter ;
    rtl8309n_reg_set(4, 18, 15, regVal);
    
	for(i = 0; i< RTL8309N_IDLE_TIMEOUT; i++)
	{
		rtl8309n_regbit_get(4, 18, 15, 15, &bitVal);
		if(!bitVal) break;
	}
	if(RTL8309N_IDLE_TIMEOUT == i)
		return FAILED;

    rtl8309n_reg_get(4, 19, 15, &regValue0); //bit 0-15
    rtl8309n_reg_get(4, 20, 15, &regValue1); //bit 16-31    

    
    *pValue = regValue1 << 16 | regValue0;

    
    return SUCCESS;
}


int32 rtl8309n_mib_start(uint32 port)
{
    /*Start counting*/
    rtl8309n_regbit_set(4, 16, port, 15, 1);

    return SUCCESS;
}

int32 rtl8309n_mib_stop(uint32 port)
{
    /*stop counting*/
    rtl8309n_regbit_set(4, 16, port, 15, 0);

    return SUCCESS;
}


/*Function Name:
 *		rtl8309n_mib_overFlowFlag_set
 *Description;
 *      Clear a port's MIB counter overflow flag 
 *Input:
 *      port     -  port number
 *      enaled   -  ENABLED or DISABLED
 *Output:
 *      none
 *Return:
 *		SUCCESS
 *      FAILED
 *Note:
 *      Write 1 to clear dedicate port's MIB counter overflow flag. When "enabled = ENABLED", overflow flag will be cleared.
 *      Otherwise, this API will do nothing.
 */
int32 rtl8309n_mib_overFlowFlag_set(uint32 port, uint32 overflow)
{

	if(port > RTL8309N_PORT_NUMBER - 1)
		return FAILED;
	
	rtl8309n_regbit_set(4, 23, port, 15, (TRUE == overflow) ? 1 : 0);

	return SUCCESS;
}


/*Function Name:
 *		rtl8309n_mib_overFlowFlag_get
 *Description;
 *      Get a port's MIB counter overflow flag 
 *Input:
 *      none
 *Output:
 *      pValue  -  pointer point to the overflow flag.
 *Return:
 *		SUCCESS
 *      FAILED
 *Note:
 *      none
 */
int32 rtl8309n_mib_overFlowFlag_get(uint32 port, uint32 *pOverflow)
{
	uint32 bitVal;

	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
	rtl8309n_regbit_get(4, 23, port, 15, &bitVal);
	*pOverflow = bitVal ? TRUE : FALSE;

	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_mib_reset
 * Description:
 *      reset MIB counter
 * Input:
 *      port         -  port number (0 ~ 5)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8309n_mib_reset(uint32 port) 
{
	uint32 bitVal;
    
    if ((port > RTL8309N_MAX_PORT_ID))
        return FAILED;
    
    /*Enable MIB counter*/
	rtl8309n_regbit_set(4, 16, 15, 15, 1);
    /*reset MIB counter*/
    rtl8309n_regbit_set(4, 17, port, 15, 1);
    while (1)
    {
        rtl8309n_regbit_get(4, 17, port, 15, &bitVal);
        if (!bitVal)
            break;
    }
    /*Start counting*/
    rtl8309n_regbit_set(4, 16, port, 15, 1);  
	
    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_mirror_selfFilterEnable_set
 * Description:
 *		Enable mirror port self filter ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror port to filter packet sent from itself.
 */
int32 rtl8309n_mirror_selfFilterEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(5, 16, 5, 15,(RTL8309N_ENABLED == enabled) ? 1 : 0 );
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_mirror_selfFilterEnable_get
 * Description:
 *		Get status of  mirror port self filter ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror port to filter packet sent from itself.
 */
int32 rtl8309n_mirror_selfFilterEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;

	rtl8309n_regbit_get(5, 16, 5, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_mirror_pauseFrameEnable_set
 * Description:
 *		Enable mirror pause frame ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror pause frame.
 */
int32 rtl8309n_mirror_pauseFrameEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(5, 16, 6, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0 );
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_mirror_pauseFrameEnable_get
 * Description:
 *		Get status of mirror pause frame ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror pause frame.
 */
int32 rtl8309n_mirror_pauseFrameEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;

	rtl8309n_regbit_get(5, 16, 6, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;

}

/* Function Name:
 *      rtl8309n_mirror_portBased_set
 * Description:
 *      Set asic Mirror port
 * Input:
 *      mirport         -  Specify mirror port 
 *      rxmbr           -  Specify Rx mirror port mask
 *      txmbr           -  Specify Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
int32 rtl8309n_mirror_portBased_set(uint32 mirport, uint32 rxmbr, uint32 txmbr) 
{
    uint32 regValue;

    if ((mirport > RTL8309N_MAX_PORT_ID) ||(rxmbr > RTL8309N_MAX_PORTMASK) || (txmbr > RTL8309N_MAX_PORTMASK) )
        return FAILED;

    /*Set Mirror Port*/
    rtl8309n_reg_get(5, 16, 15, &regValue);
    regValue &= ~0xf;
    regValue |= mirport;
    rtl8309n_reg_set(5, 16, 15, regValue);
    
    /*enable mirror self filter */
    rtl8309n_regbit_set(5, 16, 5, 15, 1);
        
    /*Set Ports Whose RX Data are Mirrored */
    rtl8309n_reg_get(5, 18, 15, &regValue);
	regValue &= ~ 0x1FF;
    regValue |= rxmbr;
    rtl8309n_reg_set(5, 18, 15, regValue);
    
    /*Set Ports Whose TX Data are Mirrored */
    rtl8309n_reg_get(5, 17, 15, &regValue);
	regValue &= ~0x1FF;
    regValue |= txmbr;
    rtl8309n_reg_set(5, 17, 15, regValue);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_mirror_portBased_get
 * Description:
 *      Get asic Mirror port
 * Input:
 *      none 
 * Output:
 *      pMirport     -  the pointer of mirror port
 *      pRxmbr       -  the pointer of  Rx mirror port mask
 *      pTxmbr       -  the pointer of Tx mirror port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
int32 rtl8309n_mirror_portBased_get(uint32 *pMirport, uint32 *pRxmbr, uint32* pTxmbr) 
{
    uint32 regValue;

    if ((NULL == pMirport) ||(NULL == pRxmbr) || (NULL == pTxmbr)) 
        return FAILED;

    /*Get Mirror Port*/
    rtl8309n_reg_get(5, 16, 15, &regValue);
    *pMirport = regValue & 0xF;
    
    /*Get Ports Whose RX Data are Mirrored*/
    rtl8309n_reg_get(5, 18, 15, &regValue);
    *pRxmbr = regValue & 0x1FF;
    
    /*Get Ports Whose TX Data are Mirrored */
    rtl8309n_reg_get(5, 17, 15, &regValue);
    *pTxmbr = regValue & 0x1FF;
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      enabled   - enable mirror packet by mac address
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
int32 rtl8309n_mirror_macBased_set(uint8 *macAddr, uint32 enabled) 
{
    if (NULL == macAddr)
        return FAILED;

    rtl8309n_regbit_set(5, 16, 4, 15, (RTL8309N_ENABLED == enabled)? 1:0);
    rtl8309n_reg_set(5, 19, 15, (macAddr[1] << 8) | macAddr[0]);
    rtl8309n_reg_set(5, 20, 15, (macAddr[3] << 8) | macAddr[2]);
    rtl8309n_reg_set(5, 21, 15, (macAddr[5] << 8) | macAddr[4]);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      none 
 * Output:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      pEnabled   - the pointer of enable mirror packet by mac address 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * 
 */
int32 rtl8309n_mirror_macBased_get(uint8 *macAddr, uint32 *pEnabled) 
{
    uint32 regValue;
    uint32 bitValue;

    if ((NULL == macAddr) || (NULL == pEnabled))
        return FAILED;
    
    rtl8309n_regbit_get(5, 16, 4, 15, &bitValue);
    *pEnabled = (bitValue  ? RTL8309N_ENABLED : RTL8309N_DISABLED);
	
    rtl8309n_reg_get(5, 19, 15, &regValue);
    macAddr[0] = regValue & 0xFF;
    macAddr[1] = (regValue >> 8) & 0xFF;
    rtl8309n_reg_get(5, 20, 15, &regValue);
    macAddr[2] = regValue & 0xFF;
    macAddr[3] = (regValue >> 8) & 0xFF;
    rtl8309n_reg_get(5, 21, 15, &regValue);
    macAddr[4] = regValue & 0xFF;
    macAddr[5] = (regValue >> 8) & 0xFF;

	return SUCCESS;
    
}

/*Function Name:
 *		rtl8309n_l2_hashmode_set
 *Description:
 *		Set hash algorithm 
 *Input:
 *		mode
 *Output:
 *		none
 *Return:
 *		FAILED
 *		SUCCESS
 *Note:
 *      The hash algorithm could be:
 *          RTL8309N_HASH_OPT0  -   Type I hash algorithm 
 *          RTL8309N_HASH_OPT1  -   Type II hash algorithm
 */
int32 rtl8309n_l2_hashmode_set(uint32 mode)
{	
	rtl8309n_regbit_set(6 , 16, 1, 15, (RTL8309N_HASH_OPT1 == mode) ? 1 : 0);

	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_hashmode_get
 *Description:
 *		Set hash algorithm 
 *Input:
 *		none
 *Output:
 *		pMode	-	pointer point to the hash algorithm mode
 *Return:
 *		FAILED
 *		SUCCESS
 *Note:
 *      The hash algorithm could be:
 *          RTL8309N_HASH_OPT0  -   Type I hash algorithm 
 *          RTL8309N_HASH_OPT1  -   Type II hash algorithm
 */
int32 rtl8309n_l2_hashmode_get(uint32 *pMode)
{
	uint32 mode;

	rtl8309n_regbit_get(6 , 16, 1, 15, &mode);

	*pMode = mode ? RTL8309N_HASH_OPT1 : RTL8309N_HASH_OPT0;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_MacToIdx_get
 * Description:
 *      get L2 table hash value from mac address
 * Input:
 *      macAddr        -  mac address
 *		fid			   -  fid value
 * Output:
 *      pIndex         -  mac address table index   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      when a mac address is learned into mac address table, 
 *      9 bit index value is got from the mac address by hashing 
 *      algorithm, each index corresponds to 4 entry, it means
 *      the table could save 4 mac addresses at the same time
 *      whose index value is equal, so switch mac address table 
 *      has 2048 entry. the API could get hash index from 
 *      a specified mac address and its  fid.
 */
int32 rtl8309n_l2_MacToIdx_get(uint8 *macAddr, uint32 fid, uint32* pIndex)
{
    uint32 tmp_index;
    uint32 hash_mode;

    if ((NULL == macAddr) || (fid > 3) ||(NULL == pIndex))
        return FAILED;

	/*get the hash mode*/
    rtl8309n_regbit_get(6, 16, 1, 15, &hash_mode);    
    *pIndex = 0;
    
    if (hash_mode) 
    {  
        /* hash algorithm 2*/

        /* Index 0 = FID0 ^ 15 ^ 23 ^ 31 ^ 39 ^ 32 */
		*pIndex |= (fid & 0x1) ^ ((macAddr[4] & 0x1) >> 0) ^ ((macAddr[3] & 0x1) >> 0) ^ ((macAddr[2] & 0x1) >> 0) ^ ((macAddr[1] & 0x1) >> 0) ^ ((macAddr[1] & 0x80) >> 7);

        /* Index_1 = 7 ^ 14 ^ 22 ^ 30 ^ 38 ^ 47 */
        tmp_index = ((macAddr[5] & 0x1) >> 0) ^ ((macAddr[4] & 0x2) >> 1) ^ ((macAddr[3] & 0x2) >> 1) ^ ((macAddr[2] & 0x2) >> 1) ^ ((macAddr[1] & 0x2) >> 1) ^ ((macAddr[0] & 0x1) >> 0);
        *pIndex |= tmp_index << 1;
        /* Index_2 = 6 ^ 13 ^ 21 ^ 29 ^ 37 ^ 46 */
        tmp_index = ((macAddr[5] & 0x2) >> 1) ^ ((macAddr[4] & 0x4) >> 2) ^ ((macAddr[3] & 0x4) >> 2) ^ ((macAddr[2] & 0x4) >> 2) ^ ((macAddr[1] & 0x4) >> 2) ^ ((macAddr[0] & 0x2) >> 1);
        *pIndex |= tmp_index << 2;

        /* Index_3 = 5 ^ 12 ^ 20 ^ 28 ^ 36 ^ 45 */
        tmp_index = ((macAddr[5] & 0x4) >> 2) ^ ((macAddr[4] & 0x8) >> 3) ^ ((macAddr[3] & 0x8) >> 3) ^ ((macAddr[2] & 0x8) >> 3) ^ ((macAddr[1] & 0x8) >> 3) ^ ((macAddr[0] & 0x4) >> 2);
        *pIndex |= tmp_index << 3;

        /* Index_4 = 4 ^ 19 ^ 27 ^ 35 ^ 44 */
        tmp_index = ((macAddr[5] & 0x8) >> 3) ^ ((macAddr[3] & 0x10) >> 4) ^ ((macAddr[2] & 0x10) >> 4) ^ ((macAddr[1] & 0x10) >> 4) ^ ((macAddr[0] & 0x8) >> 3);
        *pIndex |= tmp_index << 4;

        /* Index_5 = 3 ^ 11 ^ 26 ^ 34 ^ 43 */
        tmp_index = ((macAddr[5] & 0x10) >> 4) ^ ((macAddr[4] & 0x10) >> 4) ^ ((macAddr[2] & 0x20) >> 5) ^ ((macAddr[1] & 0x20) >> 5) ^ ((macAddr[0] &0x10) >> 4);
        *pIndex |= tmp_index << 5;

        /* Index_6 = 2 ^ 10 ^ 18 ^ 33 ^ 42 */
        tmp_index = ((macAddr[5] & 0x20) >> 5) ^ ((macAddr[4] & 0x20) >> 5) ^ ((macAddr[3] & 0x20) >> 5) ^ ((macAddr[1] & 0x40) >> 6) ^ ((macAddr[0] & 0x20) >> 5);
        *pIndex |= tmp_index << 6;

        /* Index_7 = 1 ^ 9 ^ 17 ^ 25 ^ 41 */
        tmp_index = ((macAddr[5] & 0x40) >> 6) ^ ((macAddr[4] & 0x40) >> 6) ^ ((macAddr[3] & 0x40) >> 6) ^ ((macAddr[2] & 0x40) >> 6) ^ ((macAddr[0] & 0x40) >> 6);
        *pIndex |= tmp_index << 7;

        /* Index_8 = FID1 ^ 0 ^ 8 ^ 16 ^ 24 ^ 40 */
        tmp_index = ((fid >> 1) & 0x1) ^ ((macAddr[5] & 0x80) >> 7) ^ ((macAddr[1] & 0x80) >> 7) ^ ((macAddr[3] & 0x80) >> 7) ^ ((macAddr[2] & 0x80) >> 7) ^ ((macAddr[0] & 0x80) >> 7);

		*pIndex |= tmp_index << 8;
		

    }
    else  
    {

        /*hash algorithm I*/
		/* Index_0 = 4 ^ 11 ^ 18 ^ 25 ^ 32*/
		*pIndex = ((macAddr[5] & 0x8) >> 3) ^ ((macAddr[4] & 0x10) >> 4) ^ ((macAddr[3] & 0x20) >> 5) ^ ((macAddr[2] & 0x40) >> 6) ^ ((macAddr[1] & 0x80) >> 7);

		/* Index_1 = 3 ^ 10 ^ 17 ^ 24 ^ 47*/
		tmp_index= ((macAddr[5] & 0x10) >> 4) ^ ((macAddr[4] & 0x20) >> 5) ^ ((macAddr[3] & 0x40) >> 6) ^ ((macAddr[2] & 0x80) >> 7) ^ ((macAddr[0] & 0x1) >> 0);
		*pIndex |= tmp_index << 1;

		/* Index_2 = 2 ^ 9 ^ 16 ^ 39 ^ 46*/
		tmp_index = ((macAddr[5] & 0x20) >> 5) ^ ((macAddr[4] & 0x40) >> 6) ^ ((macAddr[3] & 0x80) >> 7) ^ ((macAddr[1] & 0x1) >> 0) ^ ((macAddr[0] & 0x2) >> 1);
		*pIndex |= tmp_index << 2;

		/* Index_3 = 1 ^ 8 ^ 31 ^ 38 ^ 45*/
		tmp_index = ((macAddr[5] & 0x40) >> 6) ^ ((macAddr[4] & 0x80) >> 7) ^ ((macAddr[2] & 0x1) >> 0) ^ ((macAddr[1] & 0x2) >> 1) ^ ((macAddr[0] & 0x4) >> 2);
		*pIndex |= tmp_index << 3;

		/* Index_4 = FID1 ^ 0 ^ 23 ^ 30 ^ 37 ^ 44*/
		tmp_index = ((fid >> 1) & 0x1) ^ ((macAddr[5] & 0x80) >> 7) ^ ((macAddr[3] & 0x1) >> 0) ^ ((macAddr[2] & 0x2) >> 1) ^ ((macAddr[1] & 0x4) >> 2) ^ ((macAddr[0] & 0x8) >> 3);
		*pIndex |= tmp_index << 4;

		/* Index_5 = FID0 ^ 15 ^ 22 ^ 29 ^ 36 ^ 43*/
		tmp_index = (fid & 0x1) ^ ((macAddr[4] & 0x1) >> 0) ^ ((macAddr[3] & 0x2) >> 1) ^ ((macAddr[2] & 0x4) >> 2) ^ ((macAddr[1] & 0x8) >> 3) ^ ((macAddr[0] & 0x10) >> 4);
		*pIndex |= tmp_index << 5;

		/* Index_6 = 7 ^ 14 ^ 21 ^ 28 ^ 35 ^ 42*/
		tmp_index = ((macAddr[5] & 0x1) >> 0) ^ ((macAddr[4] & 0x2) >> 1) ^ ((macAddr[3] & 0x4) >> 2) ^ ((macAddr[2] & 0x8) >> 3) ^ ((macAddr[1] & 0x10) >> 4) ^ ((macAddr[0] & 0x20) >> 5);
		*pIndex |= tmp_index << 6;

		/* Index_7 = 6 ^ 13 ^ 20 ^ 27 ^ 34 ^ 41*/
		tmp_index = ((macAddr[5] & 0x2) >> 1) ^ ((macAddr[4] & 0x4) >> 2) ^ ((macAddr[3] & 0x8) >> 3) ^ ((macAddr[2] & 0x10) >> 4) ^ ((macAddr[1] & 0x20) >> 5) ^ ((macAddr[0] & 0x40) >> 6);
		*pIndex |= tmp_index << 7;

		/* Index_8 = 5 ^ 12 ^ 19 ^ 26 ^ 33 ^ 40*/
        tmp_index = ((macAddr[5] & 0x4) >> 2) ^ ((macAddr[4] & 0x8) >> 3) ^ ((macAddr[3] & 0x10) >> 4) ^ ((macAddr[2] & 0x20) >> 5) ^ ((macAddr[1] & 0x40) >> 6) ^ ((macAddr[0] & 0x80) >> 7);
		*pIndex |= tmp_index << 8;

    }
	
    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_macAddr_get
 * Description:
 *		Get mac address from l2 table entry
 * Input:
 *		entryAddr	-	entry address
 * Output:
 *		macAddr		-	pointer point to array of mac address
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		In a RTL8309N l2 table entry, 48 bits mac address are set in the same order and place from data[47:0], regardless
 *		of whether it's a unicast entry or a multicast entry. So this API can be called to get 48 bits mac address from l2 
 *		table entry, and then whether the entry is a unicast entry or a multicast can be decided based on the mac address bit[47]
 */
int32 rtl8309n_l2_macAddr_get(uint32 entryAddr, uint8 *macAddr)
{
	uint32 regVal, bitVal;
	uint32 pollCnt;
	
    /*Enable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 1);
	/*entry address*/
    regVal = entryAddr & 0x7FF;
    rtl8309n_reg_set(6, 18, 15, regVal);
	/*Read Command*/
	rtl8309n_regbit_set(6, 18, 12, 15, 1);
    rtl8309n_regbit_set(6, 18, 11, 15, 1);
    
    /*Waiting for Read command done and prevent polling dead loop*/
    for (pollCnt = 0; pollCnt < RTL8309N_IDLE_TIMEOUT; pollCnt ++) 
    {
        rtl8309n_regbit_get(6, 18, 11, 15, &bitVal);
        if (!bitVal)
            break;
    }
    if (pollCnt == RTL8309N_IDLE_TIMEOUT)
        return FAILED;
	
	/*Read data[47:32]*/
	rtl8309n_reg_get(6, 20, 15, &regVal);
	macAddr[5] = (regVal >> 8) & 0xFF;
	macAddr[4] = regVal & 0xFF;
	/*Read data[31:16]*/
	rtl8309n_reg_get(6, 21, 15, &regVal);
	macAddr[3] = (regVal >> 8) & 0xFF;
	macAddr[2] = regVal & 0xFF;
	/*Read data[15:0]*/
	rtl8309n_reg_get(6, 22, 15, &regVal);
	macAddr[1] = (regVal >> 8) & 0xFF;
	macAddr[0] = regVal & 0xFF;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_ucastEntry_set
 * Description:
 *      write an unicast mac address into L2 table
 * Input: 
 *      entryAddr           -  the entry address
 *      pL2_data            -  pointer point to the struct describing the unicast entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out 
 *          RTL8309N_LUT_AGE100     -   100s
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 */ 
int32 rtl8309n_l2_ucastEntry_set(uint32 entryAddr, rtl8309n_l2_ucastEntry_t *pL2_data)
{
    uint32 regValue, pollcnt;
    uint32 bitValue, age;

    /*For unicast entry, MAC[47] is 0  */
    if (pL2_data->mac.octet[0] & 0x1)
        return FAILED;
	if ((pL2_data->age > RTL8309N_LUT_AGE300) || (pL2_data->port > RTL8309N_PORT8))
		return FAILED;
	
    /*Enable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 1);

    /*Write Data[59:48]*/
	regValue = 0;
    if (RTL8309N_LUT_AGE300 == pL2_data->age) 
        age = 0x2;
    else if (RTL8309N_LUT_AGE200 == pL2_data->age)
        age = 0x3;
    else if (RTL8309N_LUT_AGE100 == pL2_data->age)
        age = 0x1;
    else if (RTL8309N_LUT_AGEOUT == pL2_data->age)
        age = 0;
    regValue = ((pL2_data->auth ? 1:0 ) << 11) | (pL2_data->da_block << 10) | (pL2_data->sa_block << 9) | 
		((pL2_data->isStatic ? 1:0) << 8) | (age << 6) | (pL2_data->port << 2) | pL2_data->fid;
	rtl8309n_reg_set(6, 19, 15, regValue & 0xFFFF);

    /*write Data[47:32], MAC[0:15],MAC[0,1],pMAC[4,5]*/
    rtl8309n_reg_set(6, 20, 15, ((pL2_data->mac.octet[5]) << 8) | pL2_data->mac.octet[4]);
    /*wrtie Data[31:16]*/
    rtl8309n_reg_set(6, 21, 15, ((pL2_data->mac.octet[3]) << 8) | pL2_data->mac.octet[2]);
    /*wrtie Data[15:0]*/
    rtl8309n_reg_set(6, 22, 15, ((pL2_data->mac.octet[1]) << 8) | pL2_data->mac.octet[0]);

	/*write entry address*/
	regValue = entryAddr & 0x7FF;
	rtl8309n_reg_set(6, 18, 15, regValue);
    /*Write Command*/
    rtl8309n_regbit_set(6, 18, 12, 15, 0);
    rtl8309n_regbit_set(6, 18, 11, 15, 1);
    
    /*Waiting for write command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8309N_IDLE_TIMEOUT; pollcnt ++) 
    {
        rtl8309n_regbit_get(6, 18, 11, 15, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt == RTL8309N_IDLE_TIMEOUT)
        return FAILED;

    /*Disable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 0);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_ucastEntry_get
 * Description:
 *      read an unicast entry from L2 table
 * Input:
 *      entryaddr       -  Specify the entry address to be read (0 ~ 2047)
 * Output:
 *      pL2Data         -  pointer point to the struct describes the unicast entry data
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out 
 *          RTL8309N_LUT_AGE100     -   100s
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 */ 
int32 rtl8309n_l2_ucastEntry_get(uint32 entryAddr, rtl8309n_l2_ucastEntry_t *pL2_data)
{
    //uint32 entryAddrHd;
    uint32 regValue, pollcnt;
    uint32 bitValue;

	if(NULL == pL2_data)
      	return FAILED;
		
    /*Enable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 1);
	/*entry address*/
    regValue = entryAddr & 0x7FF;
    rtl8309n_reg_set(6, 18, 15, regValue);
	/*Read Command*/
	rtl8309n_regbit_set(6, 18, 12, 15, 1);
    rtl8309n_regbit_set(6, 18, 11, 15, 1);

    /*Waiting for Read command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt <RTL8309N_IDLE_TIMEOUT; pollcnt++) 
    {
        rtl8309n_regbit_get(6, 18, 11, 15, &bitValue);
        if (!bitValue)
            break;
    }
	
	if (pollcnt == RTL8309N_IDLE_TIMEOUT)
		return FAILED;

    /*Read Data[63:48]*/
    rtl8309n_reg_get(6, 19, 15, &regValue);
    pL2_data->auth = ((regValue >> 11) & 0x1) ? TRUE: FALSE;
	pL2_data->da_block = ((regValue >> 10) & 0x1) ? TRUE : FALSE;
	pL2_data->sa_block = ((regValue >> 9) & 0x1) ? TRUE : FALSE;
    pL2_data->isStatic = ((regValue >> 8) & 0x1) ? TRUE : FALSE;
	switch ((regValue >> 6) & 0x3)
	{
		case 0:
			pL2_data->age = RTL8309N_LUT_AGEOUT;
			break;
		case 1:
			pL2_data->age = RTL8309N_LUT_AGE100;
			break;
		case 2:
			pL2_data->age = RTL8309N_LUT_AGE300;
			break;
		case 3:
			pL2_data->age= RTL8309N_LUT_AGE200;
			break;
		default:
			break;
	}
    pL2_data->port= (regValue >> 2) & 0xF;
	pL2_data->fid = regValue & 0x3;
	/*Read data[47:32]*/
	rtl8309n_reg_get(6, 20, 15, &regValue);
	pL2_data->mac.octet[5] = (regValue >> 8) & 0xFF;
	pL2_data->mac.octet[4] = regValue & 0xFF;
	/*Read data[31:16]*/
	rtl8309n_reg_get(6, 21, 15, &regValue);
	pL2_data->mac.octet[3] = (regValue >> 8) & 0xFF;
	pL2_data->mac.octet[2] = regValue & 0xFF;
	/*Read data[15:0]*/
	rtl8309n_reg_get(6, 22, 15, &regValue);
	pL2_data->mac.octet[1] = (regValue >> 8) & 0xFF;
	pL2_data->mac.octet[0] = regValue & 0xFF;

    /*Disable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 0);
	//rtlglue_printf("get entry success, entryAddr = %d\n", entryAddr);
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_mcastEntry_set
 * Description:
 *      write an multicast mac address into L2 table
 * Input:
 *      entryAddr  -  L2 table entry address
 *      pL2_data   -  pointer point to the multicast entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The auth for the look up table could be:
 *          RTL8309N_LUT_AUTH       -   authorizated
 *          RTL8309N_LUT_UNAUTH     -   unauthorizated
 */ 
int32 rtl8309n_l2_mcastEntry_set(uint32 entryAddr, rtl8309n_l2_mcastEntry_t *pL2_data)
{
    uint32 regValue, pollcnt;
    uint32 bitValue,auth;

    if ((NULL == pL2_data) || (pL2_data->port_mask > 0x1FF ))
        return FAILED;

    /*For Muticast entry, MAC[47] is 1  */
    if (!(pL2_data->mac.octet[0] & 0x1))
        return FAILED;


	
    /*Enable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 1);
    /*Write Data[55:48]*/
	auth = (RTL8309N_LUT_AUTH == pL2_data->auth) ? 1: 0;
    regValue = (auth << 11) | ((pL2_data->port_mask & 0x1ff) << 2) | (pL2_data->fid & 0x3); 
    rtl8309n_reg_set(6, 19, 15, regValue & 0xFFFF);
    /*write Data[47:32]*/
    rtl8309n_reg_set(6, 20, 15, pL2_data->mac.octet[5] << 8 | pL2_data->mac.octet[4]);
    /*wrtie Data[31:16]*/
    rtl8309n_reg_set(6, 21, 15, pL2_data->mac.octet[3] << 8 | pL2_data->mac.octet[2]);
    /*wrtie Data[15:0]*/
    rtl8309n_reg_set(6, 22, 15, pL2_data->mac.octet[1] << 8 | pL2_data->mac.octet[0]);
	/*write entry address*/
	regValue = entryAddr & 0x7FF;
	rtl8309n_reg_set(6, 18, 15, regValue);
    /*Write Command*/
    rtl8309n_regbit_set(6, 18, 12, 15, 0);
    rtl8309n_regbit_set(6, 18, 11, 15, 1);
    /*Waiting for write command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8309N_IDLE_TIMEOUT; pollcnt++)
    {
    	
        rtl8309n_regbit_get(6, 18, 11, 15, &bitValue);
        if (!bitValue)
            break;
    }

    if (pollcnt >= RTL8309N_IDLE_TIMEOUT)
        return FAILED;
    
    /*Disable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 0);

    /*record it in software cache if define RTL8309N_LUT_CACHE*/
 
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_mcastEntry_get
 * Description:
 *      Get LUT multicast entry
 * Input:
 *      entryAddr         -  Specify the LUT entry address(0~2047)
 * Output:
 *      pL2_data          -  pointer point to the struct describing the multicast entry 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The auth for the look up table could be:
 *          RTL8309N_LUT_AUTH       -   authorizated
 *          RTL8309N_LUT_UNAUTH     -   unauthorizated
 */  
int32 rtl8309n_l2_mcastEntry_get(uint32 entryAddr, rtl8309n_l2_mcastEntry_t *pL2_data)
{
    uint32 regValue, pollcnt;
    uint32 bitValue;

    if (NULL == pL2_data)
        return FAILED;

    /*Enable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 1);
	/*entry address*/
    regValue = entryAddr & 0x7FF;
    rtl8309n_reg_set(6, 18, 15, regValue);
	/*Read Command*/
	rtl8309n_regbit_set(6, 18, 12, 15, 1);
    rtl8309n_regbit_set(6, 18, 11, 15, 1);
    
    /*Waiting for Read command done and prevent polling dead loop*/
    for (pollcnt = 0; pollcnt < RTL8309N_IDLE_TIMEOUT; pollcnt++) 
    {
        rtl8309n_regbit_get(6, 18, 11, 15, &bitValue);
        if (!bitValue)
            break;
    }
    if (pollcnt >= RTL8309N_IDLE_TIMEOUT)
        return FAILED;
    
    /*Read Data[63:48]*/
    rtl8309n_reg_get(6, 19, 15, &regValue);
    pL2_data->auth = ((regValue >> 11) & 0x1) ? RTL8309N_LUT_AUTH : RTL8309N_LUT_UNAUTH;
	pL2_data->port_mask = (regValue >> 2) & 0x1FF;
	pL2_data->fid = regValue & 0x3;
	/*Read data[47:32]*/
	rtl8309n_reg_get(6, 20, 15, &regValue);
	pL2_data->mac.octet[5] = (regValue >> 8)& 0xFF;
	pL2_data->mac.octet[4] = regValue & 0xFF;
	/*Read data[31:16]*/
	rtl8309n_reg_get(6, 21, 15, &regValue);
	pL2_data->mac.octet[3] = (regValue >> 8)& 0xFF;
	pL2_data->mac.octet[2] = regValue & 0xFF;
	/*Read data[15:0]*/
	rtl8309n_reg_get(6, 22, 15, &regValue);
	pL2_data->mac.octet[1] = (regValue >> 8)& 0xFF;
	pL2_data->mac.octet[0] = regValue & 0xFF;

    /*Disable lookup table access*/
    rtl8309n_regbit_set(6, 16, 0, 15, 0);

	return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_ucastAddr_add
 * Description:
 *     Add an unicast mac address, software will detect empty entry
 *Description:
 *		add a unicast entry into l2 table
 *Input:
 *      pMac    -   pointer point to MAC address
 *      fid     -   VLAN IVL ID
 *		pL2_data		-  pointer point to the struct describing the unicast entry data
 *Output:
 *		pEntryAddr   -  pointer point to the entry address
 *Return:
 *		SUCCESS
 *      FAILED
 *      RTL8309N_LUT_FULL
 * Note:
 *      (1)Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out
 *          RTL8309N_LUT_AGE100     -   100s 
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 *      (2)The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *          otherwise the function will find an empty entry to put it.
 *          When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *          If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *          entry address of the Mac address stored.
 *          If all the four entries can not be replaced, it will return a  RTL8309N_LUT_FULL error, you can delete one of them 
 *          and rewrite the unicast address.  
 */ 
int32 rtl8309n_l2_ucastAddr_add(uint8 *pMac, uint32 fid, rtl8309n_l2_ucastEntry_t* pL2_data, uint32* pEntryaddr)
{
	rtl8309n_l2_ucastEntry_t tmp_l2_data;
	uint32 index, entryAddr;
	int32 i;
	uint32 isFull;	
	
	
	if((NULL == pMac) || (NULL == pEntryaddr) || (NULL == pL2_data))
		return FAILED;
    if (fid > 3)
        return FAILED;
	if((pMac[0] & 0x01) || (pL2_data->mac.octet[0] & 0x01))
		return FAILED;

    rtl8309n_l2_MacToIdx_get(pMac, fid, &index);
	isFull = TRUE;
    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
        if (rtl8309n_l2_ucastEntry_get(entryAddr, &tmp_l2_data) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((pMac[0] == tmp_l2_data.mac.octet[0]) && (pMac[1] == tmp_l2_data.mac.octet[1]) && 
                   (pMac[2] == tmp_l2_data.mac.octet[2]) && (pMac[3] == tmp_l2_data.mac.octet[3]) &&
                    (pMac[4] == tmp_l2_data.mac.octet[4]) && (pMac[5] == tmp_l2_data.mac.octet[5])) 
        {
			rtl8309n_l2_ucastEntry_set(entryAddr, pL2_data);
			*pEntryaddr = entryAddr;
			isFull = FALSE;
            
            return SUCCESS;
        }
    }

    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
		
        if (rtl8309n_l2_ucastEntry_get(entryAddr, &tmp_l2_data) != SUCCESS) 
        {
            return FAILED;
        }
        else if (((tmp_l2_data.mac.octet[0] & 0x1) == 0) && (!tmp_l2_data.auth) && (!tmp_l2_data.isStatic))  
        {
        
            rtl8309n_l2_ucastEntry_set(entryAddr, pL2_data);
			*pEntryaddr = entryAddr;
			isFull = FALSE;
			
            return SUCCESS;
        }
    }
    /* four way are all full, return RTL8309N_LUT_FULL*/
    if (isFull) 
    {
        *pEntryaddr = RTL8309N_LUT_NOADDR;
        return RTL8309N_LUT_FULL;
    }
	
    return SUCCESS;

}


/* Function Name:
 *      rtl8309n_l2_ucastAddr_get
 * Description:
 *      Get an unicast mac address information from l2 table
 * Input:
 *      pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 * Output:
 *      pL2_data        -  pointer point to the struct describing the unicast entry
 *      pEntryAddr      -  pointer point to the unicast entry address
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 * Note:
 *      (1)Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out
 *          RTL8309N_LUT_AGE100     -   100s 
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 *      (2)The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *          otherwise the function will find an empty entry to put it.
 *          When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *          If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *          entry address of the Mac address stored.
 *          If all the four entries can not be replaced, it will return a  RTL8309N_LUT_FULL error, you can delete one of them 
 *          and rewrite the unicast address.  
 */ 
int32 rtl8309n_l2_ucastAddr_get(uint8 *pMac, uint32 fid, rtl8309n_l2_ucastEntry_t *pL2_data, uint32 *pEntryAddr)
{
    int32  i;
	uint32 index, entryAddr;
    uint32 isHit;
	rtl8309n_l2_ucastEntry_t tmp_l2_data;

	
	if((NULL == pMac) || (pMac[0] & 0x01))
		return FAILED;
    if(fid > 3)
        return FAILED;
	if((NULL == pEntryAddr) || (NULL == pL2_data))
		return FAILED;
	
    isHit = FALSE;
	*pEntryAddr = 0;
    rtl8309n_l2_MacToIdx_get(pMac, fid, &index);
    /*scanning sequence is from entry 3 to entry 0, because priority
     *of four way is entry 3 > entry 2 > entry 1 > entry 0
    */
    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
        if (rtl8309n_l2_ucastEntry_get(entryAddr, &tmp_l2_data) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((pMac[0] == tmp_l2_data.mac.octet[0]) && (pMac[1] == tmp_l2_data.mac.octet[1]) && 
                   (pMac[2] == tmp_l2_data.mac.octet[2]) && (pMac[3] == tmp_l2_data.mac.octet[3]) &&
                    (pMac[4] == tmp_l2_data.mac.octet[4]) && (pMac[5] == tmp_l2_data.mac.octet[5])) 
        {
            if ((RTL8309N_LUT_AGEOUT == tmp_l2_data.age) && (!tmp_l2_data.isStatic) && (!tmp_l2_data.auth))
                    return  RTL8309N_LUT_NOTEXIST; 
			pL2_data->auth = tmp_l2_data.auth;
			pL2_data->da_block = tmp_l2_data.da_block;
			pL2_data->sa_block = tmp_l2_data.sa_block;
			pL2_data->isStatic = tmp_l2_data.isStatic;
			pL2_data->age = tmp_l2_data.age;
			pL2_data->port = tmp_l2_data.port;
            pL2_data->fid = tmp_l2_data.fid;
            pL2_data->mac.octet[0] = tmp_l2_data.mac.octet[0];
            pL2_data->mac.octet[1] = tmp_l2_data.mac.octet[1];
            pL2_data->mac.octet[2] = tmp_l2_data.mac.octet[2];
            pL2_data->mac.octet[3] = tmp_l2_data.mac.octet[3];
            pL2_data->mac.octet[4] = tmp_l2_data.mac.octet[4];
            pL2_data->mac.octet[5] = tmp_l2_data.mac.octet[5];
			*pEntryAddr = entryAddr;
			isHit = TRUE;
            return SUCCESS;
        }
    }
	
    if(!isHit)
    {
        *pEntryAddr = RTL8309N_LUT_NOADDR;
        
        return RTL8309N_LUT_NOTEXIST;
    }
	
    return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_ucastAddr_del
 * Description:
 *      Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 * Output:
 *      pEntryAddr      -  pointer point to the entry address
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 * Note:
 *      Use this function to delete a unicastMac address, it does not require to specify the 
 *      entry address. MAC and fid should be given as input parameter, which are needed to caculate 
 *      entery address for the entry to be deleted.if the Mac has existed in the LUT, it will be 
 *      deleted and function return value is SUCCESS.If the Mac is not existed in the LUT, the return 
 *      value is RTL8309N_LUT_NOTEXIST.
 */ 
int32 rtl8309n_l2_ucastAddr_del(uint8 *pMac, uint32 fid, uint32 *pEntryAddr)
{
	rtl8309n_l2_ucastEntry_t tmp_l2_data;
	uint32 index, entryAddr;
	uint32 isHit;
	int32 i;

	
	if((NULL == pMac) || (NULL == pEntryAddr))
		return FAILED;
	if (pMac[0] & 0x01)
		return FAILED;
    if (fid > 3)
        return FAILED;
    
	isHit = FALSE;
	rtl8309n_l2_MacToIdx_get(pMac, fid, &index);
	for(i = 3; i >= 0; i--)
	{
        entryAddr = ((uint32)i << 9) | index;
        if (rtl8309n_l2_ucastEntry_get(entryAddr, &tmp_l2_data) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((pMac[0] == tmp_l2_data.mac.octet[0]) && (pMac[1] == tmp_l2_data.mac.octet[1]) && 
                   (pMac[2] == tmp_l2_data.mac.octet[2]) && (pMac[3] == tmp_l2_data.mac.octet[3]) &&
                    (pMac[4] == tmp_l2_data.mac.octet[4]) && (pMac[5] == tmp_l2_data.mac.octet[5])) 
        {
            /*delet unicast entry attributes ,exclude mac address and fid*/
        	tmp_l2_data.auth = FALSE;
			tmp_l2_data.da_block = 0;
			tmp_l2_data.sa_block = 0;
			tmp_l2_data.isStatic = FALSE;
			tmp_l2_data.age = 0;
			tmp_l2_data.port = 0;
			tmp_l2_data.mac.octet[0] = 0;
			tmp_l2_data.mac.octet[1] = 0;
			tmp_l2_data.mac.octet[2] = 0;
			tmp_l2_data.mac.octet[3] = 0;
			tmp_l2_data.mac.octet[4] = 0;
			tmp_l2_data.mac.octet[5] = 0;
			tmp_l2_data.fid = 0;
        	rtl8309n_l2_ucastEntry_set(entryAddr, &tmp_l2_data);
			*pEntryAddr = entryAddr;			
			isHit = TRUE;

        }
	}

    if(!isHit)
    {
        *pEntryAddr = RTL8309N_LUT_NOADDR;
        
        return RTL8309N_LUT_NOTEXIST;
    }

	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_mcastAddr_add
 *Description:
 *		add a multicast entry into l2 table
 *Input:
 *		pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 *      portmask    -   port mask of multicast address entry
 *Output:
 *		pEntryAddr    -  pointer point to the entry address
 *Return:
 *		SUCCESS
 *      FAILED
 *      RTL8309N_LUT_FULL
 *Note:
 *      (1)RTL8309N use mac and fid in hash alogrim to calculate 4 way index. After index is got, mac address
 *          within the 4 way entries can be capared with the input mac address one by one from entry 3 to entry 0. 
 *      (2)If the input mac address is already in the table, the corresponded entry will be updated. 
 *         If the input mac address is not in the table, a dynamic and unauthorised unicast entry will be found out
 *         and replaced by the input mac address entry.
 *      (3)The multicast address wrote by CPU is always authorizated.
 *      (4)If no dynamic and unauthorised unicast entry is found out, it means that the 4 way entries indexed by the
 *         index calculated are full. So RTL8309N_LUT_FULL will be returned.
 */
int32 rtl8309n_l2_mcastAddr_add(uint8 *pMac, uint32 fid, uint32 portmask, uint32 *pEntryaddr)
{
	rtl8309n_l2_mcastEntry_t tmp_multicastdata;
	rtl8309n_l2_ucastEntry_t tmp_unicastdata;
    int32 i;
	uint32 isFull;
    uint32 index,entryAddr;

	if((NULL == pMac) || (NULL == pEntryaddr))
		return FAILED;
	if ((pMac[0] & 0x01) == 0)
		return FAILED;
	if ((fid > 3) || (portmask > RTL8309N_MAX_PORTMASK))
        return FAILED;
	
    rtl8309n_l2_MacToIdx_get(pMac, fid, &index);
	isFull = TRUE;
    /*
      *First scan four-ways, if the multicast entry has existed, only update the entry, that could 
      *prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
      *four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
      *replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because priority
      *of four way is entry 3 > entry 2 > entry 1 > entry 0
      */
    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
        if (rtl8309n_l2_mcastEntry_get(entryAddr, &tmp_multicastdata) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((pMac[0] == tmp_multicastdata.mac.octet[0]) && (pMac[1] == tmp_multicastdata.mac.octet[1]) && 
                   (pMac[2] == tmp_multicastdata.mac.octet[2]) && (pMac[3] == tmp_multicastdata.mac.octet[3]) &&
                    (pMac[4] == tmp_multicastdata.mac.octet[4]) && (pMac[5] == tmp_multicastdata.mac.octet[5])) 
        {
            tmp_multicastdata.auth = RTL8309N_LUT_AUTH;
            tmp_multicastdata.port_mask = portmask;
			rtl8309n_l2_mcastEntry_set(entryAddr, &tmp_multicastdata);
			
			*pEntryaddr = entryAddr;
			isFull = FALSE;
			
            return SUCCESS;
        }
    }

    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
		if (rtl8309n_l2_ucastEntry_get(entryAddr, &tmp_unicastdata) != SUCCESS) 
		{
			return FAILED;
		}
        if (((tmp_unicastdata.mac.octet[0] & 0x1) == 0) && (!tmp_unicastdata.auth) && (!tmp_unicastdata.isStatic)) 
        {
        	tmp_multicastdata.auth = RTL8309N_LUT_AUTH;
			tmp_multicastdata.port_mask = portmask;
			tmp_multicastdata.fid = fid;
			tmp_multicastdata.mac.octet[0] = pMac[0];
			tmp_multicastdata.mac.octet[1] = pMac[1];	
			tmp_multicastdata.mac.octet[2] = pMac[2];
			tmp_multicastdata.mac.octet[3] = pMac[3];	
			tmp_multicastdata.mac.octet[4] = pMac[4];
			tmp_multicastdata.mac.octet[5] = pMac[5];	
            rtl8309n_l2_mcastEntry_set(entryAddr, &tmp_multicastdata);
			*pEntryaddr = entryAddr;
            isFull = FALSE;
            break;
        }
    }

    /*If four way are all full, return RTL8309N_LUT_FULL*/
    if (isFull) 
    {
        *pEntryaddr = RTL8309N_LUT_NOADDR;
        
        return RTL8309N_LUT_NOTEXIST;
    }

    return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_multicastMac_get
 * Description:
 *      Get an multicast mac address information from l2 table
 * Input:
 *      pMac	-	pointer point to mac address
 *		fid		-	VLAN IVL ID
 * Output:
 *		pPortMask		-	port membership of a multicast address
 *      pEntryAddr      -   the pointer point to the multicast entry address                             
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 *Note:
 *      (1)RTL8309N use mac and fid in hash alogrim to calculate 4 way index. After index is got, mac address
 *          within the 4 way entries can be capared with the input mac address one by one from entry 3 to entry 0. 
 *      (2)If the input mac address is already in the table, the corresponded entry will be updated. 
 *         If the input mac address is not in the table, a dynamic and unauthorised unicast entry will be found out
 *         and replaced by the input mac address entry.
 *      (3)The multicast address wrote by CPU is always authorizated.
 *      (4)If no dynamic and unauthorised unicast entry is found out, it means that the 4 way entries indexed by the
 *         index calculated are full. So RTL8309N_LUT_FULL will be returned.
 */ 
int32 rtl8309n_l2_mcastAddr_get(uint8* pMac, uint32 fid, uint32 *pPortmask, uint32 *pEntryAddr)
{
    int32  i;
	uint32 index, entryAddr;
    uint32 isHit;
	rtl8309n_l2_mcastEntry_t tmp_multicastdata;

	if((NULL == pMac) || (NULL == pPortmask) || (NULL == pEntryAddr))
		return FAILED;
	if(!(pMac[0] & 0x01))
		return FAILED;
	if(fid > 3)
		return FAILED;

	
    isHit = FALSE;
    rtl8309n_l2_MacToIdx_get(pMac, fid, &index);

    /*scanning sequence is from entry 3 to entry 0, because priority
     *of four way is entry 3 > entry 2 > entry 1 > entry 0
    */
    for (i = 3; i >= 0; i--) 
    {
        entryAddr = ((uint32)i << 9) | index;
        if (rtl8309n_l2_mcastEntry_get(entryAddr, &tmp_multicastdata) != SUCCESS) 
        {
            return FAILED;
        }
        else if ((pMac[0] == tmp_multicastdata.mac.octet[0]) && (pMac[1] == tmp_multicastdata.mac.octet[1]) && 
                   (pMac[2] == tmp_multicastdata.mac.octet[2]) && (pMac[3] == tmp_multicastdata.mac.octet[3]) &&
                    (pMac[4] == tmp_multicastdata.mac.octet[4]) && (pMac[5] == tmp_multicastdata.mac.octet[5])) 
        {		
			*pPortmask = tmp_multicastdata.port_mask;
			*pEntryAddr = entryAddr;
			isHit = TRUE;
            return SUCCESS;
        }
    }

    if(!isHit)
    {
        *pEntryAddr = RTL8309N_LUT_NOADDR;
        
        return RTL8309N_LUT_NOTEXIST;
    }
    
    return SUCCESS;


}

/* Function Name:
 *      rtl8309n_l2_multicastMac_del
 * Description:
 *      Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      pMac	-	pointer point to mac address
 *		fid		-	VLAN IVL ID
 * Output:
 *		pEntryAddr	-	pointer point to the entry address of the deleted multicast entry 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function to delete a multicastMac address, it does not require to specify the 
 *      entry address, if the Mac has existed in the LUT, it will be deleted and function
 *      return value is SUCCESS.If the Mac is not existed in the LUT, the return value is FAILED.
 */
int32 rtl8309n_l2_mcastAddr_del(uint8 *pMac, uint32 fid, uint32 *pEntryAddr)
{
	int32 retVal;
	uint32 index, entryAddr;
	uint32 isHit;
	int32 i;
	rtl8309n_l2_mcastEntry_t tmp_multicastData;
	rtl8309n_l2_ucastEntry_t tmp_l2_data;

	if((NULL == pMac) || (NULL == pEntryAddr))
		return FAILED;
	if (!(pMac[0] & 0x01))
		return FAILED;
	if (fid > 3)
		return FAILED;

	
	isHit = FALSE;
	rtl8309n_l2_MacToIdx_get(pMac, fid, &index);
	for(i = 0; i < 4; i++)
	{
        entryAddr = ((uint32)i << 9) | index;
		retVal = rtl8309n_l2_mcastEntry_get(entryAddr, &tmp_multicastData);
		
		if(SUCCESS != retVal)
		{
			return FAILED;
		}
        else if ((pMac[0] == tmp_multicastData.mac.octet[0]) && (pMac[1] == tmp_multicastData.mac.octet[1]) && 
                   (pMac[2] == tmp_multicastData.mac.octet[2]) && (pMac[3] == tmp_multicastData.mac.octet[3]) &&
                    (pMac[4] == tmp_multicastData.mac.octet[4]) && (pMac[5] == tmp_multicastData.mac.octet[5])) 
        {
			/*change this multicast entry to a unvalid unicast entry*/
        	tmp_l2_data.auth = FALSE;
			tmp_l2_data.da_block = 0;
			tmp_l2_data.sa_block = 0;
			tmp_l2_data.isStatic = FALSE;
			tmp_l2_data.age = 0;
			tmp_l2_data.port = 0;
			tmp_l2_data.mac.octet[0] = 0;
			tmp_l2_data.mac.octet[1] = 0;
			tmp_l2_data.mac.octet[2] = 0;
			tmp_l2_data.mac.octet[3] = 0;
			tmp_l2_data.mac.octet[4] = 0;
			tmp_l2_data.mac.octet[5] = 0;
			tmp_l2_data.fid = 0;
			
        	retVal = rtl8309n_l2_ucastEntry_set(entryAddr, &tmp_l2_data);						
			if (SUCCESS != retVal)
				return FAILED;
			*pEntryAddr = entryAddr;			
			isHit = TRUE;
        }
	}
    
    if(!isHit)
    {
        *pEntryAddr = RTL8309N_LUT_NOADDR;
        
        return RTL8309N_LUT_NOTEXIST;
    }


	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_portMacLimitEnable_set
 *Description:
 *		Enable port mac learning limit function
 *Input:
 *		port	-	port number
 *		enabled	-	enable or disable
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *
 */
int32 rtl8309n_l2_portMacLimitEnable_set(uint32 port, uint32 enabled)
{    
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
	rtl8309n_regbit_set(port, 16, 1, 5, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_portMacLimitEnable_get
 *Description:
 *		Get status of port mac learning limit function
 *Input:
 *		port	-	port number
 *Output:
 *		pEnabled	-	pointer point to enable or disable
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *
 */
int32 rtl8309n_l2_portMacLimitEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if((port > RTL8309N_MAX_PORT_ID) || (NULL == pEnabled))
		return FAILED;
	rtl8309n_regbit_get(port, 16, 1, 5, &bitVal);
    *pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}


/* Function Name:
 *      rtl8309n_l2_portMacLimit_set
 * Description:
 *      Set per port mac limit counter max value
 * Input:
 *      port         -  port number (0 ~ 8)  
 *      maxVal    - auto learning MAC limit number
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8309n_l2_portMacLimit_set(uint32 port, uint32 maxVal) 
{
    uint32 regValue;

    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
	if (maxVal > 0x1F)
		return FAILED;
	
    rtl8309n_reg_get(port, 16, 5, &regValue);
    regValue &= ~(0x1F << 11);
    regValue |= (maxVal << 11);
    rtl8309n_reg_set(port, 16, 5, regValue);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_portMacLimit_get
 * Description:
 *      Get per port mac limit counter max value
 * Input:
 *      port         -  port number (0 ~ 8)  
 * Output:
 *      pMacCnt     -  auto learning MAC limit number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8309n_l2_portMacLimit_get(uint32 port, uint32 *pMacCnt) 
{
    uint32 regValue;


    if (port > RTL8309N_MAX_PORT_ID || NULL == pMacCnt)
        return FAILED;

    rtl8309n_reg_get(port, 16, 5, &regValue);
    *pMacCnt = (regValue >> 11) & 0x1F;
 
    return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_systemMacLimitEnable_set
 *Description:
 *		Enable system mac learning limit function
 *Input:
 *		enabled		-	enable or disable
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 */
int32 rtl8309n_l2_systemMacLimitEnable_set(uint32 enabled)
{
	/*enable system mac limit function*/
    rtl8309n_regbit_set(6, 23, 8, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_l2_systemMacLimitEnable_set
 *Description:
 *		Get the status of system mac learning limit function
 *Input:
 *		none
 *Output:
 *		pEnabled		-	pointer point to enable or disable
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 */
int32 rtl8309n_l2_systemMacLimitEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	if (NULL == pEnabled)
		return FAILED;
    
    rtl8309n_regbit_get(6, 23, 8, 15, &bitVal);
    *pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_systemMacLimit_set
 * Description:
 *      Set the system mac limit counter max value
 * Input:
 *      maxVal    - system auto learning MAC limit number
 *      mergMask  -  port mask for the ports merged to system mac limit
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Besides per port mac limit function, 8306E also supports system MAC limit function.
 *      mergMask is to decide whitch ports are limited by system MAC limit function.
 *      For example, when system mac limit is enabled, and mergMask is 0x15(0b010101),
 *      that means the auto learning MAC number of port 0, port 2 and port 4 will also be
 *      influenced by system MAC limit.    
 */ 
int32 rtl8309n_l2_systemMacLimit_set(uint32 maxVal, uint32 mergMask) 
{
    uint32 regValue;

	if ((maxVal > 0xFF) || (mergMask > 0x1FF))
		return FAILED;

    /*system mac limit max value*/
    rtl8309n_reg_get(6, 23, 15, &regValue);
    regValue &= ~(0xFF);
    regValue |= (maxVal & 0xFF) ;
    rtl8309n_reg_set(6, 23, 15, regValue);
    /*system merge mask*/
    rtl8309n_reg_get(6, 17, 15, &regValue);
    regValue &= ~0x1FF;
    regValue |= mergMask & 0x1FF;
    rtl8309n_reg_set(6, 17, 15, regValue);
 
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_systemMacLimit_get
 * Description:
 *      Get the system mac limit counter value
 * Input:
 *      none
 * Output:
 *      pMacCnt    -   system auto learning MAC limit number
 *      pMergMask  -  port mask for the ports merged to system mac limit
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8309n_l2_systemMacLimit_get(uint32 *pMacCnt, uint32 *pMergMask) 
{
    uint32 regValue;


	if((NULL == pMacCnt) || (NULL == pMergMask))
		return FAILED;
    
    rtl8309n_reg_get(6, 23, 15, &regValue);
    *pMacCnt = (regValue & 0xFF);
    
    rtl8309n_reg_get(6, 17, 15, &regValue);
    *pMergMask = regValue & 0x1FF;
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_macLimitAction_set
 * Description:
 *      Set the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      action      -  the action taken when auto learning MAC reach to the max value 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      This API can be called to set action when mac limit counter exceed the max value
 *          RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *          RTL8309N_ACT_DROP       -   drop
 */ 
int32 rtl8309n_l2_macLimitAction_set(uint32 action) 
{
    if (RTL8309N_ACT_DROP != action && RTL8309N_ACT_TRAP2CPU != action)
        return FAILED;
    
    if (RTL8309N_ACT_DROP == action)
        rtl8309n_regbit_set(6, 17, 9, 15, 0);
    else if (RTL8309N_ACT_TRAP2CPU == action)
        rtl8309n_regbit_set(6, 17, 9, 15, 1);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_macLimitAction_get
 * Description:
 *      Get the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      pAction      -  the action taken when auto learning MAC reach to the max value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      This API can be called to get action when mac limit counter exceed the max value
 *          RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *          RTL8309N_ACT_DROP       -   drop
 */ 
int32 rtl8309n_l2_macLimitAction_get(uint32 *pAction) 
{
    uint32 bitValue;

    if (NULL == pAction)
        return FAILED;
    
    rtl8309n_regbit_get(6, 17, 9, 15, &bitValue);
    *pAction = bitValue ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_systemLearningCnt_get
 * Description:
 *      Get the current value of  system mac auto learning counter
 * Input:
 *      none
 * Output:
 *      pMacCnt		-	pointer point to value of system mac auto learning counter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8309n_l2_systemLearningCnt_get(uint32 *pMacCnt)
{
	uint32 regVal;
	
	if(NULL == pMacCnt)
		return FAILED;

	rtl8309n_reg_get(6, 24, 15, &regVal);
	*pMacCnt = regVal & 0x7FF;
    
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_l2_systemLearningCnt_get
 * Description:
 *      Get the current value of  port mac auto learning counter
 * Input:
 *      port		-	port id
 * Output:
 *      pMacCnt		-	pointer point to value of port mac auto learning counter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
int32 rtl8309n_l2_portlearningCnt_get(uint32 port, uint32 *pMacCnt)
{
	uint32 regVal;
	
	if(NULL == pMacCnt)
		return FAILED;
	if(port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	
	rtl8309n_reg_get(port, 17, 5, &regVal);
	*pMacCnt = regVal & 0x7FF;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_lruEnable_set
 * Description:
 *		Enable LRU ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_lruEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 4, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_lruEnable_get
 * Description:
 *		Get enabled status of LRU ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_lruEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 4, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_camEnable_set
 * Description:
 *		Enable CAM ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_camEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 3, 15, (RTL8309N_ENABLED == enabled) ? 0 : 1);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_camEnable_get
 * Description:
 *		Get enabled status of CAM ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_camEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 3, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_flushEnable_set
 * Description:
 *		Enable flushing l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_flushEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 6, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_flushEnable_get
 * Description:
 *		Get enabled status of flushing l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_flushEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 6, 15, &bitVal);
	*pEnabled = (bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_agingEnable_set
 * Description:
 *		Enable aging l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_agingEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 11, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;	
}

/* Function Name:
 *		rtl8309n_l2_agingEnable_get
 * Description:
 *		Get enabled status of aging l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_agingEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 11, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_fastAgingEnable_set
 * Description:
 *		Enable fast aging l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_fastAgingEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 5, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;	
}

/* Function Name:
 *		rtl8309n_l2_fastAgingEnable_get
 * Description:
 *		Get enabled status of fast aging l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_fastAgingEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 5, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_l2_dpMskEnable_set
 * Description:
 *		Enable DP mask ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		When DP mask ability is enabled, one frame rx from port n can't tx from port n.
 */
int32 rtl8309n_l2_dpMskEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 12, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;	

}

/* Function Name:
 *		rtl8309n_l2_dpMskEnable_get
 * Description:
 *		Get enabled status of DP mark ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_l2_dpMskEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 12, 15, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_l2_unkownDaDropEnable_set
 * Description:
 *		Enable drop unkown Da unicast packets
 * Input:
 *		enabled		-	Enable or Disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_l2_unkownDaDropEnable_set(uint32 enabled)
{
	rtl8309n_regbit_set(6, 16, 13, 15, (RTL8309N_ENABLED == enabled) ? 0 : 1);
    
	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_l2_unkownDaDropEnable_get
 * Description:
 *		Get enabled status of dropping unkown Da unicast packets
 * Input:
 *		none
 * Output:
 *		pEnabled		-	Enable or Disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_l2_unkownDaDropEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(6, 16, 13, 15, &bitVal);
	*pEnabled = (bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED);
    
	return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_sablockAction_set
 * Description:
 *      Set action for packets blocked by sa block module
 * Input:
 *      action -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
int32 rtl8309n_l2_sablockAction_set(uint32 action)
{
    uint32 bitVal;
    
    if (RTL8309N_ACT_DROP != action && RTL8309N_ACT_TRAP2CPU != action)
        return FAILED;
    
    bitVal = (RTL8309N_ACT_DROP == action) ? 1 : 0;
    rtl8309n_regbit_set(6, 16, 8, 15, bitVal);
    
    return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_sablockAction_get
 * Description:
 *      Get action for packets blocked by sa block module
 * Input:
 *      none
 * Output:
 *      pAction - pointer pointed to action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
int32 rtl8309n_l2_sablockAction_get(uint32 *pAction)
{
    uint32 bitVal;

    if (NULL == pAction)
        return FAILED;

    rtl8309n_regbit_get(6, 16, 8, 15, &bitVal);
    *pAction = bitVal ? RTL8309N_ACT_DROP : RTL8309N_ACT_TRAP2CPU;

    return SUCCESS;

}

/* Function Name:
 *      rtl8309n_l2_dablockAction_set
 * Description:
 *      Set action for packets blocked by da block module
 * Input:
 *      action -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
int32 rtl8309n_l2_dablockAction_set(uint32 action)
{
    uint32 bitVal;
    
    if (RTL8309N_ACT_DROP != action && RTL8309N_ACT_TRAP2CPU != action)
        return FAILED;
    
    bitVal = (RTL8309N_ACT_DROP == action) ? 1 : 0;
    rtl8309n_regbit_set(6, 16, 7, 15, bitVal);
    
    return SUCCESS;

}


/* Function Name:
 *      rtl8309n_l2_dablockAction_get
 * Description:
 *      Get action for packets blocked by da block module
 * Input:
 *      none
 * Output:
 *      pAction - pointer pointed to action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
int32 rtl8309n_l2_dablockAction_get(uint32 *pAction)
{
    uint32 bitVal;

    if (NULL == pAction)
        return FAILED;

    rtl8309n_regbit_get(6, 16, 7, 15, &bitVal);
    *pAction = bitVal ? RTL8309N_ACT_DROP : RTL8309N_ACT_TRAP2CPU;

    return SUCCESS;

}


/* Function Name:
 *      rtl8309n_stp_set
 * Description:
 *      Set IEEE 802.1d port state
 * Input:
 *      port   -  Specify port number (0 ~ 8)
 *      state -   Specify port state
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8309N_SPAN_DISABLE    - Disable state
 *         RTL8309N_SPAN_BLOCK      - Blocking state
 *         RTL8309N_SPAN_LEARN      - Learning state
 *         RTL8309N_SPAN_FORWARD    - Forwarding state        
 */ 
int32 rtl8309n_stp_set(uint32 port, uint32 state) 
{
    uint32 regValue;
	rtl8309n_rma_entry_t rmaEntry;
	
    if ((port > RTL8309N_MAX_PORT_ID) || (state > RTL8309N_SPAN_END))
        return FAILED;
	
	/*enable trap BPDU(RMA 00) to cpu*/
	rmaEntry.action = RTL8309N_ACT_TRAP2CPU;
	rmaEntry.enable_rmapri = RTL8309N_DISABLED;
	rmaEntry.priority = 0;
	rtl8309n_rma_entry_set(RTL8309N_RMA00, &rmaEntry);
	/*set port STP state*/
	rtl8309n_reg_get(port, 16, 2, &regValue);
	regValue &= ~0x3;
	regValue |= state;
	rtl8309n_reg_set(port, 16, 2, regValue);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_stp_get
 * Description:
 *      Get IEEE 802.1d port state
 * Input:
 *      port    -  Specify port number (0 ~ 5)
 * Output:
 *      pState -  get port state
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8309N_SPAN_DISABLE   - Disable state
 *         RTL8309N_SPAN_BLOCK      - Blocking state
 *         RTL8309N_SPAN_LEARN      - Learning state
 *         RTL8309N_SPAN_FORWARD - Forwarding state
 */ 
int32 rtl8309n_stp_get(uint32 port, uint32 *pState) 
{
    uint32 regValue;

    if ((port > RTL8309N_PORT8) || (NULL == pState))
        return FAILED;
    
    rtl8309n_reg_get(port, 16, 2, &regValue);
    switch(regValue)
    {
    	case 0:
			*pState = RTL8309N_SPAN_DISABLE;
			break;
		case 1:
			*pState = RTL8309N_SPAN_BLOCK;
			break;
		case 2:
			*pState = RTL8309N_SPAN_LEARN;
			break;
		case 3:
			*pState = RTL8309N_SPAN_FORWARD;
			break;
		default:
			break;
    }
    
    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_dot1x_portBasedEnable_set
 * Description:
 *		Enable DOT1X port based access control
 * Input:
 *		port	-	port number(0~8)
 *		enabled -	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_portBasedEnable_set(uint32 port, uint32 enabled)
{
	if (port > RTL8309N_PORT_NUMBER)
		return FAILED;
	
  	rtl8309n_regbit_set(port, 16, 0, 3, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_dot1x_portBasedEnable_get
 * Description:
 *		Get enabled status of DOT1X port based access control ability
 * Input:
 *		port	-	port number(0~8)
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_portBasedEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if (port > RTL8309N_PORT_NUMBER)
		return FAILED;
	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(port, 16, 0, 3, &bitVal);

	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_dot1x_macBasedEnable_set
 * Description:
 *		Enable DOT1X mac based access control
 * Input:
 *		port	-	port number(0~8)
 *		enabled -	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_macBasedEnable_set(uint32 port, uint32 enabled)
{
	if (port > RTL8309N_PORT_NUMBER)
		return FAILED;
	
  	rtl8309n_regbit_set(port, 16, 3, 3, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_dot1x_macBasedEnable_get
 * Description:
 *		Get enabled status of DOT1X mac based access control
 * Input:
 *		port	-	port number(0~8)
 * Output:
 *		pEnabled -	enable or disable
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_macBasedEnable_get(uint32 port, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if (port > RTL8309N_PORT_NUMBER)
		return FAILED;
	if (NULL == pEnabled)
		return FAILED;
	
  	rtl8309n_regbit_get(port, 16, 3, 3, &bitVal);

	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 8)
 *      isAuth      -   Authorized or unauthorized state 
 *      direction   -    set IEEE802.1x port-based control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8309N_PORT_AUTH      - authorized
 *         RTL8309N_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8309N_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8309N_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
int32 rtl8309n_dot1x_portBased_set(uint32 port, uint32 isAuth, uint32 direction)
{

    if (port > RTL8309N_MAX_PORT_ID)
        return FAILED;
      
	
	rtl8309n_regbit_set(port, 16, 1, 3, (isAuth == RTL8309N_PORT_AUTH ? 1:0));
	rtl8309n_regbit_set(port, 16, 2, 3, (direction == RTL8309N_PORT_BOTHDIR ? 0:1));

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 8)
 * Output:
 *      pIsAuth      - the pointer of authorized or unauthorized state 
 *      pDirection   - the pointer of IEEE802.1x port-based control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8309N_PORT_AUTH      - authorized
 *         RTL8309N_PORT_UNAUTH  - unauthorized
 *     There are also two 802.1x port-based control direction:
 *         RTL8309N_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8309N_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
int32 rtl8309n_dot1x_portBased_get(uint32 port, uint32 *pIsAuth, uint32 *pDirection) 
{
    uint32 bitValue;

    if ((port > RTL8309N_MAX_PORT_ID) || (NULL == pIsAuth) || (NULL == pDirection)) 
        return FAILED;
    
    rtl8309n_regbit_get(port, 16, 1, 3, &bitValue);
    *pIsAuth = (bitValue ? RTL8309N_PORT_AUTH : RTL8309N_PORT_UNAUTH);
    rtl8309n_regbit_get(port, 16, 2, 3, &bitValue);
    *pDirection = (bitValue ? RTL8309N_PORT_INDIR : RTL8309N_PORT_BOTHDIR);
    return SUCCESS;
} 

/* Function Name:
 *      rtl8309n_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x mac-based access control
 * Input:
 *      direction    -  IEEE802.1x mac-based access control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)The mac address authentication status is saved in L2 table entry,
 *         it should be set by software.
 *      (2)there are also two mac-based control directions which are not per 
 *         port but global configurtion:
 *              RTL8309N_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will be dropped.
 *              RTL8309N_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will pass mac-based access control igress rule.
 */ 
int32 rtl8309n_dot1x_macBased_set(uint32 direction)
{

    if (direction > RTL8309N_MAC_INDIR)
        return FAILED; 
    
    rtl8309n_regbit_set(0, 16, 1, 15, (RTL8309N_MAC_INDIR == direction) ?  1 : 0);
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_dot1x_macBased_get
 * Description:
 *      Get IEEE802.1x port-based access control ability status
 * Input:
 *		none
 * Output:
 *      pDirection   -  pointer point to IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)The mac address authentication status is saved in L2 table entry,
 *         it should be set by software.
 *      (2)there are also two mac-based control directions which are not per 
 *         port but global configurtion:
 *              RTL8309N_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will be dropped.
 *              RTL8309N_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will pass mac-based access control igress rule.
 */
int32 rtl8309n_dot1x_macBased_get(uint32 *pDirection) 
{
    uint32 bitValue;

    if (NULL == pDirection)
        return FAILED;

	rtl8309n_regbit_get(0, 16, 1, 15, &bitValue);
    *pDirection = (bitValue ? RTL8309N_MAC_INDIR : RTL8309N_MAC_BOTHDIR);
	
    return SUCCESS;
}

/* Funtion Name:
 *      rtl8309n_dot1x_unauthPktAction_set
 * Description:
 *      Set action for dot1x unauthorised packets
 * Input:
 *      action  -   drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_unauthPktAction_set(uint32 action)
{
    uint32 bitVal;
    
    if (RTL8309N_ACT_DROP != action && RTL8309N_ACT_TRAP2CPU != action)
        return FAILED;

    bitVal = (RTL8309N_ACT_TRAP2CPU == action)? 1 : 0;
    rtl8309n_regbit_set(0, 16, 0, 15, bitVal);

    return SUCCESS;
}

/* Funtion Name:
 *      rtl8309n_dot1x_unauthPktAction_get
 * Description:
 *      Set action for dot1x unauthorised packets
 * Input:
 *      none
 * Output:
 *      pAction     -   pointer pointed to action for unauthorised packets
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_dot1x_unauthPktAction_get(uint32 *pAction)
{
    uint32 bitVal;
    
    if (NULL == pAction)
        return FAILED;

    rtl8309n_regbit_get(0, 16, 0, 15, &bitVal);
    *pAction = bitVal ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;
    
    return SUCCESS;    
}

/* Function Name:
 *		rtl8309n_igmp_enable_set
 * Description:
 *		Enable IGMP/MLD snooping and PPPOE bypass ability
 * Input:
 *		type	-	IGMP/MLD, PPPOE
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Enable the ASIC to parse IGMP/MLD or PPPOE packets. The action to take when the packets 
 *		   are received and detected is set by RMA function.
 *      (2)type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 */
int32 rtl8309n_igmp_enable_set(uint32 type, uint32 enabled)
{
	if(type >= RTL8309N_IGMPCTL_END)
		return FAILED;

	switch(type)
	{
		case RTL8309N_IGMP:
			rtl8309n_regbit_set(1, 16, 0, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
			
		case RTL8309N_MLD:
			rtl8309n_regbit_set(1, 16, 1, 15, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;

		case RTL8309N_PPPOE_IPV4:
		case RTL8309N_PPPOE_MLD:
			rtl8309n_regbit_set(1, 16, 2, 15, (RTL8309N_ENABLED == enabled) ? 0 : 1);
			break;

		default:
			break;
	}

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_igmp_enable_get
 * Description:
 *		Get Enabled status of IGMP/MLD snooping and PPPOE bypass ability
 * Input:
 *		type	-	IGMP/MLD, PPPOE
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Enable the ASIC to parse IGMP/MLD or PPPOE packets. The action to take when the packets 
 *		   are received and detected is set by RMA function.
 *      (2)type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 */
int32 rtl8309n_igmp_enable_get(uint32 type, uint32 *pEnabled)
{
	uint32 bitVal;
	
	if(type >= RTL8309N_IGMPCTL_END)
		return FAILED;

	switch(type)
	{
		case RTL8309N_IGMP:
			rtl8309n_regbit_get(1, 16, 0, 15, &bitVal);
			*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
			break;
			
		case RTL8309N_MLD:
			rtl8309n_regbit_get(1, 16, 1, 15, &bitVal);
			*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
			break;

		case RTL8309N_PPPOE_IPV4:
		case RTL8309N_PPPOE_MLD:
			rtl8309n_regbit_get(1, 16, 2, 15, &bitVal);
			*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;
			break;

		default:
			break;
	}

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 *      action       -  Action could be normal forward or trap
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 *     action could be:
 *          RTL8309N_ACT_PERMIT    - normal forward
 *          RTL8309N_ACT_TRAP2CPU  - trap to cpu
 *			RTL8309N_ACT_COPY2CPU  - copy to cpu
 *			RTL8309N_ACT_DROP	   - drop
 */ 
int32 rtl8309n_trap_igmpCtrlPktAction_set(uint32 type, uint32 action)
{
	int32 retVal;
	uint32 rmaType;
	rtl8309n_rma_entry_t igmp_entry;

	if(type >= RTL8309N_IGMPCTL_END)
		return FAILED;
	if(action >= RTL8309N_ACTION_END)
		return FAILED;
	
	igmp_entry.enable_rmapri = RTL8309N_DISABLED;
	igmp_entry.priority = 0;
	igmp_entry.action = action;
	switch(type)
	{
		case RTL8309N_IGMP:
			rmaType = RTL8309N_RMAIGMP;
			break;
			
		case RTL8309N_MLD:
			rmaType = RTL8309N_RMAMLD;
			break;
		case RTL8309N_PPPOE_IPV4:
			rmaType = RTL8309N_RMAPPPOE_IPV4;
			break;
			
		case RTL8309N_PPPOE_MLD:
			rmaType = RTL8309N_RMAPPPOE_MLD;
			break;
			
		default:
			break;
	}
	
	retVal = rtl8309n_rma_entry_set(rmaType, &igmp_entry);
	if(retVal != SUCCESS)
		return FAILED;
	
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function status
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 * Output:
 *      pAction       -  Action could be normal forward or trap
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 *     action could be:
 *          RTL8309N_ACT_PERMIT    - normal forward
 *          RTL8309N_ACT_TRAP2CPU  - trap to cpu
 *			RTL8309N_ACT_COPY2CPU  - copy to cpu
 *			RTL8309N_ACT_DROP	   - drop
 */ 
int32 rtl8309n_trap_igmpCtrlPktAction_get(uint32 type, uint32 *pAction)
{
	uint32 retVal;
	uint32 rmaType;
	rtl8309n_rma_entry_t igmp_entry;

	if(type >= RTL8309N_IGMPCTL_END)
		return FAILED;
	if(NULL == pAction)
		return FAILED;
	
	switch(type)
	{
		case RTL8309N_IGMP:
			rmaType = RTL8309N_RMAIGMP;
			break;
			
		case RTL8309N_MLD:
			rmaType = RTL8309N_RMAMLD;
			break;
		case RTL8309N_PPPOE_IPV4:
			rmaType = RTL8309N_RMAPPPOE_IPV4;
			break;
			
		case RTL8309N_PPPOE_MLD:
			rmaType = RTL8309N_RMAPPPOE_MLD;
			break;
			
		default:
			break;
	}
	
	retVal = rtl8309n_rma_entry_get(rmaType, &igmp_entry);
	if(retVal != SUCCESS)
		return FAILED;
	*pAction = igmp_entry.action;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 *      action       -  drop or normal forward
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_MULCAST_IPV4 - ipv4 unknown multicast
 *          RTL8309N_MULCAST_IPV6 - ipv6 unknown multicast
 *      action could be:
 *          RTL8309N_ACT_DROP      - trap to cpu 
 *          RTL8309N_ACT_PERMIT   - normal forward
 */  
int32 rtl8309n_trap_unknownIPMcastPktAction_set(uint32 type, uint32 action)
{
    uint32 bitVal;
    
	if((type >= RTL8309N_MULCAST_END) || ((action != RTL8309N_ACT_DROP) && (action != RTL8309N_ACT_PERMIT)))
		return FAILED;
	
	switch(type)
	{
		case RTL8309N_MULCAST_IPV4:
            bitVal = (RTL8309N_ACT_DROP == action) ? 1:0;
            rtl8309n_regbit_set(6, 16, 9, 15, bitVal);					
			break;

		case RTL8309N_MULCAST_IPV6:
            bitVal = (RTL8309N_ACT_DROP == action) ? 1:0;
            rtl8309n_regbit_set(6, 16, 10, 15, bitVal);	
			break;

		default:
			break;
	}

	return SUCCESS;
	
}

/* Function Name:
 *      rtl8309n_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 * Output:
 *      pAction		 -  drop or normal forward
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_MULCAST_IPV4 - ipv4 unknown multicast
 *          RTL8309N_MULCAST_IPV6 - ipv6 unknown multicast
 *      action could be:
 *          RTL8309N_ACT_DROP      - trap to cpu 
 *          RTL8309N_ACT_PERMIT   - normal forward
 */  
int32 rtl8309n_trap_unknownIPMcastPktAction_get(uint32 type, uint32 *pAction)
{
	uint32 bitVal;
	
	if((type >= RTL8309N_MULCAST_END) || (NULL == pAction))
		return FAILED;

	switch(type)
	{
		case RTL8309N_MULCAST_IPV4:
			rtl8309n_regbit_get(6, 16, 9, 15, &bitVal);
			break;
			
		case RTL8309N_MULCAST_IPV6:
			rtl8309n_regbit_get(6, 16, 10, 15, &bitVal);
			break;

		default:
			break;
	}
	*pAction = bitVal ? RTL8309N_ACT_DROP : RTL8309N_ACT_PERMIT;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_trap_abnormalPktAction_set
 * Description:
 *      set abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 *      action       -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8309N_DOT1XUNAUTH  - 802.1x authentication fail packet
 *          RTL8309N_LUTSABLOCK   - macth look up table SMAC block entry
 *   		RTL8309N_LUTDABLOCK   - match look up table DMAC block entry
 *      action could be:
 *          RTL8309N_ACT_DROP       - drop 
 *          RTL8309N_ACT_TRAP2CPU    - trap to cpu
 *			RTL8309N_ACT_PERMIT		-	normal forward
 */ 
int32 rtl8309n_trap_abnormalPktAction_set(uint32 type,  uint32 action)
{
    if (type > RTL8309N_ABNORMAL_END || (RTL8309N_ACT_DROP != action 
        && RTL8309N_ACT_TRAP2CPU != action))
        return FAILED;
    
    switch(type)
    {
        case RTL8309N_UNMATCHVID:            
            if(RTL8309N_ACT_DROP == action)
                rtl8309n_regbit_set(0, 16, 2, 16, 0);
            else if (RTL8309N_ACT_TRAP2CPU == action)
                rtl8309n_regbit_set(0, 16, 2, 16, 1);           
            break;
            
        case RTL8309N_DOT1X_UNAUTH:
            if(RTL8309N_ACT_DROP == action)
                rtl8309n_regbit_set(0, 16, 0, 15, 0);
            else if (RTL8309N_ACT_TRAP2CPU == action)
                rtl8309n_regbit_set(0, 16, 0, 15, 1);                        
            break;
			
		case RTL8309N_LUT_SABLOCK:
			if(RTL8309N_ACT_DROP == action)
				rtl8309n_regbit_set(6, 16, 8, 15, 1);
			else if(RTL8309N_ACT_TRAP2CPU)
				rtl8309n_regbit_set(6, 16, 8, 15, 0);
			break;
			
		case RTL8309N_LUT_DABLOCK:
			if(RTL8309N_ACT_DROP == action)
				rtl8309n_regbit_set(6, 16, 7, 15, 1);
			else if(RTL8309N_ACT_TRAP2CPU)
				rtl8309n_regbit_set(6, 16, 7, 15, 0);
			break;
			
        default:
            break;
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_trap_abnormalPktAction_get
 * Description:
 *      get abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8309N_DOT1XUNAUTH  - 802.1x authentication fail packet
 *          RTL8309N_LUTSABLOCK   - macth look up table SMAC block entry
 *   		RTL8309N_LUTDABLOCK   - match look up table DMAC block entry
 *      action could be:
 *          RTL8309N_ACT_DROP       - drop 
 *          RTL8309N_ACT_TRAP2CPU    - trap to cpu
 */ 
int32 rtl8309n_trap_abnormalPktAction_get(uint32 type,  uint32 *pAction)
{
    uint32 bitVal;

    if (type > RTL8309N_ABNORMAL_END)
        return FAILED;
    if (NULL == pAction)
        return FAILED;
    
    switch(type)
    {
        case RTL8309N_UNMATCHVID:            
            rtl8309n_regbit_get(0, 16, 2, 16, &bitVal);  
			*pAction = (bitVal == 1) ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;
            break;   
			
        case RTL8309N_DOT1X_UNAUTH:
            rtl8309n_regbit_get(0, 16, 0, 15, &bitVal);  
			*pAction = (bitVal == 1) ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;
            break;
			
		case RTL8309N_LUT_DABLOCK:
			rtl8309n_regbit_get(6, 16, 7, 15, &bitVal);
			*pAction = (bitVal == 0) ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;
			break;
			
		case RTL8309N_LUT_SABLOCK:
			rtl8309n_regbit_get(6, 16, 8, 15, &bitVal);
			*pAction = (bitVal == 0) ? RTL8309N_ACT_TRAP2CPU : RTL8309N_ACT_DROP;
			break;			
			
		default:
			break;
    }
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_rma_entry_set
 * Description:
 *      Set reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 *      pRmaentry    -  pointer point to struct describes the RMA entry 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are eight types  reserved addresses which user can set asic to determine 
 *      how to forwarding them:
 *      RTL8309N_RMA00, 	 reserved address "01-80-c2-00-00-00"
 *      RTL8309N_RMA01,	   reserved address "01-80-c2-00-00-01"
 *      RTL8309N_RMA02, 	 reserved address "01-80-c2-00-00-02"
 *      RTL8309N_RMA03, 	 reserved address "01-80-c2-00-00-03"
 *		RTL8309N_RMA04_0D0F,	 reserved address "01-80-C2-00-00-04 --- 01-80-C2-00-00-0D" 
 *						 "01-80-C2-00-00-0F" 
 *		RTL8309N_RMA0E, 	 reserved address "01-80-C2-00-00-0E"
 *		RTL8309N_RMA10, 	  reserved address "01-80-c2-00-00-10"
 *		RTL8309N_RMA11_1F,	  reserved address "01-80-C2-00-00-11 --- 01-80-C2-00-00-1F"
 *		RTL8309N_RMA20, 	  reserved address "01-80-c2-00-00-20"
 *		RTL8309N_RMA21, 	  reserved address "01-80-c2-00-00-21"
 *		RTL8309N_RMA22_2F,	  reserved address "01-80-c2-00-00-22 --- 01-80-c2-00-00-2F"
 *		RTL8309N_RMA31_3F,	  reserved address "01-80-c2-00-00-31 --- 01-80-c2-00-00-3F" 
 *		RTL8309N_RMAIGMP,	  IGMP packet without pppoe header
 *		RTL8309N_RMAMLD,	  MLD packet without pppoe header
 *		RTL8309N_RMAPPPOE_IPV4,   IGMP packet with pppoe header
 *		RTL8309N_RAMPPPOE_MLD,	  MLD packet with pppoe header
 *		RTL8309N_RMASWITCH_MAC,   RMA = switch MAC
 *		RTL8309N_RMAUNKNOWN_UNIDA,	  reserved address "xxxxxxx0-xx-xx-xx-xx-xx"
 *		RTL8309N_RMAUNKNOWN_MULTDA,   reserved address "xxxxxxx1-xx-xx-xx-xx-xx"
 *
 *      (2) data of rmaentry are :
 *          action:	
 *            RTL8306_ACT_FORWORD      - Drop the packet
 *            RTL8306_ACT_COPY         - Copy the packet to cpu
 *			  RTL8309N_ACT_TRAP2CPU      - Trap the packet to CPU
 *            RTL8309N_ACT_FLOOD        - Flood the packet
 *          enable_rmapri:  enable or disable the predefined RMA priority
 *          priority:  the predefined priority
 */ 
int32 rtl8309n_rma_entry_set(uint32 type, rtl8309n_rma_entry_t *pRmaentry)
{
	uint32 regVal;
	uint32 enableVal, actionVal;
	
	if(type > RTL8309N_RMAUNKNOWN_MULTDA) 
		return FAILED;
	if (NULL == pRmaentry)
		return FAILED;
	
	/*get value from pRmaentry*/
	switch(pRmaentry->action)
	{
		case RTL8309N_ACT_PERMIT:
			actionVal = 0;
			break;
			
		case RTL8309N_ACT_COPY2CPU:
			actionVal = 1;
			break;

		case RTL8309N_ACT_TRAP2CPU:
			actionVal = 2;
			break;

		case RTL8309N_ACT_DROP:
			actionVal = 3;
			break;

		default:
			break;
	}
	enableVal = (RTL8309N_ENABLED == pRmaentry->enable_rmapri) ? 1 : 0;
	/*set rma entry*/
	switch(type)
	{
		case RTL8309N_RMA00:
			rtl8309n_reg_get(2, 16, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 16, 15, regVal);
			break;
			
		case RTL8309N_RMA01:
			rtl8309n_reg_get(2, 16, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 16, 15, regVal);
			break;
			
		case RTL8309N_RMA02:
			rtl8309n_reg_get(2, 17, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 17, 15, regVal);
			break;
			
		case RTL8309N_RMA03:
			rtl8309n_reg_get(2, 17, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 17, 15, regVal);
			break;
			
		case RTL8309N_RMA04_0D0F:
			rtl8309n_reg_get(2, 18, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 18, 15, regVal);
			break;
			
		case RTL8309N_RMA0E:
			rtl8309n_reg_get(2, 18, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 18, 15, regVal);		
			break;

		case RTL8309N_RMA10:
			rtl8309n_reg_get(2, 19, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 19, 15, regVal);
			break;
			
		case RTL8309N_RMA11_1F:
			rtl8309n_reg_get(2, 19, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 19, 15, regVal);
			break;		

		case RTL8309N_RMA20:
			rtl8309n_reg_get(2, 20, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 20, 15, regVal);
			break;

		case RTL8309N_RMA21:
			rtl8309n_reg_get(2, 20, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 20, 15, regVal);
			break;	

		case RTL8309N_RMA22_2F:
			rtl8309n_reg_get(2, 21, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 21, 15, regVal);
			break;	

		case RTL8309N_RMA31_3F:
			rtl8309n_reg_get(2, 21, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 21, 15, regVal);
			break;

		case RTL8309N_RMAIGMP:
			rtl8309n_reg_get(2, 22, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 22, 15, regVal);
			break;	
			
		case RTL8309N_RMAMLD:
			rtl8309n_reg_get(2, 22, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 22, 15, regVal);
			break;

		case RTL8309N_RMAPPPOE_IPV4:
			rtl8309n_reg_get(2, 23, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 23, 15, regVal);
			break;	

		case RTL8309N_RMAPPPOE_MLD:
			rtl8309n_reg_get(2, 23, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 23, 15, regVal);
			break;

		case RTL8309N_RMASWITCH_MAC:
			rtl8309n_reg_get(2, 24, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 24, 15, regVal);
			break;				

		case RTL8309N_RMAUNKNOWN_UNIDA:
			rtl8309n_reg_get(2, 24, 15, &regVal);
			regVal &= ~(0x1F00);
			regVal |= (pRmaentry->priority << 11) | (enableVal << 10) | (actionVal << 8);
			rtl8309n_reg_set(2, 24, 15, regVal);
			break;	

		case RTL8309N_RMAUNKNOWN_MULTDA:
			rtl8309n_reg_get(2, 25, 15, &regVal);
			regVal &= ~0x001F;
			regVal |= (pRmaentry->priority << 3) | (enableVal << 2) | actionVal;
			rtl8309n_reg_set(2, 25, 15, regVal);
			break;	

		default:
			break;
	}
	
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_rma_entry_get
 * Description:
 *      Get reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are eight types  reserved addresses which user can set asic to determine 
 *      how to forwarding them:
 *      RTL8309N_RMA00, 	 reserved address "01-80-c2-00-00-00"
 *      RTL8309N_RMA01	   reserved address "01-80-c2-00-00-01"
 *      RTL8309N_RMA02, 	 reserved address "01-80-c2-00-00-02"
 *      RTL8309N_RMA03, 	 reserved address "01-80-c2-00-00-03"
 *		RTL8309N_RMA04_0D0F,	 reserved address "01-80-C2-00-00-04 --- 01-80-C2-00-00-0D" 
 *						 "01-80-C2-00-00-0F" 
 *		RTL8309N_RMA0E, 	 reserved address "01-80-C2-00-00-0E"
 *		RTL8309N_RMA10, 	  reserved address "01-80-c2-00-00-10"
 *		RTL8309N_RMA11_1F,	  reserved address "01-80-C2-00-00-11 --- 01-80-C2-00-00-1F"
 *		RTL8309N_RMA20, 	  reserved address "01-80-c2-00-00-20"
 *		RTL8309N_RMA21, 	  reserved address "01-80-c2-00-00-21"
 *		RTL8309N_RMA22_2F,	  reserved address "01-80-c2-00-00-22 --- 01-80-c2-00-00-2F"
 *		RTL8309N_RMA31_3F,	  reserved address "01-80-c2-00-00-31 --- 01-80-c2-00-00-3F" 
 *		RTL8309N_RMAIGMP,	  IGMP packet without pppoe header
 *		RTL8309N_RMAMLD,	  MLD packet without pppoe header
 *		RTL8309N_RMAPPPOE_IPV4,   IGMP packet with pppoe header
 *		RTL8309N_RAMPPPOE_MLD,	  MLD packet with pppoe header
 *		RTL8309N_RMASWITCH_MAC,   RMA = switch MAC
 *		RTL8309N_RMAUNKNOWN_UNIDA,	  reserved address "xxxxxxx0-xx-xx-xx-xx-xx"
 *		RTL8309N_RMAUNKNOWN_MULTDA,   reserved address "xxxxxxx1-xx-xx-xx-xx-xx"
 *
 *      (2) data of rmaentry are :
 *          action:	
 *            RTL8306_ACT_FORWORD      - Drop the packet
 *            RTL8306_ACT_COPY         - Copy the packet to cpu
 *			  RTL8309N_ACT_TRAP2CPU      - Trap the packet to CPU
 *            RTL8309N_ACT_FLOOD        - Flood the packet
 *          enable_rmapri:  enable or disable the predefined RMA priority
 *          priority:  the predefined priority
 */ 
int32 rtl8309n_rma_entry_get(uint32 type, rtl8309n_rma_entry_t *pRmaentry)
{
	uint32 regVal;
	uint32 action1, enable_rmapri1, priority1;

	if((type > RTL8309N_RMAUNKNOWN_MULTDA) || (NULL == pRmaentry))
		return FAILED;

	/*read rma entry*/
	switch(type)
	{
		case RTL8309N_RMA00:
			rtl8309n_reg_get(2, 16, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;
			
		case RTL8309N_RMA01:
			rtl8309n_reg_get(2, 16, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;
			
		case RTL8309N_RMA02:
			rtl8309n_reg_get(2, 17, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;
			
		case RTL8309N_RMA03:
			rtl8309n_reg_get(2, 17, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;
			
		case RTL8309N_RMA04_0D0F:
			rtl8309n_reg_get(2, 18, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;
			
		case RTL8309N_RMA0E:
			rtl8309n_reg_get(2, 18, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;		
			break;

		case RTL8309N_RMA10:
			rtl8309n_reg_get(2, 19, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;
			
		case RTL8309N_RMA11_1F:
			rtl8309n_reg_get(2, 19, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;		

		case RTL8309N_RMA20:
			rtl8309n_reg_get(2, 20, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;

		case RTL8309N_RMA21:
			rtl8309n_reg_get(2, 20, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;	

		case RTL8309N_RMA22_2F:
			rtl8309n_reg_get(2, 21, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;	

		case RTL8309N_RMA31_3F:
			rtl8309n_reg_get(2, 21, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;

		case RTL8309N_RMAIGMP:
			rtl8309n_reg_get(2, 22, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;	
			
		case RTL8309N_RMAMLD:
			rtl8309n_reg_get(2, 22, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;

		case RTL8309N_RMAPPPOE_IPV4:
			rtl8309n_reg_get(2, 23, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;	

		case RTL8309N_RMAPPPOE_MLD:
			rtl8309n_reg_get(2, 23, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;

		case RTL8309N_RMASWITCH_MAC:
			rtl8309n_reg_get(2, 24, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;				

		case RTL8309N_RMAUNKNOWN_UNIDA:
			rtl8309n_reg_get(2, 24, 15, &regVal);
			action1 = (regVal >> 8) & 0x3;
			enable_rmapri1 = (regVal >> 10) & 0x1;
			priority1 = (regVal >> 11) & 0x3;
			break;	

		case RTL8309N_RMAUNKNOWN_MULTDA:
			rtl8309n_reg_get(2, 25, 15, &regVal);
			action1 = regVal & 0x3;
			enable_rmapri1 = (regVal >> 2) & 0x1;
			priority1 = (regVal >> 3) & 0x3;
			break;	

		default:
			break;
	}
	/*get value for pRmaentry*/
	switch(action1)
	{
		case 0:
			pRmaentry->action = RTL8309N_ACT_PERMIT;
			break;
			
		case 1:
			pRmaentry->action = RTL8309N_ACT_COPY2CPU;
			break;
			
		case 2:
			pRmaentry->action = RTL8309N_ACT_TRAP2CPU;
			break;
			
		case 3:
			pRmaentry->action = RTL8309N_ACT_DROP;
			break;
			
		default:
			break;
	}
	pRmaentry->enable_rmapri = (enable_rmapri1 ? RTL8309N_ENABLED : RTL8309N_DISABLED);
	pRmaentry->priority = priority1;
	
	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_rma_userentry_set
 *Description:
 *     set user defined RMA entry
 *Input:
 *     type       -  reserved MAC address type
 *     rmaentry   -  struct describes the RMA entry
 *     macAddr    -  pointer point to MAC address
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note
 *     User can set a user defined RMA entry. A user defined RMA entry includes MAC address, RMA entry data.
 *     This API can be called to write a user defined entry to RMA entry table.
 */
int32 rtl8309n_rma_userentry_set(rtl8309n_rma_entry_t *pRmaentry, uint8 *macAddr)
{
	uint32 regVal;
	
	if((NULL == pRmaentry) || (NULL == macAddr))
		return FAILED;

	/*RMA entry data*/
	rtl8309n_reg_get(2, 25, 15, &regVal);
	regVal &= ~0x1F00;
	regVal |= (pRmaentry->priority << 11) | (pRmaentry->enable_rmapri << 10) | (pRmaentry->action << 8);
	rtl8309n_reg_set(2, 25, 15, regVal);
	/*reserved mac address*/
	/*wrtie Data[15:0]*/
    rtl8309n_reg_set(2, 26, 15, (macAddr[1] << 8) | macAddr[0]);
    /*wrtie Data[31:16]*/
    rtl8309n_reg_set(2, 27, 15, (macAddr[3] << 8) | macAddr[2]);	
    /*write Data[47:32]*/
    rtl8309n_reg_set(2, 28, 15, (macAddr[5] << 8) | macAddr[4]);
    
	return SUCCESS;
}

/*Function Name:
 *     rtl8309n_rma_userentry_set
 *Description:
 *     Get user defined RMA entry
 *Input:
 *     none
 *Output:
 *     pRmaentry   -  pointer point to struct describes the RMA entry
 *     macAddr     -  pointer point to MAC address
 *Return:
 *     SUCCESS
 *     FAILED
 *Note
 *     User can set a user defined RMA entry. A user defined RMA entry includes MAC address, RMA entry data.
 *     This API can be called to write a user defined entry to RMA entry table.
 */
int32 rtl8309n_rma_userentry_get(rtl8309n_rma_entry_t *pRmaentry, uint8 *macAddr)
{
	uint32 regVal;
	
	if((NULL == pRmaentry) || (NULL == macAddr))
		return FAILED;

	/*RMA entry data*/
	rtl8309n_reg_get(2, 25, 15, &regVal);
	pRmaentry->priority = (regVal >> 11) & 0x3;
	pRmaentry->enable_rmapri = (regVal >> 10) & 0x1;
	pRmaentry->action = (regVal >> 8) & 0x3;
	/*reserved mac address*/
	/*wrtie Data[15:0]*/
    rtl8309n_reg_get(2, 26, 15, &regVal);
	macAddr[1] = (regVal >> 8) & 0xFF;
	macAddr[0] = regVal & 0xFF;
    /*wrtie Data[31:16]*/
    rtl8309n_reg_get(2, 27, 15, &regVal);
	macAddr[3] = (regVal >> 8) & 0xFF;
	macAddr[2] = regVal & 0xFF;	
    /*write Data[47:32]*/
	rtl8309n_reg_get(2, 28, 15, &regVal);
	macAddr[5] = (regVal >> 8) & 0xFF;
	macAddr[4] = regVal & 0xFF;
	
	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_intr_enable__set
 * Description:
 *      Enable asic interrupt, set interrupt msk 
 * Input:
 *      enInt        -  Enable interrupt cpu
 *      intmask      -  interrupt event  mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt mask register:
 *         bit15: enable interrupt CPU
 *         bit12: enable unmatched sa interrupt
 *         bit11: enable wakenup frame interrupt
 *         bit10: enable loop detection interrupt
 *         bit9:  enable storm filter interrupt
 *         bit8:  enable port8 link change interrupt
 *         bit7:  enable port7 link change interrupt
 *         bit6:  enable port6 link change interrupt
 *         bit5:  enable port5 link change interrupt
 *         bit4:  enable port4 link change interrupt
 *         bit3:  enable port3 link change interrupt
 *         bit2:  enable port2 link change interrupt
 *         bit1:  enable port1 link change interrupt
 *         bit0:  enable port0 link change interrupt
 */ 
int32 rtl8309n_intr_enable_set(uint32 enInt, uint32 intmask)
{
    uint32 regValue;

    if (intmask > 0x1FFF)
        return FAILED;

    if (RTL8309N_DISABLED == enInt) 
        /*CPU interrupt disable, do not change interrupt port mask*/
        rtl8309n_regbit_set(0, 16, 15, 17, 0);
    else if (RTL8309N_ENABLED == enInt) 
        /*CPU interrupt enable*/
        rtl8309n_regbit_set(0, 16, 15, 17, 1);
 
    /*Set link change interrupt mask*/
    rtl8309n_reg_get(0, 16, 17, &regValue);
    regValue &= ~0x1FFF;
	regValue |= (intmask & 0x1fff);
    rtl8309n_reg_set(0, 16, 17, regValue);
    
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_intr_enable_get
 * Description:
 *      Get Asic interrupt enabling status, and get interrupt msk
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt mask register:
 *         bit15: enable interrupt CPU
 *         bit12: enable unmatched sa interrupt
 *         bit11: enable wakenup frame interrupt
 *         bit10: enable loop detection interrupt
 *         bit9:  enable storm filter interrupt
 *         bit8:  enable port8 link change interrupt
 *         bit7:  enable port7 link change interrupt
 *         bit6:  enable port6 link change interrupt
 *         bit5:  enable port5 link change interrupt
 *         bit4:  enable port4 link change interrupt
 *         bit3:  enable port3 link change interrupt
 *         bit2:  enable port2 link change interrupt
 *         bit1:  enable port1 link change interrupt
 *         bit0:  enable port0 link change interrupt
 */ 
int32 rtl8309n_intr_enable_get(uint32 *pEnInt, uint32 *pIntmask) 
{
    uint32 regVal, bitVal;

    if ((NULL == pEnInt) || (NULL == pIntmask))
        return FAILED;
  
    /*CPU interrupt enable*/
    rtl8309n_regbit_get(0, 16, 15, 17, &bitVal);
	*pEnInt = bitVal? RTL8309N_ENABLED : RTL8309N_DISABLED;
    
    /*get link change interrupt mask*/
    rtl8309n_reg_get(0, 16, 17, &regVal);
	*pIntmask = regVal & 0x1FFF;

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_intr_status_set
 * Description:
 *      Clear Asic interrupt event flag
 * Input:
 *      intrmask    -   interrupt mask 
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt status register:
 *         bit12: unmatched sa interrupt
 *         bit11: wakenup frame interrupt
 *         bit10: loop detection interrupt
 *         bit9:  storm filter interrupt
 *         bit8:  port8 link change interrupt
 *         bit7:  port7 link change interrupt
 *         bit6:  port6 link change interrupt
 *         bit5:  port5 link change interrupt
 *         bit4:  port4 link change interrupt
 *         bit3:  port3 link change interrupt
 *         bit2:  port2 link change interrupt
 *         bit1:  port1 link change interrupt
 *         bit0:  port0 link change interrupt
 */ 
int32 rtl8309n_intr_status_set(uint32 intrmask) 
{

    if (intrmask > 0x1fff)
        return FAILED;
    
    rtl8309n_reg_set(0, 17, 17, intrmask&0x1fff);
	
    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_intr_status_get
 * Description:
 *      Get Asic interrupt event flag
 * Input:
 *      none 
 * Output:
 *      pStatusMask - pointer pointed to interrupt mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt status register:
 *         bit12: unmatched sa interrupt
 *         bit11: wakenup frame interrupt
 *         bit10: loop detection interrupt
 *         bit9:  storm filter interrupt
 *         bit8:  port8 link change interrupt
 *         bit7:  port7 link change interrupt
 *         bit6:  port6 link change interrupt
 *         bit5:  port5 link change interrupt
 *         bit4:  port4 link change interrupt
 *         bit3:  port3 link change interrupt
 *         bit2:  port2 link change interrupt
 *         bit1:  port1 link change interrupt
 *         bit0:  port0 link change interrupt
 */ 
int32 rtl8309n_intr_status_get(uint32 *pStatusMask) 
{
    uint32 regValue;

    if (NULL == pStatusMask)
        return FAILED;
    
    rtl8309n_reg_get(0, 17, 17, &regValue);
    *pStatusMask = regValue & 0x1FFF;
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_storm_filterEnable_set
 * Description:
 *      Enable Asic storm filter 
 * Input:
 *      port      -  port number(0-8)
 *      type      -  specify storm filter type
 *      enabled   -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 4 kinds of storm filter:
 *          RTL8309N_BROADCASTPKT       -   storm filter for broadcast packet
 *          RTL8309N_MULTICASTPKT       -   storm filter for all multicast packet, include known and unknown
 *          RTL8309N_UNKOWN_UNIDAPKT    -   storm filter for unknown unicast DA packet
 *          RTL8309N_UNKOWN_MULTDAPKT   -   storm filter for unknown multicast DA packet
 */ 
int32 rtl8309n_storm_filterEnable_set(uint32 port, uint32 type, uint32 enabled)
{ 
	
	if ((port > RTL8309N_MAX_PORT_ID) || (type >= RTL8309N_PKT_TYPE_END))
		return FAILED;
	
    switch(type) 
    {
        case RTL8309N_BROADCASTPKT:
            rtl8309n_regbit_set(port, 16, 0, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
			
        case RTL8309N_MULTICASTPKT:
			rtl8309n_regbit_set(port, 19, 3, 8, 1);
            rtl8309n_regbit_set(port, 19, 0, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
            break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			rtl8309n_regbit_set(port, 22, 0, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			rtl8309n_regbit_set(port, 19, 3, 8, 0);	
			rtl8309n_regbit_set(port, 19, 0, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
			
		default:
			break;			
    }
	
    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_storm_filterEnable_get
 * Description:
 *      Get Asic storm filter enabled or disabled 
 * Input:
 *      port      -  port number(0-8)
 *      type      -  specify storm filter type
 * Output:
 *      pEnabled  -  the pointer of enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 4 kinds of storm filter:
 *          RTL8309N_BROADCASTPKT       -   storm filter for broadcast packet
 *          RTL8309N_MULTICASTPKT       -   storm filter for all multicast packet, include known and unknown
 *          RTL8309N_UNKOWN_UNIDAPKT    -   storm filter for unknown unicast DA packet
 *          RTL8309N_UNKOWN_MULTDAPKT   -   storm filter for unknown multicast DA packet
 */
int32 rtl8309n_storm_filterEnable_get(uint32 port, uint32 type, uint32 *pEnabled)
{
    uint32 bitVal1, bitVal2;

    if ((port > RTL8309N_MAX_PORT_ID) || (type >= RTL8309N_PKT_TYPE_END) || (pEnabled == NULL))
        return FAILED;
    
    switch(type) 
    {
        case RTL8309N_BROADCASTPKT:
            rtl8309n_regbit_get(port, 16, 0, 8, &bitVal2);
            break;
			
        case RTL8309N_MULTICASTPKT: 
			rtl8309n_regbit_get(port, 19, 3, 8, &bitVal1);
			if(bitVal1)
				rtl8309n_regbit_get(port, 19, 0, 8, &bitVal2);
			else 
                bitVal2 = FALSE;
            break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			rtl8309n_regbit_get(port, 22, 0, 8, &bitVal2);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			rtl8309n_regbit_get(port, 19, 3, 8, &bitVal1);
			if(!bitVal1)
				rtl8309n_regbit_get(port, 19, 0, 8, &bitVal2);
			else 
                bitVal2 = FALSE;
			break;
			
		default:
			break;			
    }
    *pEnabled = (bitVal2 ? RTL8309N_ENABLED : RTL8309N_DISABLED);
	
    return SUCCESS;
}

/*Function Name:
 *		rtl8309n_storm_filterAttr_set
 *Description:
 *		set storm filter attributes
 *Input:
 *		port	-	port number (0 - 8)
 *		storm	-	storm filter traffic type
 *		pStorm_data		-	pointer point to the struct describing the storm filter attributes
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)There are 4 types of traffic that could be filtered by the RTL8309N storm filetr. So storm_type
 *			could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 *		(2)This API can be called to set storm filter counter unit, rate, burst size and whether inclue ifg when calculate
 *			rate.
 */
int32 rtl8309n_storm_filterAttr_set(uint32 port, uint32 storm_type, rtl8309n_storm_attr_t *pStorm_data)
{
	uint32 highBit;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (storm_type >= RTL8309N_PKT_TYPE_END)
		return FAILED;
	if (NULL == pStorm_data)
		return FAILED;
	
	/*set storm filter attributes*/
	switch(storm_type)
	{
		case RTL8309N_BROADCASTPKT:
			/*counter unit*/
			rtl8309n_regbit_set(port, 16, 1, 8, pStorm_data->unit ? 1 : 0);
			/*rate*/
			highBit = (pStorm_data->rate >> 16) & 0x1;
			rtl8309n_regbit_set(port, 16, 15, 8, highBit ? 1 : 0);
			rtl8309n_reg_set(port, 17, 8, pStorm_data->rate - (highBit << 16));
			/*burst*/
			rtl8309n_reg_set(port, 18, 8, pStorm_data->burst);
			break;
			
        case RTL8309N_MULTICASTPKT:
			/*counter unit*/
			rtl8309n_regbit_set(port, 19, 1, 8, pStorm_data->unit ? 1 : 0);
			/*rate*/
			highBit = (pStorm_data->rate >> 16) & 0x1;
			rtl8309n_regbit_set(port, 19, 15, 8, highBit ? 1 : 0);
			rtl8309n_reg_set(port, 20, 8, pStorm_data->rate - (highBit << 16));
			/*both known and unknown multicast traffic*/
			rtl8309n_regbit_set(port, 19, 3, 8, 1);
			/*burst*/
			rtl8309n_reg_set(port, 21, 8, pStorm_data->burst);			
			break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			/*counter unit*/
			rtl8309n_regbit_set(port, 22, 1, 8, pStorm_data->unit ? 1 : 0);
			/*rate*/
			highBit = (pStorm_data->rate >> 16) & 0x1;
			rtl8309n_regbit_set(port, 22, 15, 8, highBit ? 1 : 0);
			rtl8309n_reg_set(port, 23, 8, pStorm_data->rate - (highBit << 16));
			/*burst*/
			rtl8309n_reg_set(port, 24, 8, pStorm_data->burst);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			/*counter unit*/
			rtl8309n_regbit_set(port, 19, 1, 8, pStorm_data->unit ? 1 : 0);
			/*rate*/
			highBit = (pStorm_data->rate >> 16) & 0x1;
			rtl8309n_regbit_set(port, 19, 15, 8, highBit ? 1 : 0);
			rtl8309n_reg_set(port, 20, 8, pStorm_data->rate - (highBit << 16));
			/*unknown multicast traffic*/
			rtl8309n_regbit_set(port, 19, 3, 8, 0);
			/*burst*/
			rtl8309n_reg_set(port, 21, 8, pStorm_data->burst);			
			break;

		default:
			break;
	}

	rtl8309n_regbit_set(6, 16, 0, 14, (RTL8309N_ENABLED == pStorm_data->ifg_include) ? 1 : 0);
	
	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_storm_filterAttr_get
 *Description:
 *		Get storm filter attributes
 *Input:
 *		port	-	port number (0 - 8)
 *		storm	-	storm filter traffic type
 *Output:
 *		pStorm_data		-	pointer point to the struct describing the storm filter attributes
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)There are 4 types of traffic that could be filtered by the RTL8309N storm filetr. So storm_type
 *			could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 *		(2)This API can be called to get storm filter counter unit, rate, burst size and whether inclue ifg when calculate
 *			rate.
 */
int32 rtl8309n_storm_filterAttr_get(uint32 port, uint32 storm_type, rtl8309n_storm_attr_t *pStorm_data)
{
	uint32 regValue0, regValue1, bitVal;
	if (port > RTL8309N_PORT_NUMBER - 1)
		return FAILED;
	if (storm_type >= RTL8309N_PKT_TYPE_END)
		return FAILED;
	if (NULL == pStorm_data)
		return FAILED;

	/*set storm filter attributes*/
	switch(storm_type)
	{
		case RTL8309N_BROADCASTPKT:
			/*counter unit*/
			rtl8309n_regbit_get(port, 16, 1, 8, &pStorm_data->unit);
			/*rate*/
			rtl8309n_regbit_get(port, 16, 15, 8, &regValue0);
			rtl8309n_reg_get(port, 17, 8, &regValue1);
			pStorm_data->rate = (regValue0 << 16) + regValue1;
			/*burst*/
			rtl8309n_reg_get(port, 18, 8, &pStorm_data->burst);
			break;
			
        case RTL8309N_MULTICASTPKT:
			/*counter unit*/
			rtl8309n_regbit_get(port, 19, 1, 8, &pStorm_data->unit);
			/*rate*/
			rtl8309n_regbit_get(port, 19, 15, 8, &regValue0);
			rtl8309n_reg_get(port, 20, 8, &regValue1);
			pStorm_data->rate = (regValue0 << 16) + regValue1;
			/*burst*/
			rtl8309n_reg_get(port, 21, 8, &pStorm_data->burst);			
			break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			/*counter unit*/
			rtl8309n_regbit_get(port, 22, 1, 8, &pStorm_data->unit);
			/*rate*/
			rtl8309n_regbit_get(port, 22, 15, 8, &regValue0);
			rtl8309n_reg_get(port, 23, 8, &regValue1);
			pStorm_data->rate = (regValue0 << 16) + regValue1;
			/*burst*/
			rtl8309n_reg_get(port, 24, 8, &pStorm_data->burst);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			/*counter unit*/
			rtl8309n_regbit_get(port, 19, 1, 8, &pStorm_data->unit);
			/*rate*/
			rtl8309n_regbit_get(port, 19, 15, 8, &regValue0);
			rtl8309n_reg_get(port, 20, 8, &regValue1);
			pStorm_data->rate = (regValue0 << 16) + regValue1;
			/*burst*/
			rtl8309n_reg_get(port, 21, 8, &pStorm_data->burst);			
			break;

		default:
			break;
	}

	rtl8309n_regbit_get(6, 16, 0, 14, &bitVal);
	pStorm_data->ifg_include = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;	
}

/*Function Name:
 *		rtl8309n_storm_filterStatus_set
 *Description:
 *		clear storm counter exceeding status
 *Input:
 *		port	-	port number (0 - 8)
 *		storm_type	-	storm traffic type
 *		enabled		-	enable clearing storm counter exceeding status
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)When the dedicated traffic exceed the storm filter rate for one port, it means that the 
 *		    the dedicated traffic storm has happened for this port. And the exceed status will be set to 1
 *		    until it is cleared by software. When software write 1 to the exceed bit, the exceeding status 
 *		    will be cleared.
 *		(2)There are 4 types of storm_type, and they could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 */
int32 rtl8309n_storm_filterStatus_set(uint32 port, uint32 storm_type, uint32 enabled)
{
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (storm_type >= RTL8309N_PKT_TYPE_END)
		return FAILED;

	switch(storm_type)
	{
		case RTL8309N_BROADCASTPKT:
			rtl8309n_regbit_set(port, 16, 2, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
			
        case RTL8309N_MULTICASTPKT:
			rtl8309n_regbit_set(port, 19, 2, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);	
			break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			rtl8309n_regbit_set(port, 22, 2, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			rtl8309n_regbit_set(port, 19, 2, 8, (RTL8309N_ENABLED == enabled) ? 1 : 0);	
			break;

		default:
			break;
	}

	return SUCCESS;
}

/*Function Name:
 *		rtl8309n_storm_filterStatus_get
 *Description:
 *		Get storm counter exceeding status
 *Input:
 *		port	-	port number (0 - 8)
 *		storm_type	-	storm traffic type
 *		pStatus	-	storm counter exceeding status
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)When the dedicated traffic exceed the storm filter rate for one port, it means that the 
 *		    the dedicated traffic storm has happened for this port. And the exceed status will be set to 1
 *		    until it is cleared by software. When software write 1 to the exceed bit, the exceeding status 
 *		    will be cleared.
 *		(2)There are 4 types of storm_type, and they could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 */
int32 rtl8309n_storm_filterStatus_get(uint32 port, uint32 storm_type, uint32 *pStatus)
{
	uint32 bitVal;
	
	if (port > RTL8309N_MAX_PORT_ID)
		return FAILED;
	if (storm_type >= RTL8309N_PKT_TYPE_END)
		return FAILED;

	switch(storm_type)
	{
		case RTL8309N_BROADCASTPKT:
			rtl8309n_regbit_get(port, 16, 2, 8, &bitVal);
			break;
			
        case RTL8309N_MULTICASTPKT:
			rtl8309n_regbit_get(port, 19, 2, 8, &bitVal);	
			break;
			
		case RTL8309N_UNKOWN_UNIDAPKT:
			rtl8309n_regbit_get(port, 22, 2, 8, &bitVal);
			break;
			
		case RTL8309N_UNKOWN_MULTDAPKT:
			rtl8309n_regbit_get(port, 19, 2, 8, &bitVal);	
			break;

		default:
			break;
	}

	*pStatus = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;
}

/* RLDP/RLPP */

/* Function Name:
 *		rtl8309n_rldp_enable_set
 * Description:
 *		Enable RLDP ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_enable_set(uint32 enabled)
{
	/*enable RLDP/RLPP*/
	rtl8309n_regbit_set(4, 19, 0, 14, (RTL8309N_ENABLED == enabled) ? 0 : 1);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_enable_set
 * Description:
 *		Get RLDP ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_enable_get(uint32 *pEnabled)
{
	uint32 bitVal;
	
	rtl8309n_regbit_get(4, 19, 0, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_priEnable_set
 * Description:
 *		Enable RLDP priority ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_priEnable_set(uint32 enabled)
{
	/*enbale RLDP/RLPP priority*/
	rtl8309n_regbit_set(4, 19, 4, 14, (RTL8309N_ENABLED == enabled) ? 1 :0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_priEnable_set
 * Description:
 *		Get RLDP priority ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_priEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(4, 19, 4, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;	

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_ttlEnable_set
 * Description:
 *		Enable RLDP ttl timer ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_ttlEnable_set(uint32 enabled)
{
	/*enable RLDP/RLPP TTL*/
	rtl8309n_regbit_set(4, 19, 7, 14, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;

}

/* Function Name:
 *		rtl8309n_rldp_ttlEnable_get
 * Description:
 *		Get RLDP ttl timer ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_ttlEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(4, 19, 7, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_stormFilterEnable_set
 * Description:
 *		Enable RLDP storm filter ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_stormFilterEnable_set(uint32 enabled)
{
	/*enable RLDP/RLPP storm filter*/
	rtl8309n_regbit_set(4, 19, 12, 14, (RTL8309N_ENABLED == enabled) ? 0 : 1);	
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_stormfilterEnable_get
 * Description:
 *		Get RLDP storm filter ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_stormFilterEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(4, 19, 12, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_vlanTgaEnable_set
 * Description:
 *		Enable RLDP vlan tag ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_vlanTagEnable_set(uint32 enabled)
{
	/*enable RLDP/RLPP vlan tag*/
	rtl8309n_regbit_set(4, 19, 13, 14, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;

}

/* Function Name:
 *		rtl8309n_rldp_vlanTagEnable_get
 * Description:
 *		Get RLDP vlan tag ability status
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_vlanTagEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(4, 19, 13, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_controlAttr_set
 * Description:
 * 		Set RLDP/RLPP attributes
 * Input:
 *		pRldp_data		-	pointer point to the struct describing the RLDP/RLPP attributes
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1)Before RLDP/RLPP function work properly, RLDP/RLPP frame transmit timer, priority, TTL timer 
 *          value,switch mac address have to set correctly.
 *          contents of struct rtl8309n_rldp_controlAttr_t:
 *              	uint32 timer;   
 *	                uint32 priority;    
 *	                rtl8309n_mac_t mac;     
 *      (2)The timer(RLDP Frame Transmit Time Period) could be:
 *              RTL8309N_RLDP_TIMER_3_5MIN
 *              RTL8309N_RLDP_TIMER_100S
 *              RTL8309N_RLDP_TIMER_10S
 *              RTL8309N_RLDP_TIMER_1S              
 */
int32 rtl8309n_rldp_controlAttr_set(rtl8309n_rldp_controlAttr_t *pRldp_data)
{
	uint32 regValue, timer;

	if(NULL == pRldp_data)
		return FAILED;
	if((pRldp_data->timer > 3) ||(pRldp_data->priority > 3) || (pRldp_data->ttl > 16))
		return FAILED;

	//rtl8309n_rldp_enable_set(RTL8309N_ENABLED);
	//rtl8309n_rldp_priEnable_set(RTL8309N_ENABLED);
	//rtl8309n_rldp_ttlEnable_set(RTL8309N_ENABLED);
	
	/*RLDP/RLPP frame transmit period*/
	switch(pRldp_data->timer)
	{
		case RTL8309N_RLDP_TIMER_3_5MIN:
			timer = 0;
			break;
			
		case RTL8309N_RLDP_TIMER_100S:
			timer = 1;
			break;
			
		case RTL8309N_RLDP_TIMER_10S:
			timer = 2;
			break;
			
		case RTL8309N_RLDP_TIMER_1S:
			timer = 3;
			break;	
		default:
			break;
	}
	rtl8309n_reg_get(4, 19, 14, &regValue);
	regValue &= ~(0x3 << 2);
	regValue |= timer << 2;
	/*RLDP/RLPP priority*/
	regValue &= ~(0x3 << 5); 
	regValue |= pRldp_data->priority << 5;
	/*RLDP/RLPP ttl*/
	regValue &= ~(0xF << 8);
	regValue |= pRldp_data->ttl << 8;
	rtl8309n_reg_set(4, 19, 14, regValue);

	/*switch MAC address*/
	/*MAC[32:47],MAC[5,4]*/
	rtl8309n_reg_set(4, 16, 14, (pRldp_data->mac.octet[1] << 8) | pRldp_data->mac.octet[0]);
	/*MAC[16:31],MAC[3,2]*/
	rtl8309n_reg_set(4, 17, 14, (pRldp_data->mac.octet[3] << 8) | pRldp_data->mac.octet[2]);
	/*MAC[0:15],MAC[0,1]*/
	rtl8309n_reg_set(4, 18, 14, (pRldp_data->mac.octet[5] << 8) | pRldp_data->mac.octet[4]);
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rldp_controlAttr_get
 * Description:
 * 		Get RLDP/RLPP attributes
 * Input:
 *		none
 * Output:
 *		pRldp_data		-	pointer point to the struct describing the RLDP/RLPP attributes
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1)Before RLDP/RLPP function work properly, RLDP/RLPP frame transmit timer, priority, TTL timer 
 *          value,switch mac address have to set correctly.
 *          contents of struct rtl8309n_rldp_controlAttr_t:
 *              	uint32 timer;   
 *	                uint32 priority;    
 *	                rtl8309n_mac_t mac;     
 *      (2)The timer(RLDP Frame Transmit Time Period) could be:
 *              RTL8309N_RLDP_TIMER_3_5MIN
 *              RTL8309N_RLDP_TIMER_100S
 *              RTL8309N_RLDP_TIMER_10S
 *              RTL8309N_RLDP_TIMER_1S              
 */
int32 rtl8309n_rldp_controlAttr_get(rtl8309n_rldp_controlAttr_t *pRldp_data)
{
	uint32 regVal;

	if(NULL == pRldp_data)
		return FAILED;

	/*RLDP/RLPP timer, priority, ttl*/
	rtl8309n_reg_get(4, 19, 14, &regVal);
	pRldp_data->timer = (regVal >> 2) & 0x3;
	pRldp_data->priority = (regVal >> 5) & 0x3;
	pRldp_data->ttl = (regVal >> 8) & 0xF;

	/*Switch MAC address*/
	/*DATA[15:0],MAC[0:15],MAC[5,4]*/
	rtl8309n_reg_get(4, 16, 14, &regVal);
	pRldp_data->mac.octet[1] = (regVal >> 8) & 0xFF;
	pRldp_data->mac.octet[0] = regVal & 0XFF;
	/*DATA[15:0],MAC[16:31],MAC[3,2]*/
	rtl8309n_reg_get(4, 17, 14, &regVal);
	pRldp_data->mac.octet[3] = (regVal >> 8) & 0xFF;
	pRldp_data->mac.octet[2] = regVal & 0xFF;
	/*DATA[15:0],MAC[32:47],MAC[0,1]*/
	rtl8309n_reg_get(4, 18, 14, &regVal);
	pRldp_data->mac.octet[5] = (regVal >> 8) & 0xFF;
	pRldp_data->mac.octet[4] = regVal & 0XFF;

	return SUCCESS;
}

 
/* Function Name:
 *		rtl8309n_rlpp_action_set
 * Description:
 *		Set RLPP frame action
 * Input:
 *		action	-	action on RLPP frame
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Action for RLPP frame could only be one of the value as follows:
 *		      RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *		      RTL8309N_ACT_PERMIT     -   forward as normal
 *		(2)When RTL8309N receive the RLPP frame, it will either trap the frame to CPU or forward it
 *		normally.
 */
int32 rtl8309n_rlpp_action_set(uint32 action)
{
    uint32 bitVal;
    
	if ((action != RTL8309N_ACT_TRAP2CPU) && (action != RTL8309N_ACT_PERMIT))
		return FAILED;

    bitVal = (RTL8309N_ACT_TRAP2CPU == action)? 0:1;
	rtl8309n_regbit_set(4, 19, 1, 14, bitVal);


	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_rlpp_action_get
 * Description:
 *		Get RLPP frame action
 * Input:
 *		none
 * Output:
 *		pAction		-	action on RLPP frame
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		Action for RLPP frame could only be one of the value as follows:
 *		      RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *		      RTL8309N_ACT_PERMIT     -   forward as normal
 *		When RTL8309N receive the RLPP frame, it will either trap the frame to CPU or forward it
 *		normally.
 */
int32 rtl8309n_rlpp_action_get(uint32 *pAction)
{
	uint32 bitVal;

	if (NULL == pAction)
		return FAILED;

	rtl8309n_regbit_get(4, 19, 1, 14, &bitVal);
	*pAction = bitVal ? RTL8309N_ACT_PERMIT : RTL8309N_ACT_TRAP2CPU;

	return SUCCESS;
}

/* Function Name:
 *      rtl8309n_rldp_identifierEnable_set
 * Description:
 *      enable calculate and put identifier into RLDPP packet
 * Input:
 *      enabled     -   enable or disable
 * Output:
 *      none
 * Return:
 *      SUCCESS
 * Note:
 *      When identifier is enabled, ASIC will calculate identifier and put it into RLDP 
 *      packet. When ASIC receive a RLDP packet, it will check identifier besides checking
 *      source mac address and protocol.
 */
int32 rtl8309n_rldp_identifierEnable_set(uint32 enabled)
{
    rtl8309n_regbit_set(4, 19, 14, 14, enabled ? RTL8309N_ENABLED : RTL8309N_DISABLED);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_rldp_identifierEnable_get
 * Description:
 *      enable calculate and put identifier into RLDPP packet
 * Input:
 *      none
 * Output:
 *      pEnabled    -   pointer pointed to enabled status
 * Return:
 *      SUCCESS
 * Note:
 *      When identifier is enabled, ASIC will calculate identifier and put it into RLDP 
 *      packet. When ASIC receive a RLDP packet, it will check identifier besides checking
 *      source mac address and protocol.
 */
int32 rtl8309n_rldp_identifierEnable_get(uint32 *pEnabled)
{
    uint32 bitVal;
    
    rtl8309n_regbit_get(4, 19, 14, 14, &bitVal);
    *pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
    
    return SUCCESS;    
}

/* Function Name:
 *      rtl8309n_rldp_protocol_set
 * Description:
 *      set protocol value for RLDP packet
 * Input:
 *      protocol    -   protocol value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 */
int32 rtl8309n_rldp_protocol_set(uint32 protocol)
{

    rtl8309n_reg_set(4, 21, 14, protocol&0xff);

    return SUCCESS;
}

/* Function Name:
 *      rtl8309n_rldp_protocol_get
 * Description:
 *      get protocol value for RLDP packet
 * Input:
 *      protocol    -   protocol value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 */
int32 rtl8309n_rldp_protocol_get(uint32 *pProtocol)
{
    rtl8309n_reg_get(4, 21, 14, pProtocol);

    return SUCCESS;
}


/* Function Name:
 *      rtl8309n_rldp_loopStatus_get
 * Description:
 *      get loop status port mask 
 * Input:
 *      none
 * Output:
 *      pStatus -   pointer pointed to loop status port mask
 * Return:
 *      SUCCESS
 * Note:
 */
int32 rtl8309n_rldp_loopStatus_get(uint32 *pStatus)
{
    rtl8309n_reg_get(4, 20, 14, pStatus);

    return SUCCESS;
}
 
/* ISP MAC*/

/* Function Name:
 *		rtl8309n_isp_enable_set
 * Description:
 * 		Enable ISP MAC function
 * Input:
 *		enabled
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *
 */
int32 rtl8309n_isp_enable_set(uint32 enabled)
{
	rtl8309n_regbit_set(1, 16, 0, 17, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_isp_enable_get
 * Description:
 * 		Get enabled status of ISP MAC function
 * Input:
 *		none
 * Output:
 *		pEnabled	-	pointer point to enabled status 
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_isp_enable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	if (NULL == pEnabled)
		return FAILED;
	
	rtl8309n_regbit_get(1, 16, 0, 17, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;
	
	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_isp_mac_set
 * Description:
 *		Set ISP MAC address
 * Input:
 *		pMac		-	pointer point to ISP MAC address
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_isp_mac_set(uint8 *pMac)
{
	if(NULL == pMac)
		return FAILED;
	
	/*set ISP MAC*/
	/*DATA[15:0],MAC[32:47],MAC[4,5],pMAC[0,1]*/
	rtl8309n_reg_set(1, 18, 17, (pMac[1] << 8) | pMac[0]);
	/*DATA[15:0],MAC[16:31],MAC[2,3],pMAC[2,3]*/
	rtl8309n_reg_set(1, 19, 17, (pMac[3] << 8) | pMac[2]);
	/*DATA[15:0],MAC[0:15],MAC[0,1],pMAC[4,5]*/
	rtl8309n_reg_set(1, 20, 17, (pMac[5] << 8) | pMac[4]);

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_isp_mac_get
 * Description:
 *		Get ISP MAC address
 * Input:
 *		none
 * Output:
 *		pMac		-	pointer point to ISP MAC address
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_isp_mac_get(uint8 *pMac)
{
	uint32 regVal;

	if (NULL == pMac)
		return FAILED;
	
	/*set ISP MAC*/
	/*DATA[15:0],MAC[32:47],MAC[4,5],pMAC[0,1]*/
	rtl8309n_reg_get(1, 18, 17, &regVal);
	pMac[1] = (regVal & 0xFF00) >> 8;
	pMac[0] = regVal & 0XFF;
	/*DATA[15:0],MAC[16:31],MAC[2,3],pMAC[2,3]*/
	rtl8309n_reg_get(1, 19, 17, &regVal);
	pMac[3] = (regVal & 0XFF00) >> 8;
	pMac[2] = regVal & 0xFF;
	/*DATA[15:0],MAC[0:15],MAC[0,1],pMAC[4,5]*/
	rtl8309n_reg_get(1, 20, 17, &regVal);
	pMac[5] = (regVal & 0xFF00) >> 8;
	pMac[4] = regVal & 0XFF;

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_isp_wanPort_set
 * Description:
 *		Set WAN port number
 * Input:
 *		wanport -   wan port number
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The value of wanport should be 0-8, and 9 means no port is set as wan port. 
 */
int32 rtl8309n_isp_wanPort_set(uint32 wanport)
{
	if(wanport > RTL8309N_NOWANPORT)
		return FAILED;
	
	/*set WAN port*/
	rtl8309n_reg_set(1, 17, 17, wanport);

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_isp_wanPort_get
 * Description:
 *		Get WAN port number
 * Input:
 *		none
 * Output:
 *		pWanport		-	pointer point to WAN port number
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
int32 rtl8309n_isp_wanPort_get(uint32 *pWanport)
{
	uint32 regVal;

	if (NULL == pWanport)
		return FAILED;
	
	/*set WAN port*/
	rtl8309n_reg_get(1, 17, 17, &regVal);
	*pWanport = regVal & 0xF;

	return SUCCESS;
}


/* LED */

/* Function Name:
 *		rtl8309n_led_rstBlinkEnable_set
 * Description:
 *		Enable reset blink
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		
 */
int32 rtl8309n_led_rstBlinkEnable_set(uint32 enabled)
{
	/*enable reset blink ability*/
	rtl8309n_regbit_set(1, 16, 4, 14, (RTL8309N_ENABLED == enabled) ? 0 : 1);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_rstBlinkEnable_get
 * Description:
 *		Get ASIC reset blink ability status
 * Input:
 *		none
 * Output:
 *		*pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:		
 */
int32 rtl8309n_led_rstBlinkEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(1, 16, 4, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_DISABLED : RTL8309N_ENABLED;

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_rldpEnable_set
 * Description:
 *		Enable LED RLDP display function
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_led_rldpEnable_set(uint32 enabled)
{
	/*enable rldp led ability*/
	rtl8309n_regbit_set(1, 16, 5, 14, (RTL8309N_ENABLED == enabled) ? 1 : 0);

    return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_rldpEnable_get
 * Description:
 *		Get LED RLDP display ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_led_rldpEnable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(1, 16, 5, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;

}

/* Function Name:
 *		rtl8309n_led_led1Enable_set
 * Description:
 *		Enable LED1 indicator 1
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_led_led1Enable_set(uint32 enabled)
{
	/*enable led1 indicator*/
	rtl8309n_regbit_set(1, 16, 14, 14, (RTL8309N_ENABLED == enabled) ? 1 : 0);

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_led1Enable_get
 * Description:
 *		Get LED indicator 1 status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *
 */
int32 rtl8309n_led_led1Enable_get(uint32 *pEnabled)
{
	uint32 bitVal;

	rtl8309n_regbit_get(1, 16, 14, 14, &bitVal);
	*pEnabled = bitVal ? RTL8309N_ENABLED : RTL8309N_DISABLED;

	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_led_modeBlinkRate_set
 * Description:
 *		Set LED mode and LED blink rate
 * Input:
 *		mode	-	LED mode
 *		rate	-	LED blink rate
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N have 2 group of LED indicators for per port: LED indicator 0 and LED indicator 1.
 *		   Each group have 6 LED modes set by the same global register. LED blink rate could be 32ms and 128 ms.
 *		   So mode could be any value as follows:
 *		        RTL8309N_LED_MODE_0
 *		        RTL8309N_LED_MODE_1
 *		        RTL8309N_LED_MODE_2
 *		        RTL8309N_LED_MODE_3
 *		        RTL8309N_LED_MODE_4
 *		        RTL8309N_LED_MODE_5
 *		 (2)rate could be any value as follows:
 *		        RTL8309N_LED_BLINKRATE_32MS		    -	32ms blink period
 *		        RTL8309N_LED_BLINKRATE_128MS		-	128ms blink period
 */
int32 rtl8309n_led_modeBlinkRate_set(uint32 mode, uint32 rate)
{
	uint32 regVal;

	if (mode >= RTL8309N_LED_MODE_END)
		return FAILED;
	if ((rate != RTL8309N_LED_BLINKRATE_128MS) && ( rate != RTL8309N_LED_BLINKRATE_32MS))
		return FAILED;
	
	/*set normal LED mode*/
	rtl8309n_reg_get(1, 16, 14, &regVal);
	regVal &= ~0x7;
	regVal |= mode;
	/*set blink rate*/
	regVal &= ~0x8;
	switch(rate)
	{
		case RTL8309N_LED_BLINKRATE_32MS:
			regVal |= (0 << 3);
			break;

		case RTL8309N_LED_BLINKRATE_128MS:
			regVal |= (1 << 3);
			break;

		default:
			break;
	}
	
	rtl8309n_reg_set(1, 16, 14, regVal);
	
	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_led_modeBlinkRate_get
 * Description:
 *		Get LED mode and LED blink rate
 * Input:
 *		none
 * Output:
 *		pMode	-	pointer point to mode
 *		pRate	-	pointer point to blink rate
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N have 2 group of LED indicators for per port: LED indicator 0 and LED indicator 1.
 *		   Each group have 6 LED modes set by the same global register. LED blink rate could be 32ms and 128 ms.
 *		   So mode could be any value as follows:
 *		        RTL8309N_LED_MODE_0
 *		        RTL8309N_LED_MODE_1
 *		        RTL8309N_LED_MODE_2
 *		        RTL8309N_LED_MODE_3
 *		        RTL8309N_LED_MODE_4
 *		        RTL8309N_LED_MODE_5
 *		 (2)rate could be any value as follows:
 *		        RTL8309N_LED_BLINKRATE_32MS		    -	32ms blink period
 *		        RTL8309N_LED_BLINKRATE_128MS		-	128ms blink period
 */
int32 rtl8309n_led_modeBlinkRate_get(uint32 *pMode, uint32 *pRate)
{
	uint32 regVal, regValue0;

	if ((NULL == pMode) || (NULL == pRate))
		return FAILED;

	rtl8309n_reg_get(1, 16, 14, &regVal);
	*pMode = regVal & 0x7;

	regValue0 = (regVal & 0x8) >> 3;
	switch(regValue0)
	{
		case 0:
			*pRate = RTL8309N_LED_BLINKRATE_32MS;
			break;

		case 1:
			*pRate = RTL8309N_LED_BLINKRATE_128MS;
			break;

		default:
			break;
	}

	return SUCCESS;
}


/*
 */
int32 rtl8309n_led_rlppRldp_blinkRate_set(uint32 rate)
{
	uint32 bitVal;
    
	if ((rate != RTL8309N_LED_BLINKRATE_400MS) && (rate != RTL8309N_LED_BLINKRATE_800MS))
		return FAILED;

	/*set RLDP/RLPP loop and block blink rate*/
    bitVal = (RTL8309N_LED_BLINKRATE_400MS == rate)? 0 :1;
	rtl8309n_regbit_set(1, 16, 6, 14, bitVal);   

    return SUCCESS;
}

int32 rtl8309n_led_rlppRldp_blinkRate_get(uint32 *pRate)
{
    uint32 bitVal;
    
    if (NULL == pRate)
        return FAILED;

    rtl8309n_regbit_get(1, 16, 6, 14, &bitVal);
	*pRate = (bitVal ? RTL8309N_LED_BLINKRATE_800MS : RTL8309N_LED_BLINKRATE_400MS);

    return SUCCESS;
}

int32 rtl8309n_led_rlppLedMsk_set(uint32 ledmsk)
{
	if (ledmsk > 0xFF)
		return FAILED; 

    rtl8309n_reg_set(1, 19, 14, ledmsk);

    return SUCCESS;
}

int32 rtl8309n_led_rlppLedMsk_get(uint32 *pLedmsk)
{
    if (NULL == pLedmsk)
        return FAILED;

    rtl8309n_reg_get(1, 19, 14, pLedmsk);

    return SUCCESS;
}

int32 rtl8309n_led_buzzerForm_set(uint32 waveform)
{
    if (waveform > 1)
        return FAILED;
    
	rtl8309n_regbit_set(1, 16, 7, 14, (RTL8309N_LED_BUZZER_ACTIVE == waveform) ? 1 : 0);

    return SUCCESS;
}

int32 rtl8309n_led_buzzerForm_get(uint32 *pWaveform)
{
    uint32 bitVal;
    
    if (NULL == pWaveform)
        return FAILED;

    rtl8309n_regbit_get(1, 16, 7, 14, &bitVal);
    *pWaveform = bitVal ? RTL8309N_LED_BUZZER_ACTIVE : RTL8309N_LED_BUZZER_PASIVE;

    return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_led_rtct_set
 * Description:
 *		Set RTCT LED display mode, test time, result time
 * Input:
 *		ledMode		-	led mode, parallel, scan mode
 *		test_time	-	LED time for test
 *		result time	-	LED time for result display
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1) RTCT LED display mode has 2 type: parallel mode and scan mode. The LED dispaly time for RTCT test
 *		process can be set seperately for each type. The dispaly time for displaying RTCT result will be the same
 *		for these 2 types.
 *		(2) ledMode could be any value as follows:
 *		    RTL8309N_LED_RTCTTEST_PARALLEL	-	parallel test mode
 *		    RTL8309N_LED_RTCTTEST_SCAN		-	scan test mode
 *		(3) test_time for parallel test mode could be :	
 *		    RTL8309N_LED_TIME_2SEC 	-	2 seconds
 *		    RTL8309N_LED_TIME_3SEC		-	3 seconds
 *		    RTL8309N_LED_TIME_4SEC 	-	4 seconds
 *		    RTL8309N_LED_TIME_5SEC 	-	5 seconds
 *		(4) test_time for scan test mode could be :	
 *		    RTL8309N_LED_TIME_80MS	-	80 ms
 *		    RTL8309N_LED_TIME_128MS	-	128 ms
 *		    RTL8309N_LED_TIME_256MS	-	256 ms
 *		    RTL8309N_LED_TIME_512MS	-	512 ms
 *		(5) result_time for result diplay:
 *		    RTL8309N_LED_TIME_3SEC	-	3 seconds	
 *		    RTL8309N_LED_TIME_5SEC	-	5 seconds
 */
int32 rtl8309n_led_rtct_set(uint32 led_mode, uint32 test_time, uint32 result_time)
{
	uint32 regVal, testTime, resultTime;
	
	if (led_mode >= RTL8309N_LED_RTCTTEST_END)
		return FAILED;
	if (test_time >= RTL8309N_LED_TIME_END)
		return FAILED;
	if (result_time >= RTL8309N_LED_TIME_END)
		return FAILED;

	/*get test time value*/
	switch(test_time)
	{
		case RTL8309N_LED_TIME_2SEC:
		case RTL8309N_LED_TIME_80MS:
			testTime = 0;
			break;
			
		case RTL8309N_LED_TIME_3SEC:
		case RTL8309N_LED_TIME_128MS:
			testTime = 1;
			break;

		case RTL8309N_LED_TIME_4SEC:
		case RTL8309N_LED_TIME_256MS:
			testTime = 2;
			break;

		case RTL8309N_LED_TIME_5SEC:
		case RTL8309N_LED_TIME_512MS:
			testTime = 3;
			break;

		default:
			break;
			
	}
	/*get result time value*/
	switch(result_time)
	{
		case RTL8309N_LED_TIME_3SEC:
			resultTime = 0;
			break;
			
		case RTL8309N_LED_TIME_5SEC:
			resultTime = 1;
			break;	

		default:
			break;
	}
	/*set RTCT LED display mode*/
	switch(led_mode)
	{
		case  RTL8309N_LED_RTCTTEST_PARALLEL:
			rtl8309n_regbit_set(1, 16, 8, 14, 0);
			/*set time*/
			rtl8309n_reg_get(1, 16, 14, &regVal);
			regVal &= ~(0x3 << 9);
			regVal &= ~(0x1 << 13);
			regVal |= testTime << 9;
			regVal |= resultTime << 13;
			rtl8309n_reg_set(1, 16, 14, regVal);
			break;

		case  RTL8309N_LED_RTCTTEST_SCAN:
			rtl8309n_regbit_set(1, 16, 8, 14, 1);
			/*set time*/
			rtl8309n_reg_get(1, 16, 14, &regVal);
			regVal &= ~(0x3 << 11);
			regVal &= ~(0x1 << 13);
			regVal |= testTime << 11;
			regVal |= resultTime << 13;
			rtl8309n_reg_set(1, 16, 14, regVal);
			break;

		default:
			break;
	}


	return SUCCESS;
}

/* Function Name:
 * 		rtl8309n_led_rtct_get
 * Description:
 *		Get RTCT LED display mode, test time, result time
 * Input:
 *		none
 * Output:
 *		pLedMode		-	led mode, parallel, scan mode
 *		pTest_time	-	LED time for test
 *		pResult_time	-	LED time for result display
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1) RTCT LED display mode has 2 type: parallel mode and scan mode. The LED dispaly time for RTCT test
 *		process can be set seperately for each type. The dispaly time for displaying RTCT result will be the same
 *		for these 2 types.
 *		(2) ledMode could be any value as follows:
 *		    RTL8309N_LED_RTCTTEST_PARALLEL	-	parallel test mode
 *		    RTL8309N_LED_RTCTTEST_SCAN		-	scan test mode
 *		(3) test_time for parallel test mode could be :	
 *		    RTL8309N_LED_TIME_2SEC 	-	2 seconds
 *		    RTL8309N_LED_TIME_3SEC		-	3 seconds
 *		    RTL8309N_LED_TIME_4SEC 	-	4 seconds
 *		    RTL8309N_LED_TIME_5SEC 	-	5 seconds
 *		(4) test_time for scan test mode could be :	
 *		    RTL8309N_LED_TIME_80MS	-	80 ms
 *		    RTL8309N_LED_TIME_128MS	-	128 ms
 *		    RTL8309N_LED_TIME_256MS	-	256 ms
 *		    RTL8309N_LED_TIME_512MS	-	512 ms
 *		(5) result_time for result diplay:
 *		    RTL8309N_LED_TIME_3SEC	-	3 seconds	
 *		    RTL8309N_LED_TIME_5SEC	-	5 seconds
 */
int32 rtl8309n_led_rtct_get(uint32 *pLedMode, uint32 *pTest_time, uint32 *pResult_time)
{
	uint32 regVal;
	uint32 ledmode, testTime, resultTime;
	
	if ((NULL == pLedMode) || (NULL == pTest_time) || (NULL == pResult_time))
		return FAILED;

	/*get LED display mode, test time*/
	rtl8309n_reg_get(1, 16, 14, &regVal);
	ledmode = (regVal >> 8) & 0x1;
	resultTime = (regVal >> 13) & 0x1;
	switch(ledmode)
	{
		case 0:		/*parallel mode*/
		    *pLedMode = RTL8309N_LED_RTCTTEST_PARALLEL;
			testTime = (regVal >> 9) & 0x3;
			switch(testTime)
			{
				case 0:
					*pTest_time = RTL8309N_LED_TIME_2SEC;
					break;

				case 1:
					*pTest_time = RTL8309N_LED_TIME_3SEC;
					break;
					
				case 2:
					*pTest_time = RTL8309N_LED_TIME_4SEC;
					break;
					
				case 3:
					*pTest_time = RTL8309N_LED_TIME_5SEC;
					break;
					
				default:
					break;
			}
			break;

		case 1:/*scan mode*/
			*pLedMode = RTL8309N_LED_RTCTTEST_SCAN;
			testTime = (regVal >> 11) & 0x3;
			switch(testTime)
			{
				case 0:
					*pTest_time = RTL8309N_LED_TIME_80MS;
					break;

				case 1:
					*pTest_time = RTL8309N_LED_TIME_128MS;
					break;
				case 2:
					*pTest_time = RTL8309N_LED_TIME_256MS;
					break;
				case 3:
					*pTest_time = RTL8309N_LED_TIME_512MS;
					break;
					
				default:
					break;
			}			
			break;	

		default:
			break;
	}

	/*get result time*/
    *pResult_time = resultTime ? RTL8309N_LED_TIME_5SEC : RTL8309N_LED_TIME_3SEC;
	
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_cpu_set
 * Description:
 *		Set CPU control LED ability
 * Input:
 *		ledGroup	-	select LED indicator, LED indicator 0 or 1
 *		cpuCtrlMsk	-	select CPU control portmask, ASIC control or CPU control
 *		ledStatus	-	per port LED status, on or off
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N has 2 groups of indicator per port, LED indicator 0 and 1. Each of them can be 
 *		   choosed to controlled by ASIC or CPU. When per port's LED indicator are controlled by CPU,
 *		   the LED can be turned on or off by CPU. RTL8309N only 8 PHY from 0 - 7, so portmask or ledStatus can 
 *		   not exceed 0XFF.
 *		(2)ledGroup could be any value as follows:
 *			RTL8309N_LED_GROUP_0		-	LED indicator 0
 *			RTL8309N_LED_GROUP_1		-	LED indicator 1
 */
int32 rtl8309n_led_cpu_set(uint32 ledGroup, uint32 cpuCtrlMsk, uint32 ledStatus)
{
	uint32 regVal;
	
	if (ledGroup > RTL8309N_LED_GROUP_END)
		return FAILED;
	if ((cpuCtrlMsk > 0XFF) || (ledStatus > 0XFF))
		return FAILED;

	switch(ledGroup)
	{
		case RTL8309N_LED_GROUP_0:
			/*select ASIC control or CPU control*/
			rtl8309n_reg_get(1, 17, 14, &regVal);
			regVal &= ~0xFF;
			regVal |= cpuCtrlMsk;
			rtl8309n_reg_set(1, 17, 14, regVal);
			/*set per port LED status*/
			rtl8309n_reg_get(1, 18, 14, &regVal);
			regVal &= ~0xFF;
			regVal |= ledStatus;
			rtl8309n_reg_set(1, 18, 14, regVal);			
			break;

		case RTL8309N_LED_GROUP_1:
			rtl8309n_reg_get(1, 17, 14, &regVal);
			regVal &= ~(0xFF << 8);
			regVal |= (cpuCtrlMsk << 8);
			rtl8309n_reg_set(1, 17, 14, regVal);
			/*set per port LED status*/
			rtl8309n_reg_get(1, 18, 14, &regVal);
			regVal &=  ~(0xFF << 8);
			regVal |= (ledStatus << 8);
			rtl8309n_reg_set(1, 18, 14, regVal);
			break;

		default:
			break;
	}

	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_led_cpu_get
 * Description:
 *		Get CPU control LED ability
 * Input:
 *		ledGroup	-	select LED indicator, LED indicator 0 or 1
 * Output:
 *		pCpuCtrlMsk	-	select CPU control portmask, ASIC control or CPU control
 *		pLedStatus	-	per port LED status, on or off
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N has 2 groups of indicator per port, LED indicator 0 and 1. Each of them can be 
 *		   choosed to controlled by ASIC or CPU. When per port's LED indicator are controlled by CPU,
 *		   the LED can be turned on or off by CPU. RTL8309N only 8 PHY from 0 - 7, so portmask or ledStatus can 
 *		   not exceed 0XFF.
 *		(2)ledGroup could be any value as follows:
 *			RTL8309N_LED_GROUP_0		-	LED indicator 0
 *			RTL8309N_LED_GROUP_1		-	LED indicator 1
 */
int32 rtl8309n_led_cpu_get(uint32 ledGroup, uint32 *pCpuCtrlMsk, uint32 *pLedStatus)
{
	uint32 regVal;

	if (ledGroup > RTL8309N_LED_GROUP_1)
		return FAILED;
	if ((NULL == pCpuCtrlMsk) || (NULL == pLedStatus))
		return FAILED;

	switch(ledGroup)
	{
		case RTL8309N_LED_GROUP_0:
			rtl8309n_reg_get(1, 17, 14, &regVal);
			*pCpuCtrlMsk = regVal & 0xFF;
			rtl8309n_reg_get(1, 18, 14, &regVal);
			*pLedStatus = regVal & 0xFF;
			break;

		case RTL8309N_LED_GROUP_1:
			rtl8309n_reg_get(1, 17, 14, &regVal);
			*pCpuCtrlMsk = (regVal >> 8) & 0xFF;
			rtl8309n_reg_get(1, 18, 14, &regVal);
			*pLedStatus = (regVal >> 8) & 0xFF;
			break;

		default:
			break;
	}

	return SUCCESS;
}


/* Function Name:
 *		rtl8309n_wkp_enable_set
 * Description:
 *		Set cpu wake up ability
 * Input:
 *		enable	    -	enable or disable wake up ability
 *      pinLevel   -   SLP_WKP pin status, high or low
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 *      (2)pinLevel could be any value as follows:
 *          0   -   High level indicates SLEEP status. Low level indicates WAKEUP status 
 *          1   -   Low level indicates SLEEP status. High level indicates WAKEUP status.
 */
int32 rtl8309n_wkp_enable_set(uint32 enabled, uint32 pinLevel)
{

	if((enabled > RTL8309N_ENABLED) || (pinLevel > 1))
        return FAILED;
     
    rtl8309n_regbit_set(7, 20, 0, 14, enabled);
    rtl8309n_regbit_set(7, 20, 1, 14, pinLevel);    
    
	return SUCCESS;
}


/* Function Name:
 *		rtl8309n_wkp_enable_get
 * Description:
 *		Get cpu wake up ability
 * Input:
 *      none
 * Output:
 *		pEnable	    -	enable or disable wake up ability
 *      pPinLevel   -   SLP_WKP pin status, high or low
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 *      (2)pinStatus could be any value as follows:
 *          0   -   High level indicates SLEEP status. Low level indicates WAKEUP status 
 *          1   -   Low level indicates SLEEP status. High level indicates WAKEUP status.
 */
int32 rtl8309n_wkp_enable_get(uint32 *pEnable, uint32 *pPinLevel)
{

	if((pEnable == NULL) || (pPinLevel == NULL))
        return FAILED;

    rtl8309n_regbit_get(7, 20, 0, 14, pEnable);
    rtl8309n_regbit_get(7, 20, 1, 14, pPinLevel);     
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_wkp_linkchangeEnable_set
 * Description:
 *		Enable link change to wakeup cpu
 * Input:
 *		enable	   -    enable or disable link change to wake up cpu
 *      pmsk       -    the link change status of port in "pmsk" will wake up cpu
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 */
int32 rtl8309n_wkp_linkchngEnable_set(uint32 enabled, uint32 pmsk)
{

	if((enabled > RTL8309N_ENABLED) || (pmsk > RTL8309N_MAX_PORTMASK))
        return FAILED;

    rtl8309n_regbit_set(7, 20, 2, 14, enabled);
    rtl8309n_reg_set(7, 21, 14, pmsk);    
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_wkp_linkchangeEnable_set
 * Description:
 *		Enable link change to wakeup cpu
 * Input:
 *      none
 * Output:
 *		pEnable	   -    enable or disable link change to wake up cpu
 *      pPmsk       -    the link change status of port in "pmsk" will wake up cpu
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 */
int32 rtl8309n_wkp_linkchngEnable_get(uint32 *pEnable, uint32 *pPmsk)
{

	if((pEnable == NULL) || (pPmsk == NULL))
        return FAILED;

    rtl8309n_regbit_get(7, 20, 2, 14, pEnable);
    rtl8309n_reg_get(7, 21, 14, pPmsk);    
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_wkp_linkchangeStatus_get
 * Description:
 *		Get link change status
 * Input:
 *      none
 * Output:
 *      pPmsk   -   the link change status of port
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to get the link change status of port. The link status bit
 *      is read clear.
 */
int32 rtl8309n_wkp_linkchangeStatus_get(uint32 *pPmsk)
{

	if(pPmsk == NULL)
        return FAILED;

    rtl8309n_reg_get(7, 22, 14, pPmsk);    
    
	return SUCCESS;
}


/* Function Name:
 *		rtl8309n_wkp_frameEnable_set
 * Description:
 *		Enable wake up frame to wakeup cpu
 * Input:
 *      enable    -    enable frame to wake up cpu
 *		igrpmsk	  -    packet's ingress port mask
 *      dpm       -    packet's destination port mask
 *      enbrdmul  -    0 : disable the broadcast and multicast packet to wakeup CPU
 *                     1 : enable the broadcast and multicast packet to wakeup CPU
 *      enuni     -    0 : all unicast can wakeup the CPU.;
 *                     1 : not all unicast but only the packet with DA = Switch MAC can wakeup the CPU
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to enable frame to wake up cpu.
 */
int32 rtl8309n_wkp_frameEnable_set(uint32 enable, uint32 igrpmsk, uint32 dpm, uint32 enbrdmul, uint32 enuni)
{

	if((enable > RTL8309N_ENABLED) || (igrpmsk > RTL8309N_MAX_PORTMASK) || (dpm > RTL8309N_MAX_PORTMASK) ||
        (enbrdmul > RTL8309N_ENABLED) || (enuni > RTL8309N_ENABLED))
        return FAILED;

    rtl8309n_regbit_set(7, 20, 3, 14, enable);
    rtl8309n_reg_set(7, 23, 14, igrpmsk);
    rtl8309n_reg_set(7, 24, 14, dpm);
    rtl8309n_regbit_set(7, 25, 0, 14, enbrdmul);
    rtl8309n_regbit_set(7, 25, 1, 14, enuni);
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_wkp_frameEnable_get
 * Description:
 *		Enable wake up frame to wakeup cpu
 * Input:
 *      none
 * Output:
 *      pEnable    -    enable frame to wake up cpu
 *		pIgrpmsk	  -    packet's ingress port mask
 *      pDpm       -    packet's destination port mask
 *      pEnbrdmul  -    0 : disable the broadcast and multicast packet to wakeup CPU
 *                     1 : enable the broadcast and multicast packet to wakeup CPU
 *      pEnuni     -    0 : all unicast can wakeup the CPU.;
 *                     1 : not all unicast but only the packet with DA = Switch MAC can wakeup the CPU
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to enable frame to wake up cpu.
 */
int32 rtl8309n_wkp_frameEnable_get(uint32 *pEnable, uint32 *pIgrpmsk, uint32 *pDpm, uint32 *pEnbrdmul, uint32 *pEnuni)
{

	if((pEnable == NULL) || (pIgrpmsk == NULL) || (pDpm == NULL) ||
        (pEnbrdmul == NULL) || (pEnuni == NULL))
        return FAILED;

    rtl8309n_regbit_get(7, 20, 3, 14, pEnable);
    rtl8309n_reg_get(7, 23, 14, pIgrpmsk);
    rtl8309n_reg_get(7, 24, 14, pDpm);
    rtl8309n_regbit_get(7, 25, 0, 14, pEnbrdmul);
    rtl8309n_regbit_get(7, 25, 1, 14, pEnuni);
    
	return SUCCESS;
}

/* Function Name:
 *		rtl8309n_wkp_frameStatus_get
 * Description:
 *		Get frame status which indicate the valid wakeup frame occurs
 * Input:
 *      none
 * Output:
 *      pStatus   -   the status indicate the valid wakeup frame occurs
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to get the status indicate the valid wakeup frame occurs
 */
int32 rtl8309n_wkp_frameStatus_get(uint32 *pStatus)
{

	if(pStatus == NULL)
        return FAILED;

    rtl8309n_regbit_get(7, 25, 2, 14, pStatus);    
    
	return SUCCESS;
}


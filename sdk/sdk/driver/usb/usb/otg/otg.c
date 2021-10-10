/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * OTG register related.
 *
 * @author Irene Lin
 */
#include "usb/config.h"
#include "usb/usb/host.h"


static void USBEX_OTGRoleChange(struct ehci_hcd* ehci, MMP_UINT16 role)
{
    MMP_UINT32 temp = 0;
    struct otg_regs* otg_regs = &ehci->otg_regs;

    /** Get the current ID */
    AHB_ReadRegister(otg_regs->ctrl_status, &temp);
    if(role == ROLE_HOST) 
    {
        if(CURR_ID(temp) == ID_A) 
        {
            LOG_INFO " Role Change: current ID A => role host \n" LOG_END
            AHB_WriteRegisterMask(otg_regs->ctrl_status, 0x0, A_DEVICE_BUS_DROP);
            AHB_WriteRegisterMask(otg_regs->ctrl_status, A_DEVICE_BUS_REQUEST, A_DEVICE_BUS_REQUEST);
        }
        else 
        {
            LOG_INFO " Role Change: current ID B => role host \n" LOG_END
            AHB_WriteRegisterMask(otg_regs->ctrl_status, 0x0, B_HNP_EN);
            AHB_WriteRegisterMask(otg_regs->ctrl_status, 0x0, B_DSCHRG_VBUS);
        }   
    }
    else 
    {
        if(CURR_ID(temp) == ID_A) 
        {
            LOG_INFO " Role Change: current ID A => role device \n" LOG_END
            AHB_WriteRegisterMask(otg_regs->ctrl_status, 0x0, A_SET_B_HNP_EN);
        }
        else 
        {
            LOG_INFO " Role Change: current ID B => role device \n" LOG_END
            AHB_WriteRegisterMask(otg_regs->ctrl_status, 0x0, B_BUS_REQUEST);
        }
        AHB_WriteRegisterMask(ehci->regs.intr_enable, 0x0, 0x3F); /** disable all host interrupt */
        AHB_WriteRegisterMask(ehci->regs.status, 0x3F, 0x3F); /** write clear */
    }
}


void USBEX_OTGSetup(struct ehci_hcd* ehci)
{
    struct otg_regs* otg_regs = &ehci->otg_regs;
    MMP_UINT32 temp = 0;

    /** Clear the interrupt status */
    AHB_ReadRegister(otg_regs->intr_status, &temp);
    AHB_WriteRegister(otg_regs->intr_status, temp);

    /** Get the current ID */
    AHB_ReadRegister(otg_regs->ctrl_status, &temp);
    if(CURR_ID(temp) == ID_B)
    {   
        LOG_INFO " OTG current ID is B! \n" LOG_END
        /** Init Interrupt */
        AHB_WriteRegisterMask(otg_regs->intr_enable, 0x0, OTGC_INT_A_TYPE);
        AHB_WriteRegisterMask(otg_regs->intr_enable, OTGC_INT_B_TYPE, OTGC_INT_B_TYPE);
        /** Init the Device mode */
        USBEX_OTGRoleChange(ehci, ROLE_DEVICE);
        /** change to B type */
        otg_regs->curr_role = ROLE_DEVICE;
    }
    else 
    {   
        LOG_INFO " OTG current ID is A! \n" LOG_END
        /** Init Interrupt */
        AHB_WriteRegisterMask(otg_regs->intr_enable, 0x0, OTGC_INT_B_TYPE);
        AHB_WriteRegisterMask(otg_regs->intr_enable, OTGC_INT_A_TYPE, OTGC_INT_A_TYPE);
        /** Init the Host mode */
        USBEX_OTGRoleChange(ehci, ROLE_HOST);
        /** change to A type */
        otg_regs->curr_role = ROLE_HOST;
    }
}

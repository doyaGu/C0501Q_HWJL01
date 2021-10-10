
#include "ite/ith.h"


static inline void otg_isr(struct ehci_hcd* ehci)
{
    uint32_t status = ithReadRegA(ehci->otg_regs.intr_status);
	ithWriteRegA(ehci->otg_regs.intr_status, status);
    //if(ehci->hcd.index & USB_HAS_DEVICE_MODE)
    //    ithPrintf(" otg intr: 0x%08X (0x%X)\n", status, ehci->otg_regs.intr_status);
}

static void usb_isr(void* arg)
{
    struct usb_hcd* hcd = (struct usb_hcd*)arg;
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);
    uint32_t status, status1;

    status = ithReadRegA(ehci->global_regs.intr_status);
	ithWriteRegA(ehci->global_regs.intr_status, status);
    if(status & G_HC_INTR)
    {
        status1 = ithReadRegA(ehci->regs.status);
        if(status1 & STS_PCD)
        {
            status1 = ithReadRegA(ehci->regs.port_status[0]);
			ithWriteRegA(ehci->regs.port_status[0], status1);
        }
        ehci_irq(hcd);
    }
    if(status & G_OTG_INTR)
    {
        otg_isr(ehci);
        //ithPrintf(" usb: 0x%X \n", status);
    }
#if defined(CFG_USB_DEVICE)	
    if(status & G_DEVICE_INTR)
        udc_isr();
#endif
}

static inline void usbIntrEnable(void)
{
    uint32_t mask = G_HC_INTR|G_OTG_INTR|G_DEVICE_INTR;
#if defined(CFG_USB_DEVICE)
    uint32_t intr_en = G_HC_INTR|G_OTG_INTR|G_DEVICE_INTR;
#else
    uint32_t intr_en = G_HC_INTR|G_OTG_INTR; // just disable device mode interrupt
#endif
    struct ehci_hcd* ehci = 0;

	/** register interrupt handler to interrupt mgr */
	if(hcd0)
	{
		ehci = hcd_to_ehci(hcd0);
		ithIntrRegisterHandlerIrq(ITH_INTR_USB0, usb_isr, (void*)hcd0);
		ithIntrSetTriggerModeIrq(ITH_INTR_USB0, ITH_INTR_LEVEL);
		ithIntrSetTriggerLevelIrq(ITH_INTR_USB0, ITH_INTR_LOW_FALLING);
		ithIntrEnableIrq(ITH_INTR_USB0);
		/** enable usb0 interrupt */
		ithWriteRegMaskA(ehci->global_regs.intr_mask, ~intr_en, mask);
	}
	if(hcd1)
	{
		ehci = hcd_to_ehci(hcd1);
		/** register interrupt handler to interrupt mgr */
		ithIntrRegisterHandlerIrq(ITH_INTR_USB1, usb_isr, (void*)hcd1);
		ithIntrSetTriggerModeIrq(ITH_INTR_USB1, ITH_INTR_LEVEL);
		ithIntrSetTriggerLevelIrq(ITH_INTR_USB1, ITH_INTR_LOW_FALLING);
		ithIntrEnableIrq(ITH_INTR_USB1);
		/** enable usb1 interrupt */
		ithWriteRegMaskA(ehci->global_regs.intr_mask, ~intr_en, mask);
	}
}



/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * ehci memory related. This file is part of ehci-hcd.c
 *
 * @author Irene Lin
 */

#define EHCI_QH_ALIGN          32
#define EHCI_QH_SIZE           ((sizeof(struct ehci_qh) + (EHCI_QH_ALIGN-1)) & ~(EHCI_QH_ALIGN-1))
#define EHCI_QH_TOTAL_SIZE     (EHCI_QH_NUM*EHCI_QH_SIZE+EHCI_QH_ALIGN)

#define EHCI_QTD_ALIGN         32
#define EHCI_QTD_SIZE          ((sizeof(struct ehci_qtd) + (EHCI_QTD_ALIGN-1)) & ~(EHCI_QTD_ALIGN-1))
#define EHCI_QTD_TOTAL_SIZE    (EHCI_QTD_NUM*EHCI_QTD_SIZE+EHCI_QTD_ALIGN)

#define EHCI_ITD_ALIGN         64  // powei, seems to Faraday's bug. should 64 align
#define EHCI_ITD_SIZE          ((sizeof(struct ehci_itd) + (EHCI_ITD_ALIGN-1)) & ~(EHCI_ITD_ALIGN-1))
#define EHCI_ITD_TOTAL_SIZE    (EHCI_ITD_NUM*EHCI_ITD_SIZE+EHCI_ITD_ALIGN)

#define EHCI_SITD_ALIGN        32
#define EHCI_SITD_SIZE         ((sizeof(struct ehci_sitd) + (EHCI_SITD_ALIGN-1)) & ~(EHCI_SITD_ALIGN-1))
#define EHCI_SITD_TOTAL_SIZE   (EHCI_SITD_NUM*EHCI_SITD_SIZE+EHCI_SITD_ALIGN)

/** periodic table (frame list) */
#define EHCI_PT_ALIGN          0x1000 // 4k
#define EHCI_PT_TOTAL_SIZE(x)  ((x)*sizeof(uint32_t) + EHCI_PT_ALIGN)

/** initialize the periodic schedule frame list */
static void ehci_periodic_init(struct ehci_hcd *ehci)
{
    int i;
    for(i=0; i<ehci->periodic_size; i++)
        ehci->periodic [i] = EHCI_LIST_END(ehci);

    memset((void*)ehci->pshadow, 0, ehci->periodic_size*sizeof(void*));
}

static inline void ehci_qtd_init(struct ehci_hcd *ehci, struct ehci_qtd *qtd,
                  dma_addr_t addr, uint32_t buf_index)
{
    memset (qtd, 0, sizeof *qtd);
    qtd->qtd_dma= addr;
    qtd->hw_token = cpu_to_le32 (QTD_STS_HALT);
    qtd->hw_next = EHCI_LIST_END(ehci);
    qtd->hw_alt_next = EHCI_LIST_END(ehci);
    INIT_LIST_HEAD (&qtd->qtd_list);
    qtd->buf_index = buf_index;
}

static struct ehci_qtd* ehci_qtd_alloc(struct ehci_hcd* ehci)
{
    struct ehci_qtd* qtd = NULL;
    uint32_t i=0;

    ithEnterCritical();
    for(i=0; i<EHCI_QTD_NUM; i++)
    {
        if(ehci->qtd_manage[i] == EHCI_MEM_FREE)
        {
            ehci->qtd_manage[i] = EHCI_MEM_USED;
            qtd = (struct ehci_qtd*)(ehci->qtd_pool+i*EHCI_QTD_SIZE);
            break;
        }
    }

    if(qtd)
        ehci_qtd_init(ehci, qtd, (dma_addr_t)qtd, i);
    //ithPrintf(" alloc qtd: %p \n", qtd);
    ithExitCritical();

    return qtd;
}

static void ehci_qtd_free(struct ehci_hcd* ehci, struct ehci_qtd* qtd)
{
    uint32_t index = 0;

    ithEnterCritical();
    //ithPrintf(" free qtd: %p \n", qtd);

    index = qtd->buf_index;
    if(ehci->qtd_manage[index] == EHCI_MEM_FREE)
        LOG_ERROR " qtd double free!!! index = %d (%p) \n", index, qtd LOG_END

    ehci->qtd_manage[index] = EHCI_MEM_FREE;

    ithExitCritical();
}

static struct ehci_itd* ehci_itd_alloc(struct ehci_hcd* ehci, gfp_t mem_flags, 
	                                       dma_addr_t *itd_dma)
{
    struct ehci_itd* itd = NULL;
    uint32_t i=0;

    ithEnterCritical();
    for(i=0; i<EHCI_ITD_NUM; i++)
    {
        if(ehci->itd_manage[i] == EHCI_MEM_FREE)
        {

            ehci->itd_manage[i] = EHCI_MEM_USED;
            itd = (struct ehci_itd*)(ehci->itd_pool+i*EHCI_ITD_SIZE);

   			break;
        }
    }
	
	if (itd) 
	{
		*itd_dma = (dma_addr_t)itd;
		itd->buf_index = i;
	}
    ithExitCritical();

    return itd;
}

static void ehci_itd_free(struct ehci_hcd* ehci, struct ehci_itd* itd)
{
    uint32_t index = 0;

    ithEnterCritical();
    //ithPrintf(" free itd: %p \n", itd);

    index = itd->buf_index;
    if(ehci->itd_manage[index] == EHCI_MEM_FREE)
        LOG_ERROR " itd double free!!! index = %d (%p) \n", index, itd LOG_END

    ehci->itd_manage[index] = EHCI_MEM_FREE;
    ithExitCritical();
}

static struct ehci_sitd* ehci_sitd_alloc(struct ehci_hcd* ehci, gfp_t mem_flags, dma_addr_t *sitd_dma)
{
    struct ehci_sitd* sitd = NULL;
    uint32_t i=0;

    ithEnterCritical();
    for(i=0; i<EHCI_SITD_NUM; i++)
    {
        if(ehci->sitd_manage[i] == EHCI_MEM_FREE)
        {
            ehci->sitd_manage[i] = EHCI_MEM_USED;
            sitd = (struct ehci_sitd*)(ehci->sitd_pool+i*EHCI_SITD_SIZE);
            break;
        }
    }

	if (sitd)
	{
		*sitd_dma = (dma_addr_t)sitd;
		sitd->buf_index = i;
	}

    //ithPrintf(" alloc sitd: %p \n", sitd);
    ithExitCritical();

    return sitd;
}

static void ehci_sitd_free(struct ehci_hcd* ehci, struct ehci_sitd* sitd)
{
    uint32_t index = 0;

    ithEnterCritical();
    //ithPrintf(" free itd: %p \n", qtd);

    index = sitd->buf_index;
    if(ehci->sitd_manage[index] == EHCI_MEM_FREE)
        LOG_ERROR " sitd double free!!! index = %d (%p) \n", index, sitd LOG_END

    ehci->sitd_manage[index] = EHCI_MEM_FREE;

    ithExitCritical();
}

static void qh_destroy(struct ehci_qh *qh)
{
    struct ehci_hcd *ehci = qh->ehci;
	
    if(qh==NULL) 
		return;

    ithEnterCritical();
    //ithPrintf(" free qh: %p \n", qh);

    if(!list_empty(&qh->qtd_list) || qh->qh_next.ptr)
    {
        LOG_ERROR " unused qh not empty! \n" LOG_END
        while(1);
    }
    if(qh->dummy)
        ehci_qtd_free(ehci, qh->dummy);

    if(ehci->qh_manage[qh->buf_index] == EHCI_MEM_FREE)
        LOG_ERROR " qh double free!!! index = %d \n", qh->buf_index LOG_END

    ehci->qh_manage[qh->buf_index] = EHCI_MEM_FREE;

    ithExitCritical();
}

static struct ehci_qh* ehci_qh_alloc(struct ehci_hcd* ehci)
{
    struct ehci_qh* qh = NULL;
    uint32_t i=0;

    ithEnterCritical();

    for(i=0; i<EHCI_QH_NUM; i++)
    {
        if(ehci->qh_manage[i] == EHCI_MEM_FREE)
        {
            ehci->qh_manage[i] = EHCI_MEM_USED;
            qh = (struct ehci_qh*)(ehci->qh_pool+i*EHCI_QH_SIZE);
            break;
        }
    }

    if(qh)
    {
        memset((void*)qh, 0x0, sizeof(*qh));
        qh->ehci = ehci;
        qh->refcount = 1;
        qh->qh_dma= (dma_addr_t)qh;
        INIT_LIST_HEAD(&qh->qtd_list);
        qh->buf_index = i;

        qh->dummy = ehci_qtd_alloc(ehci);
        if(qh->dummy == NULL)
        {
            LOG_ERROR " no dummy qtd \n" LOG_END
            ehci->qh_manage[i] = EHCI_MEM_FREE;
            qh = NULL;
        }
    //ithPrintf(" alloc qh: %p \n", qh);
    }

    ithExitCritical();

    return qh;
}

static struct ehci_qh* qh_get(struct ehci_qh* qh)
{
    ithEnterCritical();

    qh->refcount++;
    //LOG_DEBUG " qh 0x%08X refcount++ %d \n", qh, qh->refcount LOG_END

    ithExitCritical();

    return qh;
}

static void qh_put(struct ehci_qh* qh)
{
    ithEnterCritical();

    qh->refcount--;
    //LOG_DEBUG " qh 0x%08X refcount-- %d \n", qh, qh->refcount LOG_END

    if(qh->refcount==0)
        qh_destroy(qh);

    ithExitCritical();
}


static int ehci_mem_init(struct ehci_hcd *ehci)
{
    int result = 0;

    /** NOTE!!! These memory will never be released! here I will align the pool base  */

    /* QH for control/bulk/intr transfers */
    ehci->qh_pool = (uint8_t*)itpWTAlloc(EHCI_QH_TOTAL_SIZE);
    if(!ehci->qh_pool)
    {
        result = ERROR_USB_ALLOC_QH_FAIL;
        goto end;
    }
    ehci->qh_pool = (uint8_t*)(((uint32_t)ehci->qh_pool + (EHCI_QH_ALIGN-1)) & ~(EHCI_QH_ALIGN-1));

    /* QTDs for control/bulk/intr transfers */
    ehci->qtd_pool = (uint8_t*)itpWTAlloc(EHCI_QTD_TOTAL_SIZE);
    if(!ehci->qtd_pool)
    {
        result = ERROR_USB_ALLOC_QTD_FAIL;
        goto end;
    }
    ehci->qtd_pool = (uint8_t*)(((uint32_t)ehci->qtd_pool + (EHCI_QTD_ALIGN-1)) & ~(EHCI_QTD_ALIGN-1));

    /* ITDs for control/bulk/intr transfers */
    ehci->itd_pool = (uint8_t*)itpWTAlloc(EHCI_ITD_TOTAL_SIZE);
    if(!ehci->itd_pool)
    {
        result = ERROR_USB_ALLOC_ITD_FAIL;
        goto end;
    }
    ehci->itd_pool = (uint8_t*)(((uint32_t)ehci->itd_pool + (EHCI_ITD_ALIGN-1)) & ~(EHCI_ITD_ALIGN-1));

    /* SITDs for control/bulk/intr transfers */
    ehci->sitd_pool = (uint8_t*)itpWTAlloc(EHCI_SITD_TOTAL_SIZE);
    if(!ehci->sitd_pool)
    {
        result = ERROR_USB_ALLOC_SITD_FAIL;
        goto end;
    }
    ehci->sitd_pool = (uint8_t*)(((uint32_t)ehci->sitd_pool + (EHCI_SITD_ALIGN-1)) & ~(EHCI_SITD_ALIGN-1));
	
    /* Hardware periodic table */
    ehci->periodic = (uint32_t*)itpWTAlloc(EHCI_PT_TOTAL_SIZE(ehci->periodic_size));
    if(!ehci->periodic)
    {
        result = ERROR_USB_ALLOC_PERIODIC_TABLE_FAIL;
        goto end;
    }
    ehci->periodic = (uint32_t*)(((uint32_t)ehci->periodic + (EHCI_PT_ALIGN-1)) & ~(EHCI_PT_ALIGN-1));
    ehci->periodic_addr = (uint32_t)ehci->periodic;

    /* software shadow of hardware table */
    ehci->pshadow = malloc(ehci->periodic_size * sizeof(void*));
    if(!ehci->pshadow)
    {
        result = ERROR_USB_ALLOC_PT_SHADOW_FAIL;
        goto end;
    }

    ehci->async = ehci_qh_alloc(ehci);
    if(!ehci->async)
    {
        result = ERROR_USB_ALLOC_DYMMY_QH_FAIL;
        goto end;
    }
    ehci_periodic_init(ehci);

end:
    if(result)
        LOG_ERROR " ehci_mem_init() rc 0x%08X, driver index 0x%08X \n", result, ehci_to_hcd(ehci)->index LOG_END
    return result;
}



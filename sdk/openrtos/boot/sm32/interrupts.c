/* This file is part of test microkernel for OpenRISC 1000. */
/* (C) 2001 Simon Srot, srot@opencores.org */

#include "support.h"
#include "spr_defs.h"
#include "interrupts.h"
#include "openrtos/FreeRTOS.h"
#include "ite/itp.h"

void itpErrorUndef( void ) __naked;
void itpErrorDataAbort( void ) __naked;
void itpErrorDivideByZero(void) __naked;

/* Interrupt handlers table */
static struct ihnd int_handlers[MAX_INT_HANDLERS];

/* Initialize routine */
int int_init(void) {
	int i;
	
	// initialize Interrupt handler table
	for(i = 0; i < MAX_INT_HANDLERS; i++) {
		int_handlers[i].handler = 0;
		int_handlers[i].arg = 0;
	}
	
	// mask all interrupt
	mtspr(SPR_PICMR, 0x00000000);

	// set OR1200 to accept exceptions (external interrupt enable)
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);

	return 0;
}

/* Add interrupt handler */ 
int int_add(unsigned long vect, void (* handler)(void *), void *arg) {
	if(vect >= MAX_INT_HANDLERS)
		return -1;

	int_handlers[vect].handler = handler;
	int_handlers[vect].arg = arg;

	mtspr(SPR_PICMR, mfspr(SPR_PICMR) | (0x00000001L << vect));

	return 0;
}

/* Disable interrupt */ 
int int_disable(unsigned long vect) {
	if(vect >= MAX_INT_HANDLERS)
		return -1;

	mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(0x00000001L << vect));

	return 0;
}

/* Enable interrupt */ 
int int_enable(unsigned long vect) {
	if(vect >= MAX_INT_HANDLERS)
		return -1;

	mtspr(SPR_PICMR, mfspr(SPR_PICMR) | (0x00000001L << vect));

	return 0;
}

/* Main interrupt handler */
void int_main(void) {
	unsigned long picsr;
	unsigned long i;
	
	// vPortDisableInterrupts();	
	picsr = mfspr(SPR_PICSR);   // process only the interrupts asserted at signal catch, ignore all during process
	i = 0;
	while(i < 32) {
		if((picsr & (0x01L << i)) && (int_handlers[i].handler != 0)) {
			(*int_handlers[i].handler)(int_handlers[i].arg); 
		}
		i++;
	}

	mtspr(SPR_PICSR, 0);	// clear interrupt status: all modules have level interrupts, which have to be cleared by software,
                          	// thus this is safe, since non processed interrupts will get re-asserted soon enough

	// vPortEnableInterrupts();	
}

// Dummy or32 except vectors
static void stall(void) {
	while(1);
}

static void buserr_except(void) {
    int i;
	unsigned long epcr = mfspr(SPR_EPCR_BASE);
	unsigned long eear = mfspr(SPR_EEAR_BASE);

	ithPrintf("buserr_except\n");
	ithPrintf("EPCR:0x%X\n", epcr);
	ithPrintf("EEAR:0x%X\n", eear);

    itpErrorUndef();
}

static void dpf_except(void) {
	ithPrintf("dpf_except\n");
    itpErrorUndef();
}

static void ipf_except(void) {
	ithPrintf("ipf_except\n");
    itpErrorUndef();
}

static void align_except(void) {
    int i; 
	unsigned long epcr = mfspr(SPR_EPCR_BASE);
	unsigned long eear = mfspr(SPR_EEAR_BASE);

	ithPrintf("align_except\n");
	ithPrintf("EPCR:0x%X\n", epcr);
	ithPrintf("EEAR:0x%X\n", eear);
	
    itpErrorDataAbort();
}

static void illegal_except(void) {
	ithPrintf("illegal_except\n");
    itpErrorUndef();
}

static void dtlbmiss_except(void) {
	ithPrintf("dtlbmiss_except\n");
    itpErrorUndef();
}

static void itlbmiss_except(void) {
	ithPrintf("itlbmiss_except\n");
    itpErrorUndef();
}

static void range_except(void) {
	ithPrintf("range_except\n");
    itpErrorUndef();
}

static void res1_except(void) {
	ithPrintf("res1_except\n");
    itpErrorUndef();
}

static void trap_except(void) {
    uint32_t* stack = (uint32_t*)mfspr(SPR_EPCR_BASE);
    uint32_t inst = *stack;

	ithPrintf("trap_except\n");

    if ((((uint32_t)stack) & 0xf0000000) == 0)
    {
        if (inst == 0x2100000a)
        {
            itpErrorDivideByZero();
        }
        else if (inst == 0x2100000f)
        {
            ithPrintf("\n[EXCEPTION] software breakpoint...\n");
            for( ;; );
        }
        else if ((inst & 0xff000000) == 0x21000000)
        {
            ithPrintf("\n[EXCEPTION] trap(%d)...\n", (inst & 0xf));
            for( ;; );
        }
        else
        {
            ithPrintf("\n[EXCEPTION] unknown trap type...\n");
            for( ;; );
        }
    }
    else
    {
        itpErrorUndef();
    }
}

static void res2_except(void) {
	ithPrintf("res2_except\n");
    itpErrorUndef();
}

void misc_int_handler(int arg) {
	switch(arg) {
	case 0x200: { buserr_except(); 	 break; }
	case 0x300: { dpf_except();		 break; }
	case 0x400: { ipf_except(); 	 break; }
	case 0x600: { align_except(); 	 break; }
	case 0x700: { illegal_except();	 break; }
	case 0x900: { dtlbmiss_except(); break; }
	case 0xa00: { itlbmiss_except(); break; }
	case 0xb00: { range_except(); 	 break; }
	case 0xd00: { res1_except(); 	 break; }
	case 0xe00: { trap_except(); 	 break; }
	case 0xf00: { res2_except(); 	 break; }
	default: { break; }
	}
}

static void syscall_enter_critical(void) {
	unsigned int exception_sr = mfspr(SPR_ESR_BASE);
	exception_sr &= (~SPR_SR_IEE);		// disable all external interrupt
	exception_sr &= (~SPR_SR_TEE);		// disable tick timer interrupt

	mtspr(SPR_ESR_BASE, exception_sr);
}

static void syscall_exit_critical(void) {
	unsigned int exception_sr = mfspr(SPR_ESR_BASE);
	exception_sr |= SPR_SR_IEE;		// enable all external interrupt
	exception_sr |= SPR_SR_TEE;		// enable tick timer interrupt

	mtspr(SPR_ESR_BASE, exception_sr);
}

void syscall_except(int id) {
	if(id == 0x0FCC) {
		vTaskSwitchContext();
	} else if(id == 0x0FCE) {
		syscall_enter_critical();
	} else if(id == 0x0FCF) {
		syscall_exit_critical();
	} else {
		ithPrintf("0x%X", id);
		ithPrintf(" syscall is not impelmented yet....\n");
	}
}


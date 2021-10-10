/***************************************************************************
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * Codecs Startup Code
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/
#include "risc1.h"
//#include "mmio.h"

#if defined (CFG_CHIP_REV_A0)
/* For MMIO interface */
#  define MMIO_ADDR                 0xC0000000
#elif defined (CFG_CHIP_REV_A1)
  #  define MMIO_ADDR               0xC0200000
#else
 #  define MMIO_ADDR                0xC0200000
#endif

char __tm_info[] = "RISC1 Plug-Ins (c) 2008 SMedia Tech. Corp.";

// Put the *ci to the data section instead of bss section. Cause the
// bss section will clear in the codec_start function, it will
// reset the *ci to NULL.
struct _risc_api *ci __attribute__ ((section (".data"))) = 0;

extern unsigned char risc1_start_addr[];
extern unsigned char risc1_end_addr[];
extern unsigned char risc1_bss_start[];
extern unsigned char risc1_bss_end[];

//int codec_start(struct _codec_api *api) __attribute__ ( ( naked ) );
int risc1_start(void) __attribute__ ( ( naked ) );

//extern AUDIO_RISC1_STREAM* gpAudioStream;
struct _risc1_header __header __attribute__ ((section (".risc1_header"))) = {
    0, 0, 0,
    (unsigned char*)risc1_start_addr, (unsigned char*)risc1_end_addr,
    risc1_start, 0
};

void MMIO_Write(unsigned short addr, unsigned short data)
{
    *(volatile unsigned short *) (MMIO_ADDR + addr) = data;
}

unsigned int MMIO_Read(unsigned short addr)
{
    return *(volatile unsigned short *) (MMIO_ADDR + addr);
}
// startup entry
//int codec_start(struct _codec_api *api)
int risc1_start(void)
{
    // Make sure the ci is not in .bss section. The .bss section will
    // clear later.
    //ci = api;

    asm volatile ("l.movhi r1,hi(_stack)");
    asm volatile ("l.ori   r1,r1,lo(_stack)");
    asm volatile ("l.addi  r2, r1, 0x0");
    asm volatile ("l.nop");
    
    ic_enable(); 
    dc_enable();

    // Clear BSS section of CODEC. Be carefull, it will clear all of
    // global variable which un-initialized or initialize with zero's
    // variable.
    #if 1
    {
        int *ptr = (int*)risc1_bss_start;
    do {
        *ptr++ = 0;
    } while((int)ptr <= (int)risc1_bss_end);
    }
    #endif

    //MMIO_Write(AUDIO_DECODER_START_FALG, 1);       

    main();

    // Never return by codec main functions.
    //taskSOFTWARE_BREAKPOINT();
    while(1) ;

    return 0;
}


// dummy __main function, do not remove it.
void __main(void) { /* dummy */ }
void free(void) { /* dummy */ }

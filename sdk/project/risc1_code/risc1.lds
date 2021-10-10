/*
 * SMedia Tech. Corp. 2008
 * default link script
 *
 * defined symbol: CODEC_START_ADDR
 *
 * Notes: it is not a standard link script, must use gcc preprocessor
 *        to generate the standard link script.
 */

#include "risc1_defs.h"

// sm32 is predefined by gcc compiler, must undefine it.
#undef  sm32

// begin of link script
ENTRY(_risc1_start)
OUTPUT_ARCH(sm32)
GROUP(-lc -lgcc)
SEARCH_DIR(.)

MEMORY
{
    PLUGIN_RAM : ORIGIN = CODEC_START_ADDR, LENGTH = 900k //CODEC_SIZE
}

SECTIONS
{
    .risc1_header :
    {
        _risc1_start_addr = .;
        risc1_start_addr  = .;
        *(risc1_crt0.o)
        KEEP(*(.risc1_header))
        . = ALIGN(4);
    } > PLUGIN_RAM

    .text ALIGN(4) :
    {
        *(.text*)
    } > PLUGIN_RAM

    .data ALIGN(4) :
    {
        *(.rodata*)
        *(.data*)
        . = ALIGN(4);
    } > PLUGIN_RAM

    /DISCARD/ :
    {
        *(.eh_frame)
    }

    .bss ALIGN(4) (NOLOAD):
    {
        risc1_bss_start = .;
        _risc1_bss_start = .;
        *(.bss* .sbss*)
        *(COMMON)
        . = ALIGN(0x4);
        _risc1_bss_end = .;
        risc1_bss_end = .;
        _risc1_end_addr = .;
        risc1_end_addr = .;
    } > PLUGIN_RAM

    /* Special trick to avoid a linker error when no other sections are
       left after garbage collection (codec plugins not for this platform) */
    .comment 0 :
    {
        KEEP(*(.comment))
    }
}


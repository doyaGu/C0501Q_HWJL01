/*
 * SMedia Tech. Corp. 2008
 * default link script
 *
 * defined symbol: RISC1_SIZE
  *
 * Notes: it is not a standard link script, must use gcc preprocessor
 *        to generate the standard link script.
 */

#include "codecs_defs.h"

// sm32 is predefined by gcc compiler, must undefine it.
#undef  sm32

// define stack size
#define STACK_SIZE              80K

// begin of link script
OUTPUT_FORMAT("elf32-sm32", "elf32-sm32", "elf32-sm32")
OUTPUT_ARCH(sm32)
_start = 0x100;
ENTRY(_start)

GROUP(-lc -lm -lgcc)

MEMORY
{
  except : ORIGIN = 0x00000000, LENGTH = 0x1000
  ram    : ORIGIN = 0x1000, LENGTH = RISC1_SIZE - 0x1000
  ram2   : ORIGIN = RISC1_SIZE , LENGTH = RISC2_SIZE
}

SECTIONS
{
  .reset :
  {
    *(.reset)
  } > except

  .risc1_header :
  {
    _risc1_start_addr = .;
    risc1_start_addr  = .;
    KEEP(*(.codecs_header))

    // codec stack for audio codec on processor 2
    _risc1_stack = .;
  } > ram

  .text :
  {
    __text_start = .;
    *(.text .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    *(.gnu.warning)
    __text_end = .;
  } > ram

  .data ALIGN(0x4) :
  {
    __CTOR_LIST__ = .;
    ___CTOR_LIST__ = .;
    /* In 3.4.4, libgcc2.c actually required -1 here, rather than the count
       , as in ld documentation
    */
    LONG(-1)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;

    __DTOR_LIST__ = .;
    ___DTOR_LIST__ = .;
    LONG(-1)
    *(.dtors)
    LONG(0)
    __DTOR_END__ = .;

    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)

    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
    *(.data .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    . = ALIGN(32 / 8);
  } > ram

  . = ALIGN(32 / 8);
  __bss_start = .;
  _bss_start = .;
  risc1_bss_start = .;
  .bss ALIGN(0x4) :
  {
   *(.dynbss)
   *(.sbss .sbss.* .gnu.linkonce.b.*)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
   . = ALIGN(32 / 8);
  } > ram
  _end = .;
  _bss_end = .;
  risc1_bss_end = .;
  
  PROVIDE (end = .);


  .stack ALIGN(4) (NOLOAD) :
    {
    __stack_start = .;
    . += STACK_SIZE;
    __stack_end = .;
    _stack      = .;
    __end       = .;

    _risc1_end_addr = .;
    risc1_end_addr  = .;
  } > ram
  
  .risc2_header :
  {
    _risc2_start_addr = .;
    risc2_start_addr  = .;

    _risc2_end_addr = .;
    risc2_end_addr  = .;
  } > ram2

  /DISCARD/ : { *(.note.GNU-stack) }
}

ENTRY(reset)
SEARCH_DIR(.)

#define RESERVE_SHARE_INFO_SIZE 256
#define SRAM_SIZE               16*1024
MEMORY
{
        ram : ORIGIN = 0, LENGTH = SRAM_SIZE - RESERVE_SHARE_INFO_SIZE
}

SECTIONS
{
    .reset :
    {
        *(.reset)
    } > ram

    .text :
    {
        _dst_beg = .;
        *(.text .stub .text.* .gnu.linkonce.t.*)
    } > ram

    .data ALIGN(0x4) :
    {
        *(.data .rodata .rodata.* .ctors .dtors .gnu.linkonce.r.* .gcc_except_table)
        *(COMMON)
        . = ALIGN(0x4);
        _dst_end = .;
    } > ram

    .bss ALIGN(0x4) :
    {
        _bss_start = .;
        *(.bss .sbss)
        *(COMMON)
        _bss_end = .;
    } > ram

    .stack ALIGN(0x10) (NOLOAD) :
    {
        . += 1K;
        _stack = .;
        __end  = .;
    } > ram
}


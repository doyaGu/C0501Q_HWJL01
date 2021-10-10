add_definitions(
    -DHAVE_MMAP=0
    -DINTERNAL_NEWLIB
    -DMORECORE_CANNOT_TRIM
    -DUSE_LOCKS=1
    )

if (DEFINED CFG_GCC_LTO)
    add_definitions("-fno-lto")
endif ()

add_library(malloc STATIC
    calloc.c
    malign.c
    malloc.c
    mallocr.c
    msize.c
    mstats.c
    mtrim.c
    realloc.c
    valloc.c
    )

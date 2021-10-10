add_definitions(
    -DUSE_ALLOCATOR=1
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

if ($ENV{CFG_PLATFORM} STREQUAL win32)
    add_definitions(
        -DSTRUCT_MALLINFO_DECLARED=1
        )
endif()

add_library(nedmalloc STATIC
    malloc.c.h
    nedmalloc.c
    nedmalloc.h
    )

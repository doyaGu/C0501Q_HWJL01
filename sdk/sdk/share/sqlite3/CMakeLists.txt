add_definitions(
    -DSQLITE_MAX_MMAP_SIZE=0
    #-DSQLITE_NO_SYNC
    -DSQLITE_OMIT_RANDOMNESS
    #-DSQLITE_OMIT_SHARED_CACHE
    -DSQLITE_OMIT_WAL=1
    -DSQLITE_OS_UNIX=1
    #-DSQLITE_THREADSAFE=0
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DSQLITE_DEBUG
        -DSQLITE_DEBUG_OS_TRACE=1
        -DSQLITE_ENABLE_IOTRACE
        -DSQLITE_FORCE_OS_TRACE
    )
endif()

add_library(sqlite3 STATIC
    sqlite3-all.c
    sqlite3.h
)

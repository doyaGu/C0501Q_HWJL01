include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/async_file
    )

add_library(async_file STATIC
        file.c
        init.c
        mutex.c
        msgq.c
        string.c
        thread.c
        error.c
        heap.c
    )

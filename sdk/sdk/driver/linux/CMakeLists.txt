
include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    )

add_library(linux STATIC
    nls.c
    completion.c
    tasklet.c
    timer.c
    )


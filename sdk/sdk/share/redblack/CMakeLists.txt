include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/redblack
    )
    
add_library(redblack STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/redblack/redblack.h
    redblack.c
    )


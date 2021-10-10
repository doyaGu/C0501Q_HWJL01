include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include
    ${PROJECT_SOURCE_DIR}/sdk/include/iniparser
    )
    
add_library(iniparser STATIC
    dictionary.c
    iniparser.c
    )

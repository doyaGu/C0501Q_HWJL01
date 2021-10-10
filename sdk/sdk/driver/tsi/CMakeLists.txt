   
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
    .
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/tsi
    )

ADD_LIBRARY(tsi STATIC
    mmp_tsi.c
    ${PROJECT_SOURCE_DIR}/sdk/include/tsi/mmp_tsi.h
    )


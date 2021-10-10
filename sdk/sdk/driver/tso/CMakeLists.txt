if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
    .
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/tso
    )

ADD_LIBRARY(tso STATIC
    tso.c
    ${PROJECT_SOURCE_DIR}/sdk/include/tso/mmp_tso.h
    )

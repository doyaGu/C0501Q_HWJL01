add_definitions(
    -DPING_USE_SOCKETS=0
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DLWIP_DEBUG
        )
endif()

add_library(ping STATIC
    ping.c
    ping.h
    )

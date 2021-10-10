if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DLWIP_DEBUG
        -DSNTP_DEBUG=LWIP_DBG_ON
        )
endif()

add_library(sntp STATIC
    sntp.c
    sntp.h
    )

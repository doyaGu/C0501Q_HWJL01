include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/mac/inc/
    )

add_library(mac STATIC
    mii.c
    )


if (CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910 OR CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/mac/mac100/
    )

    add_library(mac STATIC
    mac100/ite_mac100.c
        )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/driver/mac/gmac/
    )

    add_library(mac STATIC
        gmac/skb.c
        gmac/ite_gmac.c
        )
endif()
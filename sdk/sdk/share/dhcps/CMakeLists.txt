add_definitions(
    -DCFG_DHCPS_ADDR_COUNT=${CFG_DHCPS_ADDR_COUNT}
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/
)

add_library(dhcps STATIC
    dhcps.c
    dhcps.h
    )

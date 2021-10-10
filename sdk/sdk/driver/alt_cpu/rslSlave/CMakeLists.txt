if (CFG_CHIP_FAMILY EQUAL 9850)
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu_it9850/lib/sm32/rslSlave.hex
        DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/rslSlave/.
    )
endif()

if (DEFINED CFG_RSL_SLAVE AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(rslSlave STATIC
        rslSlaveDevice.c
    )
endif()
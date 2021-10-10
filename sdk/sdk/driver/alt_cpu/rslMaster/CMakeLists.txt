if (CFG_CHIP_FAMILY EQUAL 9850)
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu_it9850/lib/sm32/rslMaster.hex
        DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/rslMaster/.
    )
endif()

if (DEFINED CFG_RSL_MASTER AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(rslMaster STATIC
        rslMasterDevice.c
    )
endif()
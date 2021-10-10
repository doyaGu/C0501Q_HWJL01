if (CFG_CHIP_FAMILY EQUAL 9850)
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu_it9850/lib/sm32/swPwm.hex
        DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/swPwm/.
    )
endif()

if (DEFINED CFG_SW_PWM AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(swPwm STATIC
        swPwmDevice.c
    )
endif()
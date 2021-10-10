if (NOT $ENV{CFG_PLATFORM} STREQUAL win32)
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${CMAKE_BINARY_DIR}/lib/sm32
)

if (CFG_CHIP_FAMILY EQUAL 9850)
    add_library(risc STATIC
        it9850/ite_risc.c
        it9850/risc2_ctrl.c
    )
elseif (CFG_CHIP_FAMILY EQUAL 9910)
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_ex_it9910/lib/sm32/wiegand.hex
        DESTINATION ${CMAKE_BINARY_DIR}/lib/sm32
        FILES_MATCHING PATTERN "*.hex"
    )

    add_library(risc STATIC
        it9910/ite_risc.c
        it9910/risc2_ctrl.c
    )
else ()
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/risc1_code/lib/sm32/risc_startup.hex
        DESTINATION ${CMAKE_BINARY_DIR}/lib/sm32
        FILES_MATCHING PATTERN "*.hex"
    )

    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_ex/lib/sm32/wiegand.hex
        DESTINATION ${CMAKE_BINARY_DIR}/lib/sm32
        FILES_MATCHING PATTERN "*.hex"
    )

    add_library(risc STATIC
        it9070/ite_risc.c
        it9070/risc2_ctrl.c
    )
endif()
else ()
    add_library(risc STATIC
        ite_risc_null.c
    )
endif()
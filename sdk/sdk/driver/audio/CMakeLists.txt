include_directories(
    ${CMAKE_BINARY_DIR}/lib/sm32
)

if (DEFINED CFG_AUDIO_ENABLE AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(audio STATIC
        ite_audio.c
    )

    if (DEFINED CFG_RISC_TS_DEMUX_PLUGIN)
        add_library(audio STATIC
            ite_risc_ts_demux.c
        )
    endif()

    if (CFG_CHIP_FAMILY EQUAL 9910)
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_it9910/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    elseif (CFG_CHIP_FAMILY EQUAL 9850)
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_it9850/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    else()
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    endif()

    if (CFG_CHIP_PKG_IT9910)
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_ex_it9910/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    elseif (CFG_CHIP_FAMILY EQUAL 9850)
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_ex_it9850/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    else()
        file(COPY
            ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_ex/lib/sm32
            DESTINATION ${CMAKE_BINARY_DIR}/lib
            FILES_MATCHING PATTERN "*.hex"
        )
    endif()

else()
    add_library(audio STATIC
        ite_audio_null.c
    )
endif()


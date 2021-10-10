INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(WMA_SRCS
    asf.c
    bitstream.c
    divdi3.c
    fft.c
    freqinfo.c
    main.c
    mdct.c
    wmadec.c
    wmafixed.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(wmadecode.plugin
        ${WMA_SRCS}
    )

    if (DEFINED CFG_ROM_COMPRESS)
        set(args ${args} -z -b 512K)

    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET wmadecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/wmadecode.plugin ${LIBRARY_OUTPUT_PATH}/wma.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/wmadecode.plugin ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs ${LIBRARY_OUTPUT_PATH}/wma.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET wmadecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/wmadecode.plugin ${LIBRARY_OUTPUT_PATH}/wma.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/wmadecode.plugin ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/wma.rawcodecs ${LIBRARY_OUTPUT_PATH}/wma.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET wmadecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .codec_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/wmadecode.plugin ${LIBRARY_OUTPUT_PATH}/wma.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wma.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)    
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(wmadecode STATIC
        ${WMA_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


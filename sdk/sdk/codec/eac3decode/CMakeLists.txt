INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(AC3_SRCS
    ac3dec.c
    ac3tab.c
    aht.c
    bitalloc.c
    block.c
    cpl.c
    dnmx.c
    dolby_imdct.c
    frame.c
    imdct.c
    main.c
    mantissa.c
    math.c    
    spx.c
    tpnp.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(eac3decode.plugin
        ${AC3_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)
    set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET eac3decode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/eac3decode.plugin ${LIBRARY_OUTPUT_PATH}/eac3.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/eac3decode.plugin ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs ${LIBRARY_OUTPUT_PATH}/eac3.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET eac3decode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/eac3decode.plugin ${LIBRARY_OUTPUT_PATH}/eac3.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/eac3decode.plugin ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/eac3.rawcodecs ${LIBRARY_OUTPUT_PATH}/eac3.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    ADD_CUSTOM_COMMAND(
        TARGET eac3decode.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/eac3decode.plugin ${LIBRARY_OUTPUT_PATH}/eac3.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/eac3.codecs
    )
  endif (DEFINED CFG_ROM_COMPRESS)    
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(eac3decode STATIC
        ${AC3_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


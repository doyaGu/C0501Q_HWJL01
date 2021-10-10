INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(AC3_SRCS
    bit_allocate.c
    bitstream.c
    downmix.c
    imdct.c
    parse.c
    main.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(ac3decode.plugin
        ${AC3_SRCS}
    )

    ADD_CUSTOM_COMMAND(
        TARGET ac3decode.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/ac3decode.plugin ${LIBRARY_OUTPUT_PATH}/ac3.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/ac3.codecs
    )
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(ac3decode STATIC
        ${AC3_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


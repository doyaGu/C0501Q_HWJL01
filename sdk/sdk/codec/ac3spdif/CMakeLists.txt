INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(AC3_SPDIF_SRCS
    main.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(ac3spdif.plugin
        ${AC3_SPDIF_SRCS}
    )

    ADD_CUSTOM_COMMAND(
        TARGET ac3spdif.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/ac3spdif.plugin ${LIBRARY_OUTPUT_PATH}/ac3spdif.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/ac3spdif.codecs
    )
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(ac3spdif STATIC
        ${AC3_SPDIF_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


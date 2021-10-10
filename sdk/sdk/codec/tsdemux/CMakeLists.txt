INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(TS_SRCS
    def.h
    main.c
    pes_packet.c
    pes_packet.h
    plugin.h
    queue_mgr.c
    queue_mgr.h
    share_info.c
    share_info.h
    string.c
    string.h
    ts.c
    ts.h
    tsi.c
    tsi.h
    type.h
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(tsdemux.plugin
        ${TS_SRCS}
    )

    ADD_CUSTOM_COMMAND(
        TARGET tsdemux.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/tsdemux.plugin ${LIBRARY_OUTPUT_PATH}/ts.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/ts.codecs
    )
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(tsdemux STATIC
        ${TS_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


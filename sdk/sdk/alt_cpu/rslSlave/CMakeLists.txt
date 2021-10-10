INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(RSL_SLAVE_SRCS
    main.c
    )

ADD_EXECUTABLE(rslSlave.plugin
    ${RSL_SLAVE_SRCS}
)

ADD_CUSTOM_COMMAND(
    TARGET rslSlave.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/rslSlave.plugin ${LIBRARY_OUTPUT_PATH}/rslSlave.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/rslSlave.plugin > ${LIBRARY_OUTPUT_PATH}/rslSlave.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/rslSlave.plugin > ${CMAKE_CURRENT_BINARY_DIR}/rslSlave.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/rslSlave.raw
)
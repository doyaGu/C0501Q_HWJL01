INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(RSL_MASTER_SRCS
    main.c
    )

ADD_EXECUTABLE(rslMaster.plugin
    ${RSL_MASTER_SRCS}
)

ADD_CUSTOM_COMMAND(
    TARGET rslMaster.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/rslMaster.plugin ${LIBRARY_OUTPUT_PATH}/rslMaster.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/rslMaster.plugin > ${LIBRARY_OUTPUT_PATH}/rslMaster.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/rslMaster.plugin > ${CMAKE_CURRENT_BINARY_DIR}/rslMaster.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/rslMaster.raw
)
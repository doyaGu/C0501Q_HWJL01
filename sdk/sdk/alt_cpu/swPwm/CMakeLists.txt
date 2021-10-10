INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(SW_PWM_SRC
    main.c
    )

ADD_EXECUTABLE(swPwm.plugin
    ${SW_PWM_SRC}
)

ADD_CUSTOM_COMMAND(
    TARGET swPwm.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
            -O binary ${CMAKE_CURRENT_BINARY_DIR}/swPwm.plugin ${LIBRARY_OUTPUT_PATH}/swPwm.raw
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/swPwm.plugin > ${LIBRARY_OUTPUT_PATH}/swPwm.dbg
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/swPwm.plugin > ${CMAKE_CURRENT_BINARY_DIR}/swPwm.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/swPwm.raw
)
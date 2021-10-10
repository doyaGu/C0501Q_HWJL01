INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

add_definitions(
    -DHAVE_CONFIG_H
    )

SET(WIEGAND_SRCS
    main.c
    wiegand.c
    swuart.c
    ith_clock.c
    )

IF(ENABLE_WIEGAND_PLUGIN)
    ADD_EXECUTABLE(wiegand.plugin
        ${WIEGAND_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)
		set(args ${args} -z -b 512K)
    ADD_CUSTOM_COMMAND(
        TARGET wiegand.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                #-O binary ${CMAKE_CURRENT_BINARY_DIR}/wiegand.plugin ${LIBRARY_OUTPUT_PATH}/wiegand.codecs
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/wiegand.plugin ${LIBRARY_OUTPUT_PATH}/wiegand.rawcodecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wiegand.rawcodecs
        COMMAND mkrom
        ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/wiegand.rawcodecs ${LIBRARY_OUTPUT_PATH}/wiegand.codecs
    )
  else (DEFINED CFG_ROM_COMPRESS)
    ADD_CUSTOM_COMMAND(
        TARGET wiegand.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/wiegand.plugin ${LIBRARY_OUTPUT_PATH}/wiegand.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wiegand.codecs
    )
  endif (DEFINED CFG_ROM_COMPRESS)   
ELSE(ENABLE_WIEGAND_PLUGIN)
    ADD_LIBRARY(wiegand STATIC
        ${WIEGAND_SRCS}
    )
ENDIF(ENABLE_WIEGAND_PLUGIN)

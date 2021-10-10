INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include
    )
SET(FLAC_SRCS
    crc.c
    flac.c
    flacdata.c
    flacdec.c
    main.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(flacdecode.plugin
        ${FLAC_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)
	  set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET flacdecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/flacdecode.plugin ${LIBRARY_OUTPUT_PATH}/flac.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/flacdecode.plugin ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs ${LIBRARY_OUTPUT_PATH}/flac.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET flacdecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/flacdecode.plugin ${LIBRARY_OUTPUT_PATH}/flac.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/flacdecode.plugin ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/flac.rawcodecs ${LIBRARY_OUTPUT_PATH}/flac.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    ADD_CUSTOM_COMMAND(
        TARGET flacdecode.plugin
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
                -O binary ${CMAKE_CURRENT_BINARY_DIR}/flacdecode.plugin ${LIBRARY_OUTPUT_PATH}/flac.codecs
        COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/flac.codecs
    )
  endif (DEFINED CFG_ROM_COMPRESS)   
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(flacdecode STATIC
        ${FLAC_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


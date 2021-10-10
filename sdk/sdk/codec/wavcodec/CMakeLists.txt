INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include    
    )

SET(WAV_SRCS
    main.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(wave.plugin
        ${WAV_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)    
	  set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET wave.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/wave.plugin ${LIBRARY_OUTPUT_PATH}/wave.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/wave.plugin ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs ${LIBRARY_OUTPUT_PATH}/wave.codecs                
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET wave.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/wave.plugin ${LIBRARY_OUTPUT_PATH}/wave.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/wave.plugin ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/wave.rawcodecs ${LIBRARY_OUTPUT_PATH}/wave.codecs                
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET wave.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/wave.plugin ${LIBRARY_OUTPUT_PATH}/wave.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/wave.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(wavdecode STATIC
        ${WAV_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


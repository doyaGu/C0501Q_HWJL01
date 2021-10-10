INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include    
    )

SET(MP3_SRCS
    bit.c
    drc.c
    hufftabs.c
    mp3dec.c
    scalfact.c
    equalizer.c
    imdct.c
    mp3decode.c
    stproc.c
    dist10.c
    freqinfo.c
    io.c
    mp3tabs.c
    trigtabs.c
    dqchan.c
    huffman.c
    layer12.c
    reverb.c
    voiceoff.c
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(mp3decode.plugin
        ${MP3_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)    
	  set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET mp3decode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/mp3decode.plugin ${LIBRARY_OUTPUT_PATH}/mp3.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/mp3decode.plugin ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs ${LIBRARY_OUTPUT_PATH}/mp3.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET mp3decode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/mp3decode.plugin ${LIBRARY_OUTPUT_PATH}/mp3.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/mp3decode.plugin ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/mp3.rawcodecs ${LIBRARY_OUTPUT_PATH}/mp3.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (DEFINED CFG_CHIP_PKG_IT9070)
        ADD_CUSTOM_COMMAND(
            TARGET mp3decode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .codec_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/mp3decode.plugin ${LIBRARY_OUTPUT_PATH}/mp3.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/mp3.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(mp3decode STATIC
        ${MP3_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)


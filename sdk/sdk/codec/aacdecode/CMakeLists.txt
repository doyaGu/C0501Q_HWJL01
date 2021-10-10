INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

SET(AAC_SRCS
    aacdec.c
    aactabs.c
    bitstream.c
    buffers.c
    dct4.c
    decelmnt.c
    dequant.c
    drc.c
    equalizer.c
    fft.c
    filefmt.c
    freqinfo.c
    huffman.c
    hufftabs.c
    imdct.c
    main.c
    noiseless.c
    pns.c
    reverb.c
    sbr.c
    sbrfft.c
    sbrfreq.c
    sbrhfadj.c
    sbrhfgen.c
    sbrhuff.c
    sbrimdct.c
    sbrmath.c
    sbrqmf.c
    sbrside.c
    sbrtabs.c
    stproc.c
    tns.c
    trigtabs.c
    voiceoff.c
    aac_latm.c
    ps_dec.c
    ps_syntax.c        
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(aacdecode.plugin
        ${AAC_SRCS}
    )
  if (DEFINED CFG_ROM_COMPRESS)
		set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET aacdecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/aacdecode.plugin ${LIBRARY_OUTPUT_PATH}/aac.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/aacdecode.plugin ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs ${LIBRARY_OUTPUT_PATH}/aac.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET aacdecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/aacdecode.plugin ${LIBRARY_OUTPUT_PATH}/aac.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/aacdecode.plugin ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/aac.rawcodecs ${LIBRARY_OUTPUT_PATH}/aac.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET aacdecode.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary -j .codec_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/aacdecode.plugin ${LIBRARY_OUTPUT_PATH}/aac.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/aac.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)   
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(aacdecode STATIC
        ${AAC_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)



if (DEFINED CFG_TS_EXTRACTOR_ENABLE)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        ${PROJECT_SOURCE_DIR}/sdk/include/tsi
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_extractor
        ${PROJECT_SOURCE_DIR}/sdk/include/demod_ctrl
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_aggre
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_extract
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_extract/ts_packet_analysis
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_extract/ts_split
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_extract/ts_packet_analysis/pes
        ${PROJECT_SOURCE_DIR}/sdk/driver/ts_extractor/ts_extract/ts_packet_analysis/psi
        )


    if (DEFINED CFG_AGGRE_IT9300)
        include_directories(
            ./ts_aggre/it9300
            )
        add_subdirectory(ts_aggre/it9300)
    endif()

        #ts_aggre
    FILE(GLOB ts_aggre
        ts_aggre/ts_aggre_cfg.h
        ts_aggre/ts_aggre_defs.h
        ts_aggre/ts_aggre_desc.h
        ts_aggre/ts_aggre_err.h
        ts_aggre/ts_aggre_ctrl.c
        ts_aggre/ts_aggre_ctrl.h
        ts_aggre/ts_aggre_endeavour.c
        )
    source_group("ts_aggre" FILES ${ts_aggre})

        # ts_packet_analysis
    FILE(GLOB pes
        ts_extract/ts_packet_analysis/pes/pes_packet_decode.c
        ts_extract/ts_packet_analysis/pes/pes_packet_decode.h
        ts_extract/ts_packet_analysis/pes/pes_stream_audio.c
        ts_extract/ts_packet_analysis/pes/pes_stream_audio.h
        ts_extract/ts_packet_analysis/pes/pes_stream_cfg.h
        ts_extract/ts_packet_analysis/pes/pes_stream_operator.h
        ts_extract/ts_packet_analysis/pes/pes_stream_subtitle.c
        ts_extract/ts_packet_analysis/pes/pes_stream_subtitle.h
        ts_extract/ts_packet_analysis/pes/pes_stream_teletext.c
        ts_extract/ts_packet_analysis/pes/pes_stream_teletext.h
        ts_extract/ts_packet_analysis/pes/pes_stream_video.c
        ts_extract/ts_packet_analysis/pes/pes_stream_video.h
        )
    source_group("pes" FILES ${pes})
    
    FILE(GLOB psi
        ts_extract/ts_packet_analysis/psi/psi_packet_decode.c
        ts_extract/ts_packet_analysis/psi/psi_packet_decode.h
        ts_extract/ts_packet_analysis/psi/psi_section_kit.c
        ts_extract/ts_packet_analysis/psi/psi_section_kit.h
        ts_extract/ts_packet_analysis/psi/psi_descriptor_kit.c
        ts_extract/ts_packet_analysis/psi/psi_descriptor_kit.h
        ts_extract/ts_packet_analysis/psi/psi_table_cfg.h
        ts_extract/ts_packet_analysis/psi/psi_table_operator.h
        ts_extract/ts_packet_analysis/psi/psi_priv_sect_demux.h
        ts_extract/ts_packet_analysis/psi/psi_table_pat.c
        ts_extract/ts_packet_analysis/psi/psi_table_pat.h
        ts_extract/ts_packet_analysis/psi/psi_table_pmt.c
        ts_extract/ts_packet_analysis/psi/psi_table_pmt.h
        ts_extract/ts_packet_analysis/psi/psi_table_sdt.c
        ts_extract/ts_packet_analysis/psi/psi_table_sdt.h    
        )
    source_group("psi" FILES ${psi})
    
    FILE(GLOB ts_packet_analysis
        ${pes}
        ${psi}    
        ts_extract/ts_packet_analysis/crc.c
        ts_extract/ts_packet_analysis/crc.h
        ts_extract/ts_packet_analysis/ts_packet_analysis.c
        ts_extract/ts_packet_analysis/ts_packet_analysis.h
        ts_extract/ts_packet_analysis/ts_packet_analysis_defs.h
        )
    source_group("ts_packet_analysis" FILES ${ts_packet_analysis})

        #ts_split
    FILE(GLOB ts_split
        ts_extract/ts_split/ts_split.c
        ts_extract/ts_split/ts_split.h
        ts_extract/ts_split/ts_split_defs.h
        )
    source_group("ts_split" FILES ${ts_split})

        #ts_extract
    FILE(GLOB ts_extract
        ts_extract/ts_extract.c
        ts_extract/ts_extract.h
        ts_extract/ts_packet_demux.c
        ts_extract/ts_packet_demux.h
        )
    source_group("ts_extract" FILES ${ts_extract})

    add_library(ts_extractor STATIC
        ${ts_extract}
        ${ts_split}
        ${ts_packet_analysis}
        ${ts_aggre}
        ite_ts_extractor.c
        register_template.h
        ring_buf_opt.h
        ts_extractor_cfg.h
        ts_extractor_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_extractor/ite_ts_extractor.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_extractor/ts_extractor_demod_attr.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_extractor/ts_extractor_err.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_extractor/ts_extractor_attr_set.h
        )

endif()


if (DEFINED CFG_TS_DEMUX_ENABLE)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ./ts_parser/psi
        ./ts_parser/pes
        ./ts_parser
        ./ts_txt_conv
        ./ts_info
        ${SMEDIA_INC_DIRS}
        ${PROJECT_SOURCE_DIR}/sdk/include/tsi
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_demuxer
        ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc
        )

    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/include/demod_ctrl
        )

        #psi
    FILE(GLOB psi
        ts_parser/psi/psi_descriptor_0x58.c
        ts_parser/psi/psi_descriptor_kit.c
        ts_parser/psi/psi_packet.c
        ts_parser/psi/psi_section_kit.c
        ts_parser/psi/psi_table_demux.c
        ts_parser/psi/psi_table_eit.c
        ts_parser/psi/psi_table_nit.c
        ts_parser/psi/psi_table_pat.c
        ts_parser/psi/psi_table_pmt.c
        ts_parser/psi/psi_table_sdt.c
        ts_parser/psi/psi_table_tdt.c
        ts_parser/psi/psi_table_tot.c
        ts_parser/psi/psi_time.c
        ##############
        # h file
        ts_parser/psi/bcd.h
        ts_parser/psi/psi_descriptor_0x58.h
        ts_parser/psi/psi_descriptor_kit.h
        ts_parser/psi/psi_packet.h
        ts_parser/psi/psi_section_kit.h
        ts_parser/psi/psi_table_demux.h
        ts_parser/psi/psi_table_eit.h
        ts_parser/psi/psi_table_nit.h
        ts_parser/psi/psi_table_pat.h
        ts_parser/psi/psi_table_pmt.h
        ts_parser/psi/psi_table_sdt.h
        ts_parser/psi/psi_table_tdt.h
        ts_parser/psi/psi_table_tot.h
        ts_parser/psi/psi_time.h
        )
    source_group("psi" FILES ${psi})

        #pes
    FILE(GLOB pes
        ts_parser/pes/pes_share_info.h
        ts_parser/pes/pes_packet.h
        ts_parser/pes/pes_packet.c
        ts_parser/pes/pes_queue_mgr.c
        ts_parser/pes/pes_queue_mgr.h
        )
    source_group("pes" FILES ${pes})

        #ts_info
    FILE(GLOB ts_info
        ts_info/ts_channel_info.c
        ts_info/ts_channel_info.h
        ts_info/ts_service_info.c
        ts_info/ts_service_info.h
        ts_info/global_freq_info.c
        ts_info/global_freq_info.h
        ts_info/ts_epg_info.c
        ts_info/ts_epg_info.h
        ts_info/mbt.c
        ts_info/mbt.h
        )
    source_group("ts_info" FILES ${ts_info})

        #ts_parser
    FILE(GLOB ts_parser
        ts_parser/ts_parser.c
        ts_parser/ts_parser.h
        ts_parser/ts.c
        ts_parser/ts.h
        ts_parser/crc.c
        ts_parser/crc.h
        )
    source_group("ts_parser" FILES ${ts_parser})

        #ts_txt_conv
    FILE(GLOB ts_txt_conv
        ts_txt_conv/iso6937.c
        ts_txt_conv/iso6937.h
        ts_txt_conv/iso8859.c
        ts_txt_conv/iso8859.h
        ts_txt_conv/utf8.c
        ts_txt_conv/utf8.h
        ts_txt_conv/ts_txt_conv.c
        ts_txt_conv/ts_txt_conv.h
        )
    source_group("ts_txt_conv" FILES ${ts_txt_conv})

    add_library(ts_demuxer STATIC
        mmp_types.h
        ts_demuxer_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_demuxer/ts_demuxer_err.h
        bitstream_kit.h
        list_template.h
        register_template.h
        ite_ts_demuxer.c
        ${PROJECT_SOURCE_DIR}/sdk/include/ts_demuxer/ite_ts_demuxer.h
        ${ts_parser}
        ${psi}
        ${pes}
        ${ts_info}
        ${ts_txt_conv}
        )
endif()
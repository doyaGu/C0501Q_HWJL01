if (DEFINED CFG_ITC_ERR)
    add_definitions(
        -DCFG_ITC_ERR
        )
endif()

if (DEFINED CFG_ITC_WARN)
    add_definitions(
        -DCFG_ITC_WARN
        )
endif()

if (DEFINED CFG_ITC_INFO)
    add_definitions(
        -DCFG_ITC_INFO
        )
endif()

if (DEFINED CFG_ITC_DBG)
    add_definitions(
        -DCFG_ITC_DBG
        )
endif()

add_library(itc STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/itc.h
    itc_arraystream.c
    itc_bufferstream.c
    itc_cfg.h
    itc_crc.c
    itc_filestream.c
    itc_stream.c
    itc_tree.c
    itc_url.c
    )

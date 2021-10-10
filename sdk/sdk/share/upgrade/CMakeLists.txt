if (DEFINED CFG_UG_ERR)
    add_definitions(
        -DCFG_UG_ERR
        )
endif()

if (DEFINED CFG_UG_WARN)
    add_definitions(
        -DCFG_UG_WARN
        )
endif()

if (DEFINED CFG_UG_INFO)
    add_definitions(
        -DCFG_UG_INFO
        )
endif()

if (DEFINED CFG_UG_DBG)
    add_definitions(
        -DCFG_UG_DBG
        )
endif()

if (DEFINED CFG_UG_BUF_SIZE)
    add_definitions(
        -DCFG_UG_BUF_SIZE=${CFG_UG_BUF_SIZE}
        )
else()
    add_definitions(
        -DCFG_UG_BUF_SIZE=0x20000
        )
endif()

add_library(upgrade STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ug.h
    ug_cfg.h
    ug_filecrc.c
    ug_device.c
    ug_directory.c
    ug_end.c
    ug_file.c
    ug_mark.c
    ug_package.c
    ug_partition.c
    ug_rawdata.c
    ug_reset.c
    ug_unformatted.c
    ug_crc.c
    )

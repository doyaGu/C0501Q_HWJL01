
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
    .
    ./cmd_packet
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl
    )

    #cmd packet
FILE(GLOB cmd_packet
    ./cmd_packet/cmd_pkt_codec.c
    ./cmd_packet/cmd_pkt_codec.h
    ./cmd_packet/cmd_pkt_register.h
    ./cmd_packet/cmd_pkt_ccHDtv.c
    ./cmd_packet/cmd_pkt_dev_IO.c
    ./cmd_packet/cmd_pkt_dev_mgt.c
    ./cmd_packet/cmd_pkt_image.c
    ./cmd_packet/cmd_pkt_media.c
    ./cmd_packet/cmd_pkt_metadata.c
    ./cmd_packet/cmd_pkt_PTZ.c
    ./cmd_packet/cmd_pkt_video_analytics.c
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_ccHDtv.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_dev_IO.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_dev_mgt.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_image.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_media.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_metadata.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_PTZ.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/cmd_pkt_video_analytics.h
    )
source_group("cmd_packet" FILES ${cmd_packet})

add_library(tscam_ctrl STATIC
    queue_template.h
    register_template.h
    tscam_ctrl_cfg.h
    tscam_ctrl_def.h
    tscam_ctrl.c
    tscam_simplex_rs232.c
    tscam_duplex_rs232.c
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/tscam_ctrl.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/tscam_ctrl_err.h
    ${PROJECT_SOURCE_DIR}/sdk/include/tscam_ctrl/tscam_cmd_service.h
    ${cmd_packet}
    )



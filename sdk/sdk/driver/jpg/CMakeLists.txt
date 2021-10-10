add_definitions(
    -DCFG_ITP_PLATFORM
    # -DCFG_PAL_PLATFORM
)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
    )
endif()

include_directories(
    .
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
)

if (CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910)
ADD_LIBRARY(jpg STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/ite_jpg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_err.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_stream.h
    it9070/ite_jpg.c
    it9070/jpg_types.h
    it9070/jpg_codec.h
    it9070/jpg_common.c
    it9070/jpg_common.h
    it9070/jpg_config.h
    it9070/jpg_dec_mjpg.c
    it9070/jpg_dec_cmd.c
    it9070/jpg_decoder.c
    it9070/jpg_defs.h
    it9070/jpg_enc_mjpg.c
    it9070/jpg_encoder.c
    it9070/jpg_hw.c
    it9070/jpg_hw.h
    it9070/jpg_parser.c
    it9070/jpg_parser.h
    it9070/jpg_reg.h
    it9070/jpg_stream_file.c
    it9070/jpg_stream_mem.c
    it9070/register_template.h
    it9070/jpg_extern_link.c
    it9070/jpg_extern_link.h
)
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
ADD_LIBRARY(jpg STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/ite_jpg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_err.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_stream.h
    it9850/ite_jpg.c
    it9850/jpg_types.h
    it9850/jpg_codec.h
    it9850/jpg_common.c
    it9850/jpg_common.h
    it9850/jpg_config.h
    it9850/jpg_dec_mjpg.c
    it9850/jpg_dec_cmd.c
    it9850/jpg_decoder.c
    it9850/jpg_defs.h
    it9850/jpg_enc_mjpg.c
    it9850/jpg_encoder.c
    it9850/jpg_hw.c
    it9850/jpg_hw.h
    it9850/jpg_parser.c
    it9850/jpg_parser.h
    it9850/jpg_reg.h
    it9850/jpg_stream_file.c
    it9850/jpg_stream_mem.c
    it9850/register_template.h
    it9850/jpg_extern_link.c
    it9850/jpg_extern_link.h
)
endif()

if (CFG_CHIP_FAMILY STREQUAL 9920)
ADD_LIBRARY(jpg STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/ite_jpg.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_err.h
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg/jpg_stream.h
    it9920/ite_jpg.c
    it9920/jpg_types.h
    it9920/jpg_codec.h
    it9920/jpg_common.c
    it9920/jpg_common.h
    it9920/jpg_config.h
    it9920/jpg_dec_mjpg.c
    it9920/jpg_dec_cmd.c
    it9920/jpg_decoder.c
    it9920/jpg_defs.h
    it9920/jpg_enc_mjpg.c
    it9920/jpg_encoder.c
    it9920/jpg_hw.c
    it9920/jpg_hw.h
    it9920/jpg_parser.c
    it9920/jpg_parser.h
    it9920/jpg_reg.h
    it9920/jpg_stream_file.c
    it9920/jpg_stream_mem.c
    it9920/register_template.h
    it9920/jpg_extern_link.c
    it9920/jpg_extern_link.h
)
endif()

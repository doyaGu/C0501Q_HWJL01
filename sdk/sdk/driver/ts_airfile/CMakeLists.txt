
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
    .
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/ts_airfile
    )

add_library(ts_airfile STATIC
    ite_ts_airfile.c
    ts_airfile_gateway.c
    ts_airfile_split_gateway.c
    ts_airfile_aggre_gateway.c
    ts_airfile_nettv.c
    ${PROJECT_SOURCE_DIR}/sdk/include/ts_airfile/ite_ts_airfile.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ts_airfile/ts_airfile_err.h
    register_template.h
    ts_airfile.h
    ts_airfile_cfg.h
    ts_airfile_def.h
    )



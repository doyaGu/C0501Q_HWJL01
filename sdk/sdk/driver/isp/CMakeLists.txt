if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(
        -DDEBUG
    )
endif()

INCLUDE_DIRECTORIES(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
)

if (CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910)
    ADD_LIBRARY(isp STATIC
        it9070/isp.c
        it9070/isp_hw.c
        it9070/isp_hw_op.c
        it9070/isp_queue.c
        it9070/isp_util.c
        it9070/mmp_isp.c

        # inlcude file
        it9070/isp.h
        it9070/isp_config.h
        it9070/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it9070/isp_hw.h
        it9070/isp_hw_op.h
        it9070/isp_queue.h
        it9070/isp_queue_type.h
        it9070/isp_reg.h
        it9070/isp_types.h
        it9070/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
    ADD_LIBRARY(isp STATIC
        it9850/isp.c
        it9850/isp_hw.c
        it9850/isp_util.c
        it9850/mmp_isp.c

        # inlcude file
        it9070/isp.h
        it9070/isp_config.h
        it9070/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it9070/isp_hw.h
        it9070/isp_reg.h
        it9070/isp_types.h
        it9070/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9920)
    ADD_LIBRARY(isp STATIC
        it9920/isp.c
        it9920/isp_hw.c
        it9920/isp_hw_op.c
        it9920/isp_queue.c
        it9920/isp_util.c
        it9920/mmp_isp.c

        # inlcude file
        it9920/isp.h
        it9920/isp_config.h
        it9920/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it9920/isp_hw.h
        it9920/isp_hw_op.h
        it9920/isp_queue.h
        it9920/isp_queue_type.h
        it9920/isp_reg.h
        it9920/isp_types.h
        it9920/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp_9920.h
    )
endif()
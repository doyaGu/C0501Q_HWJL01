INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro
    )


ADD_LIBRARY(mspro STATIC
    mmp_mspro.c
    ms_pro_controller.c
    ms_pro.c
    ms_hw.c
    ms_common.c
    ms_controller.c
    ms.c
    ms_port.c
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/config.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_error.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_reg.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_common.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_pro.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_type.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_hw.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/mspro/mspro/ms_port.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ite_mspro.h
    )

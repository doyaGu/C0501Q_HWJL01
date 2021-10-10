INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/include/rtl8304mb/
    )

ADD_LIBRARY(rtl8304mb
    mdcmdio.c
    rtk_api.c
    rtl8309n_asicdrv.c
    )


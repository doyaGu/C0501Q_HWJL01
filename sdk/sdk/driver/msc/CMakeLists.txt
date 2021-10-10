INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/msc/
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
    )

ADD_LIBRARY(msc STATIC
    usb.c
    transport.c
    scsi.c
    mmp_msc.c
    )

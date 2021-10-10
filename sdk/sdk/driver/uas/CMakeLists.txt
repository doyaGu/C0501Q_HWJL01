INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/uas/inc/
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
    )

ADD_LIBRARY(uas STATIC
    ite_uas.c
    usb.c
    uas.c
    scsi.c
    )

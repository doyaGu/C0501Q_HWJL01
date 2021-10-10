INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc
    )

ADD_LIBRARY(usb STATIC
    mmp_usbex.c
    usb_port.c
    usb/hcd.c
    usb/usb.c
    usb/urb.c
    usb/message.c
    usb/config.c
    usb/quirks.c
    usb/hub.c
    usb/driver.c
    usb/hcd/ehci_hcd.c
#    usb/otg/otg.c
    )

if (DEFINED CFG_USBD_FILE_STORAGE)
    add_library(usb STATIC
        gadget/fsg/file_storage.c
        )
endif()

if (DEFINED CFG_USBD_IDB)
add_definitions(
    -DCONFIG_USBD_IDB=1
)
add_library(usb STATIC
    gadget/idb/idbInjection.c
    gadget/idb/idbProperty.cpp
)
else ()
add_definitions(
    -DCONFIG_USBD_IDB=0
)
endif()

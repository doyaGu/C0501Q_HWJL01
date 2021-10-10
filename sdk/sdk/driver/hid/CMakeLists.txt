
include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
    )

if (DEFINED CFG_USB_MOUSE)
    add_library(hid STATIC
        usbmouse.c
        )
endif()

if (DEFINED CFG_USB_KBD)
    add_library(hid STATIC
        usbkbd.c
        )
endif()


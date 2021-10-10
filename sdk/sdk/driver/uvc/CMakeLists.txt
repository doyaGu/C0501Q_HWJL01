INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc/
    )

ADD_LIBRARY(uvc STATIC
	ctrl.c
	ctrl-gen.c
	device.c
	diag.c
	frame.c
	init.c
	misc.c
	stream.c
	usb.c
    )

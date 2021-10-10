include_directories(
	.
	..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/wiegand
    )

add_library(wiegand STATIC
    wiegand.c
    )

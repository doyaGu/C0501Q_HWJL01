include_directories(
	.
	..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/iic
    )

add_library(iic_sw STATIC
    iic_sw.c
	iic.c
    )

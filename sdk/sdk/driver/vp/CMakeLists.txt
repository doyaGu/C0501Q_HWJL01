include_directories(
	.
	..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/vp
    ${PROJECT_SOURCE_DIR}/sdk/include/encoder
    )

add_library(vp STATIC
	  vp.c
    vp_hw.c
    vp_util.c
    mmp_vp.c
    )

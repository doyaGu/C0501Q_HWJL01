include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
	${SMEDIA_INC_DIRS}
    )

add_library(irda STATIC
    mmp_irda.c
    )


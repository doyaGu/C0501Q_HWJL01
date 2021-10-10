INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
	${PROJECT_SOURCE_DIR}/sdk/driver/mmp/include
    ${PROJECT_SOURCE_DIR}/sdk/include/xcpu_slave
    )

ADD_LIBRARY(xcpu_slave
    sys_msgq.c
	xcpu.c
	xcpu_api.c
	xcpu_msgq.c

    # inlcude file
    ../../include/xcpu_slave/sys_msgq.h
    ../../include/xcpu_slave/xcpu.h
    ../../include/xcpu_slave/xcpu_api.h
    ../../include/xcpu_slave/xcpu_msgq.h

    )



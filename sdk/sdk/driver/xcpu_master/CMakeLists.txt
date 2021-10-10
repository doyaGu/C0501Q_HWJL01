INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/xcpu_master
    )

ADD_LIBRARY(xcpu_master
    xcpu_io.c
    xcpu_msgq.c
    sys_msgq.c
    file.c
    itx.c
    bus.c
    init9919_300_4M_1.h
    init9919_300_4M_2.h
    init9919_300_4M_3.h
    pal.c
    decompress.c

    # inlcude file
    ../../include/xcpu_master/xcpu_io.h
    ../../include/xcpu_master/xcpu_msgq.h
    ../../include/xcpu_master/sys_msgq.h
    ../../include/xcpu_master/file.h
    ../../include/xcpu_master/itx.h
    ../../include/xcpu_master/bus.h
    ../../include/xcpu_master/pal.h
    ../../include/xcpu_master/decompress.h

    )



add_definitions(
    -DHAVE_CONFIG_H
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        #-DNTFS_ENABLE_LOG
        )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/ntfs
    )

add_library(ntfs STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ntfs/disc_io.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ntfs/gccore.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ntfs/gctypes.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ntfs/ntfs.h
    acls.c
    acls.h
    attrib.c
    attrib.h
    attrdef.c
    attrdef.h
    attrlist.c
    attrlist.h
    bitmap.c
    bitmap.h
    bit_ops.h
    boot.c
    boot.h
    bootsect.c
    bootsect.h
    cache.c
    cache.h
    cache2.c
    cache2.h
    collate.c
    collate.h
    compat.c
    compat.h
    compress.c
    compress.h
    config.h
    debug.c
    debug.h
    device.c
    device.h
    device_io.c
    device_io.h
    dir.c
    dir.h
    efs.c
    efs.h
    endians.h
    gekko_io.c
    gekko_io.h
    index.c
    index.h
    inode.c
    inode.h
    layout.h
    lcnalloc.c
    lcnalloc.h
    logfile.c
    logfile.h
    logging.c
    logging.h
    mem_allocate.h
    mft.c
    mft.h
    misc.c
    misc.h
    mkntfs.c
    mst.c
    mst.h
    ntfs.c
    ntfsdir.c
    ntfsdir.h
    ntfsfile.c
    ntfsfile.h
    ntfsinternal.c
    ntfsinternal.h
    ntfstime.h
    object_id.c
    object_id.h
    param.h
    realpath.c
    realpath.h
    reparse.c
    reparse.h
    runlist.c
    runlist.h
    sd.c
    sd.h
    security.c
    security.h
    support.h
    types.h
    unistr.c
    unistr.h
    utils.c
    utils.h
    volume.c
    volume.h
    xattrs.c
    xattrs.h
    )

if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE)
    add_library(ntfs STATIC
        ntfs_sd.c
        )
endif()

if (DEFINED CFG_USB0_ENABLE OR DEFINED CFG_USB1_ENABLE)
    add_library(ntfs STATIC
        ntfs_usb.c
        )
endif()

if (DEFINED CFG_WIN32_SIMULATOR AND DEFINED CFG_WIN32_FS_HW)
    add_library(ntfs STATIC
        ntfs_win32.c
        )
endif()

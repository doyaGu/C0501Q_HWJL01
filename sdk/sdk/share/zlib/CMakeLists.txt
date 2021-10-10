if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

add_library(zlib STATIC
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
	contrib/minizip/ioapi.c
	contrib/minizip/miniunz.c
	contrib/minizip/minizip.c
	contrib/minizip/mztools.c
	contrib/minizip/unzip.c
	contrib/minizip/zip.c
    )

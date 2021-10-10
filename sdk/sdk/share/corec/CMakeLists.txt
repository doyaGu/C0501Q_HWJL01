include_directories(
    .
    )

add_library(corec STATIC
    array.c
    buffer.c
    bufstream.c
    charconvert_utf8.c
    file_libc.c
    hotkey.c
    memstream.c
    multithread_pthread.c
    node.c
    node_internal.h
    nodelookup.c
    nodetree.c
    parser2.c
    str.c
    str_utf8.c
    stream_stdio.c
    streams.c
    strtab.c
    strtypes.c
    tools.c
    urlpart.c
    )

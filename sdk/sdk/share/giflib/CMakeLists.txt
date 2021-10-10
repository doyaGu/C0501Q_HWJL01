add_definitions(
    -DHAVE_CONFIG_H
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
	.
    )    
    
add_library(giflib STATIC
    dev2gif.c
    dgif_lib.c
    egif_lib.c
    gif_err.c 
    gif_font.c
	gif_hash.c
    gifalloc.c
    qprintf.c 
    quantize.c
    )

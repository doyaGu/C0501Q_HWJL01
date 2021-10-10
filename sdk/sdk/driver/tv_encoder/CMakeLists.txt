include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/tv_encoder
    )


add_library(tv_encoder STATIC
        cs8556.c
        
    )

include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/capture_module
    )

add_library(capture_module STATIC
        adv7180.c
        gc0308.c
        gc0328.c
        gt5110e1.c
        hm1375.c
        TVP5155.c
        ${CFG_CAPTURE_MODULE}
        ${CFG_SECOND_CAPTURE_MODULE}
        capture_module.c

        
    )

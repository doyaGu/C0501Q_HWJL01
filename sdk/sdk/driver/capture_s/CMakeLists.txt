    include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/capture_s
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
    ${PROJECT_SOURCE_DIR}/sdk/include
    )

if (CFG_CHIP_FAMILY STREQUAL 9070)
    add_library(capture_s STATIC
        it9070/capture_hw.h
        it9070/capture_reg.h
        it9070/capture_hw.c
        it9070/ite_capture.c
    )
    
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)

    add_library(capture_s STATIC
        it9850/capture_hw.h
        it9850/capture_reg.h
        it9850/capture_hw.c
        it9850/ite_capture.c
    )
    
endif()


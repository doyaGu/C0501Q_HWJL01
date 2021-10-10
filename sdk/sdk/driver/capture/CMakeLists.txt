if (DEFINED CFG_CHIP_PKG_IT9910)
    include_directories(
        .
        ..
        ${SMEDIA_INC_DIRS}
        ${PROJECT_SOURCE_DIR}/sdk/include/capture
        ${PROJECT_SOURCE_DIR}/sdk/include/encoder
        ${PROJECT_SOURCE_DIR}/sdk/include
    )

    add_library(capture STATIC
        it9910/mmp_capture.c
        it9910/capture_hw.c
		it9910/capture_hw.h
		it9910/capture_config.h
		it9910/capture_reg.h
		it9910/capture_util.h
        it9910/capture.c
		it9910/capture.h
        it9910/cap_util.c
    )
if (SENSOR_FW_NAME STREQUAL OMNIVISION_OV7725)  
    add_library(capture STATIC
    	it9910/io_sensor_omnivision_ov7725.c
    )
elseif (SENSOR_FW_NAME STREQUAL PIXELPLUS_PO3100)
    add_library(capture STATIC
    	it9910/io_sensor_pixelplus_po3100.c
    )
elseif (SENSOR_FW_NAME STREQUAL NOVATEK_NT99141)
    add_library(capture STATIC
    	it9910/io_sensor_novatek_nt99141.c
    )    
elseif (SENSOR_FW_NAME STREQUAL AR0130)
    add_library(capture STATIC
    	it9910/io_sensor_ar0130.c
    )
endif()      
endif()


if (CFG_CHIP_FAMILY STREQUAL 9920)
    include_directories(
        .
        ..
        ${SMEDIA_INC_DIRS}
        ${PROJECT_SOURCE_DIR}/sdk/include/capture
        ${PROJECT_SOURCE_DIR}/sdk/include/encoder
        ${PROJECT_SOURCE_DIR}/sdk/include
    )

    add_library(capture STATIC
        it9920/mmp_capture.c
        it9920/capture_hw.c
		it9920/capture_hw.h
		it9920/capture_config.h
		it9920/capture_reg.h
		it9920/capture_util.h
        it9920/capture.c
		it9920/capture.h
        it9920/cap_util.c
		it9920/io_video.c
    )
endif()
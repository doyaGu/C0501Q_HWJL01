INCLUDE_DIRECTORIES(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/
    ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9850
)

LINK_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
)

if (CFG_CHIP_FAMILY STREQUAL 9850)
    if (CFG_VIDEO_FRAME_FUN_ENABLE)
        add_library(itv STATIC
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/itv.h
            it9850/itv_ff.c  
            fc_sync.h
            fc_sync.c
        )
    else()
        add_library(itv STATIC
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/itv.h
            it9850/itv.c 
            fc_sync.h
            fc_sync.c
        )
    endif()
elseif (CFG_CHIP_FAMILY STREQUAL 9920)
        add_library(itv STATIC
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/itv.h
            it9920/itv.c 
            fc_sync.h
            fc_sync.c
        )
else()
    add_library(itv STATIC
        ${PROJECT_SOURCE_DIR}/sdk/include/ite/itv.h
        it9070/itv.c
        fc_sync.h
        fc_sync.c
    )
endif()

target_link_libraries(itv
    isp
)
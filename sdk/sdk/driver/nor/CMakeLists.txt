if (DEFINED CFG_ARM_NOR_WRITER)
    add_definitions(
        -DARM_NOR_WRITER
    )
endif()

include_directories(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/nor
)

if (CFG_CHIP_FAMILY STREQUAL 9070)
    add_library(nor STATIC
        ./it9070/nor.c
        ./it9070/nor_msg.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9910)
    add_library(nor STATIC
        ./it9910/nor.c
        ./it9910/nor_msg.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(nor STATIC
        ./it9850/nor.c
        ./it9850/nor_msg.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9920)
    add_library(nor STATIC
        ./it9920/nor.c
        ./it9920/nor_msg.h
    )
endif()
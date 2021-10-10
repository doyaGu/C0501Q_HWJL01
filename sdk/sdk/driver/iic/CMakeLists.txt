include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/iic
)

if (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(iic STATIC
    	it9850/iic.c
    )
elseif (CFG_CHIP_FAMILY STREQUAL 9910)
    add_library(iic STATIC
    	it9910/iic.c
    )
else()
    add_library(iic STATIC
    	it9070/iic.c
    )
endif()    
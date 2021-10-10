if (DEFINED CFG_ARM_NOR_WRITER)
    add_definitions(
        -DARM_NOR_WRITER
    )
endif()


include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/nor
    ${SMEDIA_INC_DIRS}
)

if (CFG_CHIP_FAMILY STREQUAL 9070)
    add_library(spi STATIC
        ./it9070/mmp_spi.c
        ./it9070/armNorWriterUtility.c
    )    
endif()

if (CFG_CHIP_FAMILY STREQUAL 9910)
    add_library(spi STATIC
        ./it9910/mmp_spi.c
        ./it9910/armNorWriterUtility.c
    )    
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(spi STATIC
        ./it9850/spi_hw.c
        ./it9850/mmp_spi.c
        ./it9850/armNorWriterUtility.c
    )    
endif()

#if (CFG_CHIP_FAMILY STREQUAL 9920)
#    add_library(spi STATIC
#        ./it9850/spi_hw.c
#        ./it9850/mmp_spi.c
#        ./it9850/armNorWriterUtility.c
#    )    
#endif()

if (CFG_CHIP_FAMILY STREQUAL 9920)
    add_library(spi STATIC
        ./it9920/axispi_hw.c
		./it9920/mmp_spi.c
    )    
endif()
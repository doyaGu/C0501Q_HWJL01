include_directories(
    ${CMAKE_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/include/fat
    )

if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE)
    add_library(fat_sd STATIC
        mmcsd/ite_mmcsd.c
        )
endif()

if (DEFINED CFG_MS_ENABLE)
    add_library(fat_mspro STATIC
        mspro/ite_mspro_fat.c
        )
endif()

if (DEFINED CFG_MSC_ENABLE)
    add_library(fat_msc STATIC
        msc/ite_msc_fat.c
        )
endif()

if (DEFINED CFG_XD_ENABLE)
    add_library(fat_xd STATIC
        xd/fat_xddrv.c
        )
endif()

if (DEFINED CFG_NOR_ENABLE)
    add_library(fat_nor STATIC
        nor/cache2.c
        nor/cache2.h
        nor/nordrv_f.c
        nor/nordrv_f.h
        )
endif()

if (DEFINED CFG_RAMDISK_ENABLE)
    add_library(fat_ramdisk STATIC
        ram/ramdrv_f.c
        ram/ramdrv_f.h
        )
endif()

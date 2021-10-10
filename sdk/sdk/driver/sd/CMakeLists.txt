
if (DEFINED CFG_MMC_ENABLE)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/mmc/include
    )

ADD_LIBRARY(sd STATIC
    mmc/core/bus.c
    mmc/core/core.c
    mmc/core/host.c
    mmc/core/mmc.c
    mmc/core/mmc_ops.c
    mmc/core/sd.c
    mmc/core/sd_ops.c
    mmc/core/sdio.c
    mmc/core/sdio_cis.c
    mmc/core/sdio_ops.c
    mmc/core/sdio_io.c
    mmc/core/sdio_bus.c
    mmc/core/sdio_irq.c
    mmc/card/sd_card.c
    )

if (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(sd STATIC
    mmc/host/ite_sdc.c
    )
else()
    add_library(sd STATIC
        mmc/host/sdhci.c
        mmc/host/sdhci-pltfm-ite.c
        )
endif()

else()

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc
    )

add_library(sd STATIC
    sd/ite_sd.c
    sd/sd_hw.c
    sd/sd.c
    sd/mmc.c
    sd/common.c
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc/config.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc/sd_error.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc/sd_reg.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc/sd_hw.h
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/sd/inc/sd.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ite_sd.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_defs.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith.h
    )

endif()

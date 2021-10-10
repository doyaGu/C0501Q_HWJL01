if (DEFINED CFG_NAND_ENABLE)
    INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc
    )

    if (DEFINED CFG_PPI_NAND)
        add_library(nand STATIC
            NF_LB_HWECC.c
            NFDrv.c
            ite_nand.c
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/configs.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/NF_Reg.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/NF_LB_HWECC.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/NFDrv.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ite_nand.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_defs.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith.h
        )
    elseif (DEFINED CFG_SPI_NAND)
        add_library(spi_nand STATIC
        	spinfdrv.c
            NF_SPI_NAND.c
            NFDrv.c
            ite_nand.c
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/configs.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/spinfdrv.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/NF_SPI_NAND.h
            ${PROJECT_SOURCE_DIR}/sdk/driver/nand/inc/NFDrv.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ite_nand.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_defs.h
            ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith.h
        )
    endif()
endif()
message("CFG_LCD_INIT_WITH_SPI")

if (DEFINED CFG_SPI_LCD_854X480_DX050H049)
    add_library(spilcd STATIC
    dx050h049.c
    )

endif()
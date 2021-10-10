if (CFG_CHIP_FAMILY STREQUAL 9070)

    ADD_LIBRARY(i2s STATIC
        it9070/i2s.c
        it9070/i2s_reg.h
    )

endif()

if (CFG_CHIP_FAMILY STREQUAL 9910)

    ADD_LIBRARY(i2s STATIC
        it9910/i2s_it9910.c
        it9910/i2s_reg_it9910.h
    )

endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)

    ADD_LIBRARY(i2s STATIC
        it9850/i2s_9850.c
        it9850/i2s_reg_9850.h
    )

endif()
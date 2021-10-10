add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_LCD_COLOR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_lcd.c
        )
elseif (DEFINED CFG_TEST_LCD_INTR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_lcd_intr.c
        )
elseif (DEFINED CFG_TEST_LCDCONSOLE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_lcdconsole.c
        )
elseif (DEFINED CFG_TEST_OSDCONSOLE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_osdconsole.c
        )
elseif (DEFINED CFG_TEST_BACKLIGHT)
    add_executable(${CMAKE_PROJECT_NAME}
        test_backlight.c
        )
elseif (DEFINED CFG_TEST_STNLCD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_stnlcd.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

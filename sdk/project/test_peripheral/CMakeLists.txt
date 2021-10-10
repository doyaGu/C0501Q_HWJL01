add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_KEYPAD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_keypad.c
        )
elseif (DEFINED CFG_TEST_IR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_ir.c
        )
elseif (DEFINED CFG_TEST_LED)
    add_executable(${CMAKE_PROJECT_NAME}
        test_led.c
        )
elseif (DEFINED CFG_TEST_RTC)
    add_executable(${CMAKE_PROJECT_NAME}
        test_rtc.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

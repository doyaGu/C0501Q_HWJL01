add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_GPIO_EX_1)
    add_executable(${CMAKE_PROJECT_NAME}
        test_gpio_ex1.c
        )
elseif (DEFINED CFG_TEST_GPIO_EX_2)
    add_executable(${CMAKE_PROJECT_NAME}
        test_gpio_ex2.c
        )
elseif (DEFINED CFG_TEST_GPIO_EX_3)
    add_executable(${CMAKE_PROJECT_NAME}
        test_gpio_ex3.c
        )
elseif (DEFINED CFG_TEST_GPIO_EX_4)
    add_executable(${CMAKE_PROJECT_NAME}
        test_gpio_ex4.c
        )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

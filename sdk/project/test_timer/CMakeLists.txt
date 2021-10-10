add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

add_executable(${CMAKE_PROJECT_NAME}
    test_main.c
    )
    
if (DEFINED CFG_TIMER_BASIC_TEST)
    add_definitions(
       -DCFG_TIMER_BASIC_TEST
    )
elseif (DEFINED CFG_TIMER_TIMEOUT_TEST)
    add_definitions(
       -DCFG_TIMER_TIMEOUT_TEST
    )
elseif (DEFINED CFG_TIMER_IRQ_TEST)
    add_definitions(
       -DCFG_TIMER_IRQ_TEST
    )
elseif (DEFINED CFG_TIMER_PWM_TEST)
    add_definitions(
       -DCFG_TIMER_PWM_TEST
    )
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

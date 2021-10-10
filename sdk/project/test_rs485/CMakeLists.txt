add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    test_rs485.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

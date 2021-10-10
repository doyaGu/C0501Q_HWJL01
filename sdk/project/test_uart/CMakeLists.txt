add_executable(${CMAKE_PROJECT_NAME}
    test_openrtos.c
    test_uart.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

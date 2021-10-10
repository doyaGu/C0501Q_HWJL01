include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/include/SDL
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    test_touch.c
    )
    
# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)
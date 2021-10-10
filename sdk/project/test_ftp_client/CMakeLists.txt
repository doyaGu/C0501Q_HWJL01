add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c  
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr/ 
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    config.c
    network.c
    ctrlboard.h
    test_ftp_client.c
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)	

add_executable(${CMAKE_PROJECT_NAME}
    test_openrtos.c
    test_atcmd.c    
    )
    
target_link_libraries(${CMAKE_PROJECT_NAME}
	bt	
	)    

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

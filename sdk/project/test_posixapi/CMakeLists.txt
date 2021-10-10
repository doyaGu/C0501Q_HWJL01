
include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_STDIO)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_stdio.c
    )
endif()

if (DEFINED CFG_TEST_DIRENT)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_dirent.c
    )
endif()

if (DEFINED CFG_TEST_SEMAPHORE)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_semaphore.c
    )
endif()

if (DEFINED CFG_TEST_STDLIB)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_stdlib.c
    )
endif()

if (DEFINED CFG_TEST_SYSTIME)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_systime.c
    )
endif()

if (DEFINED CFG_TEST_SYSTIMES)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_systimes.c
    )
endif()

if (DEFINED CFG_TEST_SYSSTATVFS)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_sysstatvfs.c
    )
endif()

if (DEFINED CFG_TEST_SYSSTAT)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_sysstat.c
    )
endif()

if (DEFINED CFG_TEST_UNISTD)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_unistd.c
    )
endif()

if (DEFINED CFG_TEST_MALLOC)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_malloc.c
    )
endif()

if (DEFINED CFG_TEST_SCHED)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_sched.c
    )
endif()

if (DEFINED CFG_TEST_TIME)

add_executable(${CMAKE_PROJECT_NAME}
    test_time.c
    )
endif()

if (DEFINED CFG_TEST_MQUEUE)

add_executable(${CMAKE_PROJECT_NAME}
    test_mqueue.c
    )
endif()
###########################################

if (DEFINED CFG_TEST_PTHREAD_ATTR)
    
add_executable(${CMAKE_PROJECT_NAME}
    Pthread/test_pthread_attr.c
    )
endif()

if (DEFINED CFG_TEST_PTHREAD_COND)
    
add_executable(${CMAKE_PROJECT_NAME}
    Pthread/test_pthread_cond.c
    )
endif()

if (DEFINED CFG_TEST_PTHREAD_KEY)
    
add_executable(${CMAKE_PROJECT_NAME}
    Pthread/test_pthread_key.c 
    )
endif()

if (DEFINED CFG_TEST_PTHREAD_MUTEX)
    
add_executable(${CMAKE_PROJECT_NAME}
    Pthread/test_pthread_mutex.c
    )
endif()

if (DEFINED CFG_TEST_PTHREAD_OTHERS)
    
add_executable(${CMAKE_PROJECT_NAME}
    Pthread/test_pthread_others.c
    )
endif()


# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

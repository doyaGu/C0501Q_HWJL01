add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    )

if (DEFINED CFG_TEST_CHECKCRC)
    add_executable(${CMAKE_PROJECT_NAME}
    test_checkcrc.c
        )
endif()

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/backup/A
    )

file(COPY
    ${PROJECT_SOURCE_DIR}/data/font
    DESTINATION ${CMAKE_BINARY_DIR}/data/public
    )

if (DEFINED CFG_TEST_CHECKCRC)
    execute_process(COMMAND makecrc -c ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat -i ${CMAKE_BINARY_DIR}/data/private/backup/A)
endif()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

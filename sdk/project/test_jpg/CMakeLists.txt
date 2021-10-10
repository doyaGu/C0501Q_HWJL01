#if (DEFINED CFG_GCC_LTO)
    add_definitions("-fno-lto")
#endif ()

include_directories(
    ./
    ${PROJECT_SOURCE_DIR}/sdk/include/jpg
    ${PROJECT_SOURCE_DIR}/sdk/driver/jpg
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
    ${PROJECT_SOURCE_DIR}/sdk/driver/isp
    )

if ($ENV{CFG_PLATFORM} STREQUAL win32)
    set(EXAMPLE_LINK_LIBS
        Vfw32
        winmm
        )

    add_executable(${CMAKE_PROJECT_NAME}
        main_win32.c
        )

    configure_file(${PROJECT_SOURCE_DIR}/tool/bin/FTCSPI.dll ${CMAKE_CURRENT_BINARY_DIR}/FTCSPI.dll COPYONLY)

elseif($ENV{CFG_PLATFORM} STREQUAL openrtos)
    add_executable(${CMAKE_PROJECT_NAME}
        main_alpha_openrtos.c
        )
endif()

add_executable(${CMAKE_PROJECT_NAME}
    globle.h
    test_factory.c
    test_items.c
    test_items.h
    test_item_isp.c
    test_item_isp.h
    test_item_jpg.c
    test_item_jpg.h
    io_api.c
    io_api.h
    io_mem.c
    io_file.c
    simple_draw.c
    simple_draw.h
    )

# link ilib
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)


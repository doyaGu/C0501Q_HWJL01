set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

add_executable(${CMAKE_PROJECT_NAME}
    config.c
    dynamic_ui.c
    dynamic_ui.h
    function_table.c
    main.c
    scene.c
    )

# files
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/itu/dynamic_ui.itu
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/dynamic_ui.ini
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font/${CFG_FONT_FILENAME}
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/font
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

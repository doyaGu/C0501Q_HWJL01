file(MAKE_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
)
    
set(DATACONV dataconv)
SET(ENABLE_WIEGAND_PLUGIN 1)
add_definitions(
    -DENABLE_WIEGAND_PLUGIN=1
)

set(CODECS_LD "default.ld")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -g3 -mhard-div -fno-short-enums")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -g3 -mhard-div -fno-short-enums -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -mhard-div -fno-short-enums")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g3 -mhard-div -fno-short-enums -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/${CODECS_LD}\" \"${LIBRARY_OUTPUT_PATH}/or32_crt0.o\" \"${LIBRARY_OUTPUT_PATH}/codec_crt0.o\" \"${LIBRARY_OUTPUT_PATH}/ticktimer.o\"")

get_directory_property(defs COMPILE_DEFINITIONS)

foreach (def ${defs})
    set(defargs ${defargs} -D${def})
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(defargs ${defargs} -g)
endif()

## Build tickTimer.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/ticktimer.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/ticktimer.o
        ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/ticktimer.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/ticktimer.c
)
add_custom_target(
    ticktimer ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/ticktimer.o
)

## Build codec_crt0.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
        ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/codec_crt0.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/codec_crt0.c
)

add_custom_target(
    codec_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/codec_crt0.o
)

## Build or32_crt0.S
ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/${SMEDIA_PLATFORM}/include/or32
        ${SMEDIA_DEFS}
        -c
        -o ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
        ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/or32_crt0.S
    DEPENDS ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/or32_crt0.S
)
ADD_CUSTOM_TARGET(
    or32_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
)

## Build default.ld
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
    COMMAND ${CPP}
        ${CMAKE_CUSTOM_C_FLAGS}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -E
        -P
        -o ${LIBRARY_OUTPUT_PATH}/default.ld
        ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/default.lds
    DEPENDS ${CMAKE_SOURCE_DIR}/project/codec_ex_it9850/default.lds
)

add_custom_target(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)

add_subdirectory(${CMAKE_SOURCE_DIR}/sdk/codec_ex ${CMAKE_BINARY_DIR}/sdk/codec_ex)

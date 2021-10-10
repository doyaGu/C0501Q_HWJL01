file(MAKE_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
)

set(DATACONV dataconv)

set(ALT_CPU_LD "default.ld")
set(CMAKE_C_FLAGS_DEBUG     "${EXTRA_FLAGS} -gstabs -O0 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums")
set(CMAKE_C_FLAGS_RELEASE   "${EXTRA_FLAGS} -gstabs -O3 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${EXTRA_FLAGS} -gstabs -O0 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums")
set(CMAKE_CXX_FLAGS_RELEASE "${EXTRA_FLAGS} -gstabs -O3 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS  "${EXTRA_FLAGS} -gstabs -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/${ALT_CPU_LD}\" \"${LIBRARY_OUTPUT_PATH}/alt_cpu_crt0.o\" \"${LIBRARY_OUTPUT_PATH}/ticktimer.o\" \"${LIBRARY_OUTPUT_PATH}/gpio.o\"")

get_directory_property(defs COMPILE_DEFINITIONS)

foreach (def ${defs})
    set(defargs ${defargs} -D${def})
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(defargs ${defargs} -g)
endif()

LINK_DIRECTORIES(
    ${LIBRARY_OUTPUT_PATH}
)

## Build gpio.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/gpio.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        -gstabs -O3 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums -DNDEBUG
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/gpio.o
        ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/gpio.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/gpio.c
)
add_custom_target(
    gpio ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/gpio.o
)

## Build tickTimer.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/ticktimer.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        -gstabs -O3 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums -DNDEBUG
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/ticktimer.o
        ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/ticktimer.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/ticktimer.c
)
add_custom_target(
    ticktimer ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/ticktimer.o
)


## Build alt_cpu_crt0.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/alt_cpu_crt0.o
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        -gstabs -O3 -g3 -mno-delay -msoft-div -msoft-float -msoft-mul -mno-ror -mno-cmov -mno-sext -fomit-frame-pointer -fno-short-enums -DNDEBUG
        ${defargs}        
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/alt_cpu_crt0.o
        ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/alt_cpu_crt0.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/alt_cpu_crt0.c
)

add_custom_target(
    alt_cpu_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/alt_cpu_crt0.o
)

## Build default.ld
ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
    COMMAND ${CPP}
        ${CMAKE_CUSTOM_C_FLAGS}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -E
        -P
        -o ${LIBRARY_OUTPUT_PATH}/default.ld
        ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/default.lds
    DEPENDS ${CMAKE_SOURCE_DIR}/project/alt_cpu_it9850/default.lds
)

ADD_CUSTOM_TARGET(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)

add_subdirectory(${CMAKE_SOURCE_DIR}/sdk/alt_cpu ${CMAKE_BINARY_DIR}/sdk/alt_cpu)
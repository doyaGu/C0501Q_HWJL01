file(MAKE_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib/${CFG_CPU_NAME}
)

set(DATACONV dataconv)
SET(ENABLE_CODECS_PLUGIN 1)
add_definitions(
    -DENABLE_CODECS_PLUGIN=1
)

if (DEFINED CFG_GCC_LTO)
    if (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
        set(EXTRA_FLAGS "-flto")
    endif()

    # For GCC 4.9 or latter version.
    # Use -ffat-lto-objects to create files which contain additionally the
    # object code. This will slow down the linking time, but it solves
    # the undefined symbol when linking.
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -ffat-lto-objects")
    endif()

    add_definitions(
        -DCFG_GCC_LTO
    )
    message("<<enable LTO>>")
endif()

set(RISC1_LD "default.ld")
set(CMAKE_C_FLAGS_DEBUG     "${EXTRA_FLAGS} -gstabs -O0 -g3 -mhard-div -fno-short-enums")
set(CMAKE_C_FLAGS_RELEASE   "${EXTRA_FLAGS} -gstabs -O3 -g3 -mhard-div -fno-short-enums -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${EXTRA_FLAGS} -gstabs -O0 -g3 -mhard-div -fno-short-enums")
set(CMAKE_CXX_FLAGS_RELEASE "${EXTRA_FLAGS} -gstabs -O3 -g3 -mhard-div -fno-short-enums -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS  "${EXTRA_FLAGS} -gstabs -nostartfiles -T\"${LIBRARY_OUTPUT_PATH}/${RISC1_LD}\" \"${LIBRARY_OUTPUT_PATH}/or32_crt0.o\" \"${LIBRARY_OUTPUT_PATH}/risc1_crt0.o\"")

get_directory_property(defs COMPILE_DEFINITIONS)

foreach (def ${defs})
    set(defargs ${defargs} -D${def})
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(defargs ${defargs} -g)
endif()

SET(RISC1_SRCS
    main.c
)

LINK_DIRECTORIES(
    ${LIBRARY_OUTPUT_PATH}
)

## Build risc1_code_crt0.c
add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/risc1_crt0.o
    PRE_BUILD
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}
        -I${PROJECT_SOURCE_DIR}/sdk/include
        -c
        -o ${LIBRARY_OUTPUT_PATH}/risc1_crt0.o
        ${CMAKE_SOURCE_DIR}/project/risc1_code/risc1_crt0.c
    DEPENDS ${CMAKE_SOURCE_DIR}/project/risc1_code/risc1_crt0.c
)

add_custom_target(
    risc1_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/risc1_crt0.o
)

## Build or32_crt0.S
ADD_CUSTOM_COMMAND(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
    PRE_BUILD
    COMMAND ${CMAKE_C_COMPILER}
        ${CMAKE_C_COMPILER_ARG1}
        ${CMAKE_CUSTOM_C_FLAGS}
        ${defargs}
        -I${PROJECT_SOURCE_DIR}/${SMEDIA_PLATFORM}/include/or32
        ${SMEDIA_DEFS}
        -c
        -o ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
        ${CMAKE_SOURCE_DIR}/project/risc1_code/or32_crt0.S
    DEPENDS ${CMAKE_SOURCE_DIR}/project/risc1_code/or32_crt0.S
)

ADD_CUSTOM_TARGET(
    or32_crt0 ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/or32_crt0.o
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
        ${CMAKE_SOURCE_DIR}/project/risc1_code/default.lds
    DEPENDS ${CMAKE_SOURCE_DIR}/project/risc1_code/default.lds
)

ADD_CUSTOM_TARGET(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)

ADD_EXECUTABLE(risc1_testcode.plugin
    ${RISC1_SRCS}
)
ADD_DEPENDENCIES(risc1_testcode.plugin
    risc1_crt0 or32_crt0 default.ld
)

ADD_CUSTOM_COMMAND(
    TARGET risc1_testcode.plugin
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP}
            -D ${CMAKE_CURRENT_BINARY_DIR}/risc1_testcode.plugin > ${LIBRARY_OUTPUT_PATH}/risc1.dbg
    COMMAND ${CMAKE_OBJCOPY}
            -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/risc1_testcode.plugin ${LIBRARY_OUTPUT_PATH}/risc1.raw
    COMMAND ${CMAKE_OBJCOPY}
            -O binary -j .reset ${CMAKE_CURRENT_BINARY_DIR}/risc1_testcode.plugin ${LIBRARY_OUTPUT_PATH}/risc_startup.raw
    COMMAND ${READELF}
            -a ${CMAKE_CURRENT_BINARY_DIR}/risc1_testcode.plugin > ${CMAKE_CURRENT_BINARY_DIR}/risc1.symbol
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/risc1.raw
    COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/risc_startup.raw
)
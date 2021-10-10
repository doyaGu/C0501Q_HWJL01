add_library(boot STATIC
    init.c
)

if (DEFINED CFG_LCD_ENABLE AND NOT DEFINED CFG_LCD_MULTIPLE)
    if (DEFINED CFG_VIDEO_ENABLE OR DEFINED CFG_LCD_TRIPLE_BUFFER)
       add_definitions(
           -DCFG_LCDC_HEIGHT=${CFG_LCD_HEIGHT}
           -DCFG_LCDC_PITCH=${CFG_LCD_PITCH}
       )          
    else()
        add_definitions(
        -DCFG_LCDC_HEIGHT=0
        -DCFG_LCDC_PITCH=0
        -DCFG_GAP_SIZE=0
        )
   endif()       
else()
    add_definitions(
        -DCFG_LCDC_HEIGHT=0
        -DCFG_LCDC_PITCH=0
        -DCFG_GAP_SIZE=0
        -DCFG_LCD_PITCH=0
        -DCFG_LCD_HEIGHT=0
    )
endif()

if (DEFINED CFG_GCC_LTO)
    add_definitions(-fno-lto)
endif()

get_directory_property(defs COMPILE_DEFINITIONS)

foreach (def ${defs})
    set(defargs ${defargs} -D${def})
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(defargs ${defargs} -g)
endif()

add_custom_target(
    startup.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/startup.o
)
if (DEFINED CFG_CPU_FA626)
add_custom_target(
    basic_op2.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/basic_op2.o
)
endif()

if (DEFINED CFG_CPU_FA626)
add_custom_target(
    cfft_64.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/cfft_64.o
)
endif()

if (DEFINED CFG_CPU_FA626)
add_custom_target(
    fdsr.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/fdsr.o
)
endif()

if (DEFINED CFG_CPU_FA626)
add_custom_target(
    HowlingCtrl.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/HowlingCtrl.o
)
endif()

if (DEFINED CFG_CPU_FA626)
add_custom_target(
    tlb.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/tlb.o
)
endif()

if (DEFINED CFG_CPU_FA626)
add_custom_target(
    tlb_wt.o ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/tlb_wt.o
)
endif()

add_custom_target(
    default.ld ALL
    DEPENDS ${LIBRARY_OUTPUT_PATH}/default.ld
)

if (DEFINED CFG_CPU_FA626)

    add_library(boot STATIC
        fa626/sync.c
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/startup.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/startup.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/startup.S
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/startup.S
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/basic_op2.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/basic_op2.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/basic_op2.s
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/basic_op2.s
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/fdsr.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/fdsr.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/fdsr.s
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/fdsr.s
    )
    
    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/HowlingCtrl.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/HowlingCtrl.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/HowlingCtrl.s
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/HowlingCtrl.s
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/cfft_64.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/cfft_64.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/cfft_64.s
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/cfft_64.s
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/tlb.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/tlb.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/tlb.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/tlb.c
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/tlb_wt.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -DFORCE_CPU_WRITE_THROUGH
            -c
            -o ${LIBRARY_OUTPUT_PATH}/tlb_wt.o
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/tlb.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/tlb.c
    )
	
    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
        COMMAND ${CPP}
            ${defargs}
            -E
            -P
            -o ${LIBRARY_OUTPUT_PATH}/default.ld
            ${CMAKE_CURRENT_SOURCE_DIR}/fa626/default.lds
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fa626/default.lds
    )
elseif (DEFINED CFG_CPU_SM32)
    if (DEFINED CFG_CPU_LITTLE_ENDIAN)
        set(defargs ${defargs} -mlittle-endian)
    endif()

    add_library(boot STATIC
        sm32/interrupts.c
        sm32/support.c
        sm32/sync.c
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/startup.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -c
            -o ${LIBRARY_OUTPUT_PATH}/startup.o
            ${CMAKE_CURRENT_SOURCE_DIR}/sm32/startup.S
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sm32/startup.S
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
        COMMAND ${CPP}
            ${defargs}
            -E
            -P
            -o ${LIBRARY_OUTPUT_PATH}/default.ld
            ${CMAKE_CURRENT_SOURCE_DIR}/sm32/default.lds
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sm32/default.lds
    )
elseif (DEFINED CFG_CPU_RISCV)
    add_library(boot STATIC
        riscv/sync.c
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/startup.o
        COMMAND ${CMAKE_C_COMPILER}
            ${CMAKE_C_COMPILER_ARG1}
            ${defargs}
            -march=rv32imac -mabi=ilp32
            -c
            -o ${LIBRARY_OUTPUT_PATH}/startup.o
            ${CMAKE_CURRENT_SOURCE_DIR}/riscv/startup.S
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/riscv/startup.S
    )

    add_custom_command(
        OUTPUT ${LIBRARY_OUTPUT_PATH}/default.ld
        COMMAND ${CPP}
            ${defargs}
            -E
            -P
            -o ${LIBRARY_OUTPUT_PATH}/default.ld
            ${CMAKE_CURRENT_SOURCE_DIR}/riscv/default.lds
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/riscv/default.lds
    )
endif()

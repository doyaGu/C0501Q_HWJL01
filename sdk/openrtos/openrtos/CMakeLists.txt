include_directories(
    ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include/openrtos
    )

add_library(openrtos STATIC
    croutine.c
    event_groups.c
    list.c
    queue.c
    tasks.c
    timers.c
    )

if (DEFINED CFG_CPU_FA626)
    include_directories(
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include/openrtos/fa626
        )

    if (DEFINED CFG_GCC_LTO)
        set_source_files_properties( fa626/portISR.c PROPERTIES COMPILE_FLAGS -fno-lto )
    endif ()

    add_library(openrtos STATIC
        fa626/port.c
        fa626/portISR.c
        )

elseif (DEFINED CFG_CPU_SM32)
    include_directories(
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include/openrtos/sm32
        )

    if (DEFINED CFG_GCC_LTO)
        set_source_files_properties( sm32/portasm.S PROPERTIES COMPILE_FLAGS -fno-lto )
    endif ()

    if (DEFINED CFG_CPU_LITTLE_ENDIAN)
        set_source_files_properties( sm32/portasm.S PROPERTIES COMPILE_FLAGS -mlittle-endian )
    endif ()

    add_library(openrtos STATIC
        sm32/port.c
        sm32/portasm.S
        )
elseif (DEFINED CFG_CPU_RISCV)
    include_directories(
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include/openrtos/riscv
        )

    if (DEFINED CFG_GCC_LTO)
        set_source_files_properties( riscv/portasm.S PROPERTIES COMPILE_FLAGS "-march=rv32imac -mabi=ilp32 -fno-lto" )
    else()
        set_source_files_properties( riscv/portasm.S PROPERTIES COMPILE_FLAGS "-march=rv32imac -mabi=ilp32" )
    endif ()

    add_library(openrtos STATIC
        riscv/port.c
        riscv/portasm.S
        )
endif()

if (${CFG_OPENRTOS_HEAP_SIZE} STRGREATER "0")
    add_library(openrtos STATIC
        heap_4.c
        )
else()
    add_library(openrtos STATIC
        heap_3.c
        )
endif()

if (DEFINED CFG_DBG_TRACE_ANALYZER AND DEFINED CFG_DBG_TRACE)
    add_library(openrtos STATIC
        trcBase.c
        trcHardwarePort.c
        trcKernel.c
        trcKernelPort.c
        trcUser.c
        )
endif()

if (DEFINED CFG_DBG_CLI)
    add_library(openrtos STATIC
        FreeRTOS_CLI.c
        )
endif()

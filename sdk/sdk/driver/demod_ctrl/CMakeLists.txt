

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/demod_ctrl/
    )

if(DEFINED CFG_BUILD_USB)
    include_directories(        
        ${PROJECT_SOURCE_DIR}/sdk/driver/usb/inc
        )

    set(usb_demod_ctrl
        # usb demod ctrl
        usb_demod/usb_demod.c
        usb_demod/usb_demod_transport.c
        usb_demod/usb_demod.h
        usb_demod/usb_demod_transport.h
        usb_demod/usb_demod_cfg.h
        )
endif()
    
    #demod_ctrl
FILE(GLOB demodCtrl 
    demod_config.h
    demod_ctrl.c
    ${PROJECT_SOURCE_DIR}/sdk/include/demod_ctrl/demod_ctrl.h
    demod.h
    demod_omega.c
    demod_it9135.c
    demod_it9137.c
    demod_it9137_usb.c
    ${usb_demod_ctrl}
    )
source_group("demodCtrl" FILES ${demodCtrl})

add_library(demod_ctrl STATIC
    ${demodCtrl}
    )

if (DEFINED CFG_DEMOD_OMEGA)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ./it9133
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        )

        #it9133_OMEGA
    FILE(GLOB it9133_OMEGA
        it9133/IT9133.cpp
        it9133/standard.cpp
        it9133/user.cpp
        it9133/Omega.cpp
        it9133/tuner.cpp
        it9133/cmd.cpp
        it9133/error.h
        it9133/IT9133.h
        it9133/register.h
        it9133/standard.h
        it9133/type.h
        it9133/user.h
        it9133/variable.h
        it9133/version.h
        it9133/Omega.h
        it9133/tuner.h
        it9133/cmd.h
        it9133/Firmware_Afa_Omega_Script.h
        )
    source_group("it9133_OMEGA" FILES ${it9133_OMEGA})

    add_library(demod STATIC
        ${it9133_OMEGA}
        )

    if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
        set_target_properties(demod PROPERTIES COMPILE_FLAGS "-Dfirmware_OMEGA")
    endif()
    
elseif (DEFINED CFG_DEMOD_IT9135 OR CFG_DEMOD_IT9137_USB)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ./it9135
        ./it9135/bus
        ./it9135/tuner
        ./usb_demod
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        )

        #it9135
    FILE(GLOB it9135_DEMOD
        it9135/error.h
        it9135/firmware.h
        it9135/firmware_V2.h
        it9135/it9130.cpp
        it9135/it9130.h
        it9135/register.h
        it9135/standard.cpp
        it9135/standard.h
        it9135/type.h
        it9135/user.cpp
        it9135/user.h
        it9135/variable.h
        it9135/version.h
        it9135/bus/cmd.cpp
        it9135/bus/cmd.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_1_Script.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_1_Script_V2.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_2_Script.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_2_Script_V2.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_3_Script_V2.h
        it9135/tuner/Firmware_Afa_Omega_LNA_Config_4_Script_V2.h
        it9135/tuner/Firmware_Afa_Omega_Script.h
        it9135/tuner/Firmware_Afa_Omega_Script_V2.h
        it9135/tuner/omega.cpp
        it9135/tuner/omega.h
        it9135/tuner/tuner.cpp
        it9135/tuner/tuner.h
        )
    source_group("it9135_DEMOD" FILES ${it9135_DEMOD})

    add_library(demod STATIC
        ${it9135_DEMOD}
        )

    if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
        set_target_properties(demod PROPERTIES COMPILE_FLAGS "-Dfirmware_OMEGA")
    endif()
elseif (DEFINED CFG_DEMOD_IT9137)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ./it9137
        ./it9137/bus
        ./it9137/tuner
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        )

        #it9137
    FILE(GLOB it9137_DEMOD
        it9137/error.h
        it9137/firmware.h
        it9137/firmware_V2.h
        it9137/firmware_V2I.h
        it9137/it9130.cpp
        it9137/it9130.h
        it9137/register.h
        it9137/standard.cpp
        it9137/standard.h
        it9137/type.h
        it9137/user.cpp
        it9137/user.h
        it9137/variable.h
        it9137/version.h
        it9137/bus/cmd.cpp
        it9137/bus/cmd.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_1_Script.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_1_Script_V2.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_1_Script_V2I.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_2_Script.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_2_Script_V2.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_2_Script_V2I.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_3_Script_V2.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_3_Script_V2I.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_4_Script_V2I.h
        it9137/tuner/Firmware_Afa_Omega_LNA_Config_5_Script_V2I.h
        it9137/tuner/Firmware_Afa_Omega_Script.h
        it9137/tuner/Firmware_Afa_Omega_Script_V2.h
        it9137/tuner/omega.cpp
        it9137/tuner/omega.h
        it9137/tuner/tuner.cpp
        it9137/tuner/tuner.h
        )
    source_group("it9137_DEMOD" FILES ${it9137_DEMOD})

    add_library(demod STATIC
        ${it9137_DEMOD}
        )

    if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
        set_target_properties(demod PROPERTIES COMPILE_FLAGS "-Dfirmware_OMEGA")
    endif()
endif()


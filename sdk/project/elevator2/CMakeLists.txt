set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

add_definitions(
    -DCFG_WEBSERVER_PORT=${CFG_WEBSERVER_PORT}
    -DCFG_WEBSERVER_PORT_STR="${CFG_WEBSERVER_PORT}"
    -DCFG_UPGRADE_DATA_URL="${CFG_UPGRADE_DATA_URL}"
    -DCFG_UPGRADE_FW_URL="${CFG_UPGRADE_FW_URL}"
    )

if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    add_definitions(
        -DCFG_CHECK_FILES_CRC_ON_BOOTING
        )
endif()

add_definitions(
    -DCFG_RES_PATH="${CFG_RES_PATH}"
    )

if (DEFINED CFG_UPGRADE_GUI)
    add_definitions(
        -DCFG_UPGRADE_GUI
        )
    execute_process(COMMAND dataconv
        -x ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/upgrade_h.itu
        -o ${CMAKE_CURRENT_BINARY_DIR}/upgrade_h.inc
        )        
    execute_process(COMMAND dataconv
        -x ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/upgrade_v.itu
        -o ${CMAKE_CURRENT_BINARY_DIR}/upgrade_v.inc
        )                
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/mmp/include
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/lwip_ftpd
    ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd
    ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd/include    
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    )
    
add_executable(${CMAKE_PROJECT_NAME}
    audio.c
    config.c
    elevator.h
    external.c
    function_table.c
    layer_main.c
    layer_setting.c
    main.c
    reset.c
    scene.c
    scene.h
    screen.c
    storage.c
    upgrade.c
    )

if (DEFINED CFG_NET_ENABLE)
    add_executable(${CMAKE_PROJECT_NAME}
        network.c
        webserver.c
        )
endif()

# files
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/elevator.ini ${CMAKE_BINARY_DIR}/data/public/elevator.ini COPYONLY)

file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/elevator_h.itu
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/elevator_v.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/public/${CFG_RES_PATH}
    )

if (DEFINED CFG_NET_WIFI_WPA_ENABLE)
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd_wpa.conf.in
    ${CMAKE_BINARY_DIR}/data/public/hostapd.conf
    NEWLINE_STYLE UNIX
    )
else()
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd.conf.in
    ${CMAKE_BINARY_DIR}/data/public/hostapd.conf
    NEWLINE_STYLE UNIX
    )
endif()

# backups
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/elevator.ini ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/elevator.ini COPYONLY)
# crc
if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    file(REMOVE ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/ite_crc.dat)
    execute_process(COMMAND makecrc -c ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat -i ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
    execute_process(COMMAND makecrc -v ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat -i ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
    file(COPY ${CMAKE_BINARY_DIR}/data/public/ite_crc.dat DESTINATION ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE})
endif()

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font/${CFG_FONT_FILENAME}
    DESTINATION ${CMAKE_BINARY_DIR}/data/private/font
    )

# web
file(GLOB files ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/*.*)
foreach (src ${files})
    string(REPLACE "${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/" "${CMAKE_BINARY_DIR}/data/private/web/" dest ${src})
    configure_file(${src} ${dest} COPYONLY)
endforeach()

# codec
if (DEFINED CFG_COMPRESS_AUDIO_PLUGIN)
    if (CFG_CHIP_FAMILY STREQUAL 9850)
    configure_file(${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_it9850/lib/sm32/mp3.codecs ${CMAKE_BINARY_DIR}/data/private/codec/mp3.codecs COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec_it9850/lib/sm32/wave.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wave.codecs COPYONLY)
    else()
    configure_file(${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec/lib/sm32/mp3.codecs ${CMAKE_BINARY_DIR}/data/private/codec/mp3.codecs COPYONLY)
    configure_file(${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/codec/lib/sm32/wave.codecs ${CMAKE_BINARY_DIR}/data/private/codec/wave.codecs COPYONLY)
    endif()
endif()

if (DEFINED CFG_NET_WIFI_WPA_ENABLE)
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd_wpa.conf.in
    ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/hostapd.conf
    NEWLINE_STYLE UNIX
    )
else()    
configure_file(
    ${PROJECT_SOURCE_DIR}/sdk/share/wifi_hostapd/hostapd.conf.in
    ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/hostapd.conf
    NEWLINE_STYLE UNIX
    )
endif()

# res
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Music
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Photo1
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Photo2
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Text1
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Text2    
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Video
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/Voice    
    DESTINATION ${CMAKE_BINARY_DIR}/data/public/${CFG_RES_PATH}
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

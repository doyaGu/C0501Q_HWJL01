set(PROJ_NAME ${CMAKE_PROJECT_NAME})

if (DEFINED CFG_BOOTLOADER_ENABLE)
    set(CMAKE_PROJECT_NAME bootloader)
    add_subdirectory(../bootloader ../bootloader)
endif()

set(CMAKE_PROJECT_NAME ${PROJ_NAME})

if (DEFINED CFG_CHECK_FILES_CRC_ON_BOOTING)
    add_definitions(
        -DCFG_CHECK_FILES_CRC_ON_BOOTING
        )
endif()

if (DEFINED CFG_PLAY_VIDEO_ON_BOOTING)
    add_definitions(
        -DCFG_PLAY_VIDEO_ON_BOOTING
        -DCFG_BOOT_VIDEO_BGCOLOR=${CFG_BOOT_VIDEO_BGCOLOR}
        -DCFG_BOOT_VIDEO_VOLUME=${CFG_BOOT_VIDEO_VOLUME}
        )
endif()

if (DEFINED CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE)
    add_definitions(
        -DCFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
        -DCFG_VIDEO_WINDOW_X_POS=${CFG_VIDEO_WINDOW_X_POS}
        -DCFG_VIDEO_WINDOW_Y_POS=${CFG_VIDEO_WINDOW_Y_POS}
        -DCFG_VIDEO_WINDOW_WIDTH=${CFG_VIDEO_WINDOW_WIDTH}
        -DCFG_VIDEO_WINDOW_HEIGHT=${CFG_VIDEO_WINDOW_HEIGHT}
        )
endif()

if (DEFINED CFG_ENABLE_ROTATE)
    add_definitions(
        -DCFG_ENABLE_ROTATE
        )
endif()

if (DEFINED CFG_UPGRADE_GUI)
    add_definitions(
        -DCFG_UPGRADE_GUI
        )
    execute_process(COMMAND dataconv
        -x ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/upgrade.itu
        -o ${CMAKE_CURRENT_BINARY_DIR}/upgrade.inc
        )        
endif()

if (DEFINED CFG_NET_ENABLE)
    add_definitions(
        -DCFG_WEBSERVER_PORT=${CFG_WEBSERVER_PORT}
        -DCFG_WEBSERVER_PORT_STR="${CFG_WEBSERVER_PORT}"
        )
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/driver/mmp/include
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv
    
if (CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9850
    )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9070
    )
endif()    
    )

if (DEFINED CFG_NET_ENABLE)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd
        ${PROJECT_SOURCE_DIR}/sdk/share/microhttpd/include
    )
endif()

add_executable(${CMAKE_PROJECT_NAME}
    audio.c
    config.c
    airconditioner.h
    external.c
    function_table.c
    layer_engineer.c
    layer_engineer_machine.c
    layer_engineer_power.c
    layer_engineer_try.c
    layer_logo.c
    layer_main.c
    layer_setting.c
    layer_videoplayer.c
    main.c
    power.c
    reset.c
    scene.c
    scene.h
    screen.c
    string.c
    upgrade.c
    )

# boot video
if (DEFINED CFG_PLAY_VIDEO_ON_BOOTING)
	add_executable(${CMAKE_PROJECT_NAME}
		video.c
		)
		
	file(COPY
		${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/boot/boot.mp4
		DESTINATION ${CMAKE_BINARY_DIR}/data/private/media
		)
endif()

if (DEFINED CFG_PLAY_MJPEG_ON_BOOTING)
	add_executable(${CMAKE_PROJECT_NAME}
		video_mjpeg.c
		)
		
	file(COPY
        ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/boot/${CFG_BOOT_MJPEG_FILE}
		DESTINATION ${CMAKE_BINARY_DIR}/data/private/media
		)
endif()

if (DEFINED CFG_NET_ENABLE)
    add_executable(${CMAKE_PROJECT_NAME}
        network.c    
        webserver.c
        )
endif()

# files
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/airconditioner.ini ${CMAKE_BINARY_DIR}/data/public/airconditioner.ini COPYONLY)

file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/${CFG_ITU_PATH}/airconditioner.itu
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# backups
configure_file(${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/airconditioner.ini ${CMAKE_BINARY_DIR}/data/private/backup/${CFG_PUBLIC_DRIVE}/airconditioner.ini COPYONLY)
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

# sound
file(COPY
    ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/sound
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

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

# media
file(GLOB files ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/*.*)
foreach (src ${files})
   string(REPLACE "${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/media/" "${CMAKE_BINARY_DIR}/data/private/media/" dest ${src})
   configure_file(${src} ${dest} COPYONLY)
endforeach()

# web
file(GLOB files ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/*.*)
foreach (src ${files})
    string(REPLACE "${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/web/" "${CMAKE_BINARY_DIR}/data/private/web/" dest ${src})
    configure_file(${src} ${dest} COPYONLY)
endforeach()

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(qrdecode)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(qrencode)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(face_recognizer)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(finger_recognizer)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(opencv)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(dtmf_dec)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(pillowtalk)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(mongoose)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(httpserver)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(microhttpd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(lwip_ftpd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(tre)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(yajl)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(onvif_nvt)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(airplay_nmt)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(shairport)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(shairport_dacp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ao)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ushare)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(urender)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(gnash)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(flirt)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(mad)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(leaf)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(linphone)


if (DEFINED CFG_BUILD_LIVE555)
    target_link_libraries(${CMAKE_PROJECT_NAME}
         live555MediaClient
    )
endif()

if (DEFINED CFG_BUILD_AUDIO_PREPROCESS)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        voipdsp
    )
endif()

if (DEFINED CFG_BUILD_MEDIASTREAMER2)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        mediastreamer
        voipdsp
    )
endif()

if (DEFINED CFG_BUILD_SQLITE3)
    target_link_libraries(${CMAKE_PROJECT_NAME}
       sqlite3  
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(matroska)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ebml)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(corec)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(fm2018)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(agg)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(boost)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sdl_ttf)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sdl_image)

if (DEFINED CFG_BUILD_SDL)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        sdl_main
        sdl
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(iniparser)

if (DEFINED CFG_BUILD_ITU)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        itu
        itu_private
        itu
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(pyinput)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(fontconfig)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(freetype)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(xml2)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(giflib)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(png)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(jpeg)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(speex)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(wifi_mgr)

if (DEFINED CFG_BUILD_WIFI_WPA)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        wpa_supplicant
    )
endif()

if (DEFINED CFG_BUILD_WIFI_HOSTAPD)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        hostapd
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(upnp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(airplaylib)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(wac)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(dns_sd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ping)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(json)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sntp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sntp_server)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(dhcps)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(siproxd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(eXosip2)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(osip2)

if (DEFINED CFG_BUILD_OPENSSL)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ssl
        crypto
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(stund)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ortp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(upgrade)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(itc)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(curl)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(redblack)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(zlib)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(xcpu_master)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(tslib)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(polarssl)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(audio_mgr)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(itv)
#if (DEFINED CFG_LCD_ENABLE)
    if (DEFINED CFG_VIDEO_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
           ffmpeg
        )
    endif()
#endif()
#ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ffmpeg)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(audio)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(OpenVG)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(m2d)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(jpg)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(isp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(async_file)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(uiEnc)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(camera)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(i2s)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(itp)

if (DEFINED CFG_BUILD_NAND AND DEFINED CFG_SPI_NAND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        spi_nand
    )
elseif (DEFINED CFG_BUILD_NAND AND DEFINED CFG_PPI_NAND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        nand
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(xd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(hdmi)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(hdmitx)

if (DEFINED CFG_BUILD_IIC)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        iic
    )
endif()

if (DEFINED CFG_BUILD_IIC_SW)
target_link_libraries(${CMAKE_PROJECT_NAME}
        iic_sw
    )
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(pcf8575)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(wiegand)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(modbus)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(nor)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(spi)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(lwip)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(rtl8304mb)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(mac)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(wifi)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(wifi_eus)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ucl)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(encrypt)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ntfs)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(bt)

if (DEFINED CFG_BUILD_FAT)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        fat
    )

    if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_sd
        )
    endif()

    if (DEFINED CFG_MS_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_mspro
        )
    endif()

    if (DEFINED CFG_MSC_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_msc
        )
    endif()

    if (DEFINED CFG_BUILD_NAND)
        if (CFG_NAND_PAGE_SIZE EQUAL 2048)
            target_link_libraries(${CMAKE_PROJECT_NAME}
                fat_nand2k
            )                    
        endif()
        
        if (CFG_NAND_PAGE_SIZE EQUAL 4096)
            target_link_libraries(${CMAKE_PROJECT_NAME}
                fat_nand4k
            )                    
        endif()
    endif()

    if (DEFINED CFG_BUILD_XD)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_xd
        )
    endif()

    if (DEFINED CFG_BUILD_NOR)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_nor
        )
    endif()

    if (DEFINED CFG_RAMDISK_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            fat_ramdisk
        )
    endif()
endif()

ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(uvc)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(ameba_sdio)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sd)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(mspro)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(msc)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(uas)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(hid)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(usb)

#temp solution
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(capture)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(capture_s)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(capture_module)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(vp)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(sensor)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(encoder)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(video_encoder)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(tv_encoder)

if (DEFINED CFG_AUDIO_ENABLE)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(risc)
endif()

# Ts Module
if (DEFINED CFG_PURE_TS_STREAM)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        tsi
    )
elseif (DEFINED CFG_ISDB_STANDARDS)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ts_demuxer
        demod_ctrl
        tsi
    )
elseif (DEFINED CFG_DVB_STANDARDS)

    target_link_libraries(${CMAKE_PROJECT_NAME}
        ts_airfile
    )

    if (DEFINED CFG_TS_DEMUX_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            ts_demuxer
        )
    endif()

    if (DEFINED CFG_TS_EXTRACTOR_ENABLE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            ts_extractor
        )

        if (NOT DEFINED CFG_AGGRE_NONE)
            target_link_libraries(${CMAKE_PROJECT_NAME}
                aggre
            )
        endif()
    endif()

    target_link_libraries(${CMAKE_PROJECT_NAME}
        demod_ctrl
        tsi
    )

    if (NOT DEFINED CFG_DEMOD_NONE)
        target_link_libraries(${CMAKE_PROJECT_NAME}
            demod
        )
    endif()
endif()

# return channel option
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(tscam_ctrl)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(irda)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(nedmalloc)
ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(linux)

if (DEFINED CFG_RAMDISK_ENABLE)
    execute_process(COMMAND makedisk -p ${CMAKE_BINARY_DIR}/data/private -o ${CMAKE_BINARY_DIR}/ramdisk.img -s ${CFG_RAMDISK_SIZE})
    execute_process(COMMAND dataconv -x ${CMAKE_BINARY_DIR}/ramdisk.img -o ${CMAKE_BINARY_DIR}/ramdisk.inc)
endif()

if ($ENV{CFG_PLATFORM} STREQUAL win32)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        boot
        itp_boot
        ith_platform
        ith
        ptw32
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/lib/FTCSPI.lib
        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/lib/Packet.lib
        imm32.lib
        Vfw32.lib
        version.lib
        winmm.lib
        ws2_32.lib
    )

    configure_file(${PROJECT_SOURCE_DIR}/tool/bin/FTCSPI.dll ${CMAKE_CURRENT_BINARY_DIR}/FTCSPI.dll COPYONLY)
    
    if (NOT $ENV{AUTOBUILD})
        if (DEFINED CFG_PRIVATE_DRIVE)
            execute_process(COMMAND subst ${CFG_PRIVATE_DRIVE}: /D ERROR_QUIET)
            execute_process(COMMAND subst ${CFG_PRIVATE_DRIVE}: ${CMAKE_BINARY_DIR}/data/private)
        endif()

        if (DEFINED CFG_PUBLIC_DRIVE)
            execute_process(COMMAND subst ${CFG_PUBLIC_DRIVE}: /D ERROR_QUIET)
            execute_process(COMMAND subst ${CFG_PUBLIC_DRIVE}: ${CMAKE_BINARY_DIR}/data/public)
        endif()

        if (DEFINED CFG_TEMP_DRIVE)
            execute_process(COMMAND subst ${CFG_TEMP_DRIVE}: /D ERROR_QUIET)
            execute_process(COMMAND subst ${CFG_TEMP_DRIVE}: ${CMAKE_BINARY_DIR}/data/temp)
        endif()
    endif()

    file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/debug/win32/*.in)

    foreach (src ${files})
        string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/debug/win32/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
        string(REPLACE ".in" "" dest ${tmp})
        configure_file(${src} ${dest})
    endforeach()

elseif ($ENV{CFG_PLATFORM} STREQUAL openrtos)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        boot
        itp_boot
        ith_platform
        ith
        openrtos
        malloc
    )

    # add risc libraray here to solve linking order issue
    if (DEFINED CFG_AUDIO_ENABLE)
    ITE_LINK_LIBRARY_IF_DEFINED_CFG_BUILD_LIB(risc)
    endif()

# ALT CPU
if (DEFINED CFG_ALT_CPU_ENABLE)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        risc
    )
    if (DEFINED CFG_RSL_MASTER)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        rslMaster
    )
    endif()

    if (DEFINED CFG_RSL_SLAVE)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        rslSlave
    )
    endif()

    if (DEFINED CFG_SW_PWM)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        swPwm
    )
    endif()
endif()

    # remove previous built target file
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME})

    # mkrom arguments
    set(args -max_file 10)

    if (DEFINED CFG_ROM_COMPRESS)
        set(args ${args} -z -b 512K)
    endif()

    if (DEFINED CFG_SD_DUAL_BOOT)
        if (DEFINED CFG_SD0_ENABLE AND NOT DEFINED CFG_SD0_STATIC)
            set(args ${args} -d ${CFG_GPIO_SD0_CARD_DETECT})
        elseif (DEFINED CFG_SD1_ENABLE AND NOT DEFINED CFG_SD1_STATIC)
            set(args ${args} -d ${CFG_GPIO_SD1_CARD_DETECT})
        endif()
    endif()

    if (DEFINED CFG_SPI_NAND_BOOT)
        set(args ${args} -snf ${CFG_NAND_PAGE_SIZE} ${CFG_NAND_BLOCK_SIZE} ${CFG_SPI_NAND_ADDR_TYPE})
    endif()

    # make rom
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${OBJCOPY}
        ARGS -O binary ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
        COMMAND ${READELF}
        ARGS -a ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.symbol
    )

    if ((DEFINED CFG_CPU_FA626) AND ("${CMAKE_PROJECT_NAME}" STREQUAL "bootloader"))

        # remove previous generated script file
        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/init.scr)

        get_directory_property(defs COMPILE_DEFINITIONS)

        foreach (def ${defs})
            set(defargs ${defargs} -D${def})

        endforeach()

        if (CMAKE_BUILD_TYPE STREQUAL Debug)
            set(defargs ${defargs} -g)
        endif()

        if (DEFINED CFG_LCD_INIT_ON_BOOTING)
            add_custom_command(
                TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
                COMMAND ${CMAKE_C_COMPILER}
                    ARGS ${CMAKE_C_COMPILER_ARG1} ${defargs} -mcpu=fa626te -O3 -fPIC
                        -I${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include
                        -I${PROJECT_SOURCE_DIR}/sdk/include
                        -o ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear
                        ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/lcd_clear.c
                        -Ttext 0x80000000 -nostartfiles
                COMMAND ${OBJCOPY}
                    ARGS -O binary ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/lcd_clear.bin
                COMMAND ${READELF}
                    ARGS -a ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear > ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear.symbol
                #COMMAND ${OBJDUMP}
                #    ARGS -d ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear > ${CMAKE_CURRENT_BINARY_DIR}/lcd_clear.dbg
                DEPENDS ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/lcd_clear.c
            )

            if (DEFINED CFG_BACKLIGHT_ENABLE)
                add_custom_command(
                    TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
                    COMMAND ${CMAKE_C_COMPILER}
                        ARGS ${CMAKE_C_COMPILER_ARG1} ${defargs} -mcpu=fa626te -O3 -fPIC
                            -I${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include
                            -I${PROJECT_SOURCE_DIR}/sdk/include
                            -o ${CMAKE_CURRENT_BINARY_DIR}/backlight
                            ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/backlight.c
                            -Ttext 0x80000000 -nostartfiles
                    COMMAND ${OBJCOPY}
                        ARGS -O binary ${CMAKE_CURRENT_BINARY_DIR}/backlight ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backlight.bin
                    COMMAND ${READELF}
                        ARGS -a ${CMAKE_CURRENT_BINARY_DIR}/backlight > ${CMAKE_CURRENT_BINARY_DIR}/backlight.symbol
                    #COMMAND ${OBJDUMP}
                    #    ARGS -d ${CMAKE_CURRENT_BINARY_DIR}/backlight > ${CMAKE_CURRENT_BINARY_DIR}/backlight.dbg
                    DEPENDS ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/backlight.c
                )
            endif()
        endif()

        add_custom_command(
            TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            ARGS
                -DCFG_BACKLIGHT_ENABLE=${CFG_BACKLIGHT_ENABLE}
                -DCFG_LCD_INIT_ON_BOOTING=${CFG_LCD_INIT_ON_BOOTING}
                -DCFG_LCD_MULTIPLE=${CFG_LCD_MULTIPLE}
                -DCFG_LCD_BPP=${CFG_LCD_BPP}
                -DCFG_LCD_BOOT_BITMAP=${CFG_LCD_BOOT_BITMAP}
                -DCFG_LCD_BOOT_BGCOLOR=${CFG_LCD_BOOT_BGCOLOR}
                -DCFG_LCD_HEIGHT=${CFG_LCD_HEIGHT}
                -DCFG_LCD_INIT_SCRIPT=${CFG_LCD_INIT_SCRIPT}
                -DCFG_LCD_PITCH=${CFG_LCD_PITCH}
                -DCFG_LCD_WIDTH=${CFG_LCD_WIDTH}
                -DCFG_RAM_INIT_SCRIPT=${CFG_RAM_INIT_SCRIPT}
                -DCFG_TILING_WIDTH_128=${CFG_TILING_WIDTH_128}
                -DCFG_SET_ROTATE=${CFG_SET_ROTATE}
                -DCFG_BACKLIGHT_BOOTLODER_DELAY=${CFG_BACKLIGHT_BOOTLODER_DELAY}
                -DCPP=${CPP}
                -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -DCMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
                -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                -P ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/init.cmake
            COMMAND mkrom
            ARGS ${args} ${CMAKE_CURRENT_BINARY_DIR}/init.scr ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/kproc.sys
        )

    else()

        add_custom_command(
            TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/kproc.sys
        )

    endif()

    # debug
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${OBJDUMP}
        ARGS -d ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.dbg
    )

    # make pkg
    unset(args)
    unset(packargs)
    unset(pkgdsk)
    unset(bakpkgargs)

    # bootloader
    if (DEFINED CFG_UPGRADE_BOOTLOADER)
        if (DEFINED CFG_UPGRADE_BOOTLOADER_EXTERNAL_PROJECT)
            set(bootloader_path ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_PLATFORM}/bootloader/project/bootloader/kproc.sys)
        else()
            set(bootloader_path ${CMAKE_BINARY_DIR}/project/bootloader/kproc.sys)
        endif()

        set(pack_bootloader_path "./bootloader/kproc.sys")

        if (DEFINED CFG_UPGRADE_BOOTLOADER_NAND)
            set(args ${args} --nand-unformatted-data0 ${bootloader_path})
            set(packargs ${packargs} --nand-unformatted-data0 ${pack_bootloader_path})
            set(pkgdsk ${pkgdsk} --nand-unformatted-data0 ${bootloader_path})

            if (DEFINED CFG_UPGRADE_BOOTLOADER_NAND_BACKUP)
                set(args ${args} --nand-unformatted-data1 ${bootloader_path})
                set(args ${args} --nand-unformatted-data1-pos ${CFG_UPGRADE_BOOTLOADER_NAND_BACKUP_POS})

                set(packargs ${packargs} --nand-unformatted-data1 ${pack_bootloader_path})
                set(packargs ${packargs} --nand-unformatted-data1-pos ${CFG_UPGRADE_BOOTLOADER_NAND_BACKUP_POS})
                
                #set(pkgdsk ${pkgdsk} --nand-unformatted-data1 ${bootloader_path})
                #set(pkgdsk ${pkgdsk} --nand-unformatted-data1-pos ${CFG_UPGRADE_BOOTLOADER_NAND_BACKUP_POS})
            endif()

        elseif (DEFINED CFG_UPGRADE_BOOTLOADER_NOR)
            set(args ${args} --nor-unformatted-data0 ${bootloader_path})
            set(packargs ${packargs} --nor-unformatted-data0 ${pack_bootloader_path})
            #set(pkgdsk ${pkgdsk} --nor-unformatted-data0 ${pack_bootloader_path})
        elseif (DEFINED CFG_UPGRADE_BOOTLOADER_SD0)
            set(args ${args} --sd0-unformatted-data0 ${bootloader_path})
            set(packargs ${packargs} --sd0-unformatted-data0 ${pack_bootloader_path})
            #set(pkgdsk ${pkgdsk} --sd0-unformatted-data0 ${pack_bootloader_path})
        elseif (DEFINED CFG_UPGRADE_BOOTLOADER_SD1)
            set(args ${args} --sd1-unformatted-data0 ${bootloader_path})
            set(packargs ${packargs} --sd1-unformatted-data0 ${pack_bootloader_path})
            #set(pkgdsk ${pkgdsk} --sd1-unformatted-data0 ${pack_bootloader_path})
        endif()
    endif()

    # image
    if (DEFINED CFG_UPGRADE_IMAGE)
        if (NOT (${CMAKE_PROJECT_NAME} STREQUAL "bootloader"))
            if (DEFINED CFG_UPGRADE_IMAGE_NAND)
                if (DEFINED CFG_UPGRADE_BOOTLOADER_NAND_BACKUP)
                    set(args ${args} --nand-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                    set(args ${args} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                    set(packargs ${packargs} --nand-unformatted-data2 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                    set(packargs ${packargs} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                    #set(pkgdsk ${pkgdsk} --nand-unformatted-data2 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                    #set(pkgdsk ${pkgdsk} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                    if (DEFINED CFG_UPGRADE_IMAGE_NAND_BACKUP)
                        set(args ${args} --nand-unformatted-data3 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                        set(args ${args} --nand-unformatted-data3-pos ${CFG_UPGRADE_IMAGE_POS})

                        set(packargs ${packargs} --nand-unformatted-data3 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                        set(packargs ${packargs} --nand-unformatted-data3-pos ${CFG_UPGRADE_IMAGE_POS})

                        #set(pkgdsk ${pkgdsk} --nand-unformatted-data3 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                        #set(pkgdsk ${pkgdsk} --nand-unformatted-data3-pos ${CFG_UPGRADE_IMAGE_POS})

                    endif()
                else()
                    set(args ${args} --nand-unformatted-data1 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                    set(args ${args} --nand-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                    set(packargs ${packargs} --nand-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                    set(packargs ${packargs} --nand-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                    set(pkgdsk ${pkgdsk} --nand-unformatted-data1 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                    set(pkgdsk ${pkgdsk} --nand-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                    set(bakpkgargs ${bakpkgargs} --nand-unformatted-data0 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                    set(bakpkgargs ${bakpkgargs} --nand-unformatted-data0-pos ${CFG_UPGRADE_IMAGE_POS})

                    if (DEFINED CFG_UPGRADE_IMAGE_NAND_BACKUP)
                        set(args ${args} --nand-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                        set(args ${args} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                        set(packargs ${packargs} --nand-unformatted-data2 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                        set(packargs ${packargs} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                        #set(pkgdsk ${pkgdsk} --nand-unformatted-data2 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                        #set(pkgdsk ${pkgdsk} --nand-unformatted-data2-pos ${CFG_UPGRADE_IMAGE_POS})

                    endif()
                endif()
            elseif (DEFINED CFG_UPGRADE_IMAGE_NOR)
                set(args ${args} --nor-unformatted-data1 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(args ${args} --nor-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(packargs ${packargs} --nor-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                set(packargs ${packargs} --nor-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                #set(pkgdsk ${pkgdsk} --nor-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                #set(pkgdsk ${pkgdsk} --nor-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(bakpkgargs ${bakpkgargs} --nor-unformatted-data0 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(bakpkgargs ${bakpkgargs} --nor-unformatted-data0-pos ${CFG_UPGRADE_IMAGE_POS})

            elseif (DEFINED CFG_UPGRADE_IMAGE_SD0)
                set(args ${args} --sd0-unformatted-data1 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(args ${args} --sd0-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(packargs ${packargs} --sd0-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                set(packargs ${packargs} --sd0-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                #set(pkgdsk ${pkgdsk} --sd0-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                #set(pkgdsk ${pkgdsk} --sd0-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(bakpkgargs ${bakpkgargs} --sd0-unformatted-data0 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(bakpkgargs ${bakpkgargs} --sd0-unformatted-data0-pos ${CFG_UPGRADE_IMAGE_POS})

            elseif (DEFINED CFG_UPGRADE_IMAGE_SD1)
                set(args ${args} --sd1-unformatted-data1 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(args ${args} --sd1-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(packargs ${packargs} --sd1-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                set(packargs ${packargs} --sd1-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                #set(pkgdsk ${pkgdsk} --sd1-unformatted-data1 "./${CMAKE_PROJECT_NAME}/kproc.sys")
                #set(pkgdsk ${pkgdsk} --sd1-unformatted-data1-pos ${CFG_UPGRADE_IMAGE_POS})

                set(bakpkgargs ${bakpkgargs} --sd1-unformatted-data0 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/kproc.sys)
                set(bakpkgargs ${bakpkgargs} --sd1-unformatted-data0-pos ${CFG_UPGRADE_IMAGE_POS})

            endif()
        endif()
    endif()

    # backup package
    if (DEFINED CFG_UPGRADE_BACKUP_PACKAGE)
        if (NOT (${CMAKE_PROJECT_NAME} STREQUAL "bootloader"))
            if (DEFINED CFG_UPGRADE_IMAGE_NAND)
                set(args ${args} --nand-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backup.data)
                set(args ${args} --nand-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

                set(packargs ${packargs} --nand-unformatted-data2 "./${CMAKE_PROJECT_NAME}/backup.data")
                set(packargs ${packargs} --nand-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

                set(pkgdsk ${pkgdsk} --nand-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backup.data)
                set(pkgdsk ${pkgdsk} --nand-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

            elseif (DEFINED CFG_UPGRADE_IMAGE_NOR)
                set(args ${args} --nor-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backup.data)
                set(args ${args} --nor-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

                set(packargs ${packargs} --nor-unformatted-data2 "./${CMAKE_PROJECT_NAME}/backup.data")
                set(packargs ${packargs} --nor-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

            elseif (DEFINED CFG_UPGRADE_IMAGE_SD0)
                set(args ${args} --sd0-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backup.data)
                set(args ${args} --sd0-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

                set(packargs ${packargs} --sd0-unformatted-data2 "./${CMAKE_PROJECT_NAME}/backup.data")
                set(packargs ${packargs} --sd0-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

            elseif (DEFINED CFG_UPGRADE_IMAGE_SD1)
                set(args ${args} --sd1-unformatted-data2 ${CMAKE_BINARY_DIR}/project/${CMAKE_PROJECT_NAME}/backup.data)
                set(args ${args} --sd1-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

                set(packargs ${packargs} --sd1-unformatted-data2 "./${CMAKE_PROJECT_NAME}/backup.data")
                set(packargs ${packargs} --sd1-unformatted-data2-pos ${CFG_UPGRADE_BACKUP_PACKAGE_POS})

            endif()
        endif()
    endif()

    # unformatted
    if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_NAND_RESERVED_SIZE)
        set(args ${args} --nand-unformatted-size ${CFG_NAND_RESERVED_SIZE})
        set(packargs ${packargs} --nand-unformatted-size ${CFG_NAND_RESERVED_SIZE})
        set(pkgdsk ${pkgdsk} --nand-unformatted-size ${CFG_NAND_RESERVED_SIZE})
        set(bakpkgargs ${bakpkgargs} --nand-unformatted-size ${CFG_NAND_RESERVED_SIZE})
    endif()
    if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_NOR_RESERVED_SIZE)
        set(args ${args} --nor-unformatted-size ${CFG_NOR_RESERVED_SIZE})
        set(packargs ${packargs} --nor-unformatted-size ${CFG_NOR_RESERVED_SIZE})
        #set(pkgdsk ${pkgdsk} --nor-unformatted-size ${CFG_NOR_RESERVED_SIZE})
        set(bakpkgargs ${bakpkgargs} --nor-unformatted-size ${CFG_NOR_RESERVED_SIZE})
    endif()
    if (DEFINED CFG_SD0_RESERVED_SIZE)
        set(args ${args} --sd0-unformatted-size ${CFG_SD0_RESERVED_SIZE})
        set(packargs ${packargs} --sd0-unformatted-size ${CFG_SD0_RESERVED_SIZE})
        #set(pkgdsk ${pkgdsk} --sd0-unformatted-size ${CFG_SD0_RESERVED_SIZE})
        set(bakpkgargs ${bakpkgargs} --sd0-unformatted-size ${CFG_SD0_RESERVED_SIZE})
    endif()
    if (DEFINED CFG_SD1_RESERVED_SIZE)
        set(args ${args} --sd1-unformatted-size ${CFG_SD1_RESERVED_SIZE})
        set(packargs ${packargs} --sd1-unformatted-size ${CFG_SD1_RESERVED_SIZE})
        #set(pkgdsk ${pkgdsk} --sd1-unformatted-size ${CFG_SD1_RESERVED_SIZE})
        set(bakpkgargs ${bakpkgargs} --sd1-unformatted-size ${CFG_SD1_RESERVED_SIZE})
    endif()

    # data
    if (DEFINED CFG_UPGRADE_DATA)

        # private data
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_UPGRADE_PRIVATE_NAND)
            set(args ${args} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
            set(packargs ${packargs} --nand-partiton0-dir "%CURR_PATH%/data/private")
            set(bakpkgargs ${bakpkgargs} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
        elseif (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_UPGRADE_PRIVATE_NOR)
            set(args ${args} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
            set(packargs ${packargs} --nor-partiton0-dir "%CURR_PATH%/data/private")
            set(bakpkgargs ${bakpkgargs} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
        elseif (DEFINED CFG_UPGRADE_PRIVATE_SD0)
            set(args ${args} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
            set(packargs ${packargs} --sd0-partiton0-dir "%CURR_PATH%/data/private")
            set(bakpkgargs ${bakpkgargs} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
        elseif (DEFINED CFG_UPGRADE_PRIVATE_SD1)
            set(args ${args} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
            set(packargs ${packargs} --sd1-partiton0-dir "%CURR_PATH%/data/private")
            set(bakpkgargs ${bakpkgargs} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/private)
        endif()

        # public data
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_UPGRADE_PUBLIC_NAND)
            if (DEFINED CFG_UPGRADE_PRIVATE_NAND)
                set(args ${args} --nand-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --nand-partiton1-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --nand-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
            else()
                set(args ${args} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --nand-partiton0-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
            endif()
        elseif (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_UPGRADE_PUBLIC_NOR)
            if (DEFINED CFG_UPGRADE_PRIVATE_NOR)
                set(args ${args} --nor-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --nor-partiton1-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --nor-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
            else()
                set(args ${args} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --nor-partiton0-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
            endif()
        elseif (DEFINED CFG_UPGRADE_PUBLIC_SD0)
            if (DEFINED CFG_UPGRADE_PRIVATE_SD0)
                set(args ${args} --sd0-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --sd0-partiton1-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --sd0-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
            else()
                set(args ${args} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --sd0-partiton0-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
            endif()
        elseif (DEFINED CFG_UPGRADE_PUBLIC_SD1)
            if (DEFINED CFG_UPGRADE_PRIVATE_SD0)
                set(args ${args} --sd1-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --sd1-partiton1-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --sd1-partiton1-dir ${CMAKE_BINARY_DIR}/data/public)
            else()
                set(args ${args} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
                set(packargs ${packargs} --sd1-partiton0-dir "%CURR_PATH%/data/public")
                set(bakpkgargs ${bakpkgargs} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/public)
            endif()
        endif()

        # temporary data
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_UPGRADE_TEMP_NAND)
            if (DEFINED CFG_UPGRADE_PRIVATE_NAND AND DEFINED CFG_UPGRADE_PUBLIC_NAND)
                set(args ${args} --nand-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nand-partiton2-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nand-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
            elseif (DEFINED CFG_UPGRADE_PRIVATE_NAND OR DEFINED CFG_UPGRADE_PUBLIC_NAND)
                set(args ${args} --nand-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nand-partiton1-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nand-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
            else()
                set(args ${args} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nand-partiton0-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nand-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
            endif()
        elseif (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_UPGRADE_TEMP_NOR)
            if (DEFINED CFG_UPGRADE_PRIVATE_NOR AND DEFINED CFG_UPGRADE_PUBLIC_NOR)
                set(args ${args} --nor-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nor-partiton2-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nor-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
            elseif (DEFINED CFG_UPGRADE_PRIVATE_NOR OR DEFINED CFG_UPGRADE_PUBLIC_NOR)
                set(args ${args} --nor-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nor-partiton1-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nor-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
            else()
                set(args ${args} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --nor-partiton0-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --nor-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
            endif()
        elseif (DEFINED CFG_UPGRADE_TEMP_SD0)
            if (DEFINED CFG_UPGRADE_PRIVATE_SD0 AND DEFINED CFG_UPGRADE_PUBLIC_SD0)
                set(args ${args} --sd0-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd0-partiton2-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd0-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
            elseif (DEFINED CFG_UPGRADE_PRIVATE_SD0 OR DEFINED CFG_UPGRADE_PUBLIC_SD0)
                set(args ${args} --sd0-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd0-partiton1-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd0-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
            else()
                set(args ${args} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd0-partiton0-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd0-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
            endif()
        elseif (DEFINED CFG_UPGRADE_TEMP_SD1)
            if (DEFINED CFG_UPGRADE_PRIVATE_SD1 AND DEFINED CFG_UPGRADE_PUBLIC_SD1)
                set(args ${args} --sd1-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd1-partiton2-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd1-partiton2-dir ${CMAKE_BINARY_DIR}/data/temp)
            elseif (DEFINED CFG_UPGRADE_PRIVATE_SD1 OR DEFINED CFG_UPGRADE_PUBLIC_SD1)
                set(args ${args} --sd1-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd1-partiton1-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd1-partiton1-dir ${CMAKE_BINARY_DIR}/data/temp)
            else()
                set(args ${args} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
                set(packargs ${packargs} --sd1-partiton0-dir "%CURR_PATH%/data/temp")
                set(bakpkgargs ${bakpkgargs} --sd1-partiton0-dir ${CMAKE_BINARY_DIR}/data/temp)
            endif()
        endif()

        # partition
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_NAND_PARTITION0)
            set(args ${args} --nand-partiton0-size ${CFG_NAND_PARTITION0_SIZE})
            set(packargs ${packargs} --nand-partiton0-size ${CFG_NAND_PARTITION0_SIZE})
            set(bakpkgargs ${bakpkgargs} --nand-partiton0-size ${CFG_NAND_PARTITION0_SIZE})
        endif()
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_NAND_PARTITION1)
            set(args ${args} --nand-partiton1-size ${CFG_NAND_PARTITION1_SIZE})
            set(packargs ${packargs} --nand-partiton1-size ${CFG_NAND_PARTITION1_SIZE})
            set(bakpkgargs ${bakpkgargs} --nand-partiton1-size ${CFG_NAND_PARTITION1_SIZE})
        endif()
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_NAND_PARTITION2)
            set(args ${args} --nand-partiton2-size ${CFG_NAND_PARTITION2_SIZE})
            set(packargs ${packargs} --nand-partiton2-size ${CFG_NAND_PARTITION2_SIZE})
            set(bakpkgargs ${bakpkgargs} --nand-partiton2-size ${CFG_NAND_PARTITION2_SIZE})
        endif()
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_NAND_PARTITION3)
            set(args ${args} --nand-partiton3-size ${CFG_NAND_PARTITION3_SIZE})
            set(packargs ${packargs} --nand-partiton3-size ${CFG_NAND_PARTITION3_SIZE})
            set(bakpkgargs ${bakpkgargs} --nand-partiton3-size ${CFG_NAND_PARTITION3_SIZE})
        endif()
        if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_NOR_PARTITION0)
            set(args ${args} --nor-partiton0-size ${CFG_NOR_PARTITION0_SIZE})
            set(packargs ${packargs} --nor-partiton0-size ${CFG_NOR_PARTITION0_SIZE})
            set(bakpkgargs ${bakpkgargs} --nor-partiton0-size ${CFG_NOR_PARTITION0_SIZE})
        endif()
        if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_NOR_PARTITION1)
            set(args ${args} --nor-partiton1-size ${CFG_NOR_PARTITION1_SIZE})
            set(packargs ${packargs} --nor-partiton1-size ${CFG_NOR_PARTITION1_SIZE})
            set(bakpkgargs ${bakpkgargs} --nor-partiton1-size ${CFG_NOR_PARTITION1_SIZE})
        endif()
        if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_NOR_PARTITION2)
            set(args ${args} --nor-partiton2-size ${CFG_NOR_PARTITION2_SIZE})
            set(packargs ${packargs} --nor-partiton2-size ${CFG_NOR_PARTITION2_SIZE})
            set(bakpkgargs ${bakpkgargs} --nor-partiton2-size ${CFG_NOR_PARTITION2_SIZE})
        endif()
        if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_NOR_PARTITION3)
            set(args ${args} --nor-partiton3-size ${CFG_NOR_PARTITION3_SIZE})
            set(packargs ${packargs} --nor-partiton3-size ${CFG_NOR_PARTITION3_SIZE})
            set(bakpkgargs ${bakpkgargs} --nor-partiton3-size ${CFG_NOR_PARTITION3_SIZE})
        endif()
        if (DEFINED CFG_SD0_PARTITION0)
            set(args ${args} --sd0-partiton0-size ${CFG_SD0_PARTITION0_SIZE})
            set(packargs ${packargs} --sd0-partiton0-size ${CFG_SD0_PARTITION0_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd0-partiton0-size ${CFG_SD0_PARTITION0_SIZE})
        endif()
        if (DEFINED CFG_SD0_PARTITION1)
            set(args ${args} --sd0-partiton1-size ${CFG_SD0_PARTITION1_SIZE})
            set(packargs ${packargs} --sd0-partiton1-size ${CFG_SD0_PARTITION1_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd0-partiton1-size ${CFG_SD0_PARTITION1_SIZE})
        endif()
        if (DEFINED CFG_SD0_PARTITION2)
            set(args ${args} --sd0-partiton2-size ${CFG_SD0_PARTITION2_SIZE})
            set(packargs ${packargs} --sd0-partiton2-size ${CFG_SD0_PARTITION2_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd0-partiton2-size ${CFG_SD0_PARTITION2_SIZE})
        endif()
        if (DEFINED CFG_SD0_PARTITION3)
            set(args ${args} --sd0-partiton3-size ${CFG_SD0_PARTITION3_SIZE})
            set(packargs ${packargs} --sd0-partiton3-size ${CFG_SD0_PARTITION3_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd0-partiton3-size ${CFG_SD0_PARTITION3_SIZE})
        endif()
        if (DEFINED CFG_SD1_PARTITION0)
            set(args ${args} --sd1-partiton0-size ${CFG_SD1_PARTITION0_SIZE})
            set(packargs ${packargs} --sd1-partiton0-size ${CFG_SD1_PARTITION0_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd1-partiton0-size ${CFG_SD1_PARTITION0_SIZE})
        endif()
        if (DEFINED CFG_SD1_PARTITION1)
            set(args ${args} --sd1-partiton1-size ${CFG_SD1_PARTITION1_SIZE})
            set(packargs ${packargs} --sd1-partiton1-size ${CFG_SD1_PARTITION1_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd1-partiton1-size ${CFG_SD1_PARTITION1_SIZE})
        endif()
        if (DEFINED CFG_SD1_PARTITION2)
            set(args ${args} --sd1-partiton2-size ${CFG_SD1_PARTITION2_SIZE})
            set(packargs ${packargs} --sd1-partiton2-size ${CFG_SD1_PARTITION2_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd1-partiton2-size ${CFG_SD1_PARTITION2_SIZE})
        endif()
        if (DEFINED CFG_SD1_PARTITION3)
            set(args ${args} --sd1-partiton3-size ${CFG_SD1_PARTITION3_SIZE})
            set(packargs ${packargs} --sd1-partiton3-size ${CFG_SD1_PARTITION3_SIZE})
            set(bakpkgargs ${bakpkgargs} --sd1-partiton3-size ${CFG_SD1_PARTITION3_SIZE})
        endif()

        if (DEFINED CFG_UPGRADE_PARTITION)
            set(args ${args} --partition)
            set(packargs ${packargs} --partition)
            set(bakpkgargs ${bakpkgargs} --partition)
        endif()
        
        if (DEFINED CFG_UPGRADE_FORMAT_PARTITION)
            set(args ${args} --format-partition)
            set(packargs ${packargs} --format-partition)
            set(bakpkgargs ${bakpkgargs} --format-partition)
        endif()        
    endif()

    if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_UPGRADE_NAND_IMAGE AND NOT (${CMAKE_PROJECT_NAME} STREQUAL bootloader))
        set(pkgdsk ${pkgdsk} --nand-unformatted-data2 ${CFG_UPGRADE_NAND_IMAGE_FILENAME})
        set(pkgdsk ${pkgdsk} --nand-unformatted-data2-pos ${CFG_NAND_RESERVED_SIZE})
    endif()
        
    if (DEFINED CFG_UPGRADE_PACKAGE_VERSION)
        set(args ${args} --version ${CFG_UPGRADE_PACKAGE_VERSION})
        set(packargs ${packargs} --version ${CFG_UPGRADE_PACKAGE_VERSION})
        set(pkgdsk ${pkgdsk} --version ${CFG_UPGRADE_PACKAGE_VERSION})
        set(bakpkgargs ${bakpkgargs} --version ${CFG_UPGRADE_PACKAGE_VERSION})
    endif()

    if (DEFINED CFG_UPGRADE_BOOTLOADER OR DEFINED CFG_UPGRADE_IMAGE OR DEFINED CFG_UPGRADE_DATA)
    
        if (DEFINED CFG_UPGRADE_BACKUP_PACKAGE)
            message("pkgtool ${bakpkgargs}")
            
            add_custom_command(
                TARGET ${CMAKE_PROJECT_NAME}
                POST_BUILD
                COMMAND pkgtool
                ARGS -o ${CFG_UPGRADE_FILENAME}_BAK ${bakpkgargs} --key ${CFG_UPGRADE_ENC_KEY}
                COMMAND pkgtool
                ARGS -l ${CFG_UPGRADE_FILENAME}_BAK
                COMMAND fs
                ARGS ${CFG_UPGRADE_FILENAME}_BAK backup.data
            )
        endif()
    
        message("pkgtool ${args}")
        add_custom_command(
            TARGET ${CMAKE_PROJECT_NAME}
            POST_BUILD
            COMMAND pkgtool
            ARGS -o ${CFG_UPGRADE_FILENAME} ${args} --key ${CFG_UPGRADE_ENC_KEY}
            COMMAND pkgtool
            ARGS -l ${CFG_UPGRADE_FILENAME}
        )

        if (DEFINED CFG_NOR_ENABLE AND DEFINED CFG_UPGRADE_NOR_IMAGE AND NOT (${CMAKE_PROJECT_NAME} STREQUAL bootloader))
            add_custom_command(
                TARGET ${CMAKE_PROJECT_NAME}
                POST_BUILD
                COMMAND pkgtool
                ARGS -s ${CFG_UPGRADE_NOR_IMAGE_SIZE} -o ${CFG_UPGRADE_NOR_IMAGE_FILENAME} -n ${CFG_UPGRADE_FILENAME}
            )
        endif()
        
        if (DEFINED CFG_NAND_ENABLE AND DEFINED CFG_UPGRADE_NAND_IMAGE AND NOT (${CMAKE_PROJECT_NAME} STREQUAL bootloader))
            add_custom_command(
                TARGET ${CMAKE_PROJECT_NAME}
                POST_BUILD
                COMMAND pkgtool
                ARGS -s ${CFG_UPGRADE_NAND_IMAGE_SIZE} -o ${CFG_UPGRADE_NAND_IMAGE_FILENAME} -N ${CFG_UPGRADE_FILENAME} -e ${CFG_UPGRADE_NAND_SECTOR_SIZE}
            )

            # parse partitions as disk image
            add_custom_command(
                TARGET ${CMAKE_PROJECT_NAME}
                POST_BUILD
                COMMAND pkgtool
                ARGS -t -o ${CFG_UPGRADE_NAND_IMAGE_FILENAME} -M ${CFG_UPGRADE_FILENAME} -e ${CFG_UPGRADE_NAND_SECTOR_SIZE} -s ${CFG_UPGRADE_NAND_IMAGE_SIZE}
            )

            # build PKG file with disk image
	        add_custom_command(
	            TARGET ${CMAKE_PROJECT_NAME}
	            POST_BUILD
	            COMMAND pkgtool
	            ARGS -o ${CFG_UPGRADE_FILENAME} ${pkgdsk} --key ${CFG_UPGRADE_ENC_KEY}
	            COMMAND pkgtool
	            ARGS -l ${CFG_UPGRADE_FILENAME}
	        )
        endif()   
        
    endif()

    file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/debug/*.in)

    foreach (src ${files})
        string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/debug/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
        string(REPLACE ".in" "" dest ${tmp})
        configure_file(${src} ${dest})
    endforeach()

    if (DEFINED CFG_CPU_FA626)
        file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/debug/fa626/*.in)

        foreach (src ${files})
            string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/debug/fa626/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
            string(REPLACE ".in" "" dest ${tmp})
            configure_file(${src} ${dest})
        endforeach()

    elseif (DEFINED CFG_CPU_SM32)
        file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/debug/sm32/*.in)

        foreach (src ${files})
            string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/debug/sm32/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
            string(REPLACE ".in" "" dest ${tmp})
            configure_file(${src} ${dest})
        endforeach()

    elseif (DEFINED CFG_CPU_RISCV)
        file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/debug/riscv/*.in)

        foreach (src ${files})
            string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/debug/riscv/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
            string(REPLACE ".in" "" dest ${tmp})
            configure_file(${src} ${dest})
        endforeach()

    endif()

    configure_file(${PROJECT_SOURCE_DIR}/sdk/target/debug/cvd.csf.in ${CMAKE_BINARY_DIR}/cvd.csf)

endif()

if (DEFINED CFG_GENERATE_DOC)
    configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)
    execute_process(COMMAND doxygen ${CMAKE_BINARY_DIR}/Doxyfile)
endif()

if ((DEFINED CFG_GENERATE_PACK_ENV) AND NOT (${CMAKE_PROJECT_NAME} STREQUAL bootloader))
    set(CFG_PACK_NAME "${CMAKE_PROJECT_NAME}_${CFG_VERSION_MAJOR}${CFG_VERSION_MINOR}${CFG_VERSION_PATCH}${CFG_VERSION_CUSTOM}${CFG_VERSION_TWEAK}")

    unset(args)

    foreach (def ${packargs})
        set(args "${args} ${def}")
    endforeach()

    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS
            -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
            -DCFG_PACK_NAME=${CFG_PACK_NAME}
            -DCMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
            -Dbootloader_path=${bootloader_path}
            -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DCFG_UPGRADE_FILENAME=${CFG_UPGRADE_FILENAME}
            -Dargs=${args}
            -DCFG_UPGRADE_ENC_KEY=${CFG_UPGRADE_ENC_KEY}
            -DCFG_UPGRADE_NOR_IMAGE_SIZE=${CFG_UPGRADE_NOR_IMAGE_SIZE}
            -DCFG_UPGRADE_NOR_IMAGE_FILENAME=${CFG_UPGRADE_NOR_IMAGE_FILENAME}
            -P ${PROJECT_SOURCE_DIR}/sdk/target/sdk/pack.cmake
    )

endif()

if ($ENV{CFG_PLATFORM} STREQUAL openrtos AND DEFINED CFG_DBG_TRACE_ANALYZER)
    if (DEFINED CFG_DBG_VCD)
        set(DUMPVCD "1")
    else()
        set(DUMPVCD "0")
    endif()

    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS
            -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
            -DCMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
            -DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DDUMPVCD=${DUMPVCD}
            -P ${PROJECT_SOURCE_DIR}/sdk/target/trace/trace.cmake
    )
endif()


if (DEFINED CFG_ITH_ERR)
    add_definitions(
        -DCFG_ITH_ERR
    )
endif()

if (DEFINED CFG_ITH_WARN)
    add_definitions(
        -DCFG_ITH_WARN
    )
endif()

if (DEFINED CFG_ITH_INFO)
    add_definitions(
        -DCFG_ITH_INFO
    )
endif()

if (DEFINED CFG_ITH_DBG)
    add_definitions(
        -DCFG_ITH_DBG
    )
endif()

if (DEFINED CFG_ITH_FPGA)
    add_definitions(
        -DCFG_ITH_FPGA
        -DCFG_ITH_FPGA_CLK_CPU=${CFG_ITH_FPGA_CLK_CPU}
        -DCFG_ITH_FPGA_CLK_MEM=${CFG_ITH_FPGA_CLK_MEM}
        -DCFG_ITH_FPGA_CLK_BUS=${CFG_ITH_FPGA_CLK_BUS}
    )
endif()

if (DEFINED CFG_BUILD_MEDIASTREAMER2)
    add_definitions(
        -DCFG_BUILD_MEDIASTREAMER2
    )
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/include
)

add_library(ith STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_defs.h
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith.h
    ith_cfg.h
    ith_chipid.c
    ith_clock.c
    ith_isp.c
    ith_dma.c
    ith_host.c
    ith_interrupt.c
    ith_interrupt_handler.c
    ith_printf.c
    ith_pwm.c
    ith_timer.c
    ith_utility.c
    #ith_codec.c
)

include_directories(
    ${CMAKE_BINARY_DIR}/lib/sm32
)

if (DEFINED CFG_CMDQ_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9920)
        add_library(ith STATIC
        it9920/ith_cmdq.c
        it9920/ith_cmdq.h
    )
    else()
        add_library(ith STATIC
        it9850/ith_cmdq.c
        it9850/ith_cmdq.h
    )
    endif()
endif()

if (DEFINED CFG_KEYPAD_ENABLE)
    add_library(ith STATIC
        ith_keypad.c
    )
endif()


if (DEFINED CFG_LCD_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9850)
        add_library(ith STATIC
        it9850/ith_lcd.c
        it9850/ith_lcd.h
    )
    elseif (CFG_CHIP_FAMILY STREQUAL 9920)
        add_library(ith STATIC
        it9920/ith_lcd.c
        it9920/ith_lcd.h
    )
    else()
        add_library(ith STATIC
        it9070/ith_lcd.c
        it9070/ith_lcd.h
    )
    endif()
endif()

if (DEFINED CFG_RTC_ENABLE)
    add_library(ith STATIC
        ith_rtc.c
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9920)
    add_library(ith STATIC
        it9920/ith_gpio.c
        it9920/ith_gpio_handler.c
    )
elseif (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(ith STATIC
        it9850/ith_gpio.c
        it9850/ith_gpio_handler.c
        it9850/ith_pcr.c
    )
elseif(CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910)
    add_library(ith STATIC
        it9070/ith_gpio.c
        it9070/ith_gpio_handler.c
        it9070/ith_fpc.c
    )
endif()
if (DEFINED CFG_UART0_ENABLE OR DEFINED CFG_UART1_ENABLE OR DEFINED CFG_RS485_0_ENABLE OR DEFINED CFG_RS485_1_ENABLE)
    add_library(ith STATIC
        ith_uart.c
    )
endif()

if (DEFINED CFG_TVOUT_ENABLE)
    add_library(ith STATIC
        ith_tve.c
    )
endif()

if (DEFINED CFG_VIDEO_ENABLE)
    add_library(ith STATIC
        ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_vregs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/ite/ith_video.h
        ith_video.c
    )

    if (CFG_CHIP_FAMILY STREQUAL 9920) #temp to do this.
    add_library(ith STATIC
        it9920/ith_cap.c
    )
    endif()
endif()

if (DEFINED CFG_JPEG_HW_ENABLE)
    add_library(ith STATIC
        ith_jpeg.c
    )
endif()

if (DEFINED CFG_UI_ENC_ENABLE)
    add_library(ith STATIC
        ith_uiEnc.c
    )
endif()

if (DEFINED CFG_BUILD_CAPTURE_MODULE)
	if (CFG_CHIP_FAMILY STREQUAL 9920)
    add_library(ith STATIC
        it9920/ith_cap.c
    )
	elseif (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(ith STATIC
        it9850/ith_cap.c
    )
	elseif(CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910)
    add_library(ith STATIC
        it9070/ith_cap.c
    )
	endif()
endif()

if (DEFINED CFG_OPENVG_ENABLE)
    add_library(ith STATIC
        ith_openvg.c
    )
endif()

if (DEFINED CFG_SD0_ENABLE OR DEFINED CFG_SD1_ENABLE OR DEFINED CFG_CF_ENABLE OR DEFINED CFG_MS_ENABLE OR DEFINED CFG_XD_ENABLE OR DEFINED CFG_NOR_ENABLE OR DEFINED CFG_NAND_ENABLE)
    add_library(ith STATIC
        ith_card.c
    )
endif()

if (DEFINED CFG_USB0_ENABLE OR DEFINED CFG_USB1_ENABLE)
    add_library(ith STATIC
        ith_usb.c
    )
endif()

if (DEFINED CFG_WATCHDOG_ENABLE)
    add_library(ith STATIC
        ith_watchdog.c
    )
endif()

if (DEFINED CFG_DCPS_ENABLE)
    add_library(ith STATIC
        ith_dcps.c
    )
endif()

if (DEFINED CFG_DPU_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9850)
        add_library(ith STATIC
            it9850/ith_dpu.c
        )
    else()    
        add_library(ith STATIC
            it9070/ith_dpu.c
        )
    endif()
endif()

if (DEFINED CFG_NAND_ENABLE)
    add_library(ith STATIC
        ith_nand.c
    )
endif()

if (DEFINED CFG_XD_ENABLE)
    add_library(ith STATIC
        ith_xd.c
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/driver/ith
		${PROJECT_SOURCE_DIR}/sdk/driver/ith/it9850
    )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/driver/ith
		${PROJECT_SOURCE_DIR}/sdk/driver/ith/it9070
    )
endif()

if (DEFINED CFG_IR_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9850)
    add_library(ith STATIC
        it9850/ith_ir.c
    )
    else()    
    add_library(ith STATIC
        it9070/ith_ir.c
    )
    endif()
endif()

if (DEFINED CFG_CPU_WB OR $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(ith STATIC
        ith_vmem.c
    )
endif()

if ($ENV{CFG_PLATFORM} STREQUAL win32)

    execute_process(COMMAND mkscript ${PROJECT_SOURCE_DIR}/sdk/target/debug/${CFG_DBG_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/ram.inc)
    
    add_library(ith_platform STATIC
        ith_win32.c
    )

    if (DEFINED CFG_WIN32_SIMULATOR)
        add_library(ith_platform STATIC
            ith_win32_sim.c
        )
    else()
        add_library(ith_platform STATIC
            ith_win32_spi.c
        )
    endif()

else()

    add_library(ith_platform STATIC
        ith_openrtos.c
    )

endif()

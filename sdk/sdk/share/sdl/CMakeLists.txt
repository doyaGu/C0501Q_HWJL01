if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
    )
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/include/SDL
)

add_library(sdl STATIC
    SDL.c
    SDL_assert.c
    SDL_assert_c.h
    SDL_error.c
    SDL_error_c.h
    SDL_fatal.c
    SDL_fatal.h
    SDL_hints.c
    SDL_log.c
    atomic/SDL_atomic.c
    atomic/SDL_spinlock.c
    cpuinfo/SDL_cpuinfo.c
    events/blank_cursor.h
    events/default_cursor.h
    events/scancodes_windows.h
    events/SDL_clipboardevents.c
    events/SDL_clipboardevents_c.h
    events/SDL_events.c
    events/SDL_events_c.h
    events/SDL_gesture.c
    events/SDL_gesture_c.h
    events/SDL_keyboard.c
    events/SDL_keyboard_c.h
    events/SDL_mouse.c
    events/SDL_mouse_c.h
    events/SDL_quit.c
    events/SDL_sysevents.h
    events/SDL_touch.c
    events/SDL_touch_c.h
    events/SDL_windowevents.c
    events/SDL_windowevents_c.h
    file/SDL_rwops.c
    render/SDL_render.c
    render/SDL_sysrender.h
    render/SDL_yuv_sw.c
    render/SDL_yuv_sw_c.h
    #stdlib/SDL_malloc.c
    #stdlib/SDL_qsort.c
    #stdlib/SDL_stdlib.c            
    stdlib/SDL_string.c
    thread/SDL_systhread.h
    thread/SDL_thread.c
    thread/SDL_thread_c.h
    timer/SDL_timer.c
    timer/SDL_timer_c.h
    video/SDL_blit.c
    video/SDL_blit.h
    video/SDL_blit_0.c
    video/SDL_blit_1.c
    video/SDL_blit_A.c
    video/SDL_blit_auto.c
    video/SDL_blit_auto.h
    video/SDL_blit_copy.c
    video/SDL_blit_copy.h
    video/SDL_blit_N.c
    video/SDL_blit_slow.c
    video/SDL_blit_slow.h
    #video/SDL_bmp.c
    #video/SDL_clipboard.c
    video/SDL_fillrect.c
    video/SDL_pixels.c
    video/SDL_pixels_c.h
    video/SDL_rect.c
    video/SDL_rect_c.h
    video/SDL_RLEaccel.c
    video/SDL_RLEaccel_c.h
    video/SDL_stretch.c
    video/SDL_surface.c
    video/SDL_sysvideo.h
    video/SDL_video.c
)

if (DEFINED CFG_AUDIO_ENABLE)
    add_library(sdl STATIC
        audio/castor3/main_processor_message_queue.c
        #audio/SDL_audio.c
        #audio/SDL_audiocvt.c
        #audio/SDL_audiodev.c
        #audio/SDL_audiodev_c.h
        #audio/SDL_audiomem.h
        #audio/SDL_audiotypecvt.c
        #audio/SDL_audio_c.h
        #audio/SDL_mixer.c
        #audio/SDL_sysaudio.h
        #audio/SDL_wave.c
        #audio/SDL_wave.h
        #audio/dummy/SDL_dummyaudio.c
        #audio/dummy/SDL_dummyaudio.h        
    )
endif()

if (DEFINED CFG_TOUCH_ENABLE)
    add_library(sdl STATIC
        video/castor3/SDL_castor3touch.c
        video/castor3/SDL_castor3touch.h
    )
endif()

if (DEFINED CFG_WIN32_SIMULATOR)
    add_library(sdl STATIC
        power/windows/SDL_syspower.c
    )
else()
    add_library(sdl STATIC
        video/castor3/SDL_castor3events.c
        video/castor3/SDL_castor3events_c.h
        video/castor3/SDL_castor3video.c
        video/castor3/SDL_castor3video.h
        video/castor3/SDL_castor3window.c
        video/castor3/SDL_castor3window.h
    )

    if (DEFINED CFG_LCD_ENABLE)
        add_library(sdl STATIC
            video/castor3/SDL_castor3framebuffer.c
            video/castor3/SDL_castor3framebuffer_c.h
        )
    endif()

    if (DEFINED CFG_OPENVG_ENABLE)
        add_library(sdl STATIC
            render/openvg/SDL_render_ovg.c
            render/software/SDL_blendfillrect.c
            render/software/SDL_blendfillrect.h
            render/software/SDL_blendline.c
            render/software/SDL_blendline.h
            render/software/SDL_blendpoint.c
            render/software/SDL_blendpoint.h
            render/software/SDL_draw.h
            render/software/SDL_drawline.c
            render/software/SDL_drawline.h
            render/software/SDL_drawpoint.c
            render/software/SDL_drawpoint.h
            render/software/SDL_render_sw.c
            render/software/SDL_render_sw_c.h
            render/software/SDL_rotate.c
            render/software/SDL_rotate.h            
            video/castor3/SDL_castor3openvg.c
            video/castor3/SDL_castor3openvg.h
        )
    endif()

    if (DEFINED CFG_KEYPAD_ENABLE)
        add_library(sdl STATIC
            video/castor3/SDL_castor3keypad.c
            video/castor3/SDL_castor3keypad.h
        )

        configure_file(${PROJECT_SOURCE_DIR}/sdk/target/keypad/${CFG_KEYPAD_MAPPING_TABLE} ${CMAKE_CURRENT_BINARY_DIR}/keypad_mapping_table.inc)

    endif()

    if (DEFINED CFG_IR_ENABLE)
        add_library(sdl STATIC
            video/castor3/SDL_castor3ir.c
            video/castor3/SDL_castor3ir.h
        )

        configure_file(${PROJECT_SOURCE_DIR}/sdk/target/ir/${CFG_IR_MAPPING_TABLE} ${CMAKE_CURRENT_BINARY_DIR}/ir_mapping_table.inc)

    endif()

    if (DEFINED CFG_POWER_ENABLE)
        add_library(sdl STATIC
            #power/SDL_power.c
            #power/castor3/SDL_syspower.c
        )
    endif()

    if (DEFINED CFG_USB_MOUSE)
        add_library(sdl STATIC
            video/castor3/SDL_castor3mouse.c
            video/castor3/SDL_castor3mouse.h
        )
    endif()

    if (DEFINED CFG_USB_KBD)
        add_library(sdl STATIC
            video/castor3/SDL_castor3keyboard.c
            video/castor3/SDL_castor3keyboard.h
        )
    endif()

endif()

if ($ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(sdl STATIC
        audio/winmm/SDL_winmm.c
        audio/winmm/SDL_winmm.h
        core/windows/SDL_windows.c
        core/windows/SDL_windows.h
        loadso/windows/SDL_sysloadso.c
        render/SDL_yuv_mmx.c
        stdlib/SDL_getenv.c
        stdlib/SDL_iconv.c
        thread/windows/SDL_sysmutex.c
        thread/windows/SDL_syssem.c
        thread/windows/SDL_systhread.c
        thread/windows/SDL_systhread_c.h
        timer/windows/SDL_systimer.c
        video/SDL_shape.c
        video/SDL_shape_internals.h        
        video/windows/SDL_msctf.h
        video/windows/SDL_vkeys.h
        video/windows/SDL_windowsclipboard.c
        video/windows/SDL_windowsclipboard.h
        video/windows/SDL_windowsevents.c
        video/windows/SDL_windowsevents.h
        video/windows/SDL_windowsframebuffer.c
        video/windows/SDL_windowsframebuffer.h
        video/windows/SDL_windowskeyboard.c
        video/windows/SDL_windowskeyboard.h
        video/windows/SDL_windowsmodes.c
        video/windows/SDL_windowsmodes.h
        video/windows/SDL_windowsmouse.c
        video/windows/SDL_windowsmouse.h
        video/windows/SDL_windowsopenvg.c
        video/windows/SDL_windowsopenvg.h
        video/windows/SDL_windowsshape.c
        video/windows/SDL_windowsshape.h
        video/windows/SDL_windowsvideo.c
        video/windows/SDL_windowsvideo.h
        video/windows/SDL_windowswindow.c
        video/windows/SDL_windowswindow.h
        video/windows/wmmsg.h
    )

    add_library(sdl_main STATIC
        main/windows/SDL_windows_main.c
        main/windows/version.rc
    )

elseif ($ENV{CFG_PLATFORM} STREQUAL openrtos)
    add_library(sdl STATIC
        #loadso/dlopen/SDL_sysloadso.c    
        thread/generic/SDL_syscond.c
        thread/openrtos/SDL_sysmutex.c
        thread/openrtos/SDL_sysmutex_c.h
        thread/openrtos/SDL_syssem.c
        thread/openrtos/SDL_systhread.c
        thread/openrtos/SDL_systhread_c.h
        timer/openrtos/SDL_systimer.c
    )

    add_library(sdl_main STATIC
        main/openrtos/SDL_openrtos_main.c
    )

endif()

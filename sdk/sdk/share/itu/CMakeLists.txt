add_definitions(
    -DFT2_BUILD_LIBRARY
    )

if (DEFINED CFG_ITU_ERR)
    add_definitions(
        -DCFG_ITU_ERR
   )
endif()

if (DEFINED CFG_ITU_WARN)
    add_definitions(
        -DCFG_ITU_WARN
   )
endif()

if (DEFINED CFG_ITU_INFO)
    add_definitions(
        -DCFG_ITU_INFO
    )
endif()

if (DEFINED CFG_ITU_DBG)
    add_definitions(
        -DCFG_ITU_DBG
    )
endif()

if (DEFINED CFG_ITU_LOAD)
    add_definitions(
        -DCFG_ITU_LOAD
    )
endif()

if (DEFINED CFG_ITU_UPDATE)
    add_definitions(
        -DCFG_ITU_UPDATE
    )
endif()

if (DEFINED CFG_ITU_DRAW)
    add_definitions(
        -DCFG_ITU_DRAW
    )
endif()

if (DEFINED CFG_ITU_FT_CACHE_ENABLE)
    add_definitions(
        -DCFG_ITU_FT_CACHE_SIZE=${CFG_ITU_FT_CACHE_SIZE}
    )

    add_library(itu STATIC
        itu_freetype_cache.c
    )
else()
    add_library(itu STATIC
        itu_freetype.c
    )
endif()

if (DEFINED CFG_ITU_UCL_ENABLE)
    add_definitions(
        -DCFG_ITU_UCL_ENABLE
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/itu
)

if (CFG_CHIP_FAMILY STREQUAL 9850)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9850
    )
else()
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/itu/it9070
    )
endif()

add_library(itu STATIC
    ${PROJECT_SOURCE_DIR}/sdk/include/ite/itu.h
    itu_action.c
    itu_analogclock.c
    itu_animation.c
    itu_audio.c
    itu_background.c
    itu_backgroundbutton.c
    itu_base.c
    itu_blur.c
    itu_borderwindow.c
    itu_button.c
    itu_calendar.c
    itu_checkbox.c
    itu_cfg.h
    itu_colorpicker.c
    itu_container.c
    itu_circleprogressbar.c
    itu_coverflow.c
    itu_curve.c
    itu_effect.c
    itu_digitalclock.c
    itu_dragicon.c
    itu_drawpen.c
    itu_fadeeffect.c
    itu_filelistbox.c
    itu_flowwindow.c
    itu_icon.c
    itu_iconlistbox.c
    itu_imagecoverflow.c
    itu_jpeg.c
    itu_keyboard.c
    itu_languagesprite.c
    itu_layer.c
    itu_listbox.c
    itu_mediafilelistbox.c
    itu_meter.c
    itu_pageflow.c
    itu_png.c
    itu_popupbutton.c
    itu_popupradiobox.c
    itu_progressbar.c
    itu_radiobox.c
    itu_ripplebackground.c
    itu_scalecoverflow.c
    itu_scaleeffect.c
    itu_scalefadeeffect.c
    itu_scene.c
    itu_scrollbar.c
    itu_scrolleffect.c
    itu_scrollfadeeffect.c
    itu_scrolliconlistbox.c
    itu_scrolllistbox.c
    itu_scrollmediafilelistbox.c
    itu_scrolltext.c
    itu_shadow.c
    itu_slideshow.c
    itu_sprite.c
    itu_spritebutton.c
    itu_stringset.c
    itu_surface.c
    itu_tablelistbox.c
    itu_tableiconlistbox.c
    itu_text.c
    itu_textbox.c
    itu_trackbar.c
    itu_utility.c
    itu_video.c
    itu_wheel.c
    itu_wheelbackground.c
    itu_widget.c
    itu_wipeeffect.c
)

if (DEFINED CFG_VIDEO_ENABLE)
    include_directories(
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavcodec
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavformat
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavdevice
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavutil
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswresample
        ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswscale
        ${PROJECT_SOURCE_DIR}/sdk/driver/itv
    )
    if (CFG_CHIP_FAMILY STREQUAL 9850)
        if (CFG_VIDEO_FRAME_FUN_ENABLE)
             add_library(itu STATIC
                it9850/itu_framefunc_ff.c
             )
         else()
            add_library(itu STATIC
                it9850/itu_framefunc.c
            )
        endif()
    else()
        add_library(itu STATIC
             it9070/itu_framefunc.c
        )
    endif()
endif()

if (DEFINED CFG_LCD_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9850 AND DEFINED CFG_M2D_ENABLE)
        add_library(itu STATIC
            it9850/itu_lcd.c
        )
    else()
        add_library(itu STATIC
            it9070/itu_lcd.c
        )
    endif()

endif()

if (DEFINED CFG_M2D_ENABLE)
    if (CFG_CHIP_FAMILY STREQUAL 9850)
        add_library(itu STATIC
            it9850/itu_m2d.c
            it9850/itu_private.h
        )
    else()
        add_library(itu STATIC
            it9070/itu_m2d.c
            it9070/itu_private.h
        )
    endif()
else()
    add_library(itu STATIC
        itu_sw.c
    )
endif()

if (DEFINED CFG_STNLCD_ENABLE)
    add_library(itu STATIC
        itu_stnlcd.c
   )
endif()

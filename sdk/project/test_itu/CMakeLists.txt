include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
)

if (DEFINED CFG_TEST_SDL)
    add_executable(${CMAKE_PROJECT_NAME}
        test_sdl.c
        )
elseif (DEFINED CFG_TEST_STNLCD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_$ENV{CFG_PLATFORM}.c
        test_stnlcd.c
        )
elseif (DEFINED CFG_TEST_HELLOWORLD)
    add_executable(${CMAKE_PROJECT_NAME}
        test_$ENV{CFG_PLATFORM}.c
        test_helloworld.c
        )
elseif (DEFINED CFG_TEST_BUTTON)
    add_executable(${CMAKE_PROJECT_NAME}
        test_button.c
        )
elseif (DEFINED CFG_TEST_LANGUAGE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_language.c
        )
elseif (DEFINED CFG_TEST_CURSOR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_cursor.c
        )
elseif (DEFINED CFG_TEST_PROGRESSBAR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_progressbar.c
        )
elseif (DEFINED CFG_TEST_LISTBOX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_listbox.c
        )
elseif (DEFINED CFG_TEST_TRACKBAR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_trackbar.c
        )
elseif (DEFINED CFG_TEST_ARRAY)
    execute_process(COMMAND dataconv -x ${PROJECT_SOURCE_DIR}/project/${CMAKE_PROJECT_NAME}/examples/helloworld.itu -o ${CMAKE_CURRENT_BINARY_DIR}/helloworld.inc)
    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        )
    add_executable(${CMAKE_PROJECT_NAME}
        test_$ENV{CFG_PLATFORM}.c
        test_array.c
        )
elseif (DEFINED CFG_TEST_JPEG)
    add_executable(${CMAKE_PROJECT_NAME}
        test_jpeg.c
        )
elseif (DEFINED CFG_TEST_MULTILINE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_multiline.c
        )
elseif (DEFINED CFG_TEST_COMPRESS)
    add_executable(${CMAKE_PROJECT_NAME}
        test_compress.c
        )
elseif (DEFINED CFG_TEST_FUNCTION)
    add_executable(${CMAKE_PROJECT_NAME}
        test_function.c
        )
elseif (DEFINED CFG_TEST_LISTBOX2)
    add_executable(${CMAKE_PROJECT_NAME}
        test_listbox2.c
        )
elseif (DEFINED CFG_TEST_CUSTOM)
    add_executable(${CMAKE_PROJECT_NAME}
        test_custom.c
        )
elseif (DEFINED CFG_TEST_ICONLISTBOX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_iconlistbox.c
        )
elseif (DEFINED CFG_TEST_CIRCLEPROGRESSBAR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_circleprogressbar.c
        )
elseif (DEFINED CFG_TEST_DRAW)
    add_executable(${CMAKE_PROJECT_NAME}
        test_draw.c
        )
elseif (DEFINED CFG_TEST_SCROLLBAR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_scrollbar.c
        )
elseif (DEFINED CFG_TEST_WHEEL)
    add_executable(${CMAKE_PROJECT_NAME}
        test_wheel.c
        )
elseif (DEFINED CFG_TEST_COVERFLOW)
    add_executable(${CMAKE_PROJECT_NAME}
        test_coverflow.c
        )
elseif (DEFINED CFG_TEST_SCROLLLISTBOX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_scrolllistbox.c
        )
elseif (DEFINED CFG_TEST_METER)
    add_executable(${CMAKE_PROJECT_NAME}
        test_meter.c
        )
elseif (DEFINED CFG_TEST_VIDEO)
    add_executable(${CMAKE_PROJECT_NAME}
        test_video.c
        )
elseif (DEFINED CFG_TEST_COLORPICKER)
    add_executable(${CMAKE_PROJECT_NAME}
        test_colorpicker.c
        )
elseif (DEFINED CFG_TEST_IMAGECOVERFLOW)
    add_executable(${CMAKE_PROJECT_NAME}
        test_imagecoverflow.c
        )
elseif (DEFINED CFG_TEST_CURVE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_curve.c
        )
elseif (DEFINED CFG_TEST_TABLELISTBOX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_tablelistbox.c
        )
elseif (DEFINED CFG_TEST_PNG)
    add_executable(${CMAKE_PROJECT_NAME}
        test_png.c
        )
elseif (DEFINED CFG_TEST_TABLEICONLISTBOX)
    add_executable(${CMAKE_PROJECT_NAME}
        test_tableiconlistbox.c
        )
elseif (DEFINED CFG_TEST_MULTIFONT)
    add_executable(${CMAKE_PROJECT_NAME}
        test_multifont.c        
        )
elseif (DEFINED CFG_TEST_CLONE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_clone.c        
        )        
elseif (DEFINED CFG_TEST_TABLELISTBOX2)
    add_executable(${CMAKE_PROJECT_NAME}
        test_tablelistbox2.c        
        )                
elseif (DEFINED CFG_TEST_ANIMPAGE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_animpage.c
        )                        
elseif (DEFINED CFG_TEST_BITBLT)
    add_executable(${CMAKE_PROJECT_NAME}
        test_bitblt.c
        )                        
elseif (DEFINED CFG_TEST_FILESLIDESHOW)
    add_executable(${CMAKE_PROJECT_NAME}
        test_fileslideshow.c
        )                                
elseif (DEFINED CFG_TEST_SHOWLOGO)
    add_executable(${CMAKE_PROJECT_NAME}
        test_showlogo.c
        )                                        
elseif (DEFINED CFG_TEST_MULTIGESTURE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_multigesture.c
        )
elseif (DEFINED CFG_TEST_CLIPPINGCOLOR)
    add_executable(${CMAKE_PROJECT_NAME}
        test_clippingcolor.c
        )        
elseif (DEFINED CFG_TEST_ROTATE)
    add_executable(${CMAKE_PROJECT_NAME}
        test_rotate.c
        )                
endif()

# font
file(COPY
    ${PROJECT_SOURCE_DIR}/data/font
    DESTINATION ${CMAKE_BINARY_DIR}/data/private
    )

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

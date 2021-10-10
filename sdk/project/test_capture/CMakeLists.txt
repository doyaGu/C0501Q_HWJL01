include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavcodec
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavformat
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavdevice
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libavutil
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswresample
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/libswscale
    #${PROJECT_SOURCE_DIR}/sdk/driver/i2s
)
    
add_executable(${CMAKE_PROJECT_NAME}
    test_$ENV{CFG_PLATFORM}.c
    test_capture.c
)

# build
include(${PROJECT_SOURCE_DIR}/sdk/build.cmake)

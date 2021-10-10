add_definitions(
    -DHAVE_CONFIG_H
    -DMEDIASTREAMER_STATIC
    -DMS2_INTERNAL
    -DMS2_MINIMAL_SIZE
    -DVIDEO_ENABLED
    -DWIN32_LEAN_AND_MEAN
    )

if (DEFINED CFG_BUILD_LEAF)
	add_definitions(
		-DCFG_BUILD_LEAF
		)
endif()

if (DEFINED CFG_LCD_ENABLE)
	add_definitions(
		-DENABLE_GENERAL_PLAYER
		)
endif()

if (DEFINED CFG_FFMPEG_H264_SW)
    add_definitions(
        -DCFG_FFMPEG_H264_SW
        )
endif()

if (DEFINED CFG_MP3_RING)
    add_definitions(
        -DCFG_MP3_RING
        )
endif()

if (DEFINED CFG_DUMP_AEC_DATA)
    add_definitions(
        -DENABLE_DUMP_AEC_DATA
        )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/mediastreamer2
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_preprocess
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr
    # ITV Buffer Manager, MTAL: medialink project
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv/
    )

add_library(mediastreamer STATIC
    mscommon.c
    msfilter.c
    msfactory.c
    msqueue.c
    msticker.c
    eventqueue.c
    alaw.c
    ulaw.c
    mssndcard.c
    msrtp.c
    dtmfgen.c
    ice.c
    tee.c
    msconf.c
    msjoin.c
    msvoip.c
    g711common.h
    msvolume.c
    mswebcam.c
    mtu.c
    void.c
    dsptools.c
    kiss_fft.c
    _kiss_fft_guts.h
    kiss_fft.h
    kiss_fftr.c
    kiss_fftr.h
    equalizer.c
    chanadapt.c
    audiomixer.c
    itc.c
    tonedetector.c
    qualityindicator.c
    g722_decode.c
    g722.h
    g722_encode.c
    msg722.c
    l16.c
    audioconference.c
    bitratedriver.c
    qosanalyzer.c
    bitratecontrol.c
    audiostream.c
    mediastream.c
    #mediastream_udp.c
    #msudp.c
    ringstream.c
    msspeex.c
    speexec.c
    msfileplayer.c
    msfilerec.c
    msresample.c
    videoenc.c
    videodec.c
    swscale.h
    ffmpeg-priv.h
    jpegwriter.c
    filewriter.c
    rfc2429.h
    pixconv.c
    sizeconv.c
    msvideo.c
    #msvideo_neon.c
    rfc3984.c
    #mire.c
    #extdisplay.c
    layouts.c
    layouts.h
    nowebcam.c
    nowebcam.h
    castor3snd.c
    basedescs.h
    voipdescs.h
    #hwengine.c
    mkv.c
    videostream.c
    #mkvplayerstream.c
    sbc_aec.c
    itc_ipcam.c
    ipcamstream.c
    mixvoice.c
    #webrtcagc.c
    adpcm.h
    adpcm.c
    sbc_aec_api.c
    sbc_aec_api.h
    cam_playback.c
    )

if (DEFINED CFG_BUILD_LEAF)
	add_library(mediastreamer STATIC
		mediastream_udp.c
		msudp.c
		)
endif()

if (DEFINED CFG_LCD_ENABLE)
    add_library(mediastreamer STATIC
        castor3-display.c
        )
endif()

if (DEFINED CFG_FFMPEG_H264_SW)
    add_library(mediastreamer STATIC
    	h264enc.c
        h264dec.c
        )

    if (DEFINED CFG_TS_MODULE_ENABLE)
        add_library(mediastreamer STATIC
            castor3cam.c
            )
    endif()

else()
	if (DEFINED CFG_SENSOR_ENABLE AND DEFINED CFG_CHIP_PKG_IT9910)
		add_library(mediastreamer STATIC
            ite_camera/enc_video_cam.c
        )
        
    endif()

    if (DEFINED CFG_SENSOR_ENABLE AND NOT DEFINED CFG_CHIP_PKG_IT9910)
		add_library(mediastreamer STATIC
            ite_camera/es_video_cam.c
        )
    endif()
    
    if (DEFINED CFG_CHIP_PKG_IT9910)
        add_library(mediastreamer STATIC
            h264dec.c
        )
    else()
        add_library(mediastreamer STATIC
            h264dec_castor3.c
        )
    endif()
endif()

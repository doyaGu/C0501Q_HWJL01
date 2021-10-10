if (DEFINED CFG_FFMPEG_ENC)
    add_definitions(
        -DCFG_FFMPEG_ENC
    )
endif()

add_definitions(
	-DAV_SYNC_STC
)

if (DEFINED CFG_FFMPEG_H264_SW)
    add_definitions(
        -DCFG_FFMPEG_H264_SW
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        -DWIN32_LEAN_AND_MEAN
    )
endif()

include_directories(
    .
    # ITV Buffer Manager, MTAL: medialink project
    ${PROJECT_SOURCE_DIR}/sdk/driver/itv/
)

if (DEFINED CFG_FFMPEG_H264_SW)
    FILE(GLOB libavcodec
        #++++++++++++++++++++
        # aac decoder
        libavcodec/aacdec.c
        libavcodec/aacsbr.c
        libavcodec/aacps.c
        libavcodec/kbdwin.c
        libavcodec/aacadtsdec.c
        # aac decoder
        #--------------------

        #++++++++++++++++++++
        # ac3/eac3 decoder
        libavcodec/ac3.c
        libavcodec/ac3dec.c
        libavcodec/ac3dec_data.c
        libavcodec/ac3dsp.c
        libavcodec/ac3tab.c
        libavcodec/eac3_data.c
        libavcodec/eac3dec.c
        # aac/eac3 decoder
        #--------------------

        #++++++++++++++++++++
        #flac decoder
        libavcodec/flac.c
        libavcodec/flacdata.c
        libavcodec/flacdec.c
        #--------------------

        #++++++++++++++++++++
        # mp3 decoder
        libavcodec/dct.c
        libavcodec/dct32.c
        libavcodec/dct32_fixed.c
        libavcodec/dct32_float.c
        libavcodec/fft.c
        libavcodec/mdct.c
        libavcodec/mdct_fixed.c
        libavcodec/mdct_float.c
        libavcodec/mpegaudio.c
        libavcodec/mpegaudiodata.c
        libavcodec/mpegaudiodec.c
        libavcodec/mpegaudiodecheader.c
        libavcodec/mpegaudiodsp.c
        libavcodec/mpegaudiodsp_fixed.c
        libavcodec/mpegaudiodsp_float.c
        libavcodec/mpegaudiodsp_template.c
        libavcodec/rdft.c
        # mp3 decoder
        #--------------------

        #++++++++++++++++++++
        # wma decoder
        libavcodec/wma.c
        libavcodec/wmadec.c
        libavcodec/wmaenc.c
        libavcodec/aactab.c
        libavformat/riff.c
        # wma decoder
        #--------------------

        #++++++++++++++++++++
        # h264 decoder
        libavcodec/h264.c
        libavcodec/h264_cabac.c
        libavcodec/h264_cavlc.c
        libavcodec/h264_direct.c
        libavcodec/h264_ps.c
        libavcodec/h264_refs.c
        libavcodec/h264_sei.c
        libavcodec/h264dsp.c
        libavcodec/h264idct.c
        libavcodec/h264pred.c
        libavcodec/h264_parser.c
        libavcodec/mpegvideo.c
        libavcodec/error_resilience.c
        # h264 decoder
        #--------------------

        #++++++++++++++++++++
        # mjpeg codec
        libavcodec/mjpeg.c
        libavcodec/mjpeg.h
        libavcodec/mjpegdec.c
        libavcodec/mjpegdec.h
        libavcodec/jpeglsdec.c
        libavcodec/jpeglsdec.h
        libavcodec/jpegls.c
        libavcodec/jpegls.h
        # mjpeg codec
        #--------------------
    )
else()
    FILE(GLOB libavcodec
        #++++++++++++++++++++
        # aac decoder
        libavcodec/aacdec_.c
        # aac decoder
        #--------------------

        #++++++++++++++++++++
        # ac3/eac3 decoder
        libavcodec/ac3dec_.c
        libavcodec/eac3dec_.c
        # aac/eac3 decoder
        #--------------------

        #++++++++++++++++++++
        #flac decoder
        libavcodec/flacdec_.c
        #--------------------

        #++++++++++++++++++++
        # mp3 decoder
        libavcodec/mpegaudiodata.c
        libavcodec/mpegaudiodecheader.c
        libavcodec/mpegaudiodec_.c
        # mp3 decoder
        #--------------------

        #++++++++++++++++++++
        # wma decoder
        libavcodec/wmadec_.c
        # wma decoder
        #--------------------

        #++++++++++++++++++++
        # h264 decoder
        #libavcodec/ite_h264.c
        libavcodec/iTE_h264_Dtv/avc.h
        libavcodec/iTE_h264_Dtv/avc.c
        libavcodec/iTE_h264_Dtv/avc_video_decoder.h
        libavcodec/iTE_h264_Dtv/avc_video_decoder.c
        # h264 decoder
        #--------------------

        #++++++++++++++++++++
        # mjpeg codec
        libavcodec/mjpeg_ite_dec.h
        libavcodec/mjpeg_ite_dec.c
        # mjpeg codec
        #--------------------
    )
endif()

# libavcodec
FILE(GLOB libavcodec
    ${libavcodec}
    libavcodec/allcodecs.c
    libavcodec/audioconvert.c
    libavcodec/avpacket.c
    libavcodec/bitstream.c
    libavcodec/bitstream_filter.c
    libavcodec/dsputil.c
    libavcodec/faanidct.c
    libavcodec/fmtconvert.c
    libavcodec/imgconvert.c
    libavcodec/jrevdct.c
    libavcodec/options.c
    libavcodec/parser.c
    libavcodec/raw.c
    libavcodec/rawdec.c
    libavcodec/resample.c
    libavcodec/resample2.c
    libavcodec/simple_idct.c
    libavcodec/utils.c
    libavcodec/pthread.c
    libavcodec/faandct.c
    libavcodec/jfdctfst.c
    libavcodec/jfdctint.c
    libavcodec/audiosync.c
    libavcodec/mpegvideo_parser.c

    #++++++++++++++++++++
    # flv decoder
    libavcodec/cabac.c
    #libavcodec/flvdec.c
    libavcodec/golomb.c
    #libavcodec/h263.c
    #libavcodec/h263dec.c
    #libavcodec/intelh263dec.c
    #libavcodec/ituh263dec.c
    #libavcodec/mpeg12.c
    #libavcodec/mpeg12data.c
    #libavcodec/mpeg4video.c
    #libavcodec/mpeg4videodec.c
    #libavcodec/mpeg4video_parser.c
    libavcodec/mpeg4audio.c
    #libavcodec/svq3.c
    # flv decoder
    #--------------------

    #++++++++++++++++++++
    # mpeg audio parser (mp1/2/3)
    libavcodec/mpegaudio_parser.c
    # mpeg audio parser (mp1/2/3)
    #--------------------

    #++++++++++++++++++++
    # amr decoder
    #libavcodec/amrnbdec.c
    # amr decoder
    #--------------------

    #++++++++++++++++++++
    # wav decoder
    libavcodec/pcm.c
    # wav decoder
    #--------------------

    #++++++++++++++++++++
    # mjpeg encoder
    libavcodec/mjpeg_ite_enc.h
    libavcodec/mjpeg_ite_enc.c
    # mjpeg encoder
    #--------------------
)

source_group("libavcodec" FILES ${libavcodec})

# libavformat
FILE(GLOB libavformat
    #++++++++++++++++++++
    # mpeg2 ps demuxer
    libavformat/mpeg.c
    # mpeg2 ps demuxer
    #--------------------

    libavformat/allformats.c
    libavformat/cutils.c
    libavformat/id3v1.c
    libavformat/id3v2.c
    libavformat/metadata.c
    libavformat/options.c
    libavformat/os_support.c
    libavformat/sdp.c
    libavformat/seek.c
    libavformat/utils.c
    libavformat/avio.c
    libavformat/aviobuf.c
    libavformat/avi.c
    libavformat/avidec.c
    libavformat/riff.c
    libavformat/mov.c
    libavformat/mov_chan.c
    libavformat/isom.c
    libavformat/matroskadec.c
    libavformat/matroska.c
    libavformat/rmdec.c
    libavformat/rm.c
    #libavformat/flvdec.c
    libavformat/mpegvideodec.c
    libavformat/rawdec.c
    libavformat/mp3dec.c
    libavformat/mpegts.c
)

if (DEFINED WIN32)
    FILE(GLOB libavformat
        ${libavformat}
        libavformat/win32/file.c
    )
else()
    FILE(GLOB libavformat
        ${libavformat}
        libavformat/file.c
    )
endif()

if (DEFINED CFG_NET_ENABLE)
    FILE(GLOB libavformat
        ${libavformat}
        # for rtsp streaming
        libavformat/network.c
        libavformat/rtpproto.c
        libavformat/rtsp.c
        libavformat/rtspdec.c
        libavformat/rtspenc.c
        libavformat/rtpenc_chain.c
        libavformat/rtpdec.c
        libavformat/rdt.c
        libavformat/http.c
        libavformat/httpauth.c
        libavformat/udp.c
        libavformat/tcp.c
        libavformat/rtp.c
        libavformat/rtpdec_amr.c
        libavformat/rtpdec_asf.c
        libavformat/asf.c
        libavformat/asfdec.c
        libavformat/asfcrypt.c
        libavformat/rtpdec_g726.c
        libavformat/rtpdec_h263.c
        libavformat/rtpdec_h264.c
        libavformat/rtpdec_latm.c
        libavformat/rtpdec_mpeg4.c
        libavformat/rtpdec_qcelp.c
        libavformat/rtpdec_qdm2.c
        libavformat/rtpdec_qt.c
        libavformat/rtpdec_svq3.c
        libavformat/rtpdec_vp8.c
        libavformat/rtpdec_xiph.c
        libavformat/avlanguage.c
    )
endif()

if (DEFINED CFG_TS_DEMUX_ENABLE)
    FILE(GLOB libavformat
        ${libavformat}
        # iTE TS demod live-streaming protocol
        libavformat/ts_demod.c
    )
endif()

if (DEFINED CFG_FFMPEG_ENC)
    FILE(GLOB libavformat
        ${libavformat}
        libavformat/mpegtsenc.c
        libavformat/adtsenc.c
        libavformat/rtpenc.c
        libavformat/rtpenc_aac.c
        libavformat/rtpenc_amr.c
        libavformat/rtpenc_h263.c
        libavformat/rtpenc_h264.c
        libavformat/rtpenc_latm.c
        libavformat/rtpenc_mpv.c
        libavformat/rtpenc_vp8.c
        libavformat/rtpenc_xiph.c
        libavformat/avc.c
    )

    if (DEFINED CFG_FFMPEG_H264_SW)
        FILE(GLOB libavcodec
            ${libavcodec}
            libavcodec/aandcttab.c
            libavcodec/flvenc.c
            libavcodec/ituh263enc.c
            libavcodec/mjpegenc.c
            libavcodec/motion_est.c
            libavcodec/mpeg4videoenc.c
            libavcodec/mpegvideo_enc.c
            libavcodec/ratecontrol.c
        )
    endif()
endif()

source_group("libavformat" FILES ${libavformat})

# libavutil
FILE(GLOB libavutil
    libavutil/adler32.c
    libavutil/aes.c
    libavutil/audioconvert.c
    libavutil/avstring.c
    libavutil/base64.c
    libavutil/cpu.c
    libavutil/crc.c
    libavutil/des.c
    libavutil/error.c
    libavutil/eval.c
    libavutil/fifo.c
    libavutil/file.c
    libavutil/imgutils.c
    libavutil/intfloat_readwrite.c
    libavutil/inverse.c
    libavutil/lfg.c
    libavutil/lls.c
    libavutil/log.c
    libavutil/lzo.c
    libavutil/mathematics.c
    libavutil/md5.c
    libavutil/mem.c
    libavutil/dict.c
    libavutil/opt.c
    libavutil/parseutils.c
    libavutil/pixdesc.c
    libavutil/random_seed.c
    libavutil/rational.c
    libavutil/rc4.c
    libavutil/samplefmt.c
    libavutil/sha.c
    libavutil/tree.c
    libavutil/utils.c
)

source_group("libavutil" FILES ${libavutil})

# libswscale
FILE(GLOB libswscale
    libswscale/options.c
    libswscale/rgb2rgb.c
    libswscale/swscale.c
    libswscale/utils.c
    libswscale/yuv2rgb.c
    libswscale/swscale_unscaled.c
)

source_group("libswscale" FILES ${libswscale})

add_library(ffmpeg STATIC
    ${libavcodec}
    ${libavformat}
    ${libavutil}
    ${libswscale}
    config.h
    castor3player.c
    file_player.c
    main_processor_message_queue.c
)
if (DEFINED CFG_BUILD_ITV)
    target_link_libraries(ffmpeg
        itv
    )
endif()

if (DEFINED CFG_BUILD_CAPTURE_MODULE)
    add_library(ffmpeg STATIC
        capdev_player.c
    )
endif()
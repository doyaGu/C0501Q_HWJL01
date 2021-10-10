if (DEFINED CFG_AUDIO_ENABLE)

    if (DEFINED CFG_AUDIO_CODEC_AACDEC)
        add_subdirectory(aacdecode)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_AC3DEC)
        add_subdirectory(ac3decode)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_AC3SPDIF)
        add_subdirectory(ac3spdif)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_AMR)
        add_subdirectory(amrcodec)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_EAC3DEC)
        add_subdirectory(eac3decode)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_MP3DEC)
        add_subdirectory(mp3decode)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_WAV)
        add_subdirectory(wavcodec)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_WMADEC)
        add_subdirectory(wmadecode)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_FLACDEC)
        add_subdirectory(flacdecode)
    endif()

    if (DEFINED CFG_RISC_TS_DEMUX_PLUGIN)
        add_subdirectory(tsdemux)
    endif()

    if (DEFINED CFG_AUDIO_CODEC_SBC)
        add_subdirectory(sbccodec)
    endif()

endif()

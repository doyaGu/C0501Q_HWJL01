# Use SD ini
if (DEFINED CFG_USE_SD_INI)
    add_definitions(
        -DCFG_USE_SD_INI
    )
endif()

#Local player
if (DEFINED CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE)
    add_definitions(
        -DCFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
    )
endif()

#Amplifier control
if (DEFINED CFG_AMPLIFIER_ENABLE)
    add_definitions(
        -DCFG_AMPLIFIER_ENABLE
    )
endif()

if (DEFINED CFG_AUDIO_MGR_M4A)
    add_definitions(
        -DCFG_AUDIO_MGR_M4A
    )
endif()

if (DEFINED CFG_AUDIO_MGR_WAV_HD)
    add_definitions(
        -DCFG_AUDIO_MGR_WAV_HD
    )
endif()

if (DEFINED CFG_GCC_LTO)
    add_definitions("-fno-lto")
endif ()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/driver/
    ${PROJECT_SOURCE_DIR}/sdk/share/audio_mgr/
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg/
)

if (DEFINED CFG_AUDIO_MGR_M4A)
    add_library(audio_mgr STATIC
        audio_mgr.c
        audio_mgr.h
        metadata.c
        lrc.c
        resample_main.c
        resample2.c
        wavfilefmt.c
        autoCheckEncode.c
        gb2312.h
        flac/flacparsing.c
        audioRecord.c
        Parsing.c
        sxa_dmx/sxa_dmx_3gpp.c
        sxa_dmx/sxa_dmx_3gpp_stbl_entry.c
        sxa_dmx/sxa_dmx_demuxer.c
        sxa_dmx/file.c
    )
else()
    add_library(audio_mgr STATIC
        audio_mgr.c
        audio_mgr.h
        metadata.c
        lrc.c
        resample_main.c
        resample2.c
        wavfilefmt.c
        autoCheckEncode.c
        gb2312.h
        flac/flacparsing.c
        audioRecord.c
        Parsing.c
        #sxa_dmx/sxa_dmx_3gpp.c
        #sxa_dmx/sxa_dmx_3gpp_stbl_entry.c
        #sxa_dmx/sxa_dmx_demuxer.c
        #sxa_dmx/file.c
    )
endif()

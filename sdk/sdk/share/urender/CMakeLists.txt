# Use SD ini
if (DEFINED CFG_USE_SD_INI)
    add_definitions(
        -DCFG_USE_SD_INI
        )
endif()

# Urender only one play
if (DEFINED CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY)
    add_definitions(
        -DCFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
        -DCFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SECONDS=${CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SECONDS}
        -DCFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SUSPEND_SECONDS=${CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SUSPEND_SECONDS}
        )
endif()

# Network switch 
if (DEFINED CFG_AUDIOLINK_NETWORK_SWITCH_ENABLE)
    add_definitions(
        -DCFG_AUDIOLINK_NETWORK_SWITCH_ENABLE
        )
endif()


# Airplay DACP
if (DEFINED CFG_NET_SHAIRPORT_DACP)
    add_definitions(
        -DCFG_NET_SHAIRPORT_DACP
        )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/upnp/
    ${PROJECT_SOURCE_DIR}/sdk/share/
    )

add_library(urender STATIC
    avt.c
    buffer.c
    cfgparser.c
    cms.c
    http.c
    mime.c
    osdep.c
    presentation.c
    rcs.c
    services.c
    trace.c
    urender.c
    util_iconv.c
    )



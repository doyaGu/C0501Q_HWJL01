# Use SD ini
if (DEFINED CFG_USE_SD_INI)
    add_definitions(
        -DCFG_USE_SD_INI
        )
endif()

# Change cpu speed
if (DEFINED CFG_AUDIOLINK_CHANGE_CPU_SPEED_ENABLE)
    add_definitions(
        -DCFG_AUDIOLINK_CHANGE_CPU_SPEED_ENABLE
        )
endif()

#Local player
if (DEFINED CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE)
    add_definitions(
        -DCFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
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

add_library(shairport_dacp STATIC
	alac.c
	alac.h
	hairtunes.c
	hairtunes.h
	shairport.c
	shairport.h
	socketlib.c
	socketlib.h
	MD5Utils.c
	MD5Utils.h
    )

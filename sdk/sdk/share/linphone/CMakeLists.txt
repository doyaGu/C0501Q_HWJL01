add_definitions(
    -DHAVE_CONFIG_H
    )

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

include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/linphone
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    )

add_library(linphone STATIC
	coreapi/linphonecore.c
	${PROJECT_SOURCE_DIR}/sdk/include/linphone/linphonecore.h
	coreapi/private.h
	coreapi/offeranswer.c
	coreapi/offeranswer.h
	coreapi/sal.c
	coreapi/sal.h
	coreapi/sal_eXosip2.c
	coreapi/sal_eXosip2.h
	coreapi/sal_eXosip2_sdp.c
	coreapi/sal_eXosip2_presence.c
	coreapi/callbacks.c
	coreapi/misc.c
	coreapi/address.c
	coreapi/enum.c
	coreapi/enum.h
	coreapi/presence.c
	coreapi/proxy.c
	coreapi/friend.c
	coreapi/authentication.c
	coreapi/lpconfig.c
	${PROJECT_SOURCE_DIR}/sdk/include/linphone/lpconfig.h
	coreapi/chat.c
	coreapi/linphonecall.c
	coreapi/sipsetup.c
	${PROJECT_SOURCE_DIR}/sdk/include/linphone/sipsetup.h
	coreapi/siplogin.c
	coreapi/lsd.c
	${PROJECT_SOURCE_DIR}/sdk/include/linphone/linphonecore_utils.h
	coreapi/ec-calibrator.c
	coreapi/conference.c
	coreapi/linphone_tunnel.cc
	console/linphonec.c
	console/linphonec.h
	console/commands.c
    ${PROJECT_SOURCE_DIR}/sdk/include/linphone/linphone_castor3.h
    )

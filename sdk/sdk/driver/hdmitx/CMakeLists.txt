INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    )

if($ENV{CFG_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DHDMITX_EXPORTS
        )

endif($ENV{CFG_PLATFORM} STREQUAL win32)

ADD_LIBRARY(hdmitx STATIC
    csc.c
    mmp_hdmitx.c
    IT6613/hdmitx_drv.c
    IT6613/hdmitx_sys.c
	IT66121/hdmitx_drv.c
    IT66121/hdmitx_sys.c
    )
    
#TARGET_LINK_LIBRARIES(hdmitx
#    itp
#    mmp
#    iic
#    )


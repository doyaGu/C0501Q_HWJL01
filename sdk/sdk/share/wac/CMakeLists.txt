add_definitions(
    -DAES_UTILS_HAS_GLADMAN_GCM=1
    -DAES_UTILS_USE_GLADMAN_AES=1
    -DTARGET_HAS_MD5_UTILS=1
    -DTARGET_HAS_SHA_UTILS=1
    -DTARGET_NO_OPENSSL=1
    -DUSE_VIA_ACE_IF_PRESENT=0
    -DTIME_PLATFORM=1
		-D_GNU_SOURCE
		-DDEBUG_EXPORT_ERROR_STRINGS=1		
    )

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG_CF_OBJECTS_ENABLED=1
        -DDEBUG_EXPORT_ERROR_STRINGS=1
        )

    if ($ENV{CFG_PLATFORM} STREQUAL openrtos)
        add_definitions(
            -DDEBUG=1
            )
    endif()

endif()

if (1)
    add_definitions(
        -DTARGET_RT_LITTLE_ENDIAN=1
        #-DTARGET_RT_BIG_ENDIAN=0
        )
elseif (DEFINED CFG_CPU_SM32)
    add_definitions(
        -DTARGET_RT_BIG_ENDIAN=1
        #-DTARGET_RT_LITTLE_ENDIAN=0
        )
endif()

include_directories(
    External/Curve25519
    External/GladmanAES
    Sources
    Support
    Platform
    ${PROJECT_SOURCE_DIR}/sdk/share/dns_sd/mDNSShared
    ${PROJECT_SOURCE_DIR}/sdk/share/dns_sd/mDNSCore
    ${PROJECT_SOURCE_DIR}/sdk/share/dns_sd/mDNSPosix    
    )

add_library(wac STATIC
    Sources/WACBonjour.c
    Sources/WACServer.c
    External/GladmanAES/aes_modes.c
    External/GladmanAES/aescrypt.c
    External/GladmanAES/aeskey.c
    External/GladmanAES/aestab.c
    External/GladmanAES/gcm.c
    External/GladmanAES/gf128mul.c
    External/Curve25519/curve25519-donna.c		    
    Support/AESUtils.c
    Support/AppleDeviceIE.c
    Support/HTTPServer.c
    Support/HTTPUtils.c
    Support/MFiSAPServer.c
    Support/SecurityUtils.c
    Support/SHAUtils.c
    Support/SocketUtils.c
    Support/StringUtils.c
    Support/TimeUtils.c
    Support/TLVUtils.c
    Support/URLUtils.c
    Platform/PlatformWac.c
    )

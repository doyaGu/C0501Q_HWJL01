add_definitions(
    -DHAVE_IPV6=0
    -DNOT_HAVE_IF_NAMETOINDEX
    -DNOT_HAVE_SA_LEN
    -DPLATFORM_NO_RLIMIT
    -DUSE_TCP_LOOPBACK
    -D_PLATFORM_HAS_STRONG_PRNG_=0
    )
        
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DMDNS_DEBUGMSGS=2
        )
else()
    add_definitions(
        -DMDNS_DEBUGMSGS=1
        )
endif()

if (DEFINED CFG_GCC_LTO)
 add_definitions("-fno-lto") 
endif ()

include_directories(
    mDNSCore
    mDNSShared
    mDNSPosix
    )

add_library(dns_sd STATIC
    #Clients/ClientCommon.c
    #Clients/ClientCommon.h
    #Clients/dns-sd.c
    mDNSCore/DNSCommon.c
    mDNSCore/DNSCommon.h
    mDNSCore/DNSDigest.c
    mDNSCore/mDNS.c
    mDNSCore/mDNSDebug.h
    mDNSCore/mDNSEmbeddedAPI.h
    mDNSCore/uDNS.c
    mDNSCore/uDNS.h
    mDNSPosix/mDNSPosix.c
    mDNSPosix/mDNSPosix.h
    mDNSPosix/mDNSUNP.c
    mDNSPosix/mDNSUNP.h
    mDNSShared/CommonServices.h
    mDNSShared/DebugServices.c
    mDNSShared/DebugServices.h
    mDNSShared/GenLinkedList.c
    mDNSShared/GenLinkedList.h
    mDNSShared/mDNSDebug.c
    mDNSShared/PlatformCommon.c
    mDNSShared/PlatformCommon.h
    )

if (DEFINED CFG_DNS_SD_RESPONDER)
    add_library(dns_sd STATIC
        mDNSPosix/Responder.c
    )
elseif (DEFINED CFG_DNS_SD_DAEMON)
    add_library(dns_sd STATIC
        mDNSShared/dnssd_clientlib.c
        mDNSShared/dnssd_clientstub.c
        mDNSShared/dnssd_ipc.c
        mDNSShared/dnssd_ipc.h
        mDNSShared/dns_sd.h
        mDNSShared/uds_daemon.c
        mDNSShared/uds_daemon.h
        mDNSPosix/PosixDaemon.c
    )
endif()

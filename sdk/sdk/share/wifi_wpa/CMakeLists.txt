include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/include/
    wpa_supplicant
    src/ap
    src/common
    src/crypto
    src/drivers
    src/eap_common
    src/eap_peer
    src/eap_server
    src/eapol_auth
    src/eapol_supp
    src/l2_packet
    src/p2p
    src/radius
    src/rsn_supp
    src/tls
    src/utils
    src/wps  
    src
    )



add_library(wpa_supplicant STATIC
    wpa_supplicant/config.c
    wpa_supplicant/notify.c
    wpa_supplicant/bss.c
    wpa_supplicant/eap_register.c
    src/utils/common.c
    src/utils/wpa_debug.c
    src/utils/wpabuf.c
    src/utils/os_unix.c
    
    # CONFIG_ELOOP
    src/utils/eloop.c
    
    # CONFIG_BACKEND
    wpa_supplicant/config_none.c
    
    # not CONFIG_NO_WPA
    src/rsn_supp/wpa.c
    src/rsn_supp/preauth.c
    src/rsn_supp/pmksa_cache.c
    src/rsn_supp/peerkey.c
    src/rsn_supp/wpa_ie.c
    src/common/wpa_common.c

    # CONFIG_WIRELESS_EXTENSION
    src/drivers/drivers.c
    src/drivers/driver_wext.c
    src/drivers/netlink.c
    src/drivers/linux_ioctl.c
    #src/drivers/rfkill.c
    #src/drivers/driver_ipw.c

    # CONFIG_L2_PACKET need pcap
    #src/l2_packet/l2_packet_freebsd.c
    src/l2_packet/l2_packet_linux.c

    # CONFIG_EAP_TLS
    #src/eap_peer/eap_tls.c

    # CONFIG_EAP_PEAP
    #src/eap_peer/eap_peap.c
    #src/eap_common/eap_peap_common.c

    # CONFIG_EAP_TTLS
    #src/eap_peer/eap_ttls.c

    # CONFIG_EAP_MD5
    #src/eap_peer/eap_md5.c

    # CONFIG_EAP_MSCHAPV2
    #src/eap_peer/eap_mschapv2.c
    #src/eap_peer/mschapv2.c

    # CONFIG_EAP_GTC
    #src/eap_peer/eap_gtc.c

    # CONFIG_EAP_OTP
    #src/eap_peer/eap_otp.c

    # CONFIG_EAP_LEAP
    #src/eap_peer/eap_leap.c

    # CONFIG_WPS
    wpa_supplicant/wps_supplicant.c
    src/utils/uuid.c
    src/eap_peer/eap_wsc.c
    src/eap_common/eap_wsc_common.c
    src/wps/wps.c
    src/wps/wps_common.c
    src/wps/wps_attr_parse.c
    src/wps/wps_attr_build.c
    src/wps/wps_attr_process.c
    src/wps/wps_dev_attr.c
    src/wps/wps_enrollee.c
    src/wps/wps_registrar.c

    # CONFIG_IEEE8021X_EAPOL
    src/eapol_supp/eapol_supp_sm.c
    src/eap_peer/eap.c 
    src/eap_peer/eap_methods.c

    # TLS_FUNCS
    #src/eap_peer/eap_tls_common.c
    #src/crypto/crypto_openssl.c
    
    # NEED_AES
    src/crypto/aes-unwrap.c
    src/crypto/aes-cbc.c

    # NEED_SHA1
    src/crypto/sha1.c
    src/crypto/sha1-tlsprf.c
   
    # NEED_MD5
    src/crypto/md5.c

    # NEED_SHA256
    src/crypto/sha256.c

    # NEED_DH_GROUPS
    src/crypto/dh_groups.c

    # CONFIG_CTRL_IFACE
    wpa_supplicant/ctrl_iface.c 
    wpa_supplicant/ctrl_iface_udp.c
    wpa_supplicant/main_none.c
    wpa_supplicant/wpa_supplicant.c 
    wpa_supplicant/events.c 
    wpa_supplicant/blacklist.c 
    wpa_supplicant/wpas_glue.c 
    wpa_supplicant/scan.c

    # NEED_BASE64
    src/utils/base64.c

    # NEED_80211_COMMON
    src/common/ieee802_11_common.c
    #src/crypto/rc4.c
    src/crypto/sha1-pbkdf2.c
    src/eap_common/eap_common.c
    src/crypto/tls_none.c
    src/crypto/crypto_openssl.c
    src/crypto/random.c
    )
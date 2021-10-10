
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/sdk/driver/sd/mmc/include
    ${PROJECT_SOURCE_DIR}/sdk/driver/net/ameba_sdio/include
    )

ADD_LIBRARY(ameba_sdio STATIC
    core/rtw_cmd.c
    core/rtw_debug.c
    core/rtw_io.c
    core/rtw_pwrctrl.c
    core/rtw_recv.c
    core/rtw_xmit.c
    hal/hal_intf.c
    hal/rtl8195a/rtl8195a_cmd.c
    hal/rtl8195a/rtl8195a_hal_init.c
    hal/rtl8195a/sdio/rtl8195a_sdio_halinit.c
    hal/rtl8195a/sdio/rtl8195a_sdio_ops.c
    hal/rtl8195a/sdio/rtl8195as_recv.c
    hal/rtl8195a/sdio/rtl8195as_xmit.c
    hci/hci_intf.c
    hci/sdio/sdio_drvio.c
    hci/sdio/sdio_intf.c
    osdep/osdep_service.c
    osdep/ite_skbuf.c
    osdep/ite_ndis_ameba.c
    osdep/linux/ioctl_linux.c
    osdep/linux/linux_sdio_intf.c
    osdep/linux/linux_service.c
    osdep/linux/os_intfs.c
    osdep/linux/recv_linux.c
    osdep/linux/sdio_ops_linux.c
    osdep/linux/xmit_linux.c
    )


#ifndef	ITE_WIFI_IOCTL_H
#define	ITE_WIFI_IOCTL_H

#include "iw_handler.h" 

//James : not need

#define IW_CONFIG(_dev, _ctrlId, _info, _iwreq, _extra)		\
            wlanControl((_dev), (_ctrlId),            \
                (struct iw_request_info *)(_info),      \
                (union iwreq_data *)(_iwreq),           \
                (char *)(_extra))
                
#define IW_PRIV                                         IW_CONFIG	

#define RESET_SETTING(_iwInfo, _iwReq)                              \
do{                                                                 \
    memset(&(_iwInfo), 0x00, sizeof(struct iw_request_info));       \
    memset(&(_iwReq), 0x00, sizeof(union iwreq_data));              \
} while(0)

//extern	const struct iw_handler_def rtw_handlers_def;

int wlanControl(struct net_device *dev, unsigned int ctrlId, 
    struct iw_request_info *info,
    union iwreq_data *wrqu, 
    char *extra);

#endif


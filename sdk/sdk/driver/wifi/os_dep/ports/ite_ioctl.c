//#include "rt_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ite_port.h"

int wlanControl(struct net_device *dev, unsigned int ctrlId, 
    struct iw_request_info *info,
    union iwreq_data *wrqu, 
    char *extra)
{
	struct iw_handler_def *iwHandler = &rtw_handlers_def;
    iw_handler *iwHandlerFunc = NULL;
    int ret = 0;

	printk("@@@@[%s]WLAN Control is %s, ID %x.\n", 
            dev->name,
            (ctrlId >= SIOCIWFIRSTPRIV ? "PRIV" : "STD"),
            ctrlId);
        
    if (ctrlId >= SIOCIWFIRSTPRIV)
    {   /*IW_PRIV*/
        if (iwHandler->private != NULL)
            iwHandlerFunc = ((iw_handler *)(iwHandler->private) + 
                                (ctrlId - SIOCIWFIRSTPRIV));
        else
            ret = -3;
    }
    else
    {   /*IW_CCONFIG*/
        if (iwHandler->standard != NULL)
            iwHandlerFunc = ((iw_handler *)(iwHandler->standard) +
                                (ctrlId - SIOCIWFIRST));
        else
            ret = -2;
    }

    if (iwHandlerFunc)
        ret = (*iwHandlerFunc)(dev, info, wrqu, extra);

    if (ret != 0)
        printk("@@@@OMG, this ID control fail! [%d]\n\n", ret);
    
    return ret;
}


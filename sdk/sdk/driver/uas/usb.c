/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 */
#include "config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
/* Storage subclass codes */
#define USB_SC_SCSI	0x06		/* Transparent */

/* Storage protocol codes */
#define USB_PR_BULK	0x50		/* bulk only */
#define USB_PR_UAS	0x62		/* USB Attached SCSI */


//=============================================================================
//                              Gloabl Data
//=============================================================================
/** currently support 2 context for two USB host controller each */
static struct uas_dev_info uas_list[2];

static struct usb_device_id uas_usb_ids[] = 
{
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, USB_SC_SCSI, USB_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, USB_SC_SCSI, USB_PR_UAS) },
    { USB_INTERFACE_INFO(0, 0, 0) }  // end marker
};

//=============================================================================
//                   Private function definition
//=============================================================================
static int uas_is_interface(struct usb_host_interface *intf)
{
	return (intf->desc.bInterfaceClass == USB_CLASS_MASS_STORAGE &&
		intf->desc.bInterfaceSubClass == USB_SC_SCSI &&
		intf->desc.bInterfaceProtocol == USB_PR_UAS);
}

static int uas_switch_interface(struct usb_device *udev,
						struct usb_interface *intf)
{
    int i;

	for (i = 0; i < intf->num_altsetting; i++) {
		struct usb_host_interface *alt = &intf->altsetting[i];

		if (uas_is_interface(alt)) {
			return usb_set_interface(udev,
						alt->desc.bInterfaceNumber,
						alt->desc.bAlternateSetting);
		}
	}

    return -1;
}

static void uas_configure_endpoints(struct uas_dev_info *devinfo)
{
    struct usb_host_endpoint *eps[4] = {0};
	struct usb_interface *intf = devinfo->intf;
    struct usb_device *udev = devinfo->udev;
	struct usb_host_endpoint *endpoint = intf->cur_altsetting->endpoint;
    unsigned i, n_endpoints = intf->cur_altsetting->desc.bNumEndpoints;

    devinfo->cmnd = NULL;

	for (i = 0; i < n_endpoints; i++) {
		unsigned char *extra = endpoint[i].extra;
		int len = endpoint[i].extralen;
		while (len > 1) {
			if (extra[1] == USB_DT_PIPE_USAGE) {
				unsigned pipe_id = extra[2];
				if (pipe_id > 0 && pipe_id < 5)
					eps[pipe_id - 1] = &endpoint[i];
				break;
			}
			len -= extra[0];
			extra += extra[0];
		}
	}

    if(!eps[0])
    {
        LOG_ERROR " We didn't find a control pipe descriptor!!! \n" LOG_END
        return;
    }

	devinfo->cmd_pipe = usb_sndbulkpipe(udev,
					eps[0]->desc.bEndpointAddress);
	devinfo->status_pipe = usb_rcvbulkpipe(udev,
					eps[1]->desc.bEndpointAddress);
	devinfo->data_in_pipe = usb_rcvbulkpipe(udev,
					eps[2]->desc.bEndpointAddress);
	devinfo->data_out_pipe = usb_sndbulkpipe(udev,
					eps[3]->desc.bEndpointAddress);

	devinfo->qdepth = usb_alloc_streams(devinfo->intf, eps + 1, 3, 256,
								GFP_KERNEL);
	if (devinfo->qdepth < 0) {
		devinfo->qdepth = 256;
		devinfo->use_streams = 0;
	} else {
		devinfo->use_streams = 1;
	}
}

static void uas_free_streams(struct uas_dev_info *devinfo)
{
	struct usb_device *udev = devinfo->udev;
	struct usb_host_endpoint *eps[3];

	eps[0] = usb_pipe_endpoint(udev, devinfo->status_pipe);
	eps[1] = usb_pipe_endpoint(udev, devinfo->data_in_pipe);
	eps[2] = usb_pipe_endpoint(udev, devinfo->data_out_pipe);
	usb_free_streams(devinfo->intf, eps, 3, GFP_KERNEL);
}

//=============================================================================
//                   Public function definition
//=============================================================================
static void* uas_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    int rc = 0, retry = 5;
    struct uas_dev_info *devinfo = NULL;
    struct usb_device *udev = interface_to_usbdev(intf);

	if (uas_switch_interface(udev, intf))
		return NULL;

    LOG_INFO " switch to uas interface! \n" LOG_END

    /** decide the context */
    if(!uas_list[0].udev)
        devinfo = &uas_list[0];
    else if(!uas_list[1].udev)
        devinfo = &uas_list[1];
    else
    {
        LOG_ERROR " No context available?? \n" LOG_END
        return NULL;
    }
    udev->type = USB_DEVICE_TYPE_UAS;
    devinfo->intf = intf;
    devinfo->udev = udev;
	devinfo->resetting = 0;
	init_usb_anchor(&devinfo->cmd_urbs);
	init_usb_anchor(&devinfo->sense_urbs);
	init_usb_anchor(&devinfo->data_urbs);
    spin_lock_init(&devinfo->lock);
    uas_configure_endpoints(devinfo);

    if(uscsi_init(devinfo))
        goto fail;

    if(uscsi_inquiry(devinfo, 0))
        goto fail;

    if(uscsi_report_luns(devinfo))
        goto fail;
    //rc = uscsi_test_unit_ready(devinfo, 0); // ?? TODO

    return (void*)devinfo;

fail:
    uscsi_release(devinfo);
    uas_free_streams(devinfo);
    udev->type = 0;
    memset((void*)devinfo, 0, sizeof(struct uas_dev_info));
    return NULL;
}

static void uas_disconnect(struct usb_interface *intf)
{
    struct uas_dev_info *devinfo = (struct uas_dev_info *)usb_get_intfdata(intf);

	devinfo->resetting = 1;
	uas_abort_work(devinfo);
	usb_kill_anchored_urbs(&devinfo->cmd_urbs);
	usb_kill_anchored_urbs(&devinfo->sense_urbs);
	usb_kill_anchored_urbs(&devinfo->data_urbs);

    uscsi_release(devinfo);
    uas_free_streams(devinfo);
    memset((void*)devinfo, 0, sizeof(struct uas_dev_info));
}

static struct usb_driver uas_driver = 
{
    "uas_driver",       /** driver name */
    0,                  /** flag */
    uas_probe,          /** probe function */
    NULL,               /** open function */
    uas_disconnect,     /** disconnect function */
    NULL,               /** ioctl */
    uas_usb_ids,        /** id table */
    NULL, NULL          /** driver_list */
};

int iteUasDriverRegister(void)
{
    return usb_register(&uas_driver);
}


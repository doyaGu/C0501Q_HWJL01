#include "../idb/idbInjection.h"

/*-------------------------------------------------------------------------*/

#define STRING_MANUFACTURER	1
#define STRING_PRODUCT		2
#define STRING_SERIAL		3
#define STRING_CONFIG		4
#define STRING_INTERFACE	5

/* There is only one configuration. */
#define	CONFIG_VALUE		1

static struct usb_device_descriptor device_desc = 
{
    .bLength			= USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,

#if !defined(__SM32__)
    .bcdUSB				= cpu_to_le16(0x0200),
#endif
    .bDeviceClass		= USB_CLASS_PER_INTERFACE,
#if 0
    /* The next three values can be overridden by module parameters */
    .idVendor =		cpu_to_le16(DRIVER_VENDOR_ID),
    .idProduct =	cpu_to_le16(DRIVER_PRODUCT_ID),
    .bcdDevice =	cpu_to_le16(0xffff),
#endif
    .iManufacturer		= STRING_MANUFACTURER,
    .iProduct			= STRING_PRODUCT,
    .iSerialNumber		= STRING_SERIAL,
    .bNumConfigurations = 1,
};

static struct usb_config_descriptor config_desc = 
{
    .bLength			= USB_DT_CONFIG_SIZE,
    .bDescriptorType	= USB_DT_CONFIG,

    /* wTotalLength computed by usb_gadget_config_buf() */
    .bNumInterfaces		= 1,
    .bConfigurationValue = CONFIG_VALUE,
    .iConfiguration		= STRING_CONFIG,
    .bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
    .bMaxPower			= 0x32,
};

/* There is only one interface. */

/* USB protocol value = the transport method */
#define USB_PR_BULK	0x50		// Bulk-only
/* USB subclass value = the protocol encapsulation */
#define USB_SC_SCSI	0x06		// Transparent SCSI

static struct usb_interface_descriptor intf_desc = 
{
    .bLength			= USB_DT_INTERFACE_SIZE,
    .bDescriptorType	= USB_DT_INTERFACE,
	IDB_DATA_INJECTION(.bInterfaceNumber = 1 IDB_COMMA)
    .bNumEndpoints		= 2,
    .bInterfaceClass	= USB_CLASS_MASS_STORAGE,
    .bInterfaceSubClass = USB_SC_SCSI,
    .bInterfaceProtocol = USB_PR_BULK,   
    .iInterface			= STRING_INTERFACE,
};

/* Two full-speed endpoint descriptors: bulk-in, bulk-out. */

static struct usb_endpoint_descriptor fs_bulk_in_desc = 
{
    .bLength			= USB_DT_ENDPOINT_SIZE,
    .bDescriptorType	= USB_DT_ENDPOINT,

    .bEndpointAddress	= USB_DIR_IN,
    .bmAttributes		= USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor fs_bulk_out_desc = 
{
    .bLength			= USB_DT_ENDPOINT_SIZE,
    .bDescriptorType	= USB_DT_ENDPOINT,

    .bEndpointAddress	= USB_DIR_OUT,
    .bmAttributes		= USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};

static const struct usb_descriptor_header *fs_function[] = {
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbInterfaceDescriptor IDB_COMMA)
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbFsEpInDescriptor IDB_COMMA)
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbFsEpOutDescriptor IDB_COMMA)
    (struct usb_descriptor_header *) &intf_desc,
    (struct usb_descriptor_header *) &fs_bulk_in_desc,
    (struct usb_descriptor_header *) &fs_bulk_out_desc,
    NULL,
};


/*
 * USB 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 *
 * That means alternate endpoint descriptors (bigger packets)
 * and a "device qualifier" ... plus more construction options
 * for the config descriptor.
 */
static struct usb_qualifier_descriptor dev_qualifier = 
{
    .bLength			= sizeof dev_qualifier,
    .bDescriptorType	= USB_DT_DEVICE_QUALIFIER,

#if !defined(__SM32__)
    .bcdUSB				= cpu_to_le16(0x0200),
#endif
    .bDeviceClass		= USB_CLASS_PER_INTERFACE,

    .bNumConfigurations = 1,
};

static struct usb_endpoint_descriptor hs_bulk_in_desc = 
{
    .bLength			= USB_DT_ENDPOINT_SIZE,
    .bDescriptorType	= USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
    .bmAttributes		= USB_ENDPOINT_XFER_BULK,
#if !defined(__SM32__)
    .wMaxPacketSize		= cpu_to_le16(512),
#endif
};

static struct usb_endpoint_descriptor hs_bulk_out_desc = 
{
    .bLength			= USB_DT_ENDPOINT_SIZE,
    .bDescriptorType	= USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
    .bmAttributes		= USB_ENDPOINT_XFER_BULK,
#if !defined(__SM32__)
    .wMaxPacketSize		= cpu_to_le16(512),
#endif
    //.bInterval			= 1,	// NAK every 1 uframe
};

static const struct usb_descriptor_header *hs_function[] = {
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbInterfaceDescriptor IDB_COMMA)
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbHsEpInDescriptor IDB_COMMA)
    IDB_DATA_INJECTION((struct usb_descriptor_header *) &idbHsEpOutDescriptor IDB_COMMA)
    (struct usb_descriptor_header *) &intf_desc,
    (struct usb_descriptor_header *) &hs_bulk_in_desc,
    (struct usb_descriptor_header *) &hs_bulk_out_desc,
    NULL,
};

/* Static strings, in UTF-8 (for simplicity we use only ASCII characters) */
static struct usb_string		strings[] = 
{
    {STRING_MANUFACTURER,	" "},
    {STRING_PRODUCT,	" "},
    {STRING_SERIAL,		" "},
    {STRING_CONFIG,		"Config string"},
    {STRING_INTERFACE,	"Mass Storage"},
    IDB_DATA_INJECTION({IDB_INTERFACE_STRING_INDEX IDB_COMMA IDB_INTERFACE_STRING}IDB_COMMA)
    {}
};

static struct usb_gadget_strings	stringtab = 
{
    .language	= 0x0409,		// en-us
    .strings	= strings,
};

/*
 * Config descriptors must agree with the code that sets configurations
 * and with code managing interfaces and their altsettings.  They must
 * also handle different speeds and other-speed requests.
 */
static int populate_config_buf(struct usb_gadget *gadget, uint8_t *buf, uint8_t type, uint8_t index)
{
    enum usb_device_speed	speed = gadget->speed;
    int len;
    const struct usb_descriptor_header **function;

    if(index > 0)
        return -1;

    if(gadget_is_dualspeed(gadget) && (type == USB_DT_OTHER_SPEED_CONFIG))
        speed = (USB_SPEED_FULL + USB_SPEED_HIGH) - speed;
    if(gadget_is_dualspeed(gadget) && speed == USB_SPEED_HIGH)
        function = hs_function;
    else
        function = fs_function;

    len = usb_gadget_config_buf(&config_desc, buf, EP0_BUFSIZE, function);
    ((struct usb_config_descriptor *) buf)->bDescriptorType = type;
    return len;
}

/* Maxpacket and other transfer characteristics vary by speed. */
static struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *fs,
        struct usb_endpoint_descriptor *hs)
{
    if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
        return hs;
    return fs;
}

static int fsg_selfpower(void)
{
    return ((config_desc.bmAttributes & USB_CONFIG_ATT_SELFPOWER) ? 1 : 0);
}
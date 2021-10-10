
#ifndef ITE_USB_MOUSE_H
#define ITE_USB_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*usb_mouse_cb)(int8_t*);

extern int iteUsbMouseRegister(void);
extern int iteUsbMouseSetCb(usb_mouse_cb cb);


#ifdef __cplusplus
}
#endif

#endif // ITE_USB_MOUSE_H

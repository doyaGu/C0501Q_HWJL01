
#ifndef ITE_USB_KBD_H
#define ITE_USB_KBD_H

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*usb_kbd_cb)(uint8_t*);

extern int iteUsbKbdRegister(void);
extern int iteUsbKbdSetCb(usb_kbd_cb cb);


#ifdef __cplusplus
}
#endif

#endif // ITE_USB_KBD_H

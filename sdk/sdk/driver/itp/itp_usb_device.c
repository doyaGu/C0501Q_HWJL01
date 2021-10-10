

static bool connected = false;
static const int diskTable[] = { CFG_USB_DEVICE_DISKS , -1 };

static void* UsbDeviceDetectHandler(void* arg)
{
    while (1) {
        if (iteOtgIsDeviceMode() == true) {
            if (connected == false) {
                #if defined(CFG_USBD_FILE_STORAGE)
                #if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_DISABLE, NULL);
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_UNMOUNT, diskTable);
                #endif
                ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_ENABLE, NULL);
				ioctl(ITP_DEVICE_USBDIDB, ITP_IOCTL_ENABLE, NULL);
                #endif // #if defined(CFG_USBD_FILE_STORAGE)
                connected = true;
            }
        }
        else {
            if (connected == true) {
                #if defined(CFG_USBD_FILE_STORAGE)
                ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_DISABLE, NULL);
                #if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_MOUNT, diskTable);
                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_ENABLE, NULL);
                /*
                #if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
                ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESET, (void*)0); // reset usb0's msc active status
                #endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

                #if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
                ioctl(ITP_DEVICE_USB, ITP_IOCTL_RESET, (void*)1); // reset usb1's msc active status
                #endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
                */
                #endif // #if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
                #endif // #if defined(CFG_USBD_FILE_STORAGE)
                connected = false;
        }
        }

        usleep(30 * 1000);
    }
}

    


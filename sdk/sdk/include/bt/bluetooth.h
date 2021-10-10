#ifndef MMP_BT_H
#define MMP_BT_H

#ifdef __cplusplus
extern "C" {
#endif

#define BT_API

BT_API int bt_dev_open(ITPDeviceType devtype);

BT_API void bt_dev_close(int fd);

BT_API int bt_dev_write(int fd, char* ptr, size_t len);

BT_API int bt_dev_read(int fd, char* ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MMP_BT_H */
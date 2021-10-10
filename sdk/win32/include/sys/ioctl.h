#ifndef ITP_SYS_IOCTL_H
#define ITP_SYS_IOCTL_H

#include <sys/sockio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Support function.  Device specific control.  A shell to convert
 * requests into a re-entrant form.
 *
 * @param fd number referring to the open file. Generally
 *  obtained from a corresponding call to open.
 * @param request Number to indicate the request being made
 *  of the driver.
 * @param ptr pointer to data that is either used by or
 *  set by the driver. Varies by request.
 * @return 0 if successful, otherwise errno will be set to indicate
 *  the source of the error.
 */
int ioctl(int file, unsigned long request, void* ptr);

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_IOCTL_H

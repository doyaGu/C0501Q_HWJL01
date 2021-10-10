#ifndef _NSS_H
#define _NSS_H	1

/* Possible results of lookup using a nss_* function.  */
enum nss_status
{
  NSS_STATUS_TRYAGAIN = -2,
  NSS_STATUS_UNAVAIL,
  NSS_STATUS_NOTFOUND,
  NSS_STATUS_SUCCESS,
  NSS_STATUS_RETURN
};

#endif /* nss.h */
 
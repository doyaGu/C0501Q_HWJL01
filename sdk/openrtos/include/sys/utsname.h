#ifndef ITP_UTSNAME_H
#define ITP_UTSNAME_H

#ifdef __cplusplus
extern "C"
{
#endif

#define __UTSNAMELEN 65	/* synchronize with kernel */

struct utsname {
    char sysname[__UTSNAMELEN];
    char nodename[__UTSNAMELEN];
    char release[__UTSNAMELEN];
    char version[__UTSNAMELEN];
    char machine[__UTSNAMELEN];
    char domainname[__UTSNAMELEN];
};

int uname(struct utsname *name);

#ifdef __cplusplus
}
#endif

#endif // ITP_UTSNAME_H

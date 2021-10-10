#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

struct _reent
{
  /* As an exception to the above put _errno first for binary
     compatibility with non _REENT_SMALL targets.  */
  int _errno;			/* local copy of errno */
};

struct _reent* __getreent(void);

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */

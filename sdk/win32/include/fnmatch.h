#ifndef	_FNMATCH_H_
#define	_FNMATCH_H_

#define	FNM_NOMATCH	1	/* Match failed. */

#define	FNM_NOESCAPE	0x01	/* Disable backslash escaping. */
#define	FNM_PATHNAME	0x02	/* Slash must be matched by slash. */
#define	FNM_PERIOD	0x04	/* Period must be matched by period. */

#if defined(_GNU_SOURCE) || !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
#define	FNM_LEADING_DIR	0x08	/* Ignore /<tail> after Imatch. */
#define	FNM_CASEFOLD	0x10	/* Case insensitive search. */
#define	FNM_IGNORECASE	FNM_CASEFOLD
#define	FNM_FILE_NAME	FNM_PATHNAME
#endif

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

int	 fnmatch(const char *, const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* !_FNMATCH_H_ */

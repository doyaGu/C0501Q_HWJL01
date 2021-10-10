﻿/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have a working `mmap' system call. */
#define HAVE_MMAP 1

/* Define to 1 if you have the `munmap' function. */
#define HAVE_MUNMAP 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "tslib"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "kergoth@handhelds.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "tslib"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "tslib 1.0.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "tslib"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.0"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to the type of arg 1 for `select'. */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for `select'. */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg 5 for `select'. */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* whether arctic2 should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_ARCTIC2_MODULE */

/* whether collie should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_COLLIE_MODULE */

/* whether corgi should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_CORGI_MODULE */

/* whether cy8mrln-palmpre should be build as part of libts or as a separate
   shared library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_CY8MRLN_PALMPRE_MODULE */

/* whether dejitter should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
#define TSLIB_STATIC_DEJITTER_MODULE 1

/* whether dmc should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_DMC_MODULE */

/* whether h3600 should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_H3600_MODULE */

/* whether input should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_INPUT_MODULE */

/* whether linear-h2200 should be build as part of libts or as a separate
   shared library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_LINEAR_H2200_MODULE */

/* whether linear should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
#define TSLIB_STATIC_LINEAR_MODULE 1

/* whether mk712 should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_MK712_MODULE */

/* whether pthres should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
#define TSLIB_STATIC_PTHRES_MODULE 1

/* whether tatung should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_TATUNG_MODULE */

/* whether touchkit should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_TOUCHKIT_MODULE */

/* whether ucb1x00 should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
/* #undef TSLIB_STATIC_UCB1X00_MODULE */

/* whether variance should be build as part of libts or as a separate shared
   library which is dlopen()-ed at runtime */
#define TSLIB_STATIC_VARIANCE_MODULE 1

#define TSLIB_STATIC_CASTOR3_MODULE 1

/* Version number of package */
#define VERSION "1.0.0"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

#ifndef ITP_DLFCN_H
#define ITP_DLFCN_H

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */
#define RTLD_NOW	0x00002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK   0x3	/* Mask of binding time value.  */
#define RTLD_NOLOAD	0x00004	/* Do not load the object.  */

#define RTLD_GLOBAL	0x00100

#ifdef __cplusplus
extern "C"
{
#endif

/* Open the shared object FILE and map it in; return a handle that can be
   passed to `dlsym' to get symbol values from it.  */
extern void *dlopen (const char *__file, int __mode);

/* Unmap and close a shared object opened by `dlopen'.
   The handle cannot be used again after calling `dlclose'.  */
extern int dlclose (void *__handle);

/* Find the run-time address in the shared object HANDLE refers to
   of the symbol called NAME.  */
extern void *dlsym (void * __handle,
		    const char * __name);

/* When any of the above functions fails, call this function
   to return a string describing the error.  Each call resets
   the error string so that a following call returns null.  */
extern char *dlerror (void);

#ifdef __cplusplus
}
#endif

#endif // ITP_DLFCN_H

#ifndef ITP_EXECINFO_H
#define ITP_EXECINFO_H

/* Store up to SIZE return address of the current program state in
   ARRAY and return the exact number of values stored.  */
extern int backtrace (void **__array, int __size);

/* Return names of functions from the backtrace list in ARRAY in a newly
   malloc()ed memory block.  */
extern char **backtrace_symbols (void *const *__array, int __size);

/* This function is similar to backtrace_symbols() but it writes the result
   immediately to a file.  */
extern void backtrace_symbols_fd (void *const *__array, int __size, int __fd);

#endif // ITP_EXECINFO_H

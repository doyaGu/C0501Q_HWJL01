#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <execinfo.h>
#include "ite/ith.h"

#define BT_MAX_DEPTH    (128)
#define BT_GET_DEPTH    (1)
#define BT_ADD_CR       (1)

static char **
_backtrace_symbols(void *const *buffer, int depth, int add_cr)
{
    char   *line[BT_MAX_DEPTH];
    int    i, x;
    char   **rv = NULL, *current;
    char   *cr;
    size_t sz, csz;

    if (buffer == NULL || depth <= 0)
        return(NULL);

    if (add_cr == BT_ADD_CR)
        cr = "\n";
    else
        cr = "";

    for (i = 0, sz = 0; i < depth; i++)
    {
        if (asprintf(&line[i], "%p%s",
                     buffer[i],
                     cr) == -1)
            goto unwind;

        sz += strlen(line[i]) + 1;
    }

    /* adjust for array */
    sz += depth * sizeof(char *);

    rv = malloc(sz);
    if (rv == NULL)
        goto unwind;

    current = (char *) &rv[depth];
    for (x = 0; x < depth; x++)
    {
        rv[x] = current;
        csz   = strlcpy(current, line[x], sz - (current - (char *) rv));
        if (csz >= sz)
        {
            free(rv);
            rv = NULL;
            goto unwind;
        }
        current += csz + 1;
    }
 unwind:
    while (--i >= 0)
        free(line[i]);

    return(rv);
}

char **
backtrace_symbols(void *const *buffer, int depth)
{
    return(_backtrace_symbols(buffer, depth, 0));
}

void itpPrintBacktrace(void)
{
#define BACKTRACE_SIZE 100
    static void *btbuf[BACKTRACE_SIZE];
    int i, btcount = backtrace(btbuf, BACKTRACE_SIZE);

    ithPrintf("Backtrace: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");
}

#ifndef ITP_GETOPT_H
#define ITP_GETOPT_H

#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum { NO_ARG, REQUIRED_ARG, OPTIONAL_ARG };
/* Define glibc names as well for compatibility.  */
#define no_argument NO_ARG
#define required_argument REQUIRED_ARG
#define optional_argument OPTIONAL_ARG

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

/* externally-defined variables */
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

int getopt(int argc, char *const argv[], const char *opstring);

int getopt_long(int argc,char *const argv[],const char *optstring,
  const struct option *longopts,int *longindex);

int getopt_long_only(int argc,char *const argv[],const char *optstring,
  const struct option *longopts,int *longindex);

#ifdef __cplusplus
}
#endif

#endif // ITP_GETOPT_H

#ifndef ITP_FTW_H
#define ITP_FTW_H

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
{
#endif
/*
 * Valid flags for the 3rd argument to the function that is passed as the
 * second argument to ftw(3) and nftw(3).  Say it three times fast!
 */
#define	FTW_F		0	/* File.  */
#define	FTW_D		1	/* Directory.  */
#define	FTW_DNR		2	/* Directory without read permission.  */
#define	FTW_DP		3	/* Directory with subdirectories visited.  */
#define	FTW_NS		4	/* Unknown type; stat() failed.  */
#define	FTW_SL		5	/* Symbolic link.  */
#define	FTW_SLN		6	/* Sym link that names a nonexistent file.  */

/*
 * Flags for use as the 4th argument to nftw(3).  These may be ORed together.
 */
#define	FTW_PHYS	0x01	/* Physical walk, don't follow sym links.  */
#define	FTW_MOUNT	0x02	/* The walk does not cross a mount point.  */
#define	FTW_DEPTH	0x04	/* Subdirs visited before the dir itself. */
#define	FTW_CHDIR	0x08	/* Change to a directory before reading it. */

struct FTW {
	int base;
	int level;
};

int ftw(const char *dirpath,
        int (*fn) (const char *fpath, const struct stat *sb,
                   int typeflag),
        int nopenfd);
int nftw(const char *dirpath,
        int (*fn) (const char *fpath, const struct stat *sb,
                   int typeflag, struct FTW *ftwbuf),
        int nopenfd, int flags);

#ifdef __cplusplus
}
#endif

#endif // ITP_FTW_H


#ifndef ITP_DIRENT_H
#define ITP_DIRENT_H

#include <sys/types.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _dirdesc {
	int dd_fd;
	long	dd_loc;
	long	dd_size;
	char	*dd_buf;
	int	dd_len;
	long	dd_seek;
} DIR;

#pragma pack(1)
struct dirent {
	long	d_ino;
	off_t	d_off;
	unsigned short	d_reclen;
	unsigned char   d_type;
	/* we need better syntax for variable-sized arrays */
	unsigned short	d_namlen;
	char		d_name[NAME_MAX + 1];
};
#pragma pack()

#define DT_UNKNOWN       0
#define DT_FIFO          1
#define DT_CHR           2
#define DT_DIR           4
#define DT_BLK           6
#define DT_REG           8
#define DT_LNK          10
#define DT_SOCK         12
#define DT_WHT          14

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
void rewinddir(DIR *);
int closedir(DIR *);
int scandir (const char *__dir,
             struct dirent ***__namelist,
             int (*select) (const struct dirent *),
             int (*compar) (const struct dirent **, const struct dirent **));
int alphasort (const struct dirent **__a, const struct dirent **__b);

#define MAXNAMLEN 1024

#ifdef __cplusplus
}
#endif

#endif // ITP_DIRENT_H

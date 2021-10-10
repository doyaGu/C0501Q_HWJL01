#ifndef FILELIST_H
#define FILELIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FILE_INDEX 100

typedef unsigned char ** FILELIST;

int filelist_reset(FILELIST filelist);

int filelist_add(FILELIST filelist, const unsigned char *file_path);

int filelist_remove(FILELIST filelist, const unsigned char *file_path);

int filelist_show_num(FILELIST filelist);

int filelist_show_max_num(FILELIST filelist);

FILELIST filelist_init(void);

void filelist_deinit(FILELIST filelist);

void filelist_add_files_from_all_drives(FILELIST filelist, const char* exts);

void filelist_dump(FILELIST filelist);

int filelist_recursive_add_files_from_path(FILELIST filelist, const char *path, const char* exts);

#ifdef __cplusplus
}
#endif

#endif

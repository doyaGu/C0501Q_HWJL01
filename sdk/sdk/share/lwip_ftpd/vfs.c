/* Copyright (c) 2013, Philipp Tölke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vfs.h"
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "ite/itp.h"

/* dirent that will be given to callers;
 * note: both APIs assume that only one dirent ever exists
 */
static vfs_dirent_t dir_ent;

static FILE guard_for_the_whole_fs;
static vfs_dir_t root_dir;
static char current_dir[PATH_MAX];
static int root_dir_index;

static char* parse_path(const char* path)
{
    static char buf[PATH_MAX];

    if (path[0] == '/')
    {
        if (path[1] == '\0')
        {
            return NULL;
        }
        buf[0] = path[2];
        buf[1] = ':';
        strcpy(&buf[2], &path[3]);

        if (strlen(buf) <= 2)
            strcat(buf, "/");
    }
    else
    {
        if (current_dir[1] == '\0')
        {
            if (path[0] == '$')
            {
                buf[0] = path[1];
                buf[1] = ':';
                buf[2] = '/';
                buf[3] = '\0';
            }
            else
            {
                strcpy(buf, "A:/");
                strcat(buf, path);
            }
        }
        else
        {
            buf[0] = current_dir[2];
            buf[1] = ':';
            strcpy(&buf[2], &current_dir[3]);
            //strcat(buf, "/");
            strcat(buf, path);
        }
    }
    return buf;
}

int vfs_read (void* buffer, int dummy, int len, vfs_file_t* file) {
	unsigned int bytesread = fread(buffer, 1, len, file);
	return bytesread;
}

vfs_dirent_t* vfs_readdir(vfs_dir_t* dir) {

    if (dir == &root_dir)
    {
        ITPDriveStatus* driveStatusTable;
        int i;

        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

        for (i = root_dir_index; i < ITP_MAX_DRIVE; i++)
        {
            ITPDriveStatus* driveStatus = &driveStatusTable[i];
            if (driveStatus->avail)
            {
                //printf("ftpd drive: %s\n", driveStatus->name);
                dir_ent.name[0] = '$';
                dir_ent.name[1] = driveStatus->name[0];
                dir_ent.name[2] = '\0';
                //strcpy(&dir_ent.name[2], &driveStatus->name[2]);
                root_dir_index = i + 1;
                break;
            }
        }
        if (i >= ITP_MAX_DRIVE)
        {
            return NULL;
        }
    }
    else
    {
        struct dirent *pent = readdir(dir);
	    if (!pent) return NULL;
	    memcpy(dir_ent.name, pent->d_name, sizeof(dir_ent.name));
    }
	return &dir_ent;
}

int vfs_stat(vfs_t* vfs, const char* filename, vfs_stat_t* st) {

    if (filename[0] == '$')
    {
	    st->st_size = 0;
	    st->st_mode = S_IFDIR;
	    st->st_mtime = CFG_RTC_DEFAULT_TIMESTAMP;
    }
    else
    {
        struct stat f;
        char* name = parse_path(filename);
	    if (0 != stat(name, &f)) {
		    return 1;
	    }
	    st->st_size = f.st_size;
	    st->st_mode = f.st_mode;
	    st->st_mtime = f.st_mtime;
    }
	return 0;
}

void vfs_close(vfs_t* vfs) {
	if (vfs && vfs != &guard_for_the_whole_fs) {
		/* Close a file */
		fclose(vfs);
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
	}
}

int vfs_write (void* buffer, int dummy, int len, vfs_file_t* file) {
	unsigned int byteswritten = fwrite(buffer, 1, len, file);
	return byteswritten;
}

vfs_t* vfs_openfs() {
    strcpy(current_dir, "/");
    root_dir_index = 0;
    
#ifdef CFG_FS_FAT
    // init fat on this task
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif    
	return &guard_for_the_whole_fs;
}

vfs_file_t* vfs_open(vfs_t* vfs, const char* filename, const char* mode) {
    char* name = parse_path(filename);
	vfs_file_t *f = fopen(name, mode);
	return f;
}

int vfs_chdir(vfs_t* vfs, const char* dir) {
    if (strcmp(dir, "..") == 0)
    {
        if (strcmp(current_dir, "/") != 0)
        {
            char* ptr;

            current_dir[strlen(current_dir) - 1] = '\0';

            ptr = strrchr(current_dir, '/');
            if (ptr)
            {
                *(ptr + 1) = '\0';
            }
        }
    }
    else if (dir[0] == '/')
    {
        strcpy(current_dir, dir);
        if (current_dir[strlen(current_dir) - 1] != '/')
            strcat(current_dir, "/");
    }
    else
    {
        strcat(current_dir, dir);
        if (current_dir[strlen(current_dir) - 1] != '/')
            strcat(current_dir, "/");
    }
    return 0;
}

char* vfs_getcwd(vfs_t* vfs, void* dummy1, int dummy2) {
    char* cwd = malloc(PATH_MAX);
	if (!cwd) {
		return NULL;
	}
    strcpy(cwd, current_dir);
	return cwd;
}

vfs_dir_t* vfs_opendir(vfs_t* vfs, const char* path) {
    vfs_dir_t* dir;
    char* name = parse_path(path);
    if (name == NULL)
        return &root_dir;

	dir = opendir(name);
	return dir;
}

void vfs_closedir(vfs_dir_t* dir) {
    if (dir == &root_dir)
    {
        root_dir_index = 0;
    }
    else
    {
	    closedir(dir);
    }
}

int vfs_rename(vfs_t* vfs, const char *from, const char *to)
{
    char *fromname, *toname, buf[PATH_MAX];
    int ret;
    
    fromname = parse_path(from);
    if (!fromname)
        return -1;

    strcpy(buf, fromname);
    
    toname = parse_path(to);
    if (!toname)
        return -1;

    ret = rename(buf, toname);
    if (ret == 0)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
    }
    return ret;
}

int vfs_mkdir(vfs_t* vfs, const char *name, mode_t mode)
{
    char* path = parse_path(name);
    int ret = mkdir(path, mode);
    if (ret == 0)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
    }
    return ret;
}

int vfs_rmdir(vfs_t* vfs, const char *name)
{
    char* path = parse_path(name);
    int ret = rmdir(path);
    if (ret == 0)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
    }
    return ret;
}

int vfs_remove(vfs_t* vfs, const char *name)
{
    char* path = parse_path(name);
    int ret = remove(path);
    if (ret == 0)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
    }
    return ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include "avformat.h"
#include "ite/itp.h"
#include "filelist.h"

#define FILENAME_LEN  256
#define COMPARE_EQUAL 0

static int filelist_count = 0;

int filelist_reset(FILELIST filelist)
{
    int i;

    // Create buffer
    if (!filelist)
        return -1;

    for (i = 0; i < MAX_FILE_INDEX; i++)
        if (filelist[i])
        {
            free(filelist[i]);
            filelist[i] = NULL;
        }

    return filelist_count = 0;
}

int filelist_add(FILELIST filelist, const unsigned char *file_path)
{
    int i;

    if ((!filelist) || (!file_path) || (file_path[0] == '\0'))
        return -1;

    for (i = 0; i < MAX_FILE_INDEX; i++)
    {
        if (NULL == filelist[i])
            break;
    }

    if (i >= MAX_FILE_INDEX)
    {
        printf("[%s:%d] filelist full\n", __FUNCTION__, __LINE__);
        return -1;
    }

    filelist[i] = strdup(file_path);
    if (filelist[i])
        ++filelist_count;
    return filelist_count;
}

int filelist_remove(FILELIST filelist, const unsigned char *file_path)
{
    int i;

    if ((!filelist) || (!file_path) || (file_path[0] == '\0'))
        return -1;

    for (i = 0; i < MAX_FILE_INDEX; i++)
    {
        if ((filelist[i] != NULL) && (strcmp(filelist[i], file_path) == 0))
        {
            free(filelist[i]);
            filelist[i] = NULL;
            return --filelist_count;
        }
    }

    return -1;
}

int filelist_show_num(FILELIST filelist)
{
    int i;
    int count = 0;

    if (!filelist)
        return -1;

    for (i = 0; i < MAX_FILE_INDEX; i++)
    {
        if (filelist[i])
            ++count;
    }

    assert(count == filelist_count);
    return filelist_count;
}

int filelist_show_max_num(FILELIST filelist)
{
    return MAX_FILE_INDEX;
}

FILELIST filelist_init(void)
{
    FILELIST filelist = NULL;

    if (filelist == NULL)
    {
        filelist       = calloc(MAX_FILE_INDEX, sizeof(unsigned char *));
        filelist_count = 0;
    }
    else
    {
        filelist_reset(filelist);
    }

    return filelist;
}

void filelist_deinit(FILELIST filelist)
{
    if (filelist)
    {
        int i;

        for (i = 0; i < MAX_FILE_INDEX; i++)
        {
            if (filelist[i])
                free(filelist[i]);
        }

        free(filelist);
    }
    filelist_count = 0;
}

int filelist_recursive_add_files_from_path(FILELIST filelist, const char *path, const char *exts)
{
    //char delimiter = '\\'; // Windows Delimiter
    char          delimiter = '/'; // Linux Delimiter
    struct dirent *filename = NULL;

    DIR           *dp       = opendir(path);
    printf("path: %s\n", path);

    if (!dp)
    {
        const char *short_name = strrchr(path, '.') + 1;

        if (av_find_input_format(short_name))
        {
            //printf("%d, %s ,short name [%s]\n", filelist_show_num(filelist), path, short_name);
            filelist_add(filelist, path);
        }

        return 1;
    }

    while (filename = readdir(dp))
    {
        int         pathLength, i;
        char        *pathStr;
        struct stat st = {0};

        if (strcmp(filename->d_name, "..") == COMPARE_EQUAL 
         || strcmp(filename->d_name, ".")  == COMPARE_EQUAL)
        {
            continue;
        }

        pathLength = strlen(path) + strlen(filename->d_name) + 2;
        pathStr    = (char *)malloc(sizeof(char) * pathLength);
        strcpy(pathStr, path);

        i          = strlen(pathStr);
        if (pathStr[i - 1] != delimiter)
        {
            pathStr[i]     = delimiter;
            pathStr[i + 1] = '\0';
        }

        strcat(pathStr, filename->d_name);

        stat(pathStr, &st);
        if (S_ISDIR(st.st_mode))
            filelist_recursive_add_files_from_path(filelist, pathStr, exts);
        else
        {
            int ret = CheckExts(pathStr, exts);
            if (ret)
                filelist_add(filelist, pathStr);
        }
        free(pathStr);
    }

    closedir(dp);

    return 1;
}

void filelist_add_files_from_all_drives(FILELIST filelist, const char *exts)
{
    ITPDriveStatus *driveStatusTable;
    int            i, s = 0;

    filelist_reset(filelist);
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

#ifndef WIN32
    for (i = 0; i < ITP_MAX_DRIVE; i++)
#else
    for (i = 0; i < 2; i++)
#endif
    {
        ITPDriveStatus *drive = &driveStatusTable[i];
        if (drive && drive->avail)
        {
            printf("Start scan %s ... ", drive->name);
            filelist_recursive_add_files_from_path(filelist, drive->name, exts);
            filelist_dump(filelist);
            printf("complete!\n");
            s++;
        }
    }

    if (!s)
        printf("Can not find storage, please insert!\n");
}

void filelist_dump(FILELIST filelist)
{
    if (filelist)
    {
        int i;

        printf("===========================\n");
        printf("         file list\n");
        printf("---------------------------\n");
        for (i = 0; i < MAX_FILE_INDEX; i++)
            if (filelist[i])
                printf("%s\n", filelist[i]);
        printf("===========================\n");
    }
}
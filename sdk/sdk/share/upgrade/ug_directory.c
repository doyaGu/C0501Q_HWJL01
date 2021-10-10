#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#pragma pack(1)
typedef struct
{
    uint32_t dirname_size;
} directory_t;

int ugUpgradeDirectory(ITCStream *f, char* drive)
{
    int ret;
    directory_t dir;
    char buf[PATH_MAX];
    char *p = NULL;
    size_t len, readsize;

    // read directory
    readsize = itcStreamRead(f, &dir, sizeof(directory_t));
    if (readsize != sizeof(directory_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(directory_t) LOG_END
        return ret;
    }

    dir.dirname_size = itpLetoh32(dir.dirname_size);
    LOG_DBG "dirname_size=%d\n", dir.dirname_size LOG_END

    strcpy(buf, drive);

    readsize = itcStreamRead(f, &buf[2], dir.dirname_size);
    if (readsize != dir.dirname_size)
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, dir.dirname_size LOG_END
        return ret;
    }

    LOG_INFO "[%d%%] Create directory %s\n", ugGetProrgessPercentage(), buf LOG_END

    if (chdir(drive))
    {
        ret = __LINE__;
        LOG_ERR "chdir %s fail: %d\n", drive, errno LOG_END
        return ret;
    }

    len = strlen(buf);
    if (buf[len - 1] == '/')
           buf[len - 1] = 0;

    ret = mkdir(buf, S_IRWXU);
    if (ret == -1)
    {
        LOG_WARN "mkdir %s fail: %d\n", buf, errno LOG_END
    }

    return 0;
}

int ugCopyDirectory(const char* dest, const char* src)
{
    DIR           *dir;
    struct dirent *ent;
    int ret = 0;

    dir = opendir(src);
    if (dir == NULL)
    {
        LOG_ERR "cannot open directory %s\n", src LOG_END
        ret = __LINE__;
        goto end;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        char destPath[PATH_MAX];
        int ret1;

        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        strcpy(destPath, dest);

        if (access(dest, W_OK))
        {
            ret1 = mkdir(destPath, S_IRWXU);
            if (ret1)
            {
                LOG_WARN "cannot create dir %s: %d\n", destPath, errno LOG_END

                if (ret == 0)
                    ret = ret1;
            }
        }
        strcat(destPath, "/");

        if (ent->d_type == DT_DIR)
        {
            char srcPath[PATH_MAX];

            strcat(destPath, ent->d_name);
            
            ret1 = mkdir(destPath, S_IRWXU);
            if (ret1)
            {
                LOG_WARN "cannot create dir %s: %d\n", destPath, errno LOG_END

                if (ret == 0)
                    ret = ret1;
            }
            else
            {
                strcpy(srcPath, src);
                strcat(srcPath, "/");
                strcat(srcPath, ent->d_name);

                ret1 = ugCopyDirectory(destPath, srcPath);
                if (ret1)
                {
                    if (ret == 0)
                        ret = ret1;
                }
            }
        }
        else
        {
            char srcPath[PATH_MAX];

            strcat(destPath, ent->d_name);
            strcpy(srcPath, src);
            strcat(srcPath, "/");
            strcat(srcPath, ent->d_name);

            ret1 = ugCopyFile(destPath, srcPath);
            if (ret1)
            {
                if (ret == 0)
                    ret = ret1;
            }
        }
    }

end:
    if (dir)
    {
        if (closedir(dir))
            LOG_WARN "cannot closedir (%s)\n", src LOG_END
    }
    return ret;
}

void ugDeleteDirectory(char* dirname)
{
    DIR           *dir;
    struct dirent *ent;

    dir = opendir(dirname);
    if (dir == NULL)
        return;

    while ((ent = readdir(dir)) != NULL)
    {
        char path[PATH_MAX];

        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        strcpy(path, dirname);
        strcat(path, "/");
        strcat(path, ent->d_name);

        if (ent->d_type == DT_DIR)
        {
            ugDeleteDirectory(path);
        }
        else
        {
            remove(path);
        }
    }
    closedir(dir);
    rmdir(dirname);
}

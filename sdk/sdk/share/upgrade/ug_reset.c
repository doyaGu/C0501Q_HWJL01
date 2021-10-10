#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>
#include "ite/ug.h"
#include "ug_cfg.h"

static unsigned long GetFileSize(const char *input)
{
    struct stat st;

    st.st_size = 0;
    stat(input, &st);

    return (unsigned long)st.st_size;
}

int ugCopyFile(const char* destPath, const char* srcPath)
{
    int ret = 0;
    FILE *rf = NULL;
    FILE *wf = NULL;
    uint8_t* buf = NULL;
    uint8_t* buf2 = NULL;
    uint32_t filesize, bufsize, readsize, size2, remainsize;
    struct statvfs info;

    filesize = GetFileSize(srcPath);

    LOG_INFO "Copy %s (%ld bytes)", srcPath, filesize LOG_END

    // check capacity
    if (statvfs(destPath, &info) == 0)
    {
        uint64_t avail = info.f_bfree * info.f_bsize;
        struct stat buffer;

        if (stat(destPath, &buffer) == 0)
            avail += buffer.st_size;

        if (filesize > avail)
        {
            ret = __LINE__;
            LOG_ERR "Out of space: %d > %d\n", filesize, avail LOG_END
            goto end;
        }
    }

    if ((rf = fopen(srcPath, "rb")) == NULL)
    {
        ret = __LINE__;
        LOG_ERR "cannot open file %s: %d\n", srcPath, errno LOG_END
        goto end;
    }

    bufsize = CFG_UG_BUF_SIZE < filesize ? CFG_UG_BUF_SIZE : filesize;
    buf = malloc(bufsize);
    if (!buf)
    {
        ret = __LINE__;
        LOG_ERR "Out of memory: %ld\n", bufsize LOG_END
        goto end;
    }

    if ((wf = fopen(destPath, "wb")) == NULL)
    {
        ret = __LINE__;
        LOG_ERR "cannot open file %s: %d\n", destPath, errno LOG_END
        goto end;
    }

    remainsize = filesize;
    do
    {
        readsize = (remainsize < bufsize) ? remainsize : bufsize;
        size2 = fread(buf, 1, readsize, rf);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        size2 = fwrite(buf, 1, readsize, wf);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot write file: %ld != %ld\n", size2, readsize LOG_END
                goto end;
        }

        remainsize -= readsize;

        putchar('.');
        fflush(stdout);
    }
    while (remainsize > 0);

    putchar('\n');

    // verify
    fclose(wf);
    wf = NULL;

    buf2 = malloc(bufsize);
    if (!buf2)
    {
        ret = __LINE__;
        LOG_ERR "Out of memory: %ld\n", bufsize LOG_END
        goto end;
    }

    wf = fopen(destPath, "rb");
    if (!wf)
    {
        ret = __LINE__;
        LOG_ERR "Cannot open file %s: %d\n", destPath, errno LOG_END
        goto end;
    }

    if (fseek(rf, 0, SEEK_SET))
    {
        ret = __LINE__;
        LOG_ERR "Cannot seek file %d\n", errno LOG_END
            goto end;
    }

    LOG_INFO "Verifying" LOG_END

    remainsize = filesize;
    do
    {
        readsize = (remainsize < bufsize) ? remainsize : bufsize;
        size2 = fread(buf, 1, readsize, rf);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        size2 = fread(buf2, 1, readsize, wf);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        if (memcmp(buf, buf2, readsize))
        {
            ret = __LINE__;
            LOG_ERR "\nVerify failed.\n" LOG_END
            goto end;
        }

        remainsize -= readsize;

        putchar('.');
        fflush(stdout);
    } while (remainsize > 0);
    putchar('\n');

end:
    if (rf)
        fclose(rf);

    if (wf)
        fclose(wf);

    if (buf2)
        free(buf2);

    if (buf)
        free(buf);

    return ret;
}

int ugRestoreDir(const char* dest, const char* src)
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
        strcat(destPath, "/");

        if (ent->d_type == DT_DIR)
        {
            if (ent->d_name[0] == '~')
            {
                strcat(destPath, &ent->d_name[1]);
                LOG_INFO "Remove dir %s\n", destPath LOG_END
                ugDeleteDirectory(destPath);
            }
            else
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

                    ret1 = ugRestoreDir(destPath, srcPath);
                    if (ret1)
                    {
                        if (ret == 0)
                            ret = ret1;
                    }
                }
            }
        }
        else
        {
            if (ent->d_name[0] == '~')
            {
                strcat(destPath, &ent->d_name[1]);

                LOG_INFO "Remove file %s\n", destPath LOG_END
                ret1 = remove(destPath);
                if (ret1)
                {
                    LOG_WARN "cannot remove file %s: %d\n", destPath, errno LOG_END
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
    }

end:
    if (dir)
    {
        if (closedir(dir))
            LOG_WARN "cannot closedir (%s)\n", src LOG_END
    }
    return ret;
}

int ugResetFactory(void)
{
    DIR           *dir;
    struct dirent *ent;
    int ret = 0;

    dir = opendir(CFG_PRIVATE_DRIVE ":/backup");
    if (dir == NULL)
    {
        LOG_ERR "cannot open directory %s\n", CFG_PRIVATE_DRIVE ":/backup" LOG_END
        ret = __LINE__;
        goto end;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        if (ent->d_type == DT_DIR)
        {
            char destPath[PATH_MAX];
            char srcPath[PATH_MAX];
            int ret1;

            strcpy(destPath, ent->d_name);
            strcat(destPath, ":");
            strcpy(srcPath, CFG_PRIVATE_DRIVE ":/backup/");
            strcat(srcPath, ent->d_name);

            ret1 = ugRestoreDir(destPath, srcPath);
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
            LOG_WARN "cannot closedir (%s)\n", CFG_PRIVATE_DRIVE ":/backup" LOG_END
    }
    return ret;
}

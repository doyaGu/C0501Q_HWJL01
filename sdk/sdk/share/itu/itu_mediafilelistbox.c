#include <assert.h>
#include <dirent.h>
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <unistd.h>
#include "redblack/redblack.h"
#include "ite/itu.h"
#include "itu_cfg.h"

static const char mmflistboxName[] = "ITUMediaFileListBox";

typedef struct
{
    char* name;
    char* path;
} FileEntry;

static int MediaFileListCompare(const void *pa, const void *pb, const void *config)
{
    FileEntry* fea = (FileEntry*) pa;
    FileEntry* feb = (FileEntry*) pb;
    assert(pa);
    assert(pb);

    return strcmp(fea->path, feb->path);
}

static void MediaFileListDestroy(ITUMediaFileListBox* mflistbox)
{
    assert(mflistbox);

    if (mflistbox->rbtree)
    {
        RBLIST *rblist;

	    if ((rblist = rbopenlist(mflistbox->rbtree)) != NULL)
	    {
            FileEntry* val;
            while ((val = (FileEntry*)rbreadlist(rblist)))
            {
                free(val->path);
                free(val);
            }
	        rbcloselist(rblist);
	    }
        rbdestroy(mflistbox->rbtree);
        mflistbox->rbtree = NULL;
    }
}

static void MediaFileListBoxParseDirectory(ITUMediaFileListBox* mflistbox, char* dir)
{
    DIR *pdir = NULL;
    char path[PATH_MAX];

    // Open the directory
    pdir = opendir(dir);
    if (pdir)
    {
        struct dirent *pent;

        if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
            goto end;

        // List the contents of the directory
        while ((pent = readdir(pdir)) != NULL)
        {
            FileEntry *fe, *ret;

            if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;

            if ((strcmp(pent->d_name, ".") == 0) || (strcmp(pent->d_name, "..") == 0))
                continue;

            strcpy(path, dir);
            strcat(path, "/");
            strcat(path, pent->d_name);

            if (pent->d_type == DT_DIR)
            {
                MediaFileListBoxParseDirectory(mflistbox, path);
                if (mflistbox->fileCount >= ITU_MEDIA_FILE_MAX_COUNT)
                    goto end;
            }
            else
            {
                bool found = false;

                if (mflistbox->extensions[0] != '\0')
                {
                    char* ptr = strrchr(path, '.');
                    if (ptr)
                    {
                        char buf[ITU_EXTENSIONS_SIZE];
                        char *ext, *saveptr;

                        ptr++;

                        strcpy(buf, mflistbox->extensions);
                        ext = strtok_r(buf, ";", &saveptr);

                        do
                        {
                            if (stricmp(ptr, ext) == 0)
                            {
                                found = true;
                                break;
                            }
                            ext = strtok_r(NULL, ";", &saveptr);
                        } while (ext);
                    }
                }
                else
                {
                    found = true;
                }
                if (found)
                {
                    fe = malloc(sizeof(FileEntry));
                    if (fe == NULL)
                    {
                        LOG_ERR "out of memory: %d\n", sizeof(FileEntry) LOG_END
                            goto end;
                    }
                    fe->path = strdup(path);
                    fe->name = strrchr(fe->path, '/') + 1;

                    ret = (FileEntry*)rbsearch((void *)fe, mflistbox->rbtree);
                    if (ret == NULL)
                    {
                        LOG_ERR "out of memory\n" LOG_END
                        free(fe->path);
                        free(fe);
                        goto end;
                    }
                    mflistbox->fileCount++;
                    if (mflistbox->fileCount >= ITU_MEDIA_FILE_MAX_COUNT)
                        goto end;
                }
            }

            if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;
        }
    }
    else
    {
        LOG_WARN "opendir(%s) failed\n", mflistbox->path LOG_END
            goto end;
    }

end:
    // Close the directory
    if (pdir)
        closedir(pdir);
}

static void* MediaFileListBoxCreateTask(void* arg)
{
    DIR *pdir = NULL;
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) arg;
    char path[PATH_MAX];
    assert(mflistbox);

    if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
        goto end;

    if (strlen(mflistbox->path) == 0)
    {
	    LOG_WARN "empty path\n" LOG_END
	    goto end;
    }

    MediaFileListDestroy(mflistbox);
    mflistbox->rbtree = rbinit(MediaFileListCompare, NULL);
    if (mflistbox->rbtree == NULL)
    {
	    LOG_ERR "insufficient memory\n" LOG_END
	    goto end;
    }

    if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
        goto end;

    // Open the directory
    pdir = opendir(mflistbox->path);
    if (pdir)
    {
        struct dirent *pent;

        if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
            goto end;

        mflistbox->fileCount = 0;

        // List the contents of the directory
        while ((pent = readdir(pdir)) != NULL)
        {
            FileEntry *fe, *ret;

            if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;

            if ((strcmp(pent->d_name, ".") == 0) || (strcmp(pent->d_name, "..") == 0))
                continue;

            strcpy(path, mflistbox->path);
            strcat(path, pent->d_name);

            if (pent->d_type == DT_DIR)
            {
                MediaFileListBoxParseDirectory(mflistbox, path);
                if (mflistbox->fileCount >= ITU_MEDIA_FILE_MAX_COUNT)
                    goto end;
            }
            else
            {
                bool found = false;

                if (mflistbox->extensions[0] != '\0')
                {
                    char* ptr = strrchr(path, '.');
                    if (ptr)
                    {
                        char buf[ITU_EXTENSIONS_SIZE];
                        char *ext, *saveptr;

                        ptr++;

                        strcpy(buf, mflistbox->extensions);
                        ext = strtok_r(buf, ";", &saveptr);

                        do
                        {
                            if (stricmp(ptr, ext) == 0)
                            {
                                found = true;
                                break;
                            }
                            ext = strtok_r(NULL, ";", &saveptr);
                        } while (ext);
                    }
                }
                else
                {
                    found = true;
                }
                if (found)
                {
                    fe = malloc(sizeof(FileEntry));
                    if (fe == NULL)
                    {
                        LOG_ERR "out of memory: %d\n", sizeof(FileEntry) LOG_END
                            goto end;
                    }
                    fe->path = strdup(path);
                    fe->name = strrchr(fe->path, '/') + 1;

                    ret = (FileEntry*)rbsearch((void *)fe, mflistbox->rbtree);
                    if (ret == NULL)
                    {
                        LOG_ERR "out of memory\n" LOG_END
                        free(fe->path);
                        free(fe);
                        goto end;
                    }
                    mflistbox->fileCount++;
                    if (mflistbox->fileCount >= ITU_MEDIA_FILE_MAX_COUNT)
                        goto end;
                }
            }

            if (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;
        }
    }
        
end:
    // Close the directory
    if (pdir)
        closedir(pdir);

    mflistbox->mflistboxFlags &= ~ITU_FILELIST_BUSYING;
    mflistbox->mflistboxFlags |= ITU_FILELIST_CREATED;
    return NULL;
}

void ituMediaFileListBoxExit(ITUWidget* widget)
{
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) widget;
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    if (mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING)
        mflistbox->mflistboxFlags |= ITU_FILELIST_DESTROYING;

    while (mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING)
        usleep(1000);

    MediaFileListDestroy(mflistbox);

    if (mflistbox->randomPlayedArray)
    {
        free(mflistbox->randomPlayedArray);
        mflistbox->randomPlayedArray = NULL;
    }
    ituWidgetExitImpl(widget);
}

bool ituMediaFileListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) widget;
    assert(mflistbox);

    result = ituListBoxUpdate(widget, ev, arg1, arg2, arg3);

    if ((ev == ITU_EVENT_TIMER) && (mflistbox->mflistboxFlags & ITU_FILELIST_CREATED))
    {
        FileEntry* val;
        ITCTree* node = widget->tree.child;
        int count;

        mflistbox->mflistboxFlags &= ~ITU_FILELIST_CREATED;

        if (node == NULL)
            return result;
        
        for (val = (FileEntry*)rblookup(RB_LUFIRST, NULL, mflistbox->rbtree);
             val != NULL; 
             val = (FileEntry*)rblookup(RB_LUNEXT, val, mflistbox->rbtree))
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;

            ituScrollTextSetString(scrolltext, val->name);
            ituWidgetSetCustomData(scrolltext, val->path);

            node = node->sibling;

            if (node == NULL)
                break;
        }

        for (; node; node = node->sibling)
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;
            ituScrollTextSetString(scrolltext, "");
        }

        count = itcTreeGetChildCount(widget);
        if (count > 0)
        {
            char buf[32];

            if (mflistbox->fileCount == 0)
            {
                listbox->pageIndex = listbox->pageCount = 0;
            }
            else
            {
                listbox->pageIndex = 1;
				listbox->pageCount = mflistbox->fileCount ? (mflistbox->fileCount + count - 1) / count : 1;
            }

            if (listbox->pageIndex == listbox->pageCount)
            {
                listbox->itemCount = mflistbox->fileCount % count;
                if (listbox->itemCount == 0)
                    listbox->itemCount = count;
            }
            else
            {
                listbox->itemCount = count;
            }
            ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
            ituListBoxSelect(listbox, -1);

            sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
        }

        result = widget->dirty = true;
    }
    return widget->visible ? result : false;
}

void ituMediaFileListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) listbox;
    ITCTree* node;
    FileEntry* val;
    int i, count;
    assert(mflistbox);
    assert(pageIndex <= listbox->pageCount);

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING) ||
        !mflistbox->rbtree)
        return;

    count = itcTreeGetChildCount(listbox);

    if (pageIndex == listbox->pageCount)
    {
        listbox->itemCount = mflistbox->fileCount % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
    {
        listbox->itemCount = count;
    }

    if (mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING)
        return;

    node = ((ITCTree*)listbox)->child;
    i = 0;
    for (val = (FileEntry*)rblookup(RB_LUFIRST, NULL, mflistbox->rbtree);
         val != NULL; 
         val = (FileEntry*)rblookup(RB_LUNEXT, val, mflistbox->rbtree))
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        if (i++ < count * (pageIndex - 1))
            continue;

        ituScrollTextSetString(scrolltext, val->name);
        ituWidgetSetCustomData(scrolltext, val->path);

        node = node->sibling;

        if (node == NULL)
            break;
    }

    for (; node; node = node->sibling)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;
        ituScrollTextSetString(scrolltext, "");
    }
    listbox->pageIndex = pageIndex;
}

void ituMediaFileListBoxInit(ITUMediaFileListBox* mflistbox, int width, char* path)
{
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    memset(mflistbox, 0, sizeof (ITUMediaFileListBox));

    ituListBoxInit(&mflistbox->listbox, width);

    ituWidgetSetType(mflistbox, ITU_MEDIAFILELISTBOX);
    ituWidgetSetName(mflistbox, mmflistboxName);
    ituWidgetSetExit(mflistbox, ituMediaFileListBoxExit);
    ituWidgetSetUpdate(mflistbox, ituMediaFileListBoxUpdate);
    ituListBoxSetOnLoadPage(mflistbox, ituMediaFileListBoxOnLoadPage);

    if (path)
    {
        pthread_t task;
        pthread_attr_t attr;
        
        strncpy(mflistbox->path, path, ITU_PATH_MAX - 1);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&task, &attr, MediaFileListBoxCreateTask, mflistbox);
        mflistbox->mflistboxFlags |= ITU_FILELIST_BUSYING;
    }
}

void ituMediaFileListBoxLoad(ITUMediaFileListBox* mflistbox, uint32_t base)
{
    assert(mflistbox);

    ituListBoxLoad(&mflistbox->listbox, base);

    ituWidgetSetExit(mflistbox, ituMediaFileListBoxExit);
    ituWidgetSetUpdate(mflistbox, ituMediaFileListBoxUpdate);
    ituListBoxSetOnLoadPage(mflistbox, ituMediaFileListBoxOnLoadPage);

    if (strlen(mflistbox->path) > 0)
    {
        pthread_t task;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&task, &attr, MediaFileListBoxCreateTask, mflistbox);
        mflistbox->mflistboxFlags |= ITU_FILELIST_BUSYING;
    }
}

void ituMediaFileListSetPath(ITUMediaFileListBox* mflistbox, char* path)
{
    ITUListBox* listbox = (ITUListBox*) mflistbox;
    int len;
    pthread_t task;
    pthread_attr_t attr;
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING))
        return;

    if (path)
        strcpy(mflistbox->path, path);
    else
        mflistbox->path[0] = '\0';

    len = strlen(mflistbox->path);
    if (len < 2)
        return;

    if (mflistbox->randomPlayedArray)
    {
        free(mflistbox->randomPlayedArray);
        mflistbox->randomPlayedArray = NULL;
        mflistbox->playIndex = 0;
        mflistbox->randomPlayedCount = 0;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, MediaFileListBoxCreateTask, mflistbox);
    mflistbox->mflistboxFlags |= ITU_FILELIST_BUSYING;
}

ITUScrollText* ituMediaFileListPlay(ITUMediaFileListBox* mflistbox)
{
    ITUWidget* widget = (ITUWidget*) mflistbox;
    ITUListBox* listbox = (ITUListBox*) mflistbox;
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING) ||
        (mflistbox->fileCount == 0))
        return NULL;

    if (mflistbox->randomPlay)
    {
        time_t t;
        int i, tryCount;

        if (mflistbox->randomPlayedArray == NULL)
        {
            mflistbox->randomPlayedArray = malloc(mflistbox->fileCount * sizeof(int));
            if (mflistbox->randomPlayedArray == NULL)
                return NULL;
        }
        for (i = 0; i < mflistbox->fileCount; ++i)
            mflistbox->randomPlayedArray[i] = -1;

        mflistbox->playIndex = 0;
        mflistbox->randomPlayedCount = 0;

        tryCount = mflistbox->fileCount * 2;

        srand((unsigned) time(&t));

        do
        {
            if (--tryCount <= 0)
                break;

            mflistbox->playIndex = rand() % mflistbox->fileCount;
        } while (mflistbox->randomPlayedArray[mflistbox->playIndex] >= 0);

        if (tryCount <= 0)
        {
            for (i = 0; i < mflistbox->fileCount; i++)
            {
                if (mflistbox->randomPlayedArray[i] == -1)
                {
                    mflistbox->playIndex = i;
                    break;
                }
            }
        }

        if (mflistbox->playIndex < mflistbox->fileCount)
        {
            int count;
            char buf[32];

            mflistbox->randomPlayedArray[mflistbox->playIndex] = mflistbox->randomPlayedCount;
            mflistbox->randomPlayedCount++;

            if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
                count = itcTreeGetChildCount(mflistbox) / 3;
            else
                count = itcTreeGetChildCount(mflistbox);

            listbox->pageIndex = mflistbox->playIndex / count + 1;
            ituListBoxOnLoadPage(listbox, listbox->pageIndex);
            ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
            ituListBoxSelect(listbox, mflistbox->playIndex % count);
            
            sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);

            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
    }
    else
    {
        if (listbox->focusIndex >= 0)
        {
            int count;

            if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
                count = itcTreeGetChildCount(mflistbox) / 3;
            else
                count = itcTreeGetChildCount(mflistbox);

            mflistbox->playIndex = (listbox->pageIndex - 1) * count + listbox->focusIndex;
        }
        else
        {
            char buf[32];

            mflistbox->playIndex = 0;
            listbox->pageIndex = 1;
            ituListBoxOnLoadPage(listbox, listbox->pageIndex);
            ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
            ituListBoxSelect(listbox, 0);

            sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
        }
        return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
    }
    return NULL;
}

ITUScrollText* ituMediaFileListPrev(ITUMediaFileListBox* mflistbox)
{
    ITUWidget* widget = (ITUWidget*) mflistbox;
    ITUListBox* listbox = (ITUListBox*) mflistbox;
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    if (listbox->focusIndex == -1)
        return ituMediaFileListPlay(mflistbox);

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING) ||
        !mflistbox->rbtree)
        return NULL;

    if (mflistbox->randomPlay)
    {
        if (mflistbox->randomPlayedArray == NULL)
            return ituMediaFileListPlay(mflistbox);

        if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ONCE || mflistbox->randomPlayedCount <= 1)
        {
            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else if (mflistbox->randomPlayedCount > 1)
        {
            int i, index = mflistbox->randomPlayedCount - 2;

            for (i = 0; i < mflistbox->fileCount; i++)
            {
                if (mflistbox->randomPlayedArray[i] == index)
                {
                    mflistbox->randomPlayedArray[mflistbox->playIndex] = -1;
                    mflistbox->randomPlayedCount--;
                    mflistbox->playIndex = i;
                    break;
                }
            }

            if (i < mflistbox->fileCount)
            {
                char buf[32];
                int count;

                if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
                    count = itcTreeGetChildCount(mflistbox) / 3;
                else
                    count = itcTreeGetChildCount(mflistbox);

                listbox->pageIndex = mflistbox->playIndex / count + 1;
                ituListBoxOnLoadPage(listbox, listbox->pageIndex);
                ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
                ituListBoxSelect(listbox, mflistbox->playIndex % count);

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);

                return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
            }
        }
    }
    else
    {
        int count;

        if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
            count = itcTreeGetChildCount(mflistbox) / 3;
        else
            count = itcTreeGetChildCount(mflistbox);

        if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_NONE)
        {
            if (listbox->focusIndex == 0 && listbox->pageIndex == 1)
                return NULL;

            ituListBoxPrev(mflistbox);

            mflistbox->playIndex = (listbox->pageIndex - 1) * count + listbox->focusIndex;

            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ONCE)
        {
            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ALL)
        {
            ituListBoxPrev(mflistbox);

            mflistbox->playIndex = (listbox->pageIndex - 1) * count + listbox->focusIndex;

            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
    }
    return NULL;
}

ITUScrollText* ituMediaFileListNext(ITUMediaFileListBox* mflistbox)
{
    ITUWidget* widget = (ITUWidget*) mflistbox;
    ITUListBox* listbox = (ITUListBox*) mflistbox;
    assert(mflistbox);
    ITU_ASSERT_THREAD();

    if (listbox->focusIndex == -1)
        return ituMediaFileListPlay(mflistbox);

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING) ||
        !mflistbox->rbtree)
        return NULL;

    if (mflistbox->randomPlay)
    {
        if (mflistbox->randomPlayedArray == NULL)
            return ituMediaFileListPlay(mflistbox);

        if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ONCE)
        {
            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else
        {
            int i, tryCount;

            tryCount = mflistbox->fileCount * 2;

            do
            {
                if (--tryCount <= 0)
                    break;

                mflistbox->playIndex = rand() % mflistbox->fileCount;
            } while (mflistbox->randomPlayedArray[mflistbox->playIndex] >= 0);

            if (tryCount <= 0)
            {
                for (i = 0; i < mflistbox->fileCount; i++)
                {
                    if (mflistbox->randomPlayedArray[i] == -1)
                    {
                        mflistbox->playIndex = i;
                        break;
                    }
                }

                if (i == mflistbox->fileCount)
                {
                    if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_NONE)
                    {
                        return NULL;
                    }
                    else if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ALL)
                    {
                        return ituMediaFileListPlay(mflistbox);
                    }
                }
            }

            if (mflistbox->playIndex < mflistbox->fileCount)
            {
                char buf[32];
                int count;

                mflistbox->randomPlayedArray[mflistbox->playIndex] = mflistbox->randomPlayedCount;
                mflistbox->randomPlayedCount++;

                if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
                    count = itcTreeGetChildCount(mflistbox) / 3;
                else
                    count = itcTreeGetChildCount(mflistbox);

                listbox->pageIndex = mflistbox->playIndex / count + 1;
                ituListBoxOnLoadPage(listbox, listbox->pageIndex);
                ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
                ituListBoxSelect(listbox, mflistbox->playIndex % count);

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);

                return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
            }
        }
    }
    else
    {
        int count;

        if (widget->type == ITU_SCROLLMEDIAFILELISTBOX)
            count = itcTreeGetChildCount(mflistbox) / 3;
        else
            count = itcTreeGetChildCount(mflistbox);

        if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_NONE)
        {
            if (listbox->focusIndex == listbox->itemCount - 1 && listbox->pageIndex == listbox->pageCount)
                return NULL;

            ituListBoxNext(mflistbox);

            mflistbox->playIndex = (listbox->pageIndex - 1) * count + listbox->focusIndex;

            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ONCE)
        {
            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
        else if (mflistbox->repeatMode == ITU_MEDIA_REPEAT_ALL)
        {
            ituListBoxNext(mflistbox);

            mflistbox->playIndex = (listbox->pageIndex - 1) * count + listbox->focusIndex;

            return (ITUScrollText*)ituListBoxGetFocusItem(listbox);
        }
    }
    return NULL;
}

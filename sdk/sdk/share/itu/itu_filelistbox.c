#include <assert.h>
#include <dirent.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "redblack/redblack.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char flistboxName[] = "ITUFileListBox";

static int FileListCompare(const void *pa, const void *pb, const void *config)
{
    struct dirent* dira = (struct dirent*) pa;
    struct dirent* dirb = (struct dirent*) pb;
    assert(pa);
    assert(pb);

    if ((dira->d_type == DT_DIR) && (dirb->d_type != DT_DIR))
        return -1;

    if ((dira->d_type != DT_DIR) && (dirb->d_type == DT_DIR))
        return 1;

    return strcmp(dira->d_name, dirb->d_name);
}

static void FileListDestroy(ITUFileListBox* flistbox)
{
    assert(flistbox);

    if (flistbox->rbtree)
    {
        RBLIST *rblist;

	    if ((rblist = rbopenlist(flistbox->rbtree)) != NULL)
	    {
            void* val;
	        while ((val = (void*) rbreadlist(rblist)))
		        free(val);

	        rbcloselist(rblist);
	    }
        rbdestroy(flistbox->rbtree);
        flistbox->rbtree = NULL;
    }
}

static void* FileListBoxCreateTask(void* arg)
{
    DIR *pdir = NULL;
    ITUFileListBox* flistbox = (ITUFileListBox*) arg;
    assert(flistbox);

    if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
        goto end;

    if (strlen(flistbox->path) == 0)
    {
	    LOG_WARN "empty path\n" LOG_END
	    goto end;
    }

    FileListDestroy(flistbox);
    flistbox->rbtree = rbinit(FileListCompare, NULL);
    if (flistbox->rbtree == NULL)
    {
	    LOG_ERR "insufficient memory\n" LOG_END
	    goto end;
    }

    if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
        goto end;

    // Open the directory
    pdir = opendir(flistbox->path);
    if (pdir)
    {
        struct dirent *pent;

        if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
            goto end;

        flistbox->fileCount = 0;

        // List the contents of the directory
        while ((pent = readdir(pdir)) != NULL)
        {
            struct dirent *dir, *ret;

            if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;

            if ((strcmp(pent->d_name, ".") == 0) || (strcmp(pent->d_name, "..") == 0))
                continue;

            dir = malloc(sizeof (struct dirent));
	        if (dir == NULL)
	        {
                LOG_ERR "out of memory: %d\n", sizeof(struct dirent) LOG_END
		        goto end;
	        }

            if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;

            memcpy(dir, pent, sizeof (struct dirent));

		    ret = (struct dirent*) rbsearch((void *)dir, flistbox->rbtree);
		    if(ret == NULL)
		    {
                LOG_ERR "out of memory: %d\n", sizeof(struct dirent) LOG_END
                free(dir);
		        goto end;
		    }

            flistbox->fileCount++;

            if (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING)
                goto end;
        }
        flistbox->flistboxFlags |= ITU_FILELIST_CREATED;
    }
    else
    {
        LOG_WARN "opendir(%s) failed\n", flistbox->path LOG_END
	    goto end;
    }

end:
    // Close the directory
    if (pdir)
        closedir(pdir);

    flistbox->flistboxFlags &= ~ITU_FILELIST_BUSYING;
    return NULL;
}

void ituFileListBoxExit(ITUWidget* widget)
{
    ITUFileListBox* flistbox = (ITUFileListBox*) widget;
    assert(flistbox);
    ITU_ASSERT_THREAD();

    if (flistbox->flistboxFlags & ITU_FILELIST_BUSYING)
        flistbox->flistboxFlags |= ITU_FILELIST_DESTROYING;

    while (flistbox->flistboxFlags & ITU_FILELIST_BUSYING)
        usleep(1000);

    FileListDestroy(flistbox);
    ituWidgetExitImpl(widget);
}

bool ituFileListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUFileListBox* flistbox = (ITUFileListBox*) widget;
    assert(flistbox);

    result = ituListBoxUpdate(widget, ev, arg1, arg2, arg3);

    if ((ev == ITU_EVENT_TIMER) && (flistbox->flistboxFlags & ITU_FILELIST_CREATED))
    {
        struct dirent* val;
        ITCTree* node = widget->tree.child;
        int count;

        flistbox->flistboxFlags &= ~ITU_FILELIST_CREATED;

        if (node == NULL)
            return result;
        
        for (val = (struct dirent*) rblookup(RB_LUFIRST, NULL, flistbox->rbtree); 
             val != NULL; 
             val = (struct dirent*)rblookup(RB_LUNEXT, val, flistbox->rbtree))
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;

            if (val->d_type == DT_DIR)
            {
                char buf[NAME_MAX + 1];
                strcpy(buf, "<");
                strcat(buf, val->d_name);
                strcat(buf, ">");
                ituScrollTextSetString(scrolltext, buf);
            }
            else
                ituScrollTextSetString(scrolltext, val->d_name);

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

            if (flistbox->fileCount == 0)
            {
                listbox->pageIndex = listbox->pageCount = 0;
            }
            else
            {
                listbox->pageIndex = 1;

                if (flistbox->fileCount <= count)
                    listbox->pageCount = 1;
                else
                    listbox->pageCount = flistbox->fileCount / count + 1;
            }
            if (listbox->pageIndex == listbox->pageCount)
            {
                listbox->itemCount = flistbox->fileCount % count;
                if (listbox->itemCount == 0)
                    listbox->itemCount = count;
            }
            else
            {
                listbox->itemCount = count;
            }
            ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);

            sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
        }
        result = widget->dirty = true;
    }
    return widget->visible ? result : false;
}

static void FileListBoxBackPage(ITUFileListBox* flistbox)
{
    ITUListBox* listbox = (ITUListBox*) flistbox;
    int len;
    char* ptr;
    pthread_t task;
    pthread_attr_t attr;
    assert(flistbox);

    if ((flistbox->flistboxFlags & ITU_FILELIST_BUSYING) ||
        (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING))
        return;

    len = strlen(flistbox->path);
    if (len <= 3)
        return;

    if (flistbox->path[len - 1] == '/')
        flistbox->path[len - 1] = '\0';

    ptr = strrchr(flistbox->path, '/');
    if (ptr)
        *ptr = '\0';

    ituListBoxSelect(listbox, -1);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, FileListBoxCreateTask, flistbox);
    flistbox->flistboxFlags |= ITU_FILELIST_BUSYING;
}

void ituFileListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_BACK:
        FileListBoxBackPage((ITUFileListBox*)widget);
        break;

    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituFileListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUFileListBox* flistbox = (ITUFileListBox*) listbox;
    ITCTree* node;
    struct dirent* val;
    int i, count;
    assert(flistbox);
    assert(pageIndex <= listbox->pageCount);

    if ((flistbox->flistboxFlags & ITU_FILELIST_BUSYING) ||
        (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING) ||
        !flistbox->rbtree)
        return;

    count = itcTreeGetChildCount(listbox);

    if (pageIndex == listbox->pageCount)
    {
        listbox->itemCount = flistbox->fileCount % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
    {
        listbox->itemCount = count;
    }

    if (flistbox->flistboxFlags & ITU_FILELIST_BUSYING)
        return;

    node = ((ITCTree*)listbox)->child;
    i = 0;
    for (val = (struct dirent*)rblookup(RB_LUFIRST, NULL, flistbox->rbtree); 
         val != NULL; 
         val = (struct dirent*)rblookup(RB_LUNEXT, val, flistbox->rbtree))
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        if (i++ < count * (pageIndex - 1))
            continue;

        if (val->d_type == DT_DIR)
        {
            char buf[NAME_MAX + 1];
            strcpy(buf, "<");
            strcat(buf, val->d_name);
            strcat(buf, ">");
            ituScrollTextSetString(scrolltext, buf);
        }
        else
            ituScrollTextSetString(scrolltext, val->d_name);

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

void ituFileListOnSelection(ITUListBox* listbox, ITUScrollText* item, bool confirm)
{
    pthread_t task;
    pthread_attr_t attr;
    ITUFileListBox* flistbox = (ITUFileListBox*) listbox;
    assert(listbox);

    if (!confirm)
        return;

    if ((flistbox->flistboxFlags & ITU_FILELIST_BUSYING) ||
        (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING))
        return;

    if (item->text.string[0] != '<')
        return;

    if (flistbox->path[strlen(flistbox->path) - 1] != '/')
        strcat(flistbox->path, "/");

    strcat(flistbox->path, &item->text.string[1]);
    flistbox->path[strlen(flistbox->path) - 1] = '\0';

    ituListBoxSelect(listbox, -1);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, FileListBoxCreateTask, flistbox);
    flistbox->flistboxFlags |= ITU_FILELIST_BUSYING;
}

void ituFileListBoxInit(ITUFileListBox* flistbox, int width, char* path)
{
    assert(flistbox);
    ITU_ASSERT_THREAD();

    memset(flistbox, 0, sizeof (ITUFileListBox));

    ituListBoxInit(&flistbox->listbox, width);

    ituWidgetSetType(flistbox, ITU_FILELISTBOX);
    ituWidgetSetName(flistbox, flistboxName);
    ituWidgetSetExit(flistbox, ituFileListBoxExit);
    ituWidgetSetUpdate(flistbox, ituFileListBoxUpdate);
    ituWidgetSetOnAction(flistbox, ituFileListBoxOnAction);
    ituListBoxSetOnLoadPage(flistbox, ituFileListBoxOnLoadPage);
    ituListBoxSetOnSelection(flistbox, ituFileListOnSelection);

    if (path)
    {
        pthread_t task;
        pthread_attr_t attr;
        
        strncpy(flistbox->path, path, ITU_PATH_MAX - 1);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&task, &attr, FileListBoxCreateTask, flistbox);
        flistbox->flistboxFlags |= ITU_FILELIST_BUSYING;
    }
}

void ituFileListBoxLoad(ITUFileListBox* flistbox, uint32_t base)
{
    assert(flistbox);

    ituListBoxLoad(&flistbox->listbox, base);

    ituWidgetSetExit(flistbox, ituFileListBoxExit);
    ituWidgetSetUpdate(flistbox, ituFileListBoxUpdate);
    ituWidgetSetOnAction(flistbox, ituFileListBoxOnAction);
    ituListBoxSetOnLoadPage(flistbox, ituFileListBoxOnLoadPage);
    ituListBoxSetOnSelection(flistbox, ituFileListOnSelection);

    if (strlen(flistbox->path) > 0)
    {
        pthread_t task;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&task, &attr, FileListBoxCreateTask, flistbox);
        flistbox->flistboxFlags |= ITU_FILELIST_BUSYING;
    }
}

void ituFileListSetPath(ITUFileListBox* flistbox, char* path)
{
    ITUListBox* listbox = (ITUListBox*) flistbox;
    int len;
    pthread_t task;
    pthread_attr_t attr;
    assert(flistbox);
    ITU_ASSERT_THREAD();

    if ((flistbox->flistboxFlags & ITU_FILELIST_BUSYING) ||
        (flistbox->flistboxFlags & ITU_FILELIST_DESTROYING))
        return;

    if (path)
        strcpy(flistbox->path, path);
    else
        flistbox->path[0] = '\0';

    len = strlen(flistbox->path);
    if (len < 2)
        return;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, FileListBoxCreateTask, flistbox);
    flistbox->flistboxFlags |= ITU_FILELIST_BUSYING;
}

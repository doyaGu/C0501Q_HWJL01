#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "redblack/redblack.h"
#include "ite/itu.h"
#include "itu_cfg.h"

static const char smflistboxName[] = "ITUScrollMediaFileListBox";

typedef struct
{
    char* name;
    char* path;
} FileEntry;

bool ituScrollMediaFileListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) widget;
    ITUScrollMediaFileListBox* smflistbox = (ITUScrollMediaFileListBox*) widget;
    assert(smflistbox);

    if (ev == ITU_EVENT_LAYOUT)
    {
        int i, count = itcTreeGetChildCount(smflistbox);

        for (i = 0; i < count; ++i)
        {
            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
            int fy = 0 - child->rect.height * count / 3;
            fy += i * child->rect.height;
            ituWidgetSetY(child, fy);
        }

        if (!listbox->pageIndexText && (listbox->pageIndexName[0] != '\0'))
        {
            listbox->pageIndexText = (ITUText*) ituSceneFindWidget(ituScene, listbox->pageIndexName);
        }

        if (listbox->pageIndexText)
        {
            char buf[8];
            sprintf(buf, "%i", listbox->pageIndex);
            ituTextSetString(listbox->pageIndexText, buf);
        }

        if (!listbox->pageCountText && listbox->pageCountName[0] != '\0')
        {
            listbox->pageCountText = (ITUText*) ituSceneFindWidget(ituScene, listbox->pageCountName);
        }

        if (listbox->pageCountText)
        {
            char buf[8];
            sprintf(buf, "%i", listbox->pageCount);
            ituTextSetString(listbox->pageCountText, buf);
        }
        smflistbox->preindex = -1;
        result = true;
    }
    else if (ev == ITU_EVENT_MOUSEMOVE)
    {
        if (ituWidgetIsEnabled(widget) && (widget->flags & ITU_DRAGGING))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;
            int offset = y - smflistbox->touchY;
            int absoffset = offset > 0 ? offset : -offset;
                
            //printf("0: offset=%d\n", offset);

            if ((absoffset <= widget->rect.height / 2) ||
                ((listbox->pageIndex > 1 && offset > 0) || 
                (listbox->pageIndex < listbox->pageCount && offset < 0)))
            {
                int i, count = itcTreeGetChildCount(widget);

                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(widget, i);
                    int fy = 0 - child->rect.height * count / 3;
                    fy += i * child->rect.height;
                    fy += offset;
                    ituWidgetSetY(child, fy);
                }
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node = ituScrollMediaFileListBoxGetCurrPageItem(smflistbox);
                int count, i = 0;

                smflistbox->preindex = -1;
                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1))
                    {
                        smflistbox->preindex = count + i;
                        break;
                    }
                    i++;
                }
                if (widget->flags & ITU_DRAGGABLE)
                {
                    smflistbox->touchY = y;

                    if (widget->flags & ITU_HAS_LONG_PRESS)
                    {
                        smflistbox->touchCount = 1;
                    }
                    else
                    {
                        widget->flags |= ITU_DRAGGING;
                        ituScene->dragged = widget;
                        ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                    }
                }
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP || ev == ITU_EVENT_MOUSEDOUBLECLICK)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int dist;
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if ((widget->flags & ITU_DRAGGABLE) && (widget->flags & ITU_DRAGGING))
            {
                int offset = y - smflistbox->touchY;
                int absoffset = offset > 0 ? offset : -offset;

                if ((absoffset > widget->rect.height / 2) &&
                    (listbox->pageIndex > 1 && offset > 0) || 
                    (listbox->pageIndex < listbox->pageCount && offset < 0))
                {
                    smflistbox->frame = absoffset / (widget->rect.height / smflistbox->totalframe) + 1;

                    if (offset == 0)
                    {
                        smflistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        smflistbox->inc = widget->rect.height / smflistbox->totalframe;
                        //printf("1: frame=%d offset=%d inc=%d pageIndex=%d\n", smflistbox->frame, offset, smflistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        smflistbox->inc = -widget->rect.height / smflistbox->totalframe;
                        //printf("2: frame=%d offset=%d inc=%d pageIndex=%d\n", smflistbox->frame, offset, smflistbox->inc, listbox->pageIndex);
                    }
                    widget->flags |= ITU_PROGRESS;
                }
                else
                {
                    smflistbox->frame = smflistbox->totalframe - absoffset / (widget->rect.height / smflistbox->totalframe);

                    if (offset == 0)
                    {
                        smflistbox->inc = 0;
                    }
                    else if (offset > 0)
                    {
                        smflistbox->inc = -widget->rect.height / smflistbox->totalframe;
                        //printf("3: frame=%d offset=%d inc=%d pageIndex=%d\n", smflistbox->frame, offset, smflistbox->inc, listbox->pageIndex);
                    }
                    else
                    {
                        smflistbox->inc = widget->rect.height / smflistbox->totalframe;
                        //printf("4: frame=%d offset=%d inc=%d pageIndex=%d\n", smflistbox->frame, offset, smflistbox->inc, listbox->pageIndex);
                    }
                }
                widget->flags |= ITU_UNDRAGGING;
                widget->flags &= ~ITU_DRAGGING;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
            
            dist = y - smflistbox->touchY;
            if (dist < 0)
                dist = -dist;

            if (ituWidgetIsInside(widget, x, y) && ((dist < ITU_DRAG_DISTANCE) || !(widget->flags & ITU_DRAGGABLE)))
            {
                ITCTree* node = ituScrollMediaFileListBoxGetCurrPageItem(smflistbox);
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && smflistbox->preindex == count + i)
                    {
                        if (ev == ITU_EVENT_MOUSEUP)
                        {
                            if (ituScene->focused != widget)
                            {
                                ituFocusWidget(widget);
                            }
                            if (!(widget->flags & ITU_LONG_PRESSING))
                            {
                                ituListBoxSelect(listbox, i);
                                ituExecActions(widget, listbox->actions, ITU_EVENT_SELECT, i);
                            }
                        }
                        else
                        {
                            ituListBoxOnSelection(listbox, (ITUScrollText*)item, true);
                        }
                        break;
                    }
                    i++;
                }
                smflistbox->preindex = -1;
                result = true;
            }
            smflistbox->touchCount = 0;
            widget->flags &= ~ITU_LONG_PRESSING;
        }
    }
    else if (ev == ITU_EVENT_MOUSELONGPRESS)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node = ituScrollMediaFileListBoxGetCurrPageItem(smflistbox);
                int count, i = 0;

                count = listbox->itemCount;

                for (; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1) && smflistbox->preindex == count + i)
                    {
                        ituListBoxCheck(listbox, i);
                        ituExecActions(widget, listbox->actions, ev, i);
                        result |= widget->dirty;
                        break;
                    }
                    i++;
                }
                widget->flags |= ITU_LONG_PRESSING;
            }
        }
        smflistbox->touchCount = 0;
    }
    else if (ev == ITU_EVENT_TOUCHSLIDEUP || ev == ITU_EVENT_TOUCHSLIDEDOWN)
    {
        smflistbox->touchCount = 0;

        if (ituWidgetIsEnabled(widget) && !smflistbox->inc)
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                if (ev == ITU_EVENT_TOUCHSLIDEUP)
                {
                    int i, count = itcTreeGetChildCount(smflistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        smflistbox->inc = 0;
                    }

                    if (listbox->pageIndex < listbox->pageCount)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (smflistbox->inc == 0)
                                smflistbox->inc = 0 - widget->rect.height;

                            fy += smflistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (smflistbox->inc == 0)
                                smflistbox->inc = 0 - widget->rect.height / 5;

                            fy += smflistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    smflistbox->frame = 1;
                }
                else // if (ev == ITU_EVENT_TOUCHSLIDEDOWN)
                {
                    int i, count = itcTreeGetChildCount(smflistbox);

                    if (widget->flags & ITU_DRAGGING)
                    {
                        widget->flags &= ~ITU_DRAGGING;
                        ituScene->dragged = NULL;
                        smflistbox->inc = 0;
                    }

                    if (listbox->pageIndex > 1)
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (smflistbox->inc == 0)
                                smflistbox->inc = widget->rect.height;

                            fy += smflistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                    }
                    else
                    {
                        for (i = 0; i < count; ++i)
                        {
                            ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                            int fy = 0 - child->rect.height * count / 3;
                            fy += i * child->rect.height;

                            if (smflistbox->inc == 0)
                                smflistbox->inc = widget->rect.height / 5;

                            fy += smflistbox->inc;
                            ituWidgetSetY(child, fy);
                        }
                        widget->flags |= ITU_BOUNCING;
                    }
                    smflistbox->frame = 1;
                }
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_TIMER)
    {
        if (smflistbox->touchCount > 0)
        {
            int y, dist;

            assert(widget->flags & ITU_HAS_LONG_PRESS);

            ituWidgetGetGlobalPosition(widget, NULL, &y);

            dist = ituScene->lastMouseY - (y + smflistbox->touchY);

            if (dist < 0)
                dist = -dist;

            if (dist >= ITU_DRAG_DISTANCE)
            {
                widget->flags |= ITU_DRAGGING;
                ituScene->dragged = widget;
                smflistbox->touchCount = 0;
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SYNC, (int)widget->name);
            }
        }

        if (widget->flags & ITU_UNDRAGGING)
        {
            int i, count = itcTreeGetChildCount(smflistbox);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                
                if ((widget->flags & ITU_PROGRESS) == 0)
                {
                    if (smflistbox->inc > 0)
                        fy -= widget->rect.height;
                    else if (smflistbox->inc < 0)
                        fy += widget->rect.height;
                }

                fy += i * child->rect.height;
                fy += smflistbox->inc * smflistbox->frame;

                ituWidgetSetY(child, fy);
            }
            smflistbox->frame++;

            if (smflistbox->frame > smflistbox->totalframe)
            {
                for (i = 0; i < count; ++i)
                {
                    ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                    int fy = 0 - child->rect.height * count / 3;
                    fy += i * child->rect.height;

                    ituWidgetSetY(child, fy);
                }

                if (widget->flags & ITU_PROGRESS)
                {
                    char buf[32];

                    if (smflistbox->inc > 0)
                    {
                        listbox->pageIndex--;
                    }
                    else if (smflistbox->inc < 0)
                    {
                        listbox->pageIndex++;
                    }
                    ituListBoxOnLoadPage(listbox, listbox->pageIndex);
                    ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
                    ituListBoxSelect(listbox, -1);

                    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
                }

                smflistbox->frame = 0;
                smflistbox->inc = 0;
                widget->flags &= ~ITU_UNDRAGGING;
                widget->flags &= ~ITU_PROGRESS;
            }
            result = true;
        }
        else if (widget->flags & ITU_BOUNCING)
        {
            float step;
            int i, count = itcTreeGetChildCount(smflistbox);

            if (smflistbox->frame <= smflistbox->totalframe / 2)
                step = (float)smflistbox->frame;
            else
                step = (float)smflistbox->totalframe - smflistbox->frame;

            step /= smflistbox->totalframe / 2;
            step = (float)M_PI / 2 - step * (float)M_PI / 2;
            step = 1.0f - sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(smflistbox->inc * step);

                ituWidgetSetY(child, fy);
            }

            smflistbox->frame++;

            if (smflistbox->frame > smflistbox->totalframe)
            {
                smflistbox->frame = 0;
                smflistbox->inc = 0;
                widget->flags &= ~ITU_BOUNCING;
            }
            result = true;
        }
        else if (smflistbox->inc)
        {
            int i, count = itcTreeGetChildCount(smflistbox);
            float step = (float)smflistbox->frame / smflistbox->totalframe;
            step = step * (float)M_PI / 2;
            step = sinf(step);

            //printf("step=%f\n", step);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = (ITUWidget*)itcTreeGetChildAt(smflistbox, i);
                int fy = 0 - child->rect.height * count / 3;
                fy += i * child->rect.height;
                fy += (int)(smflistbox->inc * step);

                ituWidgetSetY(child, fy);
            }
            smflistbox->frame++;

            if (smflistbox->frame > smflistbox->totalframe)
            {
                char buf[32];

                if (smflistbox->inc > 0)
                {
                    listbox->pageIndex--;
                }
                else // if (smflistbox->inc < 0)
                {
                    listbox->pageIndex++;
                }
                smflistbox->frame = 0;
                smflistbox->inc = 0;

                ituListBoxOnLoadPage(listbox, listbox->pageIndex);
                ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
                ituListBoxSelect(listbox, -1);

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
            }
            result = true;
        }

        if (mflistbox->mflistboxFlags & ITU_FILELIST_CREATED)
        {
            int count = ituScrollMediaFileListBoxGetItemCount(smflistbox);
            ITCTree* node = itcTreeGetChildAt(smflistbox, count);

            mflistbox->mflistboxFlags &= ~ITU_FILELIST_CREATED;

            if (node == NULL)
                return result;
            
            count = ituScrollMediaFileListBoxGetItemCount(smflistbox);
            if (count > 0)
            {
                char buf[32];

                listbox->pageIndex = 1;
                listbox->pageCount = mflistbox->fileCount ? (mflistbox->fileCount + count - 1) / count : 1;

                ituListBoxOnLoadPage(listbox, listbox->pageIndex);

                ituWidgetUpdate(widget, ITU_EVENT_LAYOUT, 0, 0, 0);
                ituListBoxSelect(listbox, -1);

                sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
                ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
            }
            result = widget->dirty = true;
        }
    }
    
    if (!result)
    {
        result = ituListBoxUpdate(widget, ev, arg1, arg2, arg3);
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

void ituScrollMediaFileListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITUMediaFileListBox* mflistbox = (ITUMediaFileListBox*) listbox;
    ITUScrollMediaFileListBox* smflistbox = (ITUScrollMediaFileListBox*) listbox;
    ITCTree* node;
    FileEntry* val;
    int i, count;
    assert(mflistbox);
    assert(pageIndex <= listbox->pageCount);

    if ((mflistbox->mflistboxFlags & ITU_FILELIST_BUSYING) ||
        (mflistbox->mflistboxFlags & ITU_FILELIST_DESTROYING) ||
        !mflistbox->rbtree)
        return;

    count = ituScrollMediaFileListBoxGetItemCount(smflistbox);
    node = ituScrollMediaFileListBoxGetLastPageItem(smflistbox);
    i = 0;

    for (val = (FileEntry*)rblookup(RB_LUFIRST, NULL, mflistbox->rbtree);
         val != NULL; 
         val = (FileEntry*)rblookup(RB_LUNEXT, val, mflistbox->rbtree))
    {
        if (i >= count * (pageIndex - 2))
            break;

        i++;
    }

    if (pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;
            ituScrollTextSetString(scrolltext, "");
            node = node->sibling;
        }
    }

    i = 0;
    for (;
         val != NULL; 
         val = (FileEntry*)rblookup(RB_LUNEXT, val, mflistbox->rbtree))
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        ituScrollTextSetString(scrolltext, val->name);
        ituWidgetSetCustomData(scrolltext, val->path);

        i++;

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

    if (listbox->pageIndex == listbox->pageCount)
    {
        if (i == 0)
        {
            listbox->itemCount = i;
        }
        else
        {
            listbox->itemCount = i % count;

            if (listbox->itemCount == 0)
                listbox->itemCount = count;
        }
    }
    else
        listbox->itemCount = count;
}

static void ScrollMediaFileListBoxBind(ITUScrollMediaFileListBox* smflistbox, char* param)
{
    ITUWidget* widget = (ITUWidget*) smflistbox;
    ITUWidget* target = ituSceneFindWidget(ituScene, param);
    if (target)
    {
        if (target->flags & ITU_PROGRESS)
            widget->flags |= ITU_PROGRESS;
        else
            widget->flags &= ~ITU_PROGRESS;

        if (target->flags & ITU_DRAGGING)
            widget->flags |= ITU_DRAGGING;
        else
            widget->flags &= ~ITU_DRAGGING;

        if (target->flags & ITU_UNDRAGGING)
            widget->flags |= ITU_UNDRAGGING;
        else
            widget->flags &= ~ITU_UNDRAGGING;

        if (target->flags & ITU_BOUNCING)
            widget->flags |= ITU_BOUNCING;
        else
            widget->flags &= ~ITU_BOUNCING;

        if (target->type == ITU_SCROLLLISTBOX)
        {
            ITUScrollListBox* targetslistbox = (ITUScrollListBox*) target;
            smflistbox->inc = targetslistbox->inc;
            smflistbox->frame = targetslistbox->frame;
            smflistbox->touchY = targetslistbox->touchY;
        }
        else if (target->type == ITU_SCROLLMEDIAFILELISTBOX)
        {
            ITUScrollMediaFileListBox* targetsmflistbox = (ITUScrollMediaFileListBox*) target;
            smflistbox->inc = targetsmflistbox->inc;
            smflistbox->frame = targetsmflistbox->frame;
            smflistbox->touchY = targetsmflistbox->touchY;
        }
        else if (target->type == ITU_SCROLLICONLISTBOX)
        {
            ITUScrollIconListBox* targetsilistbox = (ITUScrollIconListBox*) target;
            smflistbox->inc = targetsilistbox->inc;
            smflistbox->frame = targetsilistbox->frame;
            smflistbox->touchY = targetsilistbox->touchY;
        }
    }
}

void ituScrollMediaFileListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_BIND:
        ScrollMediaFileListBoxBind((ITUScrollMediaFileListBox*)widget, param);
        break;

    default:
        ituListBoxOnAction(widget, action, param);
        break;
    }
}

void ituScrollMediaFileListBoxInit(ITUScrollMediaFileListBox* smflistbox, int width, char* path)
{
    assert(smflistbox);
    ITU_ASSERT_THREAD();

    memset(smflistbox, 0, sizeof (ITUScrollMediaFileListBox));

    ituMediaFileListBoxInit(&smflistbox->mflistbox, width, path);

    ituWidgetSetType(smflistbox, ITU_SCROLLMEDIAFILELISTBOX);
    ituWidgetSetName(smflistbox, smflistboxName);
    ituWidgetSetUpdate(smflistbox, ituScrollMediaFileListBoxUpdate);
    ituWidgetSetOnAction(smflistbox, ituScrollMediaFileListBoxOnAction);
    ituListBoxSetOnLoadPage(smflistbox, ituScrollMediaFileListBoxOnLoadPage);
}

void ituScrollMediaFileListBoxLoad(ITUScrollMediaFileListBox* smflistbox, uint32_t base)
{
    assert(smflistbox);

    ituMediaFileListBoxLoad(&smflistbox->mflistbox, base);

    ituWidgetSetUpdate(smflistbox, ituScrollMediaFileListBoxUpdate);
    ituWidgetSetOnAction(smflistbox, ituScrollMediaFileListBoxOnAction);
    ituListBoxSetOnLoadPage(smflistbox, ituScrollMediaFileListBoxOnLoadPage);
}

int ituScrollMediaFileListBoxGetItemCount(ITUScrollMediaFileListBox* smflistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildCount(smflistbox) / 3;
}

ITCTree* ituScrollMediaFileListBoxGetLastPageItem(ITUScrollMediaFileListBox* smflistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(smflistbox, 0);
}

ITCTree* ituScrollMediaFileListBoxGetCurrPageItem(ITUScrollMediaFileListBox* smflistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(smflistbox, itcTreeGetChildCount(smflistbox) / 3);
}

ITCTree* ituScrollMediaFileListBoxGetNextPageItem(ITUScrollMediaFileListBox* smflistbox)
{
    ITU_ASSERT_THREAD();
    return itcTreeGetChildAt(smflistbox, itcTreeGetChildCount(smflistbox) * 2 / 3);
}

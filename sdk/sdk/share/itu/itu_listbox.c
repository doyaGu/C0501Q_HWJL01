#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char listboxName[] = "ITUListBox";

bool ituListBoxUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUListBox* listbox = (ITUListBox*) widget;
    assert(listbox);

    result = ituFlowWindowUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_LAYOUT)
    {
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
    }
    else if (ev == ITU_EVENT_MOUSEDOWN || ev == ITU_EVENT_MOUSEDOUBLECLICK)
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                ITCTree* node;
                int count, i = 0;

                count = listbox->itemCount;

                for (node = widget->tree.child; node; node = node->sibling)
                {
                    ITUWidget* item = (ITUWidget*) node;
                    int x1, y1;

                    if (i >= count)
                        break;

                    x1 = x - item->rect.x;
                    y1 = y - item->rect.y;

                    if (ituWidgetIsInside(item, x1, y1))
                    {
                        if (ev == ITU_EVENT_MOUSEDOWN)
                        {
                            if (ituScene->focused != widget)
                            {
                                ituFocusWidget(widget);
                            }
                            ituListBoxSelect(listbox, i);
                            ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SELECT, i);
                        }
                        else
                        {
                            ituListBoxOnSelection(listbox, (ITUScrollText*)item, true);
                        }
                        break;
                    }
                    i++;
                }
            }
        }
    }
    else if (ev >= ITU_EVENT_CUSTOM || ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget))
        {
            ituExecActions(widget, listbox->actions, ev, arg1);
        }
    }
    else if (ituWidgetIsActive(widget) && ituWidgetIsEnabled(widget))
    {
        int count;

        switch (ev)
        {
        case ITU_EVENT_KEYDOWN:
            count = itcTreeGetChildCount(listbox);
            if (count == 0)
                break;

            if (arg1 == ituScene->upKey)
            {
                ituListBoxPrev(listbox);
            }
            else if (arg1 == ituScene->downKey)
            {
                ituListBoxNext(listbox);
            }
            else if (arg1 == ituScene->enterKey)
            {
                ITUScrollText* item;

                if (listbox->focusIndex < 0)
                    break;

                item = (ITUScrollText*) itcTreeGetChildAt(listbox, listbox->focusIndex);
                if (item)
                {
                    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_SELECT, 0);
                    ituListBoxOnSelection(listbox, (ITUScrollText*)item, true);
                }
            }
            break;
        }
    }
    result |= widget->dirty;
    return widget->visible ? result : false;
}

void ituListBoxDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    uint8_t desta;
    ITURectangle prevClip;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    assert(widget);
    assert(dest);

    destx = rect->x + x;
    desty = rect->y + y;
    desta = alpha * widget->color.alpha / 255;
    desta = desta * widget->alpha / 255;
   
    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    if (desta == 255)
    {
        ituColorFill(dest, destx, desty, rect->width, rect->height, &widget->color);
    }
    else if (desta > 0)
    {
        ITUSurface* surf = ituCreateSurface(rect->width, rect->height, 0, dest->format, NULL, 0);
        if (surf)
        {
            ituColorFill(surf, 0, 0, rect->width, rect->height, &widget->color);
            ituAlphaBlend(dest, destx, desty, rect->width, rect->height, surf, 0, 0, desta);                
            ituDestroySurface(surf);
        }
    }
    ituFlowWindowDraw(widget, dest, x, y, alpha);
    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

static void ListBoxPrevPage(ITUListBox* listbox)
{
    char buf[32];

    if (itcTreeGetChildCount(listbox) == 0)
        return;

    if (listbox->pageIndex == 0 && listbox->pageCount == 0)
        return;

    if (listbox->pageIndex == 1)
        return;

    listbox->pageIndex = listbox->pageIndex - 1;
    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
    ituListBoxOnLoadPage(listbox, listbox->pageIndex);
    ituListBoxSelect(listbox, -1);
    ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
}

static void ListBoxNextPage(ITUListBox* listbox)
{
    char buf[32];

    if (itcTreeGetChildCount(listbox) == 0)
        return;

    if (listbox->pageIndex == 0 && listbox->pageCount == 0)
        return;

    if (listbox->pageIndex == listbox->pageCount)
        return;

    listbox->pageIndex = listbox->pageIndex + 1;
    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
    ituListBoxOnLoadPage(listbox, listbox->pageIndex);
    ituListBoxSelect(listbox, -1);
    ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituListBoxOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    assert(widget);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ListBoxPrevPage((ITUListBox*)widget);
        break;

    case ITU_ACTION_NEXT:
        ListBoxNextPage((ITUListBox*)widget);
        break;

    case ITU_ACTION_CHECK:
        if (param[0] != '\0')
            ituListBoxCheck((ITUListBox*)widget, atoi(param));
        break;

    case ITU_ACTION_UNCHECK:
        ituListBoxCheck((ITUListBox*)widget, -1);
        break;

    case ITU_ACTION_RELOAD:
        ituListBoxReload((ITUListBox*)widget);
        break;

    case ITU_ACTION_GOTO:
        if (param[0] != '\0')
            ituListBoxGoto((ITUListBox*)widget, atoi(param));
        break;

    case ITU_ACTION_DODELAY0:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY0, atoi(param));
        break;

    case ITU_ACTION_DODELAY1:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY1, atoi(param));
        break;

    case ITU_ACTION_DODELAY2:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY2, atoi(param));
        break;

    case ITU_ACTION_DODELAY3:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY3, atoi(param));
        break;

    case ITU_ACTION_DODELAY4:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY4, atoi(param));
        break;

    case ITU_ACTION_DODELAY5:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY5, atoi(param));
        break;

    case ITU_ACTION_DODELAY6:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY6, atoi(param));
        break;

    case ITU_ACTION_DODELAY7:
        ituExecActions(widget, ((ITUListBox*)widget)->actions, ITU_EVENT_DELAY7, atoi(param));
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

static void ListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    // DO NOTHING
}

static void ListBoxOnSelection(ITUListBox* listbox, ITUScrollText* item, bool confirm)
{
    // DO NOTHING
}

void ituListBoxInit(ITUListBox* listbox, int width)
{
    assert(listbox);
    ITU_ASSERT_THREAD();

    memset(listbox, 0, sizeof (ITUListBox));

    ituFlowWindowInit(&listbox->fwin, ITU_LAYOUT_UP);

    ituWidgetSetType(listbox, ITU_LISTBOX);
    ituWidgetSetName(listbox, listboxName);
    ituWidgetSetUpdate(listbox, ituListBoxUpdate);
    ituWidgetSetDraw(listbox, ituListBoxDraw);
    ituWidgetSetOnAction(listbox, ituListBoxOnAction);
    ituListBoxSetOnLoadPage(listbox, ListBoxOnLoadPage);
    ituListBoxSetOnSelection(listbox, ListBoxOnSelection);

    listbox->scrollDelay    = ITU_SCROLL_DELAY;
    listbox->stopDelay      = ITU_STOP_DELAY;
}

void ituListBoxLoad(ITUListBox* listbox, uint32_t base)
{
    assert(listbox);

    ituFlowWindowLoad(&listbox->fwin, base);

	if (listbox->pageIndexText){
		listbox->pageIndexText = (ITUText*)((uint32_t)listbox->pageIndexText + base);
	}

    if (listbox->pageCountText)
        listbox->pageCountText = (ITUText*)((uint32_t)listbox->pageCountText + base);

    ituWidgetSetUpdate(listbox, ituListBoxUpdate);
    ituWidgetSetDraw(listbox, ituListBoxDraw);
    ituWidgetSetOnAction(listbox, ituListBoxOnAction);
    ituListBoxSetOnLoadPage(listbox, ListBoxOnLoadPage);
    ituListBoxSetOnSelection(listbox, ListBoxOnSelection);
}

void ituListBoxSelect(ITUListBox* listbox, int index)
{
    ITUWidget* widget = (ITUWidget*) listbox;
    ITUText* item;
    int count;
    assert(listbox);
    ITU_ASSERT_THREAD();

    if (widget->type == ITU_SCROLLLISTBOX || widget->type == ITU_SCROLLMEDIAFILELISTBOX || widget->type == ITU_SCROLLICONLISTBOX)
        count = itcTreeGetChildCount(listbox) / 3;
    else
        count = 0;

    if (listbox->focusIndex >= 0)
    {
        item = (ITUText*) itcTreeGetChildAt(listbox, count + listbox->focusIndex);
        if (item)
        {
            ITUColor* c = &item->widget.color;

            ituTextSetBackColor(item, 0, 0, 0, 0);

            if (ituScrollTextIsRead(item))
                ituWidgetSetColor(item, listbox->readFontColor.alpha, listbox->readFontColor.red, listbox->readFontColor.green, listbox->readFontColor.blue);
            else
                ituWidgetSetColor(item, listbox->orgFontColor.alpha, listbox->orgFontColor.red, listbox->orgFontColor.green, listbox->orgFontColor.blue);

            ituScrollTextStop((ITUScrollText*)item);
        }
    }

    if (index >= 0)
    {
        item = (ITUText*) itcTreeGetChildAt(listbox, count + index);
        if (item)
        {
            int oldIndex = listbox->focusIndex;
            ITUColor* c = &item->widget.color;

            ituTextSetBackColor(item, listbox->focusColor.alpha, listbox->focusColor.red, listbox->focusColor.green, listbox->focusColor.blue);
            if (listbox->focusFontColor.alpha)
                ituWidgetSetColor(item, listbox->focusFontColor.alpha, listbox->focusFontColor.red, listbox->focusFontColor.green, listbox->focusFontColor.blue);

            ituScrollTextSetDelay((ITUScrollText*)item, listbox->scrollDelay, listbox->stopDelay);
            ituScrollTextStart((ITUScrollText*)item);
            listbox->focusIndex = index;
        }
    }
    else
        listbox->focusIndex = -1;

    ituWidgetSetDirty(listbox, true);
}

void ituListBoxCheck(ITUListBox* listbox, int index)
{
    ITUWidget* widget = (ITUWidget*) listbox;
    ITUText* item;
    int count;
    assert(listbox);
    ITU_ASSERT_THREAD();

    if (widget->type == ITU_SCROLLLISTBOX || widget->type == ITU_SCROLLMEDIAFILELISTBOX || widget->type == ITU_SCROLLICONLISTBOX)
        count = itcTreeGetChildCount(listbox) / 3;
    else
        count = 0;

    if (listbox->focusIndex >= 0)
    {
        item = (ITUText*) itcTreeGetChildAt(listbox, count + listbox->focusIndex);
        if (item)
        {
            ITUColor* c = &item->widget.color;

            ituTextSetBackColor(item, 0, 0, 0, 0);

            if (ituScrollTextIsRead(item))
                ituWidgetSetColor(item, listbox->readFontColor.alpha, listbox->readFontColor.red, listbox->readFontColor.green, listbox->readFontColor.blue);
            else
                ituWidgetSetColor(item, listbox->orgFontColor.alpha, listbox->orgFontColor.red, listbox->orgFontColor.green, listbox->orgFontColor.blue);

            ituScrollTextStop((ITUScrollText*)item);
        }
    }

    if (index >= 0)
    {
        item = (ITUText*) itcTreeGetChildAt(listbox, count + index);
        if (item)
        {
            int oldIndex = listbox->focusIndex;
            ITUColor* c = &item->widget.color;

            ituTextSetBackColor(item, listbox->focusColor.alpha, listbox->focusColor.red, listbox->focusColor.green, listbox->focusColor.blue);
            if (listbox->focusFontColor.alpha)
                ituWidgetSetColor(item, listbox->focusFontColor.alpha, listbox->focusFontColor.red, listbox->focusFontColor.green, listbox->focusFontColor.blue);

            ituScrollTextSetDelay((ITUScrollText*)item, listbox->scrollDelay, listbox->stopDelay);
            ituScrollTextStart((ITUScrollText*)item);
            listbox->focusIndex = index;
        }
    }
    else
        listbox->focusIndex = -1;

    ituWidgetSetDirty(listbox, true);
}

void ituListBoxReload(ITUListBox* listbox)
{
    ITU_ASSERT_THREAD();

    if (itcTreeGetChildCount(listbox) == 0)
        return;

    listbox->pageIndex = 0;
    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, 0);
    ituListBoxSelect(listbox, -1);
    ituListBoxOnLoadPage(listbox, listbox->pageIndex);
    ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
}

void ituListBoxGoto(ITUListBox* listbox, int index)
{
    char buf[32];

    ITU_ASSERT_THREAD();

    if (listbox->itemCount == 0 || index > listbox->pageCount)
        return;

    listbox->pageIndex = index;
    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, 0);
    sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
    ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
    ituListBoxOnLoadPage(listbox, listbox->pageIndex);
    ituListBoxSelect(listbox, -1);
}

void ituListBoxPrevImpl(ITUListBox* listbox)
{
    int i;

    if (listbox->itemCount == 0)
        return;

    if (listbox->focusIndex > 0)
    {
        i = listbox->focusIndex;
    }
    else
    {
        char buf[32];

        listbox->pageIndex = (listbox->pageIndex == 1) ? listbox->pageCount : (listbox->pageIndex - 1);
        sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
        ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
        ituListBoxOnLoadPage(listbox, listbox->pageIndex);
        i = listbox->itemCount;
    }
    ituListBoxSelect(listbox, i - 1);
}

void ituListBoxNextImpl(ITUListBox* listbox)
{
    int i;

    if (listbox->itemCount == 0)
        return;

    if (listbox->focusIndex < listbox->itemCount - 1)
    {
        i = listbox->focusIndex;
    }
    else
    {
        char buf[32];

        listbox->pageIndex = (listbox->pageIndex == listbox->pageCount) ? 1 : (listbox->pageIndex + 1);
        sprintf(buf, "%d %d", listbox->pageIndex, listbox->pageCount);
        ituExecActions((ITUWidget*)listbox, listbox->actions, ITU_EVENT_LOAD, (int)buf);
        ituListBoxOnLoadPage(listbox, listbox->pageIndex);
        i = -1;
    }
    ituListBoxSelect(listbox, i + 1);
}

ITUWidget* ituListBoxGetFocusItem(ITUListBox* listbox)
{
    ITUWidget* widget = (ITUWidget*) listbox;

    ITU_ASSERT_THREAD();

    if (listbox->focusIndex >= 0)
    {
        ITUWidget* item;
        int count;

        if (widget->type == ITU_SCROLLLISTBOX || widget->type == ITU_SCROLLMEDIAFILELISTBOX || widget->type == ITU_SCROLLICONLISTBOX)
            count = itcTreeGetChildCount(listbox) / 3;
        else
            count = 0;

        item = (ITUWidget*)itcTreeGetChildAt(listbox, count + listbox->focusIndex);
        return item;
    }
    return NULL;
}

void ituListBoxSetItemRead(ITUListBox* listbox, ITUWidget* item, bool read)
{
    ITUWidget* widget = (ITUWidget*) listbox;
    assert(listbox);
    ITU_ASSERT_THREAD();

    if (item)
    {
        ITUWidget* focusItem = ituListBoxGetFocusItem(listbox);

        if (read)
        {
            if (item == focusItem && listbox->focusFontColor.alpha)
                ituWidgetSetColor(item, listbox->focusFontColor.alpha, listbox->focusFontColor.red, listbox->focusFontColor.green, listbox->focusFontColor.blue);
            else            
                ituWidgetSetColor(item, listbox->readFontColor.alpha, listbox->readFontColor.red, listbox->readFontColor.green, listbox->readFontColor.blue);

            ituScrollTextSetAsRead(item);
        }
        else
        {
            if (item == focusItem && listbox->focusFontColor.alpha)
                ituWidgetSetColor(item, listbox->focusFontColor.alpha, listbox->focusFontColor.red, listbox->focusFontColor.green, listbox->focusFontColor.blue);
            else
                ituWidgetSetColor(item, listbox->orgFontColor.alpha, listbox->orgFontColor.red, listbox->orgFontColor.green, listbox->orgFontColor.blue);

            ituScrollTextSetAsUnread(item);
        }
        ituWidgetSetDirty(listbox, true);
    }
}

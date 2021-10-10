#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUScrollIconListBox* checkListScrollIconListBox;
static ITUScrollListBox* checkListScrollListBox;
static ITUScrollBar* checkListScrollBar;

static const char* checkListNameArray[] =
{
    "iPhone 6",
    "Samsung Galaxy S6",
    "HTC One M9",
    "Galaxy Note 4",
    "iPhone 6 Plus",
    "LG G4",
    "Nexus 6",
    "Motorola Droid Turbo",
    "Samsung Galaxy Alpha",
    "iPhone 5s",
    "Samsung Galaxy S6 Edge",
    "Honor 7",
    "Google Nexus 5X",
    "Motorola Moto X Play",
    "Samsung Galaxy Note5"
};

#define MAX_CHECKLIST_COUNT (sizeof(checkListNameArray) / sizeof(checkListNameArray[0]))

typedef struct
{
    bool selected;
    const char* name;
} CheckListEntry;

static int checkListCount;
static CheckListEntry checkListArray[MAX_CHECKLIST_COUNT];

static void CheckListInit(void)
{
    int i;

    for (i = 0; i < MAX_CHECKLIST_COUNT; i++)
    {
        checkListArray[i].selected = false;
        checkListArray[i].name = checkListNameArray[i];
    }
    checkListCount = MAX_CHECKLIST_COUNT;
}

bool CheckListScrollIconListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUScrollIconListBox* silistbox = (ITUScrollIconListBox*) listbox;
    ITCTree* node;
    CheckListEntry* entry;
    int i, j, count;
    assert(listbox);

    count = ituScrollIconListBoxGetItemCount(silistbox);
    node = ituScrollIconListBoxGetLastPageItem(silistbox);

    listbox->pageCount = checkListCount ? (checkListCount + count - 1) / count : 1;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;
    }

    if (listbox->pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;
            ituScrollTextSetString(scrolltext, "");

            node = node->sibling;
        }
    }

    i = 0;
    j = count * (listbox->pageIndex - 2);
    if (j < 0)
        j = 0;

    for (; j < checkListCount; j++)
    {
        char buf[4];
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        entry = &checkListArray[j];
        sprintf(buf, "%d", entry->selected ? 1 : 0);

        ituScrollTextSetString(scrolltext, buf);
        ituWidgetSetCustomData(scrolltext, entry);

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

    if (listbox->pageIndex == listbox->pageCount)
    {
        listbox->itemCount = i % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
        listbox->itemCount = count;

    return true;
}

bool CheckListScrollListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUScrollListBox* slistbox = (ITUScrollListBox*) listbox;
    ITCTree* node;
    CheckListEntry* entry;
    int i, j, count;
    assert(listbox);

    count = ituScrollListBoxGetItemCount(slistbox);
    node = ituScrollListBoxGetLastPageItem(slistbox);

    listbox->pageCount = checkListCount ? (checkListCount + count - 1) / count : 1;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;

        ituScrollBarSetLength(checkListScrollBar, listbox->pageCount);
        ituScrollBarSetPosition(checkListScrollBar, listbox->pageIndex);
    }

    if (listbox->pageIndex <= 1)
    {
        for (i = 0; i < count; i++)
        {
            ITUScrollText* scrolltext = (ITUScrollText*) node;
            ituScrollTextSetString(scrolltext, "");

            node = node->sibling;
        }
    }

    i = 0;
    j = count * (listbox->pageIndex - 2);
    if (j < 0)
        j = 0;

    for (; j < checkListCount; j++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        entry = &checkListArray[j];
        ituScrollTextSetString(scrolltext, (char*)entry->name);

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

    if (listbox->pageIndex == listbox->pageCount)
    {
        listbox->itemCount = i % count;
        if (listbox->itemCount == 0)
            listbox->itemCount = count;
    }
    else
        listbox->itemCount = count;

    return true;
}

bool CheckListScrollIconListBoxOnSelection(ITUWidget* widget, char* param)
{
    ITUListBox* listbox = (ITUListBox*) widget;
    ITUScrollIconListBox* silistbox = (ITUScrollIconListBox*) listbox;
    ITUScrollText* scrolltext = (ITUScrollText*)ituListBoxGetFocusItem(listbox);

    if (checkListCount == 0)
        return false;

    if (scrolltext)
    {
        CheckListEntry* entry = ituWidgetGetCustomData(scrolltext);
        if (entry->selected)
        {
            entry->selected = false;
            ituScrollTextSetString(scrolltext, "0");
        }
        else
        {
            entry->selected = true;
            ituScrollTextSetString(scrolltext, "1");
        }
    }
    return true;
}

bool CheckListScrollListBoxOnSelection(ITUWidget* widget, char* param)
{
    return true;
}

bool CheckListResetButtonOnPress(ITUWidget* widget, char* param)
{
    CheckListInit();
    return true;
}

bool CheckListDeleteButtonOnPress(ITUWidget* widget, char* param)
{
    int i, index, count;

    index = count = 0;
    for (i = 0; i < checkListCount; i++)
    {
        if (checkListArray[index].selected)
        {
            memmove(&checkListArray[index], &checkListArray[index + 1], sizeof(CheckListEntry) * (checkListCount - i));
            count++;
        }
        else
        {
            index++;
        }
    }
    checkListCount -= count;

    return true;
}

bool CheckListOnEnter(ITUWidget* widget, char* param)
{
    if (!checkListScrollIconListBox)
    {
        checkListScrollIconListBox = ituSceneFindWidget(&theScene, "checkListScrollIconListBox");
        assert(checkListScrollIconListBox);

        checkListScrollListBox = ituSceneFindWidget(&theScene, "checkListScrollListBox");
        assert(checkListScrollListBox);

        checkListScrollBar = ituSceneFindWidget(&theScene, "checkListScrollBar");
        assert(checkListScrollBar);
    }
    CheckListInit();
    return true;
}

void CheckListReset(void)
{
    checkListScrollIconListBox = NULL;
}

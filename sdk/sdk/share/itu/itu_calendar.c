#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char calendarName[] = "ITUCalendar";

static const char* calendarMonthName[] =
{
    "\xE4\xB8\x80\xE6\x9C\x88", 
    "\xE4\xBA\x8C\xE6\x9C\x88", 
    "\xE4\xB8\x89\xE6\x9C\x88", 
    "\xE5\x9B\x9B\xE6\x9C\x88", 
    "\xE4\xBA\x94\xE6\x9C\x88", 
    "\xE5\x85\xAD\xE6\x9C\x88", 
    "\xE4\xB8\x83\xE6\x9C\x88", 
    "\xE5\x85\xAB\xE6\x9C\x88", 
    "\xE4\xB9\x9D\xE6\x9C\x88", 
    "\xE5\x8D\x81\xE6\x9C\x88", 
    "\xE5\x8D\x81\xE4\xB8\x80\xE6\x9C\x88", 
    "\xE5\x8D\x81\xE4\xBA\x8C\xE6\x9C\x88", 
};

static int CalendarDay(int m1, int y1)
{
    int d;

    if (m1==1 || m1==3 || m1==5 || m1==7 || m1==8 || m1==10 || m1==12)
        d=31;
    else if(m1==4 || m1==6 || m1==9 || m1==11)
        d=30;
    else if((y1%100!=0 && y1%4==0) || y1%400==0)
        d=29;
    else
        d=28;

    return d;
}

static char* GetDayOf(int wCurYear, int wCurMonth, int wCurDay)
{
#if 0
    /* The Heavenly Stems name */ 
    const char* cTianGan[] = {"\xE7\x94\xB2", "\xE4\xB9\x99", "\xE4\xB8\x99", "\xE4\xB8\x81", "\xE6\x88\x8A", "\xE5\xB7\xB1", "\xE5\xBA\x9A", "\xE8\xBE\x9B", "\xE5\xA3\xAC", "\xE7\x99\xB8"};

    /* Earthly name */ 
    const char* cDiZhi[] = {"\xE5\xAD\x90", "\xE4\xB8\x91", "\xE5\xAF\x85", "\xE5\x8D\xAF", "\xE8\xBE\xB0", "\xE5\xB7\xB3", "\xE5\x8D\x88", "\xE6\x9C\xAA", "\xE7\x94\xB3", "\xE9\x85\x89", "\xE6\x88\x8C","\xE4\xBA\xA5"}; 

    /* Zodiac name */ 
    const char* cShuXiang[] = {"\xE9\xBC\xA0", "\xE7\x89\x9B", "\xE8\x99\x8E", "\xE5\x85\x94", "\xE9\xBE\x99", "\xE8\x9B\x87", "\xE9\xA9\xAC", "\xE7\xBE\x8A", "\xE7\x8C\xB4", "\xE9\xB8\xA1", "\xE7\x8B\x97", "\xE7\x8C\xAA"}; 

    /* Lunar month name */ 
    const char* cMonName[] = {"*", "\xE6\xAD\xA3", "\xE4\xBA\x8C", "\xE4\xB8\x89", "\xE5\x9B\x9B", "\xE4\xBA\x94", "\xE5\x85\xAD", "\xE4\xB8\x83", "\xE5\x85\xAB", "\xE4\xB9\x9D", "\xE5\x8D\x81","\xE5\x8D\x81\xE4\xB8\x80","\xE8\x85\x8A"};
#endif // 0

    /* Lunar name */ 
    const char* cDayName[] = {"*", 
        "\xE5\x88\x9D\xE4\xB8\x80", "\xE5\x88\x9D\xE4\xBA\x8C", "\xE5\x88\x9D\xE4\xB8\x89", "\xE5\x88\x9D\xE5\x9B\x9B", "\xE5\x88\x9D\xE4\xBA\x94", "\xE5\x88\x9D\xE5\x85\xAD", "\xE5\x88\x9D\xE4\xB8\x83", "\xE5\x88\x9D\xE5\x85\xAB" , "\xE5\x88\x9D\xE4\xB9\x9D", "\xE5\x88\x9D\xE5\x8D\x81", 
        "\xE5\x8D\x81\xE4\xB8\x80", "\xE5\x8D\x81\xE4\xBA\x8C", "\xE5\x8D\x81\xE4\xB8\x89", "\xE5\x8D\x81\xE5\x9B\x9B", "\xE5\x8D\x81\xE4\xBA\x94", "\xE5\x8D\x81\xE5\x85\xAD", "\xE5\x8D\x81\xE4\xB8\x83", "\xE5\x8D\x81\xE5\x85\xAB" , "\xE5\x8D\x81\xE4\xB9\x9D", "\xE4\xBA\x8C\xE5\x8D\x81",
        "\xE5\xBB\xBF\xE4\xB8\x80", "\xE5\xBB\xBF\xE4\xBA\x8C", "\xE5\xBB\xBF\xE4\xB8\x89", "\xE5\xBB\xBF\xE5\x9B\x9B", "\xE5\xBB\xBF\xE4\xBA\x94", "\xE5\xBB\xBF\xE5\x85\xAD", "\xE5\xBB\xBF\xE4\xB8\x83", "\xE5\xBB\xBF\xE5\x85\xAB" , "\xE5\xBB\xBF\xE4\xB9\x9D", "\xE4\xB8\x89\xE5\x8D\x81" } ; 

    /* In front of the calendar monthly number of days */ 
    const int wMonthAdd[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; 
    
    /* Lunar data */ 
    const int wNongliData[100] = {2635, 333387, 1701, 1748, 267701, 694, 2391, 133423, 1175, 396438,
        3402, 3749, 331177, 1453, 694, 201326, 2350, 465197, 3221, 3402,
        400202, 2901, 1386, 267611, 605, 2349, 137515, 2709, 464533, 1738,
        2901, 330421, 1242, 2651, 199255, 1323, 529706, 3733, 1706, 398762,
        2741, 1206, 267438, 2647, 1318, 204070, 3477, 461653, 1386, 2413,
        330077, 1197, 2637, 268877, 3365, 531109, 2900, 2922, 398042, 2395,
        1179, 267415, 2635, 661067, 1701, 1748, 398772, 2742, 2391, 330031,
        1175, 1611, 200010, 3749, 527717, 1452, 2742, 332397, 2350, 3222,
        268949, 3402, 3493, 133973, 1386, 464219, 605, 2349, 334123, 2709,
        2890, 267946, 2773, 592565, 1210, 2651, 395863, 1323, 2707, 265877}; 
    
    static int nTheDate, nIsEnd, m, k, n, i, nBit; 
    static char szNongli[30], szNongliDay[10], szShuXiang[10]; 

    /* --- The number of days of the initial time February 8, 1921: 1921-2-8 (first day) --- */ 
    nTheDate = (wCurYear - 1921) * 365 + (wCurYear - 1921) / 4 + wCurDay + wMonthAdd[wCurMonth - 1] - 38; 
    if ((!(wCurYear % 4)) && (wCurMonth > 2)) 
        nTheDate = nTheDate + 1;

    /* - Calculation of the Lunar Heavenly Stems and Earthly Branches, month, day --- */ 
    nIsEnd = 0; 
    m = 0; 
    while (nIsEnd != 1) 
    { 
        if (wNongliData[m] < 4095) 
            k = 11; 
        else 
            k = 12; 
    
        n = k; 
        while (n >= 0) 
        { 
            // Get of wNongliData (m) of the first n-bit binary value
            nBit = wNongliData[m]; 
            
            for (i = 1; i < n + 1; i++) 
                nBit = nBit / 2;

            nBit = nBit % 2;
            if (nTheDate <= (29 + nBit)) 
            { 
                nIsEnd = 1; 
                break; 
            }
            nTheDate = nTheDate - 29 - nBit; 
            n = n - 1; 
        } 
        if (nIsEnd)
            break;

        m = m + 1; 
    } 
    wCurYear = 1921 + m;
    wCurMonth = k - n + 1;
    wCurDay = nTheDate;
    if (k == 12)
    { 
        if (wCurMonth == wNongliData[m] / 65536 + 1)
        { 
            wCurMonth = 1 - wCurMonth;
        } 
        else if (wCurMonth> wNongliData[m] / 65536 + 1)
        { 
            wCurMonth = wCurMonth - 1;
        } 
    }
#if 0
    /* - To generate Lunar New Heavenly Stems, Earthly Branches, Zodiac ==> wNongli - */ 
    strcpy(szShuXiang, "%s", cShuXiang[((wCurYear - 4) % 60) % 12]); 
    sprintf(szNongli, "%s (%s%s) years", szShuXiang, cTianGan[((wCurYear - 4) % 60) % 10], cDiZhi[((wCurYear - 4) % 60) % 12]);
    /* - Generated lunar months, day ==> wNongliDay, - */ 
    if (wCurMonth < 1)
    { 
        sprintf(szNongliDay, "leap% s", cMonName[-wCurMonth]);
    } 
    else
    { 
        strcpy(szNongliDay, cMonName[wCurMonth]);
    }
    strcat(szNongliDay, "month"); 
    strcat(szNongliDay, cDayName[wCurDay]); 
    return strcat(szNongli, szNongliDay);
#else
    return (char*)cDayName[wCurDay];
#endif
}

static void CalendarUpdateWeekDays(ITUCalendar* cal)
{
    long unsigned int t;
    unsigned int y,y1,m,m1,d,i,j,k,l,w,days;

    memset(cal->weekDays, 0, sizeof (cal->weekDays));

    y = cal->year;
    m = cal->month;
    y1=0;
    t=0;
    while(y1<y)
    {
        if((y1%100!=0 && y1%4==0) || y1%400==0)
            t=t+366;
		else
		    t=t+365;

		y1++;
    }
    m1=1;
  
    while(m1<m)
    {
        d=CalendarDay(m1,y);
		t=t+d;
		m1++;
    }
    d=t%7;
    k=1;
    l=0;
    w=0;
    days=CalendarDay(m,y);
    for(i=1;i<=days;i++,k++)
    {
        if(i==1)
		{
            if(d==0)
		    {
		        for(j=1;j<7;j++,k++)
                {
                    w=j-1;
                    cal->weekDays[w][0] = 0;
                }
		    }
		    else
		    {
		        for(j=1;j<d;j++,k++)
                {
                    w=j-1;
                    cal->weekDays[w][0] = 0;
                }
		    }
		}
        if(k%7==0)
        {
            w=6;
        }
        else
        {
            w=k%7-1;
        }
        cal->weekDays[w][l] = i;
        if(k%7==0)
        {
            l++;
        }
    }
}

static int CalendarListBoxOnLoadPage(ITUListBox* listbox, int pageIndex)
{
    ITCTree* node;
    ITUScrollText* scrolltext;
    ITUCalendar* cal = (ITUCalendar*)((ITCTree*)listbox)->parent;
    int i, count, w;
    bool hasToday = false;
    assert(listbox);
    assert(cal);

    if (listbox == cal->sunListBox)
    {
        w = 0;
    }
    else if (listbox == cal->monListBox)
    {
        w = 1;
    }
    else if (listbox == cal->tueListBox)
    {
        w = 2;
    }
    else if (listbox == cal->wedListBox)
    {
        w = 3;
    }
    else if (listbox == cal->thuListBox)
    {
        w = 4;
    }
    else if (listbox == cal->friListBox)
    {
        w = 5;
    }
    else if (listbox == cal->satListBox)
    {
        w = 6;
    }
    else
    {
        return 0;
    }

    count = itcTreeGetChildCount(listbox);
    node = ((ITCTree*)listbox)->child;

    for (i = 0; i < count; i++)
    {
        int d = cal->weekDays[w][i];
        scrolltext = (ITUScrollText*) node;

        if (d > 0)
        {
            if (cal->type == ITU_CAL_LUNAR)
            {
                char* s = GetDayOf(cal->year, cal->month, d);
                ituTextSetString(scrolltext, s);
            }
            else
            {
                char buf[4];
                sprintf(buf, "%d", d);
                ituTextSetString(scrolltext, buf);
            }

            if (d == cal->day)
            {
                ituWidgetSetCustomData(scrolltext, (void*)d);
                ituListBoxSelect(listbox, i);
                hasToday = true;
            }
            else 
                ituWidgetSetCustomData(scrolltext, NULL);
        }
        else
        {
            ituTextSetString(scrolltext, "");
            ituWidgetSetCustomData(scrolltext, NULL);
        }

        node = node->sibling;
        if (node == NULL)
            break;
    }

    if (!hasToday)
        ituListBoxSelect(listbox, -1);

    ituWidgetUpdate(listbox, ITU_EVENT_LAYOUT, 0, 0, 0);
    return count;
}

static void CalendarListOnSelection(ITUListBox* listbox, ITUScrollText* item, bool confirm)
{
    int d = (int)ituWidgetGetCustomData(item);
    if (d == 0)
        ituListBoxSelect(listbox, -1);
}

static void CalendarUpdate(ITUCalendar* cal)
{
    CalendarUpdateWeekDays(cal);

    if (cal->sunListBox)
        CalendarListBoxOnLoadPage(cal->sunListBox, 1);

    if (cal->monListBox)
        CalendarListBoxOnLoadPage(cal->monListBox, 1);

    if (cal->tueListBox)
        CalendarListBoxOnLoadPage(cal->tueListBox, 1);

    if (cal->wedListBox)
        CalendarListBoxOnLoadPage(cal->wedListBox, 1);

    if (cal->thuListBox)
        CalendarListBoxOnLoadPage(cal->thuListBox, 1);

    if (cal->friListBox)
        CalendarListBoxOnLoadPage(cal->friListBox, 1);

    if (cal->satListBox)
        CalendarListBoxOnLoadPage(cal->satListBox, 1);

    if (cal->yearText)
    {
        char buf[8];
        sprintf(buf, "%i", cal->year);
        ituTextSetString(cal->yearText, buf);
    }

    if (cal->monthTarget)
    {
        if (cal->monthTarget->type == ITU_TEXT)
        {
            if (cal->type == ITU_CAL_LUNAR)
            {
                ituTextSetString(cal->monthTarget, calendarMonthName[cal->month - 1]);
            }
            else
            {
                char buf[8];
                sprintf(buf, "%i", cal->month);
                ituTextSetString(cal->monthTarget, buf);
            }
        }
        else if (cal->monthTarget->type == ITU_SPRITE)
        {
            ituSpriteGoto((ITUSprite*)cal->monthTarget, cal->month - 1);
        }
    }

    if (cal->dayText)
    {
        char buf[8];
        sprintf(buf, "%i", cal->day);
        ituTextSetString(cal->dayText, buf);
    }
    ituWidgetSetDirty(cal, true);
}

bool ituCalendarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUCalendar* cal = (ITUCalendar*) widget;
    assert(cal);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);
    if (ev == ITU_EVENT_TIMER)
    {
        if (cal->day > 0)
        {
            struct timeval tv;
            struct tm* tm;

            gettimeofday(&tv, NULL);
            tm = localtime(&tv.tv_sec);

            if (cal->year != tm->tm_year + 1900 ||
                cal->month != tm->tm_mon + 1 ||
                tm->tm_mday != cal->day)
            {
                cal->year   = tm->tm_year + 1900;
                cal->month  = tm->tm_mon + 1;
                cal->day    = tm->tm_mday;

                CalendarUpdate(cal);

                result = true;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (cal->year == 0 || cal->month == 0)
        {
            struct timeval tv;
            struct tm* tm;

            gettimeofday(&tv, NULL);
            tm = localtime(&tv.tv_sec);

            cal->year   = tm->tm_year + 1900;
            cal->month  = tm->tm_mon + 1;
            cal->day    = tm->tm_mday;

            CalendarUpdateWeekDays(cal);
        }

        if (!cal->sunListBox && (cal->sunName[0] != '\0'))
        {
            cal->sunListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->sunName);
            ituListBoxSetOnLoadPage(cal->sunListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->sunListBox, CalendarListOnSelection);
            cal->sunListBox->pageIndex = 1;
            cal->sunListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->sunListBox, 1);
        }
        if (!cal->monListBox && (cal->monName[0] != '\0'))
        {
            cal->monListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->monName);
            ituListBoxSetOnLoadPage(cal->monListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->monListBox, CalendarListOnSelection);
            cal->monListBox->pageIndex = 1;
            cal->monListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->monListBox, 1);
        }
        if (!cal->tueListBox && (cal->tueName[0] != '\0'))
        {
            cal->tueListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->tueName);
            ituListBoxSetOnLoadPage(cal->tueListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->tueListBox, CalendarListOnSelection);
            cal->tueListBox->pageIndex = 1;
            cal->tueListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->tueListBox, 1);
        }
        if (!cal->wedListBox && (cal->wedName[0] != '\0'))
        {
            cal->wedListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->wedName);
            ituListBoxSetOnLoadPage(cal->wedListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->wedListBox, CalendarListOnSelection);
            cal->wedListBox->pageIndex = 1;
            cal->wedListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->wedListBox, 1);
        }
        if (!cal->thuListBox && (cal->thuName[0] != '\0'))
        {
            cal->thuListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->thuName);
            ituListBoxSetOnLoadPage(cal->thuListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->thuListBox, CalendarListOnSelection);
            cal->thuListBox->pageIndex = 1;
            cal->thuListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->thuListBox, 1);
        }
        if (!cal->friListBox && (cal->friName[0] != '\0'))
        {
            cal->friListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->friName);
            ituListBoxSetOnLoadPage(cal->friListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->friListBox, CalendarListOnSelection);
            cal->friListBox->pageIndex = 1;
            cal->friListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->friListBox, 1);
        }
        if (!cal->satListBox && (cal->satName[0] != '\0'))
        {
            cal->satListBox = (ITUListBox*) ituSceneFindWidget(ituScene, cal->satName);
            ituListBoxSetOnLoadPage(cal->satListBox, CalendarListBoxOnLoadPage);
            ituListBoxSetOnSelection(cal->satListBox, CalendarListOnSelection);
            cal->satListBox->pageIndex = 1;
            cal->satListBox->pageCount = 1;
            CalendarListBoxOnLoadPage(cal->satListBox, 1);
        }
        if (!cal->yearText && (cal->yearName[0] != '\0'))
        {
            cal->yearText = (ITUText*) ituSceneFindWidget(ituScene, cal->yearName);
        }
        if (cal->yearText)
        {
            char buf[8];
            sprintf(buf, "%i", cal->year);
            ituTextSetString(cal->yearText, buf);
        }
        if (!cal->monthTarget && (cal->monthName[0] != '\0'))
        {
            cal->monthTarget = (ITUWidget*) ituSceneFindWidget(ituScene, cal->monthName);
        }
        if (cal->monthTarget)
        {
            if (cal->monthTarget->type == ITU_TEXT)
            {
                if (cal->type == ITU_CAL_LUNAR)
                {
                    ituTextSetString(cal->monthTarget, calendarMonthName[cal->month - 1]);
                }
                else
                {
                    char buf[8];
                    sprintf(buf, "%i", cal->month);
                    ituTextSetString(cal->monthTarget, buf);
                }
            }
            else if (cal->monthTarget->type == ITU_SPRITE)
            {
                ituSpriteGoto((ITUSprite*)cal->monthTarget, cal->month - 1);
            }
        }
        if (!cal->dayText && (cal->dayName[0] != '\0'))
        {
            cal->dayText = (ITUText*) ituSceneFindWidget(ituScene, cal->dayName);
        }
        if (cal->dayText)
        {
            char buf[8];
            sprintf(buf, "%i", cal->day);
            ituTextSetString(cal->dayText, buf);
        }
        result = widget->dirty = true;
    }
    return widget->visible ? result : false;
}

void ituCalendarOnAction(ITUWidget* widget, ITUActionType action, char* param)
{
    ITUCalendar* cal = (ITUCalendar*) widget;
    assert(cal);

    switch (action)
    {
    case ITU_ACTION_PREV:
        ituCalendarLastMonth(cal);
        break;

    case ITU_ACTION_NEXT:
        ituCalendarNextMonth(cal);
        break;

    case ITU_ACTION_BACK:
        ituCalendarToday(cal);
        break;

    default:
        ituWidgetOnActionImpl(widget, action, param);
        break;
    }
}

void ituCalendarLastMonth(ITUCalendar* cal)
{
    struct timeval tv;
    struct tm* tm;

    ITU_ASSERT_THREAD();

    if (!cal->year || !cal->month)
        return;

    if (cal->month > 1)
    {
        cal->month--;
    }
    else if (cal->year > cal->minYear)
    {
        cal->year--;
        cal->month = 12;
    }
    else
        return;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    if (tm->tm_year + 1900 == cal->year && tm->tm_mon + 1 == cal->month)
        cal->day = tm->tm_mday;
    else
        cal->day = 0;

    CalendarUpdate(cal);
}

void ituCalendarNextMonth(ITUCalendar* cal)
{
    struct timeval tv;
    struct tm* tm;

    ITU_ASSERT_THREAD();

    if (!cal->year || !cal->month)
        return;

    if (cal->month < 12)
    {
        cal->month++;
    }
    else if (cal->year < cal->maxYear)
    {
        cal->year++;
        cal->month = 1;
    }
    else
        return;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    if (tm->tm_year + 1900 == cal->year && tm->tm_mon + 1 == cal->month)
        cal->day = tm->tm_mday;
    else
        cal->day = 0;

    CalendarUpdate(cal);
}

void ituCalendarToday(ITUCalendar* cal)
{
    struct timeval tv;
    struct tm* tm;

    ITU_ASSERT_THREAD();

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    cal->year   = tm->tm_year + 1900;
    cal->month  = tm->tm_mon + 1;
    cal->day    = tm->tm_mday;

    CalendarUpdate(cal);
}

void ituCalendarGoto(ITUCalendar* cal, int year, int month)
{
    ITU_ASSERT_THREAD();

    cal->year   = year;
    cal->month  = month;
    cal->day    = 0;

    CalendarUpdate(cal);
}

void ituCalendarInit(ITUCalendar* cal)
{
    assert(cal);
    ITU_ASSERT_THREAD();

    memset(cal, 0, sizeof (ITUCalendar));

    ituBackgroundInit(&cal->bg);

    ituWidgetSetType(cal, ITU_CALENDAR);
    ituWidgetSetName(cal, calendarName);
    ituWidgetSetUpdate(cal, ituCalendarUpdate);
    ituWidgetSetOnAction(cal, ituCalendarOnAction);
}

void ituCalendarLoad(ITUCalendar* cal, uint32_t base)
{
    assert(cal);

    ituBackgroundLoad(&cal->bg, base);

    if (cal->sunListBox)
        cal->sunListBox = (ITUListBox*)((uint32_t)cal->sunListBox + base);

    if (cal->monListBox)
        cal->monListBox = (ITUListBox*)((uint32_t)cal->monListBox + base);

    if (cal->tueListBox)
        cal->tueListBox = (ITUListBox*)((uint32_t)cal->tueListBox + base);

    if (cal->wedListBox)
        cal->wedListBox = (ITUListBox*)((uint32_t)cal->wedListBox + base);

    if (cal->thuListBox)
        cal->thuListBox = (ITUListBox*)((uint32_t)cal->thuListBox + base);

    if (cal->friListBox)
        cal->friListBox = (ITUListBox*)((uint32_t)cal->friListBox + base);

    if (cal->satListBox)
        cal->satListBox = (ITUListBox*)((uint32_t)cal->satListBox + base);

    if (cal->yearText)
        cal->yearText = (ITUText*)((uint32_t)cal->yearText + base);

    if (cal->monthTarget)
        cal->monthTarget = (ITUWidget*)((uint32_t)cal->monthTarget + base);

    if (cal->dayText)
        cal->dayText = (ITUText*)((uint32_t)cal->dayText + base);

    ituWidgetSetUpdate(cal, ituCalendarUpdate);
    ituWidgetSetOnAction(cal, ituCalendarOnAction);
}

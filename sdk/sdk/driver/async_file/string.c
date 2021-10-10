#include "async_file/config.h"
#include "async_file/pal.h"
#include <malloc.h>
#include <string.h>

#define _f_toupper(ch)  (((ch)>= 'a' && (ch)<= 'z') ? ((ch)- 'a'+ 'A') : (ch))
#define _f_wtoupper(ch) (((ch)>=L'a' && (ch)<=L'z') ? ((ch)-L'a'+L'A') : (ch))
#define _f_tolower(ch)  (((ch)>= 'A' && (ch)<= 'Z') ? ((ch)- 'A'+ 'a') : (ch))
#define _f_wtolower(ch) (((ch)>=L'A' && (ch)<=L'Z') ? ((ch)-L'A'+L'a') : (ch))

void*
PalMemcpy(
    void* s1,
    const void* s2,
    MMP_SIZE_T n)
{
    return memcpy(s1, s2, n);
}

void*
PalMemset(
    void* s,
    MMP_INT c,
    MMP_SIZE_T n)
{
    return memset(s, c, n);
}

MMP_INT
PalMemcmp(
    const void* s1,
    const void* s2,
    MMP_SIZE_T n)
{
    return memcmp(s1, s2, n);
}

MMP_WCHAR*
PalWcscat(
    MMP_WCHAR* s1,
    const MMP_WCHAR* s2)
{
    return (MMP_WCHAR*) wcscat(s1, s2);
}

MMP_WCHAR*
PalWcscpy(
    MMP_WCHAR* s1,
    const MMP_WCHAR* s2)
{
    return (MMP_WCHAR*) wcscpy(s1, s2);
}

MMP_INT
PalWcscmp(
    const MMP_WCHAR* s1,
    const MMP_WCHAR* s2)
{
    return wcscmp(s1, s2);
}

MMP_SIZE_T
PalWcslen(
    const MMP_WCHAR* s)
{
    return wcslen(s);
}

void
PalStrToUpper(
    MMP_WCHAR* string)
{
    MMP_SIZE_T strlen = PalWcslen(string);

    if ( strlen > 0 )
    {
        MMP_UINT32 i = 0;

        for ( i=0; i<strlen; i++ )
        {
            string[i] = _f_wtoupper(string[i]);
        }
    }
}

MMP_INT PalWcmpwname (
    const MMP_WCHAR *wname,
    const MMP_WCHAR *name)
{
    MMP_INT wchCnt = 0;
    MMP_WCHAR wch;
    MMP_WCHAR ch;
    MMP_INT i = 0;

    for(;;)
    {
        wch = (MMP_WCHAR)_f_toupper(*wname);

        if ((!wch))
            break;

        wchCnt++;
        wname++;
    }

    for(;;)
    {
        ch = (MMP_WCHAR)_f_toupper(*name);

        if((!ch))
            break;

        name++;
    }

    for(i=0;i<wchCnt;i++)
    {
        wname--;
        name--;
        wch = (MMP_WCHAR)_f_toupper(*wname);
        ch  = (MMP_WCHAR)_f_toupper(*name);

        if ((wch=='*') && (ch))
        {
            break;
        }

        if (wch!=ch)
        {
            return 0;
        }
    }

    return 1;
}

MMP_WCHAR*
PalWcsstr(
    const MMP_WCHAR *str,
    const MMP_WCHAR *strSearch)
{
    return (MMP_WCHAR*)wcsstr(str, strSearch);
}

/***
 *int PalWcsicmp(dst, src) - compare wide-character strings, ignore case
 *
 *Purpose:
 *       PalWcsicmp perform a case-insensitive MMP_WCHAR string comparision.
 *       PalWcsicmp is independent of locale.
 *
 *Entry:
 *       MMP_WCHAR *string1, *src - strings to compare
 *
 *Return:
 *       Returns <0 if string1 < string2
 *       Returns 0  if string1 = string2
 *       Returns >0 if string1 > string2
 *       Returns _NLSCMPERROR is something went wrong
 *
 *******************************************************************************/
MMP_INT
PalWcsicmp(
    const MMP_WCHAR *string1,
    const MMP_WCHAR *string2)
{
    MMP_WCHAR f,l;

    /* validation section */
    //PalAssert(string1 != NULL);
    //PalAssert(string2 != NULL);

    do
    {
        f = _f_wtolower(*string1);
        l = _f_wtolower(*string2);
        string1++;
        string2++;
    } while ( (f) && (f == l) );

    return (int)(f - l);
}

/***
 *MMP_WCHAR *_wcsdup(string) - duplicate string into malloc'd memory
 *
 *Purpose:
 *       Allocates enough storage via malloc() for a copy of the
 *       string, copies the string into the new memory, and returns
 *       a pointer to it (wide-character).
 *
 *Entry:
 *       MMP_WCHAR *string - string to copy into new memory
 *
 *Exit:
 *       returns a pointer to the newly allocated storage with the
 *       string in it.
 *
 *       returns NULL if enough memory could not be allocated, or
 *       string was NULL.
 *
 *Uses:
 *
 *Exceptions:
 *
 *******************************************************************************/
MMP_WCHAR*
PalWcsdup(
    const MMP_WCHAR *string)
{
    MMP_WCHAR *memory;
    MMP_SIZE_T size = 0;

    if (!string)
        return (MMP_NULL);

    size = wcslen(string) + 1;

    if (memory = (MMP_WCHAR *) calloc(size, sizeof(MMP_WCHAR)))
    {
        wcsncpy(memory, string, size);
        return memory;
    }

    return(NULL);
}

MMP_WCHAR*
PalWcsrchr(
   const MMP_WCHAR  *str,
   MMP_WCHAR        c)
{
    return (MMP_WCHAR*)wcsrchr(str, c);
}


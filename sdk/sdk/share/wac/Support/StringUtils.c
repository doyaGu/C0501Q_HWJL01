/*
    File:    StringUtils.c
    Package: WACServer
    Version: WAC_POSIX_Server_1.20
    
    Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple Inc. ("Apple"), in your
    capacity as a current, and in good standing, Licensee in the MFi Licensing Program. Use of this
    Apple software is governed by and subject to the terms and conditions of your MFi License,
    including, but not limited to, the restrictions specified in the provision entitled ”Public
    Software”, and is further subject to your agreement to the following additional terms, and your
    agreement that the use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these additional terms. If you do not agree with these additional terms,
    please do not use, install, modify or redistribute this Apple software.
    
    Subject to all of these terms and in consideration of your agreement to abide by them, Apple grants
    you, for as long as you are a current and in good-standing MFi Licensee, a personal, non-exclusive
    license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, and modify the Apple Software in source form, and to use, reproduce, modify, and
    redistribute the Apple Software, with or without modifications, in binary form. While you may not
    redistribute the Apple Software in source form, should you redistribute the Apple Software in binary
    form, you must retain this notice and the following text and disclaimers in all such redistributions
    of the Apple Software. Neither the name, trademarks, service marks, or logos of Apple Inc. may be
    used to endorse or promote products derived from the Apple Software without specific prior written
    permission from Apple. Except as expressly stated in this notice, no other rights or licenses,
    express or implied, are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple Software may be
    incorporated.
    
    Unless you explicitly state otherwise, if you provide any ideas, suggestions, recommendations, bug
    fixes or enhancements to Apple in connection with this software (“Feedback”), you hereby grant to
    Apple a non-exclusive, fully paid-up, perpetual, irrevocable, worldwide license to make, use,
    reproduce, incorporate, modify, display, perform, sell, make or have made derivative works of,
    distribute (directly or indirectly) and sublicense, such Feedback in connection with Apple products
    and services. Providing this Feedback is voluntary, but if you do provide Feedback to Apple, you
    acknowledge and agree that Apple may exercise the license granted above without the payment of
    royalties or further consideration to Participant.
    
    The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR
    IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR
    IN COMBINATION WITH YOUR PRODUCTS.
    
    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
    AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
    
    Copyright (C) 2013 Apple Inc. All Rights Reserved.
*/

#include "StringUtils.h"
#include "Debug.h"



//===========================================================================================================================
//  memrlen
//
//  Returns the number of bytes until the last 0 in the string.
//===========================================================================================================================

size_t memrlen( const void *inSrc, size_t inMaxLen )
{
    const uint8_t * const       ptr = (const uint8_t *) inSrc;
    size_t                      i;

    for( i = inMaxLen; ( i > 0 ) && ( ptr[ i - 1 ] == 0 ); --i ) {}
    return i;
}

//===========================================================================================================================
//  TextToHardwareAddress
//
//  Parses hardware address text (e.g. AA:BB:CC:00:11:22:33:44 for Fibre Channel) into an n-byte array.
//  Segments can be separated by a colon ':', dash '-', or a space ' '. Segments do not need zero padding 
//  (e.g. "0:1:2:3:4:5:6:7" is equivalent to "00:01:02:03:04:05:06:07").
//===========================================================================================================================

OSStatus TextToHardwareAddress( const void *inText, size_t inTextSize, size_t inAddrSize, void *outAddr )
{
    OSStatus            err;
    const char *        src;
    const char *        end;
    int                 i;
    int                 x;
    char                c;
    uint8_t *           dst;

    if( inTextSize == kSizeCString ) inTextSize = strlen( (const char *) inText );
    src = (const char *) inText;
    end = src + inTextSize;
    dst = (uint8_t *) outAddr;

    while( inAddrSize-- > 0 )
    {
        x = 0;
        i = 0;
        while( ( i < 2 ) && ( src < end ) )
        {
            c = *src++;
            if(      isdigit_safe(  c ) ) { x = ( x * 16 )      +               ( c   - '0' ); ++i; }
            else if( isxdigit_safe( c ) ) { x = ( x * 16 ) + 10 + ( tolower_safe( c ) - 'a' ); ++i; }
            else if( ( i != 0 ) || ( ( c != ':' ) && ( c != '-' ) && ( c != ' ' ) ) ) break;
        }
        if( i == 0 )
        {
            err = kMalformedErr;
            goto exit;
        }
        require_action( (( x >= 0x00 ) && ( x <= 0xFF )), exit, err = kRangeErr );
        if( dst ) *dst++ = (uint8_t) x;
    }
    err = kNoErr;

exit:
    return err;
}

char* DataToHexString( const uint8_t *inBuf, size_t inBufLen )
{
		uint32_t i;
    char* buf_str = NULL;
    size_t buf_str_max_length = ( 2 * inBufLen );
    require_quiet( inBuf, error );
    require_quiet( inBufLen > 0, error );

    buf_str = (char*) malloc ( buf_str_max_length + 1 );
    require( buf_str, error );
    char* buf_ptr = buf_str;
    for ( i = 0; i < inBufLen; i++ ) 
    	buf_ptr += snprintf( buf_ptr, buf_str_max_length, "%02X", inBuf[i] );
    *buf_ptr = '\0';
    return buf_str;

error:
    FreeSafe( buf_str );
    return NULL;
}

char* DataToHexStringWithSpaces( const uint8_t *inBuf, size_t inBufLen )
{
    char* buf_str = NULL;
    uint32_t i;
    size_t buf_str_max_length = ( 3 * inBufLen );
    require_quiet( inBuf, error );
    require_quiet( inBufLen > 0, error );

    buf_str = (char*) malloc ( buf_str_max_length + 1 );
    require( buf_str, error );
    char* buf_ptr = buf_str;
    for ( i = 0; i < inBufLen; i++ ) buf_ptr += snprintf( buf_ptr, buf_str_max_length, "%02X ", inBuf[i] );
    *buf_ptr = '\0';
    return buf_str;

error:
    FreeSafe( buf_str );
    return NULL;
}

char* DataToHexStringWithColons( const uint8_t *inBuf, size_t inBufLen )
{
    char* buf_str = NULL;
    uint32_t i;
    size_t buf_str_max_length = ( 3 * inBufLen );
    require_quiet( inBuf, error );
    require_quiet( inBufLen > 0, error );

    buf_str = (char*) malloc ( buf_str_max_length + 1 );
    require( buf_str, error );
    char* buf_ptr = buf_str;
    for ( i = 0; i < inBufLen; i++ )
    {
        if ( i == inBufLen - 1 )
            buf_ptr += snprintf( buf_ptr, buf_str_max_length, "%02X", inBuf[i] );
        else
            buf_ptr += snprintf( buf_ptr, buf_str_max_length, "%02X:", inBuf[i] );
    }
    *buf_ptr = '\0';
    return buf_str;

error:
    FreeSafe( buf_str );
    return NULL;
}

char* DataToCString( const uint8_t *inBuf, size_t inBufLen )
{
    char* cString = NULL;
    require_quiet( inBuf, error );
    require_quiet( inBufLen > 0, error );

    cString = (char*) malloc( inBufLen + 1 );
    require( cString, error );
    memcpy( cString, inBuf, inBufLen );
    *( cString + inBufLen ) = '\0';
    return cString;

error:
    FreeSafe( cString );
    return NULL;
}

//===========================================================================================================================
//  strnicmp
//
//  Like the ANSI C strncmp routine, but performs a case-insensitive compare.
//===========================================================================================================================
#if 0
int strnicmp( const char *inS1, const char *inS2, size_t inMax )
{
    const char *        end;
    int                 c1;
    int                 c2;

    end = inS1 + inMax;
    while( inS1 < end )
    {
        c1 = tolower( *( (const unsigned char *) inS1 ) );
        c2 = tolower( *( (const unsigned char *) inS2 ) );
        if( c1 < c2 )    return( -1 );
        if( c1 > c2 )    return(  1 );
        if( c1 == '\0' ) break;

        ++inS1;
        ++inS2;
    }
    return( 0 );
}
#endif
//===========================================================================================================================
//  strnicmpx
//
//  Like the ANSI C strncmp routine, but case-insensitive and requires all characters in s1 match all characters in s2.
//===========================================================================================================================

int strnicmpx( const void *inS1, size_t inN, const char *inS2 )
{
    const unsigned char *       s1;
    const unsigned char *       s2;
    int                         c1;
    int                         c2;

    s1 = (const unsigned char *) inS1;
    s2 = (const unsigned char *) inS2;
    while( inN-- > 0 )
    {
        c1 = tolower( *s1 );
        c2 = tolower( *s2 );
        if( c1 < c2 ) return( -1 );
        if( c1 > c2 ) return(  1 );
        if( c2 == 0 ) return(  0 );

        ++s1;
        ++s2;
    }
    if( *s2 != 0 ) return( -1 );
    return( 0 );
}

//===========================================================================================================================
//  VSNScanF - va_list version of SNScanF.
//===========================================================================================================================

int VSNScanF( const void *inString, size_t inSize, const char *inFormat, va_list inArgs )
{
    int                         matched;
    const unsigned char *       src;
    const unsigned char *       end;
    const unsigned char *       fmt;
    const unsigned char *       old;
    const unsigned char *       setStart;
    const unsigned char *       setEnd;
    const unsigned char *       set;
    int                         notSet;
    int                         suppress;
    int                         alt;
    int                         storePtr;
    int                         fieldWidth;
    int                         sizeModifier;
    unsigned char *             s;
    int *                       i;
    int                         base;
    int                         negative;
    unsigned char               c;
    int64_t                     x;
    int                         v;
    void *                      p;
    const unsigned char **      ptrArg;
    size_t *                    sizeArg;
    size_t                      len;

    if( inSize == kSizeCString ) inSize = strlen( (const char *) inString );
    src = (const unsigned char *) inString;
    end = src + inSize;
    fmt = (const unsigned char *) inFormat;

    matched = 0;
    for( ;; )
    {
        // Skip whitespace. 1 or more whitespace in the format matches 0 or more whitepsace in the string.

        if( isspace( *fmt ) )
        {
            ++fmt;
            while( isspace( *fmt ) )                  ++fmt;
            while( ( src < end ) && isspace( *src ) ) ++src;
        }
        if( *fmt == '\0' ) break;

        // If it's not a conversion, it must match exactly. Otherwise, move onto conversion handling.

        if( *fmt != '%' )
        {
            if( src >= end )        break;
            if( *fmt++ != *src++ )  break;
            continue;
        }
        ++fmt;

        // Flags

        suppress = 0;
        alt      = 0;
        storePtr = 0;
        for( ;; )
        {
            c = *fmt;
            if(      c == '*' ) suppress = 1;
            else if( c == '#' ) alt     += 1;
            else if( c == '&' ) storePtr = 1;
            else break;
            ++fmt;
        }

        // Field width. If none, use INT_MAX to simplify no-width vs width cases.

        if( isdigit( *fmt ) )
        {
            fieldWidth = 0;
            do
            {
                fieldWidth = ( fieldWidth * 10 ) + ( *fmt++ - '0' );

            }   while( isdigit( *fmt ) );
        }
        else if( *fmt == '.' )
        {
            ++fmt;
            fieldWidth = va_arg( inArgs, int );
            if( fieldWidth < 0 ) goto exit;
        }
        else
        {
            fieldWidth = INT_MAX;
        }

        // Size modifier. Note: converts double-char (e.g. hh) into unique char (e.g. H) for easier processing.

        c = *fmt;
        switch( c )
        {
            case 'h':
                if( *( ++fmt ) == 'h' ) { sizeModifier = 'H'; ++fmt; }  // hh for char * / unsigned char *
                else                      sizeModifier = 'h';           // h  for short * / unsigned short *
                break;

            case 'l':
                if( *( ++fmt ) == 'l' ) { sizeModifier = 'L'; ++fmt; }  // ll for long long * / unsigned long long *
                else                      sizeModifier = 'l';           // l  for long * / unsigned long *
                break;

            case 'j':   // j for intmax_t * / uintmax_t *
            case 'z':   // z for size_t *
            case 't':   // t for ptrdiff_t *
                sizeModifier = c;
                ++fmt;
                break;

            default:
                sizeModifier = 0;
                break;
        }
        if( *fmt == '\0' ) break;

        // Conversions

        switch( *fmt++ )
        {
            case 'd':   // %d: Signed decimal integer.
                base = 10;
                break;

            case 'u':   // %u: Unsigned decimal integer.
                base = 10;
                break;

            case 'p':   // %x/%X/%p: Hexidecimal integer.
                if( sizeModifier == 0 ) sizeModifier = 'p';
            case 'x':
            case 'X':
                base = 16;
                break;

            case 'o':   // %o: Octal integer.
                base = 8;
                break;

            case 'i':   // %i: Integer using an optional prefix to determine the base (e.g. 10, 0xA, 012, 0b1010 for decimal 10).
                base = 0;
                break;

            case 'b':   // %b: Binary integer.
                base = 2;
                break;

            case 'c':   // %c: 1 or more characters.

                if( sizeModifier != 0 ) goto exit;
                if( storePtr )
                {
                    len = (size_t)( end - src );
                    if( len > (size_t) fieldWidth )
                    {
                        len = (size_t) fieldWidth;
                    }
                    if( suppress ) { src += len; continue; }

                    ptrArg = va_arg( inArgs, const unsigned char ** );
                    if( ptrArg ) *ptrArg = src;

                    sizeArg = va_arg( inArgs, size_t * );
                    if( sizeArg ) *sizeArg = len;

                    src += len;
                }
                else
                {
                    if( fieldWidth == INT_MAX )         fieldWidth = 1;
                    if( ( end - src ) < fieldWidth )    goto exit;
                    if( suppress )                      { src += fieldWidth; continue; }

                    s = va_arg( inArgs, unsigned char * );
                    if( !s ) goto exit;

                    while( fieldWidth-- > 0 ) *s++ = *src++;
                }
                ++matched;
                continue;

            case 's':   // %s: string of non-whitespace characters with a null terminator.

                if( sizeModifier != 0 ) goto exit;

                // Skip leading white space first since fieldWidth does not include leading whitespace.

                while( ( src < end ) && isspace( *src ) ) ++src;
                if( !alt && ( ( src >= end ) || ( *src == '\0' ) ) ) goto exit;

                // Copy the string until a null terminator, whitespace, or the max fieldWidth is hit.

                if( suppress )
                {
                    while( ( src < end ) && ( *src != '\0' ) && !isspace( *src ) && ( fieldWidth-- > 0 ) ) ++src;
                }
                else if( storePtr )
                {
                    old = src;
                    while( ( src < end ) && ( *src != '\0' ) && !isspace( *src ) && ( fieldWidth-- > 0 ) ) ++src;

                    ptrArg = va_arg( inArgs, const unsigned char ** );
                    if( ptrArg ) *ptrArg = old;

                    sizeArg = va_arg( inArgs, size_t * );
                    if( sizeArg ) *sizeArg = (size_t)( src - old );

                    ++matched;
                }
                else
                {
                    s = va_arg( inArgs, unsigned char * );
                    if( !s ) goto exit;

                    while( ( src < end ) && ( *src != '\0' ) && !isspace( *src ) && ( fieldWidth-- > 0 ) ) *s++ = *src++;
                    *s = '\0';

                    ++matched;
                }
                continue;

            case '[':   // %[: Match a scanset (set between brackets or the compliment set if it starts with ^).

                if( sizeModifier != 0 ) goto exit;

                notSet = ( *fmt == '^' );   // A scanlist starting with ^ matches all characters not in the scanlist.
                if( notSet ) ++fmt;
                setStart = fmt;
                if( *fmt == ']' ) ++fmt;    // A scanlist (after a potential ^) starting with ] includes ] in the set.

                // Find the end of the scanlist.

                while( ( *fmt != '\0' ) && ( *fmt != ']' ) ) ++fmt;
                if( *fmt == '\0' ) goto exit;
                setEnd = fmt++;

                // Parse until a mismatch, null terminator, or the max fieldWidth is hit.

                old = src;
                if( notSet )
                {
                    while( ( src < end ) && ( *src != '\0' ) && ( fieldWidth-- > 0 ) )
                    {
                        c = *src;
                        for( set = setStart; ( set < setEnd ) && ( *set != c ); ++set ) {}
                        if( set < setEnd ) break;
                        ++src;
                    }
                }
                else
                {
                    while( ( src < end ) && ( *src != '\0' ) && ( fieldWidth-- > 0 ) )
                    {
                        c = *src;
                        for( set = setStart; ( set < setEnd ) && ( *set != c ); ++set ) {}
                        if( set >= setEnd ) break;
                        ++src;
                    }
                }
                if( ( old == src ) && !alt ) goto exit;
                if( !suppress )
                {
                    if( storePtr )
                    {
                        ptrArg = va_arg( inArgs, const unsigned char ** );
                        if( ptrArg ) *ptrArg = old;

                        sizeArg = va_arg( inArgs, size_t * );
                        if( sizeArg ) *sizeArg = (size_t)( src - old );
                    }
                    else
                    {
                        s = va_arg( inArgs, unsigned char * );
                        if( !s ) goto exit;

                        while( old < src ) *s++ = *old++;
                        *s = '\0';
                    }
                    ++matched;
                }
                continue;

            case '%':   // %%: Match a literal % character.

                if( sizeModifier != 0 )     goto exit;
                if( fieldWidth != INT_MAX ) goto exit;
                if( suppress )              goto exit;
                if( src >= end )            goto exit;
                if( *src++ != '%' )         goto exit;
                continue;

            case 'n':   // %n: Return the number of characters read so far.

                if( sizeModifier != 0 )     goto exit;
                if( fieldWidth != INT_MAX ) goto exit;
                if( suppress )              goto exit;

                i = va_arg( inArgs, int * );
                if( !i ) goto exit;

                *i = (int)( src - ( (const unsigned char *) inString ) );
                continue;

            default:    // Unknown conversion.
                goto exit;
        }

        // Number conversion. Skip leading white space since number conversions ignore leading white space.

        while( ( src < end ) && isspace( *src ) ) ++src;

        // Handle +/- prefix for negative/positive (even for unsigned numbers).

        negative = 0;
        if( ( ( end - src ) > 1 ) && ( fieldWidth > 0 ) )
        {
            if( src[ 0 ] == '-' )
            {
                negative = 1;
                ++src;
                --fieldWidth;
            }
            else if( src[ 0 ] == '+' )
            {
                ++src;
                --fieldWidth;
            }
        }

        // Detect the base for base 0 and skip valid prefixes.

        old = src;
        if( base == 0 )
        {
            if( ( ( end - src ) > 2 ) && ( fieldWidth >= 2 ) &&
                ( src[ 0 ] == '0' ) && ( tolower( src[ 1 ] ) == 'x' ) && isxdigit( src[ 2 ] ) )
            {
                base         = 16;
                src         +=  2;
                fieldWidth  -=  2;
            }
            else if( ( ( end - src ) > 2 ) && ( fieldWidth >= 2 ) &&
                     ( src[ 0 ] == '0' ) && ( tolower( src[ 1 ] ) == 'b' ) &&
                     ( ( src[ 2 ] == '0' ) || ( src[ 2 ] == '1' ) ) )
            {
                base         = 2;
                src         += 2;
                fieldWidth  -= 2;
            }
            else if( ( ( end - src ) > 1 ) && ( fieldWidth >= 1 ) &&
                     ( src[ 0 ] == '0' ) && ( src[ 1 ] >= '0' ) && ( src[ 1 ] <= '7' ) )
            {
                base         = 8;
                src         += 1;
                fieldWidth  -= 1;
            }
            else
            {
                base = 10;
            }
        }
        else if( ( base == 16 ) && ( ( end - src ) >= 2 ) && ( fieldWidth >= 2 ) &&
                 ( src[ 0 ] == '0' ) && ( tolower( src[ 1 ] ) == 'x' ) )
        {
            src         += 2;
            fieldWidth  -= 2;
        }
        else if( ( base == 2 ) && ( ( end - src ) >= 2 ) && ( fieldWidth >= 2 ) &&
                 ( src[ 0 ] == '0' ) && ( tolower( src[ 1 ] ) == 'b' ) )
        {
            src         += 2;
            fieldWidth  -= 2;
        }

        // Convert the string to a number.

        x = 0;
        while( ( src < end ) && ( fieldWidth-- > 0 ) )
        {
            c = *src;
            if(      isdigit(  c ) ) v = c - '0';
            else if( isxdigit( c ) ) v = 10 + ( tolower( c ) - 'a' );
            else break;
            if( v >= base ) break;

            x = ( x * base ) + v;
            ++src;
        }
        if( src == old ) goto exit;
        if( suppress )   continue;
        if( negative )   x = -x;

        // Store the result.

        p = va_arg( inArgs, void * );
        if( !p ) goto exit;

        switch( sizeModifier )
        {
            case   0: *( (int       *) p ) = (int)                  x; break;
            case 'l': *( (long      *) p ) = (long)                 x; break;
            case 'H': *( (char      *) p ) = (char)                 x; break;
            case 'h': *( (short     *) p ) = (short)                x; break;
            case 'L': *( (int64_t   *) p ) =                        x; break;
            case 'j': *( (intmax_t  *) p ) = (intmax_t)             x; break;
            case 'z': *( (size_t    *) p ) = (size_t)               x; break;
            case 't': *( (ptrdiff_t *) p ) = (ptrdiff_t)            x; break;
            case 'p': *( (void     **) p ) = (void *)( (uintptr_t)  x ); break;

            default:    // Unknown size modifier.
                goto exit;
        }
        ++matched;
    }

exit:
    return( matched );
}

//===========================================================================================================================
//  strnicmp_suffix
//
//  Like strnicmp routine, but only returns 0 if the entire suffix matches.
//===========================================================================================================================

int strnicmp_suffix( const void *inStr, size_t inMaxLen, const char *inSuffix )
{
    const char *        stringPtr;
    size_t              stringLen;
    size_t              suffixLen;

    stringPtr = (const char *) inStr;
    stringLen = strnlen( stringPtr, inMaxLen );
    suffixLen = strlen( inSuffix );
    if( suffixLen <= stringLen )
    {
        return( strnicmpx( stringPtr + ( stringLen - suffixLen ), suffixLen, inSuffix ) );
    }
    return( -1 );
}


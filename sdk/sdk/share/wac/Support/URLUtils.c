/*
    File:    URLUtils.c
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

#include "Common.h"
#include "Debug.h"

#include "StringUtils.h"

#include "URLUtils.h"

#define url_utils_log(M, ...) custom_log("URLUtils", M, ##__VA_ARGS__)

//===========================================================================================================================
//  URLParseComponents
//===========================================================================================================================

OSStatus    URLParseComponents( const char *inSrc, const char *inEnd, URLComponents *outComponents, const char **outSrc )
{
    const char *        ptr;
    const char *        schemePtr;
    const char *        schemeEnd;
    const char *        userPtr;
    const char *        userEnd;
    const char *        passwordPtr;
    const char *        passwordEnd;
    const char *        hostPtr;
    const char *        hostEnd;
    const char *        pathPtr;
    const char *        pathEnd;
    const char *        queryPtr;
    const char *        queryEnd;
    const char *        fragmentPtr;
    const char *        fragmentEnd;
    char                c;
    
    /*
        URL breakdown from RFC 3986.
    
         foo://example.com:8042/over/there?name=ferret#nose
         \_/   \______________/\_________/ \_________/ \__/
          |           |            |            |        |
        scheme    authority       path        query   fragment
          |   _____________________|__
         / \ /                        \ 
         urn:example:animal:ferret:nose
    */
    
    if( inEnd == NULL ) inEnd = inSrc + strlen( inSrc );
    
    // Parse an optional scheme (the "ftp" in "ftp://tom:secret@abc.com/test?x#y").
    
    schemePtr = NULL;
    schemeEnd = NULL;
    
    c = '\0';
    ptr = inSrc;
    while( ( ptr < inEnd ) && ( ( c = *ptr ) != ':' ) && ( c != '/' ) && ( c != '?' ) && ( c != '#' ) ) ++ptr;
    if( c == ':' )
    {
        schemePtr = inSrc;
        schemeEnd = ptr;
        inSrc = ptr + 1;
    }
    
    // Parse an optional authority (the "tom:secret@abc.com" in "ftp://tom:secret@abc.com/test?x#y").
    
    userPtr     = NULL;
    userEnd     = NULL;
    passwordPtr = NULL;
    passwordEnd = NULL;
    hostPtr     = NULL;
    hostEnd     = NULL;
    
    if( ( ( inEnd - inSrc ) >= 2 ) && ( inSrc[ 0 ] == '/' ) && ( inSrc[ 1 ] == '/' ) )
    {
        const char *        authorityPtr;
        const char *        authorityEnd;
        const char *        userInfoPtr;
        const char *        userInfoEnd;
        
        inSrc += 2;
        authorityPtr = inSrc;
        while( ( inSrc < inEnd ) && ( ( c = *inSrc ) != '/' ) && ( c != '?' ) && ( c != '#' ) ) ++inSrc;
        authorityEnd = inSrc;
        
        // Parse an optional userinfo (the "tom:secret" in the above URL).
        
        userInfoPtr = authorityPtr;
        userInfoEnd = userInfoPtr;
        while( ( userInfoEnd < authorityEnd ) && ( *userInfoEnd != '@' ) ) ++userInfoEnd;
        if( userInfoEnd < authorityEnd )
        {
            // Parse the username (the "tom" in the above URL).
            
            userPtr = userInfoPtr;
            userEnd = userPtr;
            while( ( userEnd < userInfoEnd ) && ( *userEnd != ':' ) ) ++userEnd;
            if( userEnd < userInfoEnd )
            {
                // The rest is password/auth info. Note: passwords in URLs are deprecated (see RFC 3986 section 3.2.1).
                
                passwordPtr = userEnd + 1;
                passwordEnd = userInfoEnd;
            }
            
            // The host is the rest of the authority (the "abc.com" in "ftp://tom:secret@abc.com/test?x#y").
            
            hostPtr = userInfoEnd + 1;
            hostEnd = authorityEnd;
        }
        else
        {
            // The host is the entire authority (the "abc.com" in "ftp://tom:secret@abc.com/test?x#y").
            
            hostPtr = authorityPtr;
            hostEnd = authorityEnd;
        }
    }
    
    // Parse the path (the "/test" in "ftp://tom:secret@abc.com/test?x#y").
    
    c = '\0';
    pathPtr = inSrc;
    while( ( inSrc < inEnd ) && ( ( c = *inSrc ) != '?' ) && ( c != '#' ) ) ++inSrc;
    pathEnd = inSrc;
    
    // Parse an optional query (the "x" in "ftp://tom:secret@abc.com/test?x#y").
    
    queryPtr = NULL;
    queryEnd = NULL;    
    if( c == '?' )
    {
        queryPtr = ++inSrc;
        while( ( inSrc < inEnd ) && ( ( c = *inSrc ) != '#' ) ) ++inSrc;
        queryEnd = inSrc;
    }
    
    // Parse an optional fragment  (the "y" in "ftp://tom:secret@abc.com/test?x#y").
    
    fragmentPtr = NULL;
    fragmentEnd = NULL;
    if( c == '#' )
    {
        fragmentPtr = ++inSrc;
        fragmentEnd = inEnd;
        inSrc = inEnd;
    }
    
    outComponents->schemePtr    = schemePtr;
    outComponents->schemeLen    = (size_t)( schemeEnd - schemePtr );
    outComponents->userPtr      = userPtr;
    outComponents->userLen      = (size_t)( userEnd - userPtr );
    outComponents->passwordPtr  = passwordPtr;
    outComponents->passwordLen  = (size_t)( passwordEnd - passwordPtr );
    outComponents->hostPtr      = hostPtr;
    outComponents->hostLen      = (size_t)( hostEnd - hostPtr );
    outComponents->pathPtr      = pathPtr;
    outComponents->pathLen      = (size_t)( pathEnd - pathPtr );
    outComponents->queryPtr     = queryPtr;
    outComponents->queryLen     = (size_t)( queryEnd - queryPtr );
    outComponents->fragmentPtr  = fragmentPtr;
    outComponents->fragmentLen  = (size_t)( fragmentEnd - fragmentPtr );
    outComponents->segmentPtr   = ( ( pathPtr < pathEnd ) && ( *pathPtr      == '/' ) ) ? ( pathPtr + 1 ) : pathPtr;
    outComponents->segmentEnd   = ( ( pathPtr < pathEnd ) && ( pathEnd[ -1 ] == '/' ) ) ? ( pathEnd - 1 ) : pathEnd;
    if( outSrc ) *outSrc = inSrc;
    return kNoErr;
}

void PrintURL( URLComponents *inURL )
{
    (void)inURL;
#if DEBUG
    char *scheme   = DataToCString( (uint8_t*)inURL->schemePtr, inURL->schemeLen );
    char *user     = DataToCString( (uint8_t*)inURL->userPtr, inURL->userLen );
    char *password = DataToCString( (uint8_t*)inURL->passwordPtr, inURL->passwordLen );
    char *host     = DataToCString( (uint8_t*)inURL->hostPtr, inURL->hostLen );
    char *path     = DataToCString( (uint8_t*)inURL->pathPtr, inURL->pathLen );
    char *query    = DataToCString( (uint8_t*)inURL->queryPtr, inURL->queryLen );
    char *fragment = DataToCString( (uint8_t*)inURL->fragmentPtr, inURL->fragmentLen );
    char *segment  = DataToCString( (uint8_t*)inURL->segmentPtr, inURL->segmentEnd - inURL->segmentPtr );

    url_utils_log("scheme: %s", scheme);
    url_utils_log("user: %s", user);
    url_utils_log("password: %s", password);
    url_utils_log("host: %s", host);
    url_utils_log("path: %s", path);
    url_utils_log("query: %s", query);
    url_utils_log("fragment: %s", fragment);
    url_utils_log("segment: %s", segment);

    FreeSafe( scheme );
    FreeSafe( user );
    FreeSafe( password );
    FreeSafe( host );
    FreeSafe( path );
    FreeSafe( query );
    FreeSafe( fragment );
    FreeSafe( segment );
#endif // DEBUG
}


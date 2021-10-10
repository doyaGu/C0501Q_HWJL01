/*
    File:    HTTPUtils.c
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

#include "HTTPUtils.h"

#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/select.h>
#include "Debug.h"

#include "StringUtils.h"

#define kCRLFNewLine     "\r\n"
#define kCRLFLineEnding  "\r\n\r\n"

#define http_utils_log(M, ...) custom_log("HTTPUtils", M, ##__VA_ARGS__)

OSStatus SocketReadHTTPHeader( int inSock, HTTPHeader_t *inHeader )
{
    OSStatus        err;
    char *          buf;
    char *          src;
    char *          dst;
    char *          lim;
    char *          end;
    size_t          len;
    ssize_t         n;

    buf = inHeader->buf;
    src = buf;
    dst = buf + inHeader->len;
    lim = buf + sizeof( inHeader->buf );
    for( ;; )
    {
        // If there's data from a previous read, move it to the front to search it first.
        len = inHeader->extraDataLen;
        if( len > 0 )
        {
            require_action( len <= (size_t)( lim - dst ), exit, err = kParamErr );
            memmove( dst, inHeader->extraDataPtr, len );
            inHeader->extraDataLen = 0;
        }
        else
        {
            do
            {
                n = read( inSock, dst, (size_t)( lim - dst ) );
                err = map_socket_value_errno( inSock, n >= 0, n );
            }   while( err == EINTR );
            if(      n  > 0 ) len = (size_t) n;
            else if( n == 0 ) { err = kConnectionErr; goto exit; }
            else goto exit;
        }
        dst += len;
        inHeader->len += len;

        // Check for interleaved binary data (4 byte header that begins with $). See RFC 2326 section 10.12.
        if( ( ( dst - buf ) >= 4 ) && ( buf[ 0 ] == '$' ) )
        {
            end = buf + 4;
            goto foundHeader;
        }

        // Find an empty line (separates the header and body). The HTTP spec defines it as CRLFCRLF, but some
        // use LFLF or weird combos like CRLFLF so this handles CRLFCRLF, LFLF, and CRLFLF (but not CRCR).
        end = dst;
        for( ;; )
        {
            while( ( src < end ) && ( *src != '\n' ) ) ++src;
            if( src >= end ) break;

            len = (size_t)( end - src );
            if( ( len >= 3 ) && ( src[ 1 ] == '\r' ) && ( src[ 2 ] == '\n' ) ) // CRLFCRLF or LFCRLF.
            {
                end = src + 3;
                goto foundHeader;
            }
            else if( ( len >= 2 ) && ( src[ 1 ] == '\n' ) ) // LFLF or CRLFLF.
            {
                end = src + 2;
                goto foundHeader;
            }
            else if( len <= 1 )
            {
                break;
            }
            ++src;
        }
    }

foundHeader:
    inHeader->len = (size_t)( end - buf );
    err = HTTPHeaderParse( inHeader );
    require_noerr( err, exit );

    inHeader->extraDataPtr = end;
    inHeader->extraDataLen = (size_t)( dst - end );

exit:
    return( err );
}

OSStatus SocketReadHTTPBody( int inSock, HTTPHeader_t *inHeader )
{
    OSStatus err = kParamErr;
    ssize_t readResult;
    int selectResult;
    fd_set readSet;

    require( inHeader, exit );

    err = kNotReadableErr;

    FD_ZERO( &readSet );
    FD_SET( inSock, &readSet );
    while ( inHeader->extraDataLen < inHeader->contentLength )
    {
        selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
        require( selectResult >= 1, exit );

        readResult = read( inSock,
                           (uint8_t*)( inHeader->extraDataPtr + inHeader->extraDataLen ),
                           ( inHeader->contentLength - inHeader->extraDataLen ) );
        require( readResult >= 0, exit );

        inHeader->extraDataLen += readResult;

    }

    err = kNoErr;

exit:
    return err;
}

OSStatus HTTPHeaderParse( HTTPHeader_t *ioHeader )
{
    OSStatus            err;
    const char *        src;
    const char *        end;
    const char *        ptr;
    char                c;
    const char *        value;
    size_t              valueSize;
    int                 x;

    require_action( ioHeader->len < sizeof( ioHeader->buf ), exit, err = kParamErr );

    // Reset fields up-front to good defaults to simplify handling of unused fields later.

    ioHeader->methodPtr         = "";
    ioHeader->methodLen         = 0;
    ioHeader->urlPtr            = "";
    ioHeader->urlLen            = 0;
    memset( &ioHeader->url, 0, sizeof( ioHeader->url ) );
    ioHeader->protocolPtr       = "";
    ioHeader->protocolLen       = 0;
    ioHeader->statusCode        = -1;
    ioHeader->reasonPhrasePtr   = "";
    ioHeader->reasonPhraseLen   = 0;
    ioHeader->channelID         = 0;
    ioHeader->contentLength     = 0;
    ioHeader->persistent        = false;

    // Check for a 4-byte interleaved binary data header (see RFC 2326 section 10.12). It has the following format:
    //
    //      '$' <1:channelID> <2:dataSize in network byte order> ... followed by dataSize bytes of binary data.
    src = ioHeader->buf;
    if( ( ioHeader->len == 4 ) && ( src[ 0 ] == '$' ) )
    {
        const uint8_t *     usrc;

        usrc = (const uint8_t *) src;
        ioHeader->channelID     =   usrc[ 1 ];
        ioHeader->contentLength = ( usrc[ 2 ] << 8 ) | usrc[ 3 ];

        ioHeader->methodPtr = src;
        ioHeader->methodLen = 1;

        err = kNoErr;
        goto exit;
    }

    // Parse the start line. This will also determine if it's a request or response.
    // Requests are in the format <method> <url> <protocol>/<majorVersion>.<minorVersion>, for example:
    //
    //      GET /abc/xyz.html HTTP/1.1
    //      GET http://www.host.com/abc/xyz.html HTTP/1.1
    //      GET http://user:password@www.host.com/abc/xyz.html HTTP/1.1
    //
    // Responses are in the format <protocol>/<majorVersion>.<minorVersion> <statusCode> <reasonPhrase>, for example:
    //
    //      HTTP/1.1 404 Not Found
    ptr = src;
    end = src + ioHeader->len;
    for( c = 0; ( ptr < end ) && ( ( c = *ptr ) != ' ' ) && ( c != '/' ); ++ptr ) {}
    require_action( ptr < end, exit, err = kMalformedErr );

    if( c == ' ' ) // Requests have a space after the method. Responses have '/' after the protocol.
    {
        ioHeader->methodPtr = src;
        ioHeader->methodLen = (size_t)( ptr - src );
        ++ptr;

        // Parse the URL.
        ioHeader->urlPtr = ptr;
        while( ( ptr < end ) && ( *ptr != ' ' ) ) ++ptr;
        ioHeader->urlLen = (size_t)( ptr - ioHeader->urlPtr );
        require_action( ptr < end, exit, err = kMalformedErr );
        ++ptr;

        err = URLParseComponents( ioHeader->urlPtr, ioHeader->urlPtr + ioHeader->urlLen, &ioHeader->url, NULL );
        require_noerr( err, exit );

        // Parse the protocol and version.
        ioHeader->protocolPtr = ptr;
        while( ( ptr < end ) && ( ( c = *ptr ) != '\r' ) && ( c != '\n' ) ) ++ptr;
        ioHeader->protocolLen = (size_t)( ptr - ioHeader->protocolPtr );
        require_action( ptr < end, exit, err = kMalformedErr );
        ++ptr;
    }
    else // Response
    {
        // Parse the protocol version.
        ioHeader->protocolPtr = src;
        for( ++ptr; ( ptr < end ) && ( *ptr != ' ' ); ++ptr ) {}
        ioHeader->protocolLen = (size_t)( ptr - ioHeader->protocolPtr );
        require_action( ptr < end, exit, err = kMalformedErr );
        ++ptr;

        // Parse the status code.
        x = 0;
        for( c = 0; ( ptr < end ) && ( ( c = *ptr ) >= '0' ) && ( c <= '9' ); ++ptr ) x = ( x * 10 ) + ( c - '0' ); 
        ioHeader->statusCode = x;
        if( c == ' ' ) ++ptr;

        // Parse the reason phrase.
        ioHeader->reasonPhrasePtr = ptr;
        while( ( ptr < end ) && ( ( c = *ptr ) != '\r' ) && ( c != '\n' ) ) ++ptr;
        ioHeader->reasonPhraseLen = (size_t)( ptr - ioHeader->reasonPhrasePtr );
        require_action( ptr < end, exit, err = kMalformedErr );
        ++ptr;
    }

    // There should at least be a blank line after the start line so make sure there's more data.
    require_action( ptr < end, exit, err = kMalformedErr );

    // Determine persistence. Note: HTTP 1.0 defaults to non-persistent if a Connection header field is not present.
    err = HTTPGetHeaderField( ioHeader->buf, ioHeader->len, "Connection", NULL, NULL, &value, &valueSize, NULL );
    if( err )   ioHeader->persistent = (Boolean)( strnicmpx( ioHeader->protocolPtr, ioHeader->protocolLen, "HTTP/1.0" ) != 0 );
    else        ioHeader->persistent = (Boolean)( strnicmpx( value, valueSize, "close" ) != 0 );

    // Content-Length is such a common field that we get it here during general parsing.
    HTTPScanFHeaderValue( ioHeader->buf, ioHeader->len, "Content-Length", "%llu", &ioHeader->contentLength );
    err = kNoErr;

exit:
    return err;
}

OSStatus HTTPGetHeaderField( const char *inHeaderPtr, 
                             size_t     inHeaderLen, 
                             const char *inName, 
                             const char **outNamePtr, 
                             size_t     *outNameLen, 
                             const char **outValuePtr, 
                             size_t     *outValueLen, 
                             const char **outNext )
{
    const char *        src;
    const char *        end;
    size_t              matchLen;
    char                c;

    if( inHeaderLen == kSizeCString ) inHeaderLen = strlen( inHeaderPtr );
    src = inHeaderPtr;
    end = src + inHeaderLen;
    matchLen = inName ? strlen( inName ) : 0;
    for( ;; )
    {
        const char *        linePtr;
        const char *        lineEnd;
        size_t              lineLen;
        const char *        valuePtr;
        const char *        valueEnd;

        // Parse a line and check if it begins with the header field we're looking for.
        linePtr = src;
        while( ( src < end ) && ( ( c = *src ) != '\r' ) && ( c != '\n' ) ) ++src;
        if( src >= end ) break;
        lineEnd = src;
        lineLen = (size_t)( src - linePtr );
        if( ( src < end ) && ( *src == '\r' ) ) ++src;
        if( ( src < end ) && ( *src == '\n' ) ) ++src;

        if( !inName ) // Null name means to find the next header for iteration.
        {
            const char *        nameEnd;

            nameEnd = linePtr;
            while( ( nameEnd < lineEnd ) && ( *nameEnd != ':' ) ) ++nameEnd;
            if( nameEnd >= lineEnd ) continue;
            matchLen = (size_t)( nameEnd - linePtr );
        }
        else if( ( lineLen <= matchLen ) || ( linePtr[ matchLen ] != ':' ) || 
                 ( strnicmp( linePtr, inName, matchLen ) != 0 ) )
        {
            continue;
        }

        // Found the header field. Separate name and value and skip leading whitespace in the value.
        valuePtr = linePtr + matchLen + 1;
        valueEnd = lineEnd;
        while( ( valuePtr < valueEnd ) && ( ( ( c = *valuePtr ) == ' ' ) || ( c == '\t' ) ) ) ++valuePtr;

        // If the next line is a continuation line then keep parsing until we get to the true end.
        while( ( src < end ) && ( ( ( c = *src ) == ' ' ) || ( c == '\t' ) ) )
        {
            ++src;
            while( ( src < end ) && ( ( c = *src ) != '\r' ) && ( c != '\n' ) ) ++src;
            valueEnd = src;
            if( ( src < end ) && ( *src == '\r' ) ) ++src;
            if( ( src < end ) && ( *src == '\n' ) ) ++src;
        }

        if( outNamePtr )    *outNamePtr     = linePtr;
        if( outNameLen )    *outNameLen     = matchLen;
        if( outValuePtr )   *outValuePtr    = valuePtr;
        if( outValueLen )   *outValueLen    = (size_t)( valueEnd - valuePtr );
        if( outNext )       *outNext        = src;
        return( kNoErr );
    }
    return kNotFoundErr;
}

int HTTPScanFHeaderValue( const char *inHeaderPtr, size_t inHeaderLen, const char *inName, const char *inFormat, ... )
{
    int                 n;
    const char *        valuePtr;
    size_t              valueLen;
    va_list             args;

    n = (int) HTTPGetHeaderField( inHeaderPtr, inHeaderLen, inName, NULL, NULL, &valuePtr, &valueLen, NULL );
    require_noerr_quiet( n, exit );

    va_start( args, inFormat );
    n = VSNScanF( valuePtr, valueLen, inFormat, args );
    va_end( args );

exit:
    return( n );
}

OSStatus HTTPHeaderMatchMethod( HTTPHeader_t *inHeader, const char *method )
{
    if( strnicmpx( inHeader->methodPtr, inHeader->methodLen, method ) == 0 )
        return kNoErr;

    return kNotFoundErr;
}

OSStatus HTTPHeaderMatchURL( HTTPHeader_t *inHeader, const char *url )
{
    if( strnicmp_suffix( inHeader->url.pathPtr, inHeader->url.pathLen, url ) == 0 )
        return kNoErr;

    return kNotFoundErr;
}

void HTTPHeaderClear( HTTPHeader_t *inHeader )
{
    inHeader->len = 0;
    inHeader->extraDataLen = 0;
}

OSStatus CreateSimpleHTTPOKMessage( uint8_t **outMessage, size_t *outMessageSize )
{
    OSStatus err = kNoMemoryErr;
    size_t outMessageMaxSize = 200;
    *outMessage = malloc( outMessageMaxSize );
    require( *outMessage, exit );

    snprintf( (char*)*outMessage,
              outMessageMaxSize,
              "%s %s %s%s",
              "HTTP/1.1", "200", "OK", kCRLFLineEnding );
    *outMessageSize = strlen( (char*)*outMessage );

    err = kNoErr;

exit:
    return err;
}

OSStatus CreateSimpleHTTPMessage( const char *contentType, uint8_t *inData, size_t inDataLen, uint8_t **outMessage, size_t *outMessageSize )
{
    OSStatus err = kParamErr;

    require( contentType, exit );
    require( inData, exit );
    require( inDataLen, exit );

    err = kNoMemoryErr;
    size_t outMessageMaxSize = 2048;
    *outMessage = malloc( outMessageMaxSize + 1 );
    require( *outMessage, exit );

    // Create HTTP Response
    snprintf( (char*)*outMessage,
              outMessageMaxSize,
              "%s %s %s%s%s %s%s%s %d%s",
              "HTTP/1.1", "200", "OK", kCRLFNewLine,
              "Content-Type:", contentType, kCRLFNewLine,
              "Content-Length:", (int)inDataLen, kCRLFLineEnding );

    // outMessageSize will be the length of the HTTP Header plus the data length
    *outMessageSize = strlen( (char*)*outMessage ) + inDataLen;

    uint8_t *endOfHTTPHeader = *outMessage + strlen( (char*)*outMessage );
    memcpy( endOfHTTPHeader, inData, inDataLen );
    err = kNoErr;

exit:
    return err;
}

void PrintHTTPHeader( HTTPHeader_t *inHeader )
{
    (void)inHeader; // Fix warning when debug=0
    http_utils_log("Header:\n %s", inHeader->buf);
    http_utils_log("Length: %d", (int)inHeader->len);
    http_utils_log("Method: %s", inHeader->methodPtr);
    http_utils_log("URL: %s", inHeader->urlPtr);
    http_utils_log("Protocol: %s", inHeader->protocolPtr);
    http_utils_log("Status Code: %d", inHeader->statusCode);
    http_utils_log("ChannelID: %d", inHeader->channelID);
    http_utils_log("Content length: %llu", inHeader->contentLength);
    http_utils_log("Persistent: %s", YesOrNo( inHeader->persistent ));
    char *extraData = DataToHexString( (uint8_t*)inHeader->extraDataPtr, inHeader->extraDataLen );
    http_utils_log("Extra data: %s", extraData );
    FreeSafe( extraData );
    http_utils_log("contentlength: %llu", inHeader->contentLength );
}


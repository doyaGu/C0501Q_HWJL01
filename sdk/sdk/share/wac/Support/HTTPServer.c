/*
    File:    HTTPServer.c
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

#include "HTTPServer.h"

#include "Debug.h"

#include "StringUtils.h"
#include "SocketUtils.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#define PREFERRED_SERVER_PORT  5000
#define HTTP_SERVER_QUEUE_SIZE 1

#define http_server_log(M, ...) custom_log("HTTPServer", M, ##__VA_ARGS__)

#define DEVNULL "/dev/null"

static uint32_t gUUIDCounter = 0;

// ==== THREADS ====
static void *_HTTPServerListenerThread( void *inServer );

OSStatus HTTPServerCreate( const char *name, void *callback, void *callerPayload, HTTPServer_t *outServer )
{
    Trace();
    OSStatus err = kParamErr;

    require( name, exit );
    require( callback, exit );
#if ENABLE_IPV6
    // Set an initial state if an error were to occur
    outServer->state = eStateUnconfigured;

    // Verify the sockAddr struct is zero'd
    memset( &outServer->sockAddr, 0, sizeof( struct sockaddr_in6 ) );

    // Add server name to HTTPServer_t
    outServer->name = name;

    // Assign callback to HTTPServer_t
    outServer->callback = callback;

    // Assign caller payload to HTTPServer_t
    outServer->callerPayload = callerPayload;

    // Assign UUID for server
    outServer->uuid = gUUIDCounter++;

    // Setup socket parameters
    outServer->sockAddr.sin6_family = AF_INET6;
    outServer->sockAddr.sin6_addr   = in6addr_any;
    outServer->sockAddr.sin6_port   = htons( PREFERRED_SERVER_PORT );

    // Create socket
    outServer->acceptSocket = socket( outServer->sockAddr.sin6_family, SOCK_STREAM, 0 );
    require_action( SocketIsValid( outServer->acceptSocket ), exit, err = kNoResourcesErr );

    // Mark socket as non-blocking
    SocketMakeNonBlocking( outServer->acceptSocket );

    // Try to Bind preferred port
    err = bind( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, sizeof( struct sockaddr_in6 ) );
    err = map_socket_noerr_errno( outServer->acceptSocket, err );

    // If we couldn't bind to the preferred port, bind to dynamic
    if ( err )
    {
        http_server_log("WARNING: Could not bind preferred socket %d", PREFERRED_SERVER_PORT);
        perror("Bind perror");
        outServer->sockAddr.sin6_port = htons( 0 );
        err = bind( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, sizeof( struct sockaddr_in6 ) );
        err = map_socket_noerr_errno( outServer->acceptSocket, err );
        socklen_t sockLen = sizeof( struct sockaddr_in6 );
        err = getsockname( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, &sockLen );
        err = map_socket_noerr_errno( outServer->acceptSocket, err );
    }
    require_noerr_action( err, exit, http_server_log("ERROR: Could not bind to socket"); perror("Bind perror") );

    // Listen on socket
    err = listen( outServer->acceptSocket, HTTP_SERVER_QUEUE_SIZE );
    require_noerr( err, exit );

    // Server is now ready, but not started
    outServer->state = eStateNotStarted;
#else
    // Set an initial state if an error were to occur
    outServer->state = eStateUnconfigured;

    // Verify the sockAddr struct is zero'd
    memset( &outServer->sockAddr, 0, sizeof( struct sockaddr_in ) );

    // Add server name to HTTPServer_t
    outServer->name = name;

    // Assign callback to HTTPServer_t
    outServer->callback = callback;

    // Assign caller payload to HTTPServer_t
    outServer->callerPayload = callerPayload;

    // Assign UUID for server
    outServer->uuid = gUUIDCounter++;

    // Setup socket parameters
    outServer->sockAddr.sin_family = AF_INET;
    outServer->sockAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    outServer->sockAddr.sin_port   = htons( PREFERRED_SERVER_PORT );

    // Create socket
    outServer->acceptSocket = socket( outServer->sockAddr.sin_family, SOCK_STREAM, 0 );
    require_action( SocketIsValid( outServer->acceptSocket ), exit, err = kNoResourcesErr );

    // Mark socket as non-blocking
    SocketMakeNonBlocking( outServer->acceptSocket );

    // Try to Bind preferred port
    err = bind( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, sizeof( struct sockaddr_in ) );
    err = map_socket_noerr_errno( outServer->acceptSocket, err );

    // If we couldn't bind to the preferred port, bind to dynamic
    if ( err )
    {
        http_server_log("WARNING: Could not bind preferred socket %d", PREFERRED_SERVER_PORT);
        perror("Bind perror");
        outServer->sockAddr.sin_port = htons( 0 );
        err = bind( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, sizeof( struct sockaddr_in ) );
        err = map_socket_noerr_errno( outServer->acceptSocket, err );
        socklen_t sockLen = sizeof( struct sockaddr_in );
        err = getsockname( outServer->acceptSocket, (struct sockaddr*)&outServer->sockAddr, &sockLen );
        err = map_socket_noerr_errno( outServer->acceptSocket, err );
    }
    require_noerr_action( err, exit, http_server_log("ERROR: Could not bind to socket"); perror("Bind perror") );

    // Listen on socket
    err = listen( outServer->acceptSocket, HTTP_SERVER_QUEUE_SIZE );
    require_noerr( err, exit );

    // Server is now ready, but not started
    outServer->state = eStateNotStarted;
    printf("httpServer create  %d \n",outServer->acceptSocket);
#endif
    err = kNoErr;
exit:
    if ( err ) outServer->state = eStateError;
    return err;
}

OSStatus HTTPServerStart( HTTPServer_t *inServer )
{
    Trace();
    OSStatus err = kParamErr;

    require( inServer, exit );

    // Create non-blocking pipe for inter-thread communication
    err = pipe( inServer->threadCommandPipe );
    inServer->threadCommandPipe[kPipeReadFD] = (DEVNULL, 2);
    inServer->threadCommandPipe[kPipeWriteFD] = (DEVNULL, 2);    
    SocketMakeNonBlocking( inServer->threadCommandPipe[kPipeReadFD] );

    // Create thread for HTTP server
    err = pthread_create( &(inServer->thread), NULL, _HTTPServerListenerThread, inServer );
    require_noerr_action( err, exit, http_server_log("ERROR: Could not create HTTPServerListenerThread %d", err); perror("pthread_create perror"); err = kUnexpectedErr );
    printf("httpServer start\n");
exit:
    if ( err ) inServer->state = eStateError;
    return err;
}

OSStatus HTTPServerStop( HTTPServer_t *inServer )
{
    Trace();
    OSStatus err = kParamErr;
    int numWritten;
    char quitCommand;
    require_quiet( inServer, exit );
    require_quiet( inServer->state != eStateStopped, exit );

    // Signal and wait for thread to end
    inServer->state = eStateStopped;
    quitCommand = 'q';
    numWritten = write( inServer->threadCommandPipe[kPipeWriteFD], &quitCommand, sizeof( quitCommand ) );
    require_action( numWritten == sizeof( quitCommand ), exit, err = kNotWritableErr );

    http_server_log("Waiting for http server listener thread to join");

    // Wait for listener thread
    pthread_join( inServer->thread, NULL );

    close( inServer->threadCommandPipe[kPipeReadFD] );
    close( inServer->threadCommandPipe[kPipeWriteFD] );

    err = kNoErr;

exit:
    return err;
}

OSStatus HTTPServerSend( HTTPServer_t *inServer, const uint8_t *inBuf, size_t inBufLen )
{
    Trace();
    OSStatus err = kParamErr;
    ssize_t writeResult;
    int selectResult;
    size_t numWritten;
    fd_set writeSet;

    require( inServer, exit );
    require( inBuf, exit );
    require( inBufLen, exit );

    err = kNotWritableErr;

    FD_ZERO( &writeSet );
    FD_SET( inServer->connectedSocket, &writeSet );
    numWritten = 0;
    do
    {
        selectResult = select( inServer->connectedSocket + 1, NULL, &writeSet, NULL, NULL );
        require( selectResult >= 1, exit );

        writeResult = write( inServer->connectedSocket, ( inBuf + numWritten ), ( inBufLen - numWritten ) );
        require( writeResult > 0, exit );

        numWritten += writeResult;

        http_server_log("Wrote %zu / %zu bytes", numWritten, inBufLen);

    } while( numWritten < inBufLen );

    require_action( numWritten == inBufLen,
                    exit,
                    http_server_log("ERROR: Did not write all the bytes in the buffer. BufLen: %zu, Bytes Written: %zu", inBufLen, numWritten ); err = kUnderrunErr );

    err = kNoErr;

exit:
    return err;
}

void HTTPServerPrint( HTTPServer_t *inServer )
{
    Trace();

    require( inServer, exit );

    http_server_log("HTTP SERVER:");
    http_server_log("    NAME............: %s", inServer->name);
    http_server_log("    UUID............: %d", inServer->uuid);
    http_server_log("    STATE...........: %s", HTTPServerStateToString( inServer->state ) );
    if ( SocketIsValid( inServer->acceptSocket ) )
    {
        http_server_log("    SOCKET..........: %d", inServer->acceptSocket);
    }
    else
    {
        http_server_log("    SOCKET..........: INVALID");
    }
    if ( SocketIsValid( inServer->connectedSocket ) )
    {
        http_server_log("    CONNECTED SOCKET: %d", inServer->connectedSocket);
    }
    else
    {
        http_server_log("    CONNECTED SOCKET: INVALID");
    }
    http_server_log("    SOCKADDR:");
#if ENABLE_IPV6
    http_server_log("        FAMILY: %d", inServer->sockAddr.sin6_family);
    http_server_log("        PORT..: %d", ntohs( inServer->sockAddr.sin6_port ) );

#else
    http_server_log("        FAMILY: %d", inServer->sockAddr.sin_family);
    http_server_log("        PORT..: %d", ntohs( inServer->sockAddr.sin_port ) );

#endif

exit:
    return;
}

static void *_HTTPServerListenerThread( void *inServer )
{
    Trace();
    OSStatus        err = kParamErr;
    HTTPHeader_t    *httpHeader = NULL;
    HTTPServer_t    *server = NULL;
    socklen_t       sockAddrLen;
    int             selectResult;
    int             maxFD;
    fd_set          readSet;
    bool            connected = false;

#if ENABLE_IPV6
    sockAddrLen = sizeof( struct sockaddr_in6 );
#else
    sockAddrLen = sizeof( struct sockaddr_in);
#endif
    require( sockAddrLen > 0, exit );
    server = inServer;
    require( server, exit );

    http_server_log("\"%s\":%d listener thread started", server->name, server->uuid);
    server->state = eStateRunning;

    // Create and initialize an HTTPHeader
    httpHeader = malloc( sizeof( HTTPHeader_t ) );
    require_action( httpHeader, exit, err = kNoMemoryErr );
    HTTPHeaderClear( httpHeader );

    do
    {
        // Set up socket set
        FD_ZERO( &readSet );
        FD_SET( server->threadCommandPipe[kPipeReadFD], &readSet );
        if ( !connected )
        {
            FD_SET( server->acceptSocket, &readSet );
            maxFD = Max( server->threadCommandPipe[kPipeReadFD], server->acceptSocket );
        }
        else
        {
            FD_SET( server->connectedSocket, &readSet );
            maxFD = Max( server->threadCommandPipe[kPipeReadFD], server->connectedSocket );
        }
printf("_HTTPServerListenerThread select begin %d %d \n",server->threadCommandPipe[kPipeReadFD],server->acceptSocket);
        // Multiplex connections with select()
        selectResult = select( maxFD + 1, &readSet, NULL, NULL, NULL );
        require( selectResult >= 0, exit );
printf("_HTTPServerListenerThread select \n");
        if ( FD_ISSET( server->acceptSocket, &readSet ) )
        {
            require( !connected, exit );
            server->connectedSocket = accept( server->acceptSocket, (struct sockaddr*)&server->sockAddr, &sockAddrLen );
            require_action( SocketIsValid( server->connectedSocket ), exit, perror( "accept perror" ) );
            connected = true;
        }
        if ( FD_ISSET( server->connectedSocket, &readSet ) )
        {
            require( connected, exit );
            err = SocketReadHTTPHeader( server->connectedSocket, httpHeader );

            switch ( err )
            {
                case kNoErr:
                    // Read the rest of the HTTP body if necessary
                    err = SocketReadHTTPBody( server->connectedSocket, httpHeader );
                    require_noerr( err, exit );
                    // Call the HTTPServer owner back with the acquired HTTP header
                    server->callback( httpHeader, false, server->callerPayload );
                    // Reuse HTTPHeader
                    HTTPHeaderClear( httpHeader );
                break;

                case EWOULDBLOCK:
                    // NO-OP, keep reading
                break;

                case kNoSpaceErr:
                    http_server_log("ERROR: Cannot fit HTTPHeader.");
                    goto exit;
                break;

                case kConnectionErr:
                    // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
                    err = kNoErr;
                    goto exit;
                break;

                default:
                    http_server_log("ERROR: HTTP Header parse internal error: %d", err);
                    goto exit;
            }
        }
        if ( FD_ISSET( server->threadCommandPipe[kPipeReadFD], &readSet ) )
        {
            char command;
            int readResult = read( server->threadCommandPipe[kPipeReadFD], &command, sizeof( command ) );
            require( readResult >= 0, exit );

            if ( readResult == sizeof( command ) && command == 'q' ) break; // Caller wants to kill thread
            else if ( readResult == 0 ) break; // pipe is closed
            else require_action( 0,
                                 exit,
                                 http_server_log("ERROR: Unknown command over pipe") ); // Unknown command, error
        }

    } while ( server->state != eStateStopped );

exit:
    server->state = eStateStopped;
    server->callback( NULL, true, server->callerPayload );
    http_server_log("HTTP server \"%s\":%d listener thread stopped", server->name, server->uuid);
    if ( err ) FreeSafe( httpHeader );
    err = close( server->connectedSocket );
    if ( err ) { http_server_log("ERROR closing connected socket"); }
    return NULL;
}


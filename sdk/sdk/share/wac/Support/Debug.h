/*
    File:    Debug.h
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

/*!
     @header Debug
      This header contains defines, macros, and functions to aid in
      debugging the WAC project.
 */

#ifndef __Debug_h__
#define __Debug_h__

#include "Common.h"

// ==== LOGGING ====
#define SHORT_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define YesOrNo(x) (x ? "YES" : "NO")

#define DEBUG 1
#define TRACE 1


#if DEBUG
    #define custom_log(N, M, ...) fprintf(stdout, "[%s: %s:%4d] " M "\n", N, SHORT_FILE, __LINE__, ##__VA_ARGS__)
    #define debug_print_assert(A,B,C,D,E,F, ...) fprintf(stderr, "[WAC: %s:%s:%4d] **ASSERT** %s""\n", D ? D : "", F, E, C ? C : "", ##__VA_ARGS__)

    #if TRACE
        #define Trace() fprintf(stdout, "[TRACE] [%s %s()]\n", SHORT_FILE, __PRETTY_FUNCTION__)
    #else  // !TRACE
        #define Trace()
    #endif // TRACE
#else // DEBUG = 0
    // IF !DEBUG, make the logs NO-OP
    #define custom_log(N, M, ...)

    #define Trace()

    #define debug_print_assert(A,B,C,D,E,F, ...)
#endif // DEBUG

// ==== PLATFORM TIMEING FUNCTIONS ====
#if TIME_PLATFORM
    #define function_timer_log(M, N, ...) fprintf(stdout, "[FUNCTION TIMER: " N "()] " M "\n", ##__VA_ARGS__)

    #define TIMEPLATFORM( FUNC, FUNC_NAME )                                             \
                do                                                                      \
                {                                                                       \
                    struct timespec startTime;                                          \
                    clock_gettime(CLOCK_MONOTONIC, &startTime);                         \
                    { FUNC; }                                                           \
                    struct timespec endTime;                                            \
                    clock_gettime(CLOCK_MONOTONIC, &endTime);                           \
                    struct timespec timeDiff = TimeDifference( startTime, endTime );    \
                    function_timer_log("%lld us",                                    \
                                        FUNC_NAME,                                   \
                                        ElapsedTimeInMicroseconds( timeDiff ));      \
                }                                                                       \
                while( 0 )
#else
    #define function_timer_log(M, N, ...)

    #define TIMEPLATFORM( FUNC, FUNC_NAME )                                             \
                do                                                                      \
                {                                                                       \
                    { FUNC; }                                                           \
                }                                                                       \
                while( 0 )
#endif


// ==== BRANCH PREDICTION & EXPRESSION EVALUATION ====
#if( !defined( unlikely ) )
    #define unlikely( EXPRESSSION )     __builtin_expect( !!(EXPRESSSION), 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check
    @abstract   Check that an expression is true (non-zero).
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method.

    Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check ) )
    #if( DEBUG )
        #define check( X )                                                                                  \
            do                                                                                              \
            {                                                                                               \
                if( unlikely( !(X) ) )                                                                      \
                {                                                                                           \
                    debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __PRETTY_FUNCTION__ );             \
                }                                                                                           \
                                                                                                            \
            }   while( 0 )
    #else
        #define check( X )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require
    @abstract   Requires that an expression evaluate to true.
    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require ) )
    #define require( X, LABEL )                                                                             \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __PRETTY_FUNCTION__ );                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_string
    @abstract   Requires that an expression evaluate to true with an explanation.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) and a custom explanation string using the default debugging output method then jumps to a label.
*/

#if( !defined( require_string ) )
    #define require_string( X, LABEL, STR )                                                                 \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, __FILE__, __LINE__, __PRETTY_FUNCTION__ );                  \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_quiet
    @abstract   Requires that an expression evaluate to true.
    @discussion
    
    If expression evalulates to false, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_quiet ) )
    #define require_quiet( X, LABEL )                                                                       \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr ) )
    #define require_noerr( ERR, LABEL )                                                                     \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __PRETTY_FUNCTION__ );        \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_string
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.), and a custom explanation string using the default debugging output method using the 
    default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr_string ) )
    #define require_noerr_string( ERR, LABEL, STR )                                                         \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __PRETTY_FUNCTION__ );         \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_string
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.), and a custom explanation string using the default debugging output method using the 
    default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action_string ) )
    #define require_noerr_action_string( ERR, LABEL, ACTION, STR )                                          \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __PRETTY_FUNCTION__ );         \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_quiet
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_quiet ) )
    #define require_noerr_quiet( ERR, LABEL )                                                               \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action ) )
    #define require_noerr_action( ERR, LABEL, ACTION )                                                      \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __PRETTY_FUNCTION__ );        \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_quiet
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion
    
    If the error code is non-0, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_action_quiet ) )
    #define require_noerr_action_quiet( ERR, LABEL, ACTION )                                                \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_action ) )
    #define require_action( X, LABEL, ACTION )                                                              \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __PRETTY_FUNCTION__ );                 \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_string
    @abstract   Requires that an expression evaluate to true with an explanation and action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) and a custom explanation string using the default debugging output method then executes an
    action and jumps to a label.
*/

#if( !defined( require_action_string ) )
    #define require_action_string( X, LABEL, ACTION, STR )                                                  \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, __FILE__, __LINE__, __PRETTY_FUNCTION__ );                  \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_quiet
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_action_quiet ) )
    #define require_action_quiet( X, LABEL, ACTION )                                                        \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )
#endif

// ==== ERROR MAPPING ====
#define global_value_errno( VALUE )                 ( errno ? errno : kUnknownErr )

#define map_global_value_errno( TEST, VALUE )       ( (TEST) ? 0 : global_value_errno(VALUE) )
#define map_global_noerr_errno( ERR )               ( !(ERR) ? 0 : global_value_errno(ERR) )
#define map_fd_creation_errno( FD )                 ( IsValidFD( FD ) ? 0 : global_value_errno( FD ) )
#define map_noerr_errno( ERR )                      map_global_noerr_errno( (ERR) )
   
#define socket_errno( SOCK )                        ( errno ? errno : kUnknownErr )
#define socket_value_errno( SOCK, VALUE )           socket_errno( SOCK )
#define map_socket_value_errno( SOCK, TEST, VALUE ) ( (TEST) ? 0 : socket_value_errno( (SOCK), (VALUE) ) ) 
#define map_socket_noerr_errno( SOCK, ERR )         ( !(ERR) ? 0 : socket_errno( (SOCK) ) )


//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check_ptr_overlap
    @abstract   Checks that two ptrs do not overlap.
*/

#define check_ptr_overlap( P1, P1_SIZE, P2, P2_SIZE )                                   \
    do                                                                                  \
    {                                                                                   \
        check( !( ( (uintptr_t)(P1) >=     (uintptr_t)(P2) ) &&                         \
                  ( (uintptr_t)(P1) <  ( ( (uintptr_t)(P2) ) + (P2_SIZE) ) ) ) );       \
        check( !( ( (uintptr_t)(P2) >=     (uintptr_t)(P1) ) &&                         \
                  ( (uintptr_t)(P2) <  ( ( (uintptr_t)(P1) ) + (P1_SIZE) ) ) ) );       \
                                                                                        \
    }   while( 0 )

#define IsValidFD( X )              ( ( X ) >= 0 )

#endif // __Debug_h__


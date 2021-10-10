/*
    File:    WACServer.c
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
#include "HTTPUtils.h"
#include "TimeUtils.h"
#include "TLVUtils.h"
#include "AppleDeviceIE.h"

#include "dns_sd.h"

#include "WACServerAPI.h"
#include "WACLogging.h"
#include "WACServerVersion.h"
#include "WACTLV.h"
#include "WACBonjour.h"

#include "PlatformSoftwareAccessPoint.h"
#include "PlatformApplyConfiguration.h"
#include "PlatformBonjour.h"
#include "PlatformMFiAuth.h"

// Platform parameter check helpers
static const char* const WACPlatformParameterErrorString = "ERROR: The platform must ";
#define CheckParameterAndReportError(X,Y,Z)                     \
    if( unlikely( !(X) ) )                                      \
    {                                                           \
        Y = true;                                               \
        wac_log("%s%s", WACPlatformParameterErrorString, Z);    \
    }

// WAC HTTP messages
#define kWACURLAuth          "/auth-setup"
#define kWACURLConfig        "/config"
#define kWACURLConfigured    "/configured"

typedef enum
{
    eState_Initialize                   = 0,
    eState_WaitingForAuthSetupMessage   = 1,
    eState_HandleAuthSetupMessage       = 2,
    eState_WaitingForConfigMessage      = 3,
    eState_HandleConfigMessage          = 4,
    eState_WaitingForConfiguredMessage  = 5,
    eState_HandleConfiguredMessage      = 6,
    eState_Complete                     = 7,
    eState_Cleanup                      = 8

} _WACState_t;

#define _WACStateToString( X ) ( \
    ( (X) == eState_Initialize )                      ? "Initialize"                              : \
    ( (X) == eState_WaitingForAuthSetupMessage )      ? "Waiting for auth setup message"          : \
    ( (X) == eState_HandleAuthSetupMessage )          ? "Handle auth setup message"               : \
    ( (X) == eState_WaitingForConfigMessage )         ? "Waiting for config message"              : \
    ( (X) == eState_HandleConfigMessage )             ? "Handle config message"                   : \
    ( (X) == eState_WaitingForConfiguredMessage )     ? "Waiting for configured message"          : \
    ( (X) == eState_HandleConfiguredMessage )         ? "Handle configured message"               : \
    ( (X) == eState_Complete )                        ? "Complete"                                : \
    ( (X) == eState_Cleanup )                         ? "Cleanup"                                 : \
                                                        "?" )

// ==== STATIC GLOBALS ====
static bool     _gWACServerRunning;
static uint8_t  _gBonjourSeedID;

// ==== THREADS ====
void *_WACServerEngine( void * inContext );

// ==== HANDLE WAC STATE PROTOS ====
static OSStatus _HandleState_Initialize                    ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_WaitingForAuthSetupMessage    ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_HandleAuthSetupMessage        ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_WaitingForConfigMessage       ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_HandleConfigMessage           ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_WaitingForConfiguredMessage   ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_HandleConfiguredMessage       ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_Complete                      ( WACContext_t * const inContext, _WACState_t *inState );
static OSStatus _HandleState_Cleanup                       ( WACContext_t * const inContext, _WACState_t *inState );

// ==== HELPER FUNCTIONS ====
static OSStatus _CreateAppleDeviceIEFromPlatformParameters( const WACPlatformParameters_t * const inPlatformParams,
                                                            uint8_t **outIEBuffer,
                                                            size_t *outIEBufferLen );

static OSStatus _ParseTLVConfigMessage( const void * const inTLVPtr,
                                        const size_t inTLVLen,
                                        char **outSSID,
                                        char **outPSK,
                                        char **outName,
                                        char **outPlayPassword );

static OSStatus _CreateTLVConfigResponseMessage( const WACPlatformParameters_t * const inPlatformParams,
                                                 uint8_t **outTLVResponse,
                                                 size_t *outTLVResponseLen );

// ==== CALLBACK FUNCTIONS ====
void _HandleHTTPServerCallback( HTTPHeader_t *inHeader, bool stopped, void *callbackContext );

// ==== DEBUG PROTOS ====
static void _PrintVersionInformation( void );

static void _PrintPlatformParams( const WACPlatformParameters_t * const inPlatformParams );

// ==== PLATFORM PARAMETER PROTOS ====
static OSStatus _CheckPlatformParams( const WACPlatformParameters_t * const inPlatformParams );


OSStatus WACServerStart( WACContext_t * const inContext, void *WACPlatformCallback )
{
    Trace();
    _PrintVersionInformation();

    OSStatus err = kUnknownErr;

    require_action_quiet( !_gWACServerRunning, exit, err = kAlreadyInitializedErr );

    _gWACServerRunning = true;

    // Initialize the HTTP Read semaphore
    err = sem_init( &inContext->httpHeaderReadSemaphore, // sem_t*
                    0,                                   // 1=shared, 0=not shared
                    0 );                                 // init count (posix semaphores count up, 0 blocks)
    require_noerr( err, exit );
    inContext->httpHeaderReadSemaphoreValid = true;

    // Initialize the HTTP Stopped semaphore
    err = sem_init( &inContext->httpServerStoppedSemaphore, // sem_t*
                    0,                                      // 1=shared, 0=not shared
                    0 );                                    // init count (posix semaphores count up, 0 blocks)
    require_noerr( err, exit );
    inContext->httpServerStoppedSemaphoreValid = true;

    // Set flags
    inContext->shouldStop = false;
    inContext->httpHeaderReadSemaphoreValid = false;
    inContext->httpServerStoppedSemaphoreValid = false;

    // Assign callback function
    inContext->callback = NULL;
    if ( WACPlatformCallback ) inContext->callback = WACPlatformCallback;
printf("_CheckPlatformParams \n");
    // Check the incoming platform paramters
    err = _CheckPlatformParams( inContext->platformParams );
    require_noerr( err, exit );
printf("_PrintPlatformParams \n");    
    _PrintPlatformParams( inContext->platformParams );

    // Start the WACServerEngine thread
    err = pthread_create( &inContext->serverEngineThread, NULL, _WACServerEngine, (void*)inContext );
    require_noerr_action( err, exit, wac_log("ERROR: Unable to start the WACServerEngine thread.") );

exit:
    return err;
}

OSStatus WACServerStop( WACContext_t * const inContext )
{
    Trace();

    inContext->shouldStop = true;

    sem_post( &inContext->httpHeaderReadSemaphore );
    // This function will block until the WAC engine has been cleaned up
    pthread_join( inContext->serverEngineThread, NULL );

    // Reset static global instance check
    _gWACServerRunning = false;

    return kNoErr;
}

static OSStatus _CheckPlatformParams ( const WACPlatformParameters_t * const inPlatformParams )
{
    Trace();
    OSStatus err        = kParamErr;
    bool     errorFound = false;

    require( inPlatformParams, exit );
    // WAC currently requires that the device be unconfigured
    CheckParameterAndReportError( inPlatformParams->isUnconfigured, errorFound, "be unconfigured to start WAC" );

    // The platform must support at least one 802.11 frequency
    CheckParameterAndReportError( inPlatformParams->supports2_4GHzWiFi || inPlatformParams->supports5GHzWiFi,
                                  errorFound,
                                  "support at least one 802.11 frequency." );

    CheckParameterAndReportError( inPlatformParams->firmwareRevision && strlen( inPlatformParams->firmwareRevision ),
                                  errorFound,
                                  "specify a FW revision.");

    CheckParameterAndReportError( inPlatformParams->hardwareRevision && strlen( inPlatformParams->hardwareRevision ),
                                  errorFound,
                                  "specify a HW revision.");

    CheckParameterAndReportError( inPlatformParams->name && strlen( inPlatformParams->name ),
                                  errorFound,
                                  "specify a name.");

    CheckParameterAndReportError( inPlatformParams->model && strlen( inPlatformParams->model ),
                                  errorFound,
                                  "specify a model.");

    CheckParameterAndReportError( inPlatformParams->manufacturer && strlen( inPlatformParams->manufacturer ),
                                  errorFound,
                                  "specify a manufacturer.");

/*
    if ( *inPlatformParams->supportedExternalAccessoryProtocols )
    {

        CheckParameterAndReportError( strlen( *inPlatformParams->supportedExternalAccessoryProtocols ),
                                      errorFound,
                                      "have EA protocols of length > 0 if the supportedExternalAccessoryProtocols pointer is non-NULL.");
        CheckParameterAndReportError( inPlatformParams->numSupportedExternalAccessoryProtocols > 0,
                                      errorFound,
                                      "specify the number of EA protocols if the supportedExternalAccessoryProtocols pointer is non-NULL.");
    }
    */

exit:
    if ( !errorFound ) err = kNoErr;
    return err;
}

void *_WACServerEngine( void *inContext )
{
    Trace();
    OSStatus    err     = kUnknownErr;
    _WACState_t state   = eState_Initialize;

    while ( ( state != eState_Cleanup ) && ( !((WACContext_t*)inContext)->shouldStop ) )
    {
        switch ( state )
        {
            case eState_Initialize:
                err = _HandleState_Initialize( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_WaitingForAuthSetupMessage:
                err = _HandleState_WaitingForAuthSetupMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_HandleAuthSetupMessage:
                err = _HandleState_HandleAuthSetupMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_WaitingForConfigMessage:
                err = _HandleState_WaitingForConfigMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_HandleConfigMessage:
                err = _HandleState_HandleConfigMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_WaitingForConfiguredMessage:
                err = _HandleState_WaitingForConfiguredMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_HandleConfiguredMessage:
                err = _HandleState_HandleConfiguredMessage( inContext, &state );
                require_noerr( err, exit );
            break;

            case eState_Complete:
                err = _HandleState_Complete( inContext, &state );
                require_noerr( err, exit );
            break;

            default:

            break;
        }
    }

exit:
    _HandleState_Cleanup( inContext, &state );
    return NULL;
}

static OSStatus _HandleState_Initialize( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kUnknownErr;

    uint8_t *ie = NULL;
    size_t  ieLen;

    require_action( inContext, exit, err = kParamErr );

    inContext->dnsServiceRef = NULL;

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_Initializing );

    // Initialize the MFi Platform
    err = PlatformMFiAuthInitialize();
    require_noerr( err, exit );

    err = _CreateAppleDeviceIEFromPlatformParameters( inContext->platformParams, &ie, &ieLen );
    require_noerr( err, exit );

    // Start Platform Software Access Point
    TIMEPLATFORM( err = PlatformSoftwareAccessPointStart( ie, ieLen ), "PlatformSoftwareAccessPointStart" );
    require_noerr( err, exit );
    wac_log("Platform Software Access Point started");

    // Create HTTP Server
    inContext->httpServer = malloc( sizeof( HTTPServer_t ) );

    err = HTTPServerCreate( "WACPreConfigListener",     // Friendly name of server
                            _HandleHTTPServerCallback,  // Callback function
                            (void*)inContext,           // Reference to the WACContext_t
                            inContext->httpServer );    // Reference to alloc'd HTTPServer_t
    require_noerr( err, exit );

    // Start HTTP Server
    err = HTTPServerStart( inContext->httpServer );
    require_noerr( err, exit );
    wac_log("HTTPServer started");

    // Start mDNSResponder on platform
    TIMEPLATFORM( err = PlatformInitializemDNSResponder(), "PlatformInitializemDNSResponder" );
    require_noerr( err, exit );

    // Register the WAC bonjour service
    _gBonjourSeedID = 1;
    err = RegisterWACBonjourService( inContext, _gBonjourSeedID );
    require_noerr( err, exit );
    wac_log("Bonjour service registered with seed: %d", _gBonjourSeedID);

    *inState = eState_WaitingForAuthSetupMessage;

exit:
    FreeSafe( ie );
    return err;
}

static OSStatus _HandleState_WaitingForAuthSetupMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kUnknownErr;

    require( inContext, exit );

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_Ready );

    // Wait on semaphore for the /auth message
    sem_wait( &inContext->httpHeaderReadSemaphore );
    require_action_quiet( !inContext->shouldStop, exit, *inState = eState_Cleanup; err = kNoErr; );

    // Check that the HTTP URL is the auth setup URL
    err = HTTPHeaderMatchURL( inContext->httpHeader, kWACURLAuth );
    require_noerr_action( err, exit, wac_log("ERROR: received different URL. Expecting: %s ", kWACURLAuth); err = kOrderErr );

    wac_log("%s received", kWACURLAuth);

    *inState = eState_HandleAuthSetupMessage;

exit:
    return err;
}

static OSStatus _HandleState_HandleAuthSetupMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kUnknownErr;
    Boolean mfiSAPComplete;

    uint8_t *httpResponse = NULL;
    size_t httpResponseLen = 0;

    uint8_t *mfiSAPResponseDataPtr = NULL;
    size_t mfiSAPResponseDataLen = 0;

    require( inContext, exit );

    // Create MFiSAP
    err = MFiSAP_Create( &inContext->mfiSAPRef, kMFiSAPVersion1 );
    require_noerr( err, exit );

    // Do MFiSAP Exchange
    err = MFiSAP_Exchange( inContext->mfiSAPRef,                            // MFiSAPRef
                           (uint8_t*)inContext->httpHeader->extraDataPtr,   // data from auth request
                           inContext->httpHeader->extraDataLen,             // length of data from auth request
                           &mfiSAPResponseDataPtr,                          // exchange data that should be sent to client
                           &mfiSAPResponseDataLen,                          // length of exchange data that should be sent to client
                           &mfiSAPComplete );                               // Boolean to see if MFiSAP is complete
    require_noerr_action( err, exit, wac_log("ERROR: MFi-SAP Exchange: %d", err) );
    require( mfiSAPComplete, exit );

    err =  CreateSimpleHTTPMessage( kMIMEType_Binary, mfiSAPResponseDataPtr, mfiSAPResponseDataLen, &httpResponse, &httpResponseLen );
    require_noerr( err, exit );
    require( httpResponse, exit );

    err = HTTPServerSend( inContext->httpServer, httpResponse, httpResponseLen );
    require_noerr( err, exit );
    wac_log("Auth response sent");

    *inState = eState_WaitingForConfigMessage;

exit:
    FreeSafe( mfiSAPResponseDataPtr );
    FreeSafe( httpResponse );
    return err;
}

static OSStatus _HandleState_WaitingForConfigMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kNoErr;

    // Wait on semaphore for the /config message
    sem_wait( &inContext->httpHeaderReadSemaphore );
    require_action_quiet( !inContext->shouldStop, exit, *inState = eState_Cleanup; err = kNoErr; );

    // Check that the HTTP URL is the config URL
    err = HTTPHeaderMatchURL( inContext->httpHeader, kWACURLConfig );
    require_noerr_action( err, exit, wac_log("ERROR: received different URL. Expecting: %s", kWACURLConfig); err = kOrderErr );

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_ConfigStart );

    wac_log("%s received", kWACURLConfig);

    *inState = eState_HandleConfigMessage;

exit:
    return err;
}

static OSStatus _HandleState_HandleConfigMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kParamErr;

    uint8_t *httpResponse = NULL;
    size_t httpResponseLen = 0;

    char *destinationSSID = NULL;
    char *destinationPSK  = NULL;
    char *accessoryName   = NULL;
    char *playPassword    = NULL;

    require( inContext, exit );

    // Remove the WAC bonjour service
    err = RemoveWACBonjourService( inContext );
    require_noerr( err, exit );

    // Decrypt the /config message
    uint8_t *decryptedConfigData = malloc( inContext->httpHeader->extraDataLen );
    require_action( decryptedConfigData, exit, err = kNoMemoryErr );
    err = MFiSAP_Decrypt( inContext->mfiSAPRef,                 // MFiSAPRef
                          inContext->httpHeader->extraDataPtr,  // Data to decrypt
                          inContext->httpHeader->extraDataLen,  // Length of data to decrpyt
                          decryptedConfigData );                // Decrypted data destination pointer
    require_noerr( err, exit );

    // Parse the /config message, TLV format
    err = _ParseTLVConfigMessage( decryptedConfigData,                  // Data pointer to parse
                                  inContext->httpHeader->extraDataLen,  // Data length
                                  &destinationSSID,                     // Pointer to SSID c string
                                  &destinationPSK,                      // Pointer to PSK c string
                                  &accessoryName,                       // Pointer to name c string
                                  &playPassword );                      // Pointer to name c string
    FreeSafe( decryptedConfigData );
    require_noerr_action( err, exit, err = kResponseErr );
    require_action( destinationSSID, exit, err = kResponseErr );

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_ConfigReceived );

    // If we have an EA response to send
    if ( inContext->platformParams->numSupportedExternalAccessoryProtocols )
    {
        // Create the TLV response
        uint8_t *eaTLVResponse;
        size_t  eaTLVResponseLen;
        err = _CreateTLVConfigResponseMessage( inContext->platformParams, &eaTLVResponse, &eaTLVResponseLen );
        require_noerr( err, exit );
        require_action( eaTLVResponse, exit, err = kNoMemoryErr );
        require_action( eaTLVResponseLen, exit, err = kNoMemoryErr );

        // Encrypt the TLV response
        uint8_t *encryptedConfigData = malloc( eaTLVResponseLen * sizeof( uint8_t ) );
        require( encryptedConfigData, exit );

        err = MFiSAP_Encrypt( inContext->mfiSAPRef,     // MFiSAPRef
                              eaTLVResponse,            // Data to encrypt
                              eaTLVResponseLen,         // Length of data to encrypt
                              encryptedConfigData );    // Encrypted data destination pointer

        err = CreateSimpleHTTPMessage( kMIMEType_TLV8,      // MIME type of message
                                       encryptedConfigData, // encrypted data to send
                                       eaTLVResponseLen,    // length of data to send
                                       &httpResponse,       // pointer to http response
                                       &httpResponseLen );  // length of http response

        FreeSafe( encryptedConfigData );
        FreeSafe( eaTLVResponse );
        wac_log("Sending EA /config response");
    }
    else
    {
        err = CreateSimpleHTTPOKMessage( &httpResponse, &httpResponseLen );
        wac_log("Sending simple (non-EA) /config response");
    }
    require_noerr( err, exit );

    err = HTTPServerSend( inContext->httpServer, httpResponse, httpResponseLen );
    require_noerr( err, exit );
    wac_log("Config response sent");

    // Stop the HTTP server, we're done
    err = HTTPServerStop( inContext->httpServer );

    // Wait on semaphore for the HTTP Server to stop
    sem_wait( &inContext->httpServerStoppedSemaphore );

    // Stop the Software Access Point
    TIMEPLATFORM( err = PlatformSoftwareAccessPointStop(), "PlatformSoftwareAccessPointStop" );
    require_noerr( err, exit );

    // Apply name from configuration
    if ( accessoryName )
    {
        TIMEPLATFORM( err = PlatformApplyName( accessoryName ), "PlatformApplyName" );
        require_noerr( err, exit );
    }

    // Apply play password from configuration
    if ( playPassword && inContext->platformParams->supportsAirPlay )
    {
        TIMEPLATFORM( err = PlatformApplyAirPlayPlayPassword( playPassword ), "PlatformApplyAirPlayPlayPassword" );
        require_noerr( err, exit );
    }
    else if ( playPassword && !inContext->platformParams->supportsAirPlay )
    {
        wac_log("WARNING: Received AirPlay Play Password on non-AirPlay device.");
    }

    // Join destination Wi-Fi network by SSID with PSK when applicable
    TIMEPLATFORM( err = PlatformJoinDestinationWiFiNetwork( destinationSSID,
                                                            ( destinationPSK ? (uint8_t*)destinationPSK : NULL ),
                                                            ( destinationPSK ? strlen( destinationPSK ) : 0 ) ),
                  "PlatformJoinDestinationWiFiNetwork" );

    require_noerr( err, exit );

    *inState = eState_WaitingForConfiguredMessage;

exit:
    FreeSafe( inContext->httpServer );
    FreeSafe( httpResponse );
    FreeSafe( destinationSSID );
    FreeSafe( destinationPSK );
    FreeSafe( accessoryName );
    FreeSafe( playPassword );

    return err;
}

static OSStatus _HandleState_WaitingForConfiguredMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kNoErr;

    require( inContext, exit );

    // Start fresh HTTP Server on destination network
    inContext->httpServer = malloc( sizeof( HTTPServer_t ) );

    err = HTTPServerCreate( "WACPreConfigListener",     // Friendly name of server
                            _HandleHTTPServerCallback,  // Callback function
                            (void*)inContext,           // Reference to the WACContext_t
                            inContext->httpServer );    // Reference to alloc'd HTTPServer_t
    require_noerr( err, exit );

    err = HTTPServerStart( inContext->httpServer );
    require_noerr( err, exit );
    wac_log("HTTPServer started for post config");

    // Increment the bonjour seed ID
    _gBonjourSeedID++;

    // Register the WAC bonjour service
    err = RegisterWACBonjourService( inContext, _gBonjourSeedID );
    require_noerr( err, exit );
    wac_log("Bonjour service registered with seed: %d", _gBonjourSeedID);

    // Wait on semaphore for the /configured message
    sem_wait( &inContext->httpHeaderReadSemaphore );
    require_action_quiet( !inContext->shouldStop, exit, *inState = eState_Cleanup; err = kNoErr; );

    // Check that the HTTP URL is the configured URL
    err = HTTPHeaderMatchURL( inContext->httpHeader, kWACURLConfigured );
    require_noerr_action( err, exit, wac_log("ERROR: received different URL. Expecting: %s", kWACURLConfigured); err = kOrderErr );

    wac_log("%s received", kWACURLConfigured);

    *inState = eState_HandleConfiguredMessage;

exit:
    return err;
}

static OSStatus _HandleState_HandleConfiguredMessage( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kUnknownErr;

    uint8_t *httpResponse = NULL;
    size_t httpResponseLen = 0;

    require( inContext, exit );

    // Remove the WAC bonjour service
    err = RemoveWACBonjourService( inContext );
    require_noerr( err, exit );

    err = CreateSimpleHTTPOKMessage( &httpResponse, &httpResponseLen );
    require_noerr( err, exit );

    err = HTTPServerSend( inContext->httpServer, httpResponse, httpResponseLen );
    require_noerr( err, exit );
    wac_log("Configured response sent");

    // Stop the HTTPServer
    err = HTTPServerStop( inContext->httpServer );
    require_noerr( err, exit );

    // Wait on semaphore for the HTTP Server to stop
    sem_wait( &inContext->httpServerStoppedSemaphore );

    *inState = eState_Complete;

exit:
    FreeSafe( httpResponse );
    return err;
}

static OSStatus _HandleState_Complete( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kNoErr;

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_ConfigComplete );

    *inState = eState_Cleanup;

    return err;
}

static OSStatus _HandleState_Cleanup( WACContext_t * const inContext, _WACState_t *inState )
{
    Trace();
    OSStatus err = kInternalErr;

    require( inContext, exit );

    // Unregister bonjour record
    RemoveWACBonjourService( inContext );

    // Stop mDNSResponder
    PlatformMayStopmDNSResponder();

    // Stop HTTPServer
    if ( inContext->httpServer ) HTTPServerStop( inContext->httpServer );
    FreeSafe( inContext->httpServer );

    // Finalize MFi Platform
    PlatformMFiAuthFinalize();

    // Destroy semaphores
    if ( inContext->httpHeaderReadSemaphoreValid )
    {
        sem_destroy( &inContext->httpHeaderReadSemaphore );
    }

    if ( inContext->httpServerStoppedSemaphoreValid )
    {
        sem_destroy( &inContext->httpServerStoppedSemaphore );
    }

exit:
    // Final State, no progression
    *inState = eState_Cleanup;

    if ( inContext->callback ) inContext->callback( inContext, WACCallbackMessage_Stopped );

    return err;
}

// ==== CALLBACK FUNCTIONS ====

void _HandleHTTPServerCallback( HTTPHeader_t *inHeader, bool stopped, void *callbackContext )
{
    Trace();
    OSStatus err = kInternalErr;

    require( callbackContext, exit );
    WACContext_t *inContext = callbackContext;

    // NOTE: Make sure to return quickly as to not block the HTTPServer thread
    if ( !stopped )
    {
        require( inHeader, exit );

        // Check for 'POST' method
        err = HTTPHeaderMatchMethod( inHeader, kHTTPPostMethod );
        require_noerr_action_quiet( err, exit, wac_log("HTTP Request Not for WAC") );

        // Assign this header to our local HTTP Header
        inContext->httpHeader = inHeader;
        inContext->httpHeader = malloc( sizeof( HTTPHeader_t ) );
        require( inContext->httpHeader, exit );
        memcpy( inContext->httpHeader, inHeader, sizeof( HTTPHeader_t ) );

        // Signal the HTTP header read semaphore for processing
        err = sem_post( &inContext->httpHeaderReadSemaphore );
        require_noerr( err, exit );
    }
    else
    {
        // Signal the HTTP stopped semaphore for processing
        err = sem_post( &inContext->httpServerStoppedSemaphore );
        require_noerr( err, exit );
    }

exit:
    if ( err )
    {
        wac_log("ERROR: %d", err);
    }
    return;
}

// ==== HELPER FUNCTIONS ====

static OSStatus _CreateAppleDeviceIEFromPlatformParameters ( const WACPlatformParameters_t * const inPlatformParams, uint8_t **outIEBuffer, size_t *outIEBufferLen )
{
    Trace();
    return CreateAppleDeviceIE( inPlatformParams->name,
                                inPlatformParams->model,
                                inPlatformParams->manufacturer,
                                inPlatformParams->macAddress,
                                inPlatformParams->isUnconfigured,
                                inPlatformParams->supportsAirPlay,
                                inPlatformParams->supportsAirPrint,
                                true, // supportsMFiConfigurationV1
                                inPlatformParams->supportsWakeOnWireless,
                                inPlatformParams->supports2_4GHzWiFi,
                                inPlatformParams->supports5GHzWiFi,
                                outIEBuffer,
                                outIEBufferLen );
}

static OSStatus _ParseTLVConfigMessage( const void * const inTLVPtr,
                                        const size_t inTLVLen,
                                        char **outSSID,
                                        char **outPSK,
                                        char **outName,
                                        char **outPlayPassword )
{
    Trace();
    OSStatus                    err = kParamErr;
    const uint8_t *             src = (const uint8_t *) inTLVPtr;
    const uint8_t * const       end = src + inTLVLen;
    uint8_t                     eid;
    const uint8_t *             ptr;
    size_t                      len;
    char *                      tmp;

    require( inTLVPtr, exit );
    require( inTLVLen, exit );
    require( outSSID, exit );
    require( outPSK, exit );
    require( outName, exit );
    require( outPlayPassword, exit );

    while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
    {
        tmp = calloc( len + 1, sizeof( uint8_t ) );
        require_action( tmp, exit, err = kNoMemoryErr );
        memcpy( tmp, ptr, len );

        switch( eid )
        {
            case kWACTLV_Name:
                *outName = tmp;
                break;

            case kWACTLV_WiFiSSID:
                *outSSID = tmp;
                break;

            case kWACTLV_WiFiPSK:
                *outPSK = tmp;
                break;

            case kWACTLV_PlayPassword:
                *outPlayPassword = tmp;
                break;

            default:
                // Don't recognize the WAC EID
                FreeSafe( tmp );
                wac_log( "Warning: Ignoring unsupported WAC EID 0x%02X\n", eid );
                break;
        }
    }

    err = kNoErr;

exit:
    return err;
}

static OSStatus _CreateTLVConfigResponseMessage( const WACPlatformParameters_t * const inPlatformParams,
                                                 uint8_t **outTLVResponse,
                                                 size_t *outTLVResponseLen )
{
    Trace();
    OSStatus err = kParamErr;

    uint8_t *eaProtocolSizes                     = NULL;
    uint8_t nameSize                             = 0;
    uint8_t manufacturerSize                     = 0;
    uint8_t modelSize                            = 0;
    uint8_t firmwareRevisionSize                 = 0;
    uint8_t hardwareRevisionSize                 = 0;
    uint8_t serialNumberSize                     = 0;
    uint8_t preferredAppBundleSeedIdentifierSize = 0;

    *outTLVResponse = NULL;
    *outTLVResponseLen = 0;

    require( inPlatformParams, exit );

    if ( inPlatformParams->name )
    {
        nameSize = strnlen( inPlatformParams->name, kWACTLV_MaxStringSize );
        *outTLVResponseLen += nameSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->manufacturer )
    {
        manufacturerSize = strnlen( inPlatformParams->manufacturer, kWACTLV_MaxStringSize );
        *outTLVResponseLen += manufacturerSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->model )
    {
        modelSize = strnlen( inPlatformParams->model, kWACTLV_MaxStringSize );
        *outTLVResponseLen += modelSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->firmwareRevision )
    {
        firmwareRevisionSize = strnlen( inPlatformParams->firmwareRevision, kWACTLV_MaxStringSize );
        *outTLVResponseLen += firmwareRevisionSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->hardwareRevision )
    {
        hardwareRevisionSize = strnlen( inPlatformParams->hardwareRevision, kWACTLV_MaxStringSize );
        *outTLVResponseLen += hardwareRevisionSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->serialNumber )
    {
        serialNumberSize = strnlen( inPlatformParams->serialNumber, kWACTLV_MaxStringSize );
        *outTLVResponseLen += serialNumberSize + kWACTLV_TypeLengthSize;
    }

    if ( inPlatformParams->preferredAppBundleSeedIdentifier )
    {
        preferredAppBundleSeedIdentifierSize = strnlen( inPlatformParams->preferredAppBundleSeedIdentifier, kWACTLV_MaxStringSize );
        *outTLVResponseLen += preferredAppBundleSeedIdentifierSize + kWACTLV_TypeLengthSize;
    }

    eaProtocolSizes = calloc( inPlatformParams->numSupportedExternalAccessoryProtocols, sizeof( uint32_t ) );
    require_action( eaProtocolSizes, exit, err = kNoMemoryErr );

    uint8_t protocolIndex;
    for( protocolIndex = 0; protocolIndex < inPlatformParams->numSupportedExternalAccessoryProtocols; protocolIndex++ )
    {
        eaProtocolSizes[protocolIndex] = strnlen( inPlatformParams->supportedExternalAccessoryProtocols[protocolIndex], kWACTLV_MaxStringSize );
        *outTLVResponseLen += eaProtocolSizes[protocolIndex] + kWACTLV_TypeLengthSize;
    }

    // Allocate space for the entire TLV
    *outTLVResponse = calloc( *outTLVResponseLen, sizeof( uint8_t ) );
    require_action( *outTLVResponse, exit, err = kNoMemoryErr );

    uint8_t *tlvPtr = *outTLVResponse;

    // Accessory Name
    if ( nameSize )
    {
        *tlvPtr++ = kWACTLV_Name;
        *tlvPtr++ = nameSize;
        memcpy( tlvPtr, inPlatformParams->name, nameSize );
        tlvPtr += nameSize;
    }

    // Accessory Manufacturer
    if ( manufacturerSize )
    {
        *tlvPtr++ = kWACTLV_Manufacturer;
        *tlvPtr++ = manufacturerSize;
        memcpy( tlvPtr, inPlatformParams->manufacturer, manufacturerSize );
        tlvPtr += manufacturerSize;
    }

    // Accessory Model
    if ( modelSize )
    {
        *tlvPtr++ = kWACTLV_Model;
        *tlvPtr++ = modelSize;
        memcpy( tlvPtr, inPlatformParams->model, modelSize );
        tlvPtr += modelSize;
    }

    // Serial Number
    if ( serialNumberSize )
    {
        *tlvPtr++ = kWACTLV_SerialNumber;
        *tlvPtr++ = serialNumberSize;
        memcpy( tlvPtr, inPlatformParams->serialNumber, serialNumberSize );
        tlvPtr += serialNumberSize;
    }

    // Firmware Revision
    if ( firmwareRevisionSize )
    {
        *tlvPtr++ = kWACTLV_FirmwareRevision;
        *tlvPtr++ = firmwareRevisionSize;
        memcpy( tlvPtr, inPlatformParams->firmwareRevision, firmwareRevisionSize );
        tlvPtr += firmwareRevisionSize;
    }

    // Hardware Revision
    if ( hardwareRevisionSize )
    {
        *tlvPtr++ = kWACTLV_HardwareRevision;
        *tlvPtr++ = hardwareRevisionSize;
        memcpy( tlvPtr, inPlatformParams->hardwareRevision, hardwareRevisionSize );
        tlvPtr += hardwareRevisionSize;
    }

    // EA Protocols
    for( protocolIndex = 0; protocolIndex < inPlatformParams->numSupportedExternalAccessoryProtocols; protocolIndex++ )
    {
        *tlvPtr++ = kWACTLV_MFiProtocol;
        *tlvPtr++ = eaProtocolSizes[protocolIndex];
        memcpy( tlvPtr, inPlatformParams->supportedExternalAccessoryProtocols[protocolIndex], eaProtocolSizes[protocolIndex] );
        tlvPtr += eaProtocolSizes[protocolIndex];
    }

    // BundleSeedID
    if ( preferredAppBundleSeedIdentifierSize )
    {
        *tlvPtr++ = kWACTLV_PreferredAppBundleSeedIdentifier;
        *tlvPtr++ = preferredAppBundleSeedIdentifierSize;
        memcpy( tlvPtr, inPlatformParams->preferredAppBundleSeedIdentifier, preferredAppBundleSeedIdentifierSize );
        tlvPtr += preferredAppBundleSeedIdentifierSize;
    }

    require_action( ( tlvPtr - *outTLVResponseLen ) == *outTLVResponse, exit, err = kSizeErr );

    err = kNoErr;

exit:
    if ( err ) FreeSafe( *outTLVResponse );
    FreeSafe( eaProtocolSizes );

    return err;
}

static void _PrintVersionInformation ( void )
{
    Trace();
    printf("\n###################################\n");
    printf("# WAC SERVER v%d.%d\n", kWACServerVersionMajor, kWACServerVersionMinor);
#ifdef GITVERSION
    printf("# GIT VERSION: %s\n", GITVERSION);
#endif
    printf("# BUILD DATE: %s, %s\n", kBuildDate, kBuildTime);
    printf("###################################\n\n");
}

static void _PrintPlatformParams( const WACPlatformParameters_t * const inPlatformParams )
{
    Trace();
    wac_log("************************************************");
    wac_log("PLATFORM ATTRIBUTES:");
    wac_log("    NAME.........: %s", inPlatformParams->name);
    wac_log("    MODEL........: %s", inPlatformParams->model);
    wac_log("    MANUFACTURER.: %s", inPlatformParams->manufacturer);
    wac_log("    FW REV.......: %s", inPlatformParams->firmwareRevision);
    wac_log("    HW REV.......: %s", inPlatformParams->hardwareRevision);
    wac_log("    SERIAL NUMBER: %s", inPlatformParams->serialNumber);
    wac_log("    UNCONFIGURED.: %s", YesOrNo(inPlatformParams->isUnconfigured));

    wac_log("PLATFORM FEATURES:");
    wac_log("    AIRPLAY.....: %s", YesOrNo(inPlatformParams->supportsAirPlay));
    wac_log("    AIRPRINT....: %s", YesOrNo(inPlatformParams->supportsAirPrint));
    wac_log("    2.4GHz Wi-Fi: %s", YesOrNo(inPlatformParams->supports2_4GHzWiFi));
    wac_log("    5.0GHz Wi-Fi: %s", YesOrNo(inPlatformParams->supports5GHzWiFi));
    wac_log("    WoW.........: %s", YesOrNo(inPlatformParams->supportsWakeOnWireless));

    wac_log("PLATFORM EA INFO:");
    wac_log("    BUNDLE SEED ID: %s", inPlatformParams->preferredAppBundleSeedIdentifier);
    uint8_t protocolIndex;
    for( protocolIndex = 0; protocolIndex < inPlatformParams->numSupportedExternalAccessoryProtocols; protocolIndex++ )
    {
        wac_log("    PROTOCOL %2d...: %s", protocolIndex, inPlatformParams->supportedExternalAccessoryProtocols[protocolIndex]);
    }
    wac_log("    NUM PROTOCOLS.: %d", inPlatformParams->numSupportedExternalAccessoryProtocols);

    wac_log("PLATFORM NETWORK INFO:");
    char* macAddressCString = DataToHexStringWithColons( inPlatformParams->macAddress, 6 );
    wac_log("    MAC ADDRESS: %s", macAddressCString );
    FreeSafe( macAddressCString );
    wac_log("************************************************");

    return;
}


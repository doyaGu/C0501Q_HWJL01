/*
    File:    WACBonjour.c
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

#include "WACBonjour.h"
#include "Debug.h"
#include "WACLogging.h"
#include "WACServerVersion.h"

#include "StringUtils.h"

#include "dns_sd.h"

#include <arpa/inet.h>


// ==== MFi / WAC BONJOUR DEFINES ====
#define kMFiConfigServiceType               "_mfi-config._tcp"
#define kMFiConfigServiceDomain             "local"
#define kMFiConfigTXTRecordKeyDeviceID      "deviceid"
#define kMFiConfigTXTRecordKeySeed          "seed"
#define kMFiConfigTXTRecordKeyFlags         "flags"
#define kMFiConfigTXTRecordKeyFeatures      "features"
#define kMFiConfigTXTRecordKeySourceVersion "srcvers"

#define kMFiConfigFeatureAccessoryHasApp    0x01
#define kMFiConfigFeatureSupportsTLV        0x04


OSStatus RegisterWACBonjourService( WACContext_t * const inContext, uint8_t inSeed )
{
    Trace();
    OSStatus err = kParamErr;

    TXTRecordRef    txtRecord;
    size_t          seedStringMaxLength = 3;
    char            seedString[ seedStringMaxLength + 1 ];
    size_t          featureStringMaxLength = 16;
    char            featureString[ featureStringMaxLength + 1 ];
    size_t          sourceVersionStringMaxLength = 16;
    char            sourceVersionString[ sourceVersionStringMaxLength + 1 ];
    uint8_t         features = kMFiConfigFeatureSupportsTLV;

    require( inContext, exit );

    // Print seed to string for bonjour TXT record
    snprintf( seedString, seedStringMaxLength, "%d", inSeed );

    if ( inContext->platformParams->numSupportedExternalAccessoryProtocols ) features += kMFiConfigFeatureAccessoryHasApp;

    // Print features to string for bonjour TXT record
    snprintf( featureString, featureStringMaxLength, "%d", features );

    // Print source version to string for bonjour TXT record
    snprintf( sourceVersionString, sourceVersionStringMaxLength, "%d.%d", kWACServerVersionMajor, kWACServerVersionMinor );

    // Create TXT Record
    TXTRecordCreate( &txtRecord, 0, NULL );

    // Add device ID (MAC address) to the bonjour TXT record
    char *txtMACAddress = DataToHexStringWithColons( inContext->platformParams->macAddress, 6 );
    require( txtMACAddress, exit );
    err = TXTRecordSetValue( &txtRecord,
                             kMFiConfigTXTRecordKeyDeviceID,
                             strlen( txtMACAddress ),
                             txtMACAddress );
    FreeSafe( txtMACAddress );
    require_noerr( err, exit );

    // Add seed ID to the bonjour TXT record
    err = TXTRecordSetValue( &txtRecord,
                             kMFiConfigTXTRecordKeySeed,
                             strlen( seedString ),
                             seedString );
    require_noerr( err, exit );

    // Add feature bit mask to the bonjour TXT record
    err = TXTRecordSetValue( &txtRecord,
                             kMFiConfigTXTRecordKeyFeatures,
                             strlen( featureString ),
                             featureString );
    require_noerr( err, exit );

    // Add source version to the bonjour TXT record
    err = TXTRecordSetValue( &txtRecord,
                             kMFiConfigTXTRecordKeySourceVersion,
                             strlen( sourceVersionString ),
                             sourceVersionString );
    require_noerr( err, exit );

#if ENABLE_IPV6

    // Register Bonjour Service
    err = DNSServiceRegister( &inContext->dnsServiceRef,                 // DNSServiceRef
                              0,                                         // DNSServiceFlags
                              kDNSServiceInterfaceIndexAny,              // interface index
                              inContext->platformParams->name,           // name
                              kMFiConfigServiceType,                     // service name
                              kMFiConfigServiceDomain,                   // domain
                              NULL,                                      // host
                              inContext->httpServer->sockAddr.sin6_port, // port
                              TXTRecordGetLength( &txtRecord ),          // txt record length
                              TXTRecordGetBytesPtr( &txtRecord ),        // txt record pointer
                              NULL,                                      // callback
                              NULL );                                    // context

    require_noerr_action( err, exit, err = kInternalErr; wac_log( "ERROR: Could not register bonjour service" ) );

    wac_log("%s@%s:%d registered successfully",
            inContext->platformParams->name,
            kMFiConfigServiceType,
            ntohs( inContext->httpServer->sockAddr.sin6_port ));
                              
#else
    // Register Bonjour Service
    err = DNSServiceRegister( &inContext->dnsServiceRef,                 // DNSServiceRef
                              0,                                         // DNSServiceFlags
                              kDNSServiceInterfaceIndexAny,              // interface index
                              inContext->platformParams->name,           // name
                              kMFiConfigServiceType,                     // service name
                              kMFiConfigServiceDomain,                   // domain
                              NULL,                                      // host
                              inContext->httpServer->sockAddr.sin_port, // port
                              TXTRecordGetLength( &txtRecord ),          // txt record length
                              TXTRecordGetBytesPtr( &txtRecord ),        // txt record pointer
                              NULL,                                      // callback
                              NULL );                                    // context
    require_noerr_action( err, exit, err = kInternalErr; wac_log( "ERROR: Could not register bonjour service" ) );

    wac_log("%s@%s:%d registered successfully",
            inContext->platformParams->name,
            kMFiConfigServiceType,
            ntohs( inContext->httpServer->sockAddr.sin_port ));
                              

#endif

exit:
    TXTRecordDeallocate( &txtRecord );
    return err;
}

OSStatus RemoveWACBonjourService( WACContext_t * const inContext )
{
    Trace();
    OSStatus err = kParamErr;

    require( inContext, exit );
    require_action_quiet( inContext->dnsServiceRef, exit, err = kNoErr );

    DNSServiceRefDeallocate( inContext->dnsServiceRef );

    inContext->dnsServiceRef = NULL;

    err = kNoErr;

exit:
    return err;
}


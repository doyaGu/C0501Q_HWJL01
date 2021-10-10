/*
    File:    AppleDeviceIE.c
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

#include "AppleDeviceIE.h"

#define SIZE_OF_VENDOR_IE_HEADER    2
#define SIZE_OF_OUI                 4
#define SIZE_OF_FEATURE_SUB_IE      4
#define SIZE_OF_BLUETOOTH_MAC_ADDR  6
#define SIZE_OF_MAC_ADDR            6
#define SIZE_OF_DEVICE_OUI          3

#define apple_device_ie_log(M, ...) custom_log("AppleDeviceIE", M, ##__VA_ARGS__)

OSStatus _IEBufferStartVendorIE( IEBuffer_t *inBuf, uint32_t inVID );
OSStatus _IEBufferEndVendorIE( IEBuffer_t *inBuf );
OSStatus _IEBufferAppendIE( IEBuffer_t *inBuf, uint8_t inEID, const void *inData, size_t inLen );

OSStatus CreateAppleDeviceIE( const char *name,
                              const char *model,
                              const char *manufacturer,
                              const uint8_t *macAddress,
                              bool isUnconfigured,
                              bool supportsAirPlay,
                              bool supportsAirPrint,
                              bool supportsMFiConfigurationV1,
                              bool supportsWakeOnWireless,
                              bool supports2_4GHzNetworks,
                              bool supports5GHzNetworks,
                              uint8_t **outIEBuffer,
                              size_t *outIEBufferLen )
{
    OSStatus   err = kParamErr;
    uint8_t    bits[ BitArray_MaxBytes( kAppleDeviceIEFlagBit_TotalBits ) ];
    IEBuffer_t ieBuf;

    require( outIEBuffer, exit );
    require( outIEBufferLen, exit );

    require_action( macAddress, exit, apple_device_ie_log("ERROR: MAC Address required") );

    // Build Apple Device IE 
    IEBufferInit( &ieBuf );
    _IEBufferStartVendorIE( &ieBuf, kIEEE80211_VID_AppleDevice );
    
    _IEBufferAppendIE( &ieBuf, kAppleDeviceIE_DeviceID, macAddress, DEVICE_ID_LENGTH );
    
    BitArray_Clear( bits, sizeof( bits ) );
    if ( isUnconfigured )             BitArray_SetBit( bits, kAppleDeviceIEFlagBit_Unconfigured );
    if ( supportsAirPlay )            BitArray_SetBit( bits, kAppleDeviceIEFlagBit_AirPlay );
    if ( supportsAirPrint )           BitArray_SetBit( bits, kAppleDeviceIEFlagBit_AirPrint );
    if ( supportsMFiConfigurationV1 ) BitArray_SetBit( bits, kAppleDeviceIEFlagBit_MFiConfigurationV1 );
    if ( supports2_4GHzNetworks )     BitArray_SetBit( bits, kAppleDeviceIEFlagBit_2_4GHzWiFi );
    if ( supports5GHzNetworks )       BitArray_SetBit( bits, kAppleDeviceIEFlagBit_5GHzWiFi );
    if ( supportsWakeOnWireless )     BitArray_SetBit( bits, kAppleDeviceIEFlagBit_WakeOnWireless );

    _IEBufferAppendIE( &ieBuf, kAppleDeviceIE_Flags, bits, BitArray_MinBytes( bits, sizeof( bits ) ) );

    _IEBufferAppendIE( &ieBuf, kAppleDeviceIE_Name, name, kSizeCString );
    _IEBufferAppendIE( &ieBuf, kAppleDeviceIE_Model, model, kSizeCString );
    _IEBufferAppendIE( &ieBuf, kAppleDeviceIE_Manufacturer, manufacturer, kSizeCString );

    err = _IEBufferEndVendorIE( &ieBuf );

    *outIEBuffer = malloc( ieBuf.len );
    memcpy( (void*)*outIEBuffer, (void*)ieBuf.buf, ieBuf.len );
    *outIEBufferLen = ieBuf.len;

exit:
    return err;
}

OSStatus _IEBufferAppendIE( IEBuffer_t *inBuf, uint8_t inEID, const void *inData, size_t inLen )
{
    OSStatus            err;
    const uint8_t *     src;
    const uint8_t *     end;
    
    require_noerr_action_quiet( inBuf->firstErr, exit2, err = inBuf->firstErr );
    
    // IEEE 802.11 IE's are in the following format:
    //
    //      <1:eid> <1:length> <length:data>
    
    if( inLen == kSizeCString ) inLen = strlen( (const char *) inData );
    require_action( ( inBuf->len + 1 + 1 + inLen ) < sizeof( inBuf->buf ), exit, err = kSizeErr );
    
    inBuf->buf[ inBuf->len++ ] = inEID;
    inBuf->buf[ inBuf->len++ ] = (uint8_t) inLen;
    
    src = (const uint8_t *) inData;
    end = src + inLen;
    while( src < end ) inBuf->buf[ inBuf->len++ ] = *src++;
    err = kNoErr;
    
exit:
    if( !inBuf->firstErr ) inBuf->firstErr = err;
    
exit2:
    return( err );
}

//===========================================================================================================================
//  _IEBufferStartVendorIE
//===========================================================================================================================

OSStatus    _IEBufferStartVendorIE( IEBuffer_t *inBuf, uint32_t inVID )
{
    OSStatus            err;
    
    require_noerr_action_quiet( inBuf->firstErr, exit2, err = inBuf->firstErr );
    
    // IEEE 802.11 vendor-specific IE's are in the following format:
    //
    //      <1:eid=0xDD> <1:length> <3:oui> <1:type> <length - 4:data>
    
    require_action( ( inBuf->len + 1 + 1 + 3 + 1 ) < sizeof( inBuf->buf ), exit, err = kSizeErr );
    
    inBuf->buf[ inBuf->len++ ]  = kIEEE80211_EID_Vendor;
    inBuf->savedOffset          = inBuf->len;
    inBuf->buf[ inBuf->len++ ]  = 0; // Placeholder to update when the IE ends.
    inBuf->buf[ inBuf->len++ ]  = (uint8_t)( ( inVID >> 24 ) & 0xFF );
    inBuf->buf[ inBuf->len++ ]  = (uint8_t)( ( inVID >> 16 ) & 0xFF );
    inBuf->buf[ inBuf->len++ ]  = (uint8_t)( ( inVID >>  8 ) & 0xFF );
    inBuf->buf[ inBuf->len++ ]  = (uint8_t)(   inVID         & 0xFF );
    err = kNoErr;
    
exit:
    if( !inBuf->firstErr ) inBuf->firstErr = err;
    
exit2:
    return( err );
}

OSStatus _IEBufferEndVendorIE( IEBuffer_t *inBuf )
{
    OSStatus            err;
    
    require_noerr_action_quiet( inBuf->firstErr, exit2, err = inBuf->firstErr );
    require_action( inBuf->savedOffset > 0, exit, err = kNotPreparedErr );
    
    inBuf->buf[ inBuf->savedOffset ] = (uint8_t)( ( inBuf->len - inBuf->savedOffset ) - 1 );
    inBuf->savedOffset = 0;
    err = kNoErr;
    
exit:
    if( !inBuf->firstErr ) inBuf->firstErr = err;
    
exit2:
    return( err );
}


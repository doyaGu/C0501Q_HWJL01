/*
    File:    MFiSAPServer.h
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
     @header MFi SAP Server
      This header contains function prototypes called by the WAC
      engine. These functions abstract the interaction with mDNSResponder.
 */

#ifndef __MFiSAP_h__
#define __MFiSAP_h__

#include "Common.h"
#include "Debug.h"

//---------------------------------------------------------------------------------------------------------------------------
/*! @group      Creation
    @abstract   APIs for performing the Made for iPod (MFi) Security Association Protocol (SAP).
*/
#define kMFiSAPVersion1     1 // Curve25519 ECDH key exchange, RSA signing, AES-128 CTR encryption.

typedef struct MFiSAP *     MFiSAPRef;

OSStatus    MFiSAP_Create( MFiSAPRef *outRef, uint8_t inVersion );
void        MFiSAP_Delete( MFiSAPRef inRef );
#define     MFiSAP_Forget( X )  do { if( *(X) ) { MFiSAP_Delete( *(X) ); *(X) = NULL; } } while( 0 )

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   MFiSAP_Exchange
    @abstract   Perform key exchange.
    @discussion
    
    This is called on both the client and server sides. Each side repeatedly cals MFiSAP_Exchange until it returns an 
    error to indicate a failure or it returns kNoErr and sets "done" to true.
*/
OSStatus
    MFiSAP_Exchange( 
        MFiSAPRef       inRef, 
        const uint8_t * inInputPtr,
        size_t          inInputLen, 
        uint8_t **      outOutputPtr,
        size_t *        outOutputLen, 
        Boolean *       outDone );
OSStatus    MFiSAP_Encrypt( MFiSAPRef inRef, const void *inPlaintext,  size_t inLen, void *inCiphertextBuf );
OSStatus    MFiSAP_Decrypt( MFiSAPRef inRef, const void *inCiphertext, size_t inLen, void *inPlaintextBuf );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   MFiSAP_DeriveAESKey
    @abstract   Derive AES key and IV from the shared secret.
*/
OSStatus
    MFiSAP_DeriveAESKey(
        MFiSAPRef       inRef, 
        const void *    inKeySaltPtr, 
        size_t          inKeySaltLen, 
        const void *    inIVSaltPtr, 
        size_t          inIVSaltLen, 
        uint8_t         outKey[ 16 ], 
        uint8_t         outIV[ 16 ] );

#endif  // __MFiSAP_h__


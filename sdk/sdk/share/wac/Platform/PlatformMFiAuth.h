/*
    File:    PlatformMFiAuth.h
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
     @header Platform MFi Authentication
      This header contains function prototypes called by Apple code
      that must be implemented by the platform. These functions
      are called when Apple code needs to interact with the Apple
      Authentication Coprocessor. Please refer to the relevant
      version of the "Auth IC" document to obtain more details on
      how to interact with the Authentication Coprocessor. This
      document can be found on the MFi Portal.
 */

#ifndef __PlatformMFiAuth_h__
#define __PlatformMFiAuth_h__

#include "Common.h"

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthInitialize
    @abstract   Performs any platform-specific initialization needed. Example: Bring up I2C interface for communication with
                the Apple Authentication Coprocessor.
*/
OSStatus PlatformMFiAuthInitialize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthFinalize
    @abstract   Performs any platform-specific cleanup needed. Example: Bringing down the I2C interface for communication with
                the Apple Authentication Coprocessor.
*/
void PlatformMFiAuthFinalize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthCreateSignature
    @abstract   Create an RSA signature from the specified SHA-1 digest using the Apple Authentication Coprocessor.

    @param      inDigestPtr         Pointer to 20-byte SHA-1 digest.
    @param      inDigestLen         Number of bytes in the digest. Must be 20.
    @param      outSignaturePtr     Receives malloc()'d ptr to RSA signature. Caller must free() on success.
    @param      outSignatureLen     Receives number of bytes in RSA signature.
*/
OSStatus PlatformMFiAuthCreateSignature( const void *inDigestPtr,
                                         size_t     inDigestLen,
                                         uint8_t    **outSignaturePtr,
                                         size_t     *outSignatureLen );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthCopyCertificate
    @abstract   Copy the certificate from the Apple Authentication Coprocessor.

    @param      outCertificatePtr   Receives malloc()'d ptr to a DER-encoded PKCS#7 message containing the certificate.
                                    Caller must free() on success.
    @param      outCertificateLen   Number of bytes in the DER-encoded certificate.
*/
OSStatus PlatformMFiAuthCopyCertificate( uint8_t **outCertificatePtr, size_t *outCertificateLen );


#endif // __PlatformMFiAuth_h__


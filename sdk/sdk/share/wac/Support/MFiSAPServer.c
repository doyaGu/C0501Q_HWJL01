/*
    File:    MFiSAPServer.c
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

#include "MFiSAPServer.h"

#include "AESUtils.h"
#include "curve25519-donna.h"
#include "Debug.h"
#include "PlatformRandomNumber.h"
#include "PlatformMFiAuth.h"
#include "SHAUtils.h"
#include "TimeUtils.h"

#define sap_log(M, ...) custom_log("MFiSAP", M, ##__VA_ARGS__)

//===========================================================================================================================
//  Constants
//===========================================================================================================================

// MFiSAPState

typedef uint8_t     MFiSAPState;
#define kMFiSAPState_Invalid        0 // Not initialized.
#define kMFiSAPState_Init           1 // Initialized and ready for either client or server.
#define kMFiSAPState_ClientM1       2 // Client ready to generate M1.
#define kMFiSAPState_ClientM2       3 // Client ready to process server's M2.
#define kMFiSAPState_ClientDone     4 // Client successfully completed exchange process.
#define kMFiSAPState_ServerM1       5 // Server ready to process client's M1 and generate server's M2.
#define kMFiSAPState_ServerDone     6 // Server successfully completed exchange process.

// Misc

#define kMFiSAP_AES_IV_SaltPtr      "AES-IV"
#define kMFiSAP_AES_IV_SaltLen      ( sizeof( kMFiSAP_AES_IV_SaltPtr ) - 1 )

#define kMFiSAP_AES_KEY_SaltPtr     "AES-KEY"
#define kMFiSAP_AES_KEY_SaltLen     ( sizeof( kMFiSAP_AES_KEY_SaltPtr ) - 1 )

#define kMFiSAP_AESKeyLen           16
#define kMFiSAP_ECDHKeyLen          32
#define kMFiSAP_VersionLen          1

//===========================================================================================================================
//  Structures
//===========================================================================================================================

struct MFiSAP
{
    MFiSAPState         state;
    uint8_t             version;
    uint8_t             sharedSecret[ kMFiSAP_ECDHKeyLen ];
    AES_CTR_Context     aesMasterContext;
    Boolean             aesMasterValid;
};

// Globals

static uint64_t         gMFiSAP_LastTicks       = 0;
static unsigned int     gMFiSAP_ThrottleCounter = 0;

//===========================================================================================================================
//  Prototypes
//===========================================================================================================================

static OSStatus
    __MFiSAP_Exchange_ServerM1( 
        MFiSAPRef       inRef, 
        const uint8_t * inInputPtr,
        size_t          inInputLen, 
        uint8_t **      outOutputPtr,
        size_t *        outOutputLen );

//===========================================================================================================================
//  MFiSAP_Create
//===========================================================================================================================

OSStatus    MFiSAP_Create( MFiSAPRef *outRef, uint8_t inVersion )
{
    Trace();
    OSStatus        err;
    MFiSAPRef       obj;
    
    require_action( inVersion == kMFiSAPVersion1, exit, err = kVersionErr );
    
    obj = (MFiSAPRef) calloc( 1, sizeof( *obj ) );
    require_action( obj, exit, err = kNoMemoryErr );
    
    obj->state   = kMFiSAPState_Init;
    obj->version = inVersion;
    
    *outRef = obj;
    err = kNoErr;
    
exit:
    return err;
}

//===========================================================================================================================
//  MFiSAP_Delete
//===========================================================================================================================

void    MFiSAP_Delete( MFiSAPRef inRef )
{
    Trace();
    if( inRef->aesMasterValid )
    {
        AES_CTR_Final( &inRef->aesMasterContext );
    }
    memset( inRef, 0, sizeof( *inRef ) ); // Clear sensitive data and reset state.
    free( inRef );
}

//===========================================================================================================================
//  MFiSAP_Exchange
//===========================================================================================================================

OSStatus
    MFiSAP_Exchange( 
        MFiSAPRef       inRef, 
        const uint8_t * inInputPtr,
        size_t          inInputLen, 
        uint8_t **      outOutputPtr,
        size_t *        outOutputLen, 
        Boolean *       outDone )
{
    Trace();
    OSStatus err;
    *outDone = false;

    if( inRef->state == kMFiSAPState_Init )
    {
        inRef->state = ( inInputPtr == NULL ) ? kMFiSAPState_ClientM1 : kMFiSAPState_ServerM1;
    }

    if( inRef->state == kMFiSAPState_ServerM1 )
    {
        err = __MFiSAP_Exchange_ServerM1( inRef, inInputPtr, inInputLen, outOutputPtr, outOutputLen );
        require_noerr( err, exit );
        *outDone = true;
    }
    else
    {
        sap_log( "bad state %d", inRef->state );
        err = kStateErr;
        goto exit;
    }
    ++inRef->state;

exit:
    if( err ) inRef->state = kMFiSAPState_Invalid; // Invalidate on errors to prevent retries.
    return err;
}

//===========================================================================================================================
//  __MFiSAP_Exchange_ServerM1
//
//  Server: process client's M1 and generate M2.
//===========================================================================================================================

static OSStatus
    __MFiSAP_Exchange_ServerM1( 
        MFiSAPRef       inRef, 
        const uint8_t * inInputPtr,
        size_t          inInputLen, 
        uint8_t **      outOutputPtr,
        size_t *        outOutputLen )
{
    Trace();
    OSStatus            err;
    const uint8_t *     inputEnd;
    const uint8_t *     clientECDHPublicKey;
    uint8_t             ourPrivateKey[ kMFiSAP_ECDHKeyLen ];
    uint8_t             ourPublicKey[ kMFiSAP_ECDHKeyLen ];
    SHA_CTX             sha1Context;
    uint8_t             digest[ 20 ];
    uint8_t *           signaturePtr = NULL;
    size_t              signatureLen;
    uint8_t *           certificatePtr = NULL;
    size_t              certificateLen;
    uint8_t             aesMasterKey[ kMFiSAP_AESKeyLen ];
    uint8_t             aesMasterIV[ kMFiSAP_AESKeyLen ];
    uint8_t *           buf;
    uint8_t *           dst;
    size_t              len;

    if( ( UpTicks() - gMFiSAP_LastTicks ) < UpTicksPerSecond() )
    {
        if( gMFiSAP_ThrottleCounter < 4 ) ++gMFiSAP_ThrottleCounter;
        SleepForUpTicks( gMFiSAP_ThrottleCounter * UpTicksPerSecond() );
    }
    else
    {
        gMFiSAP_ThrottleCounter = 0;
    }
    gMFiSAP_LastTicks = UpTicks();

    // Validate inputs. Input data must be: <1:version> <32:client's ECDH public key>.

    inputEnd = inInputPtr + inInputLen;
    require_action( inputEnd > inInputPtr, exit, err = kSizeErr ); // Detect bad length causing ptr wrap.

    require_action( ( inputEnd - inInputPtr ) >= kMFiSAP_VersionLen, exit, err = kSizeErr );
    inRef->version = *inInputPtr++;
    require_action( inRef->version == kMFiSAPVersion1, exit, err = kVersionErr );

    require_action( ( inputEnd - inInputPtr ) >= kMFiSAP_ECDHKeyLen, exit, err = kSizeErr );
    clientECDHPublicKey = inInputPtr;
    inInputPtr += kMFiSAP_ECDHKeyLen;

    require_action( inInputPtr == inputEnd, exit, err = kSizeErr );

    // Generate a random ECDH key pair.

    err = PlatformCryptoStrongRandomBytes( ourPrivateKey, sizeof( ourPrivateKey ) );
    require_noerr( err, exit );
    curve25519_donna( ourPublicKey, ourPrivateKey, NULL );

    // Use our private key and the client's public key to generate the shared secret.
    // Hash the shared secret and truncate it to form the AES master key.
    // Hash the shared secret with salt to derive the AES master IV.

    curve25519_donna( inRef->sharedSecret, ourPrivateKey, clientECDHPublicKey );
    SHA1_Init( &sha1Context );
    SHA1_Update( &sha1Context, kMFiSAP_AES_KEY_SaltPtr, kMFiSAP_AES_KEY_SaltLen );
    SHA1_Update( &sha1Context, inRef->sharedSecret, sizeof( inRef->sharedSecret ) );
    SHA1_Final( digest, &sha1Context );
    memcpy( aesMasterKey, digest, sizeof( aesMasterKey ) );

    SHA1_Init( &sha1Context );
    SHA1_Update( &sha1Context, kMFiSAP_AES_IV_SaltPtr, kMFiSAP_AES_IV_SaltLen );
    SHA1_Update( &sha1Context, inRef->sharedSecret, sizeof( inRef->sharedSecret ) );
    SHA1_Final( digest, &sha1Context );
    memcpy( aesMasterIV, digest, sizeof( aesMasterIV ) );

    // Use the auth chip to sign a hash of <32:our ECDH public key> <32:client's ECDH public key>.
    // And copy the auth chip's certificate so the client can verify the signature.

    SHA1_Init( &sha1Context );
    SHA1_Update( &sha1Context, ourPublicKey, sizeof( ourPublicKey ) );
    SHA1_Update( &sha1Context, clientECDHPublicKey, kMFiSAP_ECDHKeyLen );
    SHA1_Final( digest, &sha1Context );
    err = PlatformMFiAuthCreateSignature( digest, sizeof( digest ), &signaturePtr, &signatureLen );
    require_noerr( err, exit );

    err = PlatformMFiAuthCopyCertificate( &certificatePtr, &certificateLen );
    require_noerr( err, exit );

    // Encrypt the signature with the AES master key and master IV.
    
    err = AES_CTR_Init( &inRef->aesMasterContext, aesMasterKey, aesMasterIV );
    require_noerr( err, exit );

    err = AES_CTR_Update( &inRef->aesMasterContext, signaturePtr, signatureLen, signaturePtr );
    if( err ) AES_CTR_Final( &inRef->aesMasterContext );
    require_noerr( err, exit );

    inRef->aesMasterValid = true;

    // Return the response:
    //
    //      <32:our ECDH public key>
    //      <4:big endian certificate length>
    //      <N:certificate data>
    //      <4:big endian signature length>
    //      <N:encrypted signature data>

    len = kMFiSAP_ECDHKeyLen + 4 + certificateLen + 4 + signatureLen;
    buf = (uint8_t *) malloc( len );
    require_action( buf, exit, err = kNoMemoryErr );
    dst = buf;

    memcpy( dst, ourPublicKey, sizeof( ourPublicKey ) );
    dst += sizeof( ourPublicKey );

    *dst++ = (uint8_t)( ( certificateLen >> 24 ) & 0xFF );
    *dst++ = (uint8_t)( ( certificateLen >> 16 ) & 0xFF );
    *dst++ = (uint8_t)( ( certificateLen >>  8 ) & 0xFF );
    *dst++ = (uint8_t)(   certificateLen         & 0xFF );
    memcpy( dst, certificatePtr, certificateLen );
    dst += certificateLen;

    *dst++ = (uint8_t)( ( signatureLen >> 24 ) & 0xFF );
    *dst++ = (uint8_t)( ( signatureLen >> 16 ) & 0xFF );
    *dst++ = (uint8_t)( ( signatureLen >>  8 ) & 0xFF );
    *dst++ = (uint8_t)(   signatureLen         & 0xFF );
    memcpy( dst, signaturePtr, signatureLen );
    dst += signatureLen;

    check( dst == ( buf + len ) );
    *outOutputPtr = buf;
    *outOutputLen = (size_t)( dst - buf );

exit:
    FreeSafe( certificatePtr );
    FreeSafe( signaturePtr );
    return err;
}

//===========================================================================================================================
//  MFiSAP_Encrypt
//===========================================================================================================================

OSStatus    MFiSAP_Encrypt( MFiSAPRef inRef, const void *inPlaintext, size_t inLen, void *inCiphertextBuf )
{
    Trace();
    OSStatus        err;

    require_action( ( inRef->state == kMFiSAPState_ClientDone ) || 
                    ( inRef->state == kMFiSAPState_ServerDone ), 
                    exit, err = kStateErr );

    err = AES_CTR_Update( &inRef->aesMasterContext, inPlaintext, inLen, inCiphertextBuf );
    require_noerr( err, exit );

exit:
    return err;
}

//===========================================================================================================================
//  MFiSAP_Decrypt
//===========================================================================================================================

OSStatus    MFiSAP_Decrypt( MFiSAPRef inRef, const void *inCiphertext, size_t inLen, void *inPlaintextBuf )
{
    Trace();
    OSStatus        err;

    require_action( ( inRef->state == kMFiSAPState_ClientDone ) || 
                    ( inRef->state == kMFiSAPState_ServerDone ), 
                    exit, err = kStateErr );

    err = AES_CTR_Update( &inRef->aesMasterContext, inCiphertext, inLen, inPlaintextBuf );
    require_noerr( err, exit );

exit:
    return err;
}

//===========================================================================================================================
//  MFiSAP_DeriveAESKey
//===========================================================================================================================

OSStatus
    MFiSAP_DeriveAESKey(
        MFiSAPRef       inRef, 
        const void *    inKeySaltPtr, 
        size_t          inKeySaltLen, 
        const void *    inIVSaltPtr, 
        size_t          inIVSaltLen, 
        uint8_t         outKey[ 16 ], 
        uint8_t         outIV[ 16 ] )
{
    Trace();
    OSStatus        err;
    SHA512_CTX      shaCtx;
    uint8_t         buf[ 64 ];

    require_action( ( inRef->state == kMFiSAPState_ClientDone ) || 
                    ( inRef->state == kMFiSAPState_ServerDone ), 
                    exit, err = kStateErr );
    if( outKey )
    {
        SHA512_Init( &shaCtx );
        SHA512_Update( &shaCtx, inKeySaltPtr, inKeySaltLen );
        SHA512_Update( &shaCtx, inRef->sharedSecret, 32 );
        SHA512_Final( buf, &shaCtx );
        memcpy( outKey, buf, 16 );
    }
    if( outIV )
    {
        SHA512_Init( &shaCtx );
        SHA512_Update( &shaCtx, inIVSaltPtr, inIVSaltLen );
        SHA512_Update( &shaCtx, inRef->sharedSecret, 32 );
        SHA512_Final( buf, &shaCtx );
        memcpy( outIV, buf, 16 );
    }
    err = kNoErr;

exit:
    return err;
}


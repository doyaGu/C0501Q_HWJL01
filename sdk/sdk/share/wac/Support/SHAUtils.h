﻿/*
    File:    SHAUtils.h
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
     @header SHA Utilities
 */

#ifndef __SHAUtils_h_
#define __SHAUtils_h_

#include "Common.h"

//===========================================================================================================================
//  SHA-1
//===========================================================================================================================

typedef struct
{
    uint64_t        length;
    uint32_t        state[ 5 ];
    uint32_t        curlen;
    uint8_t         buf[ 64 ];
    
}   SHA_CTX_compat;

int SHA1_Init_compat( SHA_CTX_compat *ctx );
int SHA1_Update_compat( SHA_CTX_compat *ctx, const void *inData, size_t inLen );
int SHA1_Final_compat( unsigned char *outDigest, SHA_CTX_compat *ctx );
unsigned char * SHA1_compat( const void *inData, size_t inLen, unsigned char *outDigest );

//===========================================================================================================================
//  SHA-512
//===========================================================================================================================

typedef struct
{
    uint64_t        length;
    uint64_t        state[ 8 ];
    size_t          curlen;
    uint8_t         buf[ 128 ];
    
}   SHA512_CTX_compat;

int SHA512_Init_compat( SHA512_CTX_compat *ctx );
int SHA512_Update_compat( SHA512_CTX_compat *ctx, const void *inData, size_t inLen );
int SHA512_Final_compat( unsigned char *outDigest, SHA512_CTX_compat *ctx );
unsigned char * SHA512_compat( const void *inData, size_t inLen, unsigned char *outDigest );

//===========================================================================================================================
//  SHA-3 (Keccak)
//===========================================================================================================================

#define SHA3_DIGEST_LENGTH      64
#define SHA3_F                  1600
#define SHA3_C                  ( SHA3_DIGEST_LENGTH * 8 * 2 )  // 512=1024
#define SHA3_R                  ( SHA3_F - SHA3_C )             // 512=576
#define SHA3_BLOCK_SIZE         ( SHA3_R / 8 )

typedef struct
{
    uint64_t        state[ SHA3_F / 64 ];
    size_t          leftover;
    uint8_t         buffer[ SHA3_BLOCK_SIZE ];
    
}   SHA3_CTX_compat;

int SHA3_Init_compat( SHA3_CTX_compat *ctx );
int SHA3_Update_compat( SHA3_CTX_compat *ctx, const void *inData, size_t inLen );
int SHA3_Final_compat( unsigned char *outDigest, SHA3_CTX_compat *ctx );
uint8_t *   SHA3_compat( const void *inData, size_t inLen, uint8_t outDigest[ 64 ] );

#endif // __SHAUtils_h_


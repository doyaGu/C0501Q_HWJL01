
#include "Common.h"

OSStatus PlatformSoftwareAccessPointStart( const uint8_t *inIE, size_t inIELen )
{
    usleep(20000);
    printf("[Wac]PlatformSoftwareAccessPointStart \n");
    printf("%s %d \n",inIE,inIELen);
    return kNoErr;
}

OSStatus PlatformSoftwareAccessPointStop( void )
{

    printf("[Wac]PlatformSoftwareAccessPointStop \n");
    return kNoErr;
    
}

OSStatus PlatformJoinDestinationWiFiNetwork( const char * const inSSID, const uint8_t * const inWiFiPSK, size_t inWiFiPSKLen )
{

    printf("[Wac]PlatformJoinDestinationWiFiNetwork \n");
    return kNoErr;
    
}

OSStatus PlatformApplyName( const char * const inName )
{

    printf("[Wac]PlatformApplyName \n");
    return kNoErr;
    
}


OSStatus PlatformApplyAirPlayPlayPassword( const char * const inPlayPassword )
{
    printf("[Wac]PlatformApplyAirPlayPlayPassword \n");
    return kNoErr;
    
}

OSStatus PlatformCryptoStrongRandomBytes( void *inBuffer, size_t inByteCount )
{

    printf("[Wac]PlatformCryptoStrongRandomBytes \n");
    
    return kNoErr;
    

}

OSStatus PlatformMFiAuthInitialize( void )
{
    printf("[Wac]PlatformMFiAuthInitialize \n");
    
    return kNoErr;

}

void PlatformMFiAuthFinalize( void )
{
    printf("[Wac]PlatformMFiAuthFinalize  \n");
    
    return kNoErr;

}


OSStatus PlatformMFiAuthCreateSignature( const void *inDigestPtr,
                                         size_t     inDigestLen,
                                         uint8_t    **outSignaturePtr,
                                         size_t     *outSignatureLen )
{

    printf("[Wac]PlatformMFiAuthCreateSignature  \n");
    
    return kNoErr;


}

OSStatus PlatformMFiAuthCopyCertificate( uint8_t **outCertificatePtr, size_t *outCertificateLen )
{
    printf("[Wac]PlatformMFiAuthCopyCertificate  \n");
    
    return kNoErr;

}

OSStatus PlatformInitializemDNSResponder( void )
{
    printf("[Wac]PlatformInitializemDNSResponder  \n");
    
    return kNoErr;

}

OSStatus PlatformMayStopmDNSResponder( void )
{
    printf("[Wac]PlatformMayStopmDNSResponder  \n");
    
    return kNoErr;

}



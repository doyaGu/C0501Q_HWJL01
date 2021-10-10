#ifndef MMP_NAND_H
#define MMP_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if 0//defined(WIN32) || defined(_WIN32_WCE)
    #if defined(NAND_EXPORTS)
        #define MMP_NAND_API __declspec(dllexport)
    #else
        #define MMP_NAND_API __declspec(dllimport)
    #endif
#else
    #define MMP_NAND_API extern
#endif /* defined(WIN32) */

MMP_NAND_API int
mmpNandInitial(unsigned long driver_param);

MMP_NAND_API int
mmpNandGetCapacity(unsigned long* lastBlockId,
                   unsigned long* blockLength);
                   
MMP_NAND_API int 
mmpNandReadSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* srcBuffer);
                  
MMP_NAND_API int 
mmpNandWriteSector(unsigned long blockId,
                  unsigned long sizeInByte,
                  unsigned char* destBuffer); 

#ifdef __cplusplus
}
#endif

#endif /* MMP_NAND_H */
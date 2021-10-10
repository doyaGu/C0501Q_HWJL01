#ifndef _XD_MLAYER_H__
#define _XD_MLAYER_H__

#include "xd.h"
#include "fat/fat.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum XD_CARD_STATE_TAG
{
    XD_INIT_OK,
    XD_INSERTED
}XD_CARD_STATE;

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
F_DRIVER* f_xdinit(unsigned long driver_param);
int  xddrv_getstatus(F_DRIVER* driver, XD_CARD_STATE index);
int  xddrv_getcapacity(MMP_UINT32* sectorNum, MMP_UINT32* blockLength);
int  xddrv_readsector(F_DRIVER* driver,void* data, unsigned long sector);
int  xddrv_writesector(F_DRIVER* driver,void *data, unsigned long sector);
int  xddrv_readmultiplesector(F_DRIVER* driver,void *data, unsigned long sector, int cnt);
int  xddrv_writemultiplesector(F_DRIVER* driver,void *data, unsigned long sector, int cnt);
int  xddrv_getphy(F_DRIVER* driver, F_PHY *phy);
void xddrv_release(F_DRIVER* driver);


extern MMP_INT mmpxDInitialize(void);    

extern MMP_INT mmpxDTerminate(void);

extern MMP_INT mmpxDRead(MMP_UINT32 sector, MMP_INT count, void* data);

extern MMP_INT mmpxDWrite(MMP_UINT32 sector, MMP_INT count, void* data);

extern MMP_INT mmpxDGetCapacity(MMP_UINT32* sectorNum, MMP_UINT32* blockLength);
    
extern MMP_BOOL mmpxDGetCardState(XD_CARD_STATE index); 

#ifdef __cplusplus
}
#endif

#endif

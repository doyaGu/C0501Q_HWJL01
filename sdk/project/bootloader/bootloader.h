#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "ite/ug.h"
#include "ite/itc.h"

#ifdef __cplusplus
extern "C"
{
#endif

// logo
void ShowLogo(void);

//boot video
void PlayVideo(void);
void WaitPlayVideoFinish(void);

// upgrade
ITCStream* OpenUpgradePackage(void);
void DeleteUpgradePackage(void);
ITCStream* OpenRecoveryPackage(void);
ITCStream* OpenUsbDevicePackage(void);

// reset
int ResetFactory(void);

// restore
void RestorePackage(void);

// boot
void BootImage(void);

void BootBin(ITCStream *upgradeFile);

#ifdef __cplusplus
}
#endif

#endif // BOOTLOADER_H

#include "ite/itu.h"

extern void LogoReset(void);
extern void MainReset(void);
extern void SettingReset(void);
extern void VideoPlayerReset(void);
extern void EngineerReset(void);
extern void EngineerPowerReset(void);
extern void EngineerMachineReset(void);
extern void EngineerTryReset(void);

void resetScene(void)
{
    LogoReset();
    MainReset();
    SettingReset();
    VideoPlayerReset();
    EngineerReset();
    EngineerPowerReset();
    EngineerMachineReset();
    EngineerTryReset();
}

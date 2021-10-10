#include "ite/itu.h"

extern void TouchCalibrationReset(void);
extern void ScreensaverReset(void);
extern void MainReset(void);
extern void MainMenuReset(void);
extern void AirConditionerReset(void);
extern void VideoViewReset(void);
extern void UsbDeviceModeReset(void);

void resetScene(void)
{
	TouchCalibrationReset();
    ScreensaverReset();
    MainReset();
    MainMenuReset();
    AirConditionerReset();
    VideoViewReset();
    UsbDeviceModeReset();
}

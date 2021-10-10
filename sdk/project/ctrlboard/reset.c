#include "ite/itu.h"

extern void TouchCalibrationReset(void);
extern void ScreensaverReset(void);
extern void MainReset(void);
extern void MainMenuReset(void);
extern void ClockReset(void);
extern void AirConditionerReset(void);
extern void Keyboard2Reset(void);
extern void AudioPlayerReset(void);
extern void VideoPlayerReset(void);
extern void VideoViewReset(void);
extern void ImagePlayerReset(void);
extern void ImageViewReset(void);
extern void CalendarReset(void);
extern void ButtonReset(void);
extern void MeterReset(void);
extern void SettingDisplayReset(void);
extern void SettingSoundReset(void);
extern void SettingLangReset(void);
extern void SettingWiFiMainReset(void);
extern void SettingWiFiSsidReset(void);
extern void SettingWiFiPasswordReset(void);
extern void SettingWiFiNetworkReset(void);
extern void SettingSysInfoReset(void);
extern void CheckAnimReset(void);
extern void CheckListReset(void);
extern void CircleCtrlReset(void);
extern void ListReset(void);
extern void UsbDeviceModeReset(void);

void resetScene(void)
{
	TouchCalibrationReset();
    ScreensaverReset();
    MainReset();
    MainMenuReset();
    ClockReset();
    AirConditionerReset();
    Keyboard2Reset();
    AudioPlayerReset();
    VideoPlayerReset();
    VideoViewReset();
    ImagePlayerReset();
    ImageViewReset();
    CalendarReset();
    ButtonReset();
    MeterReset();
    SettingDisplayReset();
    SettingSoundReset();
    SettingLangReset();
    SettingWiFiMainReset();
    SettingWiFiSsidReset();
    SettingWiFiPasswordReset();
    SettingWiFiNetworkReset();
    SettingSysInfoReset();
    CheckAnimReset();
    CheckListReset();
    CircleCtrlReset();
    ListReset();
    UsbDeviceModeReset();
}

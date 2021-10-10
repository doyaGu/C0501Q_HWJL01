/** @file
 * ITE Display Control Board Modules.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2015
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup ctrlboard ITE Display Control Board Modules
 *  @{
 */
#ifndef CTRLBOARD_H
#define CTRLBOARD_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ctrlboard_audio Audio Player
 *  @{
 */

typedef int (*AudioPlayCallback)(int state);

/**
 * Initializes audio module.
 */
void AudioInit(void);

/**
 * Exits audio module.
 */
void AudioExit(void);

/**
 * Plays the specified wav file.
 *
 * @param filename The specified wav file to play.
 * @param func The callback function.
 * @return 0 for success, otherwise failed.
 */
int AudioPlay(char* filename, AudioPlayCallback func);

/**
 * Stops playing sound.
 */
void AudioStop(void);

int AudioPlayMusic(char* filename, AudioPlayCallback func);

/**
 * Plays keypad sound.
 */
void AudioPlayKeySound(void);
void AudioPauseKeySound(void);
void AudioResumeKeySound(void);

/**
 * Sets the volume of keypad sound.
 *
 * @param level the percentage of volume.
 */
void AudioSetKeyLevel(int level);

/**
 * Mutes all audio.
 */
void AudioMute(void);

/**
 * Un-mutes all audio.
 */
void AudioUnMute(void);

/**
 * Determines whether this audio is muted or not.
 *
 * @return true muted, false otherwise.
 */
bool AudioIsMuted(void);

bool AudioIsPlaying(void);

void AudioSetVolume(int level);
void AudioSetLevel(int level);
int AudioGetVolume(void);

/** @} */ // end of ctrlboard_audio

/** @defgroup ctrlboard_config Configuration
 *  @{
 */
/**
 * Language type definition.
 */
typedef enum
{
    LANG_ENG,   ///< English
    LANG_CHT,   ///< Traditional Chinese
    LANG_CHS    ///< Simplified Chinese
} LangType;

/**
 * Main menu type definition.
 */
typedef enum
{
    MAINMENU_COVERFLOW,             ///< Cover flow
    MAINMENU_COVERFLOW_REFLECTION,  ///< Cover flow with reflection effect
    MAINMENU_PAGEFLOW_FLIP,         ///< Flip page flow 
    MAINMENU_PAGEFLOW_FLIP2,        ///< Flip2 page flow 
    MAINMENU_PAGEFLOW_FOLD,         ///< Fold page flow 
    MAINMENU_COVERFLOW_RIPPLE       ///< Cover flow with ripple effect
} MainMenuType;

typedef struct
{
    // network
    int dhcp;                                                           ///< Enable DHCP or not
    char ipaddr[16];                                                    ///< IP address
    char netmask[16];                                                   ///< Netmask
    char gw[16];                                                        ///< Gateway address
    char dns[16];                                                       ///< DNS address

    // display
    int brightness;                                                     ///< Brightness, the range is 0~9
    int screensaver_time;                                               ///< Time to enter screen saver mode, unit is minute
    int screensaver_type;                                               ///< Screen saver type @see ScreensaverType
    int lang;                                                           ///< Language type @see LangType
    int mainmenu_type;                                                  ///< Main menu type @see MainMenuType

    // sound
    char      keysound[PATH_MAX];                                       // /< Key sound file path
    int       keylevel;                                                 // /< Key volume percentage, range is 0~100
    int       audiolevel;                                               // /< Audio volume percentage, range is 0~100

    // photo
    int       photo_interval;                                           // /< Photo change interval (second)
        
    // setting
    int touch_calibration;                                              ///< Need to do the touch calibration or not

    int wifi_mode;
    int wifi_status;
    char ssid[64];
    char password[256];
    char secumode[3];    

} Config;

/**
 * Global instance variable of configuration.
 */
extern Config theConfig;

/**
 * Loads configuration file.
 */
void ConfigInit(void);

/**
 * Exits configuration.
 */
void ConfigExit(void);

/**
 * Updates CRC files.
 *
 * @param filepath The file path to update the CRC value. NULL for ini file on public drive.
 */
void ConfigUpdateCrc(char* filepath);

/**
 * Saves the public part of configuration to file.
 */
void ConfigSave(void);

/** @} */ // end of ctrlboard_config

/** @defgroup ctrlboard_external External
 *  @{
 */
#define EXTERNAL_BUFFER_SIZE 64 ///< External buffer size

typedef enum
{
    EXTERNAL_SHOW_MSG,  ///< Show message
    EXTERNAL_TEST0,     ///< Test #0
    EXTERNAL_TEST1,     ///< Test #1
    EXTERNAL_TEST2,     ///< Test #2
    EXTERNAL_TEST3,     ///< Test #3
    EXTERNAL_TEST4,     ///< Test #4
    EXTERNAL_TEST5      ///< Test #5
} ExternalEventType;

typedef struct
{
    ExternalEventType type;
    int arg1;
    int arg2;
    int arg3;
    uint8_t buf1[EXTERNAL_BUFFER_SIZE];

} ExternalEvent;

/**
 * Initializes external module.
 */
void ExternalInit(void);

/**
 * Exits external module.
 */
void ExternalExit(void);

/**
 * Receives external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
int ExternalReceive(ExternalEvent* ev);

/**
 * Sends external module event.
 *
 * @param ev The external event.
 * @return 0 for success, otherwise for failure.
 */
int ExternalSend(ExternalEvent* ev);

/**
 * Initializes external process module.
 */
void ExternalProcessInit(void);

/**
 * Processes external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
void ExternalProcessEvent(ExternalEvent* ev);

/** @} */ // end of ctrlboard_external

/** @defgroup ctrlboard_network Network
 *  @{
 */
/**
 * Initializes network module.
 */
void NetworkInit(void);

/**
 * Resets network module.
 */
void NetworkReset(void);

/**
 * Determines whether the network is ready or not.
 *
 * @return true for ready; false for net ready yet.
 */
bool NetworkIsReady(void);

/** @} */ // end of ctrlboard_network

/** @defgroup ctrlboard_photo Photo Loader
 *  @{
 */

typedef void (*PhotoLoadCallback)(uint8_t* data, int size);

void PhotoInit(void);

void PhotoExit(void);

int PhotoLoad(char* filename, PhotoLoadCallback func);

/** @} */ // end of ctrlboard_photo

/** @defgroup ctrlboard_screen Screen
 *  @{
 */
/**
 * Screensaver type definition.
 */
typedef enum
{
    SCREENSAVER_NONE,   ///< No screensaver
    SCREENSAVER_CLOCK,  ///< Clock sreensaver
    SCREENSAVER_BLANK,  ///< Black screen screensaver
    SCREENSAVER_PHOTO   ///< Photo screensaver
} ScreensaverType;

/**
 * Initializes screen module.
 */
void ScreenInit(void);

/**
 * Turns off screen.
 */
void ScreenOff(void);

/**
 * Turns on screen.
 */
void ScreenOn(void);

/**
 * Is the screen off or on.
 *
 * @return true for off; false for on.
 */
bool ScreenIsOff(void);

/**
 * Sets the brightness.
 *
 * @param value The brightness value.
 */
void ScreenSetBrightness(int value);

/**
 * Gets the maximum brightness level.
 *
 * @return the maximum brightness level.
 */
int ScreenGetMaxBrightness(void);

/**
 * Re-counts the time to enter screensaver.
 */
void ScreenSaverRefresh(void);

/**
 * Checks whether it is about time to enter screensaver mode.
 *
 * @return 0 for not yet, otherwise for entering screensaver mode currently.
 */
int ScreenSaverCheck(void);

/**
 * Is on screensaver mode or not.
 */
bool ScreenSaverIsScreenSaving(void);

/**
 * Takes a screenshot to USB drive.
 *
 * @param lcdSurf The LCD surface widget.
 */
void Screenshot(void* lcdSurf);

/** @} */ // end of ctrlboard_screen

/** @defgroup ctrlboard_storage Storage
 *  @{
 */

typedef enum
{
    STORAGE_NONE = -1,
    STORAGE_USB,
    STORAGE_SD,
    STORAGE_INTERNAL,

    STORAGE_MAX_COUNT
} StorageType;

typedef enum
{
    STORAGE_UNKNOWN,
    STORAGE_SD_INSERTED,
    STORAGE_SD_REMOVED,
    STORAGE_USB_INSERTED,
    STORAGE_USB_REMOVED,
    STORAGE_USB_DEVICE_INSERTED,
    STORAGE_USB_DEVICE_REMOVED
} StorageAction;

/**
 * Initializes storage module.
 */
void StorageInit(void);

StorageAction StorageCheck(void);
StorageType StorageGetCurrType(void);
void StorageSetCurrType(StorageType type);
char* StorageGetDrivePath(StorageType type);
bool StorageIsInUsbDeviceMode(void);

/** @} */ // end of ctrlboard_storage

/** @defgroup ctrlboard_string String
 *  @{
 */
/**
 * Guard sensors definition.
 */
typedef enum
{
    GUARD_EMERGENCY,    ///< Emergency sensor
    GUARD_INFRARED,     ///< Infrared sensor
    GUARD_DOOR,         ///< Door sensor
    GUARD_WINDOW,       ///< Window sensor
    GUARD_SMOKE,        ///< Smoke sensor
    GUARD_GAS,          ///< Gas sensor
    GUARD_AREA,         ///< Area sensor
    GUARD_ROB,          ///< Rob sensor

    GUARD_SENSOR_COUNT  ///< Total sensor count
} GuardSensor;

/**
 * Gets the description of guard sensor.
 *
 * @param sensor The guard sensor.
 * @return the string of guard sensor.
 */
const char* StringGetGuardSensor(GuardSensor sensor);

/**
 * Gets the description of WiFi connected.
 *
 * @return the string of WiFi connected.
 */
const char* StringGetWiFiConnected(void);

/** @} */ // end of ctrlboard_string

/** @defgroup ctrlboard_upgrade Upgrade
 *  @{
 */
/**
 * Quit value definition.
 */
typedef enum
{
    QUIT_NONE,                  ///< Do not quit
    QUIT_DEFAULT,               ///< Quit for nothing
    QUIT_RESET_FACTORY,         ///< Quit to reset to factory setting
    QUIT_UPGRADE_FIRMWARE,      ///< Quit to upgrade firmware
    QUIT_UPGRADE_WEB,           ///< Quit to wait web upgrading
    QUIT_RESET_NETWORK          ///< Quit to reset network
} QuitValue;

/**
 * Initializes upgrade module.
 *
 * @return 0 for initializing success, non-zero for initializing failed and the value will be the QuitValue.
 */
int UpgradeInit(void);

/**
 * Sets the CRC value of the specified file path.
 *
 * @param filepath The file path to update the CRC value.
 */
void UpgradeSetFileCrc(char* filepath);

/**
 * Sets the URL to upgrade.
 *
 * @param url The url to download and upgrade.
 */
void UpgradeSetUrl(char* url);

/**
 * Sets the stream to upgrade.
 *
 * @param stream The stream to upgrade.
 */
void UpgradeSetStream(void* stream);

/**
 * Processes the upgrade procedure by QuitValue.
 *
 * @param code The QuitValue.
 * @return 0 for process success; otherwise failed.
 */
int UpgradeProcess(int code);

/**
 * Is upgrading ready or not.
 *
 * @return true for ready; otherwise not ready yet.
 */
bool UpgradeIsReady(void);

/**
 * Is upgrading finished or not.
 *
 * @return true for finished; otherwise not finish yet.
 */
bool UpgradeIsFinished(void);

/**
 * Gets the upgrading result.
 *
 * @return 0 for success, failed otherwise.
 */
int UpgradeGetResult(void);

/** @} */ // end of ctrlboard_upgrade

/** @defgroup ctrlboard_webserver Web Server
 *  @{
 */
/**
 * Initializes web server module.
 */
void WebServerInit(void);

/**
 * Exits web server module.
 */
void WebServerExit(void);

/** @} */ // end of ctrlboard_webserver

void PlayVideo(int x, int y, int width, int height, int bgColor, int volume);
void WaitPlayVideoFinish(void);

void PlayMjpeg(int x, int y, int width, int height, int bgColor, int volume);
void WaitPlayMjpegFinish(void);

#ifdef __cplusplus
}
#endif

#endif /* CTRLBOARD_H */
/** @} */ // end of ctrlboard
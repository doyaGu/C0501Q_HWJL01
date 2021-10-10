/** @file
 * ITE Elevator Modules.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup elevator ITE Elevator Modules
 *  @{
 */
#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup elevator_audio Audio Player
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
int AudioGetVolume(void);

/** @} */ // end of elevator_audio

/** @defgroup elevator_config Configuration
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

#define MAX_DATE_FOMRAT 2   // maximum date format

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
    int date_format;                                                    ///< Date format, the range is 0~2
    int logo1_format;                                                   ///< Logo1 display format, the range is 0~3
    int logo1_delay;                                                    ///< Logo1 display delay, the unit is 17 ms
    int logo2_format;                                                   ///< Logo2 display format, the range is 0~3
    int logo2_delay;                                                    ///< Logo2 display delay, the unit is 17 ms
    int web1_format;                                                    ///< Web1 display format, the range is 0~3
    int web1_delay;                                                     ///< Web1 display delay, the unit is 17 ms
    int web2_format;                                                    ///< Web2 display format, the range is 0~3
    int web2_delay;                                                     ///< Web2 display delay, the unit is 17 ms
    int web3_format;                                                    ///< Web3 display format, the range is 0~3
    int web3_delay;                                                     ///< Web3 display delay, the unit is 17 ms
    int info1_format;                                                   ///< Information1 display format, the range is 0~3
    int info1_delay;                                                    ///< Information1 display delay, the unit is 17 ms
    int info2_format;                                                   ///< Information2 display format, the range is 0~3
    int info2_delay;                                                    ///< Information2 display delay, the unit is 17 ms
    int arrow_delay;                                                    ///< Arrow display delay, the unit is 17 ms
    int demo_enable;                                                    ///< In demo mode or not

    // sound
    int audiolevel;                                                     ///< Audio volume percentage, range is 0~100
    int sound_enable;                                                   ///< Play sound or not

    // photo
    int photo1_format;                                                  ///< Photo1 display format, the range is 0~1
    int photo1_interval;                                                ///< Photo1 change interval (second)
    int photo2_format;                                                  ///< Photo2 display format, the range is 0~1
    int photo2_interval;                                                ///< Photo2 change interval (second)

    // login
    char user_id[64];
    char user_password[64];

    // wifi
    int wifi_mode;
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

/** @} */ // end of elevator_config

/** @defgroup ctrlboard_external External
 *  @{
 */

typedef enum
{
    EXTERNAL_ARROW1,        ///< Show arrow #1
    EXTERNAL_ARROW2,        ///< Show arrow #2
    EXTERNAL_FLOOR1,        ///< Show floor #1
    EXTERNAL_FLOOR2,        ///< Show floor #2
    EXTERNAL_TEMPERATURE,   ///< Show temperature
    EXTERNAL_WARN1,         ///< Show warning #1
    EXTERNAL_FLOOR_ARRIVE1, ///< Play floor arrived sounds #1
    EXTERNAL_FLOOR_ARRIVE2  ///< Play floor arrived sounds #2
} ExternalEventType;

typedef enum
{
    EXTERNAL_ARROW_NONE,    ///< Hide arrow
    EXTERNAL_ARROW_UP,      ///< Show up arrow
    EXTERNAL_ARROW_DOWN     ///< Show down arrow
} ExternalArrowType;

#define EXTERNAL_FLOOR_NONE (-1) ///< Not availible

typedef enum
{
    EXTERNAL_WARN_NONE,         ///< Hide warning
    EXTERNAL_WARN_EARTHQUAKE,   ///< Show earthquake warning
    EXTERNAL_WARN_FIRE,         ///< Show fire warning
    EXTERNAL_WARN_MAINTENANCE,  ///< Show maintenance warning
    EXTERNAL_WARN_OVERLOAD,     ///< Show overload warning
	EXTERNAL_WARN_DRIVER,       ///< Show driver
	EXTERNAL_WARN_EMERGENCY,    ///< Show emergency warning
	EXTERNAL_WARN_FULLLOAD,     ///< Show fullload warning
	EXTERNAL_WARN_LOCKED,       ///< Show locked warning
	EXTERNAL_WARN_NOSERVICE,    ///< Show no-service warning
	EXTERNAL_WARN_SPECIAL,      ///< Show special
	EXTERNAL_WARN_TROUBLE,      ///< Show trouble warning
	EXTERNAL_WARN_EXT1,         ///< Show ext #1
	EXTERNAL_WARN_EXT2          ///< Show ext #2
} ExternalWarnType;

typedef struct
{
    ExternalEventType type;
    int arg1;
    int arg2;
    int arg3;

} ExternalEvent;

typedef enum
{
    EXTERNAL_HORIZONTAL,   ///< Horizontal
    EXTERNAL_VERTICAL      ///< Vertical
} ExternalOrientation;

/**
 * Initializes external module.
 */
void ExternalInit(void);

/**
 * Exits external module.
 */
void ExternalExit(void);

/**
 * Checks external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
int ExternalCheck(ExternalEvent* ev);

/**
 * Gets current screen orientation.
 *
 * @return current screen orientation.
 */
ExternalOrientation ExternalGetOrientation(void);


/** @} */ // end of ctrlboard_external

/** @defgroup elevator_network Network
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

/** @} */ // end of elevator_network

/** @defgroup elevator_screen Screen
 *  @{
 */
/**
 * Screensaver type definition.
 */
typedef enum
{
    SCREENSAVER_NONE,   ///< No screensaver
    SCREENSAVER_BLANK   ///< Black screen screensaver
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

/** @} */ // end of elevator_screen

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
    STORAGE_USB_REMOVED
} StorageAction;

/**
 * Initializes storage module.
 */
void StorageInit(void);

StorageAction StorageCheck(void);
StorageType StorageGetCurrType(void);
void StorageSetCurrType(StorageType type);
char* StorageGetDrivePath(StorageType type);

/** @} */ // end of ctrlboard_storage

/** @defgroup elevator_upgrade Upgrade
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
    QUIT_UPGRADE_RESOURCE,      ///< Quit to upgrade resources
    QUIT_UPGRADE_WEB,           ///< Quit to wait web upgrading
    QUIT_UPGRADE_NET_RES,       ///< Quit to upgrade resources by network
    QUIT_UPGRADE_NET_FW         ///< Quit to upgrade firmware by network
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

/** @} */ // end of elevator_upgrade

/** @defgroup elevator_webserver Web Server
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

/** @} */ // end of elevator_webserver

#ifdef __cplusplus
}
#endif

#endif /* ELEVATOR_H */
/** @} */ // end of elevator
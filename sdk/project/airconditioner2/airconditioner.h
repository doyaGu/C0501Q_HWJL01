/** @file
 * ITE AirConditioner Modules.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup airconditioner ITE AirConditioner Modules
 *  @{
 */
#ifndef AIRCONDITIONER_H
#define AIRCONDITIONER_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup airconditioner_audio Audio Player
 *  @{
 */
/**
 * Sound type definition.
 */
typedef enum
{
    SOUND_KEY,
    SOUND_PURE_ON,
    SOUND_PURE_OFF,
    SOUND_MODE_COOL,
    SOUND_MODE_DRY,
    SOUND_MODE_HEAT,
    SOUND_MODE_FAN,
    SOUND_MODE_AUTO,
    SOUND_TEMPERATURE_17,
    SOUND_TEMPERATURE_18,
    SOUND_TEMPERATURE_19,
    SOUND_TEMPERATURE_20,
    SOUND_TEMPERATURE_21,
    SOUND_TEMPERATURE_22,
    SOUND_TEMPERATURE_23,
    SOUND_TEMPERATURE_24,
    SOUND_TEMPERATURE_25,
    SOUND_TEMPERATURE_26,
    SOUND_TEMPERATURE_27,
    SOUND_TEMPERATURE_28,
    SOUND_TEMPERATURE_29,
    SOUND_TEMPERATURE_30,
    SOUND_POWER_ON,
    SOUND_POWER_OFF,
    SOUND_WIND,
    SOUND_WIND_AUTO,
    SOUND_WIND_MAX,
    SOUND_SETTING_POWER,
    SOUND_SETTING_TIME,
    SOUND_SETTING_FUNCTION,
    SOUND_SETTING_SYSTEM,
    SOUND_TEMPERATURE,
    SOUND_CLOSEDOOR,
    SOUND_TEMPERATURE_LOW,

    SOUND_MAX
} SoundType;

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
 * Plays sound.
 *
 * @param sound The sound type to play.
 */
void AudioPlaySound(SoundType sound);

/**
 * Pauses keypad sound.
 */
void AudioPauseKeySound(void);

/**
 * Resumes keypad sound.
 */
void AudioResumeKeySound(void);

/**
 * Determines whether this audio is muted or not.
 *
 * @return true muted, false otherwise.
 */
bool AudioIsMuted(void);

bool AudioIsPlaying(void);

void AudioSetVolume(int level);
int AudioGetVolume(void);

/** @} */ // end of airconditioner_audio

/** @defgroup airconditioner_config Configuration
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
 * Screensaver time definition.
 */
typedef enum
{
    SCREENSAVER_15SEC,  ///< 15 seconds
    SCREENSAVER_30SEC,  ///< 30 seconds
    SCREENSAVER_1MIN,   ///< 1 minute
    SCREENSAVER_5MIN,   ///< 5 minutes
    SCREENSAVER_NONE
} ScreenSaverTime;

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
    int lang;                                                           ///< Language type @see LangType

    // sound
    int audiolevel;                                                     ///< Audio volume percentage, range is 0~100
    int sound_enable;                                                   ///< Play sound or not

    // airconditioner
    int temperature;
    int mode;
    int wind;
    int light_enable;
    int power_on_enable;
    int power_on_hour;
    int power_on_min;
    int power_off_enable;
    int power_off_hour;
    int power_off_min;
    int power_mon_enable;
    int power_tue_enable;
    int power_wed_enable;
    int power_thu_enable;
    int power_fri_enable;
    int power_sat_enable;
    int power_sun_enable;
    int power_price_dollar;
    int power_price_cent;
    int power_reset_year;
    int power_reset_month;
    int power_reset_day;
    float power_vol_mon;
    float power_vol_tue;
    float power_vol_wed;
    float power_vol_thu;
    float power_vol_fri;
    float power_vol_sat;
    float power_vol_sun;
    float power_vol_before;

    // login
    char user_id[64];
    char user_password[64];

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

/** @} */ // end of airconditioner_config

/** @defgroup airconditioner_external External
 *  @{
 */

typedef enum
{
    EXTERNAL_PM25,                      ///< Enable clearing PM2.5 mode
    EXTERNAL_MODE,                      ///< Set mode
    EXTERNAL_TEMPERATURE,               ///< Set temperature
    EXTERNAL_POWER,                     ///< Power on/off
    EXTERNAL_WIND,                      ///< Set wind
    EXTERNAL_LIGHT,                     ///< Light on/off
    EXTERNAL_TEMPERATURE_INDOOR,        ///< Get indoor temperature
    EXTERNAL_TEMPERATURE_OUTDOOR,       ///< Get outdoor temperature
    EXTERNAL_WARN,                      ///< Show warning
    EXTERNAL_POWER_ENTER,               ///< Enter power management page
    EXTERNAL_POWER_TIME,                ///< Get power running time
    EXTERNAL_POWER_VOLUME,              ///< Get current power volume
    EXTERNAL_POWER_TOTAL_VOLUME,        ///< Get total power volume
    EXTERNAL_POWER_PRICE,               ///< Set power price
    EXTERNAL_POWER_TOTAL_PRICE,         ///< Get total power price
    EXTERNAL_POWER_RESET,               ///< Reset power statistics
    EXTERNAL_TIME_POWER_ON,             ///< Set power-on time
    EXTERNAL_TIME_POWER_OFF,            ///< Set power-off time
    EXTERNAL_TIME_WEEK,                 ///< Set week to power on/off
    EXTERNAL_FUNC_VERTICAL,             ///< Vertical function
    EXTERNAL_FUNC_HORIZONTAL,           ///< Horizontal function
    EXTERNAL_FUNC_CLEAN,                ///< Clean function
    EXTERNAL_FUNC_COOLING,              ///< Cooling function
    EXTERNAL_FUNC_SAVING,               ///< Saving function
    EXTERNAL_FUNC_HEAT_HIGH,            ///< Heat high function
    EXTERNAL_FUNC_HEAT_LOW,             ///< Heat low function
    EXTERNAL_FUNC_STANDBY_CLEAN,        ///< Standby clean function
    EXTERNAL_ENG_MODE,                  ///< Enter/Leave engineer mode
    EXTERNAL_ENG_POWER_MODE,            ///< Enter/Leave power memory mode
    EXTERNAL_ENG_POWER_MACHINE_MODE,    ///< Enter/Leave machine code mode
    EXTERNAL_ENG_POWER_TRY_MODE,        ///< Enter/Leave try mode
    EXTERNAL_ENG_POWER_MEMORY           ///< Power memory on/off
} ExternalEventType;

typedef enum
{
    EXTERNAL_MODE_COOL,     ///< Cool mode
    EXTERNAL_MODE_DRY,      ///< Dry mode
    EXTERNAL_MODE_HEAT,     ///< Heat mode
    EXTERNAL_MODE_FAN,      ///< Fan mode
    EXTERNAL_MODE_AUTO,     ///< Auto mode

    EXTERNAL_MODE_MAX       ///< Maximum mode
} ExternalMode;

typedef enum
{
    EXTERNAL_WARN_NONE,     ///< Hide warning
    EXTERNAL_WARN_1,        ///< #1 warning
    EXTERNAL_WARN_2,        ///< #2 warning
    EXTERNAL_WARN_3,        ///< #3 warning
    EXTERNAL_WARN_4,        ///< #4 warning
    EXTERNAL_WARN_5,        ///< #5 warning
    EXTERNAL_WARN_6,        ///< #6 warning
    EXTERNAL_WARN_7,        ///< #7 warning
    EXTERNAL_WARN_8,        ///< #8 warning
    EXTERNAL_WARN_9,        ///< #9 warning
    EXTERNAL_WARN_10,       ///< #10 warning
    EXTERNAL_WARN_11,       ///< #11 warning
    EXTERNAL_WARN_12,       ///< #12 warning
    EXTERNAL_WARN_13,       ///< #13 warning
    EXTERNAL_WARN_14,       ///< #14 warning
    EXTERNAL_WARN_15,       ///< #15 warning
    EXTERNAL_WARN_16,       ///< #16 warning
    EXTERNAL_WARN_17,       ///< #17 warning
    EXTERNAL_WARN_18,       ///< #18 warning
    EXTERNAL_WARN_19,       ///< #19 warning
    EXTERNAL_WARN_20,       ///< #20 warning
    EXTERNAL_WARN_21,       ///< #21 warning
    EXTERNAL_WARN_22,       ///< #22 warning
    EXTERNAL_WARN_23,       ///< #23 warning
    EXTERNAL_WARN_24,       ///< #24 warning
    EXTERNAL_WARN_25,       ///< #25 warning
    EXTERNAL_WARN_26,       ///< #26 warning
    EXTERNAL_WARN_27,       ///< #27 warning
    EXTERNAL_WARN_28,       ///< #28 warning
    EXTERNAL_WARN_29,       ///< #29 warning
    EXTERNAL_WARN_30,       ///< #30 warning
    EXTERNAL_WARN_31,       ///< #31 warning
    EXTERNAL_WARN_32,       ///< #32 warning
    EXTERNAL_WARN_33,       ///< #33 warning
    EXTERNAL_WARN_34,       ///< #34 warning
    EXTERNAL_WARN_35,       ///< #35 warning
    EXTERNAL_WARN_COMM,     ///< Communication error

    EXTERNAL_WARN_COUNT     ///< Warnings count
} ExternalWarnType;

typedef struct
{
    ExternalEventType type;
    int arg1;
    int arg2;
    int arg3;

} ExternalEvent;

/**
 * Machine status definition.
 */
typedef struct
{
    int powerStatus;
    ExternalMode mode;
    float indoorTemperature;
    float outdoorTemperature;
    float exhaustTemperature;
    float temperature;
    float evaporatorTemperature;
    float condenserTemperature;
    float returnAirTemperature;
    int compressorTargetFreq;
    int compressorActualFreq;
    int wifiStrength;
    int radioID0;
    int radioID1;
    int radioID2;
    float current;
    float voltage;
    int warnings[EXTERNAL_WARN_COUNT];
    int machineCodes[22];
    int powerMemroyStatus;

} ExternalMachine;

extern ExternalMachine extMachine;

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

/** @} */ // end of airconditioner_external

/** @defgroup airconditioner_power Power
 *  @{
 */
/**
 * Initializes power module.
 */
void PowerInit(void);

/**
 * Checks whether it is about time to power on or ff.
 *
 * @return 0 for net yet, -1 for power off currently, 1 for power on currently.
 */
int PowerCheck(void);

/** @} */ // end of airconditioner_power

/** @defgroup airconditioner_screen Screen
 *  @{
 */
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
 * @return 0 for net yet, otherwise for entering screensaver mode currently.
 */
int ScreenSaverCheck(void);

/**
 * Pause to count down the time to enter screensaver mode.
 */
void ScreenSaverPause(void);

/**
 * Continue to count down the time to enter screensaver mode.
 */
void ScreenSaverContinue(void);

/**
 * Is on screensaver mode or not.
 */
bool ScreenSaverIsScreenSaving(void);

/** @} */ // end of airconditioner_screen

/** @defgroup airconditioner_string String
 *  @{
 */
/**
 * Gets the description of warning.
 *
 * @param warn The warning type.
 * @return the string of warning description.
 */
const char* StringGetWarning(ExternalWarnType warn);

/**
 * Gets the string of date format.
 *
 * @return the string of date format.
 */
const char* StringGetDateFormat(void);

/**
 * Gets the string of power on.
 *
 * @return the string of power on.
 */
const char* StringGetPowerOn(void);

/**
 * Gets the string of power off.
 *
 * @return the string of power off.
 */
const char* StringGetPowerOff(void);

/** @} */ // end of airconditioner_string

/** @defgroup airconditioner_upgrade Upgrade
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
    QUIT_UPGRADE_WEB            ///< Quit to wait web upgrading
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

/** @} */ // end of airconditioner_upgrade

#ifdef __cplusplus
}
#endif

#endif /* AIRCONDITIONER_H */
/** @} */ // end of airconditioner
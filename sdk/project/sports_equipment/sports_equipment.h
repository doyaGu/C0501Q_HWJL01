/** @file
 * ITE SportsEquipment Modules.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup sports_equipment ITE SportsEquipment Modules
 *  @{
 */
#ifndef SPORTS_EQUIPMENT_H
#define SPORTS_EQUIPMENT_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup sports_equipment_audio Audio Player
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

/** @} */ // end of sports_equipment_audio

/** @defgroup sports_equipment_config Configuration
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
    // display
    int brightness;                                                     ///< Brightness, the range is 0~9
    int lang;                                                           ///< Language type @see LangType
    int demo_enable;                                                    ///< In demo mode or not

    // sound
    int audiolevel;                                                     ///< Audio volume percentage, range is 0~100

    // sports_equipment
    int distance;
    int calorie;
    int goal_time;
	int pace;
    int slope;
    int speed;
    int age;
    int weight;
    int pulse;
    int unit_mile;
    int unit_lbs;

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

/** @} */ // end of sports_equipment_config

/** @defgroup sports_equipment_external External
 *  @{
 */

typedef enum
{
    EXTERNAL_PULSE,         ///< Pulse
    EXTERNAL_WARN           ///< Show warning
} ExternalEventType;

typedef enum
{
    EXTERNAL_WARN_NONE,         ///< Hide warning
    EXTERNAL_WARN_SAFETY        ///< Show safety warning
} ExternalWarnType;

typedef struct
{
    ExternalEventType type;
    int arg1;
    int arg2;
    int arg3;

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
 * Checks external module event.
 *
 * @param ev The external event.
 * @return 0 for no event yet, otherwise for event occured.
 */
int ExternalCheck(ExternalEvent* ev);

/** @} */ // end of sports_equipment_external

/** @defgroup sports_equipment_program Program
 *  @{
 */
#define PROG_MIN_TIME        1
#define PROG_MIN_DISTANCE    1
#define PROG_MIN_CALORIE     100
#define PROG_MIN_PULSE       89

typedef enum
{
    PROG_AEROBIC,           ///< Aerobic
    PROG_INTERMITTENT,      ///< Intermittent
    PROG_TIME,              ///< Time
    PROG_DISTANCE,          ///< Distance
    PROG_MARATHON,          ///< Marathon
    PROG_HALF_MARATHON,     ///< Half Marathon
    PROG_FAT_BURNING,       ///< Fat Burning
    PROG_BUFFER,            ///< Buffer
    PROG_CALORIE,           ///< Calorie
    PROG_PULSE,             ///< Pulse
    PROG_QUARTER_MARATHON,  ///< Quarter Marathon
    PROG_MINI_MARATHON      ///< Mini Marathon
} ProgramType;

typedef struct
{
    ProgramType prog;
    int time;
    int distance;
    int calorie;
    int pulse;
	int pace;
    bool restart;

} Program;

/**
 * Global instance variable of program.
 */
extern Program theProgram;

/**
* Initializes program module.
*/
void ProgramInit(void);

/**
 * Sets the program type.
 *
 * @param prog The program type.
 */
void ProgramSetType(ProgramType prog);

/** @} */ // end of sports_equipment_external

/** @defgroup sports_equipment_screen Screen
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

/** @} */ // end of sports_equipment_screen

/** @defgroup sports_equipment_storage Storage
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

/** @} */ // end of sports_equipment_storage

/** @defgroup sports_equipment_upgrade Upgrade
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
    QUIT_UPGRADE_RESOURCE       ///< Quit to upgrade resources
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
 * Processes the upgrade procedure by QuitValue.
 *
 * @param code The QuitValue.
 * @return 0 for process success; otherwise failed.
 */
int UpgradeProcess(int code);

/** @} */ // end of sports_equipment_upgrade

#ifdef __cplusplus
}
#endif

#endif /* SPORTS_EQUIPMENT_H */
/** @} */ // end of sports_equipment
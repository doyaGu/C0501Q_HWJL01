/** @file
 * Template project definition.
 *
 */
/** @defgroup template
 *  @{
 */
#ifndef PROJECT_H
#define PROJECT_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup template_audio Audio Player
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
int AudioGetVolume(void);

/** @} */ // end of template_audio

/** @defgroup template_config Configuration
 *  @{
 */
typedef struct
{
    // network
    int     dhcp;               ///< Enable DHCP or not
    char    ipaddr[16];         ///< IP address
    char    netmask[16];        ///< Netmask
    char    gw[16];             ///< Gateway address
    char    dns[16];            ///< DNS address
    
    // sound
    char    keysound[PATH_MAX]; ///< Key sound file path
    int     keylevel;           ///< Key volume percentage, range is 0~100
    int     audiolevel;         ///< Audio volume percentage, range is 0~100
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

/** @} */ // end of template_config

/** @defgroup template_network Network
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

/** @} */ // end of template_network

/** @defgroup template_upgrade Upgrade
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

/** @} */ // end of template_upgrade

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_H */

/** @} */ // end of template

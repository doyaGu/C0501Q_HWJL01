/** @file
 * ITE AirConditioner Scene Definition.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup airconditioner ITE AirConditioner Modules
 *  @{
 */
#ifndef SCENE_H
#define SCENE_H

#include "ite/itu.h"
#include "airconditioner.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup airconditioner_scene Scene
 *  @{
 */

#define MS_PER_FRAME                17              ///< Drawing delay per frame

/**
 * Custom event definition.
 */
typedef enum
{
	EVENT_CUSTOM_SCREENSAVER = ITU_EVENT_CUSTOM     ///< Ready to enter screensaver mode. Custom0 event on GUI Designer.

} CustomEvent;

// scene
/**
 * Initializes scene module.
 */
void SceneInit(void);

/**
 * Exits scene module.
 */
void SceneExit(void);

/**
 * Loads ITU file.
 */
void SceneLoad(void);

/**
 * Runs the main loop to receive events, update and draw scene.
 *
 * @return The QuitValue.
 */
int SceneRun(void);

/**
 * Sets the status of scene.
 *
 * @param ready true for ready, false for not ready yet.
 */
void SceneSetReady(bool ready);

/**
 * Quits the scene.
 *
 * @param value The reason to quit the scene.
 */
void SceneQuit(QuitValue value);

/**
 * Gets the current quit value.
 *
 * @return The current quit value.
 */
QuitValue SceneGetQuitValue(void);

/**
 * Changes language file.
 */
void SceneChangeLanguage(void);

// main
/**
 * Processes external event.
 *
 * @param ev The external event to process.
 */
void MainProcessExternalEvent(ExternalEvent* ev);

/**
 * Refresh time.
 */
void MainRefreshTime(void);

/**
 * Power on.
 */
void MainPowerOn(void);

/**
 * Power off.
 */
void MainPowerOff(void);

// setting
/**
 * Called on leaving.
 *
 * @param before Called before playing animation.
 */
void SettingOnLeave(bool before);

/**
 * Called per second.
 */
void SettingOnSecond(void);

/**
 * Processes external event.
 *
 * @param ev The external event to process.
 */
void SettingProcessExternalEvent(ExternalEvent* ev);

/**
 * Power on.
 */
void SettingPowerOn(void);

/**
 * Power off.
 */
void SettingPowerOff(void);

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

/** @} */ // end of airconditioner_scene

#ifdef __cplusplus
}
#endif

#endif /* SCENE_H */
/** @} */ // end of airconditioner
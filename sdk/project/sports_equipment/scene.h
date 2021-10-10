/** @file
 * ITE SportsEquipment Scene Definition.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup sports_equipment ITE SportsEquipment Modules
 *  @{
 */
#ifndef SCENE_H
#define SCENE_H

#include "ite/itu.h"
#include "sports_equipment.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup sports_equipment_scene Scene
 *  @{
 */

#define MS_PER_FRAME                17              ///< Drawing delay per frame

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
* External storage device is inserted.
*
* @param path the root path of external storage device.
*/
void MainStorageOnInserted(char* path);

/**
* External storage device is removed.
*/
void MainStorageOnRemoved(void);

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

/** @} */ // end of sports_equipment_scene

#ifdef __cplusplus
}
#endif

#endif /* SCENE_H */
/** @} */ // end of sports_equipment
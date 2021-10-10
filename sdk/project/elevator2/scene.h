/** @file
 * ITE Elevator Scene Definition.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup elevator ITE Elevator Modules
 *  @{
 */
#ifndef SCENE_H
#define SCENE_H

#include "ite/itu.h"
#include "elevator.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup elevator_scene Scene
 *  @{
 */

#define MS_PER_FRAME                25              ///< Drawing delay per frame

typedef enum
{
	EVENT_CUSTOM_GOTO_SETTING = ITU_EVENT_CUSTOM    ///< Ready to enter setting page. Custom0 event on GUI Designer.

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
 * Gets the current orientation.
 *
 * @return The current orientation.
 */
bool SceneIsVertical(void);

/**
 * Changes language file.
 */
void SceneChangeLanguage(void);

/**
 * Updates by configuration.
 */
void SceneUpdateByConfig(void);

// main
/**
 * Processes external event.
 *
 * @param ev The external event to process.
 */
void MainProcessExternalEvent(ExternalEvent* ev);

/**
 * Updates by configuration.
 */
void MainUpdateByConfig(void);

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

/** @} */ // end of elevator_scene

#ifdef __cplusplus
}
#endif

#endif /* SCENE_H */
/** @} */ // end of elevator
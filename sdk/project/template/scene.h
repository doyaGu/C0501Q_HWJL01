/** @file
 * Template scene definition.
 *
 */
/** @defgroup template
 *  @{
 */
#ifndef SCENE_H
#define SCENE_H

#include "ite/itu.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup template_scene
 *  @{
 */

#define MS_PER_FRAME    17  ///< Drawing delay per frame

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

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

/** @} */ // end of template_scene

#ifdef __cplusplus
}
#endif

#endif /* SCENE_H */

/** @} */ // end of template

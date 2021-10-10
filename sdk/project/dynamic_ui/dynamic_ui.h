#ifndef DYNAMIC_UI_H
#define DYNAMIC_UI_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "ite/itu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BUTTON_COUNT 10

/**
 * Configuration definition.
 */
typedef struct
{
    int btn_visible[MAX_BUTTON_COUNT];
    int btn_tabindex[MAX_BUTTON_COUNT];

} Config;

/**
 * Global instance variable of configuration.
 */
extern Config theConfig;

/**
 * Loads configuration file.
 */
void ConfigLoad(void);

/**
 * Saves the configuration to file.
 */
void ConfigSave(void);

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
 * @return The quit value.
 */
int SceneRun(void);

/**
 * Global instance variable of scene.
 */
extern ITUScene theScene;

#ifdef __cplusplus
}
#endif

#endif /* DYNAMIC_UI_H */

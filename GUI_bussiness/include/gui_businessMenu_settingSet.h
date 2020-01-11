/**
 * @file gui_businessMenu_settingSet.h
 *
 */

#ifndef GUI_BUSINESSMENU_SETTINGSET_H
#define GUI_BUSINESSMENU_SETTINGSET_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "iot_lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void appUiElementSet_upgradeAvailable(bool val);

void lvGui_businessMenu_settingSet(lv_obj_t * obj_Parent);

void guiDispTimeOut_pageSettingSet(void);

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_SETTINGSET_H*/




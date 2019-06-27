/**
 * @file gui_businessMenu_wifiConfig.h
 *
 */

#ifndef GUI_BUSINESSMENU_WIFICONFIG_H
#define GUI_BUSINESSMENU_WIFICONFIG_H

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

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lvGui_businessMenu_wifiConfig(lv_obj_t * obj_Parent);

void lvGui_wifiConfig_bussiness_configComplete_tipsTrig(void);
void lvGui_wifiConfig_bussiness_configComplete_tipsDetect(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_WIFICONFIG_H*/



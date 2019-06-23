/**
 * @file gui_businessMenu_delayer.h
 *
 */

#ifndef GUI_BUSINESSMENU_DELAYER_H
#define GUI_BUSINESSMENU_DELAYER_H

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
void lvGui_businessMenu_delayer(lv_obj_t * obj_Parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_DELAYER_H*/



/**
 * @file gui_businessMenu_delayerSet_C.h
 *
 */

#ifndef GUI_BUSINESSMENU_DELAYERSET_H
#define GUI_BUSINESSMENU_DELAYERSET_H

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

void lvGui_businessMenu_delayerSet(lv_obj_t * obj_Parent);

void guiDispTimeOut_pageDelayerSet(void);

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_DELAYERSET_H*/






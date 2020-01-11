/**
 * @file gui_businessMenu_timerSet.h
 *
 */

#ifndef GUI_BUSINESSMENU_TIMER_SET_H
#define GUI_BUSINESSMENU_TIMER_SET_H

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

void lvGui_businessMenu_timerSet(lv_obj_t * obj_Parent);

void guiDispTimeOut_pageTimerSet(void);

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_TIMER_SET_H*/





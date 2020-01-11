/**
 * @file gui_businessMenu_other.h
 *
 */

#ifndef GUI_BUSINESSMENU_OTHER_H
#define GUI_BUSINESSMENU_OTHER_H

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
void lvGuiOther_devInfoRefresh(void);
void lvGui_businessMenu_other(lv_obj_t * obj_Parent);

void guiDispTimeOut_pageOther(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSMENU_OTHER_H*/



/**
 * @file gui_businessHome.h
 *
 */

#ifndef GUI_BUSINESSHOME_H
#define GUI_BUSINESSHOME_H

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
#define LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK			0x0A
#define LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME			0x0B

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	bussinessType_Home = 0,
	bussinessType_Menu,
	bussinessType_menuPageOther,
	bussinessType_menuPageDelayer,
	bussinessType_menuPageTimer,
	bussinessType_menuPageLinkageConfig,
	bussinessType_menuPageSetting,
	bussinessType_menuPageWifiConfig,
}usrGuiBussiness_type;

enum{

	homepageThemeType_jianJie = 0,
	homepageThemeType_keAi,
	homepageThemeType_ouZhou,
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

extern bool volatile task_guiSwitch_Detecting_runningFlg;

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lvGui_businessInit(void);
void lvGui_usrSwitch(usrGuiBussiness_type guiPage);

void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF);
uint8_t usrAppHomepageThemeType_Get(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSHOME_H*/


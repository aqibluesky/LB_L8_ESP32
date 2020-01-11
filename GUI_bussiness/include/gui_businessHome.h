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

#define BROUND_PIC_SELECT_MAX_NUM					(28 - 1)

#define L8_STARTUP_PIC_LOGO_ROUTINE					0
#define L8_STARTUP_PIC_LOGO_SPECIFY_MERIDSMAER		0x0A

#define L8_STARTUP_PIC_LOGO_DEF						L8_STARTUP_PIC_LOGO_ROUTINE


/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	bussinessType_null = 0,
	bussinessType_Home,
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

enum{

	bGroudImg_objInsert_usrPic = 0,

	bGroudImg_objInsert_figureA,
	bGroudImg_objInsert_figureB,
	bGroudImg_objInsert_figureC,
	bGroudImg_objInsert_figureD,
	bGroudImg_objInsert_figureE,
	bGroudImg_objInsert_figureF,
	bGroudImg_objInsert_figureG,
	bGroudImg_objInsert_figureH,
	bGroudImg_objInsert_figureI,
	bGroudImg_objInsert_figureJ,
	bGroudImg_objInsert_figureK,
	bGroudImg_objInsert_figureL,
	bGroudImg_objInsert_figureM,
	bGroudImg_objInsert_figureN,
	bGroudImg_objInsert_figureO,
	bGroudImg_objInsert_figureP,
	bGroudImg_objInsert_figureQ,
	bGroudImg_objInsert_figureR,
	bGroudImg_objInsert_figureS,
	bGroudImg_objInsert_figureT,

	bGroudImg_objInsert_pureColor1,
	bGroudImg_objInsert_pureColor2,
	bGroudImg_objInsert_pureColor3,
	bGroudImg_objInsert_pureColor4,
	bGroudImg_objInsert_pureColor5,
	bGroudImg_objInsert_pureColor6,
	bGroudImg_objInsert_pureColor7,
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern SemaphoreHandle_t xSph_lvglOpreat;

extern bool volatile task_guiSwitch_Detecting_runningFlg;

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lvGui_businessInit(void);
void lvGui_usrSwitch(usrGuiBussiness_type guiPage);

lv_style_t *usrAppHomepageBkPicStyle_prevDataGet(uint8_t ist);
lv_img_dsc_t *usrAppHomepageBkPic_prevDataGet(uint8_t ist);

void pageHome_buttonMain_imageRefresh(bool freshNoRecord);

void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF);
uint8_t usrAppHomepageThemeType_Get(void);

void usrAppHomepageBgroundPicOrg_Set(const uint8_t picIst, bool nvsRecord_IF, bool refresh_IF);
uint8_t usrAppHomepageBgroundPicOrg_Get(void);

void lvGui_usrAppBussinessRunning_block(uint8_t iconType, const char *strTips, uint8_t timeOut);
void lvGui_usrAppBussinessRunning_blockCancel(void);

void lvGui_tipsFullScreen_generate(const char *strTips, uint16_t timeOut);
void lvGui_tipsFullScreen_generateAutoTime(const char *strTips);
void lvGui_tipsFullScreen_distruction(void);

void usrApp_fullScreenRefresh_self(uint16_t freshTime, lv_coord_t y);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSHOME_H*/


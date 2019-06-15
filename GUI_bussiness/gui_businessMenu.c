/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

// /* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "sdkconfig.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu.h"
#include "gui_businessHome.h"

LV_FONT_DECLARE(lv_font_dejavu_15)

LV_IMG_DECLARE(iconMenu_af);
LV_IMG_DECLARE(iconMenu_cj);
LV_IMG_DECLARE(iconMenu_sbgl);
LV_IMG_DECLARE(iconMenu_sz);
LV_IMG_DECLARE(iconMenu_tj);
LV_IMG_DECLARE(iconMenu_sbxx);
LV_IMG_DECLARE(iconMenu_bkPR);
LV_IMG_DECLARE(iconMenu_bkREL);
LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B;

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_Home);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_deviceManage(lv_obj_t *btn){

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_scence(lv_obj_t *btn){

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_security(lv_obj_t *btn){

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_statistics(lv_obj_t *btn){

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_setting(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageSetting);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_deviceInfo(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageDeviceInfo);
	
	return LV_RES_OK;
}

void lvGui_businessMenu(lv_obj_t * obj_Parent){

	lv_obj_t * menuBtnChoIcon_deviceManage;
	lv_obj_t * menuBtnChoIcon_scence;
	lv_obj_t * menuBtnChoIcon_security;
	lv_obj_t * menuBtnChoIcon_statistics;
	lv_obj_t * menuBtnChoIcon_setting;
	lv_obj_t * menuBtnChoIcon_deviceInfo;
	
	lv_obj_t * menuBtnChoIcon_fun_back;

	lv_obj_t * menuIconCho_deviceManage;
	lv_obj_t * menuIconCho_scence;
	lv_obj_t * menuIconCho_security;
	lv_obj_t * menuIconCho_statistics;
	lv_obj_t * menuIconCho_setting;
	lv_obj_t * menuIconCho_deviceInfo;

	lv_obj_t *text_Title;

	lv_obj_t * menuTextCho_deviceManage;
	lv_obj_t * menuTextCho_scence;
	lv_obj_t * menuTextCho_security;
	lv_obj_t * menuTextCho_statistics;
	lv_obj_t * menuTextCho_setting;
	lv_obj_t * menuTextCho_deviceInfo;

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_30;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "Menu");
	lv_obj_set_pos(text_Title, 90, 35);
	lv_obj_set_style(text_Title, &styleText_menuLevel_A);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 30);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);

	menuBtnChoIcon_deviceManage = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_deviceManage, LV_BTN_STATE_REL, &iconMenu_bkREL);
	lv_imgbtn_set_src(menuBtnChoIcon_deviceManage, LV_BTN_STATE_PR, &iconMenu_bkPR);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 5, 74);
	menuBtnChoIcon_scence = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_deviceManage);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 121, 74);
	menuBtnChoIcon_security = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_deviceManage);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 5, 156);
	menuBtnChoIcon_statistics = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_deviceManage);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 121, 156);
	menuBtnChoIcon_setting = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_deviceManage);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 5, 238);
	menuBtnChoIcon_deviceInfo = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_deviceManage);
	lv_obj_set_pos(menuBtnChoIcon_deviceManage, 121, 238);

	lv_obj_animate(menuBtnChoIcon_scence, 		LV_ANIM_FLOAT_LEFT, 100,   0, NULL);
	lv_obj_animate(menuBtnChoIcon_statistics, 	LV_ANIM_FLOAT_LEFT, 100,  50, NULL);
	lv_obj_animate(menuBtnChoIcon_deviceInfo, 	LV_ANIM_FLOAT_LEFT, 100, 100, NULL);
	lv_obj_animate(menuBtnChoIcon_security, 	LV_ANIM_FLOAT_LEFT, 200,  50, NULL);
	lv_obj_animate(menuBtnChoIcon_setting, 		LV_ANIM_FLOAT_LEFT, 200, 100, NULL);
	lv_obj_animate(menuBtnChoIcon_deviceManage,	LV_ANIM_FLOAT_LEFT, 200, 150, NULL);
	
	lv_btn_set_action(menuBtnChoIcon_deviceManage, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_deviceManage);
	lv_btn_set_action(menuBtnChoIcon_scence, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_scence);
	lv_btn_set_action(menuBtnChoIcon_security, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_security);
	lv_btn_set_action(menuBtnChoIcon_statistics, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_statistics);
	lv_btn_set_action(menuBtnChoIcon_setting, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_setting);
	lv_btn_set_action(menuBtnChoIcon_deviceInfo, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_deviceInfo);

	menuIconCho_deviceManage = lv_img_create(menuBtnChoIcon_deviceManage, NULL);
	lv_img_set_src(menuIconCho_deviceManage, &iconMenu_sbgl);
	menuIconCho_scence = lv_img_create(menuBtnChoIcon_scence, NULL);
	lv_img_set_src(menuIconCho_scence, &iconMenu_cj);
	menuIconCho_security = lv_img_create(menuBtnChoIcon_security, NULL);
	lv_img_set_src(menuIconCho_security, &iconMenu_af);
	menuIconCho_statistics = lv_img_create(menuBtnChoIcon_statistics, NULL);
	lv_img_set_src(menuIconCho_statistics, &iconMenu_tj);
	menuIconCho_setting = lv_img_create(menuBtnChoIcon_setting, NULL);
	lv_img_set_src(menuIconCho_setting, &iconMenu_sz);
	menuIconCho_deviceInfo = lv_img_create(menuBtnChoIcon_deviceInfo, NULL);
	lv_img_set_src(menuIconCho_deviceInfo, &iconMenu_sbxx);
	lv_obj_set_protect(menuIconCho_deviceManage, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_scence, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_security, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_statistics, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_setting, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_deviceInfo, LV_PROTECT_POS);
	lv_obj_align(menuIconCho_deviceManage, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_scence, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_security, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_statistics, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_setting, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_deviceInfo, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

	lv_style_copy(&styleText_menuLevel_B, &lv_style_plain);
	styleText_menuLevel_B.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B.text.color = LV_COLOR_WHITE;
	
	menuTextCho_deviceManage = lv_label_create(menuBtnChoIcon_deviceManage, NULL);
	lv_label_set_text(menuTextCho_deviceManage, "Device\nManage");
	lv_obj_set_style(menuTextCho_deviceManage, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_deviceManage, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_deviceManage, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);

	menuTextCho_setting = lv_label_create(menuBtnChoIcon_setting, NULL);
	lv_label_set_text(menuTextCho_setting, "Setting");
	lv_obj_set_style(menuTextCho_setting, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_setting, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_setting, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);

	menuTextCho_statistics = lv_label_create(menuBtnChoIcon_statistics, NULL);
	lv_label_set_text(menuTextCho_statistics, "Statistics");
	lv_obj_set_style(menuTextCho_statistics, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_statistics, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_statistics, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);

	menuTextCho_security = lv_label_create(menuBtnChoIcon_security, NULL);
	lv_label_set_text(menuTextCho_security, "Security");
	lv_obj_set_style(menuTextCho_security, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_security, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_security, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);

	menuTextCho_scence = lv_label_create(menuBtnChoIcon_scence, NULL);
	lv_label_set_text(menuTextCho_scence, "Scence");
	lv_obj_set_style(menuTextCho_scence, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_scence, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_scence, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);

	menuTextCho_deviceInfo = lv_label_create(menuBtnChoIcon_deviceInfo, NULL);
	lv_label_set_text(menuTextCho_deviceInfo, "Device\ninfo");
	lv_obj_set_style(menuTextCho_deviceInfo, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_deviceInfo, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_deviceInfo, NULL, LV_ALIGN_IN_LEFT_MID, 52, 5);
}



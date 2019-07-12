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

LV_IMG_DECLARE(iconMenu_linkageConfig);
LV_IMG_DECLARE(iconMenu_timer);
LV_IMG_DECLARE(iconMenu_delayer);
LV_IMG_DECLARE(iconMenu_wifiConfig);
LV_IMG_DECLARE(iconMenu_other);
LV_IMG_DECLARE(iconMenu_setting);
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

static lv_res_t funCb_btnActionClick_menuBtn_other(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageOther);
	
	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_delayer(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageDelayer);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_timer(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageTimer);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_linkageConfig(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageLinkageConfig);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_setting(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageSetting);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_menuBtn_wifiConfig(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_menuPageWifiConfig);
	
	return LV_RES_OK;
}

void lvGui_businessMenu(lv_obj_t * obj_Parent){

	lv_obj_t * menuBtnChoIcon_other;
	lv_obj_t * menuBtnChoIcon_delayer;
	lv_obj_t * menuBtnChoIcon_timer;
	lv_obj_t * menuBtnChoIcon_linkageConfig;
	lv_obj_t * menuBtnChoIcon_setting;
	lv_obj_t * menuBtnChoIcon_wifiConfig;
	
	lv_obj_t * menuBtnChoIcon_fun_back;

	lv_obj_t * menuIconCho_other;
	lv_obj_t * menuIconCho_delayer;
	lv_obj_t * menuIconCho_timer;
	lv_obj_t * menuIconCho_linkageConfig;
	lv_obj_t * menuIconCho_setting;
	lv_obj_t * menuIconCho_wifiConfig;

	lv_obj_t * text_Title;

	lv_obj_t * menuTextCho_other;
	lv_obj_t * menuTextCho_delayer;
	lv_obj_t * menuTextCho_timer;
	lv_obj_t * menuTextCho_linkageConfig;
	lv_obj_t * menuTextCho_setting;
	lv_obj_t * menuTextCho_wifiConfig;

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

	lv_obj_animate(text_Title, 		 		LV_ANIM_FLOAT_LEFT, 100,   0, NULL);
	lv_obj_animate(menuBtnChoIcon_fun_back, LV_ANIM_FLOAT_LEFT, 100,  50, NULL);

	menuBtnChoIcon_other = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_other, LV_BTN_STATE_REL, &iconMenu_bkREL);
	lv_imgbtn_set_src(menuBtnChoIcon_other, LV_BTN_STATE_PR, &iconMenu_bkPR);
	lv_obj_set_pos(menuBtnChoIcon_other, 5, 74);
	menuBtnChoIcon_delayer = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_other);
	lv_obj_set_pos(menuBtnChoIcon_other, 121, 74);
	menuBtnChoIcon_timer = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_other);
	lv_obj_set_pos(menuBtnChoIcon_other, 5, 156);
	menuBtnChoIcon_linkageConfig = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_other);
	lv_obj_set_pos(menuBtnChoIcon_other, 121, 156);
	menuBtnChoIcon_setting = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_other);
	lv_obj_set_pos(menuBtnChoIcon_other, 5, 238);
	menuBtnChoIcon_wifiConfig = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_other);
	lv_obj_set_pos(menuBtnChoIcon_other, 121, 238);

	lv_obj_animate(menuBtnChoIcon_delayer, 		 LV_ANIM_FLOAT_LEFT, 100,   0, NULL);
	lv_obj_animate(menuBtnChoIcon_linkageConfig, LV_ANIM_FLOAT_LEFT, 100,  50, NULL);
	lv_obj_animate(menuBtnChoIcon_wifiConfig, 	 LV_ANIM_FLOAT_LEFT, 100, 100, NULL);
	lv_obj_animate(menuBtnChoIcon_timer, 		 LV_ANIM_FLOAT_LEFT, 200,  50, NULL);
	lv_obj_animate(menuBtnChoIcon_setting, 		 LV_ANIM_FLOAT_LEFT, 200, 100, NULL);
	lv_obj_animate(menuBtnChoIcon_other,		 LV_ANIM_FLOAT_LEFT, 200, 150, NULL);
	
	lv_btn_set_action(menuBtnChoIcon_other, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_other);
	lv_btn_set_action(menuBtnChoIcon_delayer, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_delayer);
	lv_btn_set_action(menuBtnChoIcon_timer, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_timer);
	lv_btn_set_action(menuBtnChoIcon_linkageConfig, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_linkageConfig);
	lv_btn_set_action(menuBtnChoIcon_setting, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_setting);
	lv_btn_set_action(menuBtnChoIcon_wifiConfig, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_wifiConfig);

	menuIconCho_other = lv_img_create(menuBtnChoIcon_other, NULL);
	lv_img_set_src(menuIconCho_other, &iconMenu_other);
	menuIconCho_delayer = lv_img_create(menuBtnChoIcon_delayer, NULL);
	lv_img_set_src(menuIconCho_delayer, &iconMenu_delayer);
	menuIconCho_timer = lv_img_create(menuBtnChoIcon_timer, NULL);
	lv_img_set_src(menuIconCho_timer, &iconMenu_timer);
	menuIconCho_linkageConfig = lv_img_create(menuBtnChoIcon_linkageConfig, NULL);
	lv_img_set_src(menuIconCho_linkageConfig, &iconMenu_linkageConfig);
	menuIconCho_setting = lv_img_create(menuBtnChoIcon_setting, NULL);
	lv_img_set_src(menuIconCho_setting, &iconMenu_setting);
	menuIconCho_wifiConfig = lv_img_create(menuBtnChoIcon_wifiConfig, NULL);
	lv_img_set_src(menuIconCho_wifiConfig, &iconMenu_wifiConfig);
	lv_obj_set_protect(menuIconCho_other, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_delayer, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_timer, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_linkageConfig, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_setting, LV_PROTECT_POS);
	lv_obj_set_protect(menuIconCho_wifiConfig, LV_PROTECT_POS);
	lv_obj_align(menuIconCho_other, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);
	lv_obj_align(menuIconCho_delayer, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_timer, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_linkageConfig, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_setting, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
	lv_obj_align(menuIconCho_wifiConfig, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);

	lv_style_copy(&styleText_menuLevel_B, &lv_style_plain);
	styleText_menuLevel_B.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B.text.color = LV_COLOR_WHITE;
	
	menuTextCho_other = lv_label_create(menuBtnChoIcon_other, NULL);
	lv_label_set_text(menuTextCho_other, "system\ninfo");
	lv_obj_set_style(menuTextCho_other, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_other, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_other, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	menuTextCho_setting = lv_label_create(menuBtnChoIcon_setting, NULL);
	lv_label_set_text(menuTextCho_setting, "Setting");
	lv_obj_set_style(menuTextCho_setting, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_setting, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_setting, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	menuTextCho_linkageConfig = lv_label_create(menuBtnChoIcon_linkageConfig, NULL);
	lv_label_set_text(menuTextCho_linkageConfig, "Linkage\nconfig");
	lv_obj_set_style(menuTextCho_linkageConfig, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_linkageConfig, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_linkageConfig, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	menuTextCho_timer = lv_label_create(menuBtnChoIcon_timer, NULL);
	lv_label_set_text(menuTextCho_timer, "Timer");
	lv_obj_set_style(menuTextCho_timer, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_timer, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_timer, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	menuTextCho_delayer = lv_label_create(menuBtnChoIcon_delayer, NULL);
	lv_label_set_text(menuTextCho_delayer, "Delayer");
	lv_obj_set_style(menuTextCho_delayer, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_delayer, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_delayer, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	menuTextCho_wifiConfig = lv_label_create(menuBtnChoIcon_wifiConfig, NULL);
	lv_label_set_text(menuTextCho_wifiConfig, "Wifi\nconfig");
	lv_obj_set_style(menuTextCho_wifiConfig, &styleText_menuLevel_B);
	lv_obj_set_protect(menuTextCho_wifiConfig, LV_PROTECT_POS);
	lv_obj_align(menuTextCho_wifiConfig, NULL, LV_ALIGN_IN_LEFT_MID, 55, 5);

	vTaskDelay(50 / portTICK_PERIOD_MS);
	lv_obj_refresh_style(obj_Parent);
}



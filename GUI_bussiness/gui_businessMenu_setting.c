/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "mdf_common.h"

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

#include "gui_businessMenu_setting.h"
#include "gui_businessHome.h"

#define LABEL_SETTING_NUM	6

LV_FONT_DECLARE(lv_font_dejavu_15);

LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static const char *TAG = "lanbon_L8 - menuSetting";

static const usrGuiBussiness_type settingChildOption[LABEL_SETTING_NUM] = {

//	bussinessType_menuPageSetting_A,
//	bussinessType_menuPageSetting_B,
//	bussinessType_menuPageSetting_C,
};
static const char *setting_label[LABEL_SETTING_NUM] = {

	"device define",
	"timer set",
	"delay set",
	"setting_C",
	"setting_D",
	"setting_E",
};

static lv_style_t styleText_menuLevel_A;
static lv_style_t styleBtn_listBtnPre;
static lv_style_t styleBtn_listBtnRel;
static lv_style_t styleList_menuSetting;

static lv_obj_t *text_Title;
static lv_obj_t *menuBtnChoIcon_fun_back;

static lv_obj_t *objPageSetting_menuList;
//static lv_obj_t * menuText_devMac;

static void currentGui_elementClear(void){

	lv_obj_del(objPageSetting_menuList);
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	currentGui_elementClear();
	lvGui_usrSwitch(bussinessType_Menu);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static lv_res_t funCb_listBtnSettingRelease(lv_obj_t *list_btn){

	uint8_t loop = 0;

	for(loop = 0; loop < LABEL_SETTING_NUM; loop ++){

		if(!strcmp(setting_label[loop], lv_list_get_btn_text(list_btn))){

			MDF_LOGI("menuSetting touch get:%d.\n", loop);

			if(loop == (LABEL_SETTING_NUM - 1))mdf_info_erase("ESP-MDF"); //mdf总信息擦除 -debug
			if(loop == (LABEL_SETTING_NUM - 2)){

				extern void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if);

				usrApp_devIptdrv_paramRecalibration_set(true);
			}

			if(settingChildOption[loop]){

				currentGui_elementClear();
				lvGui_usrSwitch(settingChildOption[loop]);
			}
			
			break;
		}
	}

	if(loop >= LABEL_SETTING_NUM)
		MDF_LOGI("menuSetting touch not identify:%s.\n", lv_list_get_btn_text(list_btn));

	return LV_RES_OK;
}

static void lvGuiSetting_objStyle_Init(void){

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_30;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleList_menuSetting, &lv_style_plain);
	styleList_menuSetting.body.main_color = LV_COLOR_BLACK;
	styleList_menuSetting.body.grad_color = LV_COLOR_BLACK;
	styleList_menuSetting.body.border.part = LV_BORDER_NONE;
	styleList_menuSetting.body.radius = 0;
	styleList_menuSetting.body.opa = LV_OPA_60;
	styleList_menuSetting.body.padding.hor = 3; 
	styleList_menuSetting.body.padding.inner = 8;	

    lv_style_copy(&styleBtn_listBtnRel, &lv_style_btn_rel);
    styleBtn_listBtnRel.body.main_color = LV_COLOR_GRAY;
    styleBtn_listBtnRel.body.grad_color = LV_COLOR_GRAY;
    styleBtn_listBtnRel.body.border.color = LV_COLOR_SILVER;
    styleBtn_listBtnRel.body.border.width = 1;
    styleBtn_listBtnRel.body.border.opa = LV_OPA_50;
    styleBtn_listBtnRel.body.radius = 0;
	styleBtn_listBtnRel.body.border.part = LV_BORDER_BOTTOM;

    lv_style_copy(&styleBtn_listBtnPre, &styleBtn_listBtnRel);
    styleBtn_listBtnPre.body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
    styleBtn_listBtnPre.body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
    styleBtn_listBtnPre.text.color = LV_COLOR_MAKE(0xbb, 0xd5, 0xf1);
}

void lvGui_businessMenu_setting(lv_obj_t * obj_Parent){

	lvGuiSetting_objStyle_Init();

	//--------------原父对象基础新增-----------------------//
	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "Setting");
	lv_obj_set_pos(text_Title, 90, 35);
	lv_obj_set_style(text_Title, &styleText_menuLevel_A);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 30);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);

	//--------------当前活动界面新增----------------------//
	objPageSetting_menuList = lv_list_create(lv_scr_act(), NULL);
	lv_obj_set_size(objPageSetting_menuList, 240, 245);
	lv_obj_set_pos(objPageSetting_menuList, 0, 75);
	lv_list_set_style(objPageSetting_menuList, LV_PAGE_STYLE_BG, &styleList_menuSetting);	 
	lv_list_set_style(objPageSetting_menuList, LV_PAGE_STYLE_SB, &styleList_menuSetting);	
	lv_list_set_sb_mode(objPageSetting_menuList, LV_SB_MODE_DRAG);	 

    lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BTN_REL, &styleBtn_listBtnRel);
    lv_list_set_style(objPageSetting_menuList, LV_LIST_STYLE_BTN_PR, &styleBtn_listBtnPre);

	for(uint8_t loop = 0; loop < LABEL_SETTING_NUM; loop ++){

		 lv_list_add(objPageSetting_menuList, NULL, setting_label[loop], funCb_listBtnSettingRelease);
	}

//	menuText_devMac = lv_label_create(bGround_obj, NULL);
//	lv_obj_set_protect(menuText_devMac, LV_PROTECT_POS);
//	lv_obj_align(menuText_devMac, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
//	lv_label_set_text(menuText_devMac, (const char *)str_devMacBuff);
//	lv_obj_set_style(menuText_devMac, &styleText_menuLevel_B_infoMac);

}



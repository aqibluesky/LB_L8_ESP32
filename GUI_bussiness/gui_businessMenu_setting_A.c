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

#include "devDriver_manage.h"

#include "gui_businessMenu_setting_A.h"
#include "gui_businessHome.h"
	
#define OBJ_DDLIST_DEVTYPE_FREENUM		1
#define OBJ_DDLIST_HPTHEME_FREENUM		2

LV_FONT_DECLARE(lv_font_dejavu_15);

LV_FONT_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static const char *deviceType_listTab = {

	"Switch-1bit\n"
	"Switch-2bit\n"
	"Switch-3bit\n"
	"Dimmer\n"
	"Fans\n"
	"Scenario\n"
	"Curtain\n"	
	"Heater\n"
	"Thermostat"
};

static const char *homepageThemeStyle_listTab = {

	"theme-A\n"
	"theme-B\n"
	"theme-C"
};

static const struct stt_deviceTypeTab_disp{

	devTypeDef_enum devType;
	uint8_t devInst_disp;
	
}deviceTypeTab_disp[DEVICE_TYPE_LIST_NUM] = {

	{devTypeDef_mulitSwOneBit, 		0},
	{devTypeDef_mulitSwTwoBit, 		1},
	{devTypeDef_mulitSwThreeBit, 	2},
	{devTypeDef_dimmer,				3},
	{devTypeDef_fans,				4},
	{devTypeDef_scenario,			5},
	{devTypeDef_curtain,			6},
	{devTypeDef_heater,				7},	
	{devTypeDef_thermostat,			8},	
};

static lv_style_t stylePage_funSetOption;
static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B;
static lv_style_t styleBk_objBground;
static lv_style_t styleDdlistSettingA_devType;
static lv_style_t bg_styleDevStatusRecordIF;
static lv_style_t indic_styleDevStatusRecordIF;
static lv_style_t knob_on_styleDevStatusRecordIF;
static lv_style_t knob_off_styleDevStatusRecordIF;

static lv_obj_t *menuBtnChoIcon_fun_back;

static lv_obj_t *page_funSetOption;
static lv_obj_t *bGround_obj;
static lv_obj_t *text_Title;
static lv_obj_t *textSettingA_deviceType;
static lv_obj_t	*ddlistSettingA_deviceType;
static lv_obj_t *textSettingA_homepageThemestyle;
static lv_obj_t	*ddlistSettingA_homepageThemestyle;
static lv_obj_t	*textSettingA_devStatusRecordIF;
static lv_obj_t	*swSettingA_devStatusRecordIF;

static void currentGui_elementClear(void){

//	lv_obj_del(bGround_obj);
	lv_obj_del(page_funSetOption);
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	currentGui_elementClear();
	lvGui_usrSwitch(bussinessType_menuPageSetting);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static lv_res_t funCb_ddlistDevTypeDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_DEVTYPE_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);
		for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

			if(ddlist_opSelect == deviceTypeTab_disp[loop].devInst_disp){

				stt_devDataPonitTypedef devDataPoint_temp = {0};

				currentDev_dataPointSet(&devDataPoint_temp, true, true, true);
				currentDev_typeSet(deviceTypeTab_disp[loop].devType, true);
				devDriverManageBussiness_deviceChangeRefresh(); //驱动更新
				break;
			}
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_ddlistHomepageThemeDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_HPTHEME_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);

		usrAppHomepageThemeType_Set(ddlist_opSelect, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_swDevStatusRecordIF(lv_obj_t *sw){

	uint8_t sw_action = (uint8_t)!lv_sw_get_state(sw);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	printf("swDevStatusRecordIF set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 1;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
	}
	else
	{
		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
	}

	return LV_RES_OK;
}

void lvGui_businessMenu_setting_A(lv_obj_t * obj_Parent){

	uint8_t loop = 0;
	devTypeDef_enum devType_Temp;
	uint8_t homepageThemeStyle_temp = 0;

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "device define");
	lv_obj_set_pos(text_Title, 40, 45); 
	lv_obj_set_style(text_Title, &styleText_menuLevel_A);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack_arrowLeft);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack_arrowLeft);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 45);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);

	page_funSetOption = lv_page_create(lv_scr_act(), NULL);
	lv_obj_set_size(page_funSetOption, 240, 320);
	lv_obj_set_pos(page_funSetOption, 0, 75);
	lv_style_copy(&stylePage_funSetOption, &lv_style_plain);
	stylePage_funSetOption.body.main_color = LV_COLOR_GRAY;
	stylePage_funSetOption.body.grad_color = LV_COLOR_GRAY;
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_SB, &stylePage_funSetOption);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_BG, &stylePage_funSetOption);
	lv_page_set_sb_mode(page_funSetOption, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(page_funSetOption, false, false); //key opration
	lv_page_set_scrl_width(page_funSetOption, 225);
	lv_page_set_scrl_height(page_funSetOption, 400);
	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);

	lv_style_copy(&styleText_menuLevel_B, &lv_style_plain);
	styleText_menuLevel_B.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B.text.color = LV_COLOR_WHITE;

	textSettingA_deviceType = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_deviceType, "device Type:");
	lv_obj_set_style(textSettingA_deviceType, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_deviceType, LV_PROTECT_POS);
	lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);

	lv_style_copy(&styleDdlistSettingA_devType, &lv_style_pretty);
	styleDdlistSettingA_devType.body.shadow.width = 1;
	styleDdlistSettingA_devType.text.color = LV_COLOR_MAKE(0x10, 0x20, 0x50);
	
	ddlistSettingA_deviceType = lv_ddlist_create(page_funSetOption, NULL);
	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_SB, &styleDdlistSettingA_devType);
	styleDdlistSettingA_devType.text.font = &lv_font_dejavu_15;
	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_BG, &styleDdlistSettingA_devType);
	lv_obj_set_protect(ddlistSettingA_deviceType, LV_PROTECT_POS);
	lv_obj_align(ddlistSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, 50, 25);
	lv_ddlist_set_options(ddlistSettingA_deviceType, deviceType_listTab);
	lv_ddlist_set_action(ddlistSettingA_deviceType, funCb_ddlistDevTypeDef);
	lv_obj_set_top(ddlistSettingA_deviceType, true);
	lv_obj_set_free_num(ddlistSettingA_deviceType, OBJ_DDLIST_DEVTYPE_FREENUM);
	devType_Temp = currentDev_typeGet();
	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(devType_Temp == deviceTypeTab_disp[loop].devType){

			lv_ddlist_set_selected(ddlistSettingA_deviceType, deviceTypeTab_disp[loop].devInst_disp);		
			break;
		}
	}

	textSettingA_devStatusRecordIF = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_devStatusRecordIF, "device status record:");
	lv_obj_set_style(textSettingA_devStatusRecordIF, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devStatusRecordIF, LV_PROTECT_POS);
	lv_obj_align(textSettingA_devStatusRecordIF, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);

	lv_style_copy(&bg_styleDevStatusRecordIF, &lv_style_pretty);
	bg_styleDevStatusRecordIF.body.radius = LV_RADIUS_CIRCLE;
	lv_style_copy(&indic_styleDevStatusRecordIF, &lv_style_pretty_color);
	indic_styleDevStatusRecordIF.body.radius = LV_RADIUS_CIRCLE;
	indic_styleDevStatusRecordIF.body.main_color = LV_COLOR_HEX(0x9fc8ef);
	indic_styleDevStatusRecordIF.body.grad_color = LV_COLOR_HEX(0x9fc8ef);
	indic_styleDevStatusRecordIF.body.padding.hor = 0;
	indic_styleDevStatusRecordIF.body.padding.ver = 0;	
	lv_style_copy(&knob_off_styleDevStatusRecordIF, &lv_style_pretty);
	knob_off_styleDevStatusRecordIF.body.radius = LV_RADIUS_CIRCLE;
	knob_off_styleDevStatusRecordIF.body.shadow.width = 4;
	knob_off_styleDevStatusRecordIF.body.shadow.type = LV_SHADOW_BOTTOM;
	lv_style_copy(&knob_on_styleDevStatusRecordIF, &lv_style_pretty_color);
	knob_on_styleDevStatusRecordIF.body.radius = LV_RADIUS_CIRCLE;
	knob_on_styleDevStatusRecordIF.body.shadow.width = 4;
	knob_on_styleDevStatusRecordIF.body.shadow.type = LV_SHADOW_BOTTOM;

	swSettingA_devStatusRecordIF = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_BG, &bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_INDIC, &indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_ON, &knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_OFF, &knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devStatusRecordIF, LV_PROTECT_POS);
	lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 25);
	lv_sw_set_anim_time(swSettingA_devStatusRecordIF, 100);
	stt_devStatusRecord devStatusRecordFlg_temp = {0};
	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);
	if(devStatusRecordFlg_temp.devStatusOnOffRecord_IF)lv_sw_on(swSettingA_devStatusRecordIF);
	else lv_sw_off(swSettingA_devStatusRecordIF);
	lv_sw_set_action(swSettingA_devStatusRecordIF, funCb_swDevStatusRecordIF);

	textSettingA_homepageThemestyle = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_homepageThemestyle, "theme:");
	lv_obj_set_style(textSettingA_homepageThemestyle, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_homepageThemestyle, LV_PROTECT_POS);
	lv_obj_align(textSettingA_homepageThemestyle, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);

	ddlistSettingA_homepageThemestyle = lv_ddlist_create(page_funSetOption, NULL);
	lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_SB, &styleDdlistSettingA_devType);
	styleDdlistSettingA_devType.text.font = &lv_font_dejavu_15;
	lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_BG, &styleDdlistSettingA_devType);
	lv_obj_set_protect(ddlistSettingA_homepageThemestyle, LV_PROTECT_POS);
	lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 25);
	lv_ddlist_set_options(ddlistSettingA_homepageThemestyle, homepageThemeStyle_listTab);
	lv_ddlist_set_action(ddlistSettingA_homepageThemestyle, funCb_ddlistHomepageThemeDef);
	lv_obj_set_top(ddlistSettingA_homepageThemestyle, true);
	lv_obj_set_free_num(ddlistSettingA_homepageThemestyle, OBJ_DDLIST_HPTHEME_FREENUM);
	homepageThemeStyle_temp = usrAppHomepageThemeType_Get();
	lv_ddlist_set_selected(ddlistSettingA_homepageThemestyle, homepageThemeStyle_temp);

	lv_page_glue_obj(textSettingA_deviceType, true);
	lv_page_glue_obj(ddlistSettingA_deviceType, true);
	lv_page_glue_obj(textSettingA_homepageThemestyle, true);
	lv_page_glue_obj(ddlistSettingA_homepageThemestyle, true);
	lv_page_glue_obj(textSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(swSettingA_devStatusRecordIF, true);
}



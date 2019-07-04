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

#include "gui_businessMenu_settingSet.h"
#include "gui_businessHome.h"
	
#define OBJ_DDLIST_DEVTYPE_FREENUM			1
#define OBJ_DDLIST_HPTHEME_FREENUM			2

#define FUNCTION_NUM_DEF_SCREENLIGHT_TIME	12

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);

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

static const char *mbox_btnm_textTab[] ={"Yes", "No", ""}; /*Button description. '\221' lv_btnm like control char*/

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

static const struct stt_gearScreenLightTime{

	uint16_t screenLightTime2Sec;
	char strDisp_ref[60];
	
}screenLightTimeGear_refTab[FUNCTION_NUM_DEF_SCREENLIGHT_TIME] = {

	{60 * 00 + 1 * 10, "#FFFF00 00##C0C0FF min##FFFF00 10##C0C0FF sec#"},
	{60 * 00 + 1 * 20, "#FFFF00 00##C0C0FF min##FFFF00 20##C0C0FF sec#"},
	{60 * 00 + 1 * 30, "#FFFF00 00##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 01 + 1 * 00, "#FFFF00 01##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 01 + 1 * 30, "#FFFF00 01##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 02 + 1 * 00, "#FFFF00 02##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 02 + 1 * 30, "#FFFF00 02##C0C0FF min##FFFF00 30##C0C0FF sec#"},
	{60 * 03 + 1 * 00, "#FFFF00 03##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 05 + 1 * 00, "#FFFF00 05##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 10 + 1 * 00, "#FFFF00 10##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 20 + 1 * 00, "#FFFF00 20##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{60 * 30 + 1 * 00, "#FFFF00 30##C0C0FF min##FFFF00 00##C0C0FF sec#"}
};

static const uint8_t screenBrightness_sliderAdj_div = 20;

static lv_style_t stylePage_funSetOption;
static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B;
static lv_style_t styleBk_objBground;
static lv_style_t styleDdlistSettingA_devType;
static lv_style_t bg_styleDevStatusRecordIF;
static lv_style_t indic_styleDevStatusRecordIF;
static lv_style_t knob_on_styleDevStatusRecordIF;
static lv_style_t knob_off_styleDevStatusRecordIF;
static lv_style_t styleBtn_transport;
static lv_style_t styleMbox_bg;
static lv_style_t styleMbox_btn_pr;
static lv_style_t styleMbox_btn_rel;
static lv_style_t styleBtn_specialTransparent;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *page_funSetOption = NULL;
static lv_obj_t *text_Title = NULL;
static lv_obj_t *textSettingA_deviceType = NULL;
static lv_obj_t	*textSettingA_devStatusRecordIF;
static lv_obj_t *textSettingA_homepageThemestyle = NULL;
static lv_obj_t *textSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *textSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textSettingA_screenBrightness = NULL;
static lv_obj_t *textSettingA_screenLightTime = NULL;
static lv_obj_t *btnSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *textBtnRef_factoryRecoveryIf = NULL;
static lv_obj_t *btnSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textBtnRef_touchRecalibrationIf = NULL;
static lv_obj_t	*ddlistSettingA_deviceType = NULL;
static lv_obj_t	*ddlistSettingA_homepageThemestyle = NULL;
static lv_obj_t	*swSettingA_devStatusRecordIF = NULL;
static lv_obj_t *mboxFactoryRecovery_comfirm = NULL;
static lv_obj_t *mboxTouchRecalibration_comfirm = NULL;
static lv_obj_t *sliderSettingA_screenBrightness = NULL;
static lv_obj_t *textBtnTimeRef_screenLightTime = NULL;
static lv_obj_t *btnTimeAdd_screenLightTime = NULL;
static lv_obj_t *textBtnTimeAdd_screenLightTime = NULL;
static lv_obj_t *btnTimeCut_screenLightTime = NULL;
static lv_obj_t *textBtnTimeCut_screenLightTime = NULL;

static uint8_t  functionGearScreenTime_ref = 0;

static void currentGui_elementClear(void){

	lv_obj_del(page_funSetOption);
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

static lv_res_t funCb_mboxBtnActionClick_factoryRecovery(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("factoryRecovery mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					mdf_info_erase("ESP-MDF");
					devSystemInfoLocalRecord_allErase();
					
					usrApplication_systemRestartTrig(5);


				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxFactoryRecovery_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_touchRecalibration(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("touchRecalibration mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					extern void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if);

					usrApp_devIptdrv_paramRecalibration_set(true);

					usrApplication_systemRestartTrig(5);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchRecalibration_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_functionSet_screenBrightnessAdj(lv_obj_t *slider){

	uint8_t devScreen_brightneesValSet = lv_slider_get_value(slider) * (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div);

	devScreenDriver_configParam_brightness_set(devScreen_brightneesValSet, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_screenLightTimeAdd(lv_obj_t *btn){

	if(functionGearScreenTime_ref < (FUNCTION_NUM_DEF_SCREENLIGHT_TIME - 1))
		functionGearScreenTime_ref ++;

	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_obj_refresh_style(textBtnTimeRef_screenLightTime);

	devScreenDriver_configParam_screenLightTime_set(screenLightTimeGear_refTab[functionGearScreenTime_ref].screenLightTime2Sec, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_functionSet_screenLightTimeCut(lv_obj_t *btn){

	if(functionGearScreenTime_ref)
		functionGearScreenTime_ref --;

	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_obj_refresh_style(textBtnTimeRef_screenLightTime);

	devScreenDriver_configParam_screenLightTime_set(screenLightTimeGear_refTab[functionGearScreenTime_ref].screenLightTime2Sec, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_factoryRecovery(lv_obj_t *btn){

	mboxFactoryRecovery_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxFactoryRecovery_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxFactoryRecovery_comfirm, NULL, LV_ALIGN_CENTER, -55, -55);
	lv_obj_set_width(mboxFactoryRecovery_comfirm, 230);
	lv_mbox_set_text(mboxFactoryRecovery_comfirm, "factory recovery opreat need to restart the devive, are you sure to continue?");
	lv_mbox_add_btns(mboxFactoryRecovery_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BG, &styleMbox_bg);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
	lv_mbox_set_style(mboxFactoryRecovery_comfirm, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
	lv_mbox_set_action(mboxFactoryRecovery_comfirm, funCb_mboxBtnActionClick_factoryRecovery);
	lv_mbox_set_anim_time(mboxFactoryRecovery_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_touchRecalibration(lv_obj_t *btn){

	mboxTouchRecalibration_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxTouchRecalibration_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxTouchRecalibration_comfirm, NULL, LV_ALIGN_CENTER, -55, -55);
	lv_obj_set_width(mboxTouchRecalibration_comfirm, 230);
	lv_mbox_set_text(mboxTouchRecalibration_comfirm, "touch recalibration opreat need to restart the devive, are you sure to continue?");
	lv_mbox_add_btns(mboxTouchRecalibration_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BG, &styleMbox_bg);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchRecalibration_comfirm, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchRecalibration_comfirm, funCb_mboxBtnActionClick_touchRecalibration);
	lv_mbox_set_anim_time(mboxTouchRecalibration_comfirm, 200);

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

static void lvGuiSettingSet_objStyle_Init(void){

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&stylePage_funSetOption, &lv_style_plain);
	stylePage_funSetOption.body.main_color = LV_COLOR_GRAY;
	stylePage_funSetOption.body.grad_color = LV_COLOR_GRAY;

	lv_style_copy(&styleText_menuLevel_B, &lv_style_plain);
	styleText_menuLevel_B.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleDdlistSettingA_devType, &lv_style_pretty);
	styleDdlistSettingA_devType.body.shadow.width = 1;
	styleDdlistSettingA_devType.text.color = LV_COLOR_MAKE(0x10, 0x20, 0x50);
	styleDdlistSettingA_devType.text.font = &lv_font_dejavu_15;

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

	lv_style_copy(&styleMbox_bg, &lv_style_pretty);
	styleMbox_bg.body.main_color = LV_COLOR_MAKE(0xf5, 0x45, 0x2e);
	styleMbox_bg.body.grad_color = LV_COLOR_MAKE(0xb9, 0x1d, 0x09);
	styleMbox_bg.body.border.color = LV_COLOR_MAKE(0x3f, 0x0a, 0x03);
	styleMbox_bg.text.font = &lv_font_consola_17;
	styleMbox_bg.text.color = LV_COLOR_WHITE;
	styleMbox_bg.body.padding.hor = 12;
	styleMbox_bg.body.padding.ver = 8;
	styleMbox_bg.body.shadow.width = 8;
	lv_style_copy(&styleMbox_btn_rel, &lv_style_btn_rel);
	styleMbox_btn_rel.text.font = &lv_font_consola_19;
	styleMbox_btn_rel.body.empty = 1;					 /*Draw only the border*/
	styleMbox_btn_rel.body.border.color = LV_COLOR_WHITE;
	styleMbox_btn_rel.body.border.width = 2;
	styleMbox_btn_rel.body.border.opa = LV_OPA_70;
	styleMbox_btn_rel.body.padding.hor = 12;
	styleMbox_btn_rel.body.padding.ver = 8;
	lv_style_copy(&styleMbox_btn_pr, &styleMbox_btn_rel);
	styleMbox_btn_pr.body.empty = 0;
	styleMbox_btn_pr.body.main_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);
	styleMbox_btn_pr.body.grad_color = LV_COLOR_MAKE(0x5d, 0x0f, 0x04);

    lv_style_copy(&styleBtn_specialTransparent, &lv_style_btn_rel);
	styleBtn_specialTransparent.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.border.part = LV_BORDER_NONE;
    styleBtn_specialTransparent.body.opa = LV_OPA_TRANSP;
	styleBtn_specialTransparent.body.radius = 0;
    styleBtn_specialTransparent.body.shadow.width =  0;
}

void lvGui_businessMenu_settingSet(lv_obj_t * obj_Parent){

	const uint16_t obj_animate_time = 200;
	const uint16_t obj_animate_delay = 150;
	uint16_t obj_animate_delayBasic = 0;

	uint8_t loop = 0;
	devTypeDef_enum devType_Temp;
	uint8_t homepageThemeStyle_temp = 0;

	lvGuiSettingSet_objStyle_Init();

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
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_SB, &stylePage_funSetOption);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_BG, &stylePage_funSetOption);
	lv_page_set_sb_mode(page_funSetOption, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(page_funSetOption, false, false); //key opration
	lv_page_set_scrl_width(page_funSetOption, 220);
	lv_page_set_scrl_height(page_funSetOption, 720);
	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);
//	lv_page_scroll_ver(page_funSetOption, 480);

	textSettingA_deviceType = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_deviceType, "device Type:");
	lv_obj_set_style(textSettingA_deviceType, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_deviceType, LV_PROTECT_POS);
	lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);
	
	ddlistSettingA_deviceType = lv_ddlist_create(page_funSetOption, NULL);
	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_SB, &styleDdlistSettingA_devType);
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

	swSettingA_devStatusRecordIF = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_BG, &bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_INDIC, &indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_ON, &knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_OFF, &knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devStatusRecordIF, LV_PROTECT_POS);
	lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 20);
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
	lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_BG, &styleDdlistSettingA_devType);
	lv_obj_set_protect(ddlistSettingA_homepageThemestyle, LV_PROTECT_POS);
	lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 20);
	lv_ddlist_set_options(ddlistSettingA_homepageThemestyle, homepageThemeStyle_listTab);
	lv_ddlist_set_action(ddlistSettingA_homepageThemestyle, funCb_ddlistHomepageThemeDef);
	lv_obj_set_top(ddlistSettingA_homepageThemestyle, true);
	lv_obj_set_free_num(ddlistSettingA_homepageThemestyle, OBJ_DDLIST_HPTHEME_FREENUM);
	homepageThemeStyle_temp = usrAppHomepageThemeType_Get();
	lv_ddlist_set_selected(ddlistSettingA_homepageThemestyle, homepageThemeStyle_temp);

	textSettingA_factoryRecoveryIf = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_factoryRecoveryIf, "factory recovery:");
	lv_obj_set_style(textSettingA_factoryRecoveryIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	lv_obj_align(textSettingA_factoryRecoveryIf, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	btnSettingA_factoryRecoveryIf = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_factoryRecoveryIf, 60, 20);
	lv_obj_set_protect(btnSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_factoryRecoveryIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_factoryRecovery);
	lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, 60, 20);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnRef_factoryRecoveryIf = lv_label_create(btnSettingA_factoryRecoveryIf, NULL);
	lv_label_set_recolor(textBtnRef_factoryRecoveryIf, true);
	lv_label_set_text(textBtnRef_factoryRecoveryIf, "#00FF40 >>>o<<<<#");
	lv_obj_set_style(textBtnRef_factoryRecoveryIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_factoryRecoveryIf, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_factoryRecoveryIf, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_touchRecalibrationIf = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_touchRecalibrationIf, "touch recalibration");
	lv_obj_set_style(textSettingA_touchRecalibrationIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_touchRecalibrationIf, LV_PROTECT_POS);
	lv_obj_align(textSettingA_touchRecalibrationIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	btnSettingA_touchRecalibrationIf = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_touchRecalibrationIf, 60, 20);
	lv_obj_set_protect(btnSettingA_touchRecalibrationIf, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_touchRecalibrationIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_touchRecalibration);
	lv_obj_align(btnSettingA_touchRecalibrationIf, textSettingA_touchRecalibrationIf, LV_ALIGN_OUT_BOTTOM_LEFT, 60, 20);
    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnRef_touchRecalibrationIf = lv_label_create(btnSettingA_touchRecalibrationIf, NULL);
	lv_label_set_recolor(textBtnRef_touchRecalibrationIf, true);
	lv_label_set_text(textBtnRef_touchRecalibrationIf, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_touchRecalibrationIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_touchRecalibrationIf, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_touchRecalibrationIf, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_screenBrightness = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenBrightness, "screen brightness");
	lv_obj_set_style(textSettingA_screenBrightness, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenBrightness, LV_PROTECT_POS);
	lv_obj_align(textSettingA_screenBrightness, textSettingA_touchRecalibrationIf, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	sliderSettingA_screenBrightness = lv_slider_create(page_funSetOption, NULL);
	lv_obj_set_size(sliderSettingA_screenBrightness, 180, 15);
	lv_obj_set_protect(sliderSettingA_screenBrightness, LV_PROTECT_POS);
	lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 25);
	lv_slider_set_action(sliderSettingA_screenBrightness, funCb_slidAction_functionSet_screenBrightnessAdj);
	lv_slider_set_range(sliderSettingA_screenBrightness, 0, screenBrightness_sliderAdj_div);
	uint8_t brightnessSlider_valDisp = devScreenDriver_configParam_brightness_get();
	lv_slider_set_value(sliderSettingA_screenBrightness, brightnessSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));

	textSettingA_screenLightTime = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenLightTime, "screen light time");
	lv_obj_set_style(textSettingA_screenLightTime, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textSettingA_screenLightTime, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	textBtnTimeRef_screenLightTime = lv_label_create(page_funSetOption, NULL);
	lv_label_set_recolor(textBtnTimeRef_screenLightTime, true);
	uint32_t screenLightTime_temp = devScreenDriver_configParam_screenLightTime_get();
	for(loop = 0; loop < FUNCTION_NUM_DEF_SCREENLIGHT_TIME; loop ++){

		if(screenLightTime_temp == screenLightTimeGear_refTab[loop].screenLightTime2Sec){

			functionGearScreenTime_ref = loop;
			break;
		}
	}
	lv_label_set_text(textBtnTimeRef_screenLightTime, screenLightTimeGear_refTab[functionGearScreenTime_ref].strDisp_ref);
	lv_obj_set_style(textBtnTimeRef_screenLightTime, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeRef_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textBtnTimeRef_screenLightTime, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_MID, 35, 25);
	btnTimeAdd_screenLightTime = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTimeAdd_screenLightTime, 40, 40);
	lv_obj_set_protect(btnTimeAdd_screenLightTime, LV_PROTECT_POS);
	lv_btn_set_action(btnTimeAdd_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeAdd);
	lv_obj_align(btnTimeAdd_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeAdd_screenLightTime, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnTimeAdd_screenLightTime = lv_label_create(btnTimeAdd_screenLightTime, NULL);
	lv_label_set_recolor(textBtnTimeAdd_screenLightTime, true);
	lv_label_set_text(textBtnTimeAdd_screenLightTime, "#00FF40 >>>>#");
	lv_obj_set_style(textBtnTimeAdd_screenLightTime, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeAdd_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textBtnTimeAdd_screenLightTime, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 2);
	btnTimeCut_screenLightTime = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTimeCut_screenLightTime, 40, 40);
	lv_obj_set_protect(btnTimeCut_screenLightTime, LV_PROTECT_POS);
	lv_btn_set_action(btnTimeCut_screenLightTime, LV_BTN_ACTION_CLICK, funCb_btnActionClick_functionSet_screenLightTimeCut);
	lv_obj_align(btnTimeCut_screenLightTime, textBtnTimeRef_screenLightTime, LV_ALIGN_OUT_LEFT_BOTTOM, -10, 0);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnTimeCut_screenLightTime, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnTimeCut_screenLightTime = lv_label_create(btnTimeCut_screenLightTime, NULL);
	lv_label_set_recolor(textBtnTimeCut_screenLightTime, true);
	lv_label_set_text(textBtnTimeCut_screenLightTime, "#00FF40 <<<<#");
	lv_obj_set_style(textBtnTimeCut_screenLightTime, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnTimeCut_screenLightTime, LV_PROTECT_POS);
	lv_obj_align(textBtnTimeCut_screenLightTime, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 2);

	lv_page_glue_obj(textSettingA_deviceType, true);
	lv_page_glue_obj(ddlistSettingA_deviceType, true);
	lv_page_glue_obj(textSettingA_homepageThemestyle, true);
	lv_page_glue_obj(ddlistSettingA_homepageThemestyle, true);
	lv_page_glue_obj(textSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(swSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(textSettingA_factoryRecoveryIf, true);
	lv_page_glue_obj(btnSettingA_factoryRecoveryIf, true);
	lv_page_glue_obj(textSettingA_touchRecalibrationIf, true);
	lv_page_glue_obj(btnSettingA_touchRecalibrationIf, true);
//	lv_page_glue_obj(sliderSettingA_screenBrightness, true);
	lv_page_glue_obj(textBtnTimeRef_screenLightTime, true);
	lv_page_glue_obj(btnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(btnTimeCut_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeCut_screenLightTime, true);

	lv_obj_animate(textSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);
	lv_obj_animate(ddlistSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(swSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(ddlistSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(textBtnTimeRef_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnTimeAdd_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);
	lv_obj_animate(btnTimeCut_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);

	vTaskDelay(500 / portTICK_RATE_MS);
	lv_page_focus(page_funSetOption, btnTimeCut_screenLightTime, 1200);
}



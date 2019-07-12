/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "mdf_common.h"

/* freertos includes */
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

#include "bussiness_timerHard.h"

#include "gui_businessMenu_delayerSet.h"
#include "gui_businessHome.h"

#include "gui_businessReuse_reactionObjPage.h"

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_19);
LV_FONT_DECLARE(lv_font_arial_12);
LV_FONT_DECLARE(lv_font_arial_15);
LV_FONT_DECLARE(lv_font_arial_18);
LV_FONT_DECLARE(lv_font_ariblk_18);
LV_FONT_DECLARE(lv_font_dejavu_20);

LV_FONT_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static lv_style_t styleTabview_gmdyTimeSet_bg;
static lv_style_t styleTabview_gmdyTimeSet_indic;
static lv_style_t styleTabview_gmdyTimeSet_btn_bg;
static lv_style_t styleTabview_gmdyTimeSet_btn_pr;
static lv_style_t styleTabview_gmdyTimeSet_btn_rel;
static lv_style_t styleTabview_gmdyTimeSet_btn_tgl_pr;
static lv_style_t styleTabview_gmdyTimeSet_btn_tgl_rel;

static lv_style_t styleRoller_gmdyTimeSet_bg;
static lv_style_t styleRoller_gmdyTimeSet_sel;
static lv_style_t styleLabel_gmdyTimeSet_rollerIns;
static lv_style_t styleLabel_gmdyTimeSet_title;
static lv_style_t styleLabel_gmdyTimeSet_funSave;
static lv_style_t styleBtn_gmdyTimeSet_funSave;
static lv_style_t styleText_menuLevel_A;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *objText_menuCurrentTitle = NULL;

static lv_obj_t *objCont_gmdyOpMenu = NULL;
static lv_obj_t *objTabv_gmdyOpMenu = NULL;
static lv_obj_t *objTab_delayTrigSet = NULL;
static lv_obj_t *objTab_greenModeSet = NULL;
static lv_obj_t *objBtn_delayTrigSet_save = NULL;
static lv_obj_t *objBtn_greenModeSet_save = NULL;
static lv_obj_t *objLabel_delayTrigSet_save = NULL;
static lv_obj_t *objLabel_greenModeSet_save = NULL;

static lv_obj_t *objLine_delayTrigSet_limit_A = NULL;
static lv_obj_t *objLine_delayTrigSet_limit_B = NULL;
static lv_obj_t *objLine_greenModeSet_limit_A = NULL;
static lv_obj_t *objLine_greenModeSet_limit_B = NULL;

static lv_obj_t *objLabel_delayTrigSet_titleTimeSet = NULL;
static lv_obj_t *objLabel_delayTrigSet_titleSwValTrig = NULL;
static lv_obj_t *objLabel_greenModeSet_titleTimeSet = NULL;
static lv_obj_t *objLabel_greenModeSet_titleSwValTrig = NULL;

static lv_obj_t *objLabel_delayTrigSet_swTrigSet[3] = {NULL};
static lv_obj_t *objSw_delayTrigSet_swTrigSet[3] = {NULL};
static lv_obj_t *objSw_greenModeSet_swRunnigSet = NULL;

static lv_obj_t *objRoller_delayTrigSet_hour = NULL;
static lv_obj_t *objRoller_delayTrigSet_minute = NULL;
static lv_obj_t *objLabel_delayTrigSet_hour = NULL;
static lv_obj_t *objLabel_delayTrigSet_minute = NULL;

static lv_obj_t *objRoller_greenModeSet_hour = NULL;
static lv_obj_t *objRoller_greenModeSet_minute = NULL;
static lv_obj_t *objRoller_greenModeSet_second = NULL;
static lv_obj_t *objLabel_greenModeSet_hour = NULL;
static lv_obj_t *objLabel_greenModeSet_minute = NULL;
static lv_obj_t *objLabel_greenModeSet_second = NULL;

static uint8_t paramSetTemp_delayTrig[3] = {0}; //合并缓存
static struct sttParam_delayTrigTimeSet{ //散装缓存

	uint8_t hour;
	uint8_t minute;
}param_delayTrigTimeSet = {0};
static stt_devDataPonitTypedef param_delayTrigSwTrigSet = {0};

static uint8_t paramSetTemp_greenMode[2] = {0}; //合并缓存
static struct sttParam_greenModeTimeSet{ //散装缓存

	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}param_greenModeTimeSet = {0};

static void currentGui_elementClear(void){

	memset(paramSetTemp_delayTrig, 0, sizeof(uint8_t) * 3);
	memset(paramSetTemp_greenMode, 0, sizeof(uint8_t) * 2);
	memset(&param_delayTrigSwTrigSet, 0, sizeof(stt_devDataPonitTypedef));
	
	lv_obj_del(objTabv_gmdyOpMenu);
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

static lv_res_t funCb_btnActionClick_tabDelayTrigSet_funSave(lv_obj_t *btn){

	uint16_t delayTrig_timeCount = (param_delayTrigTimeSet.hour * 3600) + 
								   (param_delayTrigTimeSet.minute * 60);	
	stt_devDataPonitTypedef datapointParamSet_temp = {0};

	lvGui_businessReuse_reactionObjPageElement_funValConfig_get(PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST, &datapointParamSet_temp);
	memcpy(&param_delayTrigSwTrigSet, &datapointParamSet_temp, sizeof(stt_devDataPonitTypedef));

	paramSetTemp_delayTrig[0] = (uint8_t)(delayTrig_timeCount >> 8);
	paramSetTemp_delayTrig[1] = (uint8_t)(delayTrig_timeCount & 0x00FF);
	memcpy(&paramSetTemp_delayTrig[2], &param_delayTrigSwTrigSet, sizeof(stt_devDataPonitTypedef));

	usrAppParamSet_devDelayTrig(paramSetTemp_delayTrig);

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_delayTrigSetUnitSetHour(lv_obj_t *rol){

	param_delayTrigTimeSet.hour = lv_roller_get_selected(rol);
	
	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_delayTrigSetUnitSetMinute(lv_obj_t *rol){

	param_delayTrigTimeSet.minute = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

//static lv_res_t funCb_swOp_delayTrigSet_trigValSet_A(lv_obj_t *sw){

//	(lv_sw_get_state(sw))?
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit1 = 0):
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit1 = 1);

//	return LV_RES_OK;
//}

//static lv_res_t funCb_swOp_delayTrigSet_trigValSet_B(lv_obj_t *sw){

//	(lv_sw_get_state(sw))?
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit2 = 0):
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit2 = 1);

//	return LV_RES_OK;
//}

//static lv_res_t funCb_swOp_delayTrigSet_trigValSet_C(lv_obj_t *sw){

//	(lv_sw_get_state(sw))?
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit3 = 0):
//		(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit3 = 1);

//	return LV_RES_OK;
//}

static lv_res_t funCb_btnActionClick_tabGreenModeSet_funSave(lv_obj_t *btn){

	uint16_t greenMode_timeCount = (param_greenModeTimeSet.hour * 3600) +
								   (param_greenModeTimeSet.minute * 60) + 
								   (param_greenModeTimeSet.second * 1);

	paramSetTemp_greenMode[0] = (uint8_t)(greenMode_timeCount >> 8);
	paramSetTemp_greenMode[1] = (uint8_t)(greenMode_timeCount & 0x00FF);						   

	usrAppParamSet_devGreenMode(paramSetTemp_greenMode, true);
	
	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_greenModeSet_UnitSetHour(lv_obj_t *rol){

	uint16_t greenMode_timeCount = (param_greenModeTimeSet.hour * 3600) +
								   (param_greenModeTimeSet.minute * 60) + 
								   (param_greenModeTimeSet.second * 1);

	if(!greenMode_timeCount){

		lv_sw_off(objSw_greenModeSet_swRunnigSet);
		
	}else{

		lv_sw_on(objSw_greenModeSet_swRunnigSet);
	}

	param_greenModeTimeSet.hour = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_greenModeSet_UnitSetMinute(lv_obj_t *rol){

	uint16_t greenMode_timeCount = (param_greenModeTimeSet.hour * 3600) +
								   (param_greenModeTimeSet.minute * 60) + 
								   (param_greenModeTimeSet.second * 1);

	if(!greenMode_timeCount){

		lv_sw_off(objSw_greenModeSet_swRunnigSet);
		
	}else{

		lv_sw_on(objSw_greenModeSet_swRunnigSet);
	}

	param_greenModeTimeSet.minute = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_greenModeSet_UnitSetSecond(lv_obj_t *rol){

	uint16_t greenMode_timeCount = (param_greenModeTimeSet.hour * 3600) +
								   (param_greenModeTimeSet.minute * 60) + 
								   (param_greenModeTimeSet.second * 1);

	if(!greenMode_timeCount){

		lv_sw_off(objSw_greenModeSet_swRunnigSet);
		
	}else{

		lv_sw_on(objSw_greenModeSet_swRunnigSet);
	}

	param_greenModeTimeSet.second = lv_roller_get_selected(rol);

	return LV_RES_OK;
}

static lv_res_t funCb_swOp_greenModeSet_runSetUp(lv_obj_t *sw){

	param_greenModeTimeSet.hour =\
	param_greenModeTimeSet.minute =\ 
	param_greenModeTimeSet.second = 0;

	if(lv_sw_get_state(sw))
		param_greenModeTimeSet.minute = 1;

	lv_roller_set_selected(objRoller_greenModeSet_hour, param_greenModeTimeSet.hour, false);	
	lv_roller_set_selected(objRoller_greenModeSet_minute, param_greenModeTimeSet.minute, false);	
	lv_roller_set_selected(objRoller_greenModeSet_second, param_greenModeTimeSet.second, false);	

	return LV_RES_OK;
}

void lvGui_businessMenu_delayerSet(lv_obj_t * obj_Parent){

	uint16_t delayTrig_timeCount = 0;
	uint16_t greenMode_timeCount = 0;

	usrAppParamGet_devDelayTrig(paramSetTemp_delayTrig);
	delayTrig_timeCount = ((uint8_t)paramSetTemp_delayTrig[0] << 8) | ((uint8_t)paramSetTemp_delayTrig[1]);
	memcpy(&param_delayTrigSwTrigSet, &paramSetTemp_delayTrig[2], sizeof(stt_devDataPonitTypedef));
	param_delayTrigTimeSet.hour = (delayTrig_timeCount / 3600) % 24;
	param_delayTrigTimeSet.minute = (delayTrig_timeCount / 60) % 60;
	
	usrAppParamGet_devGreenMode(paramSetTemp_greenMode);
	greenMode_timeCount = ((uint8_t)paramSetTemp_greenMode[0] << 8) | ((uint8_t)paramSetTemp_greenMode[1]);
	param_greenModeTimeSet.hour = (greenMode_timeCount / 3600) % 24;
	param_greenModeTimeSet.minute = (greenMode_timeCount / 60) % 60;
	param_greenModeTimeSet.second = greenMode_timeCount % 60;

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleTabview_gmdyTimeSet_bg, &lv_style_plain);
	styleTabview_gmdyTimeSet_bg.body.main_color = LV_COLOR_WHITE;
	styleTabview_gmdyTimeSet_bg.body.grad_color = LV_COLOR_WHITE;
	lv_style_copy(&styleTabview_gmdyTimeSet_indic, &lv_style_plain_color);
	styleTabview_gmdyTimeSet_indic.text.font = &lv_font_consola_16;
	lv_style_copy(&styleTabview_gmdyTimeSet_btn_bg, &lv_style_transp);
	styleTabview_gmdyTimeSet_btn_bg.text.font = &lv_font_consola_16;
	lv_style_copy(&styleTabview_gmdyTimeSet_btn_pr, &lv_style_btn_rel);
	styleTabview_gmdyTimeSet_btn_pr.text.font = &lv_font_consola_16;
	lv_style_copy(&styleTabview_gmdyTimeSet_btn_rel, &lv_style_btn_rel);
	styleTabview_gmdyTimeSet_btn_rel.text.font = &lv_font_consola_16;
	lv_style_copy(&styleTabview_gmdyTimeSet_btn_tgl_pr, &lv_style_btn_tgl_pr);
	styleTabview_gmdyTimeSet_btn_tgl_pr.text.font = &lv_font_consola_16;
	lv_style_copy(&styleTabview_gmdyTimeSet_btn_tgl_rel, &lv_style_btn_rel);
	styleTabview_gmdyTimeSet_btn_tgl_rel.text.font = &lv_font_consola_16;

	lv_style_copy(&styleRoller_gmdyTimeSet_bg, &lv_style_plain);
	styleRoller_gmdyTimeSet_bg.body.main_color = LV_COLOR_WHITE;
	styleRoller_gmdyTimeSet_bg.body.grad_color = LV_COLOR_WHITE;
	styleRoller_gmdyTimeSet_bg.text.font = &lv_font_consola_19;
	styleRoller_gmdyTimeSet_bg.text.line_space = 5;
	styleRoller_gmdyTimeSet_bg.text.opa = LV_OPA_40;
	lv_style_copy(&styleRoller_gmdyTimeSet_sel, &lv_style_plain);
	styleRoller_gmdyTimeSet_sel.body.empty = 1;
	styleRoller_gmdyTimeSet_sel.body.radius = 30;
	styleRoller_gmdyTimeSet_sel.text.color = LV_COLOR_BLACK;
	styleRoller_gmdyTimeSet_sel.text.font = &lv_font_consola_19;

	lv_style_copy(&styleLabel_gmdyTimeSet_rollerIns, &lv_style_plain);
	styleLabel_gmdyTimeSet_rollerIns.text.font = &lv_font_arial_15;
	styleLabel_gmdyTimeSet_rollerIns.text.color = LV_COLOR_MAKE(80, 240, 80);

	lv_style_copy(&styleLabel_gmdyTimeSet_title, &lv_style_plain);
	styleLabel_gmdyTimeSet_title.text.font = &lv_font_consola_16;
	styleLabel_gmdyTimeSet_title.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleLabel_gmdyTimeSet_funSave, &lv_style_plain);
	styleLabel_gmdyTimeSet_funSave.text.font = &lv_font_consola_19;
	styleLabel_gmdyTimeSet_funSave.text.color = LV_COLOR_BLUE;

	lv_style_copy(&styleBtn_gmdyTimeSet_funSave, &lv_style_btn_rel);
	styleBtn_gmdyTimeSet_funSave.body.main_color = LV_COLOR_TRANSP;
	styleBtn_gmdyTimeSet_funSave.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_gmdyTimeSet_funSave.body.border.part = LV_BORDER_NONE;
	styleBtn_gmdyTimeSet_funSave.body.opa = LV_OPA_TRANSP;
	styleBtn_gmdyTimeSet_funSave.body.radius = 0;
	styleBtn_gmdyTimeSet_funSave.body.shadow.width = 0;

	objText_menuCurrentTitle = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(objText_menuCurrentTitle, "delay set");
	lv_obj_set_pos(objText_menuCurrentTitle, 40, 45);
	lv_obj_set_style(objText_menuCurrentTitle, &styleText_menuLevel_A);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack_arrowLeft);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack_arrowLeft);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 45);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
	
	objTabv_gmdyOpMenu = lv_tabview_create(lv_scr_act(), NULL);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BG, &styleTabview_gmdyTimeSet_bg);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_INDIC, &styleTabview_gmdyTimeSet_indic);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BTN_BG, &styleTabview_gmdyTimeSet_btn_bg);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BTN_REL, &styleTabview_gmdyTimeSet_btn_rel);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BTN_PR, &styleTabview_gmdyTimeSet_btn_pr);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BTN_TGL_REL, &styleTabview_gmdyTimeSet_btn_tgl_rel);
	lv_tabview_set_style(objTabv_gmdyOpMenu, LV_TABVIEW_STYLE_BTN_TGL_PR, &styleTabview_gmdyTimeSet_btn_tgl_pr);
    lv_obj_set_size(objTabv_gmdyOpMenu, 240, 245);
	lv_obj_set_pos(objTabv_gmdyOpMenu, 0, 75);
    objTab_delayTrigSet = lv_tabview_add_tab(objTabv_gmdyOpMenu, "delay");
    objTab_greenModeSet = lv_tabview_add_tab(objTabv_gmdyOpMenu, "greenMode");
    objBtn_delayTrigSet_save = lv_btn_create(objTab_delayTrigSet, NULL);
	lv_btn_set_action(objBtn_delayTrigSet_save, LV_BTN_ACTION_CLICK, funCb_btnActionClick_tabDelayTrigSet_funSave);
	lv_obj_set_size(objBtn_delayTrigSet_save, 60, 25);
	lv_page_glue_obj(objBtn_delayTrigSet_save, true);
	lv_btn_set_style(objBtn_delayTrigSet_save, LV_BTN_STYLE_REL, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_delayTrigSet_save, LV_BTN_STYLE_PR, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_delayTrigSet_save, LV_BTN_STYLE_TGL_REL, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_delayTrigSet_save, LV_BTN_STYLE_TGL_PR, &styleBtn_gmdyTimeSet_funSave);
	lv_obj_set_protect(objBtn_delayTrigSet_save, LV_PROTECT_POS);
	lv_obj_set_pos(objBtn_delayTrigSet_save, 150, 30);
	objLabel_delayTrigSet_save = lv_label_create(objTab_delayTrigSet, NULL);
	lv_label_set_text(objLabel_delayTrigSet_save, "save");
	lv_obj_set_protect(objLabel_delayTrigSet_save, LV_PROTECT_POS);
	lv_obj_align(objLabel_delayTrigSet_save, objBtn_delayTrigSet_save, LV_ALIGN_IN_RIGHT_MID, 0, 0);
	lv_obj_set_style(objLabel_delayTrigSet_save, &styleLabel_gmdyTimeSet_funSave);

	objLine_delayTrigSet_limit_A = lv_obj_create(objTab_delayTrigSet, NULL);
	lv_obj_set_size(objLine_delayTrigSet_limit_A , 226, 1);
	lv_obj_set_protect(objLine_delayTrigSet_limit_A, LV_PROTECT_POS);
	lv_obj_align(objLine_delayTrigSet_limit_A, objBtn_delayTrigSet_save, LV_ALIGN_OUT_BOTTOM_MID, -70, 5);
	objLine_delayTrigSet_limit_B = lv_obj_create(objTab_delayTrigSet, objLine_delayTrigSet_limit_A);
	lv_obj_set_protect(objLine_delayTrigSet_limit_B, LV_PROTECT_POS);
	lv_obj_align(objLine_delayTrigSet_limit_B, objLine_delayTrigSet_limit_A, LV_ALIGN_CENTER, 0, 160);

	objLabel_delayTrigSet_titleTimeSet = lv_label_create(objTab_delayTrigSet, NULL);
	lv_label_set_text(objLabel_delayTrigSet_titleTimeSet, "time set:");
	lv_obj_set_protect(objLabel_delayTrigSet_titleTimeSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_delayTrigSet_titleTimeSet, objLine_delayTrigSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 3, 2);
	lv_obj_set_style(objLabel_delayTrigSet_titleTimeSet, &styleLabel_gmdyTimeSet_title);
	objLabel_delayTrigSet_titleSwValTrig = lv_label_create(objTab_delayTrigSet, NULL);
	lv_label_set_text(objLabel_delayTrigSet_titleSwValTrig, "trig set:");
	lv_obj_set_protect(objLabel_delayTrigSet_titleSwValTrig, LV_PROTECT_POS);
	lv_obj_align(objLabel_delayTrigSet_titleSwValTrig, objLine_delayTrigSet_limit_B, LV_ALIGN_OUT_BOTTOM_LEFT, 3, 2);
	lv_obj_set_style(objLabel_delayTrigSet_titleSwValTrig, &styleLabel_gmdyTimeSet_title);

	objRoller_delayTrigSet_hour = lv_roller_create(objTab_delayTrigSet, NULL);
	lv_roller_set_action(objRoller_delayTrigSet_hour, funCb_rollerOpreat_delayTrigSetUnitSetHour);
	lv_roller_set_options(objRoller_delayTrigSet_hour, "00\n""01\n""02\n""03\n""04");
	lv_obj_set_protect(objRoller_delayTrigSet_hour, LV_PROTECT_POS);
	lv_obj_align(objRoller_delayTrigSet_hour, objLine_delayTrigSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 30);
	lv_roller_set_hor_fit(objRoller_delayTrigSet_hour, false);
	lv_obj_set_width(objRoller_delayTrigSet_hour, 40);
	lv_obj_set_top(objRoller_delayTrigSet_hour, true);
	lv_roller_set_selected(objRoller_delayTrigSet_hour, param_delayTrigTimeSet.hour, false);
	lv_roller_set_style(objRoller_delayTrigSet_hour, LV_ROLLER_STYLE_BG, &styleRoller_gmdyTimeSet_bg);
	lv_roller_set_style(objRoller_delayTrigSet_hour, LV_ROLLER_STYLE_SEL, &styleRoller_gmdyTimeSet_sel);

	objRoller_delayTrigSet_minute = lv_roller_create(objTab_delayTrigSet, NULL);
	lv_roller_set_action(objRoller_delayTrigSet_minute,  funCb_rollerOpreat_delayTrigSetUnitSetMinute);
	lv_roller_set_options(objRoller_delayTrigSet_minute, "00\n""01\n""02\n""03\n""04\n""05\n"
														 "06\n""07\n""08\n""09\n""10\n""11\n"
														 "12\n""13\n""14\n""15\n""16\n""17\n"
														 "18\n""19\n""20\n""21\n""22\n""23\n"
														 "24\n""25\n""26\n""27\n""28\n""29\n"
														 "30\n""31\n""32\n""33\n""34\n""35\n"
														 "36\n""37\n""38\n""39\n""40\n""41\n"
														 "42\n""43\n""44\n""45\n""46\n""47\n"
														 "48\n""49\n""50\n""51\n""52\n""53\n"
														 "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(objRoller_delayTrigSet_minute, LV_PROTECT_POS);
	lv_obj_align(objRoller_delayTrigSet_minute, objRoller_delayTrigSet_hour, LV_ALIGN_OUT_RIGHT_MID, 45, 0);
	lv_roller_set_hor_fit(objRoller_delayTrigSet_minute, false);
	lv_obj_set_width(objRoller_delayTrigSet_minute, 40);
	lv_obj_set_top(objRoller_delayTrigSet_minute, true);
	lv_roller_set_selected(objRoller_delayTrigSet_minute, param_delayTrigTimeSet.minute, false);
	lv_roller_set_style(objRoller_delayTrigSet_minute, LV_ROLLER_STYLE_BG, &styleRoller_gmdyTimeSet_bg);
	lv_roller_set_style(objRoller_delayTrigSet_minute, LV_ROLLER_STYLE_SEL, &styleRoller_gmdyTimeSet_sel);

	objLabel_delayTrigSet_hour = lv_label_create(objTab_delayTrigSet, NULL);
	lv_label_set_text(objLabel_delayTrigSet_hour, "hour");
	lv_obj_set_protect(objLabel_delayTrigSet_hour, LV_PROTECT_POS);
	lv_obj_align(objLabel_delayTrigSet_hour, objRoller_delayTrigSet_hour, LV_ALIGN_IN_RIGHT_MID, 40, 3);
	lv_obj_set_style(objLabel_delayTrigSet_hour, &styleLabel_gmdyTimeSet_rollerIns);

	objLabel_delayTrigSet_minute = lv_label_create(objTab_delayTrigSet, objLabel_delayTrigSet_hour);
	lv_label_set_text(objLabel_delayTrigSet_minute, "minute");
	lv_obj_set_protect(objLabel_delayTrigSet_minute, LV_PROTECT_POS);
	lv_obj_align(objLabel_delayTrigSet_minute, objRoller_delayTrigSet_minute, LV_ALIGN_IN_RIGHT_MID, 40, 0);
	lv_obj_set_style(objLabel_delayTrigSet_minute, &styleLabel_gmdyTimeSet_rollerIns);

//	for(uint8_t loop = 0; loop < 3; loop ++){

//		objLabel_delayTrigSet_swTrigSet[loop] = lv_label_create(objTab_delayTrigSet, NULL);
//		lv_obj_set_style(objLabel_delayTrigSet_swTrigSet[loop], &styleLabel_gmdyTimeSet_title);
//	}
//	lv_label_set_text(objLabel_delayTrigSet_swTrigSet[0], "switch-A:");
//	lv_label_set_text(objLabel_delayTrigSet_swTrigSet[1], "switch-B:");
//	lv_label_set_text(objLabel_delayTrigSet_swTrigSet[2], "switch-C:");
//	lv_obj_align(objLabel_delayTrigSet_swTrigSet[0], objLine_delayTrigSet_limit_B, LV_ALIGN_OUT_BOTTOM_LEFT, 25, 40);
//	lv_obj_align(objLabel_delayTrigSet_swTrigSet[1], objLabel_delayTrigSet_swTrigSet[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
//	lv_obj_align(objLabel_delayTrigSet_swTrigSet[2], objLabel_delayTrigSet_swTrigSet[1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

//	for(uint8_t loop = 0; loop < 3; loop ++){

//		objSw_delayTrigSet_swTrigSet[loop] = lv_sw_create(objTab_delayTrigSet, NULL);
//		lv_obj_set_size(objSw_delayTrigSet_swTrigSet[loop] , 40, 20);
//		lv_obj_align(objSw_delayTrigSet_swTrigSet[loop], objLabel_delayTrigSet_swTrigSet[loop], LV_ALIGN_OUT_RIGHT_MID, 50, 0);
//	}
//	
//	lv_sw_set_action(objSw_delayTrigSet_swTrigSet[0], funCb_swOp_delayTrigSet_trigValSet_A);
//	lv_sw_set_action(objSw_delayTrigSet_swTrigSet[1], funCb_swOp_delayTrigSet_trigValSet_B);
//	lv_sw_set_action(objSw_delayTrigSet_swTrigSet[2], funCb_swOp_delayTrigSet_trigValSet_C);
//	(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit1)?
//		(lv_sw_off(objSw_delayTrigSet_swTrigSet[0])):
//		(lv_sw_on(objSw_delayTrigSet_swTrigSet[0]));
//	(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit2)?
//		(lv_sw_off(objSw_delayTrigSet_swTrigSet[1])):
//		(lv_sw_on(objSw_delayTrigSet_swTrigSet[1]));
//	(param_delayTrigSwTrigSet.devType_mulitSwitch_threeBit.swVal_bit3)?
//		(lv_sw_off(objSw_delayTrigSet_swTrigSet[2])):
//		(lv_sw_on(objSw_delayTrigSet_swTrigSet[2]));
		
	lvGui_businessReuse_reactionObjPageElement_creat(objTab_delayTrigSet, 
													 PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST,
													 lv_obj_get_y(objLine_delayTrigSet_limit_B) + 35,
													 &param_delayTrigSwTrigSet);

    objBtn_greenModeSet_save = lv_btn_create(objTab_greenModeSet, NULL);
	lv_btn_set_action(objBtn_greenModeSet_save, LV_BTN_ACTION_CLICK, funCb_btnActionClick_tabGreenModeSet_funSave);
	lv_obj_set_size(objBtn_greenModeSet_save, 60, 25);
	lv_page_glue_obj(objBtn_greenModeSet_save, true);
	lv_btn_set_style(objBtn_greenModeSet_save, LV_BTN_STYLE_REL, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_greenModeSet_save, LV_BTN_STYLE_PR, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_greenModeSet_save, LV_BTN_STYLE_TGL_REL, &styleBtn_gmdyTimeSet_funSave);
	lv_btn_set_style(objBtn_greenModeSet_save, LV_BTN_STYLE_TGL_PR, &styleBtn_gmdyTimeSet_funSave);
	lv_obj_set_protect(objBtn_greenModeSet_save, LV_PROTECT_POS);
	lv_obj_set_pos(objBtn_greenModeSet_save, 150, 30);
	objLabel_greenModeSet_save = lv_label_create(objTab_greenModeSet, NULL);
	lv_label_set_text(objLabel_greenModeSet_save, "save");
	lv_obj_set_protect(objLabel_greenModeSet_save, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_save, objBtn_greenModeSet_save, LV_ALIGN_IN_RIGHT_MID, 0, 0);
	lv_obj_set_style(objLabel_greenModeSet_save, &styleLabel_gmdyTimeSet_funSave);

	objLine_greenModeSet_limit_A = lv_obj_create(objTab_greenModeSet, NULL);
	lv_obj_set_size(objLine_greenModeSet_limit_A , 226, 1);
	lv_obj_set_protect(objLine_greenModeSet_limit_A, LV_PROTECT_POS);
//	lv_obj_set_pos(objLine_greenModeSet_limit_A, 7, 30);
	lv_obj_align(objLine_greenModeSet_limit_A, objBtn_greenModeSet_save, LV_ALIGN_OUT_BOTTOM_MID, -70, 5);
	objLine_greenModeSet_limit_B = lv_obj_create(objTab_greenModeSet, objLine_greenModeSet_limit_A);
	lv_obj_set_protect(objLine_greenModeSet_limit_B, LV_PROTECT_POS);
	lv_obj_align(objLine_greenModeSet_limit_B, objLine_greenModeSet_limit_A, LV_ALIGN_CENTER, 0, 160);

	objLabel_greenModeSet_titleTimeSet = lv_label_create(objTab_greenModeSet, NULL);
	lv_label_set_text(objLabel_greenModeSet_titleTimeSet, "time set:");
	lv_obj_set_protect(objLabel_greenModeSet_titleTimeSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_titleTimeSet, objLine_greenModeSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 3, 2);
	lv_obj_set_style(objLabel_greenModeSet_titleTimeSet, &styleLabel_gmdyTimeSet_title);
	objLabel_greenModeSet_titleSwValTrig = lv_label_create(objTab_greenModeSet, NULL);
	lv_label_set_text(objLabel_greenModeSet_titleSwValTrig, "run setup:");
	lv_obj_set_protect(objLabel_greenModeSet_titleSwValTrig, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_titleSwValTrig, objLine_greenModeSet_limit_B, LV_ALIGN_OUT_BOTTOM_LEFT, 3, 2);
	lv_obj_set_style(objLabel_greenModeSet_titleSwValTrig, &styleLabel_gmdyTimeSet_title);

	objRoller_greenModeSet_hour = lv_roller_create(objTab_greenModeSet, NULL);
	lv_roller_set_action(objRoller_greenModeSet_hour, funCb_rollerOpreat_greenModeSet_UnitSetHour);
	lv_roller_set_options(objRoller_greenModeSet_hour, "00\n""01\n""02\n""03\n""04\n""05\n"
													   "06\n""07\n""08\n""09\n""10\n""11\n"
													   "12\n""13\n""14\n""15\n""16\n""17\n"
													   "18");
	lv_obj_set_protect(objRoller_greenModeSet_hour, LV_PROTECT_POS);
	lv_obj_align(objRoller_greenModeSet_hour, objLine_greenModeSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 30);
	lv_roller_set_hor_fit(objRoller_greenModeSet_hour, false);
	lv_obj_set_width(objRoller_greenModeSet_hour, 40);
//	lv_obj_set_top(objRoller_greenModeSet_hour, 35);
	lv_roller_set_selected(objRoller_greenModeSet_hour, param_greenModeTimeSet.hour, false);
	lv_roller_set_style(objRoller_greenModeSet_hour, LV_ROLLER_STYLE_BG, &styleRoller_gmdyTimeSet_bg);
	lv_roller_set_style(objRoller_greenModeSet_hour, LV_ROLLER_STYLE_SEL, &styleRoller_gmdyTimeSet_sel);

	objRoller_greenModeSet_minute = lv_roller_create(objTab_greenModeSet, NULL);
	lv_roller_set_action(objRoller_greenModeSet_minute, funCb_rollerOpreat_greenModeSet_UnitSetMinute);
	lv_roller_set_options(objRoller_greenModeSet_minute, "00\n""01\n""02\n""03\n""04\n""05\n"
														 "06\n""07\n""08\n""09\n""10\n""11\n"
														 "12\n""13\n""14\n""15\n""16\n""17\n"
														 "18\n""19\n""20\n""21\n""22\n""23\n"
														 "24\n""25\n""26\n""27\n""28\n""29\n"
														 "30\n""31\n""32\n""33\n""34\n""35\n"
														 "36\n""37\n""38\n""39\n""40\n""41\n"
														 "42\n""43\n""44\n""45\n""46\n""47\n"
														 "48\n""49\n""50\n""51\n""52\n""53\n"
														 "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(objRoller_greenModeSet_minute, LV_PROTECT_POS);
	lv_obj_align(objRoller_greenModeSet_minute, objRoller_greenModeSet_hour, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_roller_set_hor_fit(objRoller_greenModeSet_minute, false);
	lv_obj_set_width(objRoller_greenModeSet_minute, 40);
//	lv_obj_set_top(objRoller_greenModeSet_minute, true);
	lv_roller_set_selected(objRoller_greenModeSet_minute, param_greenModeTimeSet.minute, false);
	lv_roller_set_style(objRoller_greenModeSet_minute, LV_ROLLER_STYLE_BG, &styleRoller_gmdyTimeSet_bg);
	lv_roller_set_style(objRoller_greenModeSet_minute, LV_ROLLER_STYLE_SEL, &styleRoller_gmdyTimeSet_sel);

	objRoller_greenModeSet_second = lv_roller_create(objTab_greenModeSet, NULL);
	lv_roller_set_action(objRoller_greenModeSet_second, funCb_rollerOpreat_greenModeSet_UnitSetSecond);
	lv_roller_set_options(objRoller_greenModeSet_second, "00\n""01\n""02\n""03\n""04\n""05\n"
														 "06\n""07\n""08\n""09\n""10\n""11\n"
														 "12\n""13\n""14\n""15\n""16\n""17\n"
														 "18\n""19\n""20\n""21\n""22\n""23\n"
														 "24\n""25\n""26\n""27\n""28\n""29\n"
														 "30\n""31\n""32\n""33\n""34\n""35\n"
														 "36\n""37\n""38\n""39\n""40\n""41\n"
														 "42\n""43\n""44\n""45\n""46\n""47\n"
														 "48\n""49\n""50\n""51\n""52\n""53\n"
														 "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(objRoller_greenModeSet_second, LV_PROTECT_POS);
	lv_obj_align(objRoller_greenModeSet_second, objRoller_greenModeSet_minute, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
	lv_roller_set_hor_fit(objRoller_greenModeSet_second, false);
	lv_obj_set_width(objRoller_greenModeSet_second, 40);
//	lv_obj_set_top(objRoller_greenModeSet_second, true);
	lv_roller_set_selected(objRoller_greenModeSet_second, param_greenModeTimeSet.second, false);
	lv_roller_set_style(objRoller_greenModeSet_second, LV_ROLLER_STYLE_BG, &styleRoller_gmdyTimeSet_bg);
	lv_roller_set_style(objRoller_greenModeSet_second, LV_ROLLER_STYLE_SEL, &styleRoller_gmdyTimeSet_sel);

	objLabel_greenModeSet_hour = lv_label_create(objTab_greenModeSet, NULL);
	lv_label_set_text(objLabel_greenModeSet_hour, "hour");
	lv_obj_set_protect(objLabel_greenModeSet_hour, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_hour, objRoller_greenModeSet_hour, LV_ALIGN_IN_RIGHT_MID, 35, 3);
	lv_obj_set_style(objLabel_greenModeSet_hour, &styleLabel_gmdyTimeSet_rollerIns);

	objLabel_greenModeSet_minute = lv_label_create(objTab_greenModeSet, objLabel_greenModeSet_hour);
	lv_label_set_text(objLabel_greenModeSet_minute, "min");
	lv_obj_set_protect(objLabel_greenModeSet_minute, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_minute, objRoller_greenModeSet_minute, LV_ALIGN_IN_RIGHT_MID, 15, 0);
	lv_obj_set_style(objLabel_greenModeSet_minute, &styleLabel_gmdyTimeSet_rollerIns);

	objLabel_greenModeSet_second = lv_label_create(objTab_greenModeSet, objLabel_greenModeSet_hour);
	lv_label_set_text(objLabel_greenModeSet_second, "sec");
	lv_obj_set_protect(objLabel_greenModeSet_second, LV_PROTECT_POS);
	lv_obj_align(objLabel_greenModeSet_second, objRoller_greenModeSet_second, LV_ALIGN_IN_RIGHT_MID, 15, 0);
	lv_obj_set_style(objLabel_greenModeSet_second, &styleLabel_gmdyTimeSet_rollerIns);

	objSw_greenModeSet_swRunnigSet = lv_sw_create(objTab_greenModeSet, NULL);
	lv_sw_set_action(objSw_greenModeSet_swRunnigSet, funCb_swOp_greenModeSet_runSetUp);
	lv_obj_set_size(objSw_greenModeSet_swRunnigSet, 40, 20);
	lv_obj_align(objSw_greenModeSet_swRunnigSet, objLabel_greenModeSet_titleSwValTrig, LV_ALIGN_IN_BOTTOM_RIGHT, 45, 40);
	(greenMode_timeCount)?
		(lv_sw_on(objSw_greenModeSet_swRunnigSet)):
		(lv_sw_off(objSw_greenModeSet_swRunnigSet));

	vTaskDelay(50 / portTICK_RATE_MS);
	lv_obj_refresh_style(objTabv_gmdyOpMenu);
}



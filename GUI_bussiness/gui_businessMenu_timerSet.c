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

#include "bussiness_timerSoft.h"

#include "gui_businessMenu_timerSet.h"
#include "gui_businessHome.h"

#include "gui_businessReuse_reactionObjPage.h"

#define LV_OBJ_FREENUM_BASE_BTN_TIMESETPAGEA	0x10
#define LV_OBJ_FREENUM_BASE_SW_TIMESETPAGEA		0x20
#define LV_OBJ_FREENUM_BASE_TIMERSETINFO_NUM	0x30
#define LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW		0x40
#define LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB		0x50

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_19);
LV_FONT_DECLARE(lv_font_arial_12);
LV_FONT_DECLARE(lv_font_arial_15);
LV_FONT_DECLARE(lv_font_arial_18);
LV_FONT_DECLARE(lv_font_ariblk_18);
LV_FONT_DECLARE(lv_font_dejavu_20);

LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static const LV_OBJ_FREE_NUM_TYPE objControls_btn_timerReaptSetPage = 1;

static lv_style_t styleRoller_timeSet_bg;
static lv_style_t styleRoller_timeSet_sel;
static lv_style_t styleLebalText_timeSet;
static lv_style_t styleLebalText_dataSet;
static lv_style_t styleLebalText_devStatusSet;
static lv_style_t styleSw_devStatus_bg;
static lv_style_t styleSw_devStatus_indic;
static lv_style_t styleSw_devStatus_knobOn;
static lv_style_t styleSw_devStatus_knobOff;
static lv_style_t styleBk_objBground;
static lv_style_t styleBtn_specialTransparent_rel;
static lv_style_t styleBtn_specialTransparent_pr;
static lv_style_t styleBtn_specialTransparent_tgl_rel;
static lv_style_t styleBtn_specialTransparent_tgl_pr;
static lv_style_t stylePage_reaptSet;
static lv_style_t stylePageCb_reaptSet;
static lv_style_t styleTextPageTitle_reaptSet;
static lv_style_t stylePageBtn_reaptSet;
static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuSetChoiceTitle;
static lv_style_t styleText_menuDevStatus;
static lv_style_t styleText_menuTimeSet;
static lv_style_t styleText_menuReaptSet;
static lv_style_t styleText_menuReaptRem;

static lv_style_t stylePage_timerSetPageA;
static lv_style_t styleBtn_timerSetPageA;
static lv_style_t styleLabel_btnTimerSetPageA_timerInfo;
static lv_style_t styleLabel_btnTimerSetPageA_reaptInfo;
static lv_style_t styleLabel_SetInfoTitle_opreation;
static lv_style_t styleImg_menuFun_btnFun;

static lv_obj_t *guiObj_parentTemp = NULL;

static lv_obj_t *bGround_obj = NULL;
static lv_obj_t *objBtn_currentParamSave = NULL;
static lv_obj_t *objText_menuCurrentTitle = NULL;
static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static lv_obj_t *objRoller_timeSet_hour = NULL;
static lv_obj_t *objRoller_timeSet_minute = NULL;
static lv_obj_t *objLabel_timeSet_hour = NULL;
static lv_obj_t *objLabel_timeSet_minute = NULL;
static lv_obj_t *objLine_timeSet_limit_A = NULL;
static lv_obj_t *objLine_timeSet_limit_B = NULL;
static lv_obj_t *objLine_timeSet_limit_C = NULL;
static lv_obj_t *objCb_dataSet_mon = NULL;
static lv_obj_t *objCb_dataSet_tues = NULL;
static lv_obj_t *objCb_dataSet_wed = NULL;
static lv_obj_t *objCb_dataSet_thur = NULL;
static lv_obj_t *objCb_dataSet_fri = NULL;
static lv_obj_t *objCb_dataSet_sat = NULL;
static lv_obj_t *objCb_dataSet_sun = NULL;
static lv_obj_t *objSw_devStatus_A = NULL;
static lv_obj_t *objSw_devStatus_B = NULL;
static lv_obj_t *objSw_devStatus_C = NULL;
static lv_obj_t *objCont_reaptOp = NULL;
static lv_obj_t *objBtn_reaptSet = NULL;
static lv_obj_t *objpage_reaptSet = NULL;
static lv_obj_t *objpageBtn_confirm = NULL;
static lv_obj_t *objpageBtn_cancel = NULL;
static lv_obj_t *objpageLabel_title = NULL;
static lv_obj_t *objpageBtnLabel_confirm = NULL;
static lv_obj_t *objpageBtnLabel_cancel = NULL;
static lv_obj_t *objLabel_devStatus_A = NULL;
static lv_obj_t *objLabel_devStatus_B = NULL;
static lv_obj_t *objLabel_devStatus_C = NULL;
static lv_obj_t *objLabel_reaptSet = NULL;
static lv_obj_t *objLabel_reaptSetT = NULL;
static lv_obj_t *objLabel_reaptRem = NULL;
static lv_obj_t *objLabel_setChoiceTitle_switchSet = NULL;
static lv_obj_t *objLabel_setChoiceTitle_reaptSet = NULL;
static lv_obj_t *objLabel_setChoiceTitle_timeSet = NULL;

static lv_obj_t *page_timerSetPageA = NULL;
static lv_obj_t *lineUnit_timerSetPageA[8] = {NULL};
static lv_obj_t *btnUnit_timerSetPageA[8] = {NULL};
static lv_obj_t *swUnit_timerSetPageA[8] = {NULL};
static lv_obj_t *labelTimeUnit_timerSetPageA[8] = {NULL};
static lv_obj_t *labelInfoUnit_timerSetPageA[8] = {NULL};

static lv_obj_t *btnTitle_setInfo_save = NULL;
static lv_obj_t *btnTitle_setInfo_cancel = NULL;
static lv_obj_t *labelTitle_setInfo_save = NULL;
static lv_obj_t *labelTitle_setInfo_cancel = NULL;

static usrApp_trigTimer timerSetInfo_temp = {0};
static uint8_t timerSetInfoWeekBithold_cfmTemp = 0;
static uint8_t timerSetInfoWeekBithold_chgTemp = 0;

static uint8_t screenLandscapeCoordinate_objOffset = 0;

static const char strInfoWeek_tab[7][6] = {

	"Mon ", "Tues ", "Wed ", "Thur ", "Fri ", "Sat ", "Sun ",
};

static void lvGui_businessMenu_timerSetPageA(lv_obj_t * obj_Parent);
static void lvGui_businessMenu_timerSetUnitOpreat(lv_obj_t * obj_Parent, uint8_t timerUnit_num);

static void lvGui_bMenuSet_B_pageReaptSet_creat(uint8_t currentWeekHoldBit);

static void currentGui_elementClear(void){

	if(btnTitle_setInfo_save){
		
		lv_obj_del(btnTitle_setInfo_save);
		btnTitle_setInfo_save = NULL;
	}
	if(labelTitle_setInfo_save){
		
		lv_obj_del(labelTitle_setInfo_save);
		labelTitle_setInfo_save = NULL;
	}
	if(btnTitle_setInfo_cancel){

		lv_obj_del(btnTitle_setInfo_cancel);
		btnTitle_setInfo_cancel = NULL;
	}
	if(labelTitle_setInfo_cancel){

		lv_obj_del(labelTitle_setInfo_cancel);
		labelTitle_setInfo_cancel = NULL;
	}	
	if(bGround_obj){
		
		lv_obj_del(bGround_obj);
		bGround_obj = NULL;
	}
	if(objpage_reaptSet){

		lv_obj_del(objpage_reaptSet);
		objpage_reaptSet = NULL;
	}
	if(page_timerSetPageA){

		lv_obj_del(page_timerSetPageA);
		page_timerSetPageA = NULL;
	}

	guiObj_parentTemp = NULL;
}

void guiDispTimeOut_pageTimerSet(void){

	lvGui_usrSwitch(bussinessType_Home);

	currentGui_elementClear();
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	usrGuiBussiness_type guiChg_temp = bussinessType_Menu;

	switch(btnFreeNum){

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME:

			guiChg_temp = bussinessType_Home;

		break;

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK	:
		default:

			guiChg_temp = bussinessType_Menu;

		break;
	}

	lvGui_usrSwitch(guiChg_temp);

	currentGui_elementClear();
	
	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	lv_obj_t *objImg_colorChg = NULL;

	switch(btnFreeNum){

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME:

			objImg_colorChg = imgMenuBtnChoIcon_fun_home;

		break;

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK:
		default:

			objImg_colorChg = imgMenuBtnChoIcon_fun_back;

		break;
	}

	lv_img_set_style(objImg_colorChg, &styleImg_menuFun_btnFun);
	lv_obj_refresh_style(objImg_colorChg);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_btnReaptSet_funBack(lv_obj_t *btn){

//	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(btn);
//	uint8_t tmOpNum = id - LV_OBJ_FREENUM_BASE_TIMERSETINFO_NUM;
//	usrApp_trigTimer timerSetInfoTemp_currentUnit = {0};

//	usrAppActTrigTimer_paramUnitGet(&timerSetInfoTemp_currentUnit, tmOpNum);

	lv_obj_set_click(objBtn_reaptSet, false);

	lvGui_bMenuSet_B_pageReaptSet_creat(timerSetInfoWeekBithold_cfmTemp);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_timerDataReapt_confirm(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(btn);
	char strTemp[31] = {0};

	memset(strTemp, 0, 31);
	if(!timerSetInfoWeekBithold_cfmTemp){
	
		sprintf(strTemp, "no repeat");
	}
	else
	{
		for(uint8_t loopIa = 0; loopIa < 7; loopIa ++){
		
			if((timerSetInfoWeekBithold_cfmTemp >> loopIa) & 0x01)
				strlcat(strTemp, strInfoWeek_tab[loopIa], 31);
		}
	}
	lv_label_set_text(objLabel_reaptRem, strTemp);
	timerSetInfo_temp.tmUp_weekBitHold = timerSetInfoWeekBithold_cfmTemp;

	//业务处理完了在进行控件删除，否则容易dump
	if(objpage_reaptSet){

		lv_obj_del(objpage_reaptSet);
		objpage_reaptSet = NULL;
	}

	lv_obj_set_click(objBtn_reaptSet, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_btnPage_timerDataReapt_cancel(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(btn);

	timerSetInfoWeekBithold_cfmTemp = timerSetInfoWeekBithold_chgTemp;

	//业务处理完了在进行控件删除，否则容易dump
	if(objpage_reaptSet){

		lv_obj_del(objpage_reaptSet);
		objpage_reaptSet = NULL;
	}

	lv_obj_set_click(objBtn_reaptSet, true);
	
	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_timerUnitSetHour(lv_obj_t *rol){

	uint16_t valSet = lv_roller_get_selected(rol);

	timerSetInfo_temp.tmUp_Hour = valSet;

	return LV_RES_OK;
}

static lv_res_t funCb_rollerOpreat_timerUnitSetMinute(lv_obj_t *rol){

	uint16_t valSet = lv_roller_get_selected(rol);

	timerSetInfo_temp.tmUp_Minute = valSet;

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_timerUnitSetOpreation(lv_obj_t *cb){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(cb);
	uint8_t bitNum = id - LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB;

	if(bitNum <= (7 - 1)){

		(lv_cb_is_checked(cb))?
			(timerSetInfoWeekBithold_cfmTemp |= (1 << bitNum)):
			(timerSetInfoWeekBithold_cfmTemp &= ~(1 << bitNum));
	}

	return LV_RES_OK;
}

//static lv_res_t funCb_swOpreat_timerUnitSetOpreation(lv_obj_t *sw){

//	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(sw);

//	switch(id){

//		case (LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 0):
//		case (LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 1):
//		case (LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 2):{

//			uint8_t bitNum = id - LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW;
//			bool opVal_get = !lv_sw_get_state(sw);

//			printf("timerSet op valTrig:%d, val:%d\n", bitNum, opVal_get);

//			(opVal_get)?
//				(timerSetInfo_temp.tmUp_swValTrig |= (1 << bitNum)):
//				(timerSetInfo_temp.tmUp_swValTrig &= ~(1 << bitNum));

//		}break;

//		default:break;
//	}

//	return LV_RES_OK;
//}

static lv_res_t funCb_btnActionClick_timerSetUnit_opreatSave(lv_obj_t *btn){

	lv_obj_t *parentObjTemp_corpse = guiObj_parentTemp;

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(btn);
	uint8_t tmOpNum = id - LV_OBJ_FREENUM_BASE_TIMERSETINFO_NUM;
	stt_devDataPonitTypedef datapointParamSet_temp = {0};
	uint8_t dataTemp = 0;

	lvGui_businessReuse_reactionObjPageElement_funValConfig_get(PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST, &datapointParamSet_temp);
	memcpy(&dataTemp, &datapointParamSet_temp, sizeof(uint8_t));
	timerSetInfo_temp.tmUp_swValTrig = dataTemp;

//	printf("timer dimmer set trigVal:%d.\n", datapointParamSet_temp.devType_dimmer.devDimmer_brightnessVal);

	printf("timer%dParamSave trig, upVal:%d, en:%d, weekbit:%d, hour:%d, minute:%d", tmOpNum,
																					 timerSetInfo_temp.tmUp_swValTrig,
																				     timerSetInfo_temp.tmUp_runningEn,
																				     timerSetInfo_temp.tmUp_weekBitHold,
																				     timerSetInfo_temp.tmUp_Hour,
																				     timerSetInfo_temp.tmUp_Minute);

	usrAppActTrigTimer_paramUnitSet(&timerSetInfo_temp, tmOpNum, true);
	
	memset(&timerSetInfo_temp, 0, sizeof(usrApp_trigTimer));

//	lv_obj_clean(guiObj_parentTemp);
//	lvGui_businessMenu_timerSetPageA(guiObj_parentTemp);	
//	lv_obj_del(bGround_obj);
//	bGround_obj = NULL;

//	lv_obj_clean(guiObj_parentTemp);
	lvGui_businessMenu_timerSetPageA(parentObjTemp_corpse);
	lv_obj_del(btnTitle_setInfo_save);
	lv_obj_del(labelTitle_setInfo_save);
	lv_obj_del(btnTitle_setInfo_cancel);
	lv_obj_del(labelTitle_setInfo_cancel);
	lv_obj_del(bGround_obj);

	labelTitle_setInfo_save = NULL;
	btnTitle_setInfo_save = NULL;
	labelTitle_setInfo_cancel = NULL;
	btnTitle_setInfo_cancel = NULL;
	bGround_obj = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_timerSetUnit_opreatCancel(lv_obj_t *btn){

	lv_obj_t *parentObjTemp_corpse = guiObj_parentTemp;

	memset(&timerSetInfo_temp, 0, sizeof(usrApp_trigTimer));

//	lv_obj_clean(guiObj_parentTemp);
	lvGui_businessMenu_timerSetPageA(parentObjTemp_corpse);
	lv_obj_del(btnTitle_setInfo_save);
	lv_obj_del(labelTitle_setInfo_save);
	lv_obj_del(btnTitle_setInfo_cancel);
	lv_obj_del(labelTitle_setInfo_cancel);
	lv_obj_del(bGround_obj);

	labelTitle_setInfo_save = NULL;
	btnTitle_setInfo_save = NULL;
	labelTitle_setInfo_cancel = NULL;
	btnTitle_setInfo_cancel = NULL;
	bGround_obj = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_timerSetPageA(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(btn);
	uint8_t tmOpNum = id - LV_OBJ_FREENUM_BASE_BTN_TIMESETPAGEA;

	memset(&timerSetInfo_temp, 0, sizeof(usrApp_trigTimer));
	lvGui_businessMenu_timerSetUnitOpreat(guiObj_parentTemp, tmOpNum);

	lv_obj_del(objText_menuCurrentTitle);
	lv_obj_del(menuBtnChoIcon_fun_home);
	lv_obj_del(menuBtnChoIcon_fun_back);
	objText_menuCurrentTitle = NULL;
	menuBtnChoIcon_fun_home = NULL;
	menuBtnChoIcon_fun_back = NULL;

	//业务处理完了在进行控件删除，否则容易dump
	lv_obj_del(page_timerSetPageA);
	page_timerSetPageA = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_swAction_timerSetPageA(lv_obj_t *sw){

	LV_OBJ_FREE_NUM_TYPE id = lv_obj_get_free_num(sw);
	uint8_t tmOpNum = id - LV_OBJ_FREENUM_BASE_SW_TIMESETPAGEA;

	usrAppActTrigTimer_paramUnitGet(&timerSetInfo_temp, tmOpNum);
	(lv_sw_get_state(sw))?
		(timerSetInfo_temp.tmUp_runningEn = 1):
		(timerSetInfo_temp.tmUp_runningEn = 0);
	usrAppActTrigTimer_paramUnitSet(&timerSetInfo_temp, tmOpNum, true);
	memset(&timerSetInfo_temp, 0, sizeof(usrApp_trigTimer));

	return LV_RES_OK;
}

static void lvGui_bMenuSet_B_pageReaptSet_creat(uint8_t currentWeekHoldBit){

	timerSetInfoWeekBithold_chgTemp = currentWeekHoldBit;

	lv_style_copy(&stylePage_reaptSet, &lv_style_plain_color);
	stylePage_reaptSet.body.main_color = LV_COLOR_SILVER;
	stylePage_reaptSet.body.grad_color = LV_COLOR_SILVER;
	stylePage_reaptSet.body.border.part = LV_BORDER_NONE;
	stylePage_reaptSet.body.radius = 6;
	stylePage_reaptSet.body.opa = LV_OPA_90;
	stylePage_reaptSet.body.padding.hor = 0;
	stylePage_reaptSet.body.padding.inner = 0;	

	if(objpage_reaptSet == NULL)
		objpage_reaptSet = lv_page_create(lv_scr_act(), NULL);

	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objpage_reaptSet, 280, 190)):
		(lv_obj_set_size(objpage_reaptSet, 200, 260));
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(objpage_reaptSet, 20, 25)):
		(lv_obj_set_pos(objpage_reaptSet, 20, 45));		
	lv_page_set_style(objpage_reaptSet, LV_PAGE_STYLE_SB, &stylePage_reaptSet);
	lv_page_set_style(objpage_reaptSet, LV_PAGE_STYLE_BG, &stylePage_reaptSet);
	lv_page_set_sb_mode(objpage_reaptSet, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(objpage_reaptSet, false, true); //key opration
	lv_page_set_scrl_layout(objpage_reaptSet, LV_LAYOUT_PRETTY);

	lv_style_copy(&styleTextPageTitle_reaptSet, &lv_style_plain);
	styleTextPageTitle_reaptSet.text.font = &lv_font_consola_19;
	styleTextPageTitle_reaptSet.text.color = LV_COLOR_BLACK;
	objpageLabel_title = lv_label_create(objpage_reaptSet, NULL);
	lv_obj_set_style(objpageLabel_title, &styleTextPageTitle_reaptSet);
	lv_label_set_text(objpageLabel_title, "repeat cycle");
	lv_obj_set_protect(objpageLabel_title, LV_PROTECT_POS);
	lv_obj_align(objpageLabel_title, objpage_reaptSet, LV_ALIGN_IN_TOP_MID, 0, -5);
	
	lv_style_copy(&stylePageCb_reaptSet, &lv_style_plain);
	stylePageCb_reaptSet.body.border.part = LV_BORDER_NONE;
	stylePageCb_reaptSet.body.empty = 1;
	stylePageCb_reaptSet.text.color = LV_COLOR_BLACK;
	stylePageCb_reaptSet.text.font = &lv_font_consola_16;

	objCb_dataSet_mon = lv_cb_create(objpage_reaptSet, NULL);
	lv_obj_set_size(objCb_dataSet_mon, 160, 15);
	lv_cb_set_text(objCb_dataSet_mon, " Monday");
	lv_obj_set_protect(objCb_dataSet_mon, LV_PROTECT_POS);
	lv_obj_set_protect(objCb_dataSet_mon, LV_PROTECT_FOLLOW);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_mon, objpageLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, -60, 10)):
		(lv_obj_align(objCb_dataSet_mon, objpageLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5));
	objCb_dataSet_tues = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_tues, " Tuesday");
	lv_obj_set_protect(objCb_dataSet_tues, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_tues, objpageLabel_title, LV_ALIGN_OUT_BOTTOM_LEFT, 65, 10)):
		(lv_obj_align(objCb_dataSet_tues, objCb_dataSet_mon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	objCb_dataSet_wed = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_wed, " Wednesday");
	lv_obj_set_protect(objCb_dataSet_wed, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_wed, objCb_dataSet_mon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)):
		(lv_obj_align(objCb_dataSet_wed, objCb_dataSet_tues, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	objCb_dataSet_thur = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_thur, " Thursday");
	lv_obj_set_protect(objCb_dataSet_thur, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_thur, objCb_dataSet_tues, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)):
		(lv_obj_align(objCb_dataSet_thur, objCb_dataSet_wed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	objCb_dataSet_fri = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_fri, " Friday");
	lv_obj_set_protect(objCb_dataSet_fri, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_fri, objCb_dataSet_wed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)):
		(lv_obj_align(objCb_dataSet_fri, objCb_dataSet_thur, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	objCb_dataSet_sat = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_sat, " Saturday");
	lv_obj_set_protect(objCb_dataSet_sat, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_sat, objCb_dataSet_thur, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)):
		(lv_obj_align(objCb_dataSet_sat, objCb_dataSet_fri, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	objCb_dataSet_sun = lv_cb_create(objpage_reaptSet, NULL);
	lv_cb_set_text(objCb_dataSet_sun, " Sunday");
	lv_obj_set_protect(objCb_dataSet_sun, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objCb_dataSet_sun, objCb_dataSet_fri, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10)):
		(lv_obj_align(objCb_dataSet_sun, objCb_dataSet_sat, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10));
	lv_cb_set_style(objCb_dataSet_mon, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_tues, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_wed, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_thur, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_fri, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_sat, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_cb_set_style(objCb_dataSet_sun, LV_CB_STYLE_BG, &stylePageCb_reaptSet);
	lv_obj_set_free_num(objCb_dataSet_mon, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 0);
	lv_obj_set_free_num(objCb_dataSet_tues, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 1);
	lv_obj_set_free_num(objCb_dataSet_wed, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 2);
	lv_obj_set_free_num(objCb_dataSet_thur, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 3);
	lv_obj_set_free_num(objCb_dataSet_fri, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 4);
	lv_obj_set_free_num(objCb_dataSet_sat, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 5);
	lv_obj_set_free_num(objCb_dataSet_sun, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_CB + 6);
	lv_cb_set_action(objCb_dataSet_mon, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_tues, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_wed, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_thur, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_fri, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_sat, funCb_cbOpreat_timerUnitSetOpreation);
	lv_cb_set_action(objCb_dataSet_sun, funCb_cbOpreat_timerUnitSetOpreation);
	if((currentWeekHoldBit >> 0) & 0x01)
		lv_cb_set_checked(objCb_dataSet_mon, true);
	if((currentWeekHoldBit >> 1) & 0x01)
		lv_cb_set_checked(objCb_dataSet_tues, true);
	if((currentWeekHoldBit >> 2) & 0x01)
		lv_cb_set_checked(objCb_dataSet_wed, true);
	if((currentWeekHoldBit >> 3) & 0x01)
		lv_cb_set_checked(objCb_dataSet_thur, true);
	if((currentWeekHoldBit >> 4) & 0x01)
		lv_cb_set_checked(objCb_dataSet_fri, true);
	if((currentWeekHoldBit >> 5) & 0x01)
		lv_cb_set_checked(objCb_dataSet_sat, true);
	if((currentWeekHoldBit >> 6) & 0x01)
		lv_cb_set_checked(objCb_dataSet_sun, true);

	objpageBtn_confirm = lv_btn_create(objpage_reaptSet, NULL);
	lv_obj_set_free_num(objpageBtn_confirm, objControls_btn_timerReaptSetPage);
    lv_btn_set_style(objpageBtn_confirm, LV_BTN_STYLE_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(objpageBtn_confirm, LV_BTN_STYLE_PR, &styleBtn_specialTransparent_pr);
    lv_btn_set_style(objpageBtn_confirm, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(objpageBtn_confirm, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent_pr);
	lv_btn_set_fit(objpageBtn_confirm, false, false);
	lv_obj_set_size(objpageBtn_confirm, 70, 25);
	lv_page_glue_obj(objpageBtn_confirm, true);
	lv_obj_set_protect(objpageBtn_confirm, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objpageBtn_confirm, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageBtn_confirm, objpage_reaptSet, LV_ALIGN_IN_BOTTOM_LEFT, 45, -1)):
		(lv_obj_align(objpageBtn_confirm, objpage_reaptSet, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1));
	lv_btn_set_action(objpageBtn_confirm, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_timerDataReapt_confirm);
	objpageBtn_cancel = lv_btn_create(objpage_reaptSet, objpageBtn_confirm);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objpageBtn_cancel, objpage_reaptSet, LV_ALIGN_IN_BOTTOM_RIGHT, -45, -1)):
		(lv_obj_align(objpageBtn_cancel, objpageBtn_confirm, LV_ALIGN_CENTER, 85, 0));
	lv_btn_set_action(objpageBtn_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionPress_btnPage_timerDataReapt_cancel);

	lv_style_copy(&stylePageBtn_reaptSet, &lv_style_plain);
	stylePageBtn_reaptSet.text.font = &lv_font_ariblk_18;
	stylePageBtn_reaptSet.text.color = LV_COLOR_MAKE(0, 128, 255);
	objpageBtnLabel_confirm = lv_label_create(objpageBtn_confirm, NULL);
	lv_obj_set_style(objpageBtnLabel_confirm, &stylePageBtn_reaptSet);
	lv_label_set_text(objpageBtnLabel_confirm, "confirm");
	objpageBtnLabel_cancel = lv_label_create(objpageBtn_cancel, objpageBtnLabel_confirm);
	lv_obj_set_style(objpageBtnLabel_cancel, &stylePageBtn_reaptSet);
	lv_label_set_text(objpageBtnLabel_cancel, "cancel");

	usrApp_fullScreenRefresh_self(20, 80);
}

static void lvGui_businessMenu_timerSetPageA(lv_obj_t * obj_Parent){

	usrApp_trigTimer usrAppTimerParamInfo[USRAPP_VALDEFINE_TRIGTIMER_NUM] = {0};

	usrAppActTrigTimer_paramGet(usrAppTimerParamInfo);

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&stylePage_timerSetPageA, &lv_style_plain);
	stylePage_timerSetPageA.body.main_color = LV_COLOR_WHITE;
	stylePage_timerSetPageA.body.grad_color = LV_COLOR_WHITE;
	stylePage_timerSetPageA.body.border.color = LV_COLOR_WHITE;
	stylePage_timerSetPageA.body.border.width = 1;
	stylePage_timerSetPageA.body.border.opa = LV_OPA_70;
	stylePage_timerSetPageA.body.radius = 0;
	stylePage_timerSetPageA.body.opa = LV_OPA_60;
	stylePage_timerSetPageA.body.padding.hor = 3;
	stylePage_timerSetPageA.body.padding.inner = 8;

	lv_style_copy(&styleBtn_timerSetPageA, &lv_style_btn_rel);
	styleBtn_timerSetPageA.body.main_color = LV_COLOR_TRANSP;
	styleBtn_timerSetPageA.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_timerSetPageA.body.border.part = LV_BORDER_NONE;
	styleBtn_timerSetPageA.body.opa = LV_OPA_TRANSP;
	styleBtn_timerSetPageA.body.radius = 0;
	styleBtn_timerSetPageA.body.shadow.width = 0;

	lv_style_copy(&styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun.image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun.image.color = LV_COLOR_MAKE(200, 191, 231);

	lv_style_copy(&styleLabel_btnTimerSetPageA_timerInfo, &lv_style_plain);
	styleLabel_btnTimerSetPageA_timerInfo.text.font = &lv_font_dejavu_20;
	styleLabel_btnTimerSetPageA_timerInfo.text.color = LV_COLOR_BLACK;
	lv_style_copy(&styleLabel_btnTimerSetPageA_reaptInfo, &lv_style_plain);
	styleLabel_btnTimerSetPageA_reaptInfo.text.font = &lv_font_consola_16;
	styleLabel_btnTimerSetPageA_reaptInfo.text.color = LV_COLOR_BLACK;

	objText_menuCurrentTitle = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(objText_menuCurrentTitle, "timer set");
	lv_obj_align(objText_menuCurrentTitle, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -7);
	lv_obj_set_style(objText_menuCurrentTitle, &styleText_menuLevel_A);

	menuBtnChoIcon_fun_home = lv_imgbtn_create(obj_Parent, NULL);
	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 220, 23)):
		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 140, 23));
	lv_imgbtn_set_src(menuBtnChoIcon_fun_home, LV_BTN_STATE_REL, &iconMenu_funBack_homePage);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_home, LV_BTN_STATE_PR, &iconMenu_funBack_homePage);
	lv_imgbtn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STATE_PR, &styleImg_menuFun_btnFun);
	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_obj_set_free_num(menuBtnChoIcon_fun_home, LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, menuBtnChoIcon_fun_home);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 0, 22);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack_arrowLeft);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack_arrowLeft);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_obj_set_free_num(menuBtnChoIcon_fun_back, LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK);

//	menuBtnChoIcon_fun_home = lv_btn_create(obj_Parent, NULL);
//	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
//	(devStatusDispMethod_landscapeIf_get())?
//		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 240, 25)):
//		(lv_obj_set_pos(menuBtnChoIcon_fun_home, 160, 25));
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_home, LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_REL, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_PR, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_REL, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_PR, &styleBtn_timerSetPageA);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_home = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_home, &iconMenu_funBack_homePage);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_home, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_home, menuBtnChoIcon_fun_home, LV_ALIGN_IN_RIGHT_MID, -25, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);

//	menuBtnChoIcon_fun_back = lv_btn_create(obj_Parent, menuBtnChoIcon_fun_home);
//	lv_obj_set_pos(menuBtnChoIcon_fun_back, 0, 25);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_back, LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_back = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_back, &iconMenu_funBack_arrowLeft);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_back, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_back, menuBtnChoIcon_fun_back, LV_ALIGN_IN_LEFT_MID, 5, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_back, true);

//	menuBtnChoIcon_fun_home = lv_btn_create(obj_Parent, NULL);
//	lv_obj_set_size(menuBtnChoIcon_fun_home, 100, 50);
//	lv_obj_set_pos(menuBtnChoIcon_fun_home, 160, 25);
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_home, LV_OBJ_FREENUM_BTNNUM_DEF_MENUHOME);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_REL, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_PR, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_REL, &styleBtn_timerSetPageA);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_PR, &styleBtn_timerSetPageA);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_home, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_home = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_home, &iconMenu_funBack_homePage);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_home, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_home, menuBtnChoIcon_fun_home, LV_ALIGN_IN_RIGHT_MID, -25, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_home, true);

//	menuBtnChoIcon_fun_back = lv_btn_create(obj_Parent, menuBtnChoIcon_fun_home);
//	lv_obj_set_pos(menuBtnChoIcon_fun_back, 0, 25);
//	lv_obj_set_free_num(menuBtnChoIcon_fun_back, LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);
//	imgMenuBtnChoIcon_fun_back = lv_img_create(obj_Parent, NULL);
//	lv_img_set_src(imgMenuBtnChoIcon_fun_back, &iconMenu_funBack_arrowLeft);
//	lv_obj_set_protect(imgMenuBtnChoIcon_fun_back, LV_PROTECT_POS);
//	lv_obj_align(imgMenuBtnChoIcon_fun_back, menuBtnChoIcon_fun_back, LV_ALIGN_IN_LEFT_MID, 5, 4);
//	lv_obj_set_top(menuBtnChoIcon_fun_back, true);

	page_timerSetPageA = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(page_timerSetPageA, 320, 180)):
		(lv_obj_set_size(page_timerSetPageA, 240, 260));
	lv_obj_set_pos(page_timerSetPageA, 0, 75);
	lv_style_copy(&stylePage_timerSetPageA, &lv_style_plain);
	stylePage_timerSetPageA.body.main_color = LV_COLOR_WHITE;
	stylePage_timerSetPageA.body.grad_color = LV_COLOR_WHITE;
	lv_page_set_style(page_timerSetPageA, LV_PAGE_STYLE_SB, &stylePage_timerSetPageA);
	lv_page_set_style(page_timerSetPageA, LV_PAGE_STYLE_BG, &stylePage_timerSetPageA);
	lv_page_set_sb_mode(page_timerSetPageA, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(page_timerSetPageA, false, true); //key opration
	lv_page_set_scrl_layout(page_timerSetPageA, LV_LAYOUT_CENTER);

	//分割线
	for(uint8_t loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){

		lineUnit_timerSetPageA[loop] = lv_obj_create(page_timerSetPageA, NULL);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(lineUnit_timerSetPageA[loop] , 314, 1)):
			(lv_obj_set_size(lineUnit_timerSetPageA[loop] , 234, 1));
		lv_obj_set_protect(lineUnit_timerSetPageA[loop], LV_PROTECT_POS);
		lv_obj_set_pos(lineUnit_timerSetPageA[loop], 3, 65 * loop);
	}

	//区域按键
	for(uint8_t loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){

		btnUnit_timerSetPageA[loop] = lv_btn_create(page_timerSetPageA, NULL);
		lv_obj_set_free_num(btnUnit_timerSetPageA[loop], LV_OBJ_FREENUM_BASE_BTN_TIMESETPAGEA + loop);
		lv_btn_set_action(btnUnit_timerSetPageA[loop], LV_BTN_ACTION_CLICK, funCb_btnActionClick_timerSetPageA);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(btnUnit_timerSetPageA[loop], 240, 60)):
			(lv_obj_set_size(btnUnit_timerSetPageA[loop], 160, 60));
		lv_page_glue_obj(btnUnit_timerSetPageA[loop], true);
		lv_btn_set_style(btnUnit_timerSetPageA[loop], LV_BTN_STYLE_REL, &styleBtn_timerSetPageA);
		lv_btn_set_style(btnUnit_timerSetPageA[loop], LV_BTN_STYLE_PR, &styleBtn_timerSetPageA);
		lv_btn_set_style(btnUnit_timerSetPageA[loop], LV_BTN_STYLE_TGL_REL, &styleBtn_timerSetPageA);
		lv_btn_set_style(btnUnit_timerSetPageA[loop], LV_BTN_STYLE_TGL_PR, &styleBtn_timerSetPageA);
		lv_obj_set_protect(btnUnit_timerSetPageA[loop], LV_PROTECT_POS);
		lv_obj_align(btnUnit_timerSetPageA[loop], lineUnit_timerSetPageA[loop], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	}

	//一级菜单定时器开关
	for(uint8_t loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){

		swUnit_timerSetPageA[loop] = lv_sw_create(page_timerSetPageA, NULL);
		lv_obj_set_free_num(swUnit_timerSetPageA[loop], LV_OBJ_FREENUM_BASE_SW_TIMESETPAGEA + loop);
		lv_sw_set_action(swUnit_timerSetPageA[loop], funCb_swAction_timerSetPageA);
		lv_obj_set_size(swUnit_timerSetPageA[loop] , 50, 20);
		lv_obj_set_protect(swUnit_timerSetPageA[loop], LV_PROTECT_POS);
		lv_obj_align(swUnit_timerSetPageA[loop], lineUnit_timerSetPageA[loop], LV_ALIGN_OUT_BOTTOM_RIGHT, -17, 20);
		(usrAppTimerParamInfo[loop].tmUp_runningEn)?
			(lv_sw_on(swUnit_timerSetPageA[loop])):
			(lv_sw_off(swUnit_timerSetPageA[loop]));
	}

	//按键文本说明
	for(uint8_t loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){

		char strTemp[31] = {0};

		//时刻说明
		sprintf(strTemp, "%02d:%02d", usrAppTimerParamInfo[loop].tmUp_Hour,
								  usrAppTimerParamInfo[loop].tmUp_Minute);
		labelTimeUnit_timerSetPageA[loop] = lv_label_create(btnUnit_timerSetPageA[loop], NULL);
		lv_obj_set_style(labelTimeUnit_timerSetPageA[loop], &styleLabel_btnTimerSetPageA_timerInfo);
		lv_obj_set_size(labelTimeUnit_timerSetPageA[loop], 140, 30);
		lv_label_set_text(labelTimeUnit_timerSetPageA[loop], strTemp);
		lv_obj_set_protect(labelTimeUnit_timerSetPageA[loop], LV_PROTECT_POS);
		lv_obj_align(labelTimeUnit_timerSetPageA[loop], btnUnit_timerSetPageA[loop], LV_ALIGN_IN_TOP_LEFT, 10, 7);

		//星期重复说明
		memset(strTemp, 0, 31);
		if(!usrAppTimerParamInfo[loop].tmUp_weekBitHold){

			sprintf(strTemp, "no repeat");
		}
		else
		{
			for(uint8_t loopIa = 0; loopIa < 7; loopIa ++){
			
				if((usrAppTimerParamInfo[loop].tmUp_weekBitHold >> loopIa) & 0x01)
					strlcat(strTemp, strInfoWeek_tab[loopIa], 31);
			}
		}
		labelInfoUnit_timerSetPageA[loop] = lv_label_create(btnUnit_timerSetPageA[loop], NULL);
		lv_obj_set_style(labelInfoUnit_timerSetPageA[loop], &styleLabel_btnTimerSetPageA_reaptInfo);
		lv_label_set_long_mode(labelInfoUnit_timerSetPageA[loop], LV_LABEL_LONG_DOT);
		lv_obj_set_size(labelInfoUnit_timerSetPageA[loop], 140, 30);
		lv_label_set_text(labelInfoUnit_timerSetPageA[loop], strTemp);
		lv_obj_set_protect(labelInfoUnit_timerSetPageA[loop], LV_PROTECT_POS);
		lv_obj_align(labelInfoUnit_timerSetPageA[loop], labelTimeUnit_timerSetPageA[loop], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
	}

	vTaskDelay(20 / portTICK_PERIOD_MS);
	lv_obj_refresh_style(page_timerSetPageA);
	lv_obj_refresh_style(obj_Parent);

	usrApp_fullScreenRefresh_self(20, 80);
}

static void lvGui_businessMenu_timerSetUnitOpreat(lv_obj_t * obj_Parent, uint8_t timerUnit_num){

	char strTemp[31] = {0};
	usrApp_trigTimer timerSetInfoTemp_currentUnit = {0};

//	printf("usrApp timer:%d, set opreat.\n", timerUnit_num);

	(devStatusDispMethod_landscapeIf_get())?
		(screenLandscapeCoordinate_objOffset = 40):
		(screenLandscapeCoordinate_objOffset = 0);

	usrAppActTrigTimer_paramUnitGet(&timerSetInfoTemp_currentUnit, timerUnit_num);
	memcpy(&timerSetInfo_temp, &timerSetInfoTemp_currentUnit, sizeof(usrApp_trigTimer));
	timerSetInfoWeekBithold_cfmTemp = timerSetInfoTemp_currentUnit.tmUp_weekBitHold;

	lv_style_copy(&styleLabel_SetInfoTitle_opreation, &lv_style_plain);
	styleLabel_SetInfoTitle_opreation.text.font = &lv_font_consola_19;
	styleLabel_SetInfoTitle_opreation.text.color = LV_COLOR_WHITE;
	
	lv_style_copy(&styleBk_objBground, &lv_style_plain);
	styleBk_objBground.body.main_color = LV_COLOR_WHITE;
	styleBk_objBground.body.grad_color = LV_COLOR_WHITE;
	styleBk_objBground.body.border.color = LV_COLOR_WHITE;
	styleBk_objBground.body.border.width = 1;
	styleBk_objBground.body.border.opa = LV_OPA_70;
	styleBk_objBground.body.radius = 0;
	styleBk_objBground.body.opa = LV_OPA_60;
	styleBk_objBground.body.padding.hor = 3;			
	styleBk_objBground.body.padding.inner = 8;	

    lv_style_copy(&styleBtn_specialTransparent_rel, &lv_style_btn_rel);
    lv_style_copy(&styleBtn_specialTransparent_pr, &lv_style_btn_pr);
    lv_style_copy(&styleBtn_specialTransparent_rel, &lv_style_btn_tgl_rel);
    lv_style_copy(&styleBtn_specialTransparent_pr, &lv_style_btn_tgl_pr);

	styleBtn_specialTransparent_rel.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_rel.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_rel.body.border.part = LV_BORDER_NONE;
	styleBtn_specialTransparent_pr.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_pr.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_pr.body.border.part = LV_BORDER_NONE;
	styleBtn_specialTransparent_tgl_rel.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_tgl_rel.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_tgl_rel.body.border.part = LV_BORDER_NONE;
	styleBtn_specialTransparent_tgl_pr.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_tgl_pr.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent_tgl_pr.body.border.part = LV_BORDER_NONE;

    styleBtn_specialTransparent_rel.body.opa = LV_OPA_TRANSP;
    styleBtn_specialTransparent_pr.body.opa  = LV_OPA_TRANSP;
    styleBtn_specialTransparent_tgl_rel.body.opa = LV_OPA_TRANSP;
    styleBtn_specialTransparent_tgl_pr.body.opa = LV_OPA_TRANSP;

	styleBtn_specialTransparent_rel.body.radius = 0;
	styleBtn_specialTransparent_pr.body.radius = 0;
	styleBtn_specialTransparent_tgl_rel.body.radius = 0;
	styleBtn_specialTransparent_tgl_pr.body.radius = 0;
    styleBtn_specialTransparent_rel.body.shadow.width =  0;
    styleBtn_specialTransparent_pr.body.shadow.width  =  0;
    styleBtn_specialTransparent_tgl_rel.body.shadow.width = 0;
    styleBtn_specialTransparent_tgl_pr.body.shadow.width =  0;

	btnTitle_setInfo_save = lv_btn_create(obj_Parent, NULL);
	lv_obj_set_free_num(btnTitle_setInfo_save, LV_OBJ_FREENUM_BASE_TIMERSETINFO_NUM + timerUnit_num); //根据freeNum号进行对应timer存储操作
	lv_btn_set_action(btnTitle_setInfo_save, LV_BTN_ACTION_CLICK, funCb_btnActionClick_timerSetUnit_opreatSave);
	lv_obj_set_size(btnTitle_setInfo_save, 60, 30);
	lv_obj_set_protect(btnTitle_setInfo_save, LV_PROTECT_POS);
	lv_obj_align(btnTitle_setInfo_save, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_btn_set_style(btnTitle_setInfo_save, LV_BTN_STYLE_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(btnTitle_setInfo_save, LV_BTN_STYLE_PR, &styleBtn_specialTransparent_pr);
    lv_btn_set_style(btnTitle_setInfo_save, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(btnTitle_setInfo_save, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent_pr);
	labelTitle_setInfo_save = lv_label_create(obj_Parent, NULL);
	lv_obj_set_size(labelTitle_setInfo_save, 60, 30);
	lv_obj_set_protect(labelTitle_setInfo_save, LV_PROTECT_POS);
	lv_obj_align(labelTitle_setInfo_save, btnTitle_setInfo_save, LV_ALIGN_IN_BOTTOM_RIGHT, -5, 0);
	lv_label_set_style(labelTitle_setInfo_save, &styleLabel_SetInfoTitle_opreation);
	lv_label_set_recolor(labelTitle_setInfo_save, true);
	lv_label_set_text(labelTitle_setInfo_save, "#4444FF save#");
	lv_obj_set_top(labelTitle_setInfo_save, true);
	btnTitle_setInfo_cancel = lv_btn_create(obj_Parent, NULL);
	lv_btn_set_action(btnTitle_setInfo_cancel, LV_BTN_ACTION_CLICK, funCb_btnActionClick_timerSetUnit_opreatCancel);
	lv_obj_set_size(btnTitle_setInfo_cancel, 60, 30);
	lv_obj_set_protect(btnTitle_setInfo_cancel, LV_PROTECT_POS);
	lv_obj_align(btnTitle_setInfo_cancel, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_btn_set_style(btnTitle_setInfo_cancel, LV_BTN_STYLE_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(btnTitle_setInfo_cancel, LV_BTN_STYLE_PR, &styleBtn_specialTransparent_pr);
    lv_btn_set_style(btnTitle_setInfo_cancel, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(btnTitle_setInfo_cancel, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent_pr);
	labelTitle_setInfo_cancel = lv_label_create(obj_Parent, NULL);
	lv_obj_set_size(labelTitle_setInfo_cancel, 60, 30);
	lv_obj_set_protect(labelTitle_setInfo_cancel, LV_PROTECT_POS);
	lv_obj_align(labelTitle_setInfo_cancel, btnTitle_setInfo_cancel, LV_ALIGN_IN_BOTTOM_LEFT, 5, 0);
	lv_label_set_style(labelTitle_setInfo_cancel, &styleLabel_SetInfoTitle_opreation);
	lv_label_set_recolor(labelTitle_setInfo_cancel, true);
	lv_label_set_text(labelTitle_setInfo_cancel, "#4444FF cancel#");
	lv_obj_set_top(labelTitle_setInfo_cancel, true);
	
	bGround_obj = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(bGround_obj, 320, 165)):
		(lv_obj_set_size(bGround_obj, 240, 245));
	lv_obj_set_pos(bGround_obj, 0, 75);
	lv_style_copy(&styleBk_objBground, &lv_style_plain); 
	styleBk_objBground.body.main_color = LV_COLOR_WHITE;
	styleBk_objBground.body.grad_color = LV_COLOR_WHITE;					
	lv_page_set_style(bGround_obj, LV_PAGE_STYLE_SB, &styleBk_objBground);
	lv_page_set_style(bGround_obj, LV_PAGE_STYLE_BG, &styleBk_objBground);
	lv_page_set_sb_mode(bGround_obj, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(bGround_obj, false, true); //key opration
	lv_page_set_scrl_height(bGround_obj, 410);
	lv_page_set_scrl_layout(bGround_obj, LV_LAYOUT_PRETTY);

	//设置类分割线绘制
	objLine_timeSet_limit_A = lv_obj_create(bGround_obj, NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objLine_timeSet_limit_A , 306, 1)):
		(lv_obj_set_size(objLine_timeSet_limit_A , 226, 1));
	lv_obj_set_protect(objLine_timeSet_limit_A, LV_PROTECT_POS);
	lv_obj_align(objLine_timeSet_limit_A, bGround_obj, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	objLine_timeSet_limit_B = lv_obj_create(bGround_obj, objLine_timeSet_limit_A);
	lv_obj_set_protect(objLine_timeSet_limit_B, LV_PROTECT_POS);
	lv_obj_align(objLine_timeSet_limit_B, objLine_timeSet_limit_A, LV_ALIGN_CENTER, 0, 140);
	objLine_timeSet_limit_C = lv_obj_create(bGround_obj, objLine_timeSet_limit_A);
	lv_obj_set_protect(objLine_timeSet_limit_C, LV_PROTECT_POS);
	lv_obj_align(objLine_timeSet_limit_C, objLine_timeSet_limit_B, LV_ALIGN_CENTER, 0, 90);

	lv_style_copy(&styleText_menuSetChoiceTitle, &lv_style_plain);
	styleText_menuSetChoiceTitle.text.font = &lv_font_consola_16;
	styleText_menuSetChoiceTitle.text.color = LV_COLOR_GRAY;
	objLabel_setChoiceTitle_switchSet = lv_label_create(bGround_obj, NULL);
	lv_obj_set_style(objLabel_setChoiceTitle_switchSet, &styleText_menuSetChoiceTitle);
	lv_label_set_text(objLabel_setChoiceTitle_switchSet, "switch setting:");
	lv_obj_set_protect(objLabel_setChoiceTitle_switchSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_setChoiceTitle_switchSet, objLine_timeSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 9);
	objLabel_setChoiceTitle_reaptSet = lv_label_create(bGround_obj, NULL);
	lv_obj_set_style(objLabel_setChoiceTitle_reaptSet, &styleText_menuSetChoiceTitle);
	lv_label_set_text(objLabel_setChoiceTitle_reaptSet, "repeat setting:");
	lv_obj_set_protect(objLabel_setChoiceTitle_reaptSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_setChoiceTitle_reaptSet, objLine_timeSet_limit_B, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 9);
	objLabel_setChoiceTitle_timeSet = lv_label_create(bGround_obj, NULL);
	lv_obj_set_style(objLabel_setChoiceTitle_timeSet, &styleText_menuSetChoiceTitle);
	lv_label_set_text(objLabel_setChoiceTitle_timeSet, "time setting:");
	lv_obj_set_protect(objLabel_setChoiceTitle_timeSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_setChoiceTitle_timeSet, objLine_timeSet_limit_C, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 9);

	//定时开关状态设定选项绘制
	lv_style_copy(&styleSw_devStatus_bg, &lv_style_pretty);
	styleSw_devStatus_bg.body.radius = LV_RADIUS_CIRCLE;
	lv_style_copy(&styleSw_devStatus_indic, &lv_style_pretty_color);
	styleSw_devStatus_indic.body.radius = LV_RADIUS_CIRCLE;
	styleSw_devStatus_indic.body.main_color = LV_COLOR_HEX(0x9fc8ef);
	styleSw_devStatus_indic.body.grad_color = LV_COLOR_HEX(0x9fc8ef);
	styleSw_devStatus_indic.body.padding.hor = 0;
	styleSw_devStatus_indic.body.padding.ver = 0;	
	lv_style_copy(&styleSw_devStatus_knobOff, &lv_style_pretty);
	styleSw_devStatus_knobOff.body.radius = LV_RADIUS_CIRCLE;
	styleSw_devStatus_knobOff.body.shadow.width = 4;
	styleSw_devStatus_knobOff.body.shadow.type = LV_SHADOW_BOTTOM;
	lv_style_copy(&styleSw_devStatus_knobOn, &lv_style_pretty_color);
	styleSw_devStatus_knobOn.body.radius = LV_RADIUS_CIRCLE;
	styleSw_devStatus_knobOn.body.shadow.width = 4;
	styleSw_devStatus_knobOn.body.shadow.type = LV_SHADOW_BOTTOM;

//	objSw_devStatus_A = lv_sw_create(bGround_obj, NULL);
//	lv_obj_set_size(objSw_devStatus_A, 55, 25);
//	lv_sw_set_style(objSw_devStatus_A, LV_SW_STYLE_BG, &styleSw_devStatus_bg);
//	lv_sw_set_style(objSw_devStatus_A, LV_SW_STYLE_INDIC, &styleSw_devStatus_indic);
//	lv_sw_set_style(objSw_devStatus_A, LV_SW_STYLE_KNOB_ON, &styleSw_devStatus_knobOn);
//	lv_sw_set_style(objSw_devStatus_A, LV_SW_STYLE_KNOB_OFF, &styleSw_devStatus_knobOff);
//	lv_sw_set_anim_time(objSw_devStatus_A, 100);
//	lv_obj_set_protect(objSw_devStatus_A, LV_PROTECT_POS);
//	lv_obj_align(objSw_devStatus_A, objLine_timeSet_limit_A, LV_ALIGN_OUT_BOTTOM_RIGHT, -15, 35);
//	objSw_devStatus_B = lv_sw_create(bGround_obj, objSw_devStatus_A);
//	lv_obj_set_protect(objSw_devStatus_A, LV_PROTECT_POS);
//	lv_obj_align(objSw_devStatus_B, objSw_devStatus_A, LV_ALIGN_CENTER, 0, 35);
//	objSw_devStatus_C = lv_sw_create(bGround_obj, objSw_devStatus_A);
//	lv_obj_set_protect(objSw_devStatus_C, LV_PROTECT_POS);
//	lv_obj_align(objSw_devStatus_C, objSw_devStatus_B, LV_ALIGN_CENTER, 0, 35);
//	lv_obj_set_free_num(objSw_devStatus_A, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 0);
//	lv_obj_set_free_num(objSw_devStatus_B, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 1);
//	lv_obj_set_free_num(objSw_devStatus_C, LV_OBJ_FREENUM_BASE_TIMER_UNITOP_SW + 2);
//	lv_sw_set_action(objSw_devStatus_A, funCb_swOpreat_timerUnitSetOpreation);
//	lv_sw_set_action(objSw_devStatus_B, funCb_swOpreat_timerUnitSetOpreation);
//	lv_sw_set_action(objSw_devStatus_C, funCb_swOpreat_timerUnitSetOpreation);
//	(timerSetInfoTemp_currentUnit.tmUp_swValTrig & (1 << 0))?
//		(lv_sw_on(objSw_devStatus_A)):
//		(lv_sw_off(objSw_devStatus_A));
//	(timerSetInfoTemp_currentUnit.tmUp_swValTrig & (1 << 1))?
//		(lv_sw_on(objSw_devStatus_B)):
//		(lv_sw_off(objSw_devStatus_B));
//	(timerSetInfoTemp_currentUnit.tmUp_swValTrig & (1 << 2))?
//		(lv_sw_on(objSw_devStatus_C)):
//		(lv_sw_off(objSw_devStatus_C));

//	lv_style_copy(&styleText_menuDevStatus, &lv_style_plain);
//	styleText_menuDevStatus.text.font = &lv_font_consola_16;
//	styleText_menuDevStatus.text.color = LV_COLOR_BLACK;

//	objLabel_devStatus_A = lv_label_create(bGround_obj, NULL);
//	lv_label_set_text(objLabel_devStatus_A, "switch-A:");
//	lv_obj_set_protect(objLabel_devStatus_A, LV_PROTECT_POS);
//	lv_obj_align(objLabel_devStatus_A, objLine_timeSet_limit_A, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 40);
//	lv_obj_set_style(objLabel_devStatus_A, &styleText_menuDevStatus);
//	objLabel_devStatus_B = lv_label_create(bGround_obj, objLabel_devStatus_A);
//	lv_label_set_text(objLabel_devStatus_B, "switch-B:");
//	lv_obj_set_protect(objLabel_devStatus_B, LV_PROTECT_POS);
//	lv_obj_align(objLabel_devStatus_B, objLabel_devStatus_A, LV_ALIGN_CENTER, 0, 35);
//	objLabel_devStatus_C = lv_label_create(bGround_obj, objLabel_devStatus_A);
//	lv_label_set_text(objLabel_devStatus_C, "switch-C:");
//	lv_obj_set_protect(objLabel_devStatus_C, LV_PROTECT_POS);
//	lv_obj_align(objLabel_devStatus_C, objLabel_devStatus_B, LV_ALIGN_CENTER, 0, 35);

	uint8_t datapointReaction_temp = timerSetInfo_temp.tmUp_swValTrig;
	lvGui_businessReuse_reactionObjPageElement_creat(bGround_obj, 
													 PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST,
													 52,
													 (stt_devDataPonitTypedef *)&datapointReaction_temp);

	objBtn_reaptSet = lv_btn_create(bGround_obj, NULL);
    lv_btn_set_style(objBtn_reaptSet, LV_BTN_STYLE_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(objBtn_reaptSet, LV_BTN_STYLE_PR, &styleBtn_specialTransparent_pr);
    lv_btn_set_style(objBtn_reaptSet, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent_rel);
    lv_btn_set_style(objBtn_reaptSet, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent_pr);
	lv_btn_set_fit(objBtn_reaptSet, false, false);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(objBtn_reaptSet, 260, 55)):
		(lv_obj_set_size(objBtn_reaptSet, 200, 55));
	lv_page_glue_obj(objBtn_reaptSet, true);
	lv_obj_set_protect(objBtn_reaptSet, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(objBtn_reaptSet, LV_PROTECT_POS);
	lv_obj_align(objBtn_reaptSet, objLine_timeSet_limit_B, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 25);
	lv_obj_set_free_num(objBtn_reaptSet, LV_OBJ_FREENUM_BASE_TIMERSETINFO_NUM + timerUnit_num);
	lv_btn_set_action(objBtn_reaptSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_btnReaptSet_funBack);

	lv_style_copy(&styleText_menuReaptSet, &lv_style_plain);
	styleText_menuReaptSet.text.font = &lv_font_dejavu_20;
	styleText_menuReaptSet.text.color = LV_COLOR_MAKE(99, 94, 168);
	objLabel_reaptSet = lv_label_create(objBtn_reaptSet, NULL);
	lv_obj_set_style(objLabel_reaptSet, &styleText_menuReaptSet);
	lv_label_set_text(objLabel_reaptSet, "repeat cycle");
	lv_obj_set_protect(objLabel_reaptSet, LV_PROTECT_POS);
	lv_obj_align(objLabel_reaptSet, objBtn_reaptSet, LV_ALIGN_IN_TOP_LEFT, 40 + screenLandscapeCoordinate_objOffset, 10);
	objLabel_reaptSetT = lv_label_create(objBtn_reaptSet, NULL);
	lv_obj_set_style(objLabel_reaptSetT, &styleText_menuReaptSet);
	lv_label_set_text(objLabel_reaptSetT, ">");
	lv_obj_set_protect(objLabel_reaptSetT, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(objLabel_reaptSetT, objBtn_reaptSet, LV_ALIGN_IN_RIGHT_MID, -15, 0)):
		(lv_obj_align(objLabel_reaptSetT, objBtn_reaptSet, LV_ALIGN_IN_RIGHT_MID, 0, 0));

	lv_style_copy(&styleText_menuReaptRem, &lv_style_plain);
	styleText_menuReaptRem.text.font = &lv_font_dejavu_15;
	styleText_menuReaptRem.text.color = LV_COLOR_GRAY;
	objLabel_reaptRem = lv_label_create(objBtn_reaptSet, NULL);
	lv_obj_set_style(objLabel_reaptRem, &styleText_menuReaptRem);
	lv_label_set_long_mode(objLabel_reaptRem, LV_LABEL_LONG_DOT);
	lv_obj_set_size(objLabel_reaptRem, 130, 20);
	memset(strTemp, 0, 31);
	if(!timerSetInfoTemp_currentUnit.tmUp_weekBitHold){
	
		sprintf(strTemp, "no repeat");
	}
	else
	{
		for(uint8_t loopIa = 0; loopIa < 7; loopIa ++){
		
			if((timerSetInfoTemp_currentUnit.tmUp_weekBitHold >> loopIa) & 0x01)
				strlcat(strTemp, strInfoWeek_tab[loopIa], 31);
		}
	}
	lv_label_set_text(objLabel_reaptRem, strTemp);
	lv_obj_set_protect(objLabel_reaptRem, LV_PROTECT_POS);
	lv_obj_align(objLabel_reaptRem, objBtn_reaptSet, LV_ALIGN_IN_BOTTOM_LEFT, 40 + screenLandscapeCoordinate_objOffset, 0);

	lv_style_copy(&styleRoller_timeSet_bg, &lv_style_plain);
	styleRoller_timeSet_bg.body.main_color = LV_COLOR_WHITE;
	styleRoller_timeSet_bg.body.grad_color = LV_COLOR_WHITE;
	styleRoller_timeSet_bg.text.font = &lv_font_consola_19;
	styleRoller_timeSet_bg.text.line_space = 5;
	styleRoller_timeSet_bg.text.opa = LV_OPA_40;
	lv_style_copy(&styleRoller_timeSet_sel, &lv_style_plain);
	styleRoller_timeSet_sel.body.empty = 1;
	styleRoller_timeSet_sel.body.radius = 30;
	styleRoller_timeSet_sel.text.color = LV_COLOR_BLACK;
	styleRoller_timeSet_sel.text.font = &lv_font_consola_19;

	objRoller_timeSet_hour = lv_roller_create(bGround_obj, NULL);
	lv_roller_set_action(objRoller_timeSet_hour, funCb_rollerOpreat_timerUnitSetHour);
	lv_roller_set_options(objRoller_timeSet_hour, "00\n""01\n""02\n""03\n""04\n""05\n"
												  "06\n""07\n""08\n""09\n""10\n""11\n"
												  "12\n""13\n""14\n""15\n""16\n""17\n"
												  "18\n""19\n""20\n""21\n""22\n""23");
	lv_obj_set_protect(objRoller_timeSet_hour, LV_PROTECT_POS);
	lv_roller_set_hor_fit(objRoller_timeSet_hour, false);
	lv_obj_set_width(objRoller_timeSet_hour, 40);
	lv_obj_align(objRoller_timeSet_hour, objLine_timeSet_limit_C, LV_ALIGN_OUT_BOTTOM_LEFT, 40 + screenLandscapeCoordinate_objOffset, 35);
	lv_page_glue_obj(objRoller_timeSet_hour, true);
	lv_roller_set_selected(objRoller_timeSet_hour, timerSetInfoTemp_currentUnit.tmUp_Hour, false);

	lv_roller_set_style(objRoller_timeSet_hour, LV_ROLLER_STYLE_BG, &styleRoller_timeSet_bg);
	lv_roller_set_style(objRoller_timeSet_hour, LV_ROLLER_STYLE_SEL, &styleRoller_timeSet_sel);
	lv_roller_set_visible_row_count(objRoller_timeSet_hour, 4);

	objRoller_timeSet_minute = lv_roller_create(bGround_obj, NULL);
	lv_roller_set_action(objRoller_timeSet_minute, funCb_rollerOpreat_timerUnitSetMinute);
	lv_roller_set_options(objRoller_timeSet_minute, "00\n""01\n""02\n""03\n""04\n""05\n"
												    "06\n""07\n""08\n""09\n""10\n""11\n"
												    "12\n""13\n""14\n""15\n""16\n""17\n"
												    "18\n""19\n""20\n""21\n""22\n""23\n"
												    "24\n""25\n""26\n""27\n""28\n""29\n"
												    "30\n""31\n""32\n""33\n""34\n""35\n"
												    "36\n""37\n""38\n""39\n""40\n""41\n"
												    "42\n""43\n""44\n""45\n""46\n""47\n"
												    "48\n""49\n""50\n""51\n""52\n""53\n"
												    "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(objRoller_timeSet_minute, LV_PROTECT_POS);
	lv_roller_set_hor_fit(objRoller_timeSet_minute, false);
	lv_obj_set_width(objRoller_timeSet_minute, 40);
	lv_obj_align(objRoller_timeSet_minute, objRoller_timeSet_hour, LV_ALIGN_OUT_RIGHT_TOP, 50, 0);
	lv_roller_set_selected(objRoller_timeSet_minute, timerSetInfoTemp_currentUnit.tmUp_Minute, false);

	lv_roller_set_style(objRoller_timeSet_minute, LV_ROLLER_STYLE_BG, &styleRoller_timeSet_bg);
	lv_roller_set_style(objRoller_timeSet_minute, LV_ROLLER_STYLE_SEL, &styleRoller_timeSet_sel);
	lv_roller_set_visible_row_count(objRoller_timeSet_minute, 4);

	lv_style_copy(&styleText_menuTimeSet, &lv_style_plain);
	styleText_menuTimeSet.text.font = &lv_font_arial_15;
	styleText_menuTimeSet.text.color = LV_COLOR_MAKE(80, 240, 80);

	objLabel_timeSet_hour = lv_label_create(bGround_obj, NULL);
	lv_label_set_text(objLabel_timeSet_hour, "hour");
	lv_obj_set_protect(objLabel_timeSet_hour, LV_PROTECT_POS);
	lv_obj_align(objLabel_timeSet_hour, objRoller_timeSet_hour, LV_ALIGN_CENTER, 35, 2);
	lv_obj_set_style(objLabel_timeSet_hour, &styleText_menuTimeSet);

	objLabel_timeSet_minute = lv_label_create(bGround_obj, objLabel_timeSet_hour);
	lv_label_set_text(objLabel_timeSet_minute, "minute");
	lv_obj_set_protect(objLabel_timeSet_minute, LV_PROTECT_POS);
	lv_obj_align(objLabel_timeSet_minute, objRoller_timeSet_minute, LV_ALIGN_CENTER, 35, 0);
	lv_obj_set_style(objLabel_timeSet_minute, &styleText_menuTimeSet);

	lv_page_focus(bGround_obj, objLabel_timeSet_minute, 500);

	vTaskDelay(20 / portTICK_PERIOD_MS);
	lv_obj_refresh_style(bGround_obj);
	lv_obj_refresh_style(obj_Parent);

	usrApp_fullScreenRefresh_self(20, 0);
}

void lvGui_businessMenu_timerSet(lv_obj_t * obj_Parent){

	guiObj_parentTemp = obj_Parent;
	lvGui_businessMenu_timerSetPageA(obj_Parent);	
}




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

#include "gui_businessMenu_linkageConfig.h"
#include "gui_businessHome.h"

#include "gui_businessReuse_reactionObjPage.h"

#define PAGEREACTION_REUSE_LINKAGETEMPRATURE_THRESHOLDADJ_FNUM		(PAGEREACTION_REUSE_BUSSINESS_RESERVE_MAX + 1)

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);
LV_FONT_DECLARE(lv_font_ariblk_18);

LV_FONT_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static lv_style_t stylePage_funSetOption;
static lv_style_t stylePage_linkageDetailSetting;
static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B;
static lv_style_t styleText_menuLevel_C;
static lv_style_t styleCb_proximityDetection_screenLight;
static lv_style_t styleBtn_specialTransparent;
static lv_style_t styleTextBtn_linkageDetailSetting;
static lv_style_t styleTextTitle_linkageDetailSetting;
static lv_style_t styleRoller_bg_tempThresholdAdj;
static lv_style_t styleRoller_sel_tempThresholdAdj;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *page_funSetOption = NULL;
static lv_obj_t *bGround_obj = NULL;
static lv_obj_t *text_Title = NULL;
static lv_obj_t *textSettingA_proximityDetection = NULL;
static lv_obj_t	*textSettingA_tempratureDetection = NULL;
static lv_obj_t	*swReserveSet_proximityDetection = NULL;
static lv_obj_t	*swReserveSet_tempratureDetection = NULL;

static lv_obj_t	*cbProximityDetection_screenLight = NULL;
static lv_obj_t	*btnProximityDetection_switchTrig = NULL;
static lv_obj_t	*textBtnProximityDetection_switchTrig = NULL;
static lv_obj_t	*pageBtnProximityDetection_switchTrig = NULL;

static lv_obj_t	*btnTempratureDetection_tempThresholdAdj = NULL;
static lv_obj_t	*textBtnTempratureDetection_tempThresholdAdj = NULL;
static lv_obj_t	*btnTempratureDetection_switchTrig = NULL;
static lv_obj_t	*textBtnTempratureDetection_switchTrig = NULL;

static lv_obj_t	*pageLinkage_detailSetting = NULL;
static lv_obj_t	*labelTitle_pageLinkageDetailSetting = NULL;
static lv_obj_t	*btnConfirm_pageLinkageDetailSetting = NULL;
static lv_obj_t	*textBtnConfirm_pageLinkageDetailSetting = NULL;
static lv_obj_t	*btnCancel_pageLinkageDetailSetting = NULL;
static lv_obj_t	*textBtnCancel_pageLinkageDetailSetting = NULL;

static lv_obj_t	*rollerThresholdTempSet_tempratureDetection = NULL;
static lv_obj_t	*textRollerThresholdTempSet_tempratureDetection = NULL;

static char strTemp_textBtnTempratureDetection_tempThresholdAdj[60] = {0};

static uint8_t cfgDataTemp_linkageCondition_temprature = 0;
static bool cfgDataScrLight_linkageReaction_proximity = false;

static void lvGuiLinkageConfig_childOptionObjCreat_proximityDetection(void);
static void lvGuiLinkageConfig_childOptionObjDelete_proximityDetection(void);
static void lvGuiLinkageConfig_childOptionObjCreat_tempratureDetection(void);
static void lvGuiLinkageConfig_childOptionObjDelete_tempratureDetection(void);

static void currentGui_elementClear(void){

	lv_obj_del(page_funSetOption);
}

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};
	
	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	linkageConfigParamSet_temp.linkageReaction_proxmity_scrLightTrigIf = 
		cfgDataScrLight_linkageReaction_proximity;
	devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);

	currentGui_elementClear();
	lvGui_usrSwitch(bussinessType_Menu);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static lv_res_t funCb_swOpreat_proximityDetection_reserveEn(lv_obj_t *sw){

	bool opVal_get = lv_sw_get_state(sw);
	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};

	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	if(opVal_get){ 

		lvGuiLinkageConfig_childOptionObjCreat_proximityDetection();
	}
	else
	{
		lvGuiLinkageConfig_childOptionObjDelete_proximityDetection();
	}

	(opVal_get)?
		(linkageConfigParamSet_temp.linkageRunning_proxmity_en = 1):
		(linkageConfigParamSet_temp.linkageRunning_proxmity_en = 0);
	devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);
	
	return LV_RES_OK;
}

static lv_res_t funCb_swOpreat_tempratureDetection_reserveEn(lv_obj_t *sw){
	
	bool opVal_get = lv_sw_get_state(sw);
	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};

	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	if(opVal_get){

		lvGuiLinkageConfig_childOptionObjCreat_tempratureDetection();	
	}
	else
	{
		lvGuiLinkageConfig_childOptionObjDelete_tempratureDetection();
	}

	(opVal_get)?
		(linkageConfigParamSet_temp.linkageRunning_temprature_en = 1):
		(linkageConfigParamSet_temp.linkageRunning_temprature_en = 0);
	devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClickDetailSet_pageChoiceSelect_confirm(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnmFreeNum = lv_obj_get_free_num(btn);

	switch(btnmFreeNum){
	 
//		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{}break;
//		
//		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
		
		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{

			stt_devDataPonitTypedef dataPointReaction_valTemp = {0};
			stt_paramLinkageConfig linkageConfigParamSet_temp = {0};
			
			devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);
			lvGui_businessReuse_reactionObjPageElement_funValConfig_get(PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP, 
																		&dataPointReaction_valTemp);
			
			memcpy(&linkageConfigParamSet_temp.linkageReaction_temprature_swVal,
				   &dataPointReaction_valTemp,
				   sizeof(stt_devDataPonitTypedef));
			devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);		

		}break;
		
		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{

			stt_devDataPonitTypedef dataPointReaction_valTemp = {0};
			stt_paramLinkageConfig linkageConfigParamSet_temp = {0};

			devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);
			lvGui_businessReuse_reactionObjPageElement_funValConfig_get(PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX, 
																		&dataPointReaction_valTemp);

			memcpy(&linkageConfigParamSet_temp.linkageReaction_proxmity_swVal,
				   &dataPointReaction_valTemp,
				   sizeof(stt_devDataPonitTypedef));
			devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);		

		}break;

		case PAGEREACTION_REUSE_LINKAGETEMPRATURE_THRESHOLDADJ_FNUM:{

			stt_paramLinkageConfig linkageConfigParamSet_temp = {0};
			
			devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

			linkageConfigParamSet_temp.linkageCondition_tempratureVal = cfgDataTemp_linkageCondition_temprature;
			devSystemOpration_linkageConfig_paramSet(&linkageConfigParamSet_temp, true);

			sprintf(strTemp_textBtnTempratureDetection_tempThresholdAdj, " #00A2E8 > temprature set##C0C0FF [%d\"C]#", cfgDataTemp_linkageCondition_temprature); //温度设定值显示更新
			lv_label_set_text(textBtnTempratureDetection_tempThresholdAdj, strTemp_textBtnTempratureDetection_tempThresholdAdj);	

		}break;

		default:break;
	}

	if(pageLinkage_detailSetting){

		lv_obj_del(pageLinkage_detailSetting);
		pageLinkage_detailSetting = NULL;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClickDetailSet_pageChoiceSelect_cancel(lv_obj_t *btn){

	if(pageLinkage_detailSetting){

		lv_obj_del(pageLinkage_detailSetting);
		pageLinkage_detailSetting = NULL;
	}

	return LV_RES_OK;
}

static lv_obj_t *lvGui_pageFunctionDetailSet_creat(const char * pageTitleName, uint8_t pageObjIst){

	if(pageLinkage_detailSetting == NULL)
		pageLinkage_detailSetting = lv_page_create(lv_scr_act(), NULL);

	lv_obj_set_size(pageLinkage_detailSetting, 200, 240);
	lv_obj_set_pos(pageLinkage_detailSetting, 20, 40);					
	lv_page_set_style(pageLinkage_detailSetting, LV_PAGE_STYLE_SB, &stylePage_linkageDetailSetting);
	lv_page_set_style(pageLinkage_detailSetting, LV_PAGE_STYLE_BG, &stylePage_linkageDetailSetting);
	lv_page_set_sb_mode(pageLinkage_detailSetting, LV_SB_MODE_HIDE);	
	lv_page_set_scrl_fit(pageLinkage_detailSetting, false, true); //key opration
	lv_page_set_scrl_layout(pageLinkage_detailSetting, LV_LAYOUT_PRETTY);

	labelTitle_pageLinkageDetailSetting = lv_label_create(pageLinkage_detailSetting, NULL);
	lv_obj_set_style(labelTitle_pageLinkageDetailSetting, &styleTextTitle_linkageDetailSetting);
	lv_label_set_recolor(labelTitle_pageLinkageDetailSetting, true);
	lv_label_set_text(labelTitle_pageLinkageDetailSetting, pageTitleName);
	lv_obj_set_protect(labelTitle_pageLinkageDetailSetting, LV_PROTECT_POS);
	lv_obj_align(labelTitle_pageLinkageDetailSetting, pageLinkage_detailSetting, LV_ALIGN_IN_TOP_MID, 0, -5);

	btnConfirm_pageLinkageDetailSetting = lv_btn_create(pageLinkage_detailSetting, NULL);
	lv_btn_set_style(btnConfirm_pageLinkageDetailSetting, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnConfirm_pageLinkageDetailSetting, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
	lv_btn_set_style(btnConfirm_pageLinkageDetailSetting, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnConfirm_pageLinkageDetailSetting, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	lv_btn_set_fit(btnConfirm_pageLinkageDetailSetting, false, false);
	lv_obj_set_size(btnConfirm_pageLinkageDetailSetting, 70, 25);
	lv_page_glue_obj(btnConfirm_pageLinkageDetailSetting, true);
	lv_obj_set_protect(btnConfirm_pageLinkageDetailSetting, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(btnConfirm_pageLinkageDetailSetting, LV_PROTECT_POS);
	lv_obj_align(btnConfirm_pageLinkageDetailSetting, pageLinkage_detailSetting, LV_ALIGN_IN_BOTTOM_LEFT, 25, -1);
	lv_btn_set_action(btnConfirm_pageLinkageDetailSetting, LV_BTN_ACTION_CLICK, funCb_btnActionClickDetailSet_pageChoiceSelect_confirm);
	lv_obj_set_free_num(btnConfirm_pageLinkageDetailSetting, pageObjIst);
	btnCancel_pageLinkageDetailSetting = lv_btn_create(pageLinkage_detailSetting, btnConfirm_pageLinkageDetailSetting);
	lv_obj_align(btnCancel_pageLinkageDetailSetting, btnConfirm_pageLinkageDetailSetting, LV_ALIGN_CENTER, 85, 0);
	lv_btn_set_action(btnCancel_pageLinkageDetailSetting, LV_BTN_ACTION_CLICK, funCb_btnActionClickDetailSet_pageChoiceSelect_cancel);
	lv_obj_set_free_num(btnCancel_pageLinkageDetailSetting, pageObjIst);

	textBtnConfirm_pageLinkageDetailSetting = lv_label_create(btnConfirm_pageLinkageDetailSetting, NULL);
	lv_obj_set_style(textBtnConfirm_pageLinkageDetailSetting, &styleTextBtn_linkageDetailSetting);
	lv_label_set_recolor(textBtnConfirm_pageLinkageDetailSetting, true);
	lv_label_set_text(textBtnConfirm_pageLinkageDetailSetting, "confirm");
	textBtnCancel_pageLinkageDetailSetting = lv_label_create(btnCancel_pageLinkageDetailSetting, textBtnConfirm_pageLinkageDetailSetting);
	lv_label_set_text(textBtnCancel_pageLinkageDetailSetting, "cancel");

	return pageLinkage_detailSetting;
}

static lv_res_t funCb_cbActionCheckDetailSet_trigScrLight_proximity(lv_obj_t *cb){

	bool opVal_get = lv_cb_is_checked(cb);

	cfgDataScrLight_linkageReaction_proximity = opVal_get;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClickDetailSet_switchTrigSet_proximity(lv_obj_t *btn){

	stt_paramLinkageConfig devLinkageCfg_dataTemp = {0};
	lv_obj_t *pageDetailSet = lvGui_pageFunctionDetailSet_creat("switch set", PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX);

	devSystemOpration_linkageConfig_paramGet(&devLinkageCfg_dataTemp);

	lvGui_businessReuse_reactionObjPageElement_creat(pageDetailSet, PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX, 80, &(devLinkageCfg_dataTemp.linkageReaction_proxmity_swVal));

	return LV_RES_OK;
}

static lv_res_t funCb_rollerActionTrigDetailSet_conditionValSet_temprature(lv_obj_t *roller){

	uint8_t opVal_get = lv_roller_get_selected(roller);

	cfgDataTemp_linkageCondition_temprature = opVal_get;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClickDetailSet_valThresholdAdj_temprature(lv_obj_t *btn){

	lv_obj_t *pageSetting = lvGui_pageFunctionDetailSet_creat("temprature set", PAGEREACTION_REUSE_LINKAGETEMPRATURE_THRESHOLDADJ_FNUM);

	rollerThresholdTempSet_tempratureDetection = lv_roller_create(pageSetting, NULL);
	lv_roller_set_action(rollerThresholdTempSet_tempratureDetection, funCb_rollerActionTrigDetailSet_conditionValSet_temprature);
	lv_roller_set_options(rollerThresholdTempSet_tempratureDetection, "00\n""01\n""02\n""03\n""04\n""05\n"
																	  "06\n""07\n""08\n""09\n""10\n""11\n"
																	  "12\n""13\n""14\n""15\n""16\n""17\n"
																	  "18\n""19\n""20\n""21\n""22\n""23\n"
																	  "24\n""25\n""26\n""27\n""28\n""29\n"
																	  "30\n""31\n""32\n""33\n""34\n""35\n"
																	  "36\n""37\n""38\n""39\n""40");
	lv_obj_set_protect(rollerThresholdTempSet_tempratureDetection, LV_PROTECT_POS);
	lv_roller_set_hor_fit(rollerThresholdTempSet_tempratureDetection, false);
	lv_obj_set_width(rollerThresholdTempSet_tempratureDetection, 40);
	lv_obj_align(rollerThresholdTempSet_tempratureDetection, pageSetting, LV_ALIGN_CENTER, 0, -25);
	lv_page_glue_obj(rollerThresholdTempSet_tempratureDetection, false);
	lv_roller_set_selected(rollerThresholdTempSet_tempratureDetection, 5, false);
	lv_roller_set_visible_row_count(rollerThresholdTempSet_tempratureDetection, 4);
	lv_roller_set_style(rollerThresholdTempSet_tempratureDetection, LV_ROLLER_STYLE_BG, &styleRoller_bg_tempThresholdAdj);
	lv_roller_set_style(rollerThresholdTempSet_tempratureDetection, LV_ROLLER_STYLE_SEL, &styleRoller_sel_tempThresholdAdj);
	lv_roller_set_selected(rollerThresholdTempSet_tempratureDetection, 
						   cfgDataTemp_linkageCondition_temprature, 
						   true);

	textRollerThresholdTempSet_tempratureDetection = lv_label_create(pageSetting, NULL);
	lv_label_set_recolor(textRollerThresholdTempSet_tempratureDetection, true);
	lv_label_set_text(textRollerThresholdTempSet_tempratureDetection, "#8000FF \"C#");
	lv_obj_set_protect(textRollerThresholdTempSet_tempratureDetection, LV_PROTECT_POS);
	lv_obj_align(textRollerThresholdTempSet_tempratureDetection, rollerThresholdTempSet_tempratureDetection, LV_ALIGN_CENTER, 33, 0);
	lv_obj_set_style(textRollerThresholdTempSet_tempratureDetection, &styleTextTitle_linkageDetailSetting);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClickDetailSet_switchTrigSet_temprature(lv_obj_t *btn){

	stt_paramLinkageConfig devLinkageCfg_dataTemp = {0};
	lv_obj_t *pageDetailSet = lvGui_pageFunctionDetailSet_creat("switch set", PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP);

	devSystemOpration_linkageConfig_paramGet(&devLinkageCfg_dataTemp);

	lvGui_businessReuse_reactionObjPageElement_creat(pageDetailSet, PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP, 80, &(devLinkageCfg_dataTemp.linkageReaction_temprature_swVal));

	return LV_RES_OK;
}


static void lvGuiLinkageConfig_objStyle_Init(void){

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&stylePage_funSetOption, &lv_style_plain);
	stylePage_funSetOption.body.main_color = LV_COLOR_GRAY;
	stylePage_funSetOption.body.grad_color = LV_COLOR_GRAY;

	lv_style_copy(&styleTextBtn_linkageDetailSetting, &lv_style_plain);
	styleTextBtn_linkageDetailSetting.text.font = &lv_font_ariblk_18;
	styleTextBtn_linkageDetailSetting.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleTextTitle_linkageDetailSetting, &lv_style_plain);
	styleTextTitle_linkageDetailSetting.text.font = &lv_font_consola_19;
	styleTextTitle_linkageDetailSetting.text.color = LV_COLOR_BLACK;

	lv_style_copy(&stylePage_linkageDetailSetting, &lv_style_plain_color);
	stylePage_linkageDetailSetting.body.main_color = LV_COLOR_SILVER;
	stylePage_linkageDetailSetting.body.grad_color = LV_COLOR_SILVER;
	stylePage_linkageDetailSetting.body.border.part = LV_BORDER_NONE;
	stylePage_linkageDetailSetting.body.radius = 6;
	stylePage_linkageDetailSetting.body.opa = LV_OPA_90;
	stylePage_linkageDetailSetting.body.padding.hor = 0;
	stylePage_linkageDetailSetting.body.padding.inner = 0;	

	lv_style_copy(&styleText_menuLevel_B, &lv_style_plain);
	styleText_menuLevel_B.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleText_menuLevel_C, &lv_style_plain);
	styleText_menuLevel_C.text.font = &lv_font_consola_16;
	styleText_menuLevel_C.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleCb_proximityDetection_screenLight, &lv_style_plain);
	styleCb_proximityDetection_screenLight.body.main_color = LV_COLOR_GRAY;
	styleCb_proximityDetection_screenLight.body.grad_color = LV_COLOR_GRAY;
	styleCb_proximityDetection_screenLight.body.border.part = LV_BORDER_NONE;
	styleCb_proximityDetection_screenLight.body.radius = 6;
	styleCb_proximityDetection_screenLight.body.opa = LV_OPA_100;
	styleCb_proximityDetection_screenLight.body.padding.hor = 0;
	styleCb_proximityDetection_screenLight.body.padding.inner = 0;	
	styleCb_proximityDetection_screenLight.text.color = LV_COLOR_MAKE(0xB5, 0xE6, 0x1D);
	styleCb_proximityDetection_screenLight.text.font = &lv_font_consola_16;

    lv_style_copy(&styleBtn_specialTransparent, &lv_style_btn_rel);
	styleBtn_specialTransparent.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.border.part = LV_BORDER_NONE;
    styleBtn_specialTransparent.body.opa = LV_OPA_TRANSP;
	styleBtn_specialTransparent.body.radius = 0;
    styleBtn_specialTransparent.body.shadow.width = 0;
		
	lv_style_copy(&styleRoller_bg_tempThresholdAdj, &lv_style_plain);
	styleRoller_bg_tempThresholdAdj.body.main_color = LV_COLOR_SILVER;
	styleRoller_bg_tempThresholdAdj.body.grad_color = LV_COLOR_SILVER;
	styleRoller_bg_tempThresholdAdj.body.opa = LV_OPA_100;
	styleRoller_bg_tempThresholdAdj.text.font = &lv_font_consola_19;
	styleRoller_bg_tempThresholdAdj.text.line_space = 5;
	styleRoller_bg_tempThresholdAdj.text.opa = LV_OPA_40;
	lv_style_copy(&styleRoller_sel_tempThresholdAdj, &lv_style_plain);
	styleRoller_sel_tempThresholdAdj.body.empty = 1;
	styleRoller_sel_tempThresholdAdj.body.radius = 30;
	styleRoller_sel_tempThresholdAdj.text.color = LV_COLOR_MAKE(128, 0, 255);
	styleRoller_sel_tempThresholdAdj.text.font = &lv_font_consola_19;
}

static void lvGuiLinkageConfig_animationChildOptionActivity(lv_obj_t *optionObj, int32_t coordX_a, int32_t coordX_b){

	lv_anim_t a;	

	a.var = optionObj;												
	a.start = coordX_a;														
	a.end = coordX_b;								 
	a.fp = (lv_anim_fp_t)lv_obj_set_y;	   
	a.path = lv_anim_path_linear;					
	a.end_cb = NULL;							  
	a.act_time = 0; 								
	a.time = 300;								  
	a.playback = 0; 								
	a.playback_pause = 0;								  
	a.repeat = 0;								  
	a.repeat_pause = 0; 

	lv_anim_create(&a);
}

static void lvGuiLinkageConfig_childOptionObjCreat_proximityDetection(void){

	lv_anim_t a,b,c,d;	
	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};
	
	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	cfgDataScrLight_linkageReaction_proximity = linkageConfigParamSet_temp.linkageReaction_proxmity_scrLightTrigIf;

	lvGuiLinkageConfig_animationChildOptionActivity(textSettingA_tempratureDetection, 
													lv_obj_get_y(textSettingA_proximityDetection) + 30,
													lv_obj_get_y(textSettingA_proximityDetection) + 100);

	lvGuiLinkageConfig_animationChildOptionActivity(swReserveSet_tempratureDetection,
													lv_obj_get_y(swReserveSet_proximityDetection) + 30,
													lv_obj_get_y(swReserveSet_proximityDetection) + 100);

	if(btnTempratureDetection_tempThresholdAdj){

		lvGuiLinkageConfig_animationChildOptionActivity(btnTempratureDetection_tempThresholdAdj,
														lv_obj_get_y(swReserveSet_proximityDetection) + 30 + 15 + 15,
														lv_obj_get_y(swReserveSet_proximityDetection) + 100 + 15 + 15);

	}

	if(btnTempratureDetection_switchTrig){

		lvGuiLinkageConfig_animationChildOptionActivity(btnTempratureDetection_switchTrig,
														lv_obj_get_y(swReserveSet_proximityDetection) + 30 + 15 + 50,
														lv_obj_get_y(swReserveSet_proximityDetection) + 100 + 15 + 50);
	}

	cbProximityDetection_screenLight = lv_cb_create(page_funSetOption, NULL);
	lv_cb_set_action(cbProximityDetection_screenLight, funCb_cbActionCheckDetailSet_trigScrLight_proximity);
	lv_obj_set_protect(cbProximityDetection_screenLight, LV_PROTECT_POS);
	lv_obj_align(cbProximityDetection_screenLight, textSettingA_proximityDetection, LV_ALIGN_OUT_BOTTOM_LEFT, 17, 15);
	lv_obj_set_width(cbProximityDetection_screenLight, 175);
	lv_cb_set_style(cbProximityDetection_screenLight, LV_CB_STYLE_BG, &styleCb_proximityDetection_screenLight);
	lv_cb_set_text(cbProximityDetection_screenLight, " light the screen");
	lv_cb_set_checked(cbProximityDetection_screenLight, cfgDataScrLight_linkageReaction_proximity);

	btnProximityDetection_switchTrig = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnProximityDetection_switchTrig, 200, 20);
	lv_obj_set_protect(btnProximityDetection_switchTrig, LV_PROTECT_POS);
	lv_btn_set_action(btnProximityDetection_switchTrig, LV_BTN_ACTION_CLICK, funCb_btnActionClickDetailSet_switchTrigSet_proximity);
	lv_obj_align(btnProximityDetection_switchTrig, textSettingA_proximityDetection, LV_ALIGN_OUT_BOTTOM_LEFT, -8, 50);
	lv_btn_set_style(btnProximityDetection_switchTrig, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnProximityDetection_switchTrig, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
	lv_btn_set_style(btnProximityDetection_switchTrig, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnProximityDetection_switchTrig, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnProximityDetection_switchTrig = lv_label_create(btnProximityDetection_switchTrig, NULL);
	lv_label_set_recolor(textBtnProximityDetection_switchTrig, true);
	lv_label_set_text(textBtnProximityDetection_switchTrig, " #B5E61D > trig the switch#");
	lv_obj_set_size(textBtnProximityDetection_switchTrig, 200, 20);
	lv_label_set_align(textBtnProximityDetection_switchTrig, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_style(textBtnProximityDetection_switchTrig, &styleText_menuLevel_C);
	lv_obj_set_protect(textBtnProximityDetection_switchTrig, LV_PROTECT_POS);
	lv_obj_align(textBtnProximityDetection_switchTrig, btnProximityDetection_switchTrig, LV_ALIGN_CENTER, 0, 0);

	lv_obj_animate(cbProximityDetection_screenLight, LV_ANIM_FLOAT_RIGHT, 200,	0, 		NULL);
	lv_obj_animate(btnProximityDetection_switchTrig, LV_ANIM_FLOAT_RIGHT, 200,	100, 	NULL);

	lv_obj_refresh_style(page_funSetOption);
}

static void lvGuiLinkageConfig_childOptionObjDelete_proximityDetection(void){

	lv_anim_t a,b,c,d;	

	lv_obj_del(cbProximityDetection_screenLight);
	cbProximityDetection_screenLight = NULL;
	lv_obj_del(btnProximityDetection_switchTrig);
	btnProximityDetection_switchTrig = NULL;

	lvGuiLinkageConfig_animationChildOptionActivity(textSettingA_tempratureDetection,
													lv_obj_get_y(textSettingA_proximityDetection) + 100,
													lv_obj_get_y(textSettingA_proximityDetection) + 30);

	lvGuiLinkageConfig_animationChildOptionActivity(swReserveSet_tempratureDetection,
													lv_obj_get_y(swReserveSet_proximityDetection) + 100,
													lv_obj_get_y(swReserveSet_proximityDetection) + 30);

	if(btnTempratureDetection_tempThresholdAdj){

		lvGuiLinkageConfig_animationChildOptionActivity(btnTempratureDetection_tempThresholdAdj,
														lv_obj_get_y(swReserveSet_proximityDetection) + 100 + 15 + 15,
														lv_obj_get_y(swReserveSet_proximityDetection) + 30 + 15 + 15);
	}

	if(btnTempratureDetection_switchTrig){

		lvGuiLinkageConfig_animationChildOptionActivity(btnTempratureDetection_switchTrig,
														lv_obj_get_y(swReserveSet_proximityDetection) + 100 + 15 + 50,
														lv_obj_get_y(swReserveSet_proximityDetection) + 30 + 15 + 50);
	}

	lv_obj_refresh_style(page_funSetOption);
}

static void lvGuiLinkageConfig_childOptionObjCreat_tempratureDetection(void){

	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};
	
	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	cfgDataTemp_linkageCondition_temprature = linkageConfigParamSet_temp.linkageCondition_tempratureVal;

	btnTempratureDetection_tempThresholdAdj = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTempratureDetection_tempThresholdAdj, 200, 20);
	lv_obj_set_protect(btnTempratureDetection_tempThresholdAdj, LV_PROTECT_POS);
	lv_btn_set_action(btnTempratureDetection_tempThresholdAdj, LV_BTN_ACTION_CLICK, funCb_btnActionClickDetailSet_valThresholdAdj_temprature);
	lv_obj_align(btnTempratureDetection_tempThresholdAdj, textSettingA_tempratureDetection, LV_ALIGN_OUT_BOTTOM_LEFT, 16, 15);
	lv_btn_set_style(btnTempratureDetection_tempThresholdAdj, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_tempThresholdAdj, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_tempThresholdAdj, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_tempThresholdAdj, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnTempratureDetection_tempThresholdAdj = lv_label_create(btnTempratureDetection_tempThresholdAdj, NULL);
	lv_label_set_recolor(textBtnTempratureDetection_tempThresholdAdj, true);
	sprintf(strTemp_textBtnTempratureDetection_tempThresholdAdj, " #00A2E8 > temprature set##C0C0FF [%d\"C]#", cfgDataTemp_linkageCondition_temprature);
	lv_label_set_text(textBtnTempratureDetection_tempThresholdAdj, strTemp_textBtnTempratureDetection_tempThresholdAdj);
	lv_obj_set_size(textBtnTempratureDetection_tempThresholdAdj, 200, 20);
	lv_label_set_align(textBtnTempratureDetection_tempThresholdAdj, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_style(textBtnTempratureDetection_tempThresholdAdj, &styleText_menuLevel_C);
	lv_obj_set_protect(textBtnTempratureDetection_tempThresholdAdj, LV_PROTECT_POS);
	lv_obj_align(textBtnTempratureDetection_tempThresholdAdj, btnTempratureDetection_tempThresholdAdj, LV_ALIGN_CENTER, 0, 0);

	btnTempratureDetection_switchTrig = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnTempratureDetection_switchTrig, 200, 20);
	lv_obj_set_protect(btnTempratureDetection_switchTrig, LV_PROTECT_POS);
	lv_btn_set_action(btnTempratureDetection_switchTrig, LV_BTN_ACTION_CLICK, funCb_btnActionClickDetailSet_switchTrigSet_temprature);
	lv_obj_align(btnTempratureDetection_switchTrig, textSettingA_tempratureDetection, LV_ALIGN_OUT_BOTTOM_LEFT, -8, 50);
	lv_btn_set_style(btnTempratureDetection_switchTrig, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_switchTrig, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_switchTrig, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
	lv_btn_set_style(btnTempratureDetection_switchTrig, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnTempratureDetection_switchTrig = lv_label_create(btnTempratureDetection_switchTrig, NULL);
	lv_label_set_recolor(textBtnTempratureDetection_switchTrig, true);
	lv_label_set_text(textBtnTempratureDetection_switchTrig, " #B5E61D > trig the switch#");
	lv_obj_set_size(textBtnTempratureDetection_switchTrig, 200, 20);
	lv_label_set_align(textBtnTempratureDetection_switchTrig, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_style(textBtnTempratureDetection_switchTrig, &styleText_menuLevel_C);
	lv_obj_set_protect(textBtnTempratureDetection_switchTrig, LV_PROTECT_POS);
	lv_obj_align(textBtnTempratureDetection_switchTrig, btnTempratureDetection_switchTrig, LV_ALIGN_CENTER, 0, 0);

	lv_obj_animate(btnTempratureDetection_tempThresholdAdj, LV_ANIM_FLOAT_RIGHT, 200,	0, 		NULL);
	lv_obj_animate(btnTempratureDetection_switchTrig, 		LV_ANIM_FLOAT_RIGHT, 200,	100, 	NULL);
}

static void lvGuiLinkageConfig_childOptionObjDelete_tempratureDetection(void){

	lv_obj_del(btnTempratureDetection_tempThresholdAdj);
	btnTempratureDetection_tempThresholdAdj = NULL;
	lv_obj_del(btnTempratureDetection_switchTrig);
	btnTempratureDetection_switchTrig = NULL;
}

void lvGui_businessMenu_linkageConfig(lv_obj_t * obj_Parent){

	stt_paramLinkageConfig linkageConfigParamSet_temp = {0};

	devSystemOpration_linkageConfig_paramGet(&linkageConfigParamSet_temp);

	lvGuiLinkageConfig_objStyle_Init();

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "linkage config");
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
	lv_page_set_scrl_width(page_funSetOption, 230);
	lv_page_set_scrl_height(page_funSetOption, 640);
	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);

	textSettingA_proximityDetection = lv_label_create(page_funSetOption, NULL);
	lv_label_set_recolor(textSettingA_proximityDetection, true);
	lv_label_set_text(textSettingA_proximityDetection, "#FFFFFF proximity detection#:");
	lv_obj_set_style(textSettingA_proximityDetection, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_proximityDetection, LV_PROTECT_POS);
	lv_obj_align(textSettingA_proximityDetection, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 20);

	swReserveSet_proximityDetection = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_action(swReserveSet_proximityDetection, funCb_swOpreat_proximityDetection_reserveEn);
	lv_obj_set_size(swReserveSet_proximityDetection, 45, 15);
	lv_obj_set_protect(swReserveSet_proximityDetection, LV_PROTECT_POS);
	lv_obj_align(swReserveSet_proximityDetection, textSettingA_proximityDetection, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

	textSettingA_tempratureDetection = lv_label_create(page_funSetOption, NULL);
	lv_label_set_recolor(textSettingA_tempratureDetection, true);
	lv_label_set_text(textSettingA_tempratureDetection, "#FFFFFF temprature detection#:");
	lv_obj_set_style(textSettingA_tempratureDetection, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_tempratureDetection, LV_PROTECT_POS);
	lv_obj_align(textSettingA_tempratureDetection, textSettingA_proximityDetection, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

	swReserveSet_tempratureDetection = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_action(swReserveSet_tempratureDetection, funCb_swOpreat_tempratureDetection_reserveEn);
	lv_obj_set_size(swReserveSet_tempratureDetection, 45, 15);
	lv_obj_set_protect(swReserveSet_tempratureDetection, LV_PROTECT_POS);
	lv_obj_align(swReserveSet_tempratureDetection, swReserveSet_proximityDetection, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

	if(linkageConfigParamSet_temp.linkageRunning_temprature_en){

		lv_sw_on(swReserveSet_tempratureDetection);
		lvGuiLinkageConfig_childOptionObjCreat_tempratureDetection();
	}

	if(linkageConfigParamSet_temp.linkageRunning_proxmity_en){

		lv_sw_on(swReserveSet_proximityDetection);
		lvGuiLinkageConfig_childOptionObjCreat_proximityDetection();
	}

	lv_obj_animate(textSettingA_proximityDetection,  LV_ANIM_FLOAT_RIGHT, 200,	  0,	NULL);
	lv_obj_animate(swReserveSet_proximityDetection,  LV_ANIM_FLOAT_RIGHT, 200,	100,	NULL);
	lv_obj_animate(textSettingA_tempratureDetection, LV_ANIM_FLOAT_RIGHT, 200,	200,	NULL);
	lv_obj_animate(swReserveSet_tempratureDetection, LV_ANIM_FLOAT_RIGHT, 200,	300,	NULL);
}



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

#include "dataTrans_meshUpgrade.h"

#include "gui_businessMenu_settingSet.h"

#include "gui_businessHome.h"
#include "gui_businessReuse_reactionObjPage.h"
	
#define OBJ_DDLIST_DEVTYPE_FREENUM			1
#define OBJ_DDLIST_HPTHEME_FREENUM			2

#define FUNCTION_NUM_DEF_SCREENLIGHT_TIME	13

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);

LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);
LV_IMG_DECLARE(bGroundPrev_picFigure_sel);

static const char *deviceType_listTab = {

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE) 

	"Switch-1bit\n"
	"Switch-2bit\n"
	"Switch-3bit\n"
	"Dimmer\n"
	"Fans\n"
	"Scenario\n"
	"Curtain\n" 
	"Heater\n"
	"Thermostat"
		
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	"Switch-1bit\n"
	"Switch-2bit\n"
	"Switch-3bit\n"
	"Curtain\n" 
	"Scenario\n"
	"Thermostat\n"
	"ThermostatEx"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)
	
	"Fans"
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
	
	"Dimmer"
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)

	"Heater"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	"infrared"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	"socket"

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	"moudle 3 bit"
	
#endif
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

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE) 
	
	{devTypeDef_mulitSwOneBit,			0},
	{devTypeDef_mulitSwTwoBit,			1},
	{devTypeDef_mulitSwThreeBit,		2},
	{devTypeDef_dimmer, 				3},
	{devTypeDef_fans,					4},
	{devTypeDef_scenario,				5},
	{devTypeDef_curtain,				6},
	{devTypeDef_heater, 				7}, 
	{devTypeDef_thermostat, 			8}, 

 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

	{devTypeDef_mulitSwOneBit, 	 		0},
	{devTypeDef_mulitSwTwoBit, 	 		1},
	{devTypeDef_mulitSwThreeBit,		2},
	{devTypeDef_curtain,				3}, 
	{devTypeDef_scenario,				4},
	{devTypeDef_thermostat,		 		5}, 
	{devTypeDef_thermostatExtension,	6}, 
	
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)
		
	{devTypeDef_fans,					0},
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
		
	{devTypeDef_dimmer, 				0},
		
 #elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)
	
	{devTypeDef_heater, 				0}, 
		
 #endif
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
	{60 * 30 + 1 * 00, "#FFFF00 30##C0C0FF min##FFFF00 00##C0C0FF sec#"},
	{COUNTER_DISENABLE_MASK_SPECIALVAL_U16, 	    "#FFFF00   ALWAYS#"}
};

static const uint8_t screenBrightness_sliderAdj_div = 20;

static uint8_t devTypeDdist_istRecord = 0;

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
static lv_style_t styleImg_menuFun_btnFun;
static lv_style_t stylePageCb_elecParamDispSet;
static lv_style_t stylePageCb_screenLandscape;

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static lv_obj_t *page_funSetOption = NULL;
static lv_obj_t *text_Title = NULL;
static lv_obj_t *textSettingA_deviceType = NULL;
static lv_obj_t	*textSettingA_devStatusRecordIF = NULL;
static lv_obj_t	*textSettingA_bGroundPicSelect = NULL;
static lv_obj_t *textSettingA_homepageThemestyle = NULL;
static lv_obj_t *textSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *textSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textSettingA_screenBrightness = NULL;
static lv_obj_t *textSettingA_screenLightTime = NULL;
static lv_obj_t *textSettingA_elecParamDispSelect = NULL;
static lv_obj_t *textSettingA_elecsumResetClear = NULL;
static lv_obj_t *textSettingA_screenLandscape = NULL;
static lv_obj_t *btnSettingA_factoryRecoveryIf = NULL;
static lv_obj_t *textBtnRef_factoryRecoveryIf = NULL;
static lv_obj_t *btnSettingA_touchRecalibrationIf = NULL;
static lv_obj_t *textBtnRef_touchRecalibrationIf = NULL;
static lv_obj_t *btnSettingA_elecsumReset = NULL;
static lv_obj_t *textBtnRef_elecsumReset = NULL;
static lv_obj_t	*ddlistSettingA_deviceType = NULL;
static lv_obj_t	*ddlistSettingA_homepageThemestyle = NULL;
static lv_obj_t	*swSettingA_devStatusRecordIF = NULL;
static lv_obj_t	*pageBgroundPic_select = NULL;
static lv_obj_t	*imgBgroundPicSel_reserve = NULL;
static lv_obj_t	*btnBgroundPicSel_previewObjBase[BROUND_PIC_SELECT_MAX_NUM] = {NULL};
static lv_obj_t	*btnBgroundPicSel_preview[BROUND_PIC_SELECT_MAX_NUM] = {NULL};
static lv_obj_t *mboxFactoryRecovery_comfirm = NULL;
static lv_obj_t *mboxTouchRecalibration_comfirm = NULL;
static lv_obj_t *mboxTouchElecsumReset_comfirm = NULL;
static lv_obj_t *mboxTouchFwareUpgrade_comfirm = NULL;
static lv_obj_t *mboxScreenLandscape_comfirm = NULL;
static lv_obj_t *sliderSettingA_screenBrightness = NULL;
static lv_obj_t *sliderSettingA_screenBrightnessSleep = NULL;
static lv_obj_t *textBtnTimeRef_screenLightTime = NULL;
static lv_obj_t *btnTimeAdd_screenLightTime = NULL;
static lv_obj_t *textBtnTimeAdd_screenLightTime = NULL;
static lv_obj_t *btnTimeCut_screenLightTime = NULL;
static lv_obj_t *textBtnTimeCut_screenLightTime = NULL;
static lv_obj_t *cbDispSelect_elecParam_power = NULL;
static lv_obj_t *cbDispSelect_elecParam_elecsum = NULL;
static lv_obj_t *cbDispSelect_screenVer = NULL;
static lv_obj_t *cbDispSelect_screenHor = NULL;
static lv_obj_t *textSettingA_fwareUpgrade = NULL;
static lv_obj_t *btnSettingA_fwareUpgrade = NULL;
static lv_obj_t *textBtnRef_fwareUpgrade = NULL;

static bool upgradeAvailable_flg = false;

static uint8_t  functionGearScreenTime_ref = 0;
static bool 	opreatTemp_screenLandscapeIf = false;

static uint8_t screenLandscapeCoordinate_objOffset = 0;

static void currentGui_elementClear(void){

	lv_obj_del(page_funSetOption);
}

void guiDispTimeOut_pageSettingSet(void){

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

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK:
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

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK	:
		default:

			objImg_colorChg = imgMenuBtnChoIcon_fun_back;

		break;
	}

	lv_img_set_style(objImg_colorChg, &styleImg_menuFun_btnFun);
	lv_obj_refresh_style(objImg_colorChg);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_bGroundPicOrgSelcet(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE btnFreeNum = lv_obj_get_free_num(btn);
	uint8_t loop = 0;

	printf("bGroundOrg sel:%d.\n", btnFreeNum);

	lv_obj_del(imgBgroundPicSel_reserve);

	for(loop = 0; loop < BROUND_PIC_SELECT_MAX_NUM; loop ++){

		(btnFreeNum == loop)?
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL)):
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR));
	}

	imgBgroundPicSel_reserve = lv_img_create(btnBgroundPicSel_preview[btnFreeNum], NULL);
	lv_img_set_src(imgBgroundPicSel_reserve, &bGroundPrev_picFigure_sel);

	usrAppHomepageBgroundPicOrg_Set(btnFreeNum, true, false);

	return LV_RES_OK;
}

void appUiElementSet_upgradeAvailable(bool val){

	upgradeAvailable_flg = val;
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

static lv_res_t funCb_mboxBtnActionClick_elecsumReset(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("elecsumReset mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					devDriverBussiness_elecMeasure_elecSumResetClear();

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchElecsumReset_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_firewareUpgrade(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("firewareUpgrade mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{				

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

					//同开关类型强制批量升级
					usrApp_firewareUpgrade_trig(true, (uint8_t)currentDev_typeGet());

//					//根节点设备被授权后，有权发起所有设备一起升级 --六合一有效
//					(esp_mesh_get_layer() == MESH_ROOT)?
//						(usrApp_firewareUpgrade_trig(true)):
//						(usrApp_firewareUpgrade_trig(false));
#else

//					usrApp_firewareUpgrade_trig(false);

					usrApp_firewareUpgrade_trig(true, (uint8_t)currentDev_typeGet());
#endif

					lvGui_tipsFullScreen_generate("system upgrading...", 245);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxTouchFwareUpgrade_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_mboxBtnActionClick_screenLandscapeConfirm(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;

	printf("screen landscape opreation mbox btn: %s released\n", txt);

	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					stt_devStatusRecord devStatusRecordFlg_temp = {0};

					devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

					if(opreatTemp_screenLandscapeIf){

						devStatusRecordFlg_temp.devScreenLandscape_IF = 1;
						lv_cb_set_checked(cbDispSelect_screenVer, false);
						lv_cb_set_checked(cbDispSelect_screenHor, true);
					}
					else
					{
						devStatusRecordFlg_temp.devScreenLandscape_IF = 0;
						lv_cb_set_checked(cbDispSelect_screenVer, true);
						lv_cb_set_checked(cbDispSelect_screenHor, false);
					}

//					usrApp_devIptdrv_paramRecalibration_set(true);
					devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);
					usrApplication_systemRestartTrig(3);

				}break;

				case 1:{

					if(devStatusDispMethod_landscapeIf_get()){

						lv_cb_set_checked(cbDispSelect_screenVer, false);
						lv_cb_set_checked(cbDispSelect_screenHor, true);
					}
					else
					{
						lv_cb_set_checked(cbDispSelect_screenVer, true);
						lv_cb_set_checked(cbDispSelect_screenHor, false);
					}

				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	
	}while(mbox_btnm_textTab[loop]);

	lv_mbox_start_auto_close(mboxScreenLandscape_comfirm, 20);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_factoryRecovery(lv_obj_t *btn){

	mboxFactoryRecovery_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxFactoryRecovery_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxFactoryRecovery_comfirm, NULL, LV_ALIGN_CENTER, -35, -55);
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
	lv_obj_align(mboxTouchRecalibration_comfirm, NULL, LV_ALIGN_CENTER, -35, -55);
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

static lv_res_t funCb_btnActionClick_elecsumResetClear(lv_obj_t *btn){

	mboxTouchElecsumReset_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxTouchElecsumReset_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxTouchElecsumReset_comfirm, NULL, LV_ALIGN_CENTER, -35, -55);
	lv_obj_set_width(mboxTouchElecsumReset_comfirm, 230);
	lv_mbox_set_text(mboxTouchElecsumReset_comfirm, "electric sum will be cleared, are you sure to continue?");
	lv_mbox_add_btns(mboxTouchElecsumReset_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BG, &styleMbox_bg);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchElecsumReset_comfirm, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchElecsumReset_comfirm, funCb_mboxBtnActionClick_elecsumReset);
	lv_mbox_set_anim_time(mboxTouchElecsumReset_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_fwareUpgrade(lv_obj_t *btn){

	mboxTouchFwareUpgrade_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxTouchFwareUpgrade_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxTouchFwareUpgrade_comfirm, NULL, LV_ALIGN_CENTER, -35, -55);
	lv_obj_set_width(mboxTouchFwareUpgrade_comfirm, 230);
	lv_mbox_set_text(mboxTouchFwareUpgrade_comfirm, "the system will be updated, are you sure to continue?");
	lv_mbox_add_btns(mboxTouchFwareUpgrade_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BG, &styleMbox_bg);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
	lv_mbox_set_style(mboxTouchFwareUpgrade_comfirm, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
	lv_mbox_set_action(mboxTouchFwareUpgrade_comfirm, funCb_mboxBtnActionClick_firewareUpgrade);
	lv_mbox_set_anim_time(mboxTouchFwareUpgrade_comfirm, 200);

	return LV_RES_OK;
}

void screenLandscape_opreatConfirm(void){

	mboxScreenLandscape_comfirm = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_protect(mboxScreenLandscape_comfirm, LV_PROTECT_POS);
	lv_obj_align(mboxScreenLandscape_comfirm, NULL, LV_ALIGN_CENTER, -35, -55);
	lv_obj_set_width(mboxScreenLandscape_comfirm, 230);
	lv_mbox_set_text(mboxScreenLandscape_comfirm, "This operation needs to be restarted to take effect, confirm to continue.");
	lv_mbox_add_btns(mboxScreenLandscape_comfirm, mbox_btnm_textTab, NULL);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BG, &styleMbox_bg);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
	lv_mbox_set_style(mboxScreenLandscape_comfirm, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
	lv_mbox_set_action(mboxScreenLandscape_comfirm, funCb_mboxBtnActionClick_screenLandscapeConfirm);
	lv_mbox_set_anim_time(mboxScreenLandscape_comfirm, 200);

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_elecParamDispSelect_power(lv_obj_t *cb){

	extern void lvGui_usrAppElecParam_dispRefresh(void);

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_elecParam_elecsum, false);
		devStatusRecordFlg_temp.devElecsumDisp_IF = 0;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

		lvGui_usrAppElecParam_dispRefresh();
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_elecParam_power, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_elecParamDispSelect_elecsum(lv_obj_t *cb){

	extern void lvGui_usrAppElecParam_dispRefresh(void);

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		lv_cb_set_checked(cbDispSelect_elecParam_power, false);
		devStatusRecordFlg_temp.devElecsumDisp_IF = 1;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

		lvGui_usrAppElecParam_dispRefresh();
	}	
	else
	{
		lv_cb_set_checked(cbDispSelect_elecParam_elecsum, true);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_scrrenDispMethod_ver(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		if(devStatusRecordFlg_temp.devScreenLandscape_IF){

			opreatTemp_screenLandscapeIf = false;
			
			lv_cb_set_checked(cbDispSelect_screenHor, false);
		
			screenLandscape_opreatConfirm();
		}
	}	
	else
	{
		lv_cb_set_checked(cb, true);
		lv_cb_set_checked(cbDispSelect_screenHor, false);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_cbOpreat_scrrenDispMethod_hor(lv_obj_t *cb){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	if(lv_cb_is_checked(cb)){

		if(!devStatusRecordFlg_temp.devScreenLandscape_IF){

			opreatTemp_screenLandscapeIf = true;

			lv_cb_set_checked(cbDispSelect_screenVer, false);

			screenLandscape_opreatConfirm();
		}
	}	
	else
	{
		lv_cb_set_checked(cb, true);
		lv_cb_set_checked(cbDispSelect_screenVer, false);
	}

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_functionSet_screenBrightnessAdj(lv_obj_t *slider){

	LV_OBJ_FREE_NUM_TYPE sliderFreeNum = lv_obj_get_free_num(slider);

	uint8_t brightnessTemp = lv_slider_get_value(slider) * (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div);
	uint8_t brightnessA = devScreenDriver_configParam_brightness_get();
	uint8_t brightnessB = devScreenDriver_configParam_brightnessSleep_get();

	switch(sliderFreeNum){

		case 0:{

			if(brightnessTemp == 0){
			
				lv_slider_set_value(sliderSettingA_screenBrightness, 1);
				brightnessTemp = lv_slider_get_value(slider) * (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div);
			}

			devScreenDriver_configParam_brightness_set(brightnessTemp, true);
			if(brightnessTemp < brightnessB){

				devScreenDriver_configParam_brightnessSleep_set(brightnessTemp, true);
				lv_slider_set_value(sliderSettingA_screenBrightnessSleep, brightnessTemp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			}

		}break;

		case 1:{

			devScreenDriver_configParam_brightnessSleep_set(brightnessTemp, true);
			if(brightnessTemp > brightnessA){

				devScreenDriver_configParam_brightness_set(brightnessTemp, true);
				lv_slider_set_value(sliderSettingA_screenBrightness, brightnessTemp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
			}

		}break;
	}

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

static lv_res_t funCb_ddlistDevTypeDef(lv_obj_t *ddlist){

	uint8_t loop = 0;
	uint8_t ddlist_id = lv_obj_get_free_num(ddlist);	

	if(ddlist_id == OBJ_DDLIST_DEVTYPE_FREENUM){

		uint16_t ddlist_opSelect = lv_ddlist_get_selected(ddlist);

		if(devTypeDdist_istRecord != ddlist_opSelect){

			devTypeDdist_istRecord = ddlist_opSelect;
			
			for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){
			
				if(ddlist_opSelect == deviceTypeTab_disp[loop].devInst_disp){
			
					stt_devDataPonitTypedef devDataPoint_temp = {0};
			
					currentDev_typeSet(deviceTypeTab_disp[loop].devType, true);
					devDriverManageBussiness_deviceChangeRefresh(); //驱动更新
					switch(currentDev_typeGet()){

						case devTypeDef_thermostat:
						case devTypeDef_thermostatExtension:{

							devDataPoint_temp.devType_thermostat.devThermostat_tempratureTarget = DEVICE_THERMOSTAT_TEMPTARGET_DEFAULT;
							currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false); //数据点加载默认值

						}break;

						case devTypeDef_scenario:{

							/*什么都不做*/
				
						}break;

						default:{

							currentDev_dataPointSet(&devDataPoint_temp, true, true, true, false); //数据点清零

						}break;
					}
			
					devSystemInfoLocalRecord_normalClear();
			
					usrAppHomepageBtnTextDisp_defaultLoad(deviceTypeTab_disp[loop].devType, true); //图文显示恢复默认
					usrAppHomepageBtnIconNumDisp_defaultLoad(deviceTypeTab_disp[loop].devType, true);
					
					break;
				}
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

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	printf("swDevStatusRecordIF set value get:%d.\n", sw_action);

	if(sw_action){

		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 1;
	}
	else
	{
		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
	}
	devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, true);

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

	lv_style_copy(&styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun.image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun.image.color = LV_COLOR_MAKE(200, 191, 231);

	lv_style_copy(&stylePageCb_elecParamDispSet, &lv_style_plain);
	stylePageCb_elecParamDispSet.body.border.part = LV_BORDER_NONE;
	stylePageCb_elecParamDispSet.body.empty = 1;
	stylePageCb_elecParamDispSet.text.color = LV_COLOR_AQUA;
	stylePageCb_elecParamDispSet.text.font = &lv_font_consola_16;

	lv_style_copy(&stylePageCb_screenLandscape, &lv_style_plain);
	stylePageCb_screenLandscape.body.border.part = LV_BORDER_NONE;
	stylePageCb_screenLandscape.body.empty = 1;
	stylePageCb_screenLandscape.text.color = LV_COLOR_AQUA;
	stylePageCb_screenLandscape.text.font = &lv_font_consola_16;
}

void lvGui_businessMenu_settingSet(lv_obj_t * obj_Parent){

	const uint16_t obj_animate_time = 150;
	const uint16_t obj_animate_delay = 100;
	uint16_t obj_animate_delayBasic = 0;

	uint8_t loop = 0;
	devTypeDef_enum devType_Temp;
	uint8_t homepageThemeStyle_temp = 0;
	stt_devStatusRecord devStatusRecordFlg_temp = {0};
	
	lvGuiSettingSet_objStyle_Init();

	devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);

	(devStatusDispMethod_landscapeIf_get())?
		(screenLandscapeCoordinate_objOffset = 60):
		(screenLandscapeCoordinate_objOffset = 0);

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "device define");
	lv_obj_align(text_Title, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -7);
	lv_obj_set_style(text_Title, &styleText_menuLevel_A);

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
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
//	lv_btn_set_style(menuBtnChoIcon_fun_home, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
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

	lv_obj_set_click(menuBtnChoIcon_fun_home, false);
	lv_obj_set_click(menuBtnChoIcon_fun_back, false);

	page_funSetOption = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(page_funSetOption, 320, 165)):
		(lv_obj_set_size(page_funSetOption, 240, 245));
	lv_obj_set_pos(page_funSetOption, 0, 75);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_SB, &stylePage_funSetOption);
	lv_page_set_style(page_funSetOption, LV_PAGE_STYLE_BG, &stylePage_funSetOption);
	lv_page_set_sb_mode(page_funSetOption, LV_SB_MODE_DRAG);
	lv_page_set_scrl_fit(page_funSetOption, false, false); //key opration
	if(devStatusDispMethod_landscapeIf_get()){
		
		lv_page_set_scrl_width(page_funSetOption, 300); 
		lv_page_set_scrl_height(page_funSetOption, 850); 
	}
	else
	{
		lv_page_set_scrl_width(page_funSetOption, 220); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		lv_page_set_scrl_height(page_funSetOption, 960); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	}

	lv_page_set_scrl_layout(page_funSetOption, LV_LAYOUT_CENTER);
//	lv_page_scroll_ver(page_funSetOption, 480);

	textSettingA_deviceType = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_deviceType, "device type:");
	lv_obj_set_style(textSettingA_deviceType, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_deviceType, LV_PROTECT_POS);
	lv_obj_align(textSettingA_deviceType, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 20);
	ddlistSettingA_deviceType = lv_ddlist_create(page_funSetOption, NULL);
	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_SB, &styleDdlistSettingA_devType);
	lv_ddlist_set_style(ddlistSettingA_deviceType, LV_DDLIST_STYLE_BG, &styleDdlistSettingA_devType);
	lv_obj_set_protect(ddlistSettingA_deviceType, LV_PROTECT_POS);
	lv_obj_align(ddlistSettingA_deviceType, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 50, 25);
	lv_ddlist_set_options(ddlistSettingA_deviceType, deviceType_listTab);
	lv_ddlist_set_action(ddlistSettingA_deviceType, funCb_ddlistDevTypeDef);
	lv_obj_set_top(ddlistSettingA_deviceType, true);
	lv_obj_set_free_num(ddlistSettingA_deviceType, OBJ_DDLIST_DEVTYPE_FREENUM);
	devType_Temp = currentDev_typeGet();
	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(devType_Temp == deviceTypeTab_disp[loop].devType){

			lv_ddlist_set_selected(ddlistSettingA_deviceType, deviceTypeTab_disp[loop].devInst_disp);
			devTypeDdist_istRecord = deviceTypeTab_disp[loop].devInst_disp;
			break;
		}
	}

	textSettingA_devStatusRecordIF = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_devStatusRecordIF, "stay the status:");
	lv_obj_set_style(textSettingA_devStatusRecordIF, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_devStatusRecordIF, LV_PROTECT_POS);
	lv_obj_align(textSettingA_devStatusRecordIF, textSettingA_deviceType, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	swSettingA_devStatusRecordIF = lv_sw_create(page_funSetOption, NULL);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_BG, &bg_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_INDIC, &indic_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_ON, &knob_on_styleDevStatusRecordIF);
	lv_sw_set_style(swSettingA_devStatusRecordIF, LV_SW_STYLE_KNOB_OFF, &knob_off_styleDevStatusRecordIF);
	lv_obj_set_protect(swSettingA_devStatusRecordIF, LV_PROTECT_POS);
	lv_obj_align(swSettingA_devStatusRecordIF, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);
	lv_sw_set_anim_time(swSettingA_devStatusRecordIF, 100);
	if(devStatusRecordFlg_temp.devStatusOnOffRecord_IF)lv_sw_on(swSettingA_devStatusRecordIF);
	else lv_sw_off(swSettingA_devStatusRecordIF);
	lv_sw_set_action(swSettingA_devStatusRecordIF, funCb_swDevStatusRecordIF);

	textSettingA_bGroundPicSelect = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_bGroundPicSelect, "background pic set:");
	lv_obj_set_style(textSettingA_bGroundPicSelect, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_bGroundPicSelect, LV_PROTECT_POS);
	lv_obj_align(textSettingA_bGroundPicSelect, textSettingA_devStatusRecordIF, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	pageBgroundPic_select = lv_page_create(page_funSetOption, NULL);
	lv_obj_set_size(pageBgroundPic_select, 200, 65);
	lv_obj_set_protect(pageBgroundPic_select, LV_PROTECT_POS);
	lv_obj_align(pageBgroundPic_select, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 0);
	lv_page_set_scrl_fit(pageBgroundPic_select, true, false);
//	lv_page_set_scrl_width(pageBgroundPic_select, 720); 
	lv_page_set_scrl_height(pageBgroundPic_select, 45); 
	lv_page_set_sb_mode(pageBgroundPic_select, LV_SB_MODE_HIDE);
	lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_SB, &stylePage_funSetOption);
	lv_page_set_style(pageBgroundPic_select, LV_PAGE_STYLE_BG, &stylePage_funSetOption);
	for(loop = 0; loop < BROUND_PIC_SELECT_MAX_NUM; loop ++){

		btnBgroundPicSel_preview[loop] = lv_imgbtn_create(pageBgroundPic_select, NULL);
		lv_obj_set_size(btnBgroundPicSel_preview[loop], 40, 40);
		lv_imgbtn_set_toggle(btnBgroundPicSel_preview[loop], true);
//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_PR, usrAppHomepageBkPic_prevDataGet(loop));
//		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_REL, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_src(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR, usrAppHomepageBkPic_prevDataGet(loop));
		lv_imgbtn_set_action(btnBgroundPicSel_preview[loop], LV_BTN_ACTION_CLICK, funCb_btnActionClick_bGroundPicOrgSelcet);
		lv_obj_set_free_num(btnBgroundPicSel_preview[loop], loop);
		lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_PR, usrAppHomepageBkPicStyle_prevDataGet(loop));
		lv_imgbtn_set_style(btnBgroundPicSel_preview[loop], LV_IMGBTN_STYLE_TGL_REL, usrAppHomepageBkPicStyle_prevDataGet(loop));
		lv_obj_set_protect(btnBgroundPicSel_preview[loop], LV_PROTECT_POS);
		lv_obj_set_pos(btnBgroundPicSel_preview[loop], loop * 45 + 5, 3);
		lv_page_glue_obj(btnBgroundPicSel_preview[loop], true);
		(loop == usrAppHomepageBgroundPicOrg_Get())?
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_PR)):
			(lv_imgbtn_set_state(btnBgroundPicSel_preview[loop], LV_BTN_STATE_TGL_REL));
	}
	imgBgroundPicSel_reserve = lv_img_create(btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], NULL);
	lv_img_set_src(imgBgroundPicSel_reserve, &bGroundPrev_picFigure_sel);
	lv_page_focus(pageBgroundPic_select, btnBgroundPicSel_preview[usrAppHomepageBgroundPicOrg_Get()], 0);

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{

			textSettingA_homepageThemestyle = lv_label_create(page_funSetOption, NULL);
			lv_label_set_text(textSettingA_homepageThemestyle, "theme:");
			lv_obj_set_style(textSettingA_homepageThemestyle, &styleText_menuLevel_B);
			lv_obj_set_protect(textSettingA_homepageThemestyle, LV_PROTECT_POS);
			lv_obj_align(textSettingA_homepageThemestyle, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
			ddlistSettingA_homepageThemestyle = lv_ddlist_create(page_funSetOption, NULL);
			lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_SB, &styleDdlistSettingA_devType);
			lv_ddlist_set_style(ddlistSettingA_homepageThemestyle, LV_DDLIST_STYLE_BG, &styleDdlistSettingA_devType);
			lv_obj_set_protect(ddlistSettingA_homepageThemestyle, LV_PROTECT_POS);
			lv_obj_align(ddlistSettingA_homepageThemestyle, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 50, 20);
			lv_ddlist_set_options(ddlistSettingA_homepageThemestyle, homepageThemeStyle_listTab);
			lv_ddlist_set_action(ddlistSettingA_homepageThemestyle, funCb_ddlistHomepageThemeDef);
			lv_obj_set_top(ddlistSettingA_homepageThemestyle, true);
			lv_obj_set_free_num(ddlistSettingA_homepageThemestyle, OBJ_DDLIST_HPTHEME_FREENUM);
			homepageThemeStyle_temp = usrAppHomepageThemeType_Get();
			lv_ddlist_set_selected(ddlistSettingA_homepageThemestyle, homepageThemeStyle_temp);

		}break;

		default:{}break;
	}

	textSettingA_factoryRecoveryIf = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_factoryRecoveryIf, "factory reset:");
	lv_obj_set_style(textSettingA_factoryRecoveryIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			
			lv_obj_align(textSettingA_factoryRecoveryIf, textSettingA_homepageThemestyle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
			
		}break;

		default:{
		
			lv_obj_align(textSettingA_factoryRecoveryIf, textSettingA_bGroundPicSelect, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);

		}break;
	}
	btnSettingA_factoryRecoveryIf = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_factoryRecoveryIf, 60, 20);
	lv_obj_set_protect(btnSettingA_factoryRecoveryIf, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_factoryRecoveryIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_factoryRecovery);
	lv_obj_align(btnSettingA_factoryRecoveryIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_factoryRecoveryIf, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnRef_factoryRecoveryIf = lv_label_create(btnSettingA_factoryRecoveryIf, NULL);
	lv_label_set_recolor(textBtnRef_factoryRecoveryIf, true);
	lv_label_set_text(textBtnRef_factoryRecoveryIf, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_factoryRecoveryIf, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_factoryRecoveryIf, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_factoryRecoveryIf, NULL, LV_ALIGN_CENTER, 0, 0);

//	textSettingA_touchRecalibrationIf = lv_label_create(page_funSetOption, NULL);
//	lv_label_set_text(textSettingA_touchRecalibrationIf, "touch recalibration:");
//	lv_obj_set_style(textSettingA_touchRecalibrationIf, &styleText_menuLevel_B);
//	lv_obj_set_protect(textSettingA_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_obj_align(textSettingA_touchRecalibrationIf, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
//	btnSettingA_touchRecalibrationIf = lv_btn_create(page_funSetOption, NULL);
//	lv_obj_set_size(btnSettingA_touchRecalibrationIf, 60, 20);
//	lv_obj_set_protect(btnSettingA_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_btn_set_action(btnSettingA_touchRecalibrationIf, LV_BTN_ACTION_CLICK, funCb_btnActionClick_touchRecalibration);
//	lv_obj_align(btnSettingA_touchRecalibrationIf, textSettingA_touchRecalibrationIf, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
//    lv_btn_set_style(btnSettingA_touchRecalibrationIf, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
//	textBtnRef_touchRecalibrationIf = lv_label_create(btnSettingA_touchRecalibrationIf, NULL);
//	lv_label_set_recolor(textBtnRef_touchRecalibrationIf, true);
//	lv_label_set_text(textBtnRef_touchRecalibrationIf, "#00FF40 >>>o<<<#");
//	lv_obj_set_style(textBtnRef_touchRecalibrationIf, &styleText_menuLevel_B);
//	lv_obj_set_protect(textBtnRef_touchRecalibrationIf, LV_PROTECT_POS);
//	lv_obj_align(textBtnRef_touchRecalibrationIf, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_screenBrightness = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenBrightness, "screen brightness:");
	lv_obj_set_style(textSettingA_screenBrightness, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenBrightness, LV_PROTECT_POS);
	lv_obj_align(textSettingA_screenBrightness, textSettingA_factoryRecoveryIf, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	sliderSettingA_screenBrightness = lv_slider_create(page_funSetOption, NULL);
	lv_obj_set_size(sliderSettingA_screenBrightness, 180, 15);
	lv_obj_set_protect(sliderSettingA_screenBrightness, LV_PROTECT_POS);
	lv_obj_align(sliderSettingA_screenBrightness, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 10);
	lv_obj_set_free_num(sliderSettingA_screenBrightness, 0);
	lv_slider_set_action(sliderSettingA_screenBrightness, funCb_slidAction_functionSet_screenBrightnessAdj);
	lv_slider_set_range(sliderSettingA_screenBrightness, 0, screenBrightness_sliderAdj_div);
	uint8_t brightnessSlider_valDisp = devScreenDriver_configParam_brightness_get();
	lv_slider_set_value(sliderSettingA_screenBrightness, brightnessSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));
	sliderSettingA_screenBrightnessSleep = lv_slider_create(page_funSetOption, sliderSettingA_screenBrightness);
	lv_slider_set_action(sliderSettingA_screenBrightnessSleep, funCb_slidAction_functionSet_screenBrightnessAdj);
	lv_obj_set_free_num(sliderSettingA_screenBrightnessSleep, 1);
	lv_obj_align(sliderSettingA_screenBrightnessSleep, textSettingA_screenBrightness, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 40);
	uint8_t brightnessSleepSlider_valDisp = devScreenDriver_configParam_brightnessSleep_get();
	lv_slider_set_value(sliderSettingA_screenBrightnessSleep, brightnessSleepSlider_valDisp / (DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV / screenBrightness_sliderAdj_div));

	textSettingA_screenLightTime = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenLightTime, "screen light time:");
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
	lv_obj_align(textBtnTimeRef_screenLightTime, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_MID, screenLandscapeCoordinate_objOffset + 35, 25);
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

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_HEATER) //热水器电量功能暂隐
	textSettingA_elecParamDispSelect = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_elecParamDispSelect, "electric parameter display:");
	lv_obj_set_style(textSettingA_elecParamDispSelect, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_elecParamDispSelect, LV_PROTECT_POS);
	lv_obj_align(textSettingA_elecParamDispSelect, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	cbDispSelect_elecParam_power = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_elecParam_power, 160, 15);
	lv_cb_set_text(cbDispSelect_elecParam_power, " power");
	lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_elecParam_power, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_elecParam_power, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 6);
	(devStatusRecordFlg_temp.devElecsumDisp_IF)?
		(lv_cb_set_checked(cbDispSelect_elecParam_power, false)):
		(lv_cb_set_checked(cbDispSelect_elecParam_power, true));
	cbDispSelect_elecParam_elecsum = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_elecParam_elecsum, 160, 15);
	lv_cb_set_text(cbDispSelect_elecParam_elecsum, " electric sum");
	lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_elecParam_elecsum, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_elecParam_elecsum, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 31);
	lv_cb_set_style(cbDispSelect_elecParam_power, LV_CB_STYLE_BG, &stylePageCb_elecParamDispSet);
	lv_cb_set_style(cbDispSelect_elecParam_elecsum, LV_CB_STYLE_BG, &stylePageCb_elecParamDispSet);
	lv_cb_set_action(cbDispSelect_elecParam_power, funCb_cbOpreat_elecParamDispSelect_power);
	lv_cb_set_action(cbDispSelect_elecParam_elecsum, funCb_cbOpreat_elecParamDispSelect_elecsum);
	(devStatusRecordFlg_temp.devElecsumDisp_IF)?
		(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, true)):
		(lv_cb_set_checked(cbDispSelect_elecParam_elecsum, false));

	textSettingA_elecsumResetClear = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_elecsumResetClear, "electric sum reset:");
	lv_obj_set_style(textSettingA_elecsumResetClear, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_elecsumResetClear, LV_PROTECT_POS);
	lv_obj_align(textSettingA_elecsumResetClear, textSettingA_elecParamDispSelect, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	btnSettingA_elecsumReset = lv_btn_create(page_funSetOption, NULL);
	lv_obj_set_size(btnSettingA_elecsumReset, 60, 20);
	lv_obj_set_protect(btnSettingA_elecsumReset, LV_PROTECT_POS);
	lv_btn_set_action(btnSettingA_elecsumReset, LV_BTN_ACTION_CLICK, funCb_btnActionClick_elecsumResetClear);
	lv_obj_align(btnSettingA_elecsumReset, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
    lv_btn_set_style(btnSettingA_elecsumReset, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
	textBtnRef_elecsumReset = lv_label_create(btnSettingA_elecsumReset, NULL);
	lv_label_set_recolor(textBtnRef_elecsumReset, true);
	lv_label_set_text(textBtnRef_elecsumReset, "#00FF40 >>>o<<<#");
	lv_obj_set_style(textBtnRef_elecsumReset, &styleText_menuLevel_B);
	lv_obj_set_protect(textBtnRef_elecsumReset, LV_PROTECT_POS);
	lv_obj_align(textBtnRef_elecsumReset, NULL, LV_ALIGN_CENTER, 0, 0);

	textSettingA_screenLandscape = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenLandscape, "screen display angle:");
	lv_obj_set_style(textSettingA_screenLandscape, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLandscape, LV_PROTECT_POS);
	lv_obj_align(textSettingA_screenLandscape, textSettingA_elecsumResetClear, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	cbDispSelect_screenVer = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenVer, 160, 15);
	lv_cb_set_text(cbDispSelect_screenVer, " vertical");
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_screenVer, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 6);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenVer, false)):
		(lv_cb_set_checked(cbDispSelect_screenVer, true));
	cbDispSelect_screenHor = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenHor, 160, 15);
	lv_cb_set_text(cbDispSelect_screenHor, " horizontal");
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_screenHor, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 31);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenHor, true)):
		(lv_cb_set_checked(cbDispSelect_screenHor, false));
	lv_cb_set_style(cbDispSelect_screenVer, LV_CB_STYLE_BG, &stylePageCb_screenLandscape);
	lv_cb_set_style(cbDispSelect_screenHor, LV_CB_STYLE_BG, &stylePageCb_screenLandscape);
	lv_cb_set_action(cbDispSelect_screenVer, funCb_cbOpreat_scrrenDispMethod_ver);
	lv_cb_set_action(cbDispSelect_screenHor, funCb_cbOpreat_scrrenDispMethod_hor);
	opreatTemp_screenLandscapeIf = devStatusRecordFlg_temp.devScreenLandscape_IF;
#else

	textSettingA_screenLandscape = lv_label_create(page_funSetOption, NULL);
	lv_label_set_text(textSettingA_screenLandscape, "screen display angle:");
	lv_obj_set_style(textSettingA_screenLandscape, &styleText_menuLevel_B);
	lv_obj_set_protect(textSettingA_screenLandscape, LV_PROTECT_POS);
	lv_obj_align(textSettingA_screenLandscape, textSettingA_screenLightTime, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
	cbDispSelect_screenVer = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenVer, 160, 15);
	lv_cb_set_text(cbDispSelect_screenVer, " vertical");
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenVer, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_screenVer, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 6);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenVer, false)):
		(lv_cb_set_checked(cbDispSelect_screenVer, true));
	cbDispSelect_screenHor = lv_cb_create(page_funSetOption, NULL);
	lv_obj_set_size(cbDispSelect_screenHor, 160, 15);
	lv_cb_set_text(cbDispSelect_screenHor, " horizontal");
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_POS);
	lv_obj_set_protect(cbDispSelect_screenHor, LV_PROTECT_FOLLOW);
	lv_obj_align(cbDispSelect_screenHor, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 10, 31);
	(devStatusRecordFlg_temp.devScreenLandscape_IF)?
		(lv_cb_set_checked(cbDispSelect_screenHor, true)):
		(lv_cb_set_checked(cbDispSelect_screenHor, false));
	lv_cb_set_style(cbDispSelect_screenVer, LV_CB_STYLE_BG, &stylePageCb_screenLandscape);
	lv_cb_set_style(cbDispSelect_screenHor, LV_CB_STYLE_BG, &stylePageCb_screenLandscape);
	lv_cb_set_action(cbDispSelect_screenVer, funCb_cbOpreat_scrrenDispMethod_ver);
	lv_cb_set_action(cbDispSelect_screenHor, funCb_cbOpreat_scrrenDispMethod_hor);
	opreatTemp_screenLandscapeIf = devStatusRecordFlg_temp.devScreenLandscape_IF;
#endif

	if(upgradeAvailable_flg){

		textSettingA_fwareUpgrade = lv_label_create(page_funSetOption, NULL);
		lv_label_set_text(textSettingA_fwareUpgrade, "fireware upgrade:");
		lv_obj_set_style(textSettingA_fwareUpgrade, &styleText_menuLevel_B);
		lv_obj_set_protect(textSettingA_fwareUpgrade, LV_PROTECT_POS);
		lv_obj_align(textSettingA_fwareUpgrade, textSettingA_screenLandscape, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 60);
		btnSettingA_fwareUpgrade = lv_btn_create(page_funSetOption, NULL);
		lv_obj_set_size(btnSettingA_fwareUpgrade, 60, 20);
		lv_obj_set_protect(btnSettingA_fwareUpgrade, LV_PROTECT_POS);
		lv_btn_set_action(btnSettingA_fwareUpgrade, LV_BTN_ACTION_CLICK, funCb_btnActionClick_fwareUpgrade);
		lv_obj_align(btnSettingA_fwareUpgrade, textSettingA_fwareUpgrade, LV_ALIGN_OUT_BOTTOM_LEFT, screenLandscapeCoordinate_objOffset + 60, 20);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_REL, &styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_PR, &styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_REL, &styleBtn_specialTransparent);
	    lv_btn_set_style(btnSettingA_fwareUpgrade, LV_BTN_STYLE_TGL_PR, &styleBtn_specialTransparent);
		textBtnRef_fwareUpgrade = lv_label_create(btnSettingA_fwareUpgrade, NULL);
		lv_label_set_recolor(textBtnRef_fwareUpgrade, true);
		lv_label_set_text(textBtnRef_fwareUpgrade, "#00FF40 >>>o<<<#");
		lv_obj_set_style(textBtnRef_fwareUpgrade, &styleText_menuLevel_B);
		lv_obj_set_protect(textBtnRef_fwareUpgrade, LV_PROTECT_POS);
		lv_obj_align(textBtnRef_fwareUpgrade, NULL, LV_ALIGN_CENTER, 0, 0);
	}

	lv_page_glue_obj(textSettingA_deviceType, true);
	lv_page_glue_obj(ddlistSettingA_deviceType, true);
	switch(currentDev_typeGet()){
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			lv_page_glue_obj(textSettingA_homepageThemestyle, true);
			lv_page_glue_obj(ddlistSettingA_homepageThemestyle, true);
		}break;
		default:{}break;
	}
	lv_page_glue_obj(textSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(textSettingA_bGroundPicSelect, true);
	lv_page_glue_obj(swSettingA_devStatusRecordIF, true);
	lv_page_glue_obj(textSettingA_factoryRecoveryIf, true);
	lv_page_glue_obj(btnSettingA_factoryRecoveryIf, true);
//	lv_page_glue_obj(textSettingA_touchRecalibrationIf, true);
//	lv_page_glue_obj(btnSettingA_touchRecalibrationIf, true);
//	lv_page_glue_obj(sliderSettingA_screenBrightness, true);
//	lv_page_glue_obj(sliderSettingA_screenBrightnessSleep, true);
	lv_page_glue_obj(textBtnTimeRef_screenLightTime, true);
	lv_page_glue_obj(btnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeAdd_screenLightTime, true);
	lv_page_glue_obj(btnTimeCut_screenLightTime, true);
	lv_page_glue_obj(textBtnTimeCut_screenLightTime, true);
#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_HEATER) //热水器电量功能暂隐
	lv_page_glue_obj(textSettingA_elecParamDispSelect, true);
	lv_page_glue_obj(textSettingA_elecsumResetClear, true);
#endif
	lv_page_glue_obj(textSettingA_screenLandscape, true);

	lv_obj_animate(textSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic, NULL);
	lv_obj_animate(ddlistSettingA_deviceType, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(swSettingA_devStatusRecordIF, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	lv_obj_animate(textSettingA_bGroundPicSelect, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(pageBgroundPic_select, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

	switch(currentDev_typeGet()){
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_thermostatExtension:{
			lv_obj_animate(textSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
			lv_obj_animate(ddlistSettingA_homepageThemestyle, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
		}break;
		default:{}break;
	}
	
	lv_obj_animate(textSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_factoryRecoveryIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
//	lv_obj_animate(textSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
//	lv_obj_animate(btnSettingA_touchRecalibrationIf, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_screenBrightness, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(sliderSettingA_screenBrightnessSleep, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	
	lv_obj_animate(textSettingA_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(textBtnTimeRef_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnTimeAdd_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnTimeCut_screenLightTime, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_HEATER) //热水器电量功能暂隐
	lv_obj_animate(textSettingA_elecParamDispSelect, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(cbDispSelect_elecParam_power, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(cbDispSelect_elecParam_elecsum, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	
	lv_obj_animate(textSettingA_elecsumResetClear, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(btnSettingA_elecsumReset, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
#endif

	lv_obj_animate(textSettingA_screenLandscape, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);
	lv_obj_animate(cbDispSelect_screenVer, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	lv_obj_animate(cbDispSelect_screenHor, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay,  NULL);

	if(upgradeAvailable_flg){

		lv_obj_animate(textSettingA_fwareUpgrade, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
		lv_obj_animate(btnSettingA_fwareUpgrade, LV_ANIM_FLOAT_LEFT, obj_animate_time, obj_animate_delayBasic += obj_animate_delay, NULL);
	}

	vTaskDelay(20 / portTICK_RATE_MS);	
	lv_obj_refresh_style(page_funSetOption);
	lv_obj_refresh_style(obj_Parent);

//	vTaskDelay((obj_animate_delay * 10) / portTICK_RATE_MS);
//	lv_page_focus(page_funSetOption, btnTimeCut_screenLightTime, obj_animate_time * 5);
//	vTaskDelay((obj_animate_time * 15) / portTICK_RATE_MS);
//	lv_page_focus(page_funSetOption, textSettingA_deviceType, obj_animate_time * 3);

	lv_obj_set_click(menuBtnChoIcon_fun_back, true);
	lv_obj_set_click(menuBtnChoIcon_fun_home, true);

	usrApp_fullScreenRefresh_self(50, 0);
}



/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#include "mdf_common.h"
#include "mwifi.h"
#include "mlink.h"
#include "mupgrade.h"

// /* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "sdkconfig.h"

#include "devDataManage.h"
#include "devDriver_manage.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu_wifiConfig.h"
#include "gui_businessHome.h"

#include "lcd_adapter.h"
#include "QR_Encode.h"

#define QR_BASIC_POSITION_X				45
#define QR_BASIC_POSITION_Y				120
#define QR_PIXEL_SIZE					7

#define LV_GUI_WIFICFG_APLIST_MAX_NUM	25

#define VALDEF_BOOL_TRUE				255
#define VALDEF_BOOL_FALSE				0

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_dejavu_20);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);

LV_IMG_DECLARE(iconMenu_wifiConfig);
LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(iconHeader_wifi_A);
LV_IMG_DECLARE(iconHeader_wifi_B);
LV_IMG_DECLARE(iconHeader_wifi_C);
LV_IMG_DECLARE(iconHeader_wifi_D);

LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

extern xQueueHandle msgQh_wifiConfigCompleteTips;

uint8_t wifiConfigComplete_tipsStartCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
uint8_t wifiConfigOpreat_scanningTimeCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
uint8_t wifiConfigOpreat_comfigFailCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;

static const char *mbox_btnm_textTab[] ={"Yes", "No", ""};

static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static lv_obj_t *menuText_opTips = NULL;
static lv_obj_t *menuText_devMac = NULL;
static lv_obj_t *pageTips_wifiConfigComplete = NULL;
static lv_obj_t *imageTips_wifiConfigComplete = NULL;
static lv_obj_t *labelTips_wifiConfigComplete = NULL;
static lv_obj_t *labelRef_wifiCfg_methodSel_wifiInfo = NULL;
static lv_obj_t *labelRef_wifiCfg_methodSel = NULL;
static lv_obj_t *labelRef_wifiCfg_methodQr_btRef = NULL;
static lv_obj_t *labelRef_wifiCfg_methodKb_btRef = NULL;
static lv_obj_t *labelRef_wifiCfg_methodLab_underline = NULL;
static lv_obj_t *btn_wifiCfg_methodQr = NULL;
static lv_obj_t *btn_wifiCfg_methodKb = NULL;

static lv_obj_t *pageUsrFun_wifiApListShowUp = NULL;
static lv_obj_t *lineObjSeparate_wifiApList[LV_GUI_WIFICFG_APLIST_MAX_NUM] = {NULL};
static lv_obj_t *btnSsidSel_wifiApList[LV_GUI_WIFICFG_APLIST_MAX_NUM] = {NULL};
static lv_obj_t *imgWifiSig_wifiApList[LV_GUI_WIFICFG_APLIST_MAX_NUM] = {NULL};
static lv_obj_t *labelSsidText_wifiApList[LV_GUI_WIFICFG_APLIST_MAX_NUM] = {NULL};

static lv_obj_t *labelUsrFun_wifiConfig_ssidTips = NULL;
static lv_obj_t *labelUsrFun_wifiConfig_psdTips = NULL;
static lv_obj_t *taUsrFun_wifiConfig_ssid = NULL;
static lv_obj_t *taUsrFun_wifiConfig_psd = NULL;
static lv_obj_t *labelUsrFun_wifiConfig_kbMoveTips = NULL;
static lv_obj_t *kbUsrFun_wifiConfig = NULL;
static lv_obj_t *pageUsrFun_KbWifiConfig = NULL;
static lv_obj_t *pageKbArea_KbWifiConfig = NULL;

static lv_obj_t *mboxBtnKbCall_restart = NULL;

static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B_infoMac;
static lv_style_t styleText_menuLevel_B_infoTips;
static lv_style_t stylePage_wifiConfigComplete;
static lv_style_t styleImage_wifiConfigComplete;
static lv_style_t styleLabelRef_wifiConfigComplete;
static lv_style_t styleLabelRef_wifiCfg_methodSel_info;
static lv_style_t styleLabelRef_wifiCfg_methodSel_btnRef;
static lv_style_t styleLabelRef_wifiCfg_methodSel_btnMore;

static lv_style_t styleMbox_bg;
static lv_style_t styleMbox_btn_pr;
static lv_style_t styleMbox_btn_rel;
static lv_style_t styleImg_menuFun_btnFun;
static lv_style_t styleBtn_specialTransparent;

static lv_style_t styleEmpty;
static lv_style_t stylePageUsrFun_wifiApListShowUp;
static lv_style_t styleBtnUsrFun_wifiApListShowUp;
static lv_style_t styleImageUsrFun_wifiSigRef;
static lv_style_t styleLabelUsrFun_wifiApSsid;

static lv_style_t styleTa_usrFun_wifiCfg;
static lv_style_t styleKb_usrFun_wifiCfg_btnBg;
static lv_style_t styleKb_usrFun_wifiCfg_btnRel;
static lv_style_t styleKb_usrFun_wifiCfg_btnPre;
static lv_style_t styleKb_usrFun_wifiCfg_btnTglRel;
static lv_style_t styleKb_usrFun_wifiCfg_btnTglPre;
static lv_style_t styleKb_usrFun_wifiCfg_btnIna;
static lv_style_t stylepageKbArea_KbWifiConfig;
static lv_style_t stylepageUsrFun_KbWifiConfig;
static lv_style_t styleLabelUsrFun_wifiConfig_psdTips;
static lv_style_t styleLabelUsrFun_wifiConfig_kbMoveTips;

static volatile bool wifiScanning_Flg = false;
static volatile bool keyboardCreat_Flg = false;
static volatile uint8_t QrCodeCreat_flg = VALDEF_BOOL_FALSE;
static volatile uint8_t QrCodeCreat_trig = VALDEF_BOOL_FALSE;

static char strTemp_wifiCfg_currentWifiStatusInfo[128] = {0};

static char wifiApList_infoTemp[LV_GUI_WIFICFG_APLIST_MAX_NUM][33] = {0};
static uint8_t wifiApSsidList_infoTemp[LV_GUI_WIFICFG_APLIST_MAX_NUM][6] = {0};
static uint8_t wifiApSsidSel_temp[6] = {0};

static void lvGuiStyleInit_businessMenu_wifiConfig(void);
static void lvGui_businessMenu_wifiConfig_methodByQrCode(lv_obj_t * obj_Parent);
static void lvGui_businessMenu_wifiConfig_methodByKeyboard(lv_obj_t * obj_Parent);
static void lvGui_businessMenu_wifiConfig_unitKeyboard(lv_obj_t * obj_Parent, char *ssidIpt);
static void lvGui_businessMenu_wifiConfig_keyboardCreat(lv_obj_t * obj_Parent);

static void lvGui_businessMenu_wifiConfig_wifiScanningStart(void);
static void lvGui_businessMenu_wifiConfig_wifiScanningComplete(void);

static void currentGui_elementClear(void){
	
	if(pageUsrFun_KbWifiConfig){

		lv_obj_del(pageUsrFun_KbWifiConfig);
		pageUsrFun_KbWifiConfig = NULL;
	}

	if(pageUsrFun_wifiApListShowUp){

		lv_obj_del(pageUsrFun_wifiApListShowUp);
		pageUsrFun_wifiApListShowUp = NULL;
	}

	if(keyboardCreat_Flg)keyboardCreat_Flg = false;
		
	if(menuText_devMac){
		lv_obj_del(menuText_devMac);
		menuText_devMac = NULL;
	}
	if(labelRef_wifiCfg_methodSel_wifiInfo){
		lv_obj_del(labelRef_wifiCfg_methodSel_wifiInfo);
		labelRef_wifiCfg_methodSel_wifiInfo = NULL;
	}
	if(mboxBtnKbCall_restart){
		lv_obj_del(mboxBtnKbCall_restart);
		mboxBtnKbCall_restart = NULL;
	}
	if(labelRef_wifiCfg_methodSel){
		lv_obj_del(labelRef_wifiCfg_methodSel);
		labelRef_wifiCfg_methodSel = NULL;
	}
	if(btn_wifiCfg_methodQr){
		lv_obj_del(btn_wifiCfg_methodQr);
		btn_wifiCfg_methodQr = NULL;
	}
	if(btn_wifiCfg_methodKb){
		lv_obj_del(btn_wifiCfg_methodKb);
		btn_wifiCfg_methodKb = NULL;
	}
}

void guiDispTimeOut_pageWifiCfg(void){

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

static lv_res_t funCb_btnAction_null(lv_obj_t *btn){

	

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

static lv_res_t funCb_btnActionPress_menuBtn_wifiCfg_btnByQr(lv_obj_t *btn){

	lv_obj_del(labelRef_wifiCfg_methodSel_wifiInfo);
	lv_obj_del(labelRef_wifiCfg_methodSel);
	lv_obj_del(btn_wifiCfg_methodKb);
	lv_obj_del(btn_wifiCfg_methodQr);
	
	labelRef_wifiCfg_methodSel_wifiInfo = NULL;
	labelRef_wifiCfg_methodSel = NULL;
	btn_wifiCfg_methodQr = NULL;
	btn_wifiCfg_methodKb = NULL;

	QrCodeCreat_trig = VALDEF_BOOL_TRUE;

	return LV_RES_OK;
}


static lv_res_t funCb_mboxBtnActionClick_btnKbCallRestart(lv_obj_t * mbox, const char * txt){

	uint8_t loop = 0;
	
	do{

		if(!strcmp(mbox_btnm_textTab[loop], txt)){

			switch(loop){

				case 0:{

					mdf_info_erase("ESP-MDF");
					usrApplication_systemRestartTrig(3);

				}break;

				case 1:{


				}break;

				default:break;
			}

			break;
		}
			
		loop ++;
	}while(mbox_btnm_textTab[loop]);

	lv_obj_set_click(btn_wifiCfg_methodQr, true);
	lv_obj_set_click(btn_wifiCfg_methodKb, true);

	lv_mbox_start_auto_close(mboxBtnKbCall_restart, 20);
	mboxBtnKbCall_restart = NULL;

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_wifiCfg_btnByKb(lv_obj_t *btn){

//	lv_obj_del(labelRef_wifiCfg_methodSel_wifiInfo);
//	lv_obj_del(labelRef_wifiCfg_methodSel);
//	lv_obj_del(btn_wifiCfg_methodQr);
//	lv_obj_del(btn_wifiCfg_methodKb);
//	
//	labelRef_wifiCfg_methodSel_wifiInfo = NULL;
//	labelRef_wifiCfg_methodSel = NULL;
//	labelRef_wifiCfg_methodQr_btRef = NULL;
//	labelRef_wifiCfg_methodKb_btRef = NULL;
//	btn_wifiCfg_methodQr = NULL;
//	btn_wifiCfg_methodKb = NULL;

	lv_obj_set_click(btn_wifiCfg_methodQr, false);
	lv_obj_set_click(btn_wifiCfg_methodKb, false);
	QrCodeCreat_trig = VALDEF_BOOL_FALSE;	

	if(meshNetwork_connectReserve_IF_get()){

		mboxBtnKbCall_restart = lv_mbox_create(lv_scr_act(), NULL);
		lv_obj_set_protect(mboxBtnKbCall_restart, LV_PROTECT_POS);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_align(mboxBtnKbCall_restart, NULL, LV_ALIGN_CENTER, -70, -75)):
			(lv_obj_align(mboxBtnKbCall_restart, NULL, LV_ALIGN_CENTER, -35, -85));
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_width(mboxBtnKbCall_restart, 300)):
			(lv_obj_set_width(mboxBtnKbCall_restart, 230));
		lv_mbox_set_text(mboxBtnKbCall_restart, "The current function needs to be effective without wifi configuration, Would you like to delete the current wifi configuration and restart the switch?");
		lv_mbox_add_btns(mboxBtnKbCall_restart, mbox_btnm_textTab, NULL);
		lv_mbox_set_style(mboxBtnKbCall_restart, LV_MBOX_STYLE_BG, &styleMbox_bg);
		lv_mbox_set_style(mboxBtnKbCall_restart, LV_MBOX_STYLE_BTN_REL, &styleMbox_btn_rel);
		lv_mbox_set_style(mboxBtnKbCall_restart, LV_MBOX_STYLE_BTN_PR, &styleMbox_btn_pr);
		lv_mbox_set_action(mboxBtnKbCall_restart, funCb_mboxBtnActionClick_btnKbCallRestart);
		lv_mbox_set_anim_time(mboxBtnKbCall_restart, 200);
	}
	else
	{
		lv_obj_set_click(btn_wifiCfg_methodKb, false);
		lvGui_businessMenu_wifiConfig_methodByKeyboard(lv_scr_act());
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_wifiApListSel(lv_obj_t *btn){

	LV_OBJ_FREE_NUM_TYPE objFreeNum = lv_obj_get_free_num(btn);

	printf("aplist apist:%d click.\n", objFreeNum);

	lvGui_businessMenu_wifiConfig_unitKeyboard(lv_layer_top(), wifiApList_infoTemp[objFreeNum]);
	memcpy(wifiApSsidSel_temp, wifiApSsidList_infoTemp[objFreeNum], sizeof(uint8_t) * 6);

//	lv_obj_del(pageUsrFun_wifiApListShowUp);

	return LV_RES_OK;
}

static lv_res_t funCb_taActionClick_wifiSelCfgIpt(lv_obj_t *ta){

	LV_OBJ_FREE_NUM_TYPE objFreeNum = lv_obj_get_free_num(ta);
	const char *textIpt = lv_ta_get_text(ta);

	lv_ta_set_cursor_type(taUsrFun_wifiConfig_ssid, LV_CURSOR_NONE);
	lv_ta_set_cursor_type(taUsrFun_wifiConfig_psd, LV_CURSOR_NONE);

	printf("kb[free num:%d] ipt data:%s.\n", objFreeNum, textIpt);

	lvGui_businessMenu_wifiConfig_keyboardCreat(pageUsrFun_KbWifiConfig);

	switch(objFreeNum){

		case 0:	lv_ta_set_cursor_type(taUsrFun_wifiConfig_ssid, LV_CURSOR_LINE);
				lv_kb_set_ta(kbUsrFun_wifiConfig, taUsrFun_wifiConfig_ssid);
				break;

		case 1: lv_ta_set_cursor_type(taUsrFun_wifiConfig_psd, LV_CURSOR_LINE);
				lv_kb_set_ta(kbUsrFun_wifiConfig, taUsrFun_wifiConfig_psd);
				break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_kbActionClick_wifiSelCfgOk(lv_obj_t *kb){

	extern void mconfig_blufi_completeInadvice_byKBoard(char ssid[32], char psd[64], uint8_t bssid[6]);

	char tmp_ssid[32] = {0};
	char tmp_psd[64] = {0};

	keyboardCreat_Flg = false;

	strcpy(tmp_ssid, lv_ta_get_text(taUsrFun_wifiConfig_ssid));
	strcpy(tmp_psd, lv_ta_get_text(taUsrFun_wifiConfig_psd));

	mconfig_blufi_completeInadvice_byKBoard(tmp_ssid, 
											tmp_psd,
											wifiApSsidSel_temp);

	lv_obj_set_click(btn_wifiCfg_methodQr, true);
	lv_obj_set_click(btn_wifiCfg_methodKb, true);

	if(pageUsrFun_wifiApListShowUp){

		lv_obj_del(pageUsrFun_wifiApListShowUp);
		pageUsrFun_wifiApListShowUp = NULL;
	}

	usrApp_fullScreenRefresh_self(50, 0); //刷新

	if(pageUsrFun_KbWifiConfig){

		lv_obj_del(pageUsrFun_KbWifiConfig);
		pageUsrFun_KbWifiConfig = NULL;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_kbActionClick_wifiSelCfgHidden(lv_obj_t *kb){

	lv_obj_set_click(btn_wifiCfg_methodQr, true);
	lv_obj_set_click(btn_wifiCfg_methodKb, true);

	keyboardCreat_Flg = false;

	if(pageUsrFun_wifiApListShowUp){

		lv_obj_del(pageUsrFun_wifiApListShowUp);
		pageUsrFun_wifiApListShowUp = NULL;
	}

	if(pageUsrFun_KbWifiConfig){

		lv_obj_del(pageUsrFun_KbWifiConfig);
		pageUsrFun_KbWifiConfig = NULL;
	}

	return LV_RES_OK;
}

static void lv_apListPageShowUp_styleInit(void){

	lv_style_copy(&styleEmpty, &lv_style_plain);
	styleEmpty.body.empty = 0;

	lv_style_copy(&styleBtnUsrFun_wifiApListShowUp, &lv_style_btn_rel);
	styleBtnUsrFun_wifiApListShowUp.body.main_color = LV_COLOR_TRANSP;
	styleBtnUsrFun_wifiApListShowUp.body.grad_color = LV_COLOR_TRANSP;
	styleBtnUsrFun_wifiApListShowUp.body.border.part = LV_BORDER_NONE;
	styleBtnUsrFun_wifiApListShowUp.body.opa = LV_OPA_TRANSP;
	styleBtnUsrFun_wifiApListShowUp.body.radius = 0;
	styleBtnUsrFun_wifiApListShowUp.body.shadow.width = 0;

	lv_style_copy(&stylePageUsrFun_wifiApListShowUp, &lv_style_plain_color);
	stylePageUsrFun_wifiApListShowUp.body.main_color = LV_COLOR_SILVER;
	stylePageUsrFun_wifiApListShowUp.body.grad_color = LV_COLOR_SILVER;
	stylePageUsrFun_wifiApListShowUp.body.border.part = LV_BORDER_NONE;
	stylePageUsrFun_wifiApListShowUp.body.radius = 6;
	stylePageUsrFun_wifiApListShowUp.body.opa = 245;
	stylePageUsrFun_wifiApListShowUp.body.padding.hor = 0;
	stylePageUsrFun_wifiApListShowUp.body.padding.inner = 0;	

	lv_style_copy(&styleImageUsrFun_wifiSigRef, &lv_style_plain_color);
	styleImageUsrFun_wifiSigRef.image.color = LV_COLOR_BLACK;
	styleImageUsrFun_wifiSigRef.image.intense = LV_OPA_50;

	lv_style_copy(&styleLabelUsrFun_wifiApSsid, &lv_style_plain_color);
	styleLabelUsrFun_wifiApSsid.text.color = LV_COLOR_BLACK;
	styleLabelUsrFun_wifiApSsid.text.font = &lv_font_consola_19;
	styleLabelUsrFun_wifiApSsid.text.opa = LV_OPA_60;
}

static void lvGui_businessMenu_wifiConfig_apListPage_showUp(lv_obj_t * obj_Parent, wifi_ap_record_t *wifiList_info, uint16_t APNum){

	const uint8_t separateArea_high = 40;
	uint8_t loop = 0;

	lv_apListPageShowUp_styleInit();

	pageUsrFun_wifiApListShowUp = lv_page_create(obj_Parent, NULL);
	lv_obj_set_top(pageUsrFun_wifiApListShowUp, true);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageUsrFun_wifiApListShowUp, 300, 205)):
		(lv_obj_set_size(pageUsrFun_wifiApListShowUp, 220, 275));
	lv_obj_set_pos(pageUsrFun_wifiApListShowUp, 10, 30);
	lv_page_set_style(pageUsrFun_wifiApListShowUp, LV_PAGE_STYLE_SB, &stylePageUsrFun_wifiApListShowUp);
	lv_page_set_style(pageUsrFun_wifiApListShowUp, LV_PAGE_STYLE_BG, &stylePageUsrFun_wifiApListShowUp);
	lv_page_set_sb_mode(pageUsrFun_wifiApListShowUp, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(pageUsrFun_wifiApListShowUp, false, false); //key opration
	(devStatusDispMethod_landscapeIf_get())?
		(lv_page_set_scrl_width(pageUsrFun_wifiApListShowUp, 300)):
		(lv_page_set_scrl_width(pageUsrFun_wifiApListShowUp, 220)); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	lv_page_set_scrl_height(pageUsrFun_wifiApListShowUp, separateArea_high * (APNum + 1)); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	lv_page_set_scrl_layout(pageUsrFun_wifiApListShowUp, LV_LAYOUT_CENTER);

	if(APNum > LV_GUI_WIFICFG_APLIST_MAX_NUM)APNum = LV_GUI_WIFICFG_APLIST_MAX_NUM;
	for(loop = 0; loop < APNum; loop ++){

		lineObjSeparate_wifiApList[loop] = lv_obj_create(pageUsrFun_wifiApListShowUp, NULL);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(lineObjSeparate_wifiApList[loop] , 290, 1)):
			(lv_obj_set_size(lineObjSeparate_wifiApList[loop] , 210, 1));
		lv_obj_set_protect(lineObjSeparate_wifiApList[loop], LV_PROTECT_POS);
		lv_obj_set_pos(lineObjSeparate_wifiApList[loop], 5, separateArea_high * (loop + 1));

		btnSsidSel_wifiApList[loop] = lv_btn_create(pageUsrFun_wifiApListShowUp, NULL);
		lv_obj_set_free_num(btnSsidSel_wifiApList[loop], loop);
		lv_btn_set_action(btnSsidSel_wifiApList[loop], LV_BTN_ACTION_CLICK, funCb_btnActionClick_wifiApListSel);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(btnSsidSel_wifiApList[loop], 280, separateArea_high - 5)):
			(lv_obj_set_size(btnSsidSel_wifiApList[loop], 200, separateArea_high - 5));
		lv_page_glue_obj(btnSsidSel_wifiApList[loop], true);
		lv_btn_set_style(btnSsidSel_wifiApList[loop], LV_BTN_STYLE_REL, &styleBtnUsrFun_wifiApListShowUp);
		lv_btn_set_style(btnSsidSel_wifiApList[loop], LV_BTN_STYLE_PR, &styleBtnUsrFun_wifiApListShowUp);
		lv_btn_set_style(btnSsidSel_wifiApList[loop], LV_BTN_STYLE_TGL_REL, &styleBtnUsrFun_wifiApListShowUp);
		lv_btn_set_style(btnSsidSel_wifiApList[loop], LV_BTN_STYLE_TGL_PR, &styleBtnUsrFun_wifiApListShowUp);
		lv_obj_set_protect(btnSsidSel_wifiApList[loop], LV_PROTECT_POS);
		lv_obj_align(btnSsidSel_wifiApList[loop], lineObjSeparate_wifiApList[loop], LV_ALIGN_OUT_TOP_MID, 0, -2);

		imgWifiSig_wifiApList[loop] = lv_img_create(btnSsidSel_wifiApList[loop], NULL);
		lv_obj_set_protect(imgWifiSig_wifiApList[loop], LV_PROTECT_POS);
		lv_obj_align(imgWifiSig_wifiApList[loop], btnSsidSel_wifiApList[loop], LV_ALIGN_IN_LEFT_MID, 0, 25);
		lv_img_set_src(imgWifiSig_wifiApList[loop], &iconHeader_wifi_A);
		lv_img_set_style(imgWifiSig_wifiApList[loop], &styleImageUsrFun_wifiSigRef);

		labelSsidText_wifiApList[loop] = lv_label_create(btnSsidSel_wifiApList[loop], NULL);
		lv_obj_set_style(labelSsidText_wifiApList[loop], &styleLabelUsrFun_wifiApSsid);
		lv_obj_set_protect(labelSsidText_wifiApList[loop], LV_PROTECT_POS);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_width(labelSsidText_wifiApList[loop], 230)):
			(lv_obj_set_width(labelSsidText_wifiApList[loop], 150));
		lv_obj_align(labelSsidText_wifiApList[loop], imgWifiSig_wifiApList[loop], LV_ALIGN_OUT_RIGHT_MID, 5, 0);
		lv_label_set_text(labelSsidText_wifiApList[loop], (const char*)wifiList_info[loop].ssid);
		lv_label_set_long_mode(labelSsidText_wifiApList[loop], LV_LABEL_LONG_DOT);
		lv_label_set_align(labelSsidText_wifiApList[loop], LV_LABEL_ALIGN_LEFT);
		lv_label_set_long_mode(labelSsidText_wifiApList[loop], LV_LABEL_LONG_DOT);

		memset(wifiApList_infoTemp[loop], 0, sizeof(uint8_t) * 33);
		memset(wifiApSsidList_infoTemp[loop], 0, sizeof(uint8_t) * 33);

		memcpy(wifiApList_infoTemp[loop], &(wifiList_info[loop].ssid), sizeof(char) * 33);
		memcpy(wifiApSsidList_infoTemp[loop], &(wifiList_info[loop].bssid), sizeof(uint8_t) * 6);
	}

	usrApp_fullScreenRefresh_self(50, 80);
}

static void lv_usrKeyboardTestUnit_styleInit(void){
	
	lv_style_copy(&stylepageKbArea_KbWifiConfig, &lv_style_plain);
	stylepageKbArea_KbWifiConfig.body.main_color = LV_COLOR_BLUE;
	stylepageKbArea_KbWifiConfig.body.grad_color = LV_COLOR_BLUE;
	stylepageKbArea_KbWifiConfig.body.border.part = LV_BORDER_NONE;
	stylepageKbArea_KbWifiConfig.body.opa = LV_OPA_TRANSP;
	stylepageKbArea_KbWifiConfig.body.radius = 0;
	stylepageKbArea_KbWifiConfig.body.shadow.width = 0;

	lv_style_copy(&stylepageUsrFun_KbWifiConfig, &lv_style_plain);
	stylepageUsrFun_KbWifiConfig.body.main_color = LV_COLOR_SILVER;
	stylepageUsrFun_KbWifiConfig.body.grad_color = LV_COLOR_SILVER;
	stylepageUsrFun_KbWifiConfig.body.border.part = LV_BORDER_NONE;
	stylepageUsrFun_KbWifiConfig.body.opa = 245;
	stylepageUsrFun_KbWifiConfig.body.radius = 5;
	stylepageUsrFun_KbWifiConfig.body.shadow.width = 0;

	lv_style_copy(&styleLabelUsrFun_wifiConfig_psdTips, &lv_style_plain);
	styleLabelUsrFun_wifiConfig_psdTips.text.color = LV_COLOR_BLACK;
	styleLabelUsrFun_wifiConfig_psdTips.text.font = &lv_font_consola_19;
	styleLabelUsrFun_wifiConfig_psdTips.text.opa = LV_OPA_100;
	lv_style_copy(&styleLabelUsrFun_wifiConfig_kbMoveTips, &lv_style_plain);
	styleLabelUsrFun_wifiConfig_kbMoveTips.text.color = LV_COLOR_GREEN;
	styleLabelUsrFun_wifiConfig_kbMoveTips.text.font = &lv_font_consola_19;
	styleLabelUsrFun_wifiConfig_kbMoveTips.text.opa = LV_OPA_40;

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnBg, &lv_style_plain);
    styleKb_usrFun_wifiCfg_btnBg.body.opa = LV_OPA_80;
    styleKb_usrFun_wifiCfg_btnBg.body.radius = 4;
    styleKb_usrFun_wifiCfg_btnBg.body.main_color = lv_color_hsv_to_rgb(15, 11, 30);
    styleKb_usrFun_wifiCfg_btnBg.body.grad_color = lv_color_hsv_to_rgb(15, 11, 30);
    styleKb_usrFun_wifiCfg_btnBg.text.color = lv_color_hsv_to_rgb(15, 5, 95);

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnRel, &lv_style_pretty);
    styleKb_usrFun_wifiCfg_btnRel.body.main_color = lv_color_hsv_to_rgb(15, 10, 40);
    styleKb_usrFun_wifiCfg_btnRel.body.grad_color = lv_color_hsv_to_rgb(15, 10, 20);
    styleKb_usrFun_wifiCfg_btnRel.body.border.color = LV_COLOR_HEX3(0x111);
    styleKb_usrFun_wifiCfg_btnRel.body.border.width = 1;
    styleKb_usrFun_wifiCfg_btnRel.body.border.opa = LV_OPA_70;
    styleKb_usrFun_wifiCfg_btnRel.body.padding.hor = LV_DPI / 4;
    styleKb_usrFun_wifiCfg_btnRel.body.padding.ver = LV_DPI / 8;
    styleKb_usrFun_wifiCfg_btnRel.body.shadow.type = LV_SHADOW_BOTTOM;
    styleKb_usrFun_wifiCfg_btnRel.body.shadow.color = LV_COLOR_HEX3(0x111);
    styleKb_usrFun_wifiCfg_btnRel.body.shadow.width = LV_DPI / 30;
    styleKb_usrFun_wifiCfg_btnRel.text.color = LV_COLOR_HEX3(0xeee);

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnPre, &styleKb_usrFun_wifiCfg_btnRel);
    styleKb_usrFun_wifiCfg_btnPre.body.main_color = lv_color_hsv_to_rgb(15, 10, 30);
    styleKb_usrFun_wifiCfg_btnPre.body.grad_color = lv_color_hsv_to_rgb(15, 10, 10);

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnTglRel, &styleKb_usrFun_wifiCfg_btnRel);
    styleKb_usrFun_wifiCfg_btnTglRel.body.main_color = lv_color_hsv_to_rgb(15, 10, 20);
    styleKb_usrFun_wifiCfg_btnTglRel.body.grad_color = lv_color_hsv_to_rgb(15, 10, 40);
    styleKb_usrFun_wifiCfg_btnTglRel.body.shadow.width = LV_DPI / 40;
    styleKb_usrFun_wifiCfg_btnTglRel.text.color = LV_COLOR_HEX3(0xddd);

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnTglPre, &styleKb_usrFun_wifiCfg_btnRel);
    styleKb_usrFun_wifiCfg_btnTglPre.body.main_color = lv_color_hsv_to_rgb(15, 10, 10);
    styleKb_usrFun_wifiCfg_btnTglPre.body.grad_color = lv_color_hsv_to_rgb(15, 10, 30);
    styleKb_usrFun_wifiCfg_btnTglPre.body.shadow.width = LV_DPI / 30;
    styleKb_usrFun_wifiCfg_btnTglPre.text.color = LV_COLOR_HEX3(0xddd);

    lv_style_copy(&styleKb_usrFun_wifiCfg_btnIna, &styleKb_usrFun_wifiCfg_btnRel);
    styleKb_usrFun_wifiCfg_btnIna.body.main_color = lv_color_hsv_to_rgb(15, 10, 20);
    styleKb_usrFun_wifiCfg_btnIna.body.grad_color = lv_color_hsv_to_rgb(15, 10, 20);
    styleKb_usrFun_wifiCfg_btnIna.text.color = LV_COLOR_HEX3(0xaaa);
    styleKb_usrFun_wifiCfg_btnIna.body.shadow.width = 0;
}

static void lvGui_businessMenu_wifiConfig_keyboardCreat(lv_obj_t * obj_Parent){

	if(!keyboardCreat_Flg){

		pageKbArea_KbWifiConfig = lv_page_create(obj_Parent, NULL);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(pageKbArea_KbWifiConfig, 320, 180)):
			(lv_obj_set_size(pageKbArea_KbWifiConfig, 240, 200));
		lv_obj_set_protect(pageKbArea_KbWifiConfig, LV_PROTECT_POS);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_pos(pageKbArea_KbWifiConfig, -10, 70)):
			(lv_obj_set_pos(pageKbArea_KbWifiConfig, 0, 115));
		lv_page_set_style(pageKbArea_KbWifiConfig, LV_PAGE_STYLE_SB, &stylepageKbArea_KbWifiConfig);
		lv_page_set_style(pageKbArea_KbWifiConfig, LV_PAGE_STYLE_BG, &stylepageKbArea_KbWifiConfig);
		lv_page_set_sb_mode(pageKbArea_KbWifiConfig, LV_SB_MODE_HIDE);
		lv_page_set_scrl_fit(pageKbArea_KbWifiConfig, false, false); //key opration
		(devStatusDispMethod_landscapeIf_get())?
			(lv_page_set_scrl_width(pageKbArea_KbWifiConfig, 300)):
			(lv_page_set_scrl_width(pageKbArea_KbWifiConfig, 460)); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		(devStatusDispMethod_landscapeIf_get())?
			(lv_page_set_scrl_height(pageKbArea_KbWifiConfig, 160)):
			(lv_page_set_scrl_height(pageKbArea_KbWifiConfig, 180)); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		lv_page_set_scrl_layout(pageKbArea_KbWifiConfig, LV_LAYOUT_CENTER);

	    /*Create a keyboard and apply the styles*/
		kbUsrFun_wifiConfig = lv_kb_create(pageKbArea_KbWifiConfig, NULL);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_size(kbUsrFun_wifiConfig, 310, 140)):
			(lv_obj_set_size(kbUsrFun_wifiConfig, 420, 150));
		lv_obj_set_protect(kbUsrFun_wifiConfig, LV_PROTECT_POS);
		(devStatusDispMethod_landscapeIf_get())?
			(lv_obj_set_pos(kbUsrFun_wifiConfig, 0, 0)):
			(lv_obj_set_pos(kbUsrFun_wifiConfig, 10, 15));
	    lv_kb_set_cursor_manage(kbUsrFun_wifiConfig, true);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BG, &styleKb_usrFun_wifiCfg_btnBg);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BTN_REL, &styleKb_usrFun_wifiCfg_btnRel);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BTN_PR, &styleKb_usrFun_wifiCfg_btnPre);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BTN_TGL_REL, &styleKb_usrFun_wifiCfg_btnTglRel);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BTN_TGL_PR, &styleKb_usrFun_wifiCfg_btnTglPre);
		lv_kb_set_style(kbUsrFun_wifiConfig, LV_BTNM_STYLE_BTN_INA, &styleKb_usrFun_wifiCfg_btnIna);
		lv_kb_set_ok_action(kbUsrFun_wifiConfig, funCb_kbActionClick_wifiSelCfgOk);
		lv_kb_set_hide_action(kbUsrFun_wifiConfig, funCb_kbActionClick_wifiSelCfgHidden);

		if(!devStatusDispMethod_landscapeIf_get()){

			labelUsrFun_wifiConfig_kbMoveTips = lv_label_create(pageKbArea_KbWifiConfig, NULL);
			lv_obj_set_protect(labelUsrFun_wifiConfig_kbMoveTips, LV_PROTECT_POS);
			lv_obj_align(labelUsrFun_wifiConfig_kbMoveTips, kbUsrFun_wifiConfig, LV_ALIGN_OUT_TOP_LEFT, -5, 0);
			lv_label_set_style(labelUsrFun_wifiConfig_kbMoveTips, &styleLabelUsrFun_wifiConfig_kbMoveTips);
			lv_label_set_text(labelUsrFun_wifiConfig_kbMoveTips, "     |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |");
		}
	}

	keyboardCreat_Flg = true;

	usrApp_fullScreenRefresh_self(50, 80);
}

static void lvGui_businessMenu_wifiConfig_unitKeyboard_detail(lv_obj_t * obj_Parent, char *ssidIpt){

	labelUsrFun_wifiConfig_psdTips = lv_label_create(obj_Parent, NULL);
	lv_obj_set_protect(labelUsrFun_wifiConfig_psdTips, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(labelUsrFun_wifiConfig_psdTips, 2, 15)):
		(lv_obj_set_pos(labelUsrFun_wifiConfig_psdTips, 20, 0));
	lv_label_set_style(labelUsrFun_wifiConfig_psdTips, &styleLabelUsrFun_wifiConfig_psdTips);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_label_set_text(labelUsrFun_wifiConfig_psdTips, "      ssid:")):
		(lv_label_set_text(labelUsrFun_wifiConfig_psdTips, "ssid:"));
	labelUsrFun_wifiConfig_ssidTips = lv_label_create(obj_Parent, labelUsrFun_wifiConfig_psdTips);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(labelUsrFun_wifiConfig_ssidTips, labelUsrFun_wifiConfig_psdTips, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18)):
		(lv_obj_align(labelUsrFun_wifiConfig_ssidTips, labelUsrFun_wifiConfig_psdTips, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40));
	lv_label_set_text(labelUsrFun_wifiConfig_ssidTips, "password:");

    /*Create a text area. The keyboard will write here*/
    taUsrFun_wifiConfig_ssid = lv_ta_create(obj_Parent, NULL);
	lv_obj_set_top(taUsrFun_wifiConfig_ssid, true);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(taUsrFun_wifiConfig_ssid, 180, 25)):
		(lv_obj_set_size(taUsrFun_wifiConfig_ssid, 200, 25));
	lv_obj_set_protect(taUsrFun_wifiConfig_ssid, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(taUsrFun_wifiConfig_ssid, labelUsrFun_wifiConfig_psdTips, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, -15)):
		(lv_obj_align(taUsrFun_wifiConfig_ssid, labelUsrFun_wifiConfig_psdTips, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0));
    lv_ta_set_one_line(taUsrFun_wifiConfig_ssid, true);
    lv_ta_set_text(taUsrFun_wifiConfig_ssid, ssidIpt);
    taUsrFun_wifiConfig_psd = lv_ta_create(obj_Parent, taUsrFun_wifiConfig_ssid);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(taUsrFun_wifiConfig_psd, labelUsrFun_wifiConfig_ssidTips, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, 5)):
		(lv_obj_align(taUsrFun_wifiConfig_psd, labelUsrFun_wifiConfig_ssidTips, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0));
    lv_ta_set_text(taUsrFun_wifiConfig_psd, "");
	
	lv_obj_set_free_num(taUsrFun_wifiConfig_ssid, 0);
	lv_obj_set_free_num(taUsrFun_wifiConfig_psd, 1);
	lv_ta_set_cursor_type(taUsrFun_wifiConfig_ssid, LV_CURSOR_NONE);
	lv_ta_set_cursor_type(taUsrFun_wifiConfig_psd, LV_CURSOR_LINE);
	lv_ta_set_action(taUsrFun_wifiConfig_ssid, funCb_taActionClick_wifiSelCfgIpt);
	lv_ta_set_action(taUsrFun_wifiConfig_psd, funCb_taActionClick_wifiSelCfgIpt);

	lvGui_businessMenu_wifiConfig_keyboardCreat(obj_Parent);

    /*Assign the text area to the keyboard*/
    lv_kb_set_ta(kbUsrFun_wifiConfig, taUsrFun_wifiConfig_psd);
}

static void lvGui_businessMenu_wifiConfig_unitKeyboard(lv_obj_t * obj_Parent, char *ssidIpt){

	lv_usrKeyboardTestUnit_styleInit();

	pageUsrFun_KbWifiConfig = lv_page_create(obj_Parent, NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(pageUsrFun_KbWifiConfig, 316, 220)):
		(lv_obj_set_size(pageUsrFun_KbWifiConfig, 236, 300));
	lv_obj_set_pos(pageUsrFun_KbWifiConfig, 2, 20);
	lv_page_set_style(pageUsrFun_KbWifiConfig, LV_PAGE_STYLE_SB, &stylepageUsrFun_KbWifiConfig);
	lv_page_set_style(pageUsrFun_KbWifiConfig, LV_PAGE_STYLE_BG, &stylepageUsrFun_KbWifiConfig);
	lv_page_set_sb_mode(pageUsrFun_KbWifiConfig, LV_SB_MODE_HIDE);
	lv_page_set_scrl_fit(pageUsrFun_KbWifiConfig, false, false); //key opration
	(devStatusDispMethod_landscapeIf_get())? //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		(lv_page_set_scrl_width(pageUsrFun_KbWifiConfig, 300)):
		(lv_page_set_scrl_width(pageUsrFun_KbWifiConfig, 220));
	(devStatusDispMethod_landscapeIf_get())? //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		(lv_page_set_scrl_height(pageUsrFun_KbWifiConfig, 240)):
		(lv_page_set_scrl_height(pageUsrFun_KbWifiConfig, 300));
	lv_page_set_scrl_layout(pageUsrFun_KbWifiConfig, LV_LAYOUT_COL_L);

	lvGui_businessMenu_wifiConfig_unitKeyboard_detail(pageUsrFun_KbWifiConfig, ssidIpt);
}

static void lvGui_businessMenu_wifiConfig_qrCode_refresh(void){

	uint8_t	qrCode_devMacBuff[6] = {0};
	char str_devMacBuff[30] = {0};

	int32_t posCoord_x = 0,
			posCoord_y = 0,
			qrCode_cellSize = 0;

	if(QrCodeCreat_flg != VALDEF_BOOL_TRUE)return;

	if(devStatusDispMethod_landscapeIf_get()){

		posCoord_x = 99;
		posCoord_y = 92;
		qrCode_cellSize = 6;
	}
	else
	{
		posCoord_x = QR_BASIC_POSITION_X;
		posCoord_y = QR_BASIC_POSITION_Y;
		qrCode_cellSize = QR_PIXEL_SIZE;
	}

//	printf("QrCodeCreat_flg:%d, QrCodeCreat_trig:%d.\n", QrCodeCreat_flg,
//														 QrCodeCreat_trig);

	esp_wifi_get_mac(ESP_IF_WIFI_STA, qrCode_devMacBuff);

//	sprintf(str_devMacBuff, "%02X%02X%02X%02X%02X%02X,%d", qrCode_devMacBuff[0],
//														   qrCode_devMacBuff[1],
//														   qrCode_devMacBuff[2],
//														   qrCode_devMacBuff[3],
//														   qrCode_devMacBuff[4],
//														   qrCode_devMacBuff[5],
//														   (int)meshNetwork_connectReserve_IF_get());

	sprintf(str_devMacBuff, "%02X%02X%02X%02X%02X%02X,%d,%d", qrCode_devMacBuff[0],
															  qrCode_devMacBuff[1],
															  qrCode_devMacBuff[2],
															  qrCode_devMacBuff[3],
															  qrCode_devMacBuff[4],
															  qrCode_devMacBuff[5],
															  (int)meshNetwork_connectReserve_IF_get(),
															  L8_DEVICE_TYPE_PANEL_DEF);
	
	printf("Qr code creat res:%d.\n", EncodeData(str_devMacBuff));
//	printf("Qr code creat res:%d.\n", EncodeData("hellow, Lanbon!"));

	externSocket_ex_disp_fill(0, QR_BASIC_POSITION_Y, 240, 320, LV_COLOR_WHITE);
	for(uint8_t loopa = 0; loopa < MAX_MODULESIZE; loopa ++){

		for(uint8_t loopb = 0; loopb < MAX_MODULESIZE; loopb ++){

			(m_byModuleData[loopa][loopb])?
				(externSocket_ex_disp_fill(posCoord_x + loopa * qrCode_cellSize, posCoord_y + loopb * qrCode_cellSize, posCoord_x + qrCode_cellSize + loopa * qrCode_cellSize, posCoord_y + qrCode_cellSize + loopb * qrCode_cellSize, LV_COLOR_BLACK)):
				(externSocket_ex_disp_fill(posCoord_x + loopa * qrCode_cellSize, posCoord_y + loopb * qrCode_cellSize, posCoord_x + qrCode_cellSize + loopa * qrCode_cellSize, posCoord_y + qrCode_cellSize + loopb * qrCode_cellSize, LV_COLOR_WHITE));
		}
	}
}

static void lvGui_businessMenu_wifiConfig_methodByQrCode(lv_obj_t * obj_Parent){

	uint8_t	qrCode_devMacBuff[6] = {0};
	char str_devMacBuff[30] = {0};
	
	esp_wifi_get_mac(ESP_IF_WIFI_STA, qrCode_devMacBuff);

	QrCodeCreat_flg = VALDEF_BOOL_TRUE;

	/*MAC地址文字保留不显示*/
//	menuText_devMac = lv_label_create(lv_scr_act(), NULL);
//	sprintf(str_devMacBuff, "devMac: %02X %02X %02X %02X %02X %02X", qrCode_devMacBuff[0],
//																	 qrCode_devMacBuff[1],
//																	 qrCode_devMacBuff[2],
//																	 qrCode_devMacBuff[3],
//																	 qrCode_devMacBuff[4],
//																	 qrCode_devMacBuff[5]);
//	lv_label_set_text(menuText_devMac, (const char *)str_devMacBuff);
//	lv_obj_set_style(menuText_devMac, &styleText_menuLevel_B_infoMac);
//	lv_obj_set_pos(menuText_devMac, 20, 300);

//	vTaskDelay(1000 / portTICK_PERIOD_MS);

	lvGui_businessMenu_wifiConfig_qrCode_refresh();
}

void lvGui_businessMenu_wifiConfig_methodByKeyboard(lv_obj_t * obj_Parent){

	lvGui_businessMenu_wifiConfig_wifiScanningStart();
}

static void lvGui_businessMenu_wifiConfig_wifiScanningStart(void){

	lvGui_usrAppBussinessRunning_block(0, "   wifi\n   scanning...", 8); //wifi扫描，UI阻塞
	wifiScanning_Flg = true;
	wifiConfigOpreat_scanningTimeCounter = 5;
}

static void lvGui_businessMenu_wifiConfig_wifiScanningComplete(void){

	uint16_t loop = 0;
	uint16_t apCount = 0;
	wifi_ap_record_t *ap_list = NULL;

	esp_wifi_scan_get_ap_num(&apCount);
	ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
	esp_wifi_scan_stop();

	lvGui_usrAppBussinessRunning_blockCancel(); //UI阻塞提示解除

	lv_obj_set_click(btn_wifiCfg_methodKb, true);

//	for(loop = 0; loop < apCount; loop ++){

//		printf("ssid:[%s], mac:["MACSTR"], rssi:[%d]\n", (char *)ap_list[loop].ssid, 
//														 MAC2STR(ap_list[loop].bssid),
//														 ap_list[loop].rssi);
//	}

	lvGui_businessMenu_wifiConfig_apListPage_showUp(lv_layer_top(), ap_list, apCount);
	
	free(ap_list);
}

void lvGui_businessMenu_wifiConfig_wifiStatusReleasing(void){

	const char *textTabAlign_ver = "\0";
	const char *textTabAlign_hor = "      ";
		  char *textTabAlign = NULL;

  (devStatusDispMethod_landscapeIf_get())?
	  (textTabAlign = textTabAlign_hor):
	  (textTabAlign = textTabAlign_ver);

	if(meshNetwork_connectReserve_IF_get()){

		if(flgGet_gotRouterOrMeshConnect()){

			stt_routerCfgInfo routerCfgInfo_temp = {0};

			currentRouterCfgInfo_paramGet(&routerCfgInfo_temp);

			sprintf(strTemp_wifiCfg_currentWifiStatusInfo, "wifi status:\n  %sSSID: #B5E61D %s#\n  %sPSD: #B5E61D %s#",
															textTabAlign,
													 		routerCfgInfo_temp.routerInfo_ssid,
													 		textTabAlign,
													 		routerCfgInfo_temp.routerInfo_psd);
		}
		else
		{
			sprintf(strTemp_wifiCfg_currentWifiStatusInfo, "wifi status:\n\n	#B9B9FF %sRECONNECTING...#", textTabAlign);
		}
	}
	else
	{
		sprintf(strTemp_wifiCfg_currentWifiStatusInfo, "wifi status:\n\n	#B9B9FF    %sNO CONFIG#", textTabAlign);
	}
}

void lvGui_businessMenu_wifiConfig_methodSelect(lv_obj_t * obj_Parent){
	
	labelRef_wifiCfg_methodSel_wifiInfo = lv_label_create(obj_Parent, NULL);
	lvGui_businessMenu_wifiConfig_wifiStatusReleasing();
	lv_label_set_recolor(labelRef_wifiCfg_methodSel_wifiInfo, true);
	lv_label_set_text(labelRef_wifiCfg_methodSel_wifiInfo, strTemp_wifiCfg_currentWifiStatusInfo);
	lv_obj_set_protect(labelRef_wifiCfg_methodSel_wifiInfo, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_pos(labelRef_wifiCfg_methodSel_wifiInfo, 20, 80)):
		(lv_obj_set_pos(labelRef_wifiCfg_methodSel_wifiInfo, 20, 100));
	lv_obj_set_style(labelRef_wifiCfg_methodSel_wifiInfo, &styleLabelRef_wifiCfg_methodSel_info);
	
	labelRef_wifiCfg_methodSel = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(labelRef_wifiCfg_methodSel, "method select:");
	lv_obj_set_protect(labelRef_wifiCfg_methodSel, LV_PROTECT_POS);
	lv_obj_align(labelRef_wifiCfg_methodSel, labelRef_wifiCfg_methodSel_wifiInfo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
	lv_obj_set_style(labelRef_wifiCfg_methodSel, &styleLabelRef_wifiCfg_methodSel_info);

	btn_wifiCfg_methodQr = lv_btn_create(obj_Parent, NULL);
	lv_obj_set_size(btn_wifiCfg_methodQr, 110, 50);
	lv_obj_set_protect(btn_wifiCfg_methodQr, LV_PROTECT_POS);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(btn_wifiCfg_methodQr, labelRef_wifiCfg_methodSel, LV_ALIGN_OUT_BOTTOM_LEFT, 55, 10)):
		(lv_obj_align(btn_wifiCfg_methodQr, labelRef_wifiCfg_methodSel, LV_ALIGN_OUT_BOTTOM_LEFT, 25, 25));
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BG, &styleKb_usrFun_wifiCfg_btnBg);
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BTN_REL, &styleKb_usrFun_wifiCfg_btnRel);
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BTN_PR, &styleKb_usrFun_wifiCfg_btnPre);
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BTN_TGL_REL, &styleKb_usrFun_wifiCfg_btnTglRel);
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BTN_TGL_PR, &styleKb_usrFun_wifiCfg_btnTglPre);
	lv_kb_set_style(btn_wifiCfg_methodQr, LV_BTNM_STYLE_BTN_INA, &styleKb_usrFun_wifiCfg_btnIna);
	lv_btn_set_action(btn_wifiCfg_methodQr, LV_BTN_ACTION_CLICK, funCb_btnActionPress_menuBtn_wifiCfg_btnByQr);

	labelRef_wifiCfg_methodQr_btRef = lv_label_create(btn_wifiCfg_methodQr, NULL);
	lv_obj_set_style(labelRef_wifiCfg_methodQr_btRef, &styleLabelRef_wifiCfg_methodSel_btnRef);
	lv_label_set_text(labelRef_wifiCfg_methodQr_btRef, "#99D9EA Qr code#");
	lv_label_set_recolor(labelRef_wifiCfg_methodQr_btRef, true);

	btn_wifiCfg_methodKb = lv_btn_create(obj_Parent, btn_wifiCfg_methodQr);
	lv_obj_set_size(btn_wifiCfg_methodKb, 60, 35);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BG, &lv_style_transp);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BTN_REL, &lv_style_transp);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BTN_PR, &lv_style_transp);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BTN_TGL_REL, &lv_style_transp);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BTN_TGL_PR, &lv_style_transp);
	lv_kb_set_style(btn_wifiCfg_methodKb, LV_BTNM_STYLE_BTN_INA, &lv_style_transp);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_align(btn_wifiCfg_methodKb, btn_wifiCfg_methodQr, LV_ALIGN_OUT_BOTTOM_RIGHT, 100, -32)):
		(lv_obj_align(btn_wifiCfg_methodKb, btn_wifiCfg_methodQr, LV_ALIGN_OUT_BOTTOM_RIGHT, 70, 15));
	lv_btn_set_action(btn_wifiCfg_methodKb, LV_BTN_ACTION_CLICK, funCb_btnActionPress_menuBtn_wifiCfg_btnByKb);
	lv_btn_set_action(btn_wifiCfg_methodKb, LV_BTN_ACTION_PR, 			  funCb_btnAction_null);
	lv_btn_set_action(btn_wifiCfg_methodKb, LV_BTN_ACTION_LONG_PR, 		  funCb_btnAction_null);
	lv_btn_set_action(btn_wifiCfg_methodKb, LV_BTN_ACTION_LONG_PR_REPEAT, funCb_btnAction_null);
	labelRef_wifiCfg_methodKb_btRef = lv_label_create(btn_wifiCfg_methodKb, labelRef_wifiCfg_methodQr_btRef);
	lv_obj_set_style(labelRef_wifiCfg_methodKb_btRef, &styleLabelRef_wifiCfg_methodSel_btnMore);
	lv_label_set_text(labelRef_wifiCfg_methodKb_btRef, "#22B14C . . .#");
	
	labelRef_wifiCfg_methodLab_underline = lv_obj_create(btn_wifiCfg_methodKb, NULL); //下划线
	lv_obj_set_size(labelRef_wifiCfg_methodLab_underline, 33, 1);
	lv_obj_set_protect(labelRef_wifiCfg_methodLab_underline, LV_PROTECT_POS);
	lv_obj_align(labelRef_wifiCfg_methodLab_underline, labelRef_wifiCfg_methodKb_btRef, LV_ALIGN_IN_BOTTOM_MID, -1, 7);

	if(meshNetwork_connectReserve_IF_get()){

		lv_obj_set_size(labelRef_wifiCfg_methodLab_underline, 53, 1);
		lv_label_set_text(labelRef_wifiCfg_methodKb_btRef, "#808080 reset#");
		lv_obj_align(labelRef_wifiCfg_methodLab_underline, labelRef_wifiCfg_methodKb_btRef, LV_ALIGN_IN_BOTTOM_MID, -1, 3);
	}
	
	vTaskDelay(20 / portTICK_PERIOD_MS);
	lv_obj_refresh_style(obj_Parent);
}

void lvGui_businessMenu_wifiConfig(lv_obj_t * obj_Parent){

	lv_obj_t *text_Title;

	lvGuiStyleInit_businessMenu_wifiConfig();
	lv_apListPageShowUp_styleInit();
	lv_usrKeyboardTestUnit_styleInit();

	xQueueReset(msgQh_wifiConfigCompleteTips); //指定提示队列清空

	menuText_devMac = lv_label_create(lv_scr_act(), NULL);

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "wifi config");
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
	
//	menuText_opTips = lv_label_create(obj_Parent, NULL);
//	lv_label_set_text(menuText_opTips, "Please operate carefully according to the wifi status.");
//	lv_label_set_long_mode(menuText_opTips, LV_LABEL_LONG_SCROLL);
//	lv_obj_set_style(menuText_opTips, &styleText_menuLevel_B_infoTips);
//	lv_obj_set_protect(menuText_opTips, LV_PROTECT_POS);
//	lv_obj_align(menuText_opTips, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

//	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
//	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack);
//	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack);
//	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 35);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
//	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);

//	labelRef_wifiCfg_methodSel_wifiInfo = lv_label_create(obj_Parent, NULL);
//	lv_label_set_text(menuText_opTips, "scan the QR code to config the WIFI.");
//	lv_label_set_long_mode(menuText_opTips, LV_LABEL_LONG_SCROLL);
//	lv_obj_set_style(menuText_opTips, &styleText_menuLevel_B_infoTips);
//	lv_obj_set_protect(menuText_opTips, LV_PROTECT_POS);
//	lv_obj_align(menuText_opTips, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

	lvGui_businessMenu_wifiConfig_methodSelect(lv_scr_act());

	QrCodeCreat_trig = VALDEF_BOOL_FALSE; 
	QrCodeCreat_flg = VALDEF_BOOL_FALSE;
}

void lvGui_wifiConfig_bussiness_configComplete_tipsTrig(void){

	uint8_t msgQh_sptrDataWifiConfig = 'A';
	BaseType_t res = pdFALSE;

	xQueueReset(msgQh_wifiConfigCompleteTips);
	res = xQueueSendToFront(msgQh_wifiConfigCompleteTips, &msgQh_sptrDataWifiConfig, 1 / portTICK_PERIOD_MS);
	wifiConfigComplete_tipsStartCounter = 3;

	printf("wifi config cmp tiptrig msg tx res:%d.\n", res);
}

void lvGui_wifiConfig_bussiness_configComplete_tipsOver(void){

	uint8_t msgQh_sptrDataWifiConfig = 'B';
	
	xQueueSend(msgQh_wifiConfigCompleteTips, &msgQh_sptrDataWifiConfig, 1 / portTICK_PERIOD_MS);
}

void lvGui_wifiConfig_bussiness_configFail_tipsTrig(uint8_t tipsTime, uint8_t err){

	char strTips_temp[64] = {0};
	sprintf(strTips_temp, "#FF007F wifi connect#\n#FF007F fail[error:%d]#", err);
	lvGui_usrAppBussinessRunning_block(1, (const char*)strTips_temp, tipsTime); //UI阻塞提示，wifi连接失败
	wifiConfigOpreat_comfigFailCounter = tipsTime + 2;
}

void lvGui_wifiConfig_bussiness_configComplete_tipsDetect(void){

	const uint8_t qrCodeTrigDelay_period = 1;
	uint8_t msgQh_rptrDataWifiConfig = 0;
	static bool devNetworkReserve_flg = true;
	static uint8_t qrCodeTrigDelay_counter = qrCodeTrigDelay_period;

	static enum{

		wifiStatus_noneCfg = 0,
		wifiStatus_connected,
		wifiStatus_connecting
		
	}devWifiStatus_record = wifiStatus_noneCfg,
	 devWifiStatus_temp = wifiStatus_noneCfg;

	if(meshNetwork_connectReserve_IF_get()){

		if(flgGet_gotRouterOrMeshConnect()){

			devWifiStatus_temp = wifiStatus_connected;
		}
		else
		{
			devWifiStatus_temp = wifiStatus_connecting;
		}
	}
	else
	{
		devWifiStatus_temp = wifiStatus_noneCfg;
	}

	if(devWifiStatus_record != devWifiStatus_temp){

		devWifiStatus_record = devWifiStatus_temp;

		lvGui_businessMenu_wifiConfig_wifiStatusReleasing();
		if(labelRef_wifiCfg_methodSel_wifiInfo)
			lv_label_set_text(labelRef_wifiCfg_methodSel_wifiInfo, strTemp_wifiCfg_currentWifiStatusInfo);
	}

	if(QrCodeCreat_trig == VALDEF_BOOL_TRUE){

		if(qrCodeTrigDelay_counter)qrCodeTrigDelay_counter --;
		else{

			lvGui_businessMenu_wifiConfig_methodByQrCode(lv_scr_act());

			qrCodeTrigDelay_counter = qrCodeTrigDelay_period;
			QrCodeCreat_trig = VALDEF_BOOL_FALSE;	

//			printf("damn trig 1!!!!!!!!!!!!!!\n");
		}
	}

	if(meshNetwork_connectReserve_IF_get()){

		if(!devNetworkReserve_flg){

			lvGui_businessMenu_wifiConfig_qrCode_refresh();			
			devNetworkReserve_flg = true;

//			printf("damn trig 2!!!!!!!!!!!!!!\n");
		}
	}
	else
	{
		if(devNetworkReserve_flg)
			devNetworkReserve_flg = false;
	}

	if(wifiScanning_Flg){
	
		static bool scanningTrig_flg = false;

		if(!scanningTrig_flg){ //wifi scanning开始

			wifi_scan_config_t scanConf = {
				.ssid = NULL,
				.bssid = NULL,
				.channel = 0,
				.show_hidden = false
			};

			scanningTrig_flg = true;
			
			ESP_ERROR_CHECK(esp_wifi_disconnect());
			ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, false));
		}

		if(!wifiConfigOpreat_scanningTimeCounter){ //wifi scanning结束，计时逻辑在timerSoft内进行

			wifiScanning_Flg = false;
			scanningTrig_flg = false;
			
			lvGui_businessMenu_wifiConfig_wifiScanningComplete();
		}
	}

	if(wifiConfigOpreat_comfigFailCounter != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){

		if(!wifiConfigOpreat_comfigFailCounter){

			wifiConfigOpreat_comfigFailCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
			lvGui_businessMenu_wifiConfig_qrCode_refresh();		
		}
	}

	if(xQueueReceive(msgQh_wifiConfigCompleteTips, &msgQh_rptrDataWifiConfig, 1 / portTICK_RATE_MS) == pdTRUE){

		printf("wifi config cmp tiptrig msg rcv.\n");

	 	if(msgQh_rptrDataWifiConfig == 'A'){

//			lvGui_usrAppBussinessRunning_blockCancel(); //UI阻塞提示解除

			if(pageTips_wifiConfigComplete == NULL){

				pageTips_wifiConfigComplete = lv_page_create(lv_layer_top(), NULL);
				lv_obj_set_size(pageTips_wifiConfigComplete, 140, 90);
				lv_obj_set_protect(pageTips_wifiConfigComplete, LV_PROTECT_POS);
				lv_obj_align(pageTips_wifiConfigComplete, NULL, LV_ALIGN_CENTER, 0, 50);
//				lv_obj_animate(pageTips_wifiConfigComplete, LV_ANIM_FLOAT_BOTTOM, 400,	0, NULL);
				lv_obj_set_top(pageTips_wifiConfigComplete, true);
				lv_page_set_style(pageTips_wifiConfigComplete, LV_PAGE_STYLE_SB, &stylePage_wifiConfigComplete);
				lv_page_set_style(pageTips_wifiConfigComplete, LV_PAGE_STYLE_BG, &stylePage_wifiConfigComplete);
				lv_page_set_sb_mode(pageTips_wifiConfigComplete, LV_SB_MODE_HIDE);	
				lv_page_set_scrl_fit(pageTips_wifiConfigComplete, false, true); //key opration
				lv_page_set_scrl_layout(pageTips_wifiConfigComplete, LV_LAYOUT_PRETTY);

				imageTips_wifiConfigComplete = lv_img_create(pageTips_wifiConfigComplete, NULL);
				lv_obj_set_top(imageTips_wifiConfigComplete, true);
				lv_img_set_style(imageTips_wifiConfigComplete, &styleImage_wifiConfigComplete);
				lv_obj_set_protect(imageTips_wifiConfigComplete, LV_PROTECT_POS);
				lv_obj_align(imageTips_wifiConfigComplete, pageTips_wifiConfigComplete, LV_ALIGN_CENTER, 30, 60);

				labelTips_wifiConfigComplete = lv_label_create(pageTips_wifiConfigComplete, NULL);
				lv_obj_set_top(labelTips_wifiConfigComplete, true);
				lv_label_set_style(labelTips_wifiConfigComplete, &styleLabelRef_wifiConfigComplete);
				lv_obj_set_protect(labelTips_wifiConfigComplete, LV_PROTECT_POS);
				lv_obj_align(labelTips_wifiConfigComplete, pageTips_wifiConfigComplete, LV_ALIGN_IN_BOTTOM_MID, -28, -5);
			}

			lv_img_set_src(imageTips_wifiConfigComplete, &iconMenu_wifiConfig);
			lv_label_set_text(labelTips_wifiConfigComplete, "config ok.");

			usrApp_fullScreenRefresh_self(50, 80);
		}
		else
		if(msgQh_rptrDataWifiConfig == 'B'){

			if(pageTips_wifiConfigComplete){
			
				lv_obj_del(pageTips_wifiConfigComplete);
				pageTips_wifiConfigComplete = NULL;
			}
			currentGui_elementClear();			
			lvGui_usrSwitch(bussinessType_Home);
		}
	}
}

static void lvGuiStyleInit_businessMenu_wifiConfig(void){

	lv_style_copy(&stylePage_wifiConfigComplete, &lv_style_plain_color);
	stylePage_wifiConfigComplete.body.main_color = LV_COLOR_WHITE;
	stylePage_wifiConfigComplete.body.grad_color = LV_COLOR_WHITE;
	stylePage_wifiConfigComplete.body.border.part = LV_BORDER_NONE;
	stylePage_wifiConfigComplete.body.radius = 6;
	stylePage_wifiConfigComplete.body.opa = LV_OPA_90;
	stylePage_wifiConfigComplete.body.padding.hor = 0;
	stylePage_wifiConfigComplete.body.padding.inner = 0;	

	lv_style_copy(&styleImage_wifiConfigComplete, &lv_style_plain);
	styleImage_wifiConfigComplete.image.intense = LV_OPA_100;
	styleImage_wifiConfigComplete.image.color = LV_COLOR_MAKE(112, 146, 190);

	lv_style_copy(&styleLabelRef_wifiConfigComplete, &lv_style_plain);
	styleLabelRef_wifiConfigComplete.text.font = &lv_font_consola_17;
	styleLabelRef_wifiConfigComplete.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleText_menuLevel_B_infoTips, &lv_style_plain);
	styleText_menuLevel_B_infoTips.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B_infoTips.text.color = LV_COLOR_LIME;

	lv_style_copy(&styleText_menuLevel_B_infoMac, &lv_style_plain);
	styleText_menuLevel_B_infoMac.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B_infoMac.text.color = LV_COLOR_BLUE;
		
	lv_style_copy(&styleLabelRef_wifiCfg_methodSel_info, &lv_style_plain);
	styleLabelRef_wifiCfg_methodSel_info.text.font = &lv_font_consola_19;
	styleLabelRef_wifiCfg_methodSel_info.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleLabelRef_wifiCfg_methodSel_btnRef, &lv_style_plain);
	styleLabelRef_wifiCfg_methodSel_btnRef.text.font = &lv_font_consola_19;
	styleLabelRef_wifiCfg_methodSel_btnRef.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleLabelRef_wifiCfg_methodSel_btnMore, &lv_style_plain);
	styleLabelRef_wifiCfg_methodSel_btnMore.text.font = &lv_font_consola_16;
	styleLabelRef_wifiCfg_methodSel_btnMore.text.color = LV_COLOR_BLACK;

	lv_style_copy(&styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun.image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun.image.color = LV_COLOR_MAKE(241, 250, 252);

    lv_style_copy(&styleBtn_specialTransparent, &lv_style_btn_rel);
	styleBtn_specialTransparent.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.border.part = LV_BORDER_NONE;
    styleBtn_specialTransparent.body.opa = LV_OPA_TRANSP;
	styleBtn_specialTransparent.body.radius = 0;
    styleBtn_specialTransparent.body.shadow.width = 0;

	lv_style_copy(&styleMbox_bg, &lv_style_pretty);
	styleMbox_bg.body.main_color = LV_COLOR_MAKE(0, 128, 192);
	styleMbox_bg.body.grad_color = LV_COLOR_MAKE(0, 128, 192);
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
}


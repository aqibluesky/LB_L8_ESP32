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

#include "devDriver_manage.h"

#include "bussiness_timerSoft.h"

#include "gui_businessMenu_other.h"
#include "gui_businessHome.h"

LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);

LV_IMG_DECLARE(iconMenu_funBack_arrowLeft);
LV_IMG_DECLARE(iconMenu_funBack_homePage);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static const char strInfoWeek_tab[7][6] = {

	"Mon ", "Tues ", "Wed ", "Thur ", "Fri ", "Sat ", "Sun ",
};

static lv_obj_t *page_sysInfo = NULL;
static lv_obj_t *labelSysInfo_devTime = NULL;
static lv_obj_t *labelSysInfo_devSoftVersion = NULL;
static lv_obj_t *labelSysInfo_devReserveHeap = NULL;
static lv_obj_t *labelSysInfo_topicBssid = NULL;
static lv_obj_t *labelSysInfo_devMac = NULL;
static lv_obj_t *labelSysInfo_devParentMac = NULL;
static lv_obj_t *labelSysInfo_devParensRssi = NULL;
static lv_obj_t *labelSysInfo_devMeshNodeNum = NULL;
static lv_obj_t *labelSysInfo_devMeshLayer = NULL;
static lv_obj_t *labelSysInfo_devMutualGroupEnum = NULL;
static lv_obj_t *labelSysInfo_devTemprature = NULL;
static lv_obj_t *labelSysInfo_devPower = NULL;
static lv_obj_t *labelSysInfo_devElecSum = NULL;
static lv_obj_t *labelSysInfo_routerSSID = NULL;
static lv_obj_t *labelSysInfo_routerPSD = NULL;
static lv_obj_t *labelSysInfo_timeZone = NULL;
static lv_obj_t *labelSysInfo_versionNum = NULL;
static lv_obj_t *labelSysInfo_hwDeviceType = NULL;
static lv_obj_t *labelSysInfo_devNetworkInfo = NULL;

static lv_obj_t *objText_menuCurrentTitle = NULL;
static lv_obj_t *menuBtnChoIcon_fun_back = NULL;
static lv_obj_t *menuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_home = NULL;
static lv_obj_t *imgMenuBtnChoIcon_fun_back = NULL;

static lv_style_t labelStyle_sysInfo;
static lv_style_t stylePage_sysInfo;
static lv_style_t styleText_menuLevel_A;
static lv_style_t styleImg_menuFun_btnFun;
static lv_style_t styleBtn_specialTransparent;

static void currentGui_elementClear(void){

	lv_obj_del(page_sysInfo);
}

void guiDispTimeOut_pageOther(void){

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

		case LV_OBJ_FREENUM_BTNNUM_DEF_MENUBACK	:
		default:

			objImg_colorChg = imgMenuBtnChoIcon_fun_back;

		break;
	}

	lv_img_set_style(objImg_colorChg, &styleImg_menuFun_btnFun);
	lv_obj_refresh_style(objImg_colorChg);

	return LV_RES_OK;
}

void lvGuiOther_styleApplicationInit(void){

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_20;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	lv_style_copy(&labelStyle_sysInfo, &lv_style_plain);
	labelStyle_sysInfo.text.font = &lv_font_consola_17;

	lv_style_copy(&stylePage_sysInfo, &lv_style_plain);
	stylePage_sysInfo.body.main_color = LV_COLOR_WHITE;
	stylePage_sysInfo.body.grad_color = LV_COLOR_WHITE;

    lv_style_copy(&styleBtn_specialTransparent, &lv_style_btn_rel);
	styleBtn_specialTransparent.body.main_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.grad_color = LV_COLOR_TRANSP;
	styleBtn_specialTransparent.body.border.part = LV_BORDER_NONE;
    styleBtn_specialTransparent.body.opa = LV_OPA_TRANSP;
	styleBtn_specialTransparent.body.radius = 0;
    styleBtn_specialTransparent.body.shadow.width = 0;

	lv_style_copy(&styleImg_menuFun_btnFun, &lv_style_plain);
	styleImg_menuFun_btnFun.image.intense = LV_OPA_COVER;
	styleImg_menuFun_btnFun.image.color = LV_COLOR_MAKE(200, 191, 231);
}

void lvGuiOther_devInfoRefresh(void){

	stt_localTime 		devParamTemp_infoData 	= {0};
	uint8_t 			topicBssid[MWIFI_ADDR_LEN] = {0};
	uint8_t 			sta_mac[MWIFI_ADDR_LEN] = {0};
	mesh_addr_t 		parent_bssid        	= {0};
	mesh_assoc_t 		mesh_assoc 				= {0x0};
	uint8_t 			mutualGroupInsert_temp[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	wifi_config_t 		wifi_config				= {0};
	stt_timeZone 		timeZone_temp			= {0};
	stt_mqttCfgParam 	dtMqttParamTemp			= {0};

	const uint8_t textTempSize = 64;
	char textDisp_temp[128] = {0};

	const char *textTabAlign_ver = "\0";
	const char *textTabAlign_hor = "       ";
		  char *textTabAlign = NULL;

	(devStatusDispMethod_landscapeIf_get())?
		(textTabAlign = textTabAlign_hor):
		(textTabAlign = textTabAlign_ver);

	usrAppDevCurrentSystemTime_paramGet(&devParamTemp_infoData);
	devRouterConnectBssid_Get(topicBssid);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
	esp_mesh_get_parent_bssid(&parent_bssid);
	esp_wifi_vnd_mesh_get(&mesh_assoc);
	devMutualCtrlGroupInfo_groupInsertGet(mutualGroupInsert_temp);
	esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);
	deviceParamGet_timeZone(&timeZone_temp);
	mqttRemoteConnectCfg_paramGet(&dtMqttParamTemp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "System time: \n  #0080FF %s%04d/%02d/%02d -%s#\n#0090FF   %s%02d:%02d:%02d#\n", 
							textTabAlign,
							devParamTemp_infoData.time_Year,
							devParamTemp_infoData.time_Month,
							devParamTemp_infoData.time_Day,
							strInfoWeek_tab[devParamTemp_infoData.time_Week - 1],
							textTabAlign,
							devParamTemp_infoData.time_Hour,
							devParamTemp_infoData.time_Minute,
							devParamTemp_infoData.time_Second);
	lv_label_set_text(labelSysInfo_devTime, textDisp_temp);
	
	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Version: \n  #808080 %s"L8_DEVICE_VERSION_REF_DISCRIPTION"#", textTabAlign);
	lv_label_set_text(labelSysInfo_devSoftVersion, textDisp_temp);
	
	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Free heap: \n  #00A2E8 %s%d# Bytes", textTabAlign, esp_get_free_heap_size());
	lv_label_set_text(labelSysInfo_devReserveHeap, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Topic bssid: \n  #FF8027 %s%02X%02X%02X%02X%02X%02X#", textTabAlign, MAC2STR(topicBssid));
	lv_label_set_text(labelSysInfo_topicBssid, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Device mac: \n  #FF8027 %s"MACSTR"#", textTabAlign, MAC2STR(sta_mac));
	lv_label_set_text(labelSysInfo_devMac, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Parent bssid: \n  #FF8027 %s"MACSTR"#", textTabAlign, MAC2STR(parent_bssid.addr));
	lv_label_set_text(labelSysInfo_devParentMac, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Parent RSSI: \n  #8000FF %s%d# dB", textTabAlign, mesh_assoc.rssi);
	lv_label_set_text(labelSysInfo_devParensRssi, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Mesh node num: \n  #00A2E8 %s%d#", textTabAlign, esp_mesh_get_total_node_num());
	lv_label_set_text(labelSysInfo_devMeshNodeNum, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Mesh layer: \n  #B97A57 %s%d#", textTabAlign, esp_mesh_get_layer());
	lv_label_set_text(labelSysInfo_devMeshLayer, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Device mutual group: \n  #00FF00 %s[%d], [%d], [%d]#", textTabAlign, 
																				   mutualGroupInsert_temp[0],
																				   mutualGroupInsert_temp[1],
																				   mutualGroupInsert_temp[2]);
	lv_label_set_text(labelSysInfo_devMutualGroupEnum, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Environment temperature: \n  #408080 %s%.02f# \"C", textTabAlign, devDriverBussiness_temperatureMeasure_get());
	lv_label_set_text(labelSysInfo_devTemprature, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Device power: \n  #408080 %s%.01f# W", textTabAlign, devDriverBussiness_elecMeasure_valElecPowerGet());
	lv_label_set_text(labelSysInfo_devPower, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Electricity consumption: \n  #408080 %s%.01f# kWh", textTabAlign, devDriverBussiness_elecMeasure_valElecConsumGet());
	lv_label_set_text(labelSysInfo_devElecSum, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Router SSID: \n  #B5E61D %s%s#", textTabAlign, wifi_config.sta.ssid);
	lv_label_set_text(labelSysInfo_routerSSID, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Router password: \n  #B5E61D %s%s#", textTabAlign, wifi_config.sta.password);
	lv_label_set_text(labelSysInfo_routerPSD, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Time zone: \n  #808080 %sH:[%02d], M:[%02d]#", textTabAlign, timeZone_temp.timeZone_H, timeZone_temp.timeZone_M);
	lv_label_set_text(labelSysInfo_timeZone, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Version num: \n  #808080 %s%d -<hex:%02XH>#", textTabAlign, L8_DEVICE_VERSION, L8_DEVICE_VERSION);
	lv_label_set_text(labelSysInfo_versionNum, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Hardware type: \n  #808080 %s%d -<hex:%02XH>#", textTabAlign, L8_DEVICE_TYPE_PANEL_DEF, L8_DEVICE_TYPE_PANEL_DEF);
	lv_label_set_text(labelSysInfo_hwDeviceType, textDisp_temp);

	memset(textDisp_temp, 0, sizeof(char) * textTempSize);
	sprintf(textDisp_temp, "Network info: \n  #808080 %sip:%d.%d.%d.%d#\n  #808080 %sport[%d]#", textTabAlign, dtMqttParamTemp.ip_remote[0],
																								  	 		   dtMqttParamTemp.ip_remote[1],
																								  	 		   dtMqttParamTemp.ip_remote[2],
																								  	 		   dtMqttParamTemp.ip_remote[3],
																					   		   	 textTabAlign, dtMqttParamTemp.port_remote);
	lv_label_set_text(labelSysInfo_devNetworkInfo, textDisp_temp);
}

void lvGui_businessMenu_other(lv_obj_t * obj_Parent){

		  uint8_t textHorizontalDistance = 15;
	const uint8_t textVerticalDistance = 55;

	lvGuiOther_styleApplicationInit();

	(devStatusDispMethod_landscapeIf_get())?
		(textHorizontalDistance = 35):
		(textHorizontalDistance = 15);

	objText_menuCurrentTitle = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(objText_menuCurrentTitle, "system info");
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

	page_sysInfo = lv_page_create(lv_scr_act(), NULL);
	(devStatusDispMethod_landscapeIf_get())?
		(lv_obj_set_size(page_sysInfo, 320, 165)):
		(lv_obj_set_size(page_sysInfo, 240, 245));
	
	lv_obj_set_pos(page_sysInfo, 0, 75);
	lv_page_set_style(page_sysInfo, LV_PAGE_STYLE_SB, &stylePage_sysInfo);
	lv_page_set_style(page_sysInfo, LV_PAGE_STYLE_BG, &stylePage_sysInfo);
	lv_page_set_sb_mode(page_sysInfo, LV_SB_MODE_DRAG);
	lv_page_set_scrl_fit(page_sysInfo, false, false); //key opration
	if(devStatusDispMethod_landscapeIf_get()){
		
		lv_page_set_scrl_width(page_sysInfo, 300); 
		lv_page_set_scrl_height(page_sysInfo, 1100); 
	}
	else
	{
		lv_page_set_scrl_width(page_sysInfo, 220); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
		lv_page_set_scrl_height(page_sysInfo, 1140); //scrl尺寸必须大于set size尺寸 才可以进行拖拽
	}

	labelSysInfo_devTime = lv_label_create(page_sysInfo, NULL);
	lv_label_set_style(labelSysInfo_devTime, &labelStyle_sysInfo);
	lv_label_set_align(labelSysInfo_devTime, LV_LABEL_ALIGN_LEFT);
	lv_obj_set_size(labelSysInfo_devTime, 320, 40);
	lv_label_set_recolor(labelSysInfo_devTime, true);
	lv_obj_set_protect(labelSysInfo_devTime, LV_PROTECT_POS);
	lv_obj_align(labelSysInfo_devTime, 			page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 0 + 20);
	labelSysInfo_devSoftVersion = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devSoftVersion, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 2);
	labelSysInfo_devReserveHeap = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devReserveHeap, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 3);
	labelSysInfo_topicBssid = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_topicBssid, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 4);
	labelSysInfo_devMac = lv_label_create(page_sysInfo, 		labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devMac, 			page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 5);
	labelSysInfo_devParentMac = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devParentMac,		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 6);
	labelSysInfo_devParensRssi = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devParensRssi, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 7);
	labelSysInfo_devMeshNodeNum = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devMeshNodeNum, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 8);
	labelSysInfo_devMeshLayer = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devMeshLayer, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 9);
	labelSysInfo_devMutualGroupEnum = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devMutualGroupEnum, page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 10);
	labelSysInfo_devTemprature = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devTemprature, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 11);
	labelSysInfo_devPower = lv_label_create(page_sysInfo, 		labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devPower, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 12);
	labelSysInfo_devElecSum = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devElecSum, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 13);
	labelSysInfo_routerSSID = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_routerSSID, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 14);
	labelSysInfo_routerPSD = lv_label_create(page_sysInfo, 		labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_routerPSD, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 15);
	labelSysInfo_timeZone = lv_label_create(page_sysInfo, 		labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_timeZone, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 16);
	labelSysInfo_versionNum = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_versionNum, 		page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 17);
	labelSysInfo_hwDeviceType = lv_label_create(page_sysInfo, 	labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_hwDeviceType, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 18);
	labelSysInfo_devNetworkInfo = lv_label_create(page_sysInfo, labelSysInfo_devTime);
	lv_obj_align(labelSysInfo_devNetworkInfo, 	page_sysInfo, LV_ALIGN_IN_TOP_LEFT, textHorizontalDistance, textVerticalDistance * 19);
	
	lvGuiOther_devInfoRefresh();

	vTaskDelay(20 / portTICK_PERIOD_MS);
	lv_obj_refresh_style(page_sysInfo);
}


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

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessMenu_wifiConfig.h"
#include "gui_businessHome.h"

#include "lcd_adapter.h"
#include "QR_Encode.h"

#define QR_BASIC_POSITION_X			37
#define QR_BASIC_POSITION_Y			117
#define QR_PIXEL_SIZE				8

LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_17);

LV_IMG_DECLARE(iconMenu_wifiConfig);
LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

extern xQueueHandle msgQh_wifiConfigCompleteTips;

uint8_t wifiConfigComplete_tipsStartCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;

static lv_obj_t *menuText_devMac = NULL;
static lv_obj_t *pageTips_wifiConfigComplete = NULL;
static lv_obj_t *imageTips_wifiConfigComplete = NULL;
static lv_obj_t *labelTips_wifiConfigComplete = NULL;

static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B_infoMac;
static lv_style_t styleText_menuLevel_B_infoTips;
static lv_style_t stylePage_wifiConfigComplete;
static lv_style_t styleImage_wifiConfigComplete;
static lv_style_t styleLabelRef_wifiConfigComplete;

static void lvGuiStyleInit_businessMenu_wifiConfig(void);

static lv_res_t funCb_btnActionClick_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_del(menuText_devMac);
	lvGui_usrSwitch(bussinessType_Menu);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionPress_menuBtn_funBack(lv_obj_t *btn){

	lv_obj_t *btnFeedBk = lv_img_create(btn, NULL);
	lv_img_set_src(btnFeedBk, &imageBtn_feedBackNormal);

	return LV_RES_OK;
}

static void lvGui_businessMenu_wifiConfig_qrCode_refresh(void){

	uint8_t	qrCode_devMacBuff[6] = {0};
	char str_devMacBuff[30] = {0};

	esp_wifi_get_mac(ESP_IF_WIFI_STA, qrCode_devMacBuff);

	sprintf(str_devMacBuff, "%02X%02X%02X%02X%02X%02X,%d", qrCode_devMacBuff[0],
														   qrCode_devMacBuff[1],
														   qrCode_devMacBuff[2],
														   qrCode_devMacBuff[3],
														   qrCode_devMacBuff[4],
														   qrCode_devMacBuff[5],
														   (int)meshNetwork_connectReserve_IF_get());
	printf("Qr code creat res:%d.\n", EncodeData(str_devMacBuff));
//	printf("Qr code creat res:%d.\n", EncodeData("hellow, Lanbon!"));

	externSocket_ex_disp_fill(0, QR_BASIC_POSITION_Y, 240, 320, LV_COLOR_WHITE);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	for(uint8_t loopa = 0; loopa < MAX_MODULESIZE; loopa ++){

		for(uint8_t loopb = 0; loopb < MAX_MODULESIZE; loopb ++){

			(m_byModuleData[loopa][loopb])?
				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_BLACK)):
				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_WHITE));
		}
	}
}

void lvGui_businessMenu_wifiConfig(lv_obj_t * obj_Parent){

	lv_obj_t *text_Title;

	lv_obj_t * menuBtnChoIcon_fun_back;

	lv_obj_t * menuText_opTips;

	uint8_t	qrCode_devMacBuff[6] = {0};
	char str_devMacBuff[30] = {0};

	lvGuiStyleInit_businessMenu_wifiConfig();

	esp_wifi_get_mac(ESP_IF_WIFI_STA, qrCode_devMacBuff);

	menuText_devMac = lv_label_create(lv_scr_act(), NULL);

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_30;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "wifi config");
	lv_obj_set_pos(text_Title, 60, 35);
	lv_obj_set_style(text_Title, &styleText_menuLevel_A);

	lv_style_copy(&styleText_menuLevel_B_infoTips, &lv_style_plain);
	styleText_menuLevel_B_infoTips.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B_infoTips.text.color = LV_COLOR_LIME;
	
	menuText_opTips = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(menuText_opTips, "scan the QR code to config the WIFI.");
	lv_label_set_long_mode(menuText_opTips, LV_LABEL_LONG_SCROLL);
	lv_obj_set_style(menuText_opTips, &styleText_menuLevel_B_infoTips);
	lv_obj_set_protect(menuText_opTips, LV_PROTECT_POS);
	lv_obj_align(menuText_opTips, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

	menuBtnChoIcon_fun_back = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_REL, &iconMenu_funBack);
	lv_imgbtn_set_src(menuBtnChoIcon_fun_back, LV_BTN_STATE_PR, &iconMenu_funBack);
	lv_obj_set_pos(menuBtnChoIcon_fun_back, 8, 30);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_CLICK, funCb_btnActionClick_menuBtn_funBack);
	lv_btn_set_action(menuBtnChoIcon_fun_back, LV_BTN_ACTION_PR, funCb_btnActionPress_menuBtn_funBack);

	lvGui_businessMenu_wifiConfig_qrCode_refresh();

	lv_style_copy(&styleText_menuLevel_B_infoMac, &lv_style_plain);
	styleText_menuLevel_B_infoMac.text.font = &lv_font_dejavu_15;
	styleText_menuLevel_B_infoMac.text.color = LV_COLOR_BLUE;

	menuText_devMac = lv_label_create(lv_scr_act(), NULL);
	sprintf(str_devMacBuff, "devMac: %02X %02X %02X %02X %02X %02X", qrCode_devMacBuff[0],
																	 qrCode_devMacBuff[1],
																	 qrCode_devMacBuff[2],
																	 qrCode_devMacBuff[3],
																	 qrCode_devMacBuff[4],
																	 qrCode_devMacBuff[5]);
	lv_label_set_text(menuText_devMac, (const char *)str_devMacBuff);
	lv_obj_set_style(menuText_devMac, &styleText_menuLevel_B_infoMac);
	lv_obj_set_pos(menuText_devMac, 20, 300);
}

void lvGui_wifiConfig_bussiness_configComplete_tipsTrig(void){

	uint8_t msgQh_sptrDataWifiConfig = 'A';

	xQueueSend(msgQh_wifiConfigCompleteTips, &msgQh_sptrDataWifiConfig, 1 / portTICK_PERIOD_MS);
	wifiConfigComplete_tipsStartCounter = 3;
}

void lvGui_wifiConfig_bussiness_configComplete_tipsOver(void){

	uint8_t msgQh_sptrDataWifiConfig = 'B';

	xQueueSend(msgQh_wifiConfigCompleteTips, &msgQh_sptrDataWifiConfig, 1 / portTICK_PERIOD_MS);
}

void lvGui_wifiConfig_bussiness_configComplete_tipsDetect(void){

	uint8_t msgQh_rptrDataWifiConfig = 0;
	static bool devNetworkReserve_flg = false;

	if(meshNetwork_connectReserve_IF_get()){

		if(!devNetworkReserve_flg){

			lvGui_businessMenu_wifiConfig_qrCode_refresh();			
			devNetworkReserve_flg = true;
		}
	}
	else
	{
		if(devNetworkReserve_flg)
			devNetworkReserve_flg = false;
	}

	if(xQueueReceive(msgQh_wifiConfigCompleteTips, &msgQh_rptrDataWifiConfig, 1 / portTICK_RATE_MS) == pdTRUE){

	 	if(msgQh_rptrDataWifiConfig == 'A'){

			if(pageTips_wifiConfigComplete == NULL){

				pageTips_wifiConfigComplete = lv_page_create(lv_scr_act(), NULL);
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
		}
		else
		if(msgQh_rptrDataWifiConfig == 'B'){
		
			if(pageTips_wifiConfigComplete){

				lv_obj_del(pageTips_wifiConfigComplete);
				pageTips_wifiConfigComplete = NULL;
			}
			lv_obj_del(menuText_devMac);
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
}


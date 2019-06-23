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

LV_IMG_DECLARE(iconMenu_funBack);
LV_IMG_DECLARE(imageBtn_feedBackNormal);

static lv_style_t styleText_menuLevel_A;
static lv_style_t styleText_menuLevel_B_infoMac;
static lv_style_t styleText_menuLevel_B_infoTips;

static lv_obj_t * menuText_devMac;

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

void lvGui_businessMenu_wifiConfig(lv_obj_t * obj_Parent){

	lv_obj_t *text_Title;

	lv_obj_t * menuBtnChoIcon_fun_back;

	lv_obj_t * menuText_opTips;

	uint8_t	qrCode_devMacBuff[6] = {0};
	char str_devMacBuff[30] = {0};

	esp_wifi_get_mac(ESP_IF_WIFI_STA, qrCode_devMacBuff);

//	printf("Qr code creat res:%d.\n", EncodeData("hellow, Lanbon!"));

	menuText_devMac = lv_label_create(lv_scr_act(), NULL);

	sprintf(str_devMacBuff, "%02X%02X%02X%02X%02X%02X,%d", qrCode_devMacBuff[0],
														   qrCode_devMacBuff[1],
														   qrCode_devMacBuff[2],
														   qrCode_devMacBuff[3],
														   qrCode_devMacBuff[4],
														   qrCode_devMacBuff[5],
														   (int)flgGet_gotRouterOrMeshConnect());
	printf("Qr code creat res:%d.\n", EncodeData(str_devMacBuff));

	lv_style_copy(&styleText_menuLevel_A, &lv_style_plain);
	styleText_menuLevel_A.text.font = &lv_font_dejavu_30;
	styleText_menuLevel_A.text.color = LV_COLOR_WHITE;

	text_Title = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(text_Title, "Device info");
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

	externSocket_ex_disp_fill(0, QR_BASIC_POSITION_Y, 240, 320, LV_COLOR_WHITE);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	for(uint8_t loopa = 0; loopa < MAX_MODULESIZE; loopa ++){

		for(uint8_t loopb = 0; loopb < MAX_MODULESIZE; loopb ++){

			(m_byModuleData[loopa][loopb])?
				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_BLACK)):
				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_WHITE));
		}
	}

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



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

#include "os.h"

/* lvgl includes */
#include "iot_lvgl.h"

#include "gui_businessHome.h"

#include "gui_businessMenu.h"
#include "gui_businessMenu_deviceInfo.h"
#include "gui_businessMenu_deviceManage.h"
#include "gui_businessMenu_scence.h"
#include "gui_businessMenu_security.h"
#include "gui_businessMenu_setting.h"
#include "gui_businessMenu_setting_A.h"
#include "gui_businessMenu_setting_B.h"
#include "gui_businessMenu_setting_C.h"
#include "gui_businessMenu_statistics.h"

#include "devDataManage.h"
#include "devDriver_manage.h"

#include "bussiness_timerSoft.h"

typedef void (*guiHomeFun_local_bussiness)(lv_obj_t * obj_Parent);

LV_FONT_DECLARE(lv_font_dejavu_30);
LV_FONT_DECLARE(lv_font_dejavu_15);
LV_FONT_DECLARE(lv_font_consola_13);
LV_FONT_DECLARE(lv_font_consola_16);
LV_FONT_DECLARE(lv_font_arial_12);
LV_FONT_DECLARE(lv_font_arial_15);
LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);
//LV_FONT_DECLARE(lv_font_arabic_20);
LV_FONT_DECLARE(lv_font_arabic_17);
//LV_FONT_DECLARE(lv_font_arabic_15);
//LV_FONT_DECLARE(lv_font_hebrew_20);
LV_FONT_DECLARE(lv_font_hebrew_17);
//LV_FONT_DECLARE(lv_font_hebrew_15);

LV_IMG_DECLARE(testPic);
LV_IMG_DECLARE(testPic_P);
LV_IMG_DECLARE(ttA);
LV_IMG_DECLARE(night_Sight);
LV_IMG_DECLARE(ll_longxia);
LV_IMG_DECLARE(homepage_bkPic_danYa);
LV_IMG_DECLARE(homepage_bkPic_jiJian);
LV_IMG_DECLARE(homepage_bkPic_jianJie);
LV_IMG_DECLARE(homepage_bkPic_keAi);
LV_IMG_DECLARE(homepage_bkPic_ouZhou);

LV_IMG_DECLARE(iconHeader_wifi_A);
LV_IMG_DECLARE(iconHeader_wifi_B);
LV_IMG_DECLARE(iconHeader_wifi_C);
LV_IMG_DECLARE(iconHeader_wifi_D);
LV_IMG_DECLARE(iconHeader_alarm);
LV_IMG_DECLARE(iconHeader_greenMode);
LV_IMG_DECLARE(iconHeader_node);
LV_IMG_DECLARE(iconHeader_elec);
LV_IMG_DECLARE(iconAlarm_tips);
LV_IMG_DECLARE(button_aera);
LV_IMG_DECLARE(button_area);
LV_IMG_DECLARE(homepage_buttonPicRel_fenSe);
LV_IMG_DECLARE(homepage_buttonPicPre_fenSe);
LV_IMG_DECLARE(homepage_buttonPicRel_jianJie);
LV_IMG_DECLARE(homepage_buttonPicPre_jianJie);
LV_IMG_DECLARE(homepage_buttonPicRel_ouZhou);
LV_IMG_DECLARE(homepage_buttonPicPre_ouZhou);
LV_IMG_DECLARE(homepage_buttonPicRel_jiJian);
LV_IMG_DECLARE(homepage_buttonPicPre_jiJian);
LV_IMG_DECLARE(iconHome_menu);
LV_IMG_DECLARE(btnIconHome_meeting);
LV_IMG_DECLARE(btnIconHome_sleeping);
LV_IMG_DECLARE(btnIconHome_toilet);
LV_IMG_DECLARE(iconPage_curtainClose_pre);
LV_IMG_DECLARE(iconPage_curtainClose_rel);
LV_IMG_DECLARE(iconPage_curtainPluse_pre);
LV_IMG_DECLARE(iconPage_curtainPluse_rel);
LV_IMG_DECLARE(iconPage_curtainOpen_pre);
LV_IMG_DECLARE(iconPage_curtainOpen_rel);
LV_IMG_DECLARE(iconPage_binding);
LV_IMG_DECLARE(iconPage_unbinding);
LV_IMG_DECLARE(imageCurtain_gan);
LV_IMG_DECLARE(imageCurtain_body);

LV_IMG_DECLARE(iconLightA_HomePageDeviceDimmer);
LV_IMG_DECLARE(iconLightB_HomePageDeviceDimmer);

//业务变量
extern EventGroupHandle_t xEventGp_tipsLoopTimer;
extern uint8_t devRunningTimeFromPowerUp_couter;

EventGroupHandle_t xEventGp_screenTouch = NULL;
stt_touchEveInfo devTouchGetInfo = {0};

uint16_t ctrlObj_slidingCalmDownCounter = 0; //界面控件滑动冷却计时变量

static struct
{
    lv_img_header_t header;
    uint32_t data_size;
    uint8_t * data;
	
}testImg_data = {

	.header.always_zero = 0,
	.header.w = 120,
	.header.h = 20,
	.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
	.data_size = GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE,
	.data = NULL,
};

static lv_obj_t *imageBK;

static lv_obj_t *imageTips_timer;

static usrGuiBussiness_type guiPage_current = bussinessType_Home;
static usrGuiBussiness_type guiPage_record = bussinessType_Home;

//home界面页眉显示信息对象
static lv_obj_t *iconHeaderObj_wifi = NULL;
static lv_obj_t *iconHeaderObj_alarm = NULL;
static lv_obj_t *iconHeaderObj_greenMode = NULL;

static lv_obj_t *textHeaderObj_time = NULL;
static lv_obj_t *textHeaderObj_elec = NULL;
static lv_obj_t *textHeaderObj_meshNodeNum = NULL;
static lv_obj_t *textHeaderObj_meshRole = NULL;
static lv_obj_t *textHeaderObj_temperature = NULL;

static lv_obj_t *text_loopTimerTips = NULL;
static lv_obj_t *icon_loopTimerTips = NULL;
static bool		trigFlg_loopTimerTips = false;

static lv_obj_t *label_bk_devDimmer = NULL;
static lv_obj_t *label_bk_devHeater_timeRemind = NULL;
static lv_obj_t *label_bk_devCurtain_positionCur = NULL;
static lv_obj_t *label_bk_devCurtain_positionAdj = NULL;

//home界面开关按键对象
static lv_obj_t *btn_bk_devMulitSw_A = NULL;
static lv_obj_t *btn_bk_devMulitSw_B = NULL;
static lv_obj_t *btn_bk_devMulitSw_C = NULL;
static lv_obj_t *iconBtn_meeting = NULL;
static lv_obj_t *textBtn_meeting = NULL;
static lv_obj_t *iconBtn_sleeping = NULL;
static lv_obj_t *textBtn_sleeping = NULL;
static lv_obj_t *iconBtn_toilet = NULL;
static lv_obj_t *textBtn_toilet = NULL;
static lv_obj_t *iconBtn_binding_A = NULL;
static lv_obj_t *iconBtn_binding_B = NULL;
static lv_obj_t *iconBtn_binding_C = NULL;
static lv_obj_t *slider_bk_devDimmer = NULL;
static lv_obj_t *photoA_bk_devDimmer = NULL;
static lv_obj_t *photoB_bk_devDimmer = NULL;
static lv_obj_t *btn_bk_devCurtain_open = NULL;
static lv_obj_t *btn_bk_devCurtain_stop = NULL;
static lv_obj_t *btn_bk_devCurtain_close = NULL;
static lv_obj_t *cont_bk_devCurtainImage = NULL;
static lv_obj_t *image_bk_devCurtain_gan = NULL;
static lv_obj_t *image_bk_devCurtain_bodyLeft = NULL;
static lv_obj_t *image_bk_devCurtain_bodyRight = NULL;
static lv_obj_t *slider_bk_devCurtain = NULL;
static lv_obj_t *slider_bk_devCurtainPosTips = NULL;
static lv_obj_t *roller_bk_devFans = NULL;
static lv_obj_t *roller_bk_devHeater = NULL;
static lv_obj_t *btnm_bk_devHeater = NULL;

//home界面进入菜单按键对象
static lv_obj_t *btn_homeMenu = NULL;

//home界面相关对象风格对象
static lv_style_t styleText_elecAnodenum;
static lv_style_t styleText_temperature;
static lv_style_t styleText_loopTimerTips;
static lv_style_t styleText_time;
static lv_style_t styleBtn_Text;
static lv_style_t styleBk_secMenu;
static lv_style_t styleBtn_devMulitSw_statusOn;
static lv_style_t styleBtn_devMulitSw_statusOff;
static lv_style_t styleIconvText_devMulitSw_statusOn;
static lv_style_t styleIconvText_devMulitSw_statusOff;
static lv_style_t styleBtnImg_icon;
static lv_style_t styleSliderBk_devDimmer_bg;
static lv_style_t styleSliderBk_devDimmer_indic;
static lv_style_t styleSliderBk_devDimmer_knob;
static lv_style_t stylePhotoAk_devDimmer;
static lv_style_t stylePhotoBk_devDimmer;
static lv_style_t styleText_devDimmer_SliderBk;
static lv_style_t styleText_devHeater_Bk_timeRemind;
static lv_style_t styleText_devCurtain_Bk_positionTips;
static lv_style_t styleBtn_devCurtain_statusPre;
static lv_style_t styleBtn_devCurtain_statusRel;
static lv_style_t styleSliderBk_devCurtain_bg;
static lv_style_t styleSliderBk_devCurtain_indic;
static lv_style_t styleSliderBk_devCurtain_knob;
static lv_style_t styleRollerBk_devFans_bg;
static lv_style_t styleRollerBk_devFans_sel;
static lv_style_t styleRollerBk_devHeater_bg;
static lv_style_t styleRollerBk_devHeater_sel;
static lv_style_t styleText_devCurtain_SliderBk;

//其他本地变量
static uint8_t homepageThemeType_typeFlg = homepageThemeType_ouZhou;

static char textStr_time[10] = {0};
static char textStr_nodeNum[5] = {0}; 
static char textStr_elecSum[20] = {0};
static char textStr_temperature[10] = {0}; 

static char str_sliderBkVal_devDimmer[20] = {0};
static char str_devRunningRemind_devHeater[60] = {0};
static char str_devParamPositionCur_devCurtain[25] = {0};
static char str_devParamPositionAdj_devCurtain[25] = {0};

static uint8_t btnBindingStatus_record[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0}; //按钮绑定图标本地比较记录缓存
static bool mutualCtrlTrigIf_A = false; 
static bool mutualCtrlTrigIf_B = false;
static bool mutualCtrlTrigIf_C = false;

static const char *btnm_str_devHeater[DEVICE_HEATER_OPREAT_ACTION_NUM + 1] = {"X", "A", "B", "C", "D", ""};

//不同类型开关所需的参照常量
static const struct _rollerDispConferenceTab_devFans{

	stt_devFans_opratAct opreatActCurrent;
	uint16_t optionSelect;

}objDevFans_rollerDispConferenceTab[DEVICE_CURTAIN_OPREAT_ACTION_NUM] = { //风扇开关不同档位对应ui显示值参照表

	{fansOpreatAct_stop,		0},
	{fansOpreatAct_firstGear,	1},
	{fansOpreatAct_secondGear,	2},
	{fansOpreatAct_thirdGear,	3},
};

static const struct _rollerDispConferenceTab_devHeater{

	stt_devHeater_opratAct opreatActCurrent;
	uint16_t optionSelect;

}objDevHeater_rollerDispConferenceTab[DEVICE_HEATER_OPREAT_ACTION_NUM] = { //风扇开关不同档位对应ui显示值参照表

	{heaterOpreatAct_close,					0},
	{heaterOpreatAct_open,					1},
	{heaterOpreatAct_closeAfter30Min,		2},
	{heaterOpreatAct_closeAfter60Min,		3},
	{heaterOpreatAct_closeAfterTimeCustom,	4},
};

static void usrApp_ctrlObjSlidlingTrig(void){

	ctrlObj_slidingCalmDownCounter = 1500;
}

void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF){

	homepageThemeType_typeFlg = themeType_flg;

	switch(homepageThemeType_typeFlg){

		case homepageThemeType_jianJie:{

			styleIconvText_devMulitSw_statusOn.image.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOn.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOn.text.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOff.image.color = LV_COLOR_MAKE(166, 126, 79);
			styleIconvText_devMulitSw_statusOff.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOff.text.color = LV_COLOR_MAKE(166, 126, 79);

		}break;
		
		case homepageThemeType_keAi:{

			styleIconvText_devMulitSw_statusOn.image.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOn.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOn.text.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOff.image.color = LV_COLOR_MAKE(204, 151, 150);
			styleIconvText_devMulitSw_statusOff.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOff.text.color = LV_COLOR_MAKE(204, 151, 150);

		}break;
		
		default:{

			styleIconvText_devMulitSw_statusOn.image.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOn.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOn.text.color = LV_COLOR_WHITE;
			styleIconvText_devMulitSw_statusOff.image.color = LV_COLOR_BLACK;
			styleIconvText_devMulitSw_statusOff.image.intense = LV_OPA_COVER;
			styleIconvText_devMulitSw_statusOff.text.color = LV_COLOR_BLACK;

		}break;
	}

	styleIconvText_devMulitSw_statusOn.text.font = styleBtn_Text.text.font;
	styleIconvText_devMulitSw_statusOff.text.font = styleBtn_Text.text.font;

	stt_msgDats_dataManagementHandle sptr_msgQ_dmHandle = {0};
	sptr_msgQ_dmHandle.msgType = dataManagement_msgType_homePageThemeTypeChg;
	sptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpThemeTypeChg.themeTypeChg_notice = 1;
	xQueueSend(msgQh_dataManagementHandle, &sptr_msgQ_dmHandle, 1 / portTICK_PERIOD_MS);

	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_themeType, &homepageThemeType_typeFlg);
	}
}

uint8_t usrAppHomepageThemeType_Get(void){

	return homepageThemeType_typeFlg;
}

static lv_img_dsc_t *usrAppHomepageBkPic_dataGet(void){

	const lv_img_dsc_t *res = NULL;

	switch(homepageThemeType_typeFlg){

		case homepageThemeType_jianJie:{res = &homepage_bkPic_jianJie;}break;
		case homepageThemeType_keAi:{res = &homepage_bkPic_keAi;}break;
		case homepageThemeType_ouZhou:{res = &homepage_bkPic_jiJian;}break;
		default:{res = &homepage_bkPic_jianJie;}break;
	}

	return res;
}

static lv_img_dsc_t *usrAppHomepageBtnBkPic_dataGet(bool pic_preIf){

	const lv_img_dsc_t *res = NULL;

	switch(homepageThemeType_typeFlg){
		
		case homepageThemeType_jianJie:{

			(pic_preIf)?(res = &homepage_buttonPicPre_jianJie):(res = &homepage_buttonPicRel_jianJie);

		}break;
		
		case homepageThemeType_keAi:{

			(pic_preIf)?(res = &homepage_buttonPicPre_fenSe):(res = &homepage_buttonPicRel_fenSe);

		}break;
		
		case homepageThemeType_ouZhou:{

			(pic_preIf)?(res = &homepage_buttonPicPre_jiJian):(res = &homepage_buttonPicRel_jiJian);

		}break;
		
		default:{

			(pic_preIf)?(res = &homepage_buttonPicPre_jianJie):(res = &homepage_buttonPicRel_jianJie);

		}break;
	}

	return res;
}

static lv_font_t *usrAppHomepageBtnBkText_fontGet(uint8_t countryFlg){

	lv_font_t *res = NULL;

	switch(countryFlg){
		
		case countryT_Arabic:
			
				res = &lv_font_arabic_17;
				break;
		
		case countryT_Hebrew:

				res = &lv_font_hebrew_17;
				break;

		case countryT_EnglishSerail:
		default:

				res = &lv_font_consola_17;
				break;
	}

	return res;
}

static lv_res_t funCb_btnActionClick_devMulitSw_mainBtnA(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devAtmosphere_statusTips_trigSet(atmosphereLightType_dataSaveOpreat);

	currentDev_dataPointGet(&devDataPoint);

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:{

			devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1 = !devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_A, true);
//			(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;

		case devTypeDef_mulitSwTwoBit:{

			devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1 = !devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_A, true);
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;

		case devTypeDef_mulitSwThreeBit:{

			devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1 = !devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_A, true);
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devMulitSw_mainBtnB(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwTwoBit:{
		
			devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2 = !devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_B, true);
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;
		
		case devTypeDef_mulitSwThreeBit:{
		
			devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2 = !devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_B, true);
			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devMulitSw_mainBtnC(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	switch(currentDev_typeGet()){
		
		case devTypeDef_mulitSwThreeBit:{
		
			devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3 = !devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3;
			currentDev_dataPointSet(&devDataPoint, true, mutualCtrlTrigIf_C, true);
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOff));
//			(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//				(lv_imgbtn_set_style(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_open(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 1;

	currentDev_dataPointSet(&devDataPoint, false, true, true);

	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_stop(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 2;

	currentDev_dataPointSet(&devDataPoint, false, true, true);

	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_close(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 4;

	currentDev_dataPointSet(&devDataPoint, false, true, true);

	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_bindingBtnA(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	switch(currentDev_typeGet()){
		
		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:{

			mutualCtrlTrigIf_A = !mutualCtrlTrigIf_A;
			(mutualCtrlTrigIf_A)?
				(lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding)):
				(lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_unbinding));

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_bindingBtnB(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:{

			mutualCtrlTrigIf_B = !mutualCtrlTrigIf_B;
			(mutualCtrlTrigIf_B)?
				(lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_REL, &iconPage_binding)):
				(lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_REL, &iconPage_unbinding));

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_bindingBtnC(lv_obj_t *btn){

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwThreeBit:{
		
			mutualCtrlTrigIf_C = !mutualCtrlTrigIf_C;
			(mutualCtrlTrigIf_C)?
				(lv_imgbtn_set_src(iconBtn_binding_C, LV_BTN_STATE_REL, &iconPage_binding)):
				(lv_imgbtn_set_src(iconBtn_binding_C, LV_BTN_STATE_REL, &iconPage_unbinding));
		
		}break;
		
		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_rollAction_devFans_mainRoller(lv_obj_t *roller){

	uint16_t optionRollerSelect = lv_roller_get_selected(roller);

	for(uint8_t loop = 0; loop < DEVICE_CURTAIN_OPREAT_ACTION_NUM; loop ++){

		if(optionRollerSelect == objDevFans_rollerDispConferenceTab[loop].optionSelect){

			stt_devDataPonitTypedef devDataPoint = {0};

			devDataPoint.devType_fans.devFans_swEnumVal = objDevFans_rollerDispConferenceTab[loop].opreatActCurrent;

			currentDev_dataPointSet(&devDataPoint, true, true, true);	
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_rollAction_devHeater_mainRoller(lv_obj_t *roller){

	uint16_t optionRollerSelect = lv_roller_get_selected(roller);

	for(uint8_t loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM; loop ++){

		if(optionRollerSelect == objDevHeater_rollerDispConferenceTab[loop].optionSelect){

			stt_devDataPonitTypedef devDataPoint = {0};

			devDataPoint.devType_heater.devHeater_swEnumVal = objDevHeater_rollerDispConferenceTab[loop].opreatActCurrent;

			currentDev_dataPointSet(&devDataPoint, true, true, true);	
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnmActionClick_devHeater_gearBtnm(lv_obj_t *btnm, const char *txt){

	for(uint8_t loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM + 1; loop ++){

		if(!strcmp(txt, btnm_str_devHeater[loop])){

			printf("btn %s has be selected.\n", txt);
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_devDimmer_mainSlider(lv_obj_t *slider){

	stt_devDataPonitTypedef devDataPoint = {0};

	usrApp_ctrlObjSlidlingTrig(); //滑动冷却触发

	devDataPoint.devType_dimmer.devDimmer_brightnessVal = lv_slider_get_value(slider);

	if(lv_slider_get_value(slider)){

		stylePhotoBk_devDimmer.image.opa = ((uint8_t)lv_slider_get_value(slider)) * 2 + 50;
	}
	else
	{
		stylePhotoBk_devDimmer.image.opa = 0;
	}
	lv_img_set_style(photoB_bk_devDimmer, &stylePhotoBk_devDimmer);
	stylePhotoAk_devDimmer.image.intense = 100 - ((uint8_t)lv_slider_get_value(slider));
	lv_img_set_style(photoA_bk_devDimmer, &stylePhotoAk_devDimmer);

	sprintf(str_sliderBkVal_devDimmer, "Brightness:%d%%", lv_slider_get_value(slider));
	lv_label_set_text(label_bk_devDimmer, str_sliderBkVal_devDimmer);
	currentDev_dataPointSet(&devDataPoint, true, true, true);

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_devCurtain_mainSlider(lv_obj_t *slider){

	uint8_t orbitalTimePercent = (uint8_t)lv_slider_get_value(slider);

	usrApp_ctrlObjSlidlingTrig(); //滑动冷却触发

	devDriverBussiness_curtainSwitch_periphStatusRealesBySlide(orbitalTimePercent);
	sprintf(str_devParamPositionAdj_devCurtain, "position adjust:%d%%.", orbitalTimePercent);
	lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_devCurtainPosTips_mainSlider(lv_obj_t *slider){

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_homeMenu_click(lv_obj_t *btn){

	lvGui_usrSwitch(bussinessType_Menu);

	return LV_RES_OK;
}

static void pageHeader_infoRefreshLoop(void){

	if(devRunningTimeFromPowerUp_couter < 2)return;

	{//页眉时间信息更新

		stt_localTime devSystime_temp = {0};
		static uint8_t localRecord_min = 0;

		usrAppDevCurrentSystemTime_paramGet(&devSystime_temp);

		if(localRecord_min != devSystime_temp.time_Minute){

			localRecord_min = devSystime_temp.time_Minute;
			
			sprintf(textStr_time, "%02d:%02d", devSystime_temp.time_Hour, 
											   devSystime_temp.time_Minute);
			lv_label_set_text(textHeaderObj_time, textStr_time);
		}
	}

	{//页眉定时类图标提示更新

			   uint16_t devRunningFlg_temp 		  	  = currentDevRunningFlg_paramGet();
			   bool 	tipsAlarm_remindIf 		  	  = false;
			   bool 	tipsgreenMode_remindIf	  	  = false;
		static bool 	tipsAlarm_remindIf_record 	  = false;
		static bool 	tipsgreenMode_remindIf_record = false;

		if((devRunningFlg_temp & DEV_RUNNING_FLG_BIT_TIMER)||
		   (devRunningFlg_temp & DEV_RUNNING_FLG_BIT_DELAY)){

			tipsAlarm_remindIf = true;
		}

		if(tipsAlarm_remindIf_record != tipsAlarm_remindIf){

			tipsAlarm_remindIf_record = tipsAlarm_remindIf;

			if(tipsAlarm_remindIf_record){

				if(iconHeaderObj_alarm == NULL)iconHeaderObj_alarm = lv_img_create(lv_layer_top(), NULL);
				lv_obj_set_pos(iconHeaderObj_alarm,	60, 1);
				lv_img_set_src(iconHeaderObj_alarm, &iconHeader_alarm);
			}
			else
			{
				lv_obj_del(iconHeaderObj_alarm);
				iconHeaderObj_alarm = NULL;
			}
		}

		if(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_GREENMODE){

			tipsgreenMode_remindIf = true;
		}

		if(tipsgreenMode_remindIf_record != tipsgreenMode_remindIf){

			tipsgreenMode_remindIf_record = tipsgreenMode_remindIf;

			if(tipsgreenMode_remindIf_record){

				if(iconHeaderObj_greenMode == NULL)iconHeaderObj_greenMode = lv_img_create(lv_layer_top(), NULL);
				lv_obj_set_pos(iconHeaderObj_greenMode,	133, 1);
				lv_img_set_src(iconHeaderObj_greenMode, &iconHeader_greenMode);
			}
			else
			{
				lv_obj_del(iconHeaderObj_greenMode);
				iconHeaderObj_greenMode = NULL;
			}
		}
	}

	{//页眉信号强度提示更新

		int8_t signalStrength_temp = devMeshSignalFromParentVal_Get();
		static int8_t signalStrength_record = 0;
		const int8_t signalStrength_levelTab[3] = {-70, -90, -120};

		if(signalStrength_record != signalStrength_temp){

			signalStrength_record = signalStrength_temp;

			if(signalStrength_record > signalStrength_levelTab[0]){

				lv_img_set_src(iconHeaderObj_wifi, &iconHeader_wifi_A);
			}
			else
			if((signalStrength_record > signalStrength_levelTab[1]) &&
			   (signalStrength_record < signalStrength_levelTab[0])){

				lv_img_set_src(iconHeaderObj_wifi, &iconHeader_wifi_B);
			}
			else
			if((signalStrength_record > signalStrength_levelTab[2]) &&
			   (signalStrength_record < signalStrength_levelTab[1])){
			
				lv_img_set_src(iconHeaderObj_wifi, &iconHeader_wifi_C);
			}
			else
			if(signalStrength_record < signalStrength_levelTab[2]){

				lv_img_set_src(iconHeaderObj_wifi, &iconHeader_wifi_D);
			}	
		}
	}

	{//页眉节点数量显示更新

		enum _sEnum_devMeshRole{

			roleStatus_noConnect = 0,
			roleStatus_root,
			roleStatus_node,
		};

		uint16_t nodeNum_temp = devMeshNodeNum_Get();
		static uint16_t nodeNum_record = 0;
		uint8_t devMesh_statusRole_temp = roleStatus_noConnect;
		static uint8_t devMesh_statusRole_record = roleStatus_noConnect;

		if(nodeNum_record != nodeNum_temp){

			nodeNum_record = nodeNum_temp;

			sprintf(textStr_nodeNum, "%02d", nodeNum_record);
			lv_label_set_text(textHeaderObj_meshNodeNum, textStr_nodeNum);			
		}

		if(mwifi_is_connected()){

			if(esp_mesh_get_layer() == MESH_ROOT){

				devMesh_statusRole_temp = roleStatus_root;
			}
			else
			{
				devMesh_statusRole_temp = roleStatus_node;
			}
		}
		else
		{
			devMesh_statusRole_temp = roleStatus_noConnect;
		}
		if(devMesh_statusRole_record != devMesh_statusRole_temp){

			devMesh_statusRole_record = devMesh_statusRole_temp;
			
			switch(devMesh_statusRole_record){

				case roleStatus_root:lv_label_set_text(textHeaderObj_meshRole, "m");break;
				case roleStatus_node:lv_label_set_text(textHeaderObj_meshRole, "s");break;
				case roleStatus_noConnect:
				default:lv_label_set_text(textHeaderObj_meshRole, "x");break;
			}
		}
	}

	{//页眉电量显示更新

		float elecSum_temp = devDriverBussiness_elecMeasure_valElecConsumGet();
		static float elecSum_record = 0.0F;

		if(elecSum_record != elecSum_temp){

			elecSum_record = elecSum_temp;

			if(elecSum_record < 1.0F){

				sprintf(textStr_elecSum, "%.04fkWh", elecSum_record);
			}
			else
			if(elecSum_record > 1.0F && elecSum_record < 10.0F){

				sprintf(textStr_elecSum, "%.03fkWh", elecSum_record);
			}
			else
			if(elecSum_record > 10.0F && elecSum_record < 100.0F){

				sprintf(textStr_elecSum, "%.02fkWh", elecSum_record);
			}
			else
			if(elecSum_record > 100.0F && elecSum_record < 1000.0F){

				sprintf(textStr_elecSum, "%.02fkWh", elecSum_record);
			}
			else{

				sprintf(textStr_elecSum, "%.01fkWh", elecSum_record);
			}

			lv_label_set_text(textHeaderObj_elec, textStr_elecSum);	
		}
	}

	{//页眉温度显示更新

		float temperature_temp = devDriverBussiness_temperatureMeasure_get();
		static float temperature_record = 0.0F;

		if(temperature_record != temperature_temp){

			temperature_record = temperature_temp;

			sprintf(textStr_temperature, "%3.02f\"C", temperature_record);
			lv_label_set_text(textHeaderObj_temperature, textStr_temperature);	
		}
	}
}

static void pageActivity_infoRefreshLoop(void){

	switch(guiPage_record){

		case bussinessType_Home:{

			stt_msgDats_dataManagementHandle rptr_msgQ_dmHandle = {0};
			uint8_t btnBindingStatus_temp[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0}; 

			//控件显示信息刷新业务
			if(xQueueReceive(msgQh_dataManagementHandle, &rptr_msgQ_dmHandle, 5 / portTICK_RATE_MS) == pdTRUE){

				switch(rptr_msgQ_dmHandle.msgType){

					case dataManagement_msgType_homePageCtrlObjTextChg:{ //文字

						stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
						usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);
					
						styleBtn_Text.text.font = usrAppHomepageBtnBkText_fontGet(dataTextObjDisp_temp.countryFlg);
						styleIconvText_devMulitSw_statusOn.text.font = styleBtn_Text.text.font;
						styleIconvText_devMulitSw_statusOff.text.font = styleBtn_Text.text.font;

						switch(currentDev_typeGet()){

							case devTypeDef_mulitSwOneBit:{

								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 0))
									lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);

							}break;
							
							case devTypeDef_mulitSwTwoBit:{

								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 0))
									lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 1))
									lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);

							}break;
							
							case devTypeDef_mulitSwThreeBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 0))
									lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 1))
									lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 2))
									lv_label_set_text(textBtn_toilet, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[2]);

							}break;

							default:break;
						}

					}break;
					
					case dataManagement_msgType_homePageCtrlObjIconChg:{ //图标

//						printf("icon change ,bithold:%02X.\n", rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold);

						uint8_t dataIconObjDisp_temp[GUIBUSSINESS_CTRLOBJ_MAX_NUM] = {0};
						usrAppHomepageBtnIconNumDisp_paramGet(dataIconObjDisp_temp);
					
						switch(currentDev_typeGet()){
						
							case devTypeDef_mulitSwOneBit:{

								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 0))
									lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
							}break;
							
							case devTypeDef_mulitSwTwoBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 0))
									lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 1))
									lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));
							}break;
							
							case devTypeDef_mulitSwThreeBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 0))
									lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 1))
									lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 2))
									lv_img_set_src(iconBtn_toilet, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[2]));
							}break;
						
							default:break;
						}
						
					}break;

					case dataManagement_msgType_homePageThemeTypeChg:{ //home界面风格 

						if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpThemeTypeChg.themeTypeChg_notice){

							stt_devDataPonitTypedef devDataPoint = {0};
							
							currentDev_dataPointGet(&devDataPoint);

							switch(currentDev_typeGet()){
							
								case devTypeDef_mulitSwOneBit:{
							
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
							
								}break;
								
								case devTypeDef_mulitSwTwoBit:{
							
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
							
								}break;
								
								case devTypeDef_mulitSwThreeBit:{
									
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOff));

								}break;
							
								default:break;
							}
						}
						
					}break;

					default:break;
				}
			}

			//互控图标业务
			devMutualCtrlGroupInfo_groupInsertGet(btnBindingStatus_temp);
			if(memcmp(btnBindingStatus_record, btnBindingStatus_temp, sizeof(uint8_t) * DEVICE_MUTUAL_CTRL_GROUP_NUM)){

				switch(currentDev_typeGet()){

					case devTypeDef_mulitSwOneBit:{

						if(btnBindingStatus_record[0] != btnBindingStatus_temp[0]){

							if(btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){

								if(iconBtn_binding_A != NULL){

								   lv_obj_del(iconBtn_binding_A);
								   iconBtn_binding_A = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_A == NULL){

									iconBtn_binding_A = lv_imgbtn_create(btn_bk_devMulitSw_A, NULL);
									mutualCtrlTrigIf_A = true;
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnA);
									lv_obj_set_protect(iconBtn_binding_A, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_A, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}

					}break;

					case devTypeDef_mulitSwTwoBit:{

						if(btnBindingStatus_record[0] != btnBindingStatus_temp[0]){
						
							if(btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){
						
								if(iconBtn_binding_A != NULL){
						
								   lv_obj_del(iconBtn_binding_A);
								   iconBtn_binding_A = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_A == NULL){
						
									iconBtn_binding_A = lv_imgbtn_create(btn_bk_devMulitSw_A, NULL);
									mutualCtrlTrigIf_A = true;
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnA);
									lv_obj_set_protect(iconBtn_binding_A, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_A, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}
						
						if(btnBindingStatus_record[1] != btnBindingStatus_temp[1]){

							if(btnBindingStatus_temp[1] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[1] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){

								if(iconBtn_binding_B != NULL){

								   lv_obj_del(iconBtn_binding_B);
								   iconBtn_binding_B = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_B == NULL){

									iconBtn_binding_B = lv_imgbtn_create(btn_bk_devMulitSw_B, NULL);
									mutualCtrlTrigIf_B = true;
									lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_B, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnB);
									lv_obj_set_protect(iconBtn_binding_B, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_B, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}

					}break;

					case devTypeDef_mulitSwThreeBit:{

						if(btnBindingStatus_record[0] != btnBindingStatus_temp[0]){
						
							if(btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[0] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){
						
								if(iconBtn_binding_A != NULL){
						
								   lv_obj_del(iconBtn_binding_A);
								   iconBtn_binding_A = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_A == NULL){
						
									iconBtn_binding_A = lv_imgbtn_create(btn_bk_devMulitSw_A, NULL);
									mutualCtrlTrigIf_A = true;
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnA);
									lv_obj_set_protect(iconBtn_binding_A, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_A, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}
						
						if(btnBindingStatus_record[1] != btnBindingStatus_temp[1]){
						
							if(btnBindingStatus_temp[1] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[1] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){
						
								if(iconBtn_binding_B != NULL){
						
								   lv_obj_del(iconBtn_binding_B);
								   iconBtn_binding_B = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_B == NULL){
						
									iconBtn_binding_B = lv_imgbtn_create(btn_bk_devMulitSw_B, NULL);
									mutualCtrlTrigIf_B = true;
									lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_B, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_B, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnB);
									lv_obj_set_protect(iconBtn_binding_B, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_B, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}

						if(btnBindingStatus_record[2] != btnBindingStatus_temp[2]){
						
							if(btnBindingStatus_temp[2] == DEVICE_MUTUALGROUP_INVALID_INSERT_A ||
							   btnBindingStatus_temp[2] == DEVICE_MUTUALGROUP_INVALID_INSERT_B){
						
								if(iconBtn_binding_C != NULL){
						
								   lv_obj_del(iconBtn_binding_C);
								   iconBtn_binding_C = NULL;
								}
							}
							else
							{
								if(iconBtn_binding_C == NULL){
						
									iconBtn_binding_C = lv_imgbtn_create(btn_bk_devMulitSw_C, NULL);
									mutualCtrlTrigIf_C = true;
									lv_imgbtn_set_src(iconBtn_binding_C, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_C, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_C, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnC);
									lv_obj_set_protect(iconBtn_binding_C, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_C, NULL, LV_ALIGN_OUT_RIGHT_MID, -35, -24);
								}
							}
						}

					}break;

					default:break;
				}

				memcpy(btnBindingStatus_record, btnBindingStatus_temp, sizeof(uint8_t) * DEVICE_MUTUAL_CTRL_GROUP_NUM); //比较值更新
			}

			//针对不同开关主界面动态内容逻辑业务
			switch(currentDev_typeGet()){

				case devTypeDef_curtain:{

					stt_msgDats_devCurtainDriver rptr_msgQ_devCurtainDriver = {0};

					if(xQueueReceive(msgQh_devCurtainDriver, &rptr_msgQ_devCurtainDriver, 10 / portTICK_RATE_MS) == pdTRUE){

						switch(rptr_msgQ_devCurtainDriver.msgType){

							case msgType_devCurtainDriver_opreatStop:{ //窗帘驱动，回弹至停止消息接收后进行UI更新

								if(rptr_msgQ_devCurtainDriver.msgDats.data_opreatStop.opreatStop_sig){

									uint8_t devCurtain_orbitalPosPercent = devCurtain_currentPositionPercentGet();
									lv_coord_t imageCurtainPosAdj_temp = devCurtain_orbitalPosPercent * 12;

									imageCurtainPosAdj_temp /= 10;
									lv_obj_set_pos(image_bk_devCurtain_bodyLeft, 2 - imageCurtainPosAdj_temp, 75);
									lv_obj_set_pos(image_bk_devCurtain_bodyRight, 120 + imageCurtainPosAdj_temp, 75);

									//滑块及文字控件再刷新，防止信息同步不完整
									lv_slider_set_value(slider_bk_devCurtain, devCurtain_orbitalPosPercent);
									lv_slider_set_value(slider_bk_devCurtainPosTips, devCurtain_orbitalPosPercent);
									sprintf(str_devParamPositionCur_devCurtain, "position current:%d%%.", devCurtain_orbitalPosPercent);
									lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
									sprintf(str_devParamPositionAdj_devCurtain, "position adjust:%d%%.", devCurtain_orbitalPosPercent);
									lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);

									lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
									lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
									lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
								}

							}break;
							
							case msgType_devCurtainDriver_orbitalChgingByBtn:{

								lv_coord_t imageCurtainPosAdj_temp = 12 * rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent;
//								static lv_coord_t imageCurtainPosAdj_record = 0;

								imageCurtainPosAdj_temp /= 10;
								if(!(imageCurtainPosAdj_temp % 9)){

									lv_obj_set_pos(image_bk_devCurtain_bodyLeft, 2 - imageCurtainPosAdj_temp, 75);
									lv_obj_set_pos(image_bk_devCurtain_bodyRight, 120 + imageCurtainPosAdj_temp, 75);
								}

								lv_slider_set_value(slider_bk_devCurtain, rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								lv_slider_set_value(slider_bk_devCurtainPosTips, rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								sprintf(str_devParamPositionAdj_devCurtain, "position adjust:%d%%.", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
								sprintf(str_devParamPositionCur_devCurtain, "position current:%d%%.", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);

							}break;

							case msgType_devCurtainDriver_orbitalChgingBySlider:{

								lv_slider_set_value(slider_bk_devCurtainPosTips, rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingBySlider.orbitalPosPercent);
								sprintf(str_devParamPositionCur_devCurtain, "position current:%d%%.", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingBySlider.orbitalPosPercent);
								lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);

							}break;

							default:break;
						}
					}
				
				}break;

				case devTypeDef_heater:{

					stt_msgDats_devHeaterDriver rptr_msgQ_devHeaterDriver = {0};
					stt_devDataPonitTypedef devDataPoint = {0};
	 
					if(xQueueReceive(msgQh_devHeaterDriver, &rptr_msgQ_devHeaterDriver, 10 / portTICK_RATE_MS) == pdTRUE){

						switch(rptr_msgQ_devHeaterDriver.msgType){

							case msgType_devHeaterDriver_devActChg:{

								switch(rptr_msgQ_devHeaterDriver.msgDats.data_opreatChg.act){

									case heaterOpreatAct_close:{

										sprintf(str_devRunningRemind_devHeater, "the heater is off now.");
										devDataPoint.devType_heater.devHeater_swEnumVal = heaterOpreatAct_close;
										currentDev_dataPointSet(&devDataPoint, true, true, true);

									}break;
									
									case heaterOpreatAct_open:{

										sprintf(str_devRunningRemind_devHeater, "the heater is on now.");
										devDataPoint.devType_heater.devHeater_swEnumVal = heaterOpreatAct_open;
										currentDev_dataPointSet(&devDataPoint, true, true, true);

									}break;

									default:break;
								}
								
							}break;
							
							case msgType_devHeaterDriver_closeCounterChg:{

								sprintf(str_devRunningRemind_devHeater, "the heater will be \nturn off after \n%dmin-%dsec.",
																		(int)rptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter / 60,
																		(int)rptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter % 60);

							}break;

							default:break;
						}

						lv_label_set_text(label_bk_devHeater_timeRemind, str_devRunningRemind_devHeater);
					}
				
				}break;

				default:break;	
			}

		}break;

		default:break;
	}
}

void pageHome_buttonMain_imageRefresh(bool freshNoRecord){ //界面切换时调用一次，数据点设置时调用一次

	if(devRunningTimeFromPowerUp_couter < 1)return;

	if(guiPage_current == bussinessType_Home){

		stt_devDataPonitTypedef devDataPoint = {0};
		static stt_devDataPonitTypedef devDataPoint_record = {0};
		bool imageRefresh_IF = false;

		if(freshNoRecord)memset(&devDataPoint_record, 0xff, sizeof(stt_devDataPonitTypedef));

		currentDev_dataPointGet(&devDataPoint);

		if(memcmp(&devDataPoint_record, &devDataPoint, sizeof(stt_devDataPonitTypedef))){ //开关值变化时才刷新，避免重复刷新，影响界面显示效能

			imageRefresh_IF = true;
		}

		if(imageRefresh_IF){

			switch(currentDev_typeGet()){
	
				case devTypeDef_mulitSwOneBit:{

					if(devDataPoint_record.devType_mulitSwitch_oneBit.swVal_bit1 != devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1){
						
						(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
					}

					//必须刷新一下，否则非手动操作时图片刷不动
					lv_obj_refresh_style(btn_bk_devMulitSw_A);
					
				}break;
	
				case devTypeDef_mulitSwTwoBit:{

					if(devDataPoint_record.devType_mulitSwitch_twoBit.swVal_bit1 != devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1){

						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));			
					}

					if(devDataPoint_record.devType_mulitSwitch_twoBit.swVal_bit2 != devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2){

						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
					}

					//必须刷新一下，否则非手动操作时图片刷不动
					lv_obj_refresh_style(btn_bk_devMulitSw_A);
					lv_obj_refresh_style(btn_bk_devMulitSw_B);
		
				}break;
	
				case devTypeDef_mulitSwThreeBit:{

//					printf("sw statusData refresh:bit1:%d.\n", devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1);

					if(devDataPoint_record.devType_mulitSwitch_threeBit.swVal_bit1 != devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1){

						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
					}

					if(devDataPoint_record.devType_mulitSwitch_threeBit.swVal_bit2 != devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2){

						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));
					}

					if(devDataPoint_record.devType_mulitSwitch_threeBit.swVal_bit3 != devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3){

						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOff));
						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOff));
//						(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOn)):
//							(lv_imgbtn_set_style(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, &styleBtn_devMulitSw_statusOff));						
					}

					//必须刷新一下，否则非手动操作时图片刷不动
					lv_obj_refresh_style(btn_bk_devMulitSw_A);
					lv_obj_refresh_style(btn_bk_devMulitSw_B);
					lv_obj_refresh_style(btn_bk_devMulitSw_C);
					
				}break;
	
				case devTypeDef_dimmer:{

					lv_slider_set_value(slider_bk_devDimmer, devDataPoint.devType_dimmer.devDimmer_brightnessVal);
					sprintf(str_sliderBkVal_devDimmer, "Brightness:%d%%", lv_slider_get_value(slider_bk_devDimmer));
					lv_label_set_text(label_bk_devDimmer, str_sliderBkVal_devDimmer);

					if(lv_slider_get_value(slider_bk_devDimmer)){
					
						stylePhotoBk_devDimmer.image.opa = ((uint8_t)lv_slider_get_value(slider_bk_devDimmer)) * 2 + 50;
					}
					else
					{
						stylePhotoBk_devDimmer.image.opa = 0;
					}
					lv_img_set_style(photoB_bk_devDimmer, &stylePhotoBk_devDimmer);
					stylePhotoAk_devDimmer.image.intense = 100 - ((uint8_t)lv_slider_get_value(slider_bk_devDimmer));
					lv_img_set_style(photoA_bk_devDimmer, &stylePhotoAk_devDimmer);
					
				}break;

				case devTypeDef_curtain:{

					uint8_t devCurtain_orbitalPosPercent = devCurtain_currentPositionPercentGet();

					lv_slider_set_value(slider_bk_devCurtain, devCurtain_orbitalPosPercent);
					lv_slider_set_value(slider_bk_devCurtainPosTips, devCurtain_orbitalPosPercent);
					sprintf(str_devParamPositionCur_devCurtain, "position current:%d%%.", devCurtain_orbitalPosPercent);
					lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
					sprintf(str_devParamPositionAdj_devCurtain, "position adjust:%d%%.", devCurtain_orbitalPosPercent);
					lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
					
				}break;

				case devTypeDef_fans:{

					for(uint8_t loop = 0; loop < DEVICE_CURTAIN_OPREAT_ACTION_NUM; loop ++){

						if(devDataPoint.devType_fans.devFans_swEnumVal == objDevFans_rollerDispConferenceTab[loop].opreatActCurrent){

							lv_roller_set_selected(roller_bk_devFans, objDevFans_rollerDispConferenceTab[loop].optionSelect, false);
							break;
						}
					}					

				}break;

				case devTypeDef_heater:{

					for(uint8_t loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM; loop ++){
					
						if(devDataPoint.devType_heater.devHeater_swEnumVal == objDevHeater_rollerDispConferenceTab[loop].opreatActCurrent){
					
							lv_roller_set_selected(roller_bk_devHeater, objDevHeater_rollerDispConferenceTab[loop].optionSelect, false);
							break;
						}
					}	

				}break;
	
				default:break;
			}

			memcpy(&devDataPoint_record, &devDataPoint, sizeof(stt_devDataPonitTypedef));
		}
	}
}

static lv_res_t funCb_btnActionClick_homeMenu_prLoop(lv_obj_t *btn){

//	const lv_coord_t btnSizeShift = 22; //图标边界保护坐标偏移限定值

//	lv_indev_t * indev = lv_indev_get_act();
//	lv_point_t positionCurrent;

//	lv_indev_get_point(indev, &positionCurrent); //获取当前输入设备输入坐标值

//	//图标边界保护业务逻辑
//	if(positionCurrent.x > LV_HOR_RES - btnSizeShift)positionCurrent.x = LV_HOR_RES - btnSizeShift;
//	if(positionCurrent.x < btnSizeShift)positionCurrent.x = btnSizeShift;
//	if(positionCurrent.y > LV_VER_RES - btnSizeShift)positionCurrent.y = LV_VER_RES - btnSizeShift;
//	if(positionCurrent.y < btnSizeShift)positionCurrent.y = btnSizeShift;
//	lv_obj_set_pos(btn_homeMenu, positionCurrent.x - btnSizeShift, positionCurrent.y - btnSizeShift);
	
	return LV_RES_OK;
}

static void local_guiHomeBussiness_mulitSwOneBit(lv_obj_t * obj_Parent){

	stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
	uint8_t dataIconObjDisp_temp[GUIBUSSINESS_CTRLOBJ_MAX_NUM] = {0};
	stt_devDataPonitTypedef devDataPoint = {0};
	
	currentDev_dataPointGet(&devDataPoint);
	usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);
	usrAppHomepageBtnIconNumDisp_paramGet(dataIconObjDisp_temp);
	
	styleBtn_Text.text.font = usrAppHomepageBtnBkText_fontGet(dataTextObjDisp_temp.countryFlg);
	styleIconvText_devMulitSw_statusOn.text.font = styleBtn_Text.text.font;
	styleIconvText_devMulitSw_statusOff.text.font = styleBtn_Text.text.font;

	//home界面开关按键创建，并设置底图、位置、及加载动画
	btn_bk_devMulitSw_A = lv_imgbtn_create(obj_Parent, NULL);
//	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, usrAppHomepageBtnBkPic_dataGet(false));
	lv_obj_set_pos(btn_bk_devMulitSw_A, 5, 134);
	lv_obj_animate(btn_bk_devMulitSw_A, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	//home界面开关按键回调创建
	lv_btn_set_action(btn_bk_devMulitSw_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnA);
	//home界面开关按键说明文字创建，并设置位置
	iconBtn_meeting = lv_img_create(btn_bk_devMulitSw_A, NULL);
	lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
//	lv_img_set_style(iconBtn_meeting, &styleBtnImg_icon);
	lv_obj_set_protect(iconBtn_meeting, LV_PROTECT_POS);
	lv_obj_align(iconBtn_meeting, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);
	//home界面开关按键说明文字风格加载
	textBtn_meeting = lv_label_create(btn_bk_devMulitSw_A, NULL);
//	lv_obj_set_style(textBtn_meeting, &styleBtn_Text);
	lv_label_set_align(textBtn_meeting, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(textBtn_meeting, LV_LABEL_LONG_DOT);
	lv_obj_set_size(textBtn_meeting, 128, 25);
	lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
	lv_obj_set_protect(textBtn_meeting, LV_PROTECT_POS);
	lv_obj_align(textBtn_meeting, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);

	(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_oneBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
}

static void local_guiHomeBussiness_mulitSwTwoBit(lv_obj_t * obj_Parent){

	stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
	uint8_t dataIconObjDisp_temp[GUIBUSSINESS_CTRLOBJ_MAX_NUM] = {0};
	stt_devDataPonitTypedef devDataPoint = {0};
	
	currentDev_dataPointGet(&devDataPoint);
	usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);
	usrAppHomepageBtnIconNumDisp_paramGet(dataIconObjDisp_temp);

	styleBtn_Text.text.font = usrAppHomepageBtnBkText_fontGet(dataTextObjDisp_temp.countryFlg);
	styleIconvText_devMulitSw_statusOn.text.font = styleBtn_Text.text.font;
	styleIconvText_devMulitSw_statusOff.text.font = styleBtn_Text.text.font;

	//home界面开关按键创建，并设置底图、位置、及加载动画
	btn_bk_devMulitSw_A = lv_imgbtn_create(obj_Parent, NULL);
//	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, usrAppHomepageBtnBkPic_dataGet(false));
	lv_obj_set_pos(btn_bk_devMulitSw_A, 5, 81);
	btn_bk_devMulitSw_B = lv_imgbtn_create(obj_Parent, btn_bk_devMulitSw_A);
	lv_obj_set_pos(btn_bk_devMulitSw_B, 5, 187);
	lv_obj_animate(btn_bk_devMulitSw_A, LV_ANIM_FLOAT_RIGHT, 200,	 0, NULL);
	lv_obj_animate(btn_bk_devMulitSw_B, LV_ANIM_FLOAT_RIGHT, 200,	50, NULL);
	lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	//home界面开关按键回调创建
	lv_btn_set_action(btn_bk_devMulitSw_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnA);
	lv_btn_set_action(btn_bk_devMulitSw_B, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnB);
	//home界面开关按键说明文字创建，并设置位置
	iconBtn_meeting = lv_img_create(btn_bk_devMulitSw_A, NULL);
	iconBtn_sleeping = lv_img_create(btn_bk_devMulitSw_B, NULL);
	lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
	lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));
//	lv_img_set_style(iconBtn_meeting, &styleBtnImg_icon);
//	lv_img_set_style(iconBtn_sleeping, &styleBtnImg_icon);
	lv_obj_set_protect(iconBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(iconBtn_sleeping, LV_PROTECT_POS);
	lv_obj_align(iconBtn_meeting, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);
	lv_obj_align(iconBtn_sleeping, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);
	//home界面开关按键说明文字风格加载
	textBtn_meeting = lv_label_create(btn_bk_devMulitSw_A, NULL);
	textBtn_sleeping = lv_label_create(btn_bk_devMulitSw_B, NULL);
//	lv_obj_set_style(textBtn_meeting, &styleBtn_Text);
//	lv_obj_set_style(textBtn_sleeping, &styleBtn_Text);
	lv_label_set_align(textBtn_meeting, LV_LABEL_ALIGN_CENTER);
	lv_label_set_align(textBtn_sleeping, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(textBtn_meeting, LV_LABEL_LONG_DOT);
	lv_label_set_long_mode(textBtn_sleeping, LV_LABEL_LONG_DOT);
	lv_obj_set_size(textBtn_meeting, 128, 25);
	lv_obj_set_size(textBtn_sleeping, 128, 25);
	lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
	lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);
	lv_obj_set_protect(textBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(textBtn_sleeping, LV_PROTECT_POS);
	lv_obj_align(textBtn_meeting, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);
	lv_obj_align(textBtn_sleeping, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);

	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_twoBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));	
}

static void local_guiHomeBussiness_mulitSwThreeBit(lv_obj_t * obj_Parent){

	stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
	uint8_t dataIconObjDisp_temp[GUIBUSSINESS_CTRLOBJ_MAX_NUM] = {0};
	stt_devDataPonitTypedef devDataPoint = {0};
	
	currentDev_dataPointGet(&devDataPoint);	
	usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);
	usrAppHomepageBtnIconNumDisp_paramGet(dataIconObjDisp_temp);

	styleBtn_Text.text.font = usrAppHomepageBtnBkText_fontGet(dataTextObjDisp_temp.countryFlg);
	styleIconvText_devMulitSw_statusOn.text.font = styleBtn_Text.text.font;
	styleIconvText_devMulitSw_statusOff.text.font = styleBtn_Text.text.font;

	//home界面开关按键创建，并设置底图、位置、及加载动画
	btn_bk_devMulitSw_A = lv_imgbtn_create(obj_Parent, NULL);
//	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
	lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, usrAppHomepageBtnBkPic_dataGet(false));
	lv_obj_set_pos(btn_bk_devMulitSw_A, 5, 41);
	btn_bk_devMulitSw_B = lv_imgbtn_create(obj_Parent, btn_bk_devMulitSw_A);
	lv_obj_set_pos(btn_bk_devMulitSw_B, 5, 134);
	btn_bk_devMulitSw_C = lv_imgbtn_create(obj_Parent, btn_bk_devMulitSw_A);
	lv_obj_set_pos(btn_bk_devMulitSw_C, 5, 227);
	lv_obj_animate(btn_bk_devMulitSw_A, LV_ANIM_FLOAT_RIGHT, 200,	 0, NULL);
	lv_obj_animate(btn_bk_devMulitSw_B, LV_ANIM_FLOAT_RIGHT, 200,	50, NULL);
	lv_obj_animate(btn_bk_devMulitSw_C, LV_ANIM_FLOAT_RIGHT, 200,  100, NULL);
	lv_imgbtn_set_style(btn_bk_devMulitSw_A, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	lv_imgbtn_set_style(btn_bk_devMulitSw_B, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	lv_imgbtn_set_style(btn_bk_devMulitSw_C, LV_BTN_STATE_PR, &styleBtn_devMulitSw_statusOn);
	//home界面开关按键回调创建
	lv_btn_set_action(btn_bk_devMulitSw_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnA);
	lv_btn_set_action(btn_bk_devMulitSw_B, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnB);
	lv_btn_set_action(btn_bk_devMulitSw_C, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devMulitSw_mainBtnC);
	//home界面开关按键说明文字创建，并设置位置
	iconBtn_meeting = lv_img_create(btn_bk_devMulitSw_A, NULL);
	iconBtn_sleeping = lv_img_create(btn_bk_devMulitSw_B, NULL);
	iconBtn_toilet = lv_img_create(btn_bk_devMulitSw_C, NULL);
	lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
	lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));
	lv_img_set_src(iconBtn_toilet, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[2]));
//	lv_img_set_style(iconBtn_meeting, &styleBtnImg_icon);
//	lv_img_set_style(iconBtn_sleeping, &styleBtnImg_icon);
//	lv_img_set_style(iconBtn_toilet, &styleBtnImg_icon);
	lv_obj_set_protect(iconBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(iconBtn_sleeping, LV_PROTECT_POS);
	lv_obj_set_protect(iconBtn_toilet, LV_PROTECT_POS);
	lv_obj_align(iconBtn_meeting, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);
	lv_obj_align(iconBtn_sleeping, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);
	lv_obj_align(iconBtn_toilet, NULL, LV_ALIGN_OUT_LEFT_MID, 65, 15);

//	testImg_data.data = dataPtr_btnTextImg_sw_A;
//	lv_obj_t *iconTest = lv_img_create(btn_bk_devMulitSw_A, NULL);
//	lv_img_set_src(iconTest, &testImg_data);
//	lv_obj_set_protect(iconTest, LV_PROTECT_POS);
//	lv_obj_align(iconTest, btn_bk_devMulitSw_A, LV_ALIGN_CENTER, 0, -30);
//	lv_obj_set_top(iconTest, true);
	
	//home界面开关按键说明文字风格加载
	textBtn_meeting = lv_label_create(btn_bk_devMulitSw_A, NULL);
	textBtn_sleeping = lv_label_create(btn_bk_devMulitSw_B, NULL);
	textBtn_toilet = lv_label_create(btn_bk_devMulitSw_C, NULL);
//	lv_obj_set_style(textBtn_meeting, &styleBtn_Text);
//	lv_obj_set_style(textBtn_sleeping, &styleBtn_Text);
//	lv_obj_set_style(textBtn_toilet, &styleBtn_Text);
	lv_label_set_align(textBtn_meeting, LV_LABEL_ALIGN_CENTER);
	lv_label_set_align(textBtn_sleeping, LV_LABEL_ALIGN_CENTER);
	lv_label_set_align(textBtn_toilet, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(textBtn_meeting, LV_LABEL_LONG_ROLL);
	lv_label_set_long_mode(textBtn_sleeping, LV_LABEL_LONG_DOT);
	lv_label_set_long_mode(textBtn_toilet, LV_LABEL_LONG_DOT);
	lv_obj_set_size(textBtn_meeting, 128, 25);
	lv_obj_set_size(textBtn_sleeping, 128, 25);
	lv_obj_set_size(textBtn_toilet, 128, 25);
	lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
	lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);
	lv_label_set_text(textBtn_toilet, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[2]);
	lv_obj_set_protect(textBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(textBtn_sleeping, LV_PROTECT_POS);
	lv_obj_set_protect(textBtn_toilet, LV_PROTECT_POS);
	lv_obj_align(textBtn_meeting, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);
	lv_obj_align(textBtn_sleeping, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);
	lv_obj_align(textBtn_toilet, NULL, LV_ALIGN_IN_LEFT_MID, 71, 20);

	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOff));
	(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit3)?(lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOff));
}

static void local_guiHomeBussiness_dimmer(lv_obj_t * obj_Parent){

	stt_devDataPonitTypedef devDataPoint = {0};
	
	currentDev_dataPointGet(&devDataPoint);

	photoA_bk_devDimmer = lv_img_create(obj_Parent, NULL);
	photoB_bk_devDimmer = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(photoA_bk_devDimmer, &iconLightA_HomePageDeviceDimmer);
	lv_img_set_src(photoB_bk_devDimmer, &iconLightB_HomePageDeviceDimmer);
	lv_obj_set_protect(photoA_bk_devDimmer, LV_PROTECT_POS);
	lv_obj_set_protect(photoB_bk_devDimmer, LV_PROTECT_POS);
	lv_obj_set_pos(photoA_bk_devDimmer, 41, 60);
	lv_obj_align(photoB_bk_devDimmer, photoA_bk_devDimmer, LV_ALIGN_CENTER, 5, -25);
	lv_img_set_style(photoB_bk_devDimmer, &stylePhotoBk_devDimmer);

	slider_bk_devDimmer = lv_slider_create(obj_Parent, NULL);
	lv_obj_set_size(slider_bk_devDimmer, 200, 30);
	lv_obj_set_protect(slider_bk_devDimmer, LV_PROTECT_POS);
	lv_obj_set_pos(slider_bk_devDimmer, 20, 260);
	lv_slider_set_action(slider_bk_devDimmer, funCb_slidAction_devDimmer_mainSlider);
	lv_bar_set_value(slider_bk_devDimmer, DEVICE_DIMMER_BRIGHTNESS_MAX_VAL);

	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_BG, &styleSliderBk_devDimmer_bg);
	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_INDIC, &styleSliderBk_devDimmer_indic);
	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_KNOB, &styleSliderBk_devDimmer_knob);

	label_bk_devDimmer = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devDimmer, &styleText_devDimmer_SliderBk);
	lv_slider_set_value(slider_bk_devDimmer, devDataPoint.devType_dimmer.devDimmer_brightnessVal);
	sprintf(str_sliderBkVal_devDimmer, "Brightness:%d%%", lv_slider_get_value(slider_bk_devDimmer));
	lv_label_set_text(label_bk_devDimmer, str_sliderBkVal_devDimmer);
	lv_obj_align(label_bk_devDimmer, slider_bk_devDimmer, LV_ALIGN_OUT_TOP_MID, 0, 0);
	
	if(lv_slider_get_value(slider_bk_devDimmer)){
	
		stylePhotoBk_devDimmer.image.opa = ((uint8_t)lv_slider_get_value(slider_bk_devDimmer)) * 2 + 50;
	}
	else
	{
		stylePhotoBk_devDimmer.image.opa = 0;
	}
	lv_img_set_style(photoB_bk_devDimmer, &stylePhotoBk_devDimmer);
	stylePhotoAk_devDimmer.image.intense = 100 - ((uint8_t)lv_slider_get_value(slider_bk_devDimmer));
	lv_img_set_style(photoA_bk_devDimmer, &stylePhotoAk_devDimmer);
}

static void local_guiHomeBussiness_fans(lv_obj_t * obj_Parent){

	roller_bk_devFans = lv_roller_create(obj_Parent, NULL);
	lv_roller_set_options(roller_bk_devFans, "close\n""first gear\n""second gear\n""third gear");
	lv_obj_set_protect(roller_bk_devFans, LV_PROTECT_POS);
	lv_roller_set_hor_fit(roller_bk_devFans, false);
	lv_obj_set_width(roller_bk_devFans, 180);
	lv_obj_set_pos(roller_bk_devFans, 30, 120);
	lv_roller_set_action(roller_bk_devFans, funCb_rollAction_devFans_mainRoller);
//	lv_roller_set_style(roller_bk_devFans, LV_ROLLER_STYLE_BG, &styleRollerBk_devFans_bg);
//	lv_roller_set_style(roller_bk_devFans, LV_ROLLER_STYLE_SEL, &styleRollerBk_devFans_sel);
	lv_roller_set_visible_row_count(roller_bk_devFans, 3);
}

static void local_guiHomeBussiness_scenario(lv_obj_t * obj_Parent){

	
}

static void local_guiHomeBussiness_curtain(lv_obj_t * obj_Parent){

	uint8_t devCurtain_orbitalPosPercent = devCurtain_currentPositionPercentGet();

//	cont_bk_devCurtainImage = lv_cont_create(obj_Parent, NULL);
//	lv_cont_set_layout(cont_bk_devCurtainImage, LV_LAYOUT_CENTER);
//	lv_cont_set_fit(cont_bk_devCurtainImage, false, false);
//	lv_obj_set_size(cont_bk_devCurtainImage, 236, 260);
//	lv_obj_set_pos(cont_bk_devCurtainImage, 2, 60);
//	image_bk_devCurtain_gan = lv_img_create(cont_bk_devCurtainImage, NULL);
//	lv_img_set_src(image_bk_devCurtain_gan, &imageCurtain_gan);
//	lv_obj_set_protect(image_bk_devCurtain_gan, LV_PROTECT_POS);
//	lv_obj_set_pos(image_bk_devCurtain_gan, 2, 60);
	image_bk_devCurtain_bodyLeft = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(image_bk_devCurtain_bodyLeft, &imageCurtain_body);
	lv_obj_set_protect(image_bk_devCurtain_bodyLeft, LV_PROTECT_POS);
	lv_obj_set_pos(image_bk_devCurtain_bodyLeft, -38, 75);
	image_bk_devCurtain_bodyRight = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(image_bk_devCurtain_bodyRight, &imageCurtain_body);
	lv_obj_set_protect(image_bk_devCurtain_bodyRight, LV_PROTECT_POS);
	lv_obj_set_pos(image_bk_devCurtain_bodyRight, 160, 75);

	slider_bk_devCurtain = lv_slider_create(obj_Parent, NULL);
	lv_obj_set_size(slider_bk_devCurtain, 200, 20);
	lv_obj_set_protect(slider_bk_devCurtain, LV_PROTECT_POS);
	lv_obj_set_pos(slider_bk_devCurtain, 20, 180);
	lv_obj_set_top(slider_bk_devCurtain, true);
	lv_slider_set_action(slider_bk_devCurtain, funCb_slidAction_devCurtain_mainSlider);
	lv_bar_set_value(slider_bk_devCurtain, DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL);
	lv_slider_set_value(slider_bk_devCurtain, devCurtain_orbitalPosPercent);
	slider_bk_devCurtainPosTips = lv_slider_create(obj_Parent, slider_bk_devCurtain);
	lv_obj_set_pos(slider_bk_devCurtainPosTips, 20, 120);
	lv_obj_set_top(slider_bk_devCurtainPosTips, true);
	lv_obj_set_click(slider_bk_devCurtainPosTips, false);
	lv_slider_set_action(slider_bk_devCurtainPosTips, funCb_slidAction_devCurtainPosTips_mainSlider);
	lv_bar_set_value(slider_bk_devCurtainPosTips, DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL);
	lv_slider_set_value(slider_bk_devCurtainPosTips, devCurtain_orbitalPosPercent);

	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_BG, &styleSliderBk_devCurtain_bg);
	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_INDIC, &styleSliderBk_devCurtain_indic);
	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_KNOB, &styleSliderBk_devCurtain_knob);

	btn_bk_devCurtain_open = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &iconPage_curtainOpen_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_open, LV_BTN_STATE_PR, &iconPage_curtainOpen_rel);
	lv_obj_set_pos(btn_bk_devCurtain_open, 20, 260);
	lv_obj_set_top(btn_bk_devCurtain_open, true);
	lv_obj_animate(btn_bk_devCurtain_open, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_open, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_open);

	btn_bk_devCurtain_stop = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &iconPage_curtainPluse_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_stop, LV_BTN_STATE_PR, &iconPage_curtainPluse_rel);
	lv_obj_set_pos(btn_bk_devCurtain_stop, 80, 260);
	lv_obj_set_top(btn_bk_devCurtain_stop, true);
	lv_obj_animate(btn_bk_devCurtain_stop, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_stop, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_stop);

	btn_bk_devCurtain_close = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &iconPage_curtainClose_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_close, LV_BTN_STATE_PR, &iconPage_curtainClose_rel);
	lv_obj_set_pos(btn_bk_devCurtain_close, 140, 260);
	lv_obj_set_top(btn_bk_devCurtain_close, true);
	lv_obj_animate(btn_bk_devCurtain_close, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_close, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_close);

	label_bk_devCurtain_positionCur = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devCurtain_positionCur, &styleText_devCurtain_Bk_positionTips);
	lv_label_set_long_mode(label_bk_devCurtain_positionCur, LV_LABEL_LONG_SCROLL);
	sprintf(str_devParamPositionCur_devCurtain, "position current:%d%%.", devCurtain_orbitalPosPercent);
	lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
	lv_obj_set_protect(label_bk_devCurtain_positionCur, LV_PROTECT_POS);
//	lv_obj_set_pos(label_bk_devCurtain_positionCur, 20, 140);
	lv_obj_align(label_bk_devCurtain_positionCur, slider_bk_devCurtainPosTips, LV_ALIGN_OUT_TOP_MID, 0, -5);
	lv_obj_set_top(label_bk_devCurtain_positionCur, true);

	label_bk_devCurtain_positionAdj = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devCurtain_positionAdj, &styleText_devCurtain_Bk_positionTips);
	lv_label_set_long_mode(label_bk_devCurtain_positionAdj, LV_LABEL_LONG_SCROLL);
	sprintf(str_devParamPositionAdj_devCurtain, "position adjust:%d%%.", devCurtain_orbitalPosPercent);
	lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
	lv_obj_set_protect(label_bk_devCurtain_positionAdj, LV_PROTECT_POS);
//	lv_obj_set_pos(label_bk_devCurtain_positionAdj, 20, 140);
	lv_obj_align(label_bk_devCurtain_positionAdj, slider_bk_devCurtain, LV_ALIGN_OUT_TOP_MID, 0, -5);
	lv_obj_set_top(label_bk_devCurtain_positionAdj, true);
}

static void local_guiHomeBussiness_heater(lv_obj_t * obj_Parent){

	roller_bk_devHeater = lv_roller_create(obj_Parent, NULL);
	lv_roller_set_options(roller_bk_devHeater, "close\n""open\n""closeAfter 30min\n""closeAfter 60min\n""closeAfter custom");
	lv_obj_set_protect(roller_bk_devHeater, LV_PROTECT_POS);
	lv_roller_set_hor_fit(roller_bk_devHeater, false);
	lv_obj_set_width(roller_bk_devHeater, 180);
	lv_obj_set_pos(roller_bk_devHeater, 30, 120);
	lv_roller_set_anim_time(roller_bk_devHeater, 100);
	lv_roller_set_action(roller_bk_devHeater, funCb_rollAction_devHeater_mainRoller);
//	lv_roller_set_style(roller_bk_devHeater, LV_ROLLER_STYLE_BG, &styleRollerBk_devHeater_bg);
//	lv_roller_set_style(roller_bk_devHeater, LV_ROLLER_STYLE_SEL, &styleRollerBk_devHeater_sel);
	lv_roller_set_visible_row_count(roller_bk_devHeater, 3);

	label_bk_devHeater_timeRemind = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devHeater_timeRemind, &styleText_devHeater_Bk_timeRemind);
	lv_label_set_long_mode(label_bk_devHeater_timeRemind, LV_LABEL_LONG_SCROLL);
	sprintf(str_devRunningRemind_devHeater, "the heater is off now.");
	lv_label_set_text(label_bk_devHeater_timeRemind, str_devRunningRemind_devHeater);
	lv_obj_set_pos(label_bk_devHeater_timeRemind, 30, 50);

	btnm_bk_devHeater = lv_btnm_create(obj_Parent, NULL);
	lv_obj_set_size(btnm_bk_devHeater, 200, 40);
//	lv_btnm_get_style(btnm_bk_devHeater, );
	lv_btnm_set_action(btnm_bk_devHeater, funCb_btnmActionClick_devHeater_gearBtnm);
	lv_btnm_set_map(btnm_bk_devHeater, btnm_str_devHeater);
	lv_btnm_set_toggle(btnm_bk_devHeater, true, 3);
	lv_obj_set_protect(btnm_bk_devHeater, LV_PROTECT_POS);
	lv_obj_set_pos(btnm_bk_devHeater, 20, 245);
}

static void guiBussiness_tipsLoopTimerCreat(const char *strTips){

	if(!trigFlg_loopTimerTips){

		trigFlg_loopTimerTips = true;

		usr_loopTimer_tipsKeeper_trig();

		icon_loopTimerTips = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(icon_loopTimerTips, &iconAlarm_tips);
		lv_obj_set_pos(icon_loopTimerTips, 45, 85);

		text_loopTimerTips = lv_label_create(icon_loopTimerTips, NULL);
		lv_obj_set_style(text_loopTimerTips, &styleText_loopTimerTips);
		lv_label_set_text(text_loopTimerTips, strTips);
		lv_obj_set_protect(text_loopTimerTips, LV_PROTECT_POS);
		lv_obj_align(text_loopTimerTips, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

		lv_obj_animate(icon_loopTimerTips, LV_ANIM_FLOAT_BOTTOM, 200, 0, NULL);
	}
}

static void guiBussiness_tipsLoopTimerDelete(void){

	if(!usr_loopTimer_tipsKeeper_read()){
		
		if(icon_loopTimerTips){
		
			lv_obj_del(icon_loopTimerTips);
			trigFlg_loopTimerTips = false;
		}
	}
}

static const struct stt_guiHomeBussiness{

	guiHomeFun_local_bussiness guiHome_disp;
	devTypeDef_enum currentDevType;

}guiHomeBussiness_tab[DEVICE_TYPE_LIST_NUM] = {

	{local_guiHomeBussiness_mulitSwOneBit, 	devTypeDef_mulitSwOneBit},
	{local_guiHomeBussiness_mulitSwTwoBit, 	devTypeDef_mulitSwTwoBit},
	{local_guiHomeBussiness_mulitSwThreeBit,devTypeDef_mulitSwThreeBit},
	{local_guiHomeBussiness_dimmer,			devTypeDef_dimmer},
	{local_guiHomeBussiness_fans, 			devTypeDef_fans},	
	{local_guiHomeBussiness_scenario, 		devTypeDef_scenario},
	{local_guiHomeBussiness_curtain, 		devTypeDef_curtain},
	{local_guiHomeBussiness_heater, 		devTypeDef_heater},
};

static void lvGui_businessHome(lv_obj_t * obj_Parent){

	uint8_t loop = 0;
	devTypeDef_enum decType_temp = currentDev_typeGet();

//	//home界面进入菜单按键创建，并设置底图、位置、及加载动画
//	btn_homeMenu = lv_imgbtn_create(lv_scr_act(), NULL);
//	lv_imgbtn_set_src(btn_homeMenu, LV_BTN_STATE_REL, &iconHome_menu);
//	lv_imgbtn_set_src(btn_homeMenu, LV_BTN_STATE_PR, &iconHome_menu);
//	lv_btn_set_action(btn_homeMenu, LV_BTN_ACTION_CLICK, funCb_btnActionClick_homeMenu_click);
//	lv_btn_set_action(btn_homeMenu, LV_BTN_ACTION_LONG_PR_REPEAT, funCb_btnActionClick_homeMenu_prLoop);
//	lv_obj_set_pos(btn_homeMenu, 190, 41);
//	lv_obj_set_top(btn_homeMenu, true);
//	btn_homeMenu->drag = 1; //设置为可拖动
//	btn_homeMenu->drag_throw = 1;
//	lv_obj_animate(btn_homeMenu, LV_ANIM_FLOAT_LEFT, 300, 100, NULL);

	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(decType_temp == guiHomeBussiness_tab[loop].currentDevType){

			guiHomeBussiness_tab[loop].guiHome_disp(obj_Parent);
			break;
		}
	}
}

static lv_style_t *lvUsr_objBkReales2SecMenu(void){ //获取菜单标题头风格

	lv_obj_del(imageBK);
	imageBK = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_size(imageBK, 240, 100);

	lv_obj_set_style(imageBK, &styleBk_secMenu);

	return &styleBk_secMenu;
}

static void task_guiSwitch_Detecting(void *pvParameter){

	uint8_t loop = 0;

	EventBits_t screenTouch_etBits = 0;
	EventBits_t loopTimerTips_etBits = 0;
	
//	lv_point_t indevPoint = {0};

//	lv_indev_get_point(lv_indev_get_act(), &indevPoint);
//	printf("point_x:%d, point_y:%d.\n", indevPoint.x, indevPoint.y);
//	lv_indev_get_vect(lv_indev_get_act(), &indevPoint);
//	printf("vect_x:%d, vect_y:%d.\n", indevPoint.x, indevPoint.y);

	for(;;){

		//特殊触摸信号检测
		screenTouch_etBits = xEventGroupWaitBits(xEventGp_screenTouch, 
												 TOUCHEVENT_FLG_BITHOLD_RESERVE,
												 pdTRUE,
												 pdFALSE,
												 5);

		if((screenTouch_etBits & TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG) == TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG){

//			printf("touch get, point_x:%d, point_y:%d.\n", devTouchGetInfo.touchPosition[0].x,
//														   devTouchGetInfo.touchPosition[0].y);

//			printf("touch get, point:%d.\n", devTouchGetInfo.touchNum);
//			devScreenBkLight_weakUp();
		}

		if((screenTouch_etBits & TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP) == TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP){

			switch(devTouchGetInfo.valGestrue){

				case FTIC_EXTRA_REGISTERVAL_GES_mUP:{

					printf("gestrue move up happen.\n");

				}break;

				case FTIC_EXTRA_REGISTERVAL_GES_mDN:{

					printf("gestrue move down happen.\n");
					
				}break;

				case FTIC_EXTRA_REGISTERVAL_GES_mLT:{

					if(ctrlObj_slidingCalmDownCounter)break;

					switch(guiPage_current){
					
						case bussinessType_Home:{
					
							lvGui_usrSwitch(bussinessType_Menu);
					
						}break;
					
						default:break;
					}
					
					printf("gestrue move left happen.\n");

				}break;

				case FTIC_EXTRA_REGISTERVAL_GES_mRT:{

					printf("gestrue move right happen.\n");

				}break;

				case FTIC_EXTRA_REGISTERVAL_GES_zIN:{

					printf("gestrue zoom in happen.\n");

				}break;

				case FTIC_EXTRA_REGISTERVAL_GES_zOUT:{

					printf("gestrue zoom out happen.\n");

				}break;

				default:{

					printf("unknow gestrue:%02X.\n", devTouchGetInfo.valGestrue);
					
				}break;
			}			
		}

		//GUI界面刷新检测
		if(guiPage_record != guiPage_current){

			guiPage_record = guiPage_current;

			vTaskDelay(100 / portTICK_PERIOD_MS);
			
			switch(guiPage_record){

				case bussinessType_Home:{

					lv_img_set_src(imageBK, usrAppHomepageBkPic_dataGet());
					lvGui_businessHome(imageBK);
					pageHome_buttonMain_imageRefresh(true);

					//home界面互控图标比较值重置，以及相关互控图标对象重置（lv系统删除对象指针后 不会 进行NULL赋值）
					memset(btnBindingStatus_record, 0, sizeof(uint8_t) * DEVICE_MUTUAL_CTRL_GROUP_NUM);
					iconBtn_binding_A = NULL;
					iconBtn_binding_B = NULL;
					iconBtn_binding_C = NULL;
					
				}break;

				case bussinessType_Menu:{

					lv_img_set_src(imageBK, usrAppHomepageBkPic_dataGet());
					lvGui_businessMenu(imageBK);
					
				}break;

				case bussinessType_menuPageDeviceManage:{

//					lvUsr_objBkReales2SecMenu();
					
				}break;
				
				case bussinessType_menuPageScence:{

//					lvUsr_objBkReales2SecMenu();

				}break;
				
				case bussinessType_menuPageSecurity:{

//					lvUsr_objBkReales2SecMenu();
					
				}break;
				
				case bussinessType_menuPageStatistics:{

//					lvUsr_objBkReales2SecMenu();
					
				}break;
				
				case bussinessType_menuPageSetting:{

					//控件风格设定：二级菜单灰色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_GRAY;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();

					lvGui_businessMenu_setting(imageBK);
					lv_obj_set_size(imageBK, 240, 75);
					
				}break;

				case bussinessType_menuPageSetting_A:{
					
					//控件风格设定：二级菜单灰色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_GRAY;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();

					lvGui_businessMenu_setting_A(imageBK);
					lv_obj_set_size(imageBK, 240, 75);

				}break;

				case bussinessType_menuPageSetting_B:{
					
					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_setting_B(imageBK);
					lv_obj_set_size(imageBK, 240, 75);

				}break;

				case bussinessType_menuPageSetting_C:{

					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_setting_C(imageBK);
					lv_obj_set_size(imageBK, 240, 75);

				}break;

				case bussinessType_menuPageDeviceInfo:{

					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_deviceInfo(imageBK);
			
				}break;

				default:break;
			}
		}

		//页眉信息更新
		pageHeader_infoRefreshLoop();

		//当前活动界面信息动态刷新
		pageActivity_infoRefreshLoop();
		
		//tips事件检测 ---定时事件
		if(xEventGp_tipsLoopTimer){ //先进行一下非空检测，防止初始化延后导致读取空内存

			loopTimerTips_etBits = xEventGroupWaitBits(xEventGp_tipsLoopTimer, 
													   LOOPTIMEREVENT_FLG_BITHOLD_RESERVE,
													   pdTRUE,
													   pdFALSE,
													   5);
		}

		if(loopTimerTips_etBits){

			for(loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){
			
				if(loopTimerTips_etBits & (1 << loop)){

					char strTips[15] = {0};

					sprintf(strTips, "timer_%d up!", loop + 1);
					guiBussiness_tipsLoopTimerCreat(strTips);

//					printf("timer:%d guiTips trig rx!\n", loop);
				}
			}
		}

		if(trigFlg_loopTimerTips){

			guiBussiness_tipsLoopTimerDelete();
		}

		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void lvGui_usrSwitch(usrGuiBussiness_type guiPage){

	guiPage_current = guiPage;

	if(guiPage_record != bussinessType_Home && guiPage_record != bussinessType_Menu){

		lv_obj_del(imageBK);
		imageBK = lv_img_create(lv_scr_act(), NULL);
		lv_img_set_src(imageBK, usrAppHomepageBkPic_dataGet());
		
	}else{

		if(btn_homeMenu)lv_obj_del(btn_homeMenu);
		lv_obj_clean(imageBK);
	}
}

static void lvGuiHome_styleApplicationInit(void){

	//文字风格设定：页眉指示文字 电量及节点设备数量 
	lv_style_copy(&styleText_elecAnodenum, &lv_style_plain);
	styleText_elecAnodenum.text.font = &lv_font_consola_13;
	styleText_elecAnodenum.text.color = LV_COLOR_WHITE;

	lv_style_copy(&styleText_temperature, &lv_style_plain);
	styleText_temperature.text.font = &lv_font_consola_13;
	styleText_temperature.text.color = LV_COLOR_MAKE(128, 255, 128);

	//文字风格设定：页眉指示文字 时间
	lv_style_copy(&styleText_time, &lv_style_plain);
	styleText_time.text.font = &lv_font_dejavu_20;
	styleText_time.text.color = LV_COLOR_WHITE;

	//文字风格设定：多位开关名称文字描述
	lv_style_copy(&styleBtn_Text, &lv_style_plain);
	styleBtn_Text.text.font = &lv_font_arabic_17;
	styleBtn_Text.text.color = LV_COLOR_WHITE;

	//文字风格设定：调光开关亮度文字描述
	lv_style_copy(&styleText_devDimmer_SliderBk, &lv_style_plain);
	styleText_devDimmer_SliderBk.text.font = &lv_font_dejavu_20;
	styleText_devDimmer_SliderBk.text.color = LV_COLOR_WHITE;

	//文字风格设定：定时响应对应定时类别文字描述
	lv_style_copy(&styleText_loopTimerTips, &lv_style_plain);
	styleText_loopTimerTips.text.font = &lv_font_dejavu_20;
	styleText_loopTimerTips.text.color = LV_COLOR_WHITE;

	//文字风格设定：热水器开关对应延时关闭文字描述
	lv_style_copy(&styleText_devHeater_Bk_timeRemind, &lv_style_plain);
	styleText_devHeater_Bk_timeRemind.text.font = &lv_font_consola_16;
	styleText_devHeater_Bk_timeRemind.text.color = LV_COLOR_ORANGE;

	//文字风格设定：窗帘位置信息描述文字
	lv_style_copy(&styleText_devCurtain_Bk_positionTips, &lv_style_plain);
	styleText_devCurtain_Bk_positionTips.text.font = &lv_font_consola_19;
	styleText_devCurtain_Bk_positionTips.text.color = LV_COLOR_WHITE;

	//图片风格设定：多位开关按键 房间描述图案
	lv_style_copy(&styleBtnImg_icon, &lv_style_plain);
	styleBtnImg_icon.image.color = LV_COLOR_WHITE;
	styleBtnImg_icon.image.intense = LV_OPA_COVER;

	//图片风格设定：多位开关按键 开关状态下按键底图
	lv_style_copy(&styleBtn_devMulitSw_statusOn, &lv_style_plain);
	lv_style_copy(&styleBtn_devMulitSw_statusOff, &lv_style_plain);
	styleBtn_devMulitSw_statusOn.image.color = LV_COLOR_BLUE;
	styleBtn_devMulitSw_statusOn.image.intense = LV_OPA_50;
	styleBtn_devMulitSw_statusOff.image.color = LV_COLOR_GRAY;
	styleBtn_devMulitSw_statusOff.image.intense = LV_OPA_50;

	//图片及文字风格设定：多位开关按键 开关状态下图标及文字风格
	lv_style_copy(&styleIconvText_devMulitSw_statusOn, &lv_style_plain);
	lv_style_copy(&styleIconvText_devMulitSw_statusOff, &lv_style_plain);
	styleIconvText_devMulitSw_statusOn.image.color = LV_COLOR_WHITE;
	styleIconvText_devMulitSw_statusOn.image.intense = LV_OPA_COVER;
	styleIconvText_devMulitSw_statusOn.text.color = LV_COLOR_WHITE;
	styleIconvText_devMulitSw_statusOff.image.color = LV_COLOR_BLACK;
	styleIconvText_devMulitSw_statusOff.image.intense = LV_OPA_COVER;
	styleIconvText_devMulitSw_statusOff.text.color = LV_COLOR_BLACK;

	//图片风格设定：窗帘开关按键 开关状态下按键底图
	lv_style_copy(&styleBtn_devCurtain_statusPre, &lv_style_plain);
	lv_style_copy(&styleBtn_devCurtain_statusRel, &lv_style_plain);
	styleBtn_devCurtain_statusPre.image.color = LV_COLOR_MAKE(128, 128, 255);
	styleBtn_devCurtain_statusPre.image.intense = LV_OPA_40;
	styleBtn_devCurtain_statusRel.image.color = LV_COLOR_GRAY;
	styleBtn_devCurtain_statusRel.image.intense = LV_OPA_30;

	//图片风格设定：调光亮度提示图片底图
	lv_style_copy(&stylePhotoAk_devDimmer, &lv_style_plain);
	stylePhotoBk_devDimmer.image.intense = LV_OPA_0;
	lv_style_copy(&stylePhotoBk_devDimmer, &lv_style_plain);
	stylePhotoBk_devDimmer.image.opa = LV_OPA_0;
	
	//控件风格设定：调光开关及窗帘开关滑动条 滑块、滑条、滑条边框
	lv_style_copy(&styleSliderBk_devDimmer_bg, &lv_style_pretty);
	styleSliderBk_devDimmer_bg.body.main_color = LV_COLOR_BLACK;
	styleSliderBk_devDimmer_bg.body.grad_color = LV_COLOR_GRAY;
	styleSliderBk_devDimmer_bg.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devDimmer_bg.body.border.color = LV_COLOR_WHITE;
	lv_style_copy(&styleSliderBk_devDimmer_indic, &lv_style_pretty);
	styleSliderBk_devDimmer_indic.body.grad_color = LV_COLOR_GREEN;
	styleSliderBk_devDimmer_indic.body.main_color = LV_COLOR_LIME;
	styleSliderBk_devDimmer_indic.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devDimmer_indic.body.shadow.width = 10;
	styleSliderBk_devDimmer_indic.body.shadow.color = LV_COLOR_LIME;
	styleSliderBk_devDimmer_indic.body.padding.hor = 3;
	styleSliderBk_devDimmer_indic.body.padding.ver = 3;
	lv_style_copy(&styleSliderBk_devDimmer_knob, &lv_style_pretty);
	styleSliderBk_devDimmer_knob.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devDimmer_knob.body.opa = LV_OPA_70;
	styleSliderBk_devDimmer_knob.body.padding.ver = 10;

	lv_style_copy(&styleSliderBk_devCurtain_bg, &lv_style_pretty);
	styleSliderBk_devCurtain_bg.body.main_color = LV_COLOR_BLACK;
	styleSliderBk_devCurtain_bg.body.grad_color = LV_COLOR_GRAY;
	styleSliderBk_devCurtain_bg.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devCurtain_bg.body.border.color = LV_COLOR_WHITE;
	lv_style_copy(&styleSliderBk_devCurtain_indic, &lv_style_pretty);
	styleSliderBk_devCurtain_indic.body.grad_color = LV_COLOR_GREEN;
	styleSliderBk_devCurtain_indic.body.main_color = LV_COLOR_LIME;
	styleSliderBk_devCurtain_indic.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devCurtain_indic.body.shadow.width = 10;
	styleSliderBk_devCurtain_indic.body.shadow.color = LV_COLOR_LIME;
	styleSliderBk_devCurtain_indic.body.padding.hor = 3;
	styleSliderBk_devCurtain_indic.body.padding.ver = 3;
	lv_style_copy(&styleSliderBk_devCurtain_knob, &lv_style_pretty);
	styleSliderBk_devCurtain_knob.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devCurtain_knob.body.opa = LV_OPA_70;
	styleSliderBk_devCurtain_knob.body.padding.ver = 10;

	//控件风格设定：风扇开关档位滚动选定框背景、选中风格设置
	lv_style_copy(&styleRollerBk_devFans_bg, &lv_style_plain);
//	styleRollerBk_devFans_bg.body.main_color = LV_COLOR_TRANSP;
//	styleRollerBk_devFans_bg.body.grad_color = LV_COLOR_WHITE;
//	styleRollerBk_devFans_bg.text.font = &lv_font_dejavu_30;
//	styleRollerBk_devFans_bg.text.line_space = 5;
//	styleRollerBk_devFans_bg.text.opa = LV_OPA_TRANSP;
	lv_style_copy(&styleRollerBk_devFans_sel, &lv_style_plain);
//	styleRollerBk_devFans_sel.body.empty = 1;
//	styleRollerBk_devFans_sel.body.radius = 30;
//	styleRollerBk_devFans_sel.text.color = LV_COLOR_BLUE;
//	styleRollerBk_devFans_sel.text.font = &lv_font_consola_19;

	lv_style_copy(&styleRollerBk_devHeater_bg, &lv_style_plain);
//	styleRollerBk_devHeater_bg.body.main_color = LV_COLOR_TRANSP;
//	styleRollerBk_devHeater_bg.body.grad_color = LV_COLOR_WHITE;
//	styleRollerBk_devHeater_bg.text.font = &lv_font_dejavu_30;
//	styleRollerBk_devHeater_bg.text.line_space = 5;
//	styleRollerBk_devHeater_bg.text.opa = LV_OPA_TRANSP;
	lv_style_copy(&styleRollerBk_devHeater_sel, &lv_style_plain);
//	styleRollerBk_devHeater_sel.body.empty = 1;
//	styleRollerBk_devHeater_sel.body.radius = 30;
//	styleRollerBk_devHeater_sel.text.color = LV_COLOR_BLUE;
//	styleRollerBk_devHeater_sel.text.font = &lv_font_consola_19;
}

void lvGui_businessInit(void){

	lv_obj_t *scr = lv_obj_create(NULL, NULL);
	lv_scr_load(scr);

	lv_theme_t *th = lv_theme_alien_init(100, NULL);
	lv_theme_set_current(th);

	lvGuiHome_styleApplicationInit(); //home界面总体风格初始化

	imageBK = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(imageBK, usrAppHomepageBkPic_dataGet());

			  iconHeaderObj_wifi = lv_img_create(lv_layer_top(), NULL);
	lv_obj_t *iconHeaderObj_node = lv_img_create(lv_layer_top(), NULL);
	lv_obj_t *iconHeaderObj_elec = lv_img_create(lv_layer_top(), NULL);

	lv_obj_set_pos(iconHeaderObj_wifi,	  1, 1);
	lv_obj_set_pos(iconHeaderObj_node,	 25, 1);
	lv_obj_set_pos(iconHeaderObj_elec,	150, 1);

	lv_img_set_src(iconHeaderObj_wifi, &iconHeader_wifi_D);
	lv_img_set_src(iconHeaderObj_node, &iconHeader_node);
	lv_img_set_src(iconHeaderObj_elec, &iconHeader_elec);

	textHeaderObj_time = lv_label_create(lv_layer_top(), NULL);
	lv_label_set_text(textHeaderObj_time, "00:00");
	lv_obj_set_pos(textHeaderObj_time, 84, 2);
	lv_obj_set_style(textHeaderObj_time, &styleText_time);

	textHeaderObj_elec = lv_label_create(lv_layer_top(), NULL);
	lv_obj_set_style(textHeaderObj_elec, &styleText_elecAnodenum);
	lv_label_set_text(textHeaderObj_elec, "0.0kWh");
	lv_obj_set_protect(textHeaderObj_elec, LV_PROTECT_POS);
	lv_label_set_long_mode(textHeaderObj_elec, LV_LABEL_LONG_SCROLL);
	lv_obj_set_size(textHeaderObj_elec, 20, 11);
	lv_obj_set_pos(textHeaderObj_elec, 167, 7);

	textHeaderObj_meshNodeNum = lv_label_create(lv_layer_top(), NULL);
	lv_label_set_text(textHeaderObj_meshNodeNum, "01");
	lv_obj_set_pos(textHeaderObj_meshNodeNum, 46, 7);
	lv_obj_set_style(textHeaderObj_meshNodeNum, &styleText_elecAnodenum);

	textHeaderObj_meshRole = lv_label_create(lv_layer_top(), NULL);
	lv_label_set_text(textHeaderObj_meshRole, "x");
	lv_obj_set_protect(textHeaderObj_meshRole, LV_PROTECT_POS);
	lv_obj_align(textHeaderObj_meshRole, textHeaderObj_meshNodeNum, LV_ALIGN_OUT_TOP_MID, 0, 5);
	lv_obj_set_style(textHeaderObj_meshNodeNum, &styleText_elecAnodenum);

	textHeaderObj_temperature = lv_label_create(lv_layer_top(), NULL);
	lv_label_set_text(textHeaderObj_temperature, "#0.0\"C");
	lv_obj_set_pos(textHeaderObj_temperature, 170, 23);
	lv_obj_set_style(textHeaderObj_temperature, &styleText_temperature);

	lvGui_businessHome(imageBK);

	xEventGp_screenTouch = xEventGroupCreate();

	xTaskCreate(task_guiSwitch_Detecting,	 //Task Function
				"guiDetect", //Task Name
				1024 * 4,	 //Stack Depth
				NULL,		 //Parameters
				1,			 //Priority
				NULL);		 //Task Handler

	usrAppHomepageThemeType_Set(homepageThemeType_typeFlg, false); //数据自更新，因为guiInit在dataInit之后，对象创建较晚

//	vTaskDelay(3000 / portTICK_PERIOD_MS);
//	lv_obj_clean(imageBK);

//	printf("Qr code creat res:%d.\n", EncodeData("hellow, Lanbon!"));

//	for(uint8_t loopa = 0; loopa < MAX_MODULESIZE; loopa ++){

//		for(uint8_t loopb = 0; loopb < MAX_MODULESIZE; loopb ++){

//			(m_byModuleData[loopa][loopb])?
//				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_BLACK)):
//				(externSocket_ex_disp_fill(QR_BASIC_POSITION_X + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + loopb * QR_PIXEL_SIZE, QR_BASIC_POSITION_X + QR_PIXEL_SIZE + loopa * QR_PIXEL_SIZE, QR_BASIC_POSITION_Y + QR_PIXEL_SIZE + loopb * QR_PIXEL_SIZE, LV_COLOR_WHITE));
//		}
//	}

//	lv_obj_t * btn = lv_imgbtn_create(lv_scr_act(), NULL);
//	lv_imgbtn_set_src(btn, LV_BTN_STATE_PR, &testPic_P);
//	lv_imgbtn_set_src(btn, LV_BTN_STATE_REL, &testPic);

//	for(uint8_t loop = 0; loop < 20; loop ++){

//		vTaskDelay(1000 / portTICK_PERIOD_MS);
//		(!(loop % 2))?(lv_img_set_src(la, &testPic)):(lv_img_set_src(la, &testPic_P));
//	}
//	
//	lv_obj_set_width(wp, LV_HOR_RES * 4);
//	lv_obj_set_protect(wp, LV_PROTECT_POS);

//	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
//	lv_obj_set_pos(label, 20, 100);
//	lv_label_set_text(label, "1245AE6BCC");
}



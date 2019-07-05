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
#include "gui_businessMenu_other.h"
#include "gui_businessMenu_wifiConfig.h"
#include "gui_businessMenu_delayer.h"
#include "gui_businessMenu_timer.h"
#include "gui_businessMenu_linkageConfig.h"
#include "gui_businessMenu_setting.h"
#include "gui_businessMenu_settingSet.h"
#include "gui_businessMenu_timerSet.h"
#include "gui_businessMenu_delayerSet.h"
#include "gui_businessMenu_timer.h"
#include "gui_businessMenu_delayer.h"

#include "devDataManage.h"
#include "devDriver_manage.h"

#include "bussiness_timerSoft.h"

typedef void (*guiHomeFun_local_bussiness)(lv_obj_t * obj_Parent);

LV_FONT_DECLARE(lv_font_arialNum_100);
LV_FONT_DECLARE(lv_font_dejavu_40);
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
//LV_IMG_DECLARE(homepage_bkPic_danYa);
LV_IMG_DECLARE(homepage_bkPic_jiJian);
LV_IMG_DECLARE(homepage_bkPic_jianJie);
LV_IMG_DECLARE(homepage_bkPic_keAi);
//LV_IMG_DECLARE(homepage_bkPic_ouZhou);

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
LV_IMG_DECLARE(iconPage_curtainClose_rel);
LV_IMG_DECLARE(iconPage_curtainPluse_rel);
LV_IMG_DECLARE(iconPage_curtainOpen_rel);
LV_IMG_DECLARE(iconPage_binding);
LV_IMG_DECLARE(iconPage_unbinding);
LV_IMG_DECLARE(imageCurtain_gan);
LV_IMG_DECLARE(imageCurtain_body);

LV_IMG_DECLARE(iconLightA_HomePageDeviceDimmer);
LV_IMG_DECLARE(iconLightB_HomePageDeviceDimmer);
LV_IMG_DECLARE(iconFans_HomePageDeviceFans);
LV_IMG_DECLARE(iconCurtain_HomePageDeviceCurtain);
LV_IMG_DECLARE(iconHourglass_HomePageDeviceHeater);
LV_IMG_DECLARE(iconHeater_HomePageDeviceHeater);
LV_IMG_DECLARE(iconSet_HomePageDeviceHeater);

//业务变量
extern xQueueHandle msgQh_systemRestartDelayCounterTips;
extern EventGroupHandle_t xEventGp_tipsLoopTimer;
extern uint8_t devRunningTimeFromPowerUp_couter;

xQueueHandle msgQh_wifiConfigCompleteTips = NULL;
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

static lv_obj_t *imageBK = NULL;

static lv_obj_t *imageTips_timer = NULL;

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

static lv_obj_t *label_sysRestartTips_ref = NULL;
static lv_obj_t *label_sysRestartTips_Counter = NULL;
static lv_obj_t *page_sysRestartTips = NULL;

static lv_obj_t *label_bk_devDimmer = NULL;
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
static lv_obj_t *image_bk_devCurtain_body = NULL;
static lv_obj_t *slider_bk_devCurtain = NULL;
static lv_obj_t *btnm_bk_devFans = NULL;
static lv_obj_t *textInstract_devFans = NULL;
static lv_obj_t *icomImage_devFans = NULL;
static lv_obj_t *btnm_bk_devHeater = NULL;
static lv_obj_t *imgBtn_devHeater_timeSet = NULL;
static lv_obj_t *page_timeSet_devHeater = NULL;
static lv_obj_t *rollerH_timeSetPage_devHeater = NULL;
static lv_obj_t *rollerM_timeSetPage_devHeater = NULL;
static lv_obj_t *rollerS_timeSetPage_devHeater = NULL;
static lv_obj_t *labelH_timeSetPage_devHeater = NULL;
static lv_obj_t *labelM_timeSetPage_devHeater = NULL;
static lv_obj_t *labelS_timeSetPage_devHeater = NULL;
static lv_obj_t *labelTitle_timeSetPage_devHeater = NULL;
static lv_obj_t *labelConfirm_timeSetPage_devHeater = NULL;
static lv_obj_t *labelCancel_timeSetPage_devHeater = NULL;
static lv_obj_t *btnConfirm_timeSetPage_devHeater = NULL;
static lv_obj_t *btnCancel_timeSetPage_devHeater = NULL;
static lv_obj_t *textTimeInstract_target_devHeater = NULL;
static lv_obj_t *textTimeInstract_current_devHeater = NULL;
static lv_obj_t *icomImageA_devHeater = NULL;
static lv_obj_t *lmeterTempInstTarget_devThermostat = NULL;
static lv_obj_t *lmeterTempInstCurrent_devThermostat = NULL;
static lv_obj_t *labelTempInstTarget_devThermostat = NULL;
static lv_obj_t *labelTempInstCurrent_devThermostat = NULL;
static lv_obj_t *slider_tempAdj_devThermostat = NULL;
static lv_obj_t *btn_tempAdjAdd_devThermostat = NULL;
static lv_obj_t *textBtn_tempAdjAdd_devThermostat = NULL;
static lv_obj_t *btn_tempAdjCut_devThermostat = NULL;
static lv_obj_t *textBtn_tempAdjCut_devThermostat = NULL;
static lv_obj_t *sw_devRunningEnable_devThermostat = NULL;
static lv_obj_t *cb_devEcoEnable_devThermostat = NULL;
static lv_obj_t *preload_driverCalmDown_devScenario = NULL;

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
static lv_style_t styleText_devCurtain_Bk_positionCurr;
static lv_style_t styleText_devCurtain_Bk_positionTips;
static lv_style_t styleBtn_devCurtain_statusPre;
static lv_style_t styleBtn_devCurtain_statusRel;
static lv_style_t styleText_devCurtain_SliderBk;
static lv_style_t styleImageBk_devCurtain_bkImgBody;
static lv_style_t styleSliderBk_devCurtain_bg;
static lv_style_t styleSliderBk_devCurtain_indic;
static lv_style_t styleSliderBk_devCurtain_knob;
static lv_style_t styleImage_devFans_icon;
static lv_style_t styleText_devFans_instract;
static lv_style_t styleBtnm_devFans_btnBg;
static lv_style_t styleBtnm_devFans_btnRel;
static lv_style_t styleBtnm_devFans_btnPre;
static lv_style_t styleBtnm_devFans_btnTglRel;
static lv_style_t styleBtnm_devFans_btnTglPre;
static lv_style_t styleBtnm_devFans_btnIna;
static lv_style_t styleImage_devHeater_icon;
static lv_style_t styleText_devHeater_timeInst;
static lv_style_t styleBtnm_devHeater_btnBg;
static lv_style_t styleBtnm_devHeater_btnRel;
static lv_style_t styleBtnm_devHeater_btnPre;
static lv_style_t stylePage_devHeater_timeSet;
static lv_style_t styleTextBtn_devHeater_timeSetPage;
static lv_style_t styleTextRoller_devHeater_timeSetPage_bg;
static lv_style_t styleTextRoller_devHeater_timeSetPage_sel;
static lv_style_t styleTextRollerTips_devHeater_timeSetPage;
static lv_style_t styleLmeter_devThermostat_tempTarget;
static lv_style_t styleLmeter_devThermostat_tempCurrent;
static lv_style_t styleLabel_devThermostat_tempTarget;
static lv_style_t styleLabel_devThermostat_tempCurrent;
static lv_style_t styleSliderBk_devThermostat_bg;
static lv_style_t styleSliderBk_devThermostat_indic;
static lv_style_t styleSliderBk_devThermostat_knob;
static lv_style_t styleTextBtnBk_devThermostat_tempAdj;
static lv_style_t styleCb_devThermostat_EcoEn;
static lv_style_t stylePreload_devScenario_driverCalmDown;
static lv_style_t styleBtn_devScenario_driverCalmDown;
static lv_style_t styleImgBk_underlying;
static lv_style_t stylebtnBk_transFull;
static lv_style_t stylePage_sysRestartTips;
static lv_style_t styleLabelCounter_sysRestartTips;
static lv_style_t styleLabelRef_sysRestartTips;

//其他本地变量
static uint8_t homepageThemeType_typeFlg = homepageThemeType_ouZhou;

static char textStr_time[10] = {0};
static char textStr_nodeNum[5] = {0}; 
static char textStr_elecSum[20] = {0};
static char textStr_temperature[10] = {0}; 

static char str_sliderBkVal_devDimmer[5] = {0};
static char str_devRunningRemind_devHeater[15] = {0};
static char str_devParamPositionCur_devCurtain[5] = {0};
static char str_devParamPositionAdj_devCurtain[5] = {0};

static uint8_t btnBindingStatus_record[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0}; //按钮绑定图标本地比较记录缓存
static bool mutualCtrlTrigIf_A = false; 
static bool mutualCtrlTrigIf_B = false;
static bool mutualCtrlTrigIf_C = false;

const char *btnm_str_devHeater[DEVICE_HEATER_OPREAT_ACTION_NUM + 1] = {"X", "A", "B", "C", "D", ""};
const char *btnm_str_devFans[DEVICE_FANS_OPREAT_ACTION_NUM + 1] = {"C", "1", "2", "3", ""};

//不同类型开关所需的参照常量
static const struct _rollerDispConferenceTab_devFans{

	stt_devFans_opratAct opreatActCurrent;
	uint16_t optionSelect;

}objDevFans_btnmDispConferenceTab[DEVICE_FANS_OPREAT_ACTION_NUM] = { //风扇开关不同档位对应ui显示值参照表

	{fansOpreatAct_stop,		0},
	{fansOpreatAct_firstGear,	1},
	{fansOpreatAct_secondGear,	2},
	{fansOpreatAct_thirdGear,	3},
};

static const struct _rollerDispConferenceTab_devHeater{

	stt_devHeater_opratAct opreatActCurrent;
	uint16_t optionSelect;

}objDevHeater_btnmDispConferenceTab[DEVICE_HEATER_OPREAT_ACTION_NUM] = { //风扇开关不同档位对应ui显示值参照表

	{heaterOpreatAct_close,					0},
	{heaterOpreatAct_open,					1},
	{heaterOpreatAct_closeAfter30Min,		2},
	{heaterOpreatAct_closeAfter60Min,		3},
	{heaterOpreatAct_closeAfterTimeCustom,	4},
};

static void local_guiHomeBussiness_thermostat_EcoCb_creat(lv_obj_t * obj_Parent, bool cbVal);

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

//			usrApplication_systemRestartTrig(5); //debug

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

		case devTypeDef_scenario:{

			devDataPoint.devType_scenario.devScenario_opNum = 1;
			currentDev_dataPointSet(&devDataPoint, true, false, true);

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

		case devTypeDef_scenario:{

			devDataPoint.devType_scenario.devScenario_opNum = 2;
			currentDev_dataPointSet(&devDataPoint, true, false, true);
		
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

		case devTypeDef_scenario:{

			devDataPoint.devType_scenario.devScenario_opNum = 4;
			currentDev_dataPointSet(&devDataPoint, true, false, true);

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devThermostat_tempAdd(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};
	char tempDisp[5] = {0};

	currentDev_dataPointGet(&devDataPoint);
	
//	printf("add+.\n");

	if(devDataPoint.devType_thermostat.devThermostat_tempratureTarget == 0)
		devDataPoint.devType_thermostat.devThermostat_tempratureTarget = 16;

	if(devDataPoint.devType_thermostat.devThermostat_tempratureTarget < 32)
		devDataPoint.devType_thermostat.devThermostat_tempratureTarget += 1;

	currentDev_dataPointSet(&devDataPoint, true, false, true);
	
	lv_slider_set_value(slider_tempAdj_devThermostat, devDataPoint.devType_thermostat.devThermostat_tempratureTarget - 16);
	sprintf(tempDisp, "%02d", (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);
	lv_label_set_text(labelTempInstTarget_devThermostat, tempDisp);
	lv_lmeter_set_value(lmeterTempInstTarget_devThermostat, (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devThermostat_tempCut(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};
	char tempDisp[5] = {0};

	currentDev_dataPointGet(&devDataPoint);
	
//	printf("cut-.\n");

	if(devDataPoint.devType_thermostat.devThermostat_tempratureTarget == 0)
		devDataPoint.devType_thermostat.devThermostat_tempratureTarget = 16;

	if(devDataPoint.devType_thermostat.devThermostat_tempratureTarget > 16)
		devDataPoint.devType_thermostat.devThermostat_tempratureTarget -= 1;

	currentDev_dataPointSet(&devDataPoint, true, true, true);

	lv_slider_set_value(slider_tempAdj_devThermostat, devDataPoint.devType_thermostat.devThermostat_tempratureTarget - 16);
	sprintf(tempDisp, "%02d", (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);
	lv_label_set_text(labelTempInstTarget_devThermostat, tempDisp);
	lv_lmeter_set_value(lmeterTempInstTarget_devThermostat, (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_open(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 1;
	devDataPoint.devType_curtain.devCurtain_actMethod = 0; //按钮方式

	currentDev_dataPointSet(&devDataPoint, false, mutualCtrlTrigIf_A, true);

	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_stop(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 2;
	devDataPoint.devType_curtain.devCurtain_actMethod = 0; //按钮方式

	currentDev_dataPointSet(&devDataPoint, false, mutualCtrlTrigIf_A, true);

	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devCurtain_close(lv_obj_t *btn){

	stt_devDataPonitTypedef devDataPoint = {0};

	devDataPoint.devType_curtain.devCurtain_actEnumVal = 4;
	devDataPoint.devType_curtain.devCurtain_actMethod = 0; //按钮方式

	currentDev_dataPointSet(&devDataPoint, false, mutualCtrlTrigIf_A, true);

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
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_dimmer:
		case devTypeDef_curtain:{

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

static lv_res_t funCb_rollAction_devHeater_mainRoller(lv_obj_t *roller){

	uint16_t optionRollerSelect = lv_roller_get_selected(roller);

	for(uint8_t loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM; loop ++){

		if(optionRollerSelect == objDevHeater_btnmDispConferenceTab[loop].optionSelect){

			stt_devDataPonitTypedef devDataPoint = {0};

			devDataPoint.devType_heater.devHeater_swEnumVal = objDevHeater_btnmDispConferenceTab[loop].opreatActCurrent;

			currentDev_dataPointSet(&devDataPoint, true, true, true);	
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnmActionClick_devFans_gearBtnm(lv_obj_t *btnm, const char *txt){

	uint8_t loop = 0;

	for(loop = 0; loop < DEVICE_FANS_OPREAT_ACTION_NUM + 1; loop ++){

		if(!strcmp(txt, btnm_str_devFans[loop])){

			printf("btn %s has be selected.\n", txt);
			
			break;
		}
	}

	if(loop <= DEVICE_FANS_OPREAT_ACTION_NUM){
		
		stt_devDataPonitTypedef devDataPoint = {0};

		devDataPoint.devType_fans.devFans_swEnumVal = objDevFans_btnmDispConferenceTab[loop].opreatActCurrent;
		currentDev_dataPointSet(&devDataPoint, true, true, true);
		switch(loop){

			case fansOpreatAct_firstGear:{
			
				lv_label_set_text(textInstract_devFans, "1");
				styleImage_devFans_icon.image.color = LV_COLOR_MAKE(0, 128, 255);
				styleImage_devFans_icon.image.intense = LV_OPA_COVER;
				lv_obj_refresh_style(icomImage_devFans);
				lv_obj_set_pos(icomImage_devFans, 15, 70);
				lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
			
			}break;
			
			case fansOpreatAct_secondGear:{
			
				lv_label_set_text(textInstract_devFans, "2");
				styleImage_devFans_icon.image.color = LV_COLOR_MAKE(255, 255, 0);
				styleImage_devFans_icon.image.intense = LV_OPA_COVER;
				lv_obj_refresh_style(icomImage_devFans);
				lv_obj_set_pos(icomImage_devFans, 15, 70);
				lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
			
			}break;
			
			case fansOpreatAct_thirdGear:{
			
				lv_label_set_text(textInstract_devFans, "3");
				styleImage_devFans_icon.image.color = LV_COLOR_MAKE(255, 128, 0);
				styleImage_devFans_icon.image.intense = LV_OPA_COVER;
				lv_obj_refresh_style(icomImage_devFans);
				lv_obj_set_pos(icomImage_devFans, 15, 70);
				lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
			
			}break;
			
			case fansOpreatAct_stop:
			default:{
			
				lv_label_set_text(textInstract_devFans, "\0");
				styleImage_devFans_icon.image.color = LV_COLOR_MAKE(224, 224, 220);
				styleImage_devFans_icon.image.intense = LV_OPA_COVER;
				lv_obj_refresh_style(icomImage_devFans);
				lv_obj_set_pos(icomImage_devFans, 68, 70);
				lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
				
			}break;
		}
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devHeater_pageTimeSet_btnConfirm(lv_obj_t *btn){

	char dataDispTemp[15] = {0};
	uint32_t timeCountSet_temp = (lv_roller_get_selected(rollerH_timeSetPage_devHeater) * 3600UL) + 
								 (lv_roller_get_selected(rollerM_timeSetPage_devHeater) * 60UL) + 
								 (lv_roller_get_selected(rollerS_timeSetPage_devHeater) * 1UL);
	const stt_devDataPonitTypedef datapointTemp = {

		.devType_heater.devHeater_swEnumVal = heaterOpreatAct_closeAfterTimeCustom,
	};

	devDriverBussiness_heaterSwitch_closePeriodCustom_Set(timeCountSet_temp, true);
	devDriverBussiness_heaterSwitch_periphStatusReales(&datapointTemp);
	sprintf(dataDispTemp, "%02d:%02d:%02d", (int)lv_roller_get_selected(rollerH_timeSetPage_devHeater),
											(int)lv_roller_get_selected(rollerM_timeSetPage_devHeater),
											(int)lv_roller_get_selected(rollerS_timeSetPage_devHeater));
	lv_label_set_text(textTimeInstract_target_devHeater, dataDispTemp);

	if(page_timeSet_devHeater){

		lv_obj_del(page_timeSet_devHeater);
		page_timeSet_devHeater = NULL;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devHeater_pageTimeSet_btnCancel(lv_obj_t *btn){

	if(page_timeSet_devHeater){

		lv_obj_del(page_timeSet_devHeater);
		page_timeSet_devHeater = NULL;
	}

	return LV_RES_OK;
}

static lv_res_t funCb_btnActionClick_devHeater_timeSet(lv_obj_t *btn){

	uint32_t timeCountCustom = devDriverBussiness_heaterSwitch_closePeriodCustom_Get();

	if(page_timeSet_devHeater == NULL)
		page_timeSet_devHeater = lv_page_create(lv_scr_act(), NULL);

	lv_obj_set_size(page_timeSet_devHeater, 200, 200);
	lv_obj_set_pos(page_timeSet_devHeater, 20, 60);					
	lv_page_set_style(page_timeSet_devHeater, LV_PAGE_STYLE_SB, &stylePage_devHeater_timeSet);
	lv_page_set_style(page_timeSet_devHeater, LV_PAGE_STYLE_BG, &stylePage_devHeater_timeSet);
	lv_page_set_sb_mode(page_timeSet_devHeater, LV_SB_MODE_HIDE); 	
	lv_page_set_scrl_fit(page_timeSet_devHeater, false, true); //key opration
	lv_page_set_scrl_layout(page_timeSet_devHeater, LV_LAYOUT_PRETTY);

	labelTitle_timeSetPage_devHeater = lv_label_create(page_timeSet_devHeater, NULL);
	lv_label_set_recolor(labelTitle_timeSetPage_devHeater, true);
	lv_label_set_text(labelTitle_timeSetPage_devHeater, "#393939 time set#");
	lv_obj_set_protect(labelTitle_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_obj_align(labelTitle_timeSetPage_devHeater, page_timeSet_devHeater, LV_ALIGN_CENTER, 0, 0);
	
	rollerH_timeSetPage_devHeater = lv_roller_create(page_timeSet_devHeater, NULL);
	lv_roller_set_action(rollerH_timeSetPage_devHeater, NULL);
	lv_roller_set_options(rollerH_timeSetPage_devHeater, "00\n""01\n""02\n""03\n""04\n""05\n"
													     "06\n""07\n""08\n""09\n""10\n""11\n"
													     "12\n""13\n""14\n""15\n""16\n""17\n"
													     "18\n""19\n""20\n""21\n""22\n""23");
	lv_obj_set_protect(rollerH_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_roller_set_hor_fit(rollerH_timeSetPage_devHeater, false);
	lv_obj_set_width(rollerH_timeSetPage_devHeater, 40);
	lv_obj_align(rollerH_timeSetPage_devHeater, labelTitle_timeSetPage_devHeater, LV_ALIGN_OUT_BOTTOM_MID, -70, 20);
	lv_page_glue_obj(rollerH_timeSetPage_devHeater, true);
	lv_roller_set_selected(rollerH_timeSetPage_devHeater, timeCountCustom % (3600UL * 24UL) / 3600UL, false);

	lv_roller_set_style(rollerH_timeSetPage_devHeater, LV_ROLLER_STYLE_BG, &styleTextRoller_devHeater_timeSetPage_bg);
	lv_roller_set_style(rollerH_timeSetPage_devHeater, LV_ROLLER_STYLE_SEL, &styleTextRoller_devHeater_timeSetPage_sel);
	lv_roller_set_visible_row_count(rollerH_timeSetPage_devHeater, 4);

	rollerM_timeSetPage_devHeater = lv_roller_create(page_timeSet_devHeater, NULL);
	lv_roller_set_action(rollerM_timeSetPage_devHeater, NULL);
	lv_roller_set_options(rollerM_timeSetPage_devHeater, "00\n""01\n""02\n""03\n""04\n""05\n"
														 "06\n""07\n""08\n""09\n""10\n""11\n"
														 "12\n""13\n""14\n""15\n""16\n""17\n"
														 "18\n""19\n""20\n""21\n""22\n""23\n"
														 "24\n""25\n""26\n""27\n""28\n""29\n"
														 "30\n""31\n""32\n""33\n""34\n""35\n"
														 "36\n""37\n""38\n""39\n""40\n""41\n"
														 "42\n""43\n""44\n""45\n""46\n""47\n"
														 "48\n""49\n""50\n""51\n""52\n""53\n"
														 "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(rollerM_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_roller_set_hor_fit(rollerM_timeSetPage_devHeater, false);
	lv_obj_set_width(rollerM_timeSetPage_devHeater, 40);
	lv_obj_align(rollerM_timeSetPage_devHeater, rollerH_timeSetPage_devHeater, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
	lv_page_glue_obj(rollerM_timeSetPage_devHeater, true);
	lv_roller_set_selected(rollerM_timeSetPage_devHeater, timeCountCustom % 3600UL / 60, false);

	lv_roller_set_style(rollerM_timeSetPage_devHeater, LV_ROLLER_STYLE_BG, &styleTextRoller_devHeater_timeSetPage_bg);
	lv_roller_set_style(rollerM_timeSetPage_devHeater, LV_ROLLER_STYLE_SEL, &styleTextRoller_devHeater_timeSetPage_sel);
	lv_roller_set_visible_row_count(rollerM_timeSetPage_devHeater, 4);

	rollerS_timeSetPage_devHeater = lv_roller_create(page_timeSet_devHeater, NULL);
	lv_roller_set_action(rollerS_timeSetPage_devHeater, NULL);
	lv_roller_set_options(rollerS_timeSetPage_devHeater, "00\n""01\n""02\n""03\n""04\n""05\n"
														 "06\n""07\n""08\n""09\n""10\n""11\n"
														 "12\n""13\n""14\n""15\n""16\n""17\n"
														 "18\n""19\n""20\n""21\n""22\n""23\n"
														 "24\n""25\n""26\n""27\n""28\n""29\n"
														 "30\n""31\n""32\n""33\n""34\n""35\n"
														 "36\n""37\n""38\n""39\n""40\n""41\n"
														 "42\n""43\n""44\n""45\n""46\n""47\n"
														 "48\n""49\n""50\n""51\n""52\n""53\n"
														 "54\n""55\n""56\n""57\n""58\n""59");
	lv_obj_set_protect(rollerS_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_roller_set_hor_fit(rollerS_timeSetPage_devHeater, false);
	lv_obj_set_width(rollerS_timeSetPage_devHeater, 40);
	lv_obj_align(rollerS_timeSetPage_devHeater, rollerM_timeSetPage_devHeater, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
	lv_page_glue_obj(rollerS_timeSetPage_devHeater, true);
	lv_roller_set_selected(rollerS_timeSetPage_devHeater, timeCountCustom % 60UL, false);

	lv_roller_set_style(rollerS_timeSetPage_devHeater, LV_ROLLER_STYLE_BG, &styleTextRoller_devHeater_timeSetPage_bg);
	lv_roller_set_style(rollerS_timeSetPage_devHeater, LV_ROLLER_STYLE_SEL, &styleTextRoller_devHeater_timeSetPage_sel);
	lv_roller_set_visible_row_count(rollerS_timeSetPage_devHeater, 4);
	
	labelH_timeSetPage_devHeater = lv_label_create(page_timeSet_devHeater, NULL);
	lv_label_set_text(labelH_timeSetPage_devHeater, "hour");
	lv_obj_set_protect(labelH_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_obj_align(labelH_timeSetPage_devHeater, rollerH_timeSetPage_devHeater, LV_ALIGN_IN_RIGHT_MID, 35, 3);
	lv_obj_set_style(labelH_timeSetPage_devHeater, &styleTextRollerTips_devHeater_timeSetPage);

	labelM_timeSetPage_devHeater = lv_label_create(page_timeSet_devHeater, labelH_timeSetPage_devHeater);
	lv_label_set_text(labelM_timeSetPage_devHeater, "min");
	lv_obj_set_protect(labelM_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_obj_align(labelM_timeSetPage_devHeater, rollerM_timeSetPage_devHeater, LV_ALIGN_IN_RIGHT_MID, 15, 0);
	lv_obj_set_style(labelM_timeSetPage_devHeater, &styleTextRollerTips_devHeater_timeSetPage);

	labelS_timeSetPage_devHeater = lv_label_create(page_timeSet_devHeater, labelH_timeSetPage_devHeater);
	lv_label_set_text(labelS_timeSetPage_devHeater, "sec");
	lv_obj_set_protect(labelS_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_obj_align(labelS_timeSetPage_devHeater, rollerS_timeSetPage_devHeater, LV_ALIGN_IN_RIGHT_MID, 15, 0);
	lv_obj_set_style(labelS_timeSetPage_devHeater, &styleTextRollerTips_devHeater_timeSetPage);

	btnConfirm_timeSetPage_devHeater = lv_btn_create(page_timeSet_devHeater, NULL);
    lv_btn_set_style(btnConfirm_timeSetPage_devHeater, LV_BTN_STYLE_REL, &stylebtnBk_transFull);
    lv_btn_set_style(btnConfirm_timeSetPage_devHeater, LV_BTN_STYLE_PR, &stylebtnBk_transFull);
    lv_btn_set_style(btnConfirm_timeSetPage_devHeater, LV_BTN_STYLE_TGL_REL, &stylebtnBk_transFull);
    lv_btn_set_style(btnConfirm_timeSetPage_devHeater, LV_BTN_STYLE_TGL_PR, &stylebtnBk_transFull);
	lv_btn_set_fit(btnConfirm_timeSetPage_devHeater, false, false);
	lv_obj_set_size(btnConfirm_timeSetPage_devHeater, 70, 25);
	lv_page_glue_obj(btnConfirm_timeSetPage_devHeater, true);
	lv_obj_set_protect(btnConfirm_timeSetPage_devHeater, LV_PROTECT_FOLLOW);
	lv_obj_set_protect(btnConfirm_timeSetPage_devHeater, LV_PROTECT_POS);
	lv_obj_align(btnConfirm_timeSetPage_devHeater, page_timeSet_devHeater, LV_ALIGN_IN_BOTTOM_LEFT, 15, -1);
	lv_btn_set_action(btnConfirm_timeSetPage_devHeater, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devHeater_pageTimeSet_btnConfirm);
	btnCancel_timeSetPage_devHeater = lv_btn_create(page_timeSet_devHeater, btnConfirm_timeSetPage_devHeater);
	lv_obj_align(btnCancel_timeSetPage_devHeater, btnConfirm_timeSetPage_devHeater, LV_ALIGN_CENTER, 95, 0);
	lv_btn_set_action(btnCancel_timeSetPage_devHeater, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devHeater_pageTimeSet_btnCancel);

	labelConfirm_timeSetPage_devHeater = lv_label_create(btnConfirm_timeSetPage_devHeater, NULL);
	lv_obj_set_style(labelConfirm_timeSetPage_devHeater, &styleTextBtn_devHeater_timeSetPage);
	lv_label_set_text(labelConfirm_timeSetPage_devHeater, "confirm");
	labelCancel_timeSetPage_devHeater = lv_label_create(btnCancel_timeSetPage_devHeater, labelConfirm_timeSetPage_devHeater);
	lv_obj_set_style(labelCancel_timeSetPage_devHeater, &styleTextBtn_devHeater_timeSetPage);
	lv_label_set_text(labelCancel_timeSetPage_devHeater, "cancel");

	return LV_RES_OK;
}

static lv_res_t funCb_btnmActionClick_devHeater_gearBtnm(lv_obj_t *btnm, const char *txt){

	uint8_t loop = 0;

//	printf("btnm toggled %d is trig.\n", lv_btnm_get_toggled(btnm));

	for(loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM + 1; loop ++){

		if(!strcmp(txt, btnm_str_devHeater[loop])){

			printf("btn %s has be selected.\n", txt);
			break;
		}
	}

	if(loop <= DEVICE_HEATER_OPREAT_ACTION_NUM){

		stt_devDataPonitTypedef devDataPoint = {0};

		devDataPoint.devType_heater.devHeater_swEnumVal = objDevHeater_btnmDispConferenceTab[loop].opreatActCurrent;
		currentDev_dataPointSet(&devDataPoint, true, true, true);

		if(devDataPoint.devType_heater.devHeater_swEnumVal == heaterOpreatAct_closeAfterTimeCustom){

			if(imgBtn_devHeater_timeSet == NULL)
				imgBtn_devHeater_timeSet = lv_imgbtn_create(lv_obj_get_parent(btnm), NULL);
			lv_obj_set_protect(imgBtn_devHeater_timeSet, LV_PROTECT_POS);
			lv_obj_set_pos(imgBtn_devHeater_timeSet, 185, 205);
			lv_imgbtn_set_src(imgBtn_devHeater_timeSet, LV_BTN_STATE_REL, &iconSet_HomePageDeviceHeater);
			lv_imgbtn_set_src(imgBtn_devHeater_timeSet, LV_BTN_STATE_PR, &iconSet_HomePageDeviceHeater);
			lv_imgbtn_set_action(imgBtn_devHeater_timeSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devHeater_timeSet);
		}
		else
		{
			if(imgBtn_devHeater_timeSet){

				lv_obj_del(imgBtn_devHeater_timeSet);
				imgBtn_devHeater_timeSet = NULL;
			}
		}
		
		switch(devDataPoint.devType_heater.devHeater_swEnumVal){
			
			case heaterOpreatAct_open:{

				lv_label_set_text(textTimeInstract_target_devHeater, "\0");
				lv_label_set_text(textTimeInstract_current_devHeater, "OPEN");

			}break;
			
			case heaterOpreatAct_closeAfter30Min:{

				lv_label_set_text(textTimeInstract_target_devHeater, "00:30:00");
				lv_label_set_text(textTimeInstract_current_devHeater, "00:30:00");

			}break;
			
			case heaterOpreatAct_closeAfter60Min:{

				lv_label_set_text(textTimeInstract_target_devHeater, "00:60:00");
				lv_label_set_text(textTimeInstract_current_devHeater, "00:60:00");

			}break;
			
			case heaterOpreatAct_closeAfterTimeCustom:{

				char dataDispTemp[15] = {0};
				uint32_t closeCounterTemp = devDriverBussiness_heaterSwitch_closePeriodCustom_Get();

				sprintf(dataDispTemp, "%02d:%02d:%02d", (int)((closeCounterTemp % 86400UL) / 3600UL),
														(int)((closeCounterTemp % 3600UL) / 60UL),
														(int)(closeCounterTemp % 60UL));
				lv_label_set_text(textTimeInstract_target_devHeater, dataDispTemp);
				lv_label_set_text(textTimeInstract_current_devHeater, dataDispTemp);

			}break;
			
			case heaterOpreatAct_close:
			default:{

				lv_label_set_text(textTimeInstract_target_devHeater, "\0");
				lv_label_set_text(textTimeInstract_current_devHeater, "CLOSE");

			}break;
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

	sprintf(str_sliderBkVal_devDimmer, "%d%%", lv_slider_get_value(slider));
	lv_label_set_text(label_bk_devDimmer, str_sliderBkVal_devDimmer);
	currentDev_dataPointSet(&devDataPoint, true, true, true);

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_devCurtain_mainSlider(lv_obj_t *slider){

	uint8_t orbitalTimePercent = (uint8_t)lv_slider_get_value(slider);
	stt_devDataPonitTypedef devDataPoint = {0};

	usrApp_ctrlObjSlidlingTrig(); //滑动冷却触发

	devDataPoint.devType_curtain.devCurtain_actEnumVal = orbitalTimePercent;
	devDataPoint.devType_curtain.devCurtain_actMethod = 1; //滑条方式

	currentDev_dataPointSet(&devDataPoint, false, mutualCtrlTrigIf_A, true);
	sprintf(str_devParamPositionAdj_devCurtain, "%d%%", orbitalTimePercent);
	lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);

	return LV_RES_OK;
}

static lv_res_t funCb_slidAction_devThermostat_mainSlider(lv_obj_t *slider){

	uint8_t tempTarget = (uint8_t)lv_slider_get_value(slider);
	char tempDisp[5] = {0};
	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	usrApp_ctrlObjSlidlingTrig(); //滑动冷却触发

	devDataPoint.devType_thermostat.devThermostat_tempratureTarget = 16 + tempTarget;

	currentDev_dataPointSet(&devDataPoint, true, false, true);	

	lv_lmeter_set_value(lmeterTempInstTarget_devThermostat, tempTarget + 16);
	sprintf(tempDisp, "%02d", tempTarget + 16);
	lv_label_set_text(labelTempInstTarget_devThermostat, tempDisp);

	return LV_RES_OK;
}

static void usrApp_devThermostat_ctrlObj_reserveSet(bool reserve_IF){

	if(reserve_IF){

		lv_obj_set_click(slider_tempAdj_devThermostat, true);
		lv_obj_set_click(btn_tempAdjAdd_devThermostat, true);
		lv_obj_set_click(btn_tempAdjCut_devThermostat, true);
		
		styleLmeter_devThermostat_tempTarget.line.color = LV_COLOR_WHITE;
		styleLmeter_devThermostat_tempTarget.body.main_color = LV_COLOR_MAKE(0, 0, 255);
		styleLmeter_devThermostat_tempTarget.body.grad_color = LV_COLOR_MAKE(0, 0, 255);
		styleLmeter_devThermostat_tempCurrent.line.color = LV_COLOR_WHITE;
		styleLmeter_devThermostat_tempCurrent.body.main_color = LV_COLOR_MAKE(0, 255, 0);
		styleLmeter_devThermostat_tempCurrent.body.grad_color = LV_COLOR_MAKE(0, 255, 0);
		styleLabel_devThermostat_tempTarget.text.color = LV_COLOR_MAKE(255, 255, 0);
		styleLabel_devThermostat_tempCurrent.text.color = LV_COLOR_MAKE(0, 255, 0);
	}
	else
	{
		lv_obj_set_click(slider_tempAdj_devThermostat, false);
		lv_obj_set_click(btn_tempAdjAdd_devThermostat, false);
		lv_obj_set_click(btn_tempAdjCut_devThermostat, false);

		styleLmeter_devThermostat_tempTarget.line.color = LV_COLOR_GRAY;
		styleLmeter_devThermostat_tempTarget.body.main_color = LV_COLOR_GRAY;
		styleLmeter_devThermostat_tempTarget.body.grad_color = LV_COLOR_GRAY;
		styleLmeter_devThermostat_tempCurrent.line.color = LV_COLOR_GRAY;
		styleLmeter_devThermostat_tempCurrent.body.main_color = LV_COLOR_GRAY;
		styleLmeter_devThermostat_tempCurrent.body.grad_color = LV_COLOR_GRAY;
		styleLabel_devThermostat_tempTarget.text.color = LV_COLOR_GRAY;
		styleLabel_devThermostat_tempCurrent.text.color = LV_COLOR_GRAY;
	}

	lv_obj_refresh_style(lmeterTempInstTarget_devThermostat);
	lv_obj_refresh_style(lmeterTempInstCurrent_devThermostat);
	lv_obj_refresh_style(labelTempInstTarget_devThermostat);
	lv_obj_refresh_style(labelTempInstCurrent_devThermostat);
}

static lv_res_t funCb_swAction_devThermostat_runningEnable(lv_obj_t *sw){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	if(lv_sw_get_state(sw)){

		usrApp_devThermostat_ctrlObj_reserveSet(true);
		devDataPoint.devType_thermostat.devThermostat_running_en = true;
		local_guiHomeBussiness_thermostat_EcoCb_creat(lv_obj_get_parent(sw), devDataPoint.devType_thermostat.devThermostat_nightMode_en);
	}
	else
	{
		usrApp_devThermostat_ctrlObj_reserveSet(false);
		devDataPoint.devType_thermostat.devThermostat_running_en = false;
		lv_obj_del(cb_devEcoEnable_devThermostat);
		cb_devEcoEnable_devThermostat = NULL;
	}

	currentDev_dataPointSet(&devDataPoint, true, false, true);

	return LV_RES_OK;
}

static lv_res_t funCb_cbChecked_devThermostat_EcoEnable(lv_obj_t *cb){

	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	(lv_cb_is_checked(cb))?
		(devDataPoint.devType_thermostat.devThermostat_nightMode_en = 1):
		(devDataPoint.devType_thermostat.devThermostat_nightMode_en = 0);

	currentDev_dataPointSet(&devDataPoint, true, false, true);

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
			lv_obj_refresh_style(textHeaderObj_time);
			vTaskDelay(20 / portTICK_PERIOD_MS);
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
			lv_obj_refresh_style(textHeaderObj_elec);
			vTaskDelay(20 / portTICK_PERIOD_MS);
		}
	}

	{//页眉温度显示更新

		float temperature_temp = devDriverBussiness_temperatureMeasure_get();
		static float temperature_record = 0.0F;

		if(temperature_record != temperature_temp){

			temperature_record = temperature_temp;

			sprintf(textStr_temperature, "%3.02f\"C", temperature_record);
			lv_label_set_text(textHeaderObj_temperature, textStr_temperature);	
			lv_obj_refresh_style(textHeaderObj_temperature);
			vTaskDelay(20 / portTICK_PERIOD_MS);
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

								lv_obj_refresh_style(textBtn_meeting);

							}break;
							
							case devTypeDef_mulitSwTwoBit:{

								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 0))
									lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 1))
									lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);

								lv_obj_refresh_style(textBtn_meeting);
								lv_obj_refresh_style(textBtn_sleeping);

							}break;
							
							case devTypeDef_mulitSwThreeBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 0))
									lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 1))
									lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold & (1 << 2))
									lv_label_set_text(textBtn_toilet, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[2]);

								lv_obj_refresh_style(textBtn_meeting);
								lv_obj_refresh_style(textBtn_sleeping);
								lv_obj_refresh_style(textBtn_toilet);

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
								
								lv_obj_refresh_style(iconBtn_meeting);
							}break;
							
							case devTypeDef_mulitSwTwoBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 0))
									lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 1))
									lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));

								lv_obj_refresh_style(iconBtn_meeting);
								lv_obj_refresh_style(iconBtn_sleeping);
							}break;
							
							case devTypeDef_mulitSwThreeBit:{
								
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 0))
									lv_img_set_src(iconBtn_meeting, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[0]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 1))
									lv_img_set_src(iconBtn_sleeping, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[1]));
								if(rptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold & (1 << 2))
									lv_img_set_src(iconBtn_toilet, usrAppHomepageBtnIconDisp_dataGet(dataIconObjDisp_temp[2]));

								lv_obj_refresh_style(iconBtn_meeting);
								lv_obj_refresh_style(iconBtn_sleeping);
								lv_obj_refresh_style(iconBtn_toilet);
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

									lv_obj_refresh_style(btn_bk_devMulitSw_A);
									lv_obj_refresh_style(iconBtn_meeting);
									lv_obj_refresh_style(textBtn_meeting);
									
								}break;
								
								case devTypeDef_mulitSwTwoBit:{
							
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit1)?(lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true))):(lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false)));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff));
									(devDataPoint.devType_mulitSwitch_threeBit.swVal_bit2)?(lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn)):(lv_img_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff));

									lv_obj_refresh_style(btn_bk_devMulitSw_A);
									lv_obj_refresh_style(iconBtn_meeting);
									lv_obj_refresh_style(textBtn_meeting);
									lv_obj_refresh_style(btn_bk_devMulitSw_B);
									lv_obj_refresh_style(iconBtn_sleeping);
									lv_obj_refresh_style(textBtn_sleeping);
	
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

									lv_obj_refresh_style(btn_bk_devMulitSw_A);
									lv_obj_refresh_style(iconBtn_meeting);
									lv_obj_refresh_style(textBtn_meeting);
									lv_obj_refresh_style(btn_bk_devMulitSw_B);
									lv_obj_refresh_style(iconBtn_sleeping);
									lv_obj_refresh_style(textBtn_sleeping);
									lv_obj_refresh_style(btn_bk_devMulitSw_C);
									lv_obj_refresh_style(iconBtn_toilet);
									lv_obj_refresh_style(textBtn_toilet);

								}break;
							
								default:break;
							}
						}

						if(imageBK)
							lv_img_set_src(imageBK, usrAppHomepageBkPic_dataGet());
						lv_obj_refresh_style(imageBK);
						
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

					case devTypeDef_curtain:{

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

									iconBtn_binding_A = lv_imgbtn_create(imageBK, NULL);
									mutualCtrlTrigIf_A = true;
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnA);
									lv_obj_set_protect(iconBtn_binding_A, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_A, slider_bk_devCurtain, LV_ALIGN_OUT_TOP_LEFT, 0, -5);
								}
							}
						}

					}break;

					case devTypeDef_dimmer:{

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

									iconBtn_binding_A = lv_imgbtn_create(imageBK, NULL);
									mutualCtrlTrigIf_A = true;
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_REL, &iconPage_binding);
									lv_imgbtn_set_src(iconBtn_binding_A, LV_BTN_STATE_PR, &iconPage_unbinding);
									lv_btn_set_action(iconBtn_binding_A, LV_BTN_ACTION_CLICK, funCb_btnActionClick_bindingBtnA);
									lv_obj_set_protect(iconBtn_binding_A, LV_PROTECT_POS);
									lv_obj_align(iconBtn_binding_A, slider_bk_devDimmer, LV_ALIGN_OUT_TOP_RIGHT, 0, -5);
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

				case devTypeDef_scenario:{

					lv_obj_t *preloadParent = NULL;
					static bool scenarioDriver_preload_trigFlg = false;
					uint16_t scenarioDriverClamDown_counter = devDriverBussiness_scnarioSwitch_driverClamDown_get();
					stt_devDataPonitTypedef devDataPoint = {0};

					currentDev_dataPointGet(&devDataPoint);

					switch(devDataPoint.devType_scenario.devScenario_opNum){
					
						case 1:preloadParent = btn_bk_devMulitSw_A;break;
					
						case 2:preloadParent = btn_bk_devMulitSw_B;break;
					
						case 4:
						default:preloadParent = btn_bk_devMulitSw_C;break;
					}

					if(!scenarioDriver_preload_trigFlg){

						if(scenarioDriverClamDown_counter){

							scenarioDriver_preload_trigFlg = true;

							lv_obj_set_click(btn_bk_devMulitSw_A, false);
							lv_obj_set_click(btn_bk_devMulitSw_B, false);
							lv_obj_set_click(btn_bk_devMulitSw_C, false);
						
							if(devDataPoint.devType_scenario.devScenario_opNum){

								preload_driverCalmDown_devScenario = lv_preload_create(preloadParent, NULL);
								lv_obj_set_size(preload_driverCalmDown_devScenario, 50, 50);
								lv_obj_set_protect(preload_driverCalmDown_devScenario, LV_PROTECT_POS);
								lv_obj_align(preload_driverCalmDown_devScenario, preloadParent, LV_ALIGN_CENTER, 0, 0);
								lv_preload_set_spin_time(preload_driverCalmDown_devScenario, 750);
								lv_preload_set_style(preload_driverCalmDown_devScenario, LV_PRELOAD_STYLE_MAIN, &stylePreload_devScenario_driverCalmDown);
							}
						}
					}
					else
					{
						static uint8_t calmDownWarnning_counter = 0;
						static bool calmDownWarnning_flg = false;
						const uint8_t calmDownWarnning_period = 3;

						if(calmDownWarnning_counter < calmDownWarnning_period)calmDownWarnning_counter ++;
						else{

							calmDownWarnning_counter = 0;
							calmDownWarnning_flg = !calmDownWarnning_flg;

							(calmDownWarnning_flg)?
								(lv_imgbtn_set_style(preloadParent, LV_BTN_STATE_REL, &styleBtn_devScenario_driverCalmDown)):
								(lv_imgbtn_set_style(preloadParent, LV_BTN_STATE_REL, &lv_style_plain));
						}
					
						if(!scenarioDriverClamDown_counter){

							scenarioDriver_preload_trigFlg = false;;

							lv_obj_set_click(btn_bk_devMulitSw_A, true);
							lv_obj_set_click(btn_bk_devMulitSw_B, true);
							lv_obj_set_click(btn_bk_devMulitSw_C, true);
						
							if(preload_driverCalmDown_devScenario)
								lv_obj_del(preload_driverCalmDown_devScenario);
							preload_driverCalmDown_devScenario = NULL;

							lv_imgbtn_set_style(preloadParent, LV_BTN_STATE_REL, &lv_style_plain);
							
							lv_obj_refresh_style(btn_bk_devMulitSw_A);
							lv_obj_refresh_style(btn_bk_devMulitSw_B);
							lv_obj_refresh_style(btn_bk_devMulitSw_C);
						}
					}

				}break;

				case devTypeDef_curtain:{

					stt_msgDats_devCurtainDriver rptr_msgQ_devCurtainDriver = {0};

					if(xQueueReceive(msgQh_devCurtainDriver, &rptr_msgQ_devCurtainDriver, 10 / portTICK_RATE_MS) == pdTRUE){

						switch(rptr_msgQ_devCurtainDriver.msgType){

							case msgType_devCurtainDriver_opreatStop:{ //窗帘驱动，回弹至停止消息接收后进行UI更新

								if(rptr_msgQ_devCurtainDriver.msgDats.data_opreatStop.opreatStop_sig){

									uint8_t devCurtain_orbitalPosPercent = devCurtain_currentPositionPercentGet();
//									lv_coord_t imageCurtainPosAdj_temp = devCurtain_orbitalPosPercent * 12;

//									imageCurtainPosAdj_temp /= 10;
//									lv_obj_set_pos(image_bk_devCurtain_bodyLeft, 2 - imageCurtainPosAdj_temp, 75);
//									lv_obj_set_pos(image_bk_devCurtain_bodyRight, 120 + imageCurtainPosAdj_temp, 75);

									//滑块及文字控件再刷新，防止信息同步不完整
									lv_slider_set_value(slider_bk_devCurtain, devCurtain_orbitalPosPercent);
									sprintf(str_devParamPositionCur_devCurtain, "%d", devCurtain_orbitalPosPercent);
									lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
									sprintf(str_devParamPositionAdj_devCurtain, "%d%%", devCurtain_orbitalPosPercent);
									lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);

									lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
									lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusPre);
									lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
								}

							}break;
							
							case msgType_devCurtainDriver_orbitalChgingByBtn:{

//								lv_coord_t imageCurtainPosAdj_temp = 12 * rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent;
//								static lv_coord_t imageCurtainPosAdj_record = 0;

//								imageCurtainPosAdj_temp /= 10;
//								if(!(imageCurtainPosAdj_temp % 9)){

//									lv_obj_set_pos(image_bk_devCurtain_bodyLeft, 2 - imageCurtainPosAdj_temp, 75);
//									lv_obj_set_pos(image_bk_devCurtain_bodyRight, 120 + imageCurtainPosAdj_temp, 75);
//								}

								lv_slider_set_value(slider_bk_devCurtain, rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								sprintf(str_devParamPositionAdj_devCurtain, "%d%%", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
								sprintf(str_devParamPositionCur_devCurtain, "%d", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent);
								lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);

							}break;

							case msgType_devCurtainDriver_orbitalChgingBySlider:{
								
								sprintf(str_devParamPositionCur_devCurtain, "%d", rptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingBySlider.orbitalPosPercent);
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

										sprintf(str_devRunningRemind_devHeater, "CLOSE");
										devDataPoint.devType_heater.devHeater_swEnumVal = heaterOpreatAct_close;
										currentDev_dataPointSet(&devDataPoint, true, true, true);
										lv_label_set_text(textTimeInstract_target_devHeater, "\0");
										lv_btnm_set_toggle(btnm_bk_devHeater, true, 0);

										if(imgBtn_devHeater_timeSet && (guiPage_record == bussinessType_Home)){ //补充操作
										
											lv_obj_del(imgBtn_devHeater_timeSet);
											imgBtn_devHeater_timeSet = NULL;
										}

									}break;
									
									case heaterOpreatAct_open:{

										sprintf(str_devRunningRemind_devHeater, "OPEN");
										devDataPoint.devType_heater.devHeater_swEnumVal = heaterOpreatAct_open;
										currentDev_dataPointSet(&devDataPoint, true, true, true);
										lv_label_set_text(textTimeInstract_target_devHeater, "\0");

									}break;

									default:break;
								}
								
							}break;
							
							case msgType_devHeaterDriver_closeCounterChg:{

								sprintf(str_devRunningRemind_devHeater, "%02d:%02d:%02d",
																		(int)((rptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter % 86400UL) / 3600UL),
																		(int)((rptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter % 3600UL) / 60UL),
																		(int)(rptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter % 60UL));

							}break;

							default:break;
						}

						lv_label_set_text(textTimeInstract_current_devHeater, str_devRunningRemind_devHeater);
					}
				
				}break;

				case devTypeDef_thermostat:{

					stt_thermostat_actAttr devParam_thermostat = {0};
					static uint8_t tempCurrent_record = 0;

					devDriverBussiness_thermostatSwitch_devParam_get(&devParam_thermostat);
				
					if(tempCurrent_record != (uint8_t)(devParam_thermostat.temperatureVal_current)){

						char tempCurrentDisp[5] = {0};

						tempCurrent_record = (uint8_t)(devParam_thermostat.temperatureVal_current);
						lv_lmeter_set_value(lmeterTempInstCurrent_devThermostat, tempCurrent_record);
						sprintf(tempCurrentDisp, "%02d", tempCurrent_record);
						lv_label_set_text(labelTempInstCurrent_devThermostat, tempCurrentDisp);
					}

				}break;

				default:break;	
			}

		}break;

		case bussinessType_menuPageWifiConfig:{

			bool networkGetConnectFlg_record = meshNetwork_connectReserve_IF_get();

			if(!networkGetConnectFlg_record) //针对首次配网情况下
				if(flgGet_gotRouterOrMeshConnect())
					lvGui_wifiConfig_bussiness_configComplete_tipsTrig();

			lvGui_wifiConfig_bussiness_configComplete_tipsDetect(); //常规探测

		}break;

		case bussinessType_menuPageOther:{

			const uint8_t loopPeriod = 30;
			static uint8_t loopCounter = 0;	

			if(loopCounter < loopPeriod)loopCounter ++;
			else{

				loopCounter = 0;

				lvGuiOther_devInfoRefresh();
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

				case devTypeDef_scenario:{

					if(devDataPoint_record.devType_scenario.devScenario_opNum != devDataPoint.devType_scenario.devScenario_opNum){

						lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
						lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOff);
						lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOff);
						lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
						lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOff);
						lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOff);
						lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(false));
						lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOff);
						lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOff);

						switch(devDataPoint.devType_scenario.devScenario_opNum){

							case 1:{

								lv_imgbtn_set_src(btn_bk_devMulitSw_A, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true));
								lv_img_set_style(iconBtn_meeting, &styleIconvText_devMulitSw_statusOn);
								lv_label_set_style(textBtn_meeting, &styleIconvText_devMulitSw_statusOn);
								
							}break;

							case 2:{

								lv_imgbtn_set_src(btn_bk_devMulitSw_B, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true));
								lv_img_set_style(iconBtn_sleeping, &styleIconvText_devMulitSw_statusOn);
								lv_label_set_style(textBtn_sleeping, &styleIconvText_devMulitSw_statusOn);

							}break;

							case 4:{

								lv_imgbtn_set_src(btn_bk_devMulitSw_C, LV_BTN_STATE_REL, usrAppHomepageBtnBkPic_dataGet(true));
								lv_img_set_style(iconBtn_toilet, &styleIconvText_devMulitSw_statusOn);
								lv_label_set_style(textBtn_toilet, &styleIconvText_devMulitSw_statusOn);

							}break;

							default:break;
						}

						//必须刷新一下，否则非手动操作时图片刷不动
						lv_obj_refresh_style(btn_bk_devMulitSw_A);
						lv_obj_refresh_style(btn_bk_devMulitSw_B);
						lv_obj_refresh_style(btn_bk_devMulitSw_C);
					}

				}break;
	
				case devTypeDef_dimmer:{

					lv_slider_set_value(slider_bk_devDimmer, devDataPoint.devType_dimmer.devDimmer_brightnessVal);
					sprintf(str_sliderBkVal_devDimmer, "%d%%", lv_slider_get_value(slider_bk_devDimmer));
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
					sprintf(str_devParamPositionCur_devCurtain, "%d", devCurtain_orbitalPosPercent);
					lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
					sprintf(str_devParamPositionAdj_devCurtain, "%d%%", devCurtain_orbitalPosPercent);
					lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
					
				}break;

				case devTypeDef_fans:{

					uint8_t loop = 0;

					for(loop = 0; loop < DEVICE_FANS_OPREAT_ACTION_NUM; loop ++){

						if(devDataPoint.devType_fans.devFans_swEnumVal == objDevFans_btnmDispConferenceTab[loop].opreatActCurrent){
					
							break;
						}
					}		

					if(loop <= DEVICE_FANS_OPREAT_ACTION_NUM){

						switch(loop){

							case fansOpreatAct_firstGear:{
							
								lv_label_set_text(textInstract_devFans, "1");
								styleImage_devFans_icon.image.color = LV_COLOR_MAKE(0, 128, 255);
								styleImage_devFans_icon.image.intense = LV_OPA_COVER;
								lv_obj_refresh_style(icomImage_devFans);
								lv_obj_set_pos(icomImage_devFans, 15, 70);
								lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
							
							}break;
							
							case fansOpreatAct_secondGear:{
							
								lv_label_set_text(textInstract_devFans, "2");
								styleImage_devFans_icon.image.color = LV_COLOR_MAKE(255, 255, 0);
								styleImage_devFans_icon.image.intense = LV_OPA_COVER;
								lv_obj_refresh_style(icomImage_devFans);
								lv_obj_set_pos(icomImage_devFans, 15, 70);
								lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
								
							}break;
							
							case fansOpreatAct_thirdGear:{
							
								lv_label_set_text(textInstract_devFans, "3");
								styleImage_devFans_icon.image.color = LV_COLOR_MAKE(255, 128, 0);
								styleImage_devFans_icon.image.intense = LV_OPA_COVER;
								lv_obj_refresh_style(icomImage_devFans);
								lv_obj_set_pos(icomImage_devFans, 15, 70);
								lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
								
							}break;
							
							case fansOpreatAct_stop:
							default:{
							
								lv_label_set_text(textInstract_devFans, "\0");
								styleImage_devFans_icon.image.color = LV_COLOR_MAKE(224, 224, 220);
								styleImage_devFans_icon.image.intense = LV_OPA_COVER;
								lv_obj_refresh_style(icomImage_devFans);
								lv_obj_set_pos(icomImage_devFans, 68, 70);
								lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);
								
							}break;
						}
					}

				}break;

				case devTypeDef_heater:{

					uint8_t loop = 0;

					for(loop = 0; loop < DEVICE_HEATER_OPREAT_ACTION_NUM; loop ++){
					
						if(devDataPoint.devType_heater.devHeater_swEnumVal == objDevHeater_btnmDispConferenceTab[loop].opreatActCurrent){

							lv_btnm_set_toggle(btnm_bk_devHeater, true, loop);
							
							break;
						}
					}	

					if(loop <= DEVICE_HEATER_OPREAT_ACTION_NUM){

						switch(loop){
							
							case heaterOpreatAct_open:{
						
								lv_label_set_text(textTimeInstract_target_devHeater, "\0");
						
							}break;
							
							case heaterOpreatAct_closeAfter30Min:{
						
								lv_label_set_text(textTimeInstract_target_devHeater, "00:30:00");
						
							}break;
							
							case heaterOpreatAct_closeAfter60Min:{
						
								lv_label_set_text(textTimeInstract_target_devHeater, "00:60:00");
						
							}break;
							
							case heaterOpreatAct_closeAfterTimeCustom:{
						
								char dataDispTemp[15] = {0};
								uint32_t closeCounterTemp = devDriverBussiness_heaterSwitch_closePeriodCustom_Get();
						
								sprintf(dataDispTemp, "%02d:%02d:%02d", (int)((closeCounterTemp % 86400UL) / 3600UL),
																		(int)((closeCounterTemp % 3600UL) / 60UL),
																		(int)(closeCounterTemp % 60UL));
								lv_label_set_text(textTimeInstract_target_devHeater, dataDispTemp);
						
							}break;
							
							case heaterOpreatAct_close:
							default:{
						
								lv_label_set_text(textTimeInstract_target_devHeater, "\0");
						
							}break;
						}
					}

				}break;

				case devTypeDef_thermostat:{

					char tempDisp[5] = {0};

					if(!devDataPoint.devType_thermostat.devThermostat_tempratureTarget)
						devDataPoint.devType_thermostat.devThermostat_tempratureTarget = 16;

					lv_slider_set_value(slider_tempAdj_devThermostat, devDataPoint.devType_thermostat.devThermostat_tempratureTarget - 16);
					sprintf(tempDisp, "%02d", (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);
					lv_label_set_text(labelTempInstTarget_devThermostat, tempDisp);
					lv_lmeter_set_value(lmeterTempInstTarget_devThermostat, (uint8_t)devDataPoint.devType_thermostat.devThermostat_tempratureTarget);

					usrApp_devThermostat_ctrlObj_reserveSet(devDataPoint.devType_thermostat.devThermostat_running_en);
					if(devDataPoint.devType_thermostat.devThermostat_running_en){
					
						lv_sw_on(sw_devRunningEnable_devThermostat);

						if(cb_devEcoEnable_devThermostat == NULL){

							local_guiHomeBussiness_thermostat_EcoCb_creat(lv_obj_get_parent(sw_devRunningEnable_devThermostat), 
																		  devDataPoint.devType_thermostat.devThermostat_nightMode_en);
						}
						else
						{
							lv_cb_set_checked(cb_devEcoEnable_devThermostat, 
											  devDataPoint.devType_thermostat.devThermostat_nightMode_en);
						}
					}
					else
					{
						if(cb_devEcoEnable_devThermostat){
					
							lv_obj_del(cb_devEcoEnable_devThermostat);
							cb_devEcoEnable_devThermostat = NULL;
						}
						lv_sw_off(sw_devRunningEnable_devThermostat);
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
	lv_obj_set_pos(photoA_bk_devDimmer, 40, 60);
	lv_obj_align(photoB_bk_devDimmer, photoA_bk_devDimmer, LV_ALIGN_CENTER, 5, -25);
	lv_img_set_style(photoB_bk_devDimmer, &stylePhotoBk_devDimmer);

	slider_bk_devDimmer = lv_slider_create(obj_Parent, NULL);
	lv_obj_set_size(slider_bk_devDimmer, 200, 30);
	lv_obj_set_protect(slider_bk_devDimmer, LV_PROTECT_POS);
	lv_obj_set_pos(slider_bk_devDimmer, 20, 260);
	lv_slider_set_action(slider_bk_devDimmer, funCb_slidAction_devDimmer_mainSlider);
	lv_slider_set_range(slider_bk_devDimmer, 0, DEVICE_DIMMER_BRIGHTNESS_MAX_VAL);

	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_BG, &styleSliderBk_devDimmer_bg);
	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_INDIC, &styleSliderBk_devDimmer_indic);
	lv_slider_set_style(slider_bk_devDimmer, LV_SLIDER_STYLE_KNOB, &styleSliderBk_devDimmer_knob);

	label_bk_devDimmer = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devDimmer, &styleText_devDimmer_SliderBk);
	lv_slider_set_value(slider_bk_devDimmer, devDataPoint.devType_dimmer.devDimmer_brightnessVal);
	sprintf(str_sliderBkVal_devDimmer, "%d%%", lv_slider_get_value(slider_bk_devDimmer));
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
	stylePhotoAk_devDimmer.image.color = LV_COLOR_GRAY;
	stylePhotoAk_devDimmer.image.intense = 100 - ((uint8_t)lv_slider_get_value(slider_bk_devDimmer));
	lv_img_set_style(photoA_bk_devDimmer, &stylePhotoAk_devDimmer);
}

static void local_guiHomeBussiness_fans(lv_obj_t * obj_Parent){

	icomImage_devFans = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(icomImage_devFans, &iconFans_HomePageDeviceFans);
	lv_img_set_style(icomImage_devFans, &styleImage_devFans_icon);
	lv_obj_set_protect(icomImage_devFans, LV_PROTECT_POS);
	lv_obj_set_pos(icomImage_devFans, 68, 70);

	textInstract_devFans = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(textInstract_devFans, "\0");
	lv_label_set_style(textInstract_devFans, &styleText_devFans_instract);
	lv_obj_set_protect(textInstract_devFans, LV_PROTECT_POS);
	lv_obj_align(textInstract_devFans, icomImage_devFans, LV_ALIGN_OUT_RIGHT_BOTTOM, 40, 5);

	btnm_bk_devFans = lv_btnm_create(obj_Parent, NULL);
	lv_obj_set_size(btnm_bk_devFans, 195, 40);

	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BG, &styleBtnm_devFans_btnBg);
	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_REL, &styleBtnm_devFans_btnRel);
	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_PR, &styleBtnm_devFans_btnPre);
	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_TGL_REL, &styleBtnm_devFans_btnTglRel);
	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_TGL_PR, &styleBtnm_devFans_btnTglPre);
	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_INA, &styleBtnm_devFans_btnIna);

	lv_btnm_set_action(btnm_bk_devFans, funCb_btnmActionClick_devFans_gearBtnm);
	lv_btnm_set_map(btnm_bk_devFans, btnm_str_devFans);
	lv_btnm_set_toggle(btnm_bk_devFans, true, 0);
	lv_obj_set_protect(btnm_bk_devFans, LV_PROTECT_POS);
	lv_obj_set_pos(btnm_bk_devFans, 20, 245);
}

static void local_guiHomeBussiness_thermostat_EcoCb_creat(lv_obj_t * obj_Parent, bool cbVal){

	cb_devEcoEnable_devThermostat = lv_cb_create(obj_Parent, NULL);
	lv_cb_set_action(cb_devEcoEnable_devThermostat, funCb_cbChecked_devThermostat_EcoEnable);
	lv_obj_set_protect(cb_devEcoEnable_devThermostat, LV_PROTECT_POS);
	lv_obj_align(cb_devEcoEnable_devThermostat, labelTempInstCurrent_devThermostat, LV_ALIGN_OUT_LEFT_BOTTOM, 30, -5);
	lv_cb_set_style(cb_devEcoEnable_devThermostat, LV_CB_STYLE_BG, &styleCb_devThermostat_EcoEn);
	lv_cb_set_text(cb_devEcoEnable_devThermostat, "ECO");
	lv_cb_set_checked(cb_devEcoEnable_devThermostat, cbVal);
}

static void local_guiHomeBussiness_thermostat(lv_obj_t * obj_Parent){

	stt_thermostat_actAttr devParam_thermostat = {0};
	char tempDisp[5] = {0};

	devDriverBussiness_thermostatSwitch_devParam_get(&devParam_thermostat);
	if(devParam_thermostat.temperatureVal_target < 16)
		devParam_thermostat.temperatureVal_target = 16;

	lmeterTempInstTarget_devThermostat = lv_lmeter_create(obj_Parent, NULL);
	lv_lmeter_set_range(lmeterTempInstTarget_devThermostat, 0, 45);				 
	lv_lmeter_set_value(lmeterTempInstTarget_devThermostat, (uint8_t)devParam_thermostat.temperatureVal_target);					
	lv_lmeter_set_style(lmeterTempInstTarget_devThermostat, &styleLmeter_devThermostat_tempTarget);	
	lv_obj_set_size(lmeterTempInstTarget_devThermostat, 200, 200);
	lv_lmeter_set_scale(lmeterTempInstTarget_devThermostat, 260, 61);
	lv_obj_set_protect(lmeterTempInstTarget_devThermostat, LV_PROTECT_POS);
	lv_obj_set_pos(lmeterTempInstTarget_devThermostat, 20, 50);
	lmeterTempInstCurrent_devThermostat = lv_lmeter_create(obj_Parent, NULL);
	lv_lmeter_set_range(lmeterTempInstCurrent_devThermostat, 0, 45);				 
	lv_lmeter_set_value(lmeterTempInstCurrent_devThermostat, (uint8_t)devParam_thermostat.temperatureVal_current);					
	lv_lmeter_set_style(lmeterTempInstCurrent_devThermostat, &styleLmeter_devThermostat_tempCurrent);	
	lv_obj_set_size(lmeterTempInstCurrent_devThermostat, 180, 180);
	lv_lmeter_set_scale(lmeterTempInstCurrent_devThermostat, 260, 61);
	lv_obj_set_protect(lmeterTempInstCurrent_devThermostat, LV_PROTECT_POS);
	lv_obj_align(lmeterTempInstCurrent_devThermostat, lmeterTempInstTarget_devThermostat, LV_ALIGN_CENTER, 0, 0);

	labelTempInstTarget_devThermostat = lv_label_create(obj_Parent, NULL);
	sprintf(tempDisp, "%02d", (uint8_t)devParam_thermostat.temperatureVal_target);
	lv_label_set_text(labelTempInstTarget_devThermostat, tempDisp);
	lv_label_set_style(labelTempInstTarget_devThermostat, &styleLabel_devThermostat_tempTarget);
	lv_obj_set_protect(labelTempInstTarget_devThermostat, LV_PROTECT_POS);
	lv_obj_align(labelTempInstTarget_devThermostat, lmeterTempInstCurrent_devThermostat, LV_ALIGN_CENTER, -5, -15);
	labelTempInstCurrent_devThermostat = lv_label_create(obj_Parent, NULL);
	sprintf(tempDisp, "%02d", (uint8_t)devParam_thermostat.temperatureVal_current);
	lv_label_set_text(labelTempInstCurrent_devThermostat, tempDisp);
	lv_label_set_style(labelTempInstCurrent_devThermostat, &styleLabel_devThermostat_tempCurrent);
	lv_obj_set_protect(labelTempInstCurrent_devThermostat, LV_PROTECT_POS);
	lv_obj_align(labelTempInstCurrent_devThermostat, labelTempInstTarget_devThermostat, LV_ALIGN_OUT_BOTTOM_RIGHT, 30, -5);

	slider_tempAdj_devThermostat = lv_slider_create(obj_Parent, NULL);
	lv_obj_set_size(slider_tempAdj_devThermostat, 180, 30);
	lv_slider_set_style(slider_tempAdj_devThermostat, LV_SLIDER_STYLE_BG, &styleSliderBk_devThermostat_bg);
	lv_slider_set_style(slider_tempAdj_devThermostat, LV_SLIDER_STYLE_INDIC, &styleSliderBk_devThermostat_indic);
	lv_slider_set_style(slider_tempAdj_devThermostat, LV_SLIDER_STYLE_KNOB, &styleSliderBk_devThermostat_knob);
	lv_obj_set_protect(slider_tempAdj_devThermostat, LV_PROTECT_POS);
	lv_obj_set_pos(slider_tempAdj_devThermostat, 20, 180);
	lv_obj_align(slider_tempAdj_devThermostat, lmeterTempInstTarget_devThermostat, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_slider_set_action(slider_tempAdj_devThermostat, funCb_slidAction_devThermostat_mainSlider);
	lv_slider_set_range(slider_tempAdj_devThermostat, 0, DEVICE_THERMOSTAT_ADJUST_EQUAL_RANGE);	
	lv_bar_set_value(slider_tempAdj_devThermostat, 0);
	lv_slider_set_value(slider_tempAdj_devThermostat, devParam_thermostat.temperatureVal_target - 16);

	btn_tempAdjAdd_devThermostat = lv_btn_create(obj_Parent, NULL);
	lv_btn_set_action(btn_tempAdjAdd_devThermostat, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devThermostat_tempAdd);
	lv_obj_set_size(btn_tempAdjAdd_devThermostat, 50, 50);
    lv_btn_set_style(btn_tempAdjAdd_devThermostat, LV_BTN_STYLE_REL, 	&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjAdd_devThermostat, LV_BTN_STYLE_PR, 	&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjAdd_devThermostat, LV_BTN_STYLE_TGL_REL,&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjAdd_devThermostat, LV_BTN_STYLE_TGL_PR, &stylebtnBk_transFull);
	lv_obj_set_protect(btn_tempAdjAdd_devThermostat, LV_PROTECT_POS);
	lv_obj_align(btn_tempAdjAdd_devThermostat, slider_tempAdj_devThermostat, LV_ALIGN_OUT_TOP_RIGHT, 10, 0);
	btn_tempAdjCut_devThermostat = lv_btn_create(obj_Parent, NULL);
	lv_btn_set_action(btn_tempAdjCut_devThermostat, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devThermostat_tempCut);
	lv_obj_set_size(btn_tempAdjCut_devThermostat, 50, 50);
    lv_btn_set_style(btn_tempAdjCut_devThermostat, LV_BTN_STYLE_REL, 	&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjCut_devThermostat, LV_BTN_STYLE_PR, 	&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjCut_devThermostat, LV_BTN_STYLE_TGL_REL,&stylebtnBk_transFull);
    lv_btn_set_style(btn_tempAdjCut_devThermostat, LV_BTN_STYLE_TGL_PR, &stylebtnBk_transFull);
	lv_obj_set_protect(btn_tempAdjCut_devThermostat, LV_PROTECT_POS);
	lv_obj_align(btn_tempAdjCut_devThermostat, slider_tempAdj_devThermostat, LV_ALIGN_OUT_TOP_LEFT, -10, 0);

	sw_devRunningEnable_devThermostat = lv_sw_create(obj_Parent, NULL);
	lv_obj_set_size(sw_devRunningEnable_devThermostat, 55, 25);
	lv_obj_set_protect(sw_devRunningEnable_devThermostat, LV_PROTECT_POS);
	lv_obj_align(sw_devRunningEnable_devThermostat, lmeterTempInstTarget_devThermostat, LV_ALIGN_OUT_BOTTOM_MID, 0, -25);
	if(devParam_thermostat.deviceRunning_EN){

		lv_sw_on(sw_devRunningEnable_devThermostat);
		local_guiHomeBussiness_thermostat_EcoCb_creat(obj_Parent, devParam_thermostat.workModeInNight_IF);
	}
	else
	{
		if(cb_devEcoEnable_devThermostat){

			lv_obj_del(cb_devEcoEnable_devThermostat);
			cb_devEcoEnable_devThermostat = NULL;
		}
		lv_sw_off(sw_devRunningEnable_devThermostat);
	}
	usrApp_devThermostat_ctrlObj_reserveSet(devParam_thermostat.deviceRunning_EN);
	lv_sw_set_action(sw_devRunningEnable_devThermostat, funCb_swAction_devThermostat_runningEnable);

	textBtn_tempAdjAdd_devThermostat = lv_label_create(btn_tempAdjAdd_devThermostat, NULL);
	lv_label_set_text(textBtn_tempAdjAdd_devThermostat, "+");
	lv_label_set_style(textBtn_tempAdjAdd_devThermostat, &styleTextBtnBk_devThermostat_tempAdj);
	lv_obj_set_protect(textBtn_tempAdjAdd_devThermostat, LV_PROTECT_POS);
	lv_obj_align(textBtn_tempAdjAdd_devThermostat, btn_tempAdjAdd_devThermostat, LV_ALIGN_CENTER, 0, 0);
	textBtn_tempAdjCut_devThermostat = lv_label_create(btn_tempAdjCut_devThermostat, NULL);
	lv_label_set_text(textBtn_tempAdjCut_devThermostat, "_");	
	lv_label_set_style(textBtn_tempAdjCut_devThermostat, &styleTextBtnBk_devThermostat_tempAdj);
	lv_obj_set_protect(textBtn_tempAdjCut_devThermostat, LV_PROTECT_POS);
	lv_obj_align(textBtn_tempAdjCut_devThermostat, btn_tempAdjCut_devThermostat, LV_ALIGN_CENTER, 0, -15);
}

static void local_guiHomeBussiness_scenario(lv_obj_t * obj_Parent){

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
	lv_obj_set_protect(iconBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(iconBtn_sleeping, LV_PROTECT_POS);
	lv_obj_set_protect(iconBtn_toilet, LV_PROTECT_POS);
	lv_obj_align(iconBtn_meeting, NULL, LV_ALIGN_IN_TOP_RIGHT, 35, 20);
	lv_obj_align(iconBtn_sleeping, NULL, LV_ALIGN_IN_TOP_RIGHT, 35, 20);
	lv_obj_align(iconBtn_toilet, NULL, LV_ALIGN_IN_TOP_RIGHT, 35, 20);
	
	//home界面开关按键说明文字风格加载
	textBtn_meeting = lv_label_create(btn_bk_devMulitSw_A, NULL);
	textBtn_sleeping = lv_label_create(btn_bk_devMulitSw_B, NULL);
	textBtn_toilet = lv_label_create(btn_bk_devMulitSw_C, NULL);
	lv_label_set_align(textBtn_meeting, LV_LABEL_ALIGN_CENTER);
	lv_label_set_align(textBtn_sleeping, LV_LABEL_ALIGN_CENTER);
	lv_label_set_align(textBtn_toilet, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(textBtn_meeting, LV_LABEL_LONG_ROLL);
	lv_label_set_long_mode(textBtn_sleeping, LV_LABEL_LONG_DOT);
	lv_label_set_long_mode(textBtn_toilet, LV_LABEL_LONG_DOT);
	lv_obj_set_size(textBtn_meeting, 140, 25);
	lv_obj_set_size(textBtn_sleeping, 140, 25);
	lv_obj_set_size(textBtn_toilet, 140, 25);
	lv_label_set_text(textBtn_meeting, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[0]);
	lv_label_set_text(textBtn_sleeping, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[1]);
	lv_label_set_text(textBtn_toilet, (const char*)dataTextObjDisp_temp.dataBtnTextDisp[2]);
	lv_obj_set_protect(textBtn_meeting, LV_PROTECT_POS);
	lv_obj_set_protect(textBtn_sleeping, LV_PROTECT_POS);
	lv_obj_set_protect(textBtn_toilet, LV_PROTECT_POS);
	lv_obj_align(textBtn_meeting, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 85, 15);
	lv_obj_align(textBtn_sleeping, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 85, 15);
	lv_obj_align(textBtn_toilet, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 85, 15);

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

static void local_guiHomeBussiness_curtain(lv_obj_t * obj_Parent){

	uint8_t devCurtain_orbitalPosPercent = devCurtain_currentPositionPercentGet();

	image_bk_devCurtain_body = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(image_bk_devCurtain_body, &imageCurtain_body);
	lv_img_set_style(image_bk_devCurtain_body, &styleImageBk_devCurtain_bkImgBody);
	lv_obj_set_protect(image_bk_devCurtain_body, LV_PROTECT_POS);
	lv_obj_set_pos(image_bk_devCurtain_body, 34, 40);

	slider_bk_devCurtain = lv_slider_create(obj_Parent, NULL);
	lv_obj_set_size(slider_bk_devCurtain, 190, 30);
	lv_obj_set_protect(slider_bk_devCurtain, LV_PROTECT_POS);
	lv_obj_set_pos(slider_bk_devCurtain, 25, 235);
	lv_obj_set_top(slider_bk_devCurtain, true);
	lv_slider_set_action(slider_bk_devCurtain, funCb_slidAction_devCurtain_mainSlider);
	lv_bar_set_value(slider_bk_devCurtain, DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL);
	lv_slider_set_value(slider_bk_devCurtain, devCurtain_orbitalPosPercent);

	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_BG, &styleSliderBk_devCurtain_bg);
	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_INDIC, &styleSliderBk_devCurtain_indic);
	lv_slider_set_style(slider_bk_devCurtain, LV_SLIDER_STYLE_KNOB, &styleSliderBk_devCurtain_knob);

	btn_bk_devCurtain_open = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &iconPage_curtainOpen_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_open, LV_BTN_STATE_PR, &iconPage_curtainOpen_rel);
	lv_obj_set_pos(btn_bk_devCurtain_open, 165, 280);
	lv_obj_set_top(btn_bk_devCurtain_open, true);
	lv_obj_animate(btn_bk_devCurtain_open, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_open, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_open, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_open);

	btn_bk_devCurtain_stop = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &iconPage_curtainPluse_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_stop, LV_BTN_STATE_PR, &iconPage_curtainPluse_rel);
	lv_obj_set_pos(btn_bk_devCurtain_stop, 105, 280);
	lv_obj_set_top(btn_bk_devCurtain_stop, true);
	lv_obj_animate(btn_bk_devCurtain_stop, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_stop, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_stop, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_stop);

	btn_bk_devCurtain_close = lv_imgbtn_create(obj_Parent, NULL);
	lv_imgbtn_set_src(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &iconPage_curtainClose_rel);
	lv_imgbtn_set_src(btn_bk_devCurtain_close, LV_BTN_STATE_PR, &iconPage_curtainClose_rel);
	lv_obj_set_pos(btn_bk_devCurtain_close, 25, 280);
	lv_obj_set_top(btn_bk_devCurtain_close, true);
	lv_obj_animate(btn_bk_devCurtain_close, LV_ANIM_FLOAT_RIGHT, 200, 0, NULL);
	lv_imgbtn_set_style(btn_bk_devCurtain_close, LV_BTN_STATE_REL, &styleBtn_devCurtain_statusRel);
	lv_btn_set_action(btn_bk_devCurtain_close, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devCurtain_close);

	label_bk_devCurtain_positionCur = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devCurtain_positionCur, &styleText_devCurtain_Bk_positionCurr);
	lv_label_set_align(label_bk_devCurtain_positionCur, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(label_bk_devCurtain_positionCur, LV_LABEL_LONG_SCROLL);
	sprintf(str_devParamPositionCur_devCurtain, "%d", devCurtain_orbitalPosPercent);
	lv_label_set_text(label_bk_devCurtain_positionCur, str_devParamPositionCur_devCurtain);
	lv_obj_set_protect(label_bk_devCurtain_positionCur, LV_PROTECT_POS);
//	lv_obj_set_pos(label_bk_devCurtain_positionCur, 20, 140);
	lv_obj_align(label_bk_devCurtain_positionCur, image_bk_devCurtain_body, LV_ALIGN_CENTER, 0, 50);
	lv_obj_set_top(label_bk_devCurtain_positionCur, true);

	label_bk_devCurtain_positionAdj = lv_label_create(obj_Parent, NULL);
	lv_label_set_style(label_bk_devCurtain_positionAdj, &styleText_devCurtain_Bk_positionTips);
	lv_label_set_long_mode(label_bk_devCurtain_positionAdj, LV_LABEL_LONG_SCROLL);
	sprintf(str_devParamPositionAdj_devCurtain, "%d%%", devCurtain_orbitalPosPercent);
	lv_label_set_text(label_bk_devCurtain_positionAdj, str_devParamPositionAdj_devCurtain);
	lv_obj_set_protect(label_bk_devCurtain_positionAdj, LV_PROTECT_POS);
//	lv_obj_set_pos(label_bk_devCurtain_positionAdj, 20, 140);
	lv_obj_align(label_bk_devCurtain_positionAdj, slider_bk_devCurtain, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);
	lv_obj_set_top(label_bk_devCurtain_positionAdj, true);
}

static void local_guiHomeBussiness_heater(lv_obj_t * obj_Parent){

	stt_devDataPonitTypedef devDataPoint = {0};
	
	currentDev_dataPointGet(&devDataPoint);

	icomImageA_devHeater = lv_img_create(obj_Parent, NULL);
	lv_img_set_src(icomImageA_devHeater, &iconHeater_HomePageDeviceHeater);
	lv_img_set_style(icomImageA_devHeater, &styleImage_devHeater_icon);
	lv_obj_set_protect(icomImageA_devHeater, LV_PROTECT_POS);
	lv_obj_set_pos(icomImageA_devHeater, 15, 75);

	textTimeInstract_target_devHeater = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(textTimeInstract_target_devHeater, "\0");
	lv_label_set_style(textTimeInstract_target_devHeater, &styleText_devHeater_timeInst);
	lv_obj_set_protect(textTimeInstract_target_devHeater, LV_PROTECT_POS);
	lv_obj_align(textTimeInstract_target_devHeater, icomImageA_devHeater, LV_ALIGN_CENTER, -60, -20);
	textTimeInstract_current_devHeater = lv_label_create(obj_Parent, NULL);
	lv_label_set_text(textTimeInstract_current_devHeater, "CLOSE");
	lv_label_set_style(textTimeInstract_current_devHeater, &styleText_devHeater_timeInst);
	lv_obj_set_protect(textTimeInstract_current_devHeater, LV_PROTECT_POS);
	lv_obj_align(textTimeInstract_current_devHeater, icomImageA_devHeater, LV_ALIGN_CENTER, -35, 0);

	btnm_bk_devHeater = lv_btnm_create(obj_Parent, NULL);
	lv_obj_set_size(btnm_bk_devHeater, 200, 40);
//	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BG, &styleBtnm_devHeater_btnBg);
//	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_REL, &styleBtnm_devHeater_btnRel);
//	lv_btnm_set_style(btnm_bk_devFans, LV_BTNM_STYLE_BTN_PR, &styleBtnm_devHeater_btnPre);
	lv_btnm_set_action(btnm_bk_devHeater, funCb_btnmActionClick_devHeater_gearBtnm);
	lv_btnm_set_map(btnm_bk_devHeater, btnm_str_devHeater);
	lv_btnm_set_toggle(btnm_bk_devHeater, true, 0);
	lv_obj_set_protect(btnm_bk_devHeater, LV_PROTECT_POS);
	lv_obj_set_pos(btnm_bk_devHeater, 20, 245);

	if(devDataPoint.devType_heater.devHeater_swEnumVal == heaterOpreatAct_closeAfterTimeCustom){

		imgBtn_devHeater_timeSet = lv_imgbtn_create(obj_Parent, NULL);
		lv_obj_set_protect(imgBtn_devHeater_timeSet, LV_PROTECT_POS);
		lv_obj_set_pos(imgBtn_devHeater_timeSet, 185, 205);
		lv_imgbtn_set_src(imgBtn_devHeater_timeSet, LV_BTN_STATE_REL, &iconSet_HomePageDeviceHeater);
		lv_imgbtn_set_src(imgBtn_devHeater_timeSet, LV_BTN_STATE_PR, &iconSet_HomePageDeviceHeater);
		lv_imgbtn_set_action(imgBtn_devHeater_timeSet, LV_BTN_ACTION_CLICK, funCb_btnActionClick_devHeater_timeSet);
	}
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

static void guiBussiness_tipsSystemRestartCreat(const char *tipsDelayCounter){

	if(page_sysRestartTips == NULL){

		page_sysRestartTips = lv_page_create(lv_scr_act(), NULL);
		lv_obj_set_top(page_sysRestartTips, true);
		lv_obj_set_size(page_sysRestartTips, 120, 120);
		lv_obj_set_protect(page_sysRestartTips, LV_PROTECT_POS);
		lv_obj_align(page_sysRestartTips, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_page_set_style(page_sysRestartTips, LV_PAGE_STYLE_SB, &stylePage_sysRestartTips);
		lv_page_set_style(page_sysRestartTips, LV_PAGE_STYLE_BG, &stylePage_sysRestartTips);
		lv_page_set_sb_mode(page_sysRestartTips, LV_SB_MODE_HIDE);	
		lv_page_set_scrl_fit(page_sysRestartTips, false, true); //key opration
		lv_page_set_scrl_layout(page_sysRestartTips, LV_LAYOUT_PRETTY);

		label_sysRestartTips_Counter = lv_label_create(page_sysRestartTips, NULL);
		lv_label_set_style(label_sysRestartTips_Counter, &styleLabelCounter_sysRestartTips);
		lv_obj_set_protect(label_sysRestartTips_Counter, LV_PROTECT_POS);
		lv_obj_align(label_sysRestartTips_Counter, page_sysRestartTips, LV_ALIGN_CENTER, -10, 0);
		label_sysRestartTips_ref = lv_label_create(page_sysRestartTips, NULL);
		lv_label_set_style(label_sysRestartTips_ref, &styleLabelRef_sysRestartTips);
		lv_obj_set_protect(label_sysRestartTips_ref, LV_PROTECT_POS);
		lv_obj_align(label_sysRestartTips_ref, page_sysRestartTips, LV_ALIGN_IN_BOTTOM_MID, -28, -5);
	}

	lv_label_set_text(label_sysRestartTips_Counter, tipsDelayCounter);
	lv_label_set_text(label_sysRestartTips_ref, "sys restart");

	lv_obj_refresh_style(page_sysRestartTips);
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
	{local_guiHomeBussiness_thermostat, 	devTypeDef_thermostat},
};

static void lvGui_businessHome(lv_obj_t * obj_Parent){

	uint8_t loop = 0;
	devTypeDef_enum devType_temp = currentDev_typeGet();

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

	switch(devType_temp){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_scenario:{

			styleImgBk_underlying.image.intense = 0;
			
			lv_img_set_style(imageBK, &styleImgBk_underlying);

		}break;
		
		case devTypeDef_dimmer:
		case devTypeDef_fans:
		case devTypeDef_curtain:
		case devTypeDef_thermostat:
		case devTypeDef_heater:{

			styleImgBk_underlying.image.intense = LV_OPA_50;
			styleImgBk_underlying.image.color = LV_COLOR_GRAY;

			lv_img_set_style(imageBK, &styleImgBk_underlying);

		}break;
		
		default:{}break;
	}

	for(loop = 0; loop < DEVICE_TYPE_LIST_NUM; loop ++){

		if(devType_temp == guiHomeBussiness_tab[loop].currentDevType){

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

	uint8_t msgQrptr_devRestartdelayCounter = 0;
	
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

					switch(guiPage_current){
					
						case bussinessType_Home:{

							bool gestureReserve_flg = true;

							if(gestureReserve_flg)
								if(ctrlObj_slidingCalmDownCounter)gestureReserve_flg = false;

							if(gestureReserve_flg){

								switch(currentDev_typeGet()){

									case devTypeDef_scenario:{

										if(devDriverBussiness_scnarioSwitch_driverClamDown_get())
											gestureReserve_flg = false;
									
									}break;

									default:break;
								}
							}

							if(gestureReserve_flg)
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

				case bussinessType_menuPageOther:{

					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_other(imageBK);
					lv_obj_set_size(imageBK, 240, 75);
					
				}break;
				
				case bussinessType_menuPageDelayer:{
					
					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_delayer(imageBK);
					lv_obj_set_size(imageBK, 240, 75);

				}break;
				
				case bussinessType_menuPageTimer:{
					
					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_timer(imageBK);
					lv_obj_set_size(imageBK, 240, 75);
	
				}break;
				
				case bussinessType_menuPageLinkageConfig:{
					
					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_GRAY;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();

					lvGui_businessMenu_linkageConfig(imageBK);
					lv_obj_set_size(imageBK, 240, 75);

				}break;
				
				case bussinessType_menuPageSetting:{

//					//控件风格设定：二级菜单灰色底图对象
//					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
//					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
//					styleBk_secMenu.body.grad_color = LV_COLOR_GRAY;
//					vTaskDelay(100 / portTICK_PERIOD_MS);
//					lvUsr_objBkReales2SecMenu();

//					lvGui_businessMenu_setting(imageBK);
//					lv_obj_set_size(imageBK, 240, 75);

					//控件风格设定：二级菜单灰色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_GRAY;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_settingSet(imageBK);
					lv_obj_set_size(imageBK, 240, 75);
					
				}break;

				case bussinessType_menuPageWifiConfig:{

					//控件风格设定：二级菜单白色底图对象
					lv_style_copy(&styleBk_secMenu, &lv_style_plain);
					styleBk_secMenu.body.main_color = LV_COLOR_BLACK;
					styleBk_secMenu.body.grad_color = LV_COLOR_WHITE;
					vTaskDelay(100 / portTICK_PERIOD_MS);
					lvUsr_objBkReales2SecMenu();
					
					lvGui_businessMenu_wifiConfig(imageBK);
			
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

		//消息通知接收 ---设备重启延时执行倒计时 时间提示
		if(xQueueReceive(msgQh_systemRestartDelayCounterTips, &msgQrptr_devRestartdelayCounter, 1 / portTICK_RATE_MS) == pdTRUE){

			char counterDisp_text[5] = {0};

			sprintf(counterDisp_text, "%d", msgQrptr_devRestartdelayCounter);
			guiBussiness_tipsSystemRestartCreat(counterDisp_text);
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
	styleText_devDimmer_SliderBk.text.font = &lv_font_dejavu_30;
	styleText_devDimmer_SliderBk.text.color = LV_COLOR_WHITE;

	//文字风格设定：定时响应对应定时类别文字描述
	lv_style_copy(&styleText_loopTimerTips, &lv_style_plain);
	styleText_loopTimerTips.text.font = &lv_font_dejavu_20;
	styleText_loopTimerTips.text.color = LV_COLOR_WHITE;

	//文字风格设定：风扇开关对应档位文字描述
	lv_style_copy(&styleText_devFans_instract, &lv_style_plain);
	styleText_devFans_instract.text.font = &lv_font_arialNum_100;
	styleText_devFans_instract.text.color = LV_COLOR_MAKE(153, 217, 234);

	//文字风格设定：热水器开关对应档位文字描述
	lv_style_copy(&styleText_devHeater_timeInst, &lv_style_plain);
	styleText_devHeater_timeInst.text.font = &lv_font_consola_19;
	styleText_devHeater_timeInst.text.color = LV_COLOR_MAKE(255, 240, 155);

	//文字风格设定：热水器开关对应延时关闭文字描述
	lv_style_copy(&styleText_devHeater_Bk_timeRemind, &lv_style_plain);
	styleText_devHeater_Bk_timeRemind.text.font = &lv_font_consola_16;
	styleText_devHeater_Bk_timeRemind.text.color = LV_COLOR_ORANGE;

	//文字风格设定：窗帘位置信息描述文字
	lv_style_copy(&styleText_devCurtain_Bk_positionCurr, &lv_style_plain);
	styleText_devCurtain_Bk_positionCurr.text.font = &lv_font_dejavu_40;
	styleText_devCurtain_Bk_positionCurr.text.color = LV_COLOR_MAKE(224, 224, 220);
	lv_style_copy(&styleText_devCurtain_Bk_positionTips, &lv_style_plain);
	styleText_devCurtain_Bk_positionTips.text.font = &lv_font_consola_19;
	styleText_devCurtain_Bk_positionTips.text.color = LV_COLOR_WHITE;

	//文字风格设定：恒温器温度信息描述
	lv_style_copy(&styleLabel_devThermostat_tempTarget, &lv_style_plain);
	styleLabel_devThermostat_tempTarget.text.font = &lv_font_arialNum_100;
	styleLabel_devThermostat_tempTarget.text.color = LV_COLOR_MAKE(255, 255, 0);
	lv_style_copy(&styleLabel_devThermostat_tempCurrent, &lv_style_plain);
	styleLabel_devThermostat_tempCurrent.text.font = &lv_font_dejavu_30;
	styleLabel_devThermostat_tempCurrent.text.color = LV_COLOR_MAKE(0, 255, 0);
	lv_style_copy(&styleTextBtnBk_devThermostat_tempAdj, &lv_style_plain);
	styleTextBtnBk_devThermostat_tempAdj.text.font = &lv_font_dejavu_30;
	styleTextBtnBk_devThermostat_tempAdj.text.color = LV_COLOR_MAKE(255, 255, 255);

	lv_style_copy(&styleTextRollerTips_devHeater_timeSetPage, &lv_style_plain);
	styleTextRollerTips_devHeater_timeSetPage.text.font = &lv_font_arial_15;
	styleTextRollerTips_devHeater_timeSetPage.text.color = LV_COLOR_MAKE(80, 240, 80);

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

	//图片风格设定：窗帘图标风格
	lv_style_copy(&styleImageBk_devCurtain_bkImgBody, &lv_style_plain);
	styleImageBk_devCurtain_bkImgBody.image.color = LV_COLOR_MAKE(224, 224, 220);
	styleImageBk_devCurtain_bkImgBody.image.intense = LV_OPA_COVER;

	//图片风格设定：风扇图标图片
	lv_style_copy(&styleImage_devFans_icon, &lv_style_plain);
	styleImage_devFans_icon.image.color = LV_COLOR_MAKE(224, 224, 220);
	styleImage_devFans_icon.image.intense = LV_OPA_COVER;

	//图片风格设定：热水器图标图片
	lv_style_copy(&styleImage_devHeater_icon, &lv_style_plain);
	styleImage_devHeater_icon.image.color = LV_COLOR_MAKE(224, 224, 220);
	styleImage_devHeater_icon.image.intense = LV_OPA_COVER;

	//图片风格设定：调光亮度提示图片底图
	lv_style_copy(&stylePhotoAk_devDimmer, &lv_style_plain);
	stylePhotoBk_devDimmer.image.intense = LV_OPA_0;
	lv_style_copy(&stylePhotoBk_devDimmer, &lv_style_plain);
	stylePhotoBk_devDimmer.image.opa = LV_OPA_0;

	//图片风格设定：背景图
	lv_style_copy(&styleImgBk_underlying, &lv_style_plain);
	styleImgBk_underlying.image.intense = LV_OPA_50;
	styleImgBk_underlying.image.color = LV_COLOR_GRAY;
//	styleImgBk_underlying.image.intense = LV_OPA_0;
//	styleImgBk_underlying.image.opa = LV_OPA_0;

	//空间风格设定：按键透明
	stylebtnBk_transFull.body.main_color = LV_COLOR_TRANSP;
	stylebtnBk_transFull.body.grad_color = LV_COLOR_TRANSP;
	stylebtnBk_transFull.body.border.part = LV_BORDER_NONE;
    stylebtnBk_transFull.body.opa = LV_OPA_TRANSP;
	stylebtnBk_transFull.body.radius = 0;
    stylebtnBk_transFull.body.shadow.width =  0;

	//控件风格设定；恒温器线路表
	lv_style_copy(&styleLmeter_devThermostat_tempTarget, &lv_style_pretty_color);
	styleLmeter_devThermostat_tempTarget.line.width = 2;
	styleLmeter_devThermostat_tempTarget.body.padding.hor = 10;
	styleLmeter_devThermostat_tempTarget.line.color = LV_COLOR_WHITE;
	styleLmeter_devThermostat_tempTarget.body.main_color = LV_COLOR_MAKE(0, 0, 255); 		/*Light blue*/
	styleLmeter_devThermostat_tempTarget.body.grad_color = LV_COLOR_MAKE(0, 0, 255);		/*Dark blue*/
	lv_style_copy(&styleLmeter_devThermostat_tempCurrent, &lv_style_pretty_color);
	styleLmeter_devThermostat_tempCurrent.line.width = 2;
	styleLmeter_devThermostat_tempCurrent.body.padding.hor = 6;
	styleLmeter_devThermostat_tempCurrent.line.color = LV_COLOR_WHITE;
//	styleLmeter_devThermostat_tempCurrent.line.opa = LV_OPA_TRANSP;
	styleLmeter_devThermostat_tempCurrent.body.main_color = LV_COLOR_MAKE(0, 255, 0); 	/*Light blue*/
	styleLmeter_devThermostat_tempCurrent.body.grad_color = LV_COLOR_MAKE(0, 255, 0); 

	//控件风格设定：调光开关及窗帘开关滑动条
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

	lv_style_copy(&styleSliderBk_devThermostat_bg, &lv_style_pretty);
	styleSliderBk_devThermostat_bg.body.main_color = LV_COLOR_BLACK;
	styleSliderBk_devThermostat_bg.body.grad_color = LV_COLOR_GRAY;
	styleSliderBk_devThermostat_bg.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devThermostat_bg.body.border.color = LV_COLOR_WHITE;
	lv_style_copy(&styleSliderBk_devThermostat_indic, &lv_style_pretty);
	styleSliderBk_devThermostat_indic.body.grad_color = LV_COLOR_GREEN;
	styleSliderBk_devThermostat_indic.body.main_color = LV_COLOR_LIME;
	styleSliderBk_devThermostat_indic.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devThermostat_indic.body.shadow.width = 10;
	styleSliderBk_devThermostat_indic.body.shadow.color = LV_COLOR_LIME;
	styleSliderBk_devThermostat_indic.body.padding.hor = 3;
	styleSliderBk_devThermostat_indic.body.padding.ver = 3;
	lv_style_copy(&styleSliderBk_devThermostat_knob, &lv_style_pretty);
	styleSliderBk_devThermostat_knob.body.radius = LV_RADIUS_CIRCLE;
	styleSliderBk_devThermostat_knob.body.opa = LV_OPA_70;
	styleSliderBk_devThermostat_knob.body.padding.ver = 10;

	lv_style_copy(&styleCb_devThermostat_EcoEn, &lv_style_plain);
	styleCb_devThermostat_EcoEn.body.border.part = LV_BORDER_NONE;
	styleCb_devThermostat_EcoEn.body.empty = 1;
	styleCb_devThermostat_EcoEn.text.color = LV_COLOR_MAKE(128, 255, 255);
	styleCb_devThermostat_EcoEn.text.font = &lv_font_consola_19;
	styleCb_devThermostat_EcoEn.text.opa = LV_OPA_100;

	//控件风格设定：风扇开关档位按钮矩阵选定框背景、选中风格设置
//    lv_style_copy(&styleBtnm_devFans_btnBg, &lv_style_plain);
//    styleBtnm_devFans_btnBg.body.main_color = lv_color_hsv_to_rgb(15, 11, 30);
//    styleBtnm_devFans_btnBg.body.grad_color = lv_color_hsv_to_rgb(15, 11, 30);
//    styleBtnm_devFans_btnBg.text.color = lv_color_hsv_to_rgb(15, 5, 95);

//    lv_style_copy(&styleBtnm_devFans_btnRel, &lv_style_pretty);
//    styleBtnm_devFans_btnRel.body.main_color = lv_color_hsv_to_rgb(15, 10, 40);
//    styleBtnm_devFans_btnRel.body.grad_color = lv_color_hsv_to_rgb(15, 10, 20);
//    styleBtnm_devFans_btnRel.body.border.color = LV_COLOR_HEX3(0x111);
//    styleBtnm_devFans_btnRel.body.border.width = 1;
//    styleBtnm_devFans_btnRel.body.border.opa = LV_OPA_70;
//    styleBtnm_devFans_btnRel.body.padding.hor = LV_DPI / 4;
//    styleBtnm_devFans_btnRel.body.padding.ver = LV_DPI / 8;
//    styleBtnm_devFans_btnRel.body.shadow.type = LV_SHADOW_BOTTOM;
//    styleBtnm_devFans_btnRel.body.shadow.color = LV_COLOR_HEX3(0x111);
//    styleBtnm_devFans_btnRel.body.shadow.width = LV_DPI / 30;
//    styleBtnm_devFans_btnRel.text.color = LV_COLOR_HEX3(0xeee);

//    lv_style_copy(&styleBtnm_devFans_btnPre, &styleBtnm_devFans_btnRel);
//    styleBtnm_devFans_btnPre.body.main_color = lv_color_hsv_to_rgb(15, 10, 30);
//    styleBtnm_devFans_btnPre.body.grad_color = lv_color_hsv_to_rgb(15, 10, 10);

//    lv_style_copy(&styleBtnm_devFans_btnTglRel, &styleBtnm_devFans_btnRel);
//    styleBtnm_devFans_btnTglRel.body.main_color = lv_color_hsv_to_rgb(15, 10, 20);
//    styleBtnm_devFans_btnTglRel.body.grad_color = lv_color_hsv_to_rgb(15, 10, 40);
//    styleBtnm_devFans_btnTglRel.body.shadow.width = LV_DPI / 40;
//    styleBtnm_devFans_btnTglRel.text.color = LV_COLOR_HEX3(0xddd);

//    lv_style_copy(&styleBtnm_devFans_btnTglPre, &styleBtnm_devFans_btnRel);
//    styleBtnm_devFans_btnTglPre.body.main_color = lv_color_hsv_to_rgb(15, 10, 10);
//    styleBtnm_devFans_btnTglPre.body.grad_color = lv_color_hsv_to_rgb(15, 10, 30);
//    styleBtnm_devFans_btnTglPre.body.shadow.width = LV_DPI / 30;
//    styleBtnm_devFans_btnTglPre.text.color = LV_COLOR_HEX3(0xddd);

//    lv_style_copy(&styleBtnm_devFans_btnIna, &styleBtnm_devFans_btnRel);
//    styleBtnm_devFans_btnIna.body.main_color = lv_color_hsv_to_rgb(15, 10, 20);
//    styleBtnm_devFans_btnIna.body.grad_color = lv_color_hsv_to_rgb(15, 10, 20);
//    styleBtnm_devFans_btnIna.text.color = LV_COLOR_HEX3(0xaaa);
//    styleBtnm_devFans_btnIna.body.shadow.width = 0;

    lv_style_copy(&styleBtnm_devFans_btnBg, &lv_style_plain);
    styleBtnm_devFans_btnBg.body.main_color = LV_COLOR_HEX3(0x333);
    styleBtnm_devFans_btnBg.body.grad_color =  LV_COLOR_HEX3(0x333);
    styleBtnm_devFans_btnBg.body.border.width = 2;
    styleBtnm_devFans_btnBg.body.border.color =  LV_COLOR_HEX3(0x666);
    styleBtnm_devFans_btnBg.body.shadow.color = LV_COLOR_SILVER;

    lv_style_copy(&styleBtnm_devFans_btnRel, &lv_style_plain);
    styleBtnm_devFans_btnRel.glass = 0;
    styleBtnm_devFans_btnRel.body.empty = 1;
    styleBtnm_devFans_btnRel.body.radius = LV_RADIUS_CIRCLE;
    styleBtnm_devFans_btnRel.body.border.width = 2;
    styleBtnm_devFans_btnRel.body.border.color = lv_color_hsv_to_rgb(15, 70, 90);
    styleBtnm_devFans_btnRel.body.border.opa = LV_OPA_80;
    styleBtnm_devFans_btnRel.body.padding.hor = LV_DPI / 4;
    styleBtnm_devFans_btnRel.body.padding.ver = LV_DPI / 6;
    styleBtnm_devFans_btnRel.body.padding.inner = LV_DPI / 10;
    styleBtnm_devFans_btnRel.text.color = lv_color_hsv_to_rgb(15, 8, 96);

    lv_style_copy(&styleBtnm_devFans_btnPre, &styleBtnm_devFans_btnRel);
    styleBtnm_devFans_btnPre.body.opa = LV_OPA_COVER;
    styleBtnm_devFans_btnPre.body.empty = 0;
    styleBtnm_devFans_btnPre.body.main_color = lv_color_hsv_to_rgb(15, 50, 50);
    styleBtnm_devFans_btnPre.body.grad_color = lv_color_hsv_to_rgb(15, 50, 50);
    styleBtnm_devFans_btnPre.body.border.opa = LV_OPA_60;
    styleBtnm_devFans_btnPre.text.color = lv_color_hsv_to_rgb(15, 10, 100);

    lv_style_copy(&styleBtnm_devFans_btnTglRel, &styleBtnm_devFans_btnPre);
    styleBtnm_devFans_btnTglRel.body.opa = LV_OPA_COVER;
    styleBtnm_devFans_btnTglRel.body.empty = 0;
    styleBtnm_devFans_btnTglRel.body.main_color = lv_color_hsv_to_rgb(15, 50, 60);
    styleBtnm_devFans_btnTglRel.body.grad_color = lv_color_hsv_to_rgb(15, 50, 60);
    styleBtnm_devFans_btnTglRel.body.border.opa = LV_OPA_60;
    styleBtnm_devFans_btnTglRel.body.border.color = lv_color_hsv_to_rgb(15, 80, 90);
    styleBtnm_devFans_btnTglRel.text.color = lv_color_hsv_to_rgb(15, 0, 100);

    lv_style_copy(&styleBtnm_devFans_btnTglPre, &styleBtnm_devFans_btnTglRel);
    styleBtnm_devFans_btnTglPre.body.opa = LV_OPA_COVER;
    styleBtnm_devFans_btnTglPre.body.empty = 0;
    styleBtnm_devFans_btnTglPre.body.main_color = lv_color_hsv_to_rgb(15, 50, 50);
    styleBtnm_devFans_btnTglPre.body.grad_color = lv_color_hsv_to_rgb(15, 50, 50);
    styleBtnm_devFans_btnTglPre.body.border.opa = LV_OPA_60;
    styleBtnm_devFans_btnTglPre.body.border.color = lv_color_hsv_to_rgb(15, 80, 70);
    styleBtnm_devFans_btnTglPre.text.color = lv_color_hsv_to_rgb(15, 10, 90);

    lv_style_copy(&styleBtnm_devFans_btnIna, &styleBtnm_devFans_btnRel);
    styleBtnm_devFans_btnIna.body.border.opa = LV_OPA_60;
    styleBtnm_devFans_btnIna.body.border.color = lv_color_hsv_to_rgb(15, 10, 50);
    styleBtnm_devFans_btnIna.text.color = lv_color_hsv_to_rgb(15, 10, 90);

	//控件风格设定：热水器开关档位按钮矩阵选定框背景、选中风格设置
	lv_style_copy(&styleBtnm_devHeater_btnBg, &lv_style_plain);
	styleBtnm_devHeater_btnBg.body.main_color = LV_COLOR_SILVER;
	styleBtnm_devHeater_btnBg.body.grad_color = LV_COLOR_SILVER;
	styleBtnm_devHeater_btnBg.body.padding.hor = 0;
	styleBtnm_devHeater_btnBg.body.padding.ver = 0;
	styleBtnm_devHeater_btnBg.body.padding.inner = 0;
	lv_style_copy(&styleBtnm_devHeater_btnRel, &lv_style_btn_rel);
	styleBtnm_devHeater_btnRel.body.main_color = LV_COLOR_MAKE(0x30, 0x30, 0x30);
	styleBtnm_devHeater_btnRel.body.grad_color = LV_COLOR_BLACK;
	styleBtnm_devHeater_btnRel.body.border.color = LV_COLOR_SILVER;
	styleBtnm_devHeater_btnRel.body.border.width = 1;
	styleBtnm_devHeater_btnRel.body.border.opa = LV_OPA_50;
	styleBtnm_devHeater_btnRel.body.radius = 0;
	lv_style_copy(&styleBtnm_devHeater_btnPre, &lv_style_btn_rel);
	styleBtnm_devHeater_btnPre.body.main_color = LV_COLOR_MAKE(0x55, 0x96, 0xd8);
	styleBtnm_devHeater_btnPre.body.grad_color = LV_COLOR_MAKE(0x37, 0x62, 0x90);
	styleBtnm_devHeater_btnPre.text.color = LV_COLOR_MAKE(0xbb, 0xd5, 0xf1);

	//控件风格设定：热水器时间设置界面，滚轮设置
	lv_style_copy(&styleTextRoller_devHeater_timeSetPage_bg, &lv_style_plain);
	styleTextRoller_devHeater_timeSetPage_bg.body.main_color = LV_COLOR_SILVER;
	styleTextRoller_devHeater_timeSetPage_bg.body.grad_color = LV_COLOR_SILVER;
	styleTextRoller_devHeater_timeSetPage_bg.body.opa = LV_OPA_90;
	styleTextRoller_devHeater_timeSetPage_bg.text.font = &lv_font_consola_19;
	styleTextRoller_devHeater_timeSetPage_bg.text.line_space = 5;
	styleTextRoller_devHeater_timeSetPage_bg.text.opa = LV_OPA_40;
	lv_style_copy(&styleTextRoller_devHeater_timeSetPage_sel, &lv_style_plain);
	styleTextRoller_devHeater_timeSetPage_sel.body.empty = 1;
	styleTextRoller_devHeater_timeSetPage_sel.body.radius = 30;
	styleTextRoller_devHeater_timeSetPage_sel.body.opa = LV_OPA_90;
	styleTextRoller_devHeater_timeSetPage_sel.text.color = LV_COLOR_BLACK;
	styleTextRoller_devHeater_timeSetPage_sel.text.font = &lv_font_consola_19;

	//控件风格设定：热水器自定义时间设置界面
	lv_style_copy(&stylePage_devHeater_timeSet, &lv_style_plain_color);
	stylePage_devHeater_timeSet.body.main_color = LV_COLOR_SILVER;
	stylePage_devHeater_timeSet.body.grad_color = LV_COLOR_SILVER;
	stylePage_devHeater_timeSet.body.border.part = LV_BORDER_NONE;
	stylePage_devHeater_timeSet.body.radius = 6;
	stylePage_devHeater_timeSet.body.opa = LV_OPA_90;
	stylePage_devHeater_timeSet.body.padding.hor = 0;
	stylePage_devHeater_timeSet.body.padding.inner = 0;	

	//控件风格设定：场景开关 触发冷却 预加载器 风格
	lv_style_copy(&stylePreload_devScenario_driverCalmDown, &lv_style_plain);
	stylePreload_devScenario_driverCalmDown.line.width = 10;						   /*10 px thick arc*/
	stylePreload_devScenario_driverCalmDown.line.color = LV_COLOR_HEX3(0x258);	   /*Blueish arc color*/
	stylePreload_devScenario_driverCalmDown.body.border.color = LV_COLOR_HEX3(0xBBB); /*Gray background color*/
	stylePreload_devScenario_driverCalmDown.body.border.width = 10;
	stylePreload_devScenario_driverCalmDown.body.padding.hor = 0;

	//控件风格设定：场景开关 触发冷却 按键图片 风格
	lv_style_copy(&styleBtn_devScenario_driverCalmDown, &lv_style_plain);
	styleBtn_devScenario_driverCalmDown.image.color = LV_COLOR_YELLOW;
	styleBtn_devScenario_driverCalmDown.image.intense = LV_OPA_50;
	
	//文字风格设定：按钮字体
	lv_style_copy(&styleTextBtn_devHeater_timeSetPage, &lv_style_plain);
	styleTextBtn_devHeater_timeSetPage.text.font = &lv_font_consola_17;
	styleTextBtn_devHeater_timeSetPage.text.color = LV_COLOR_MAKE(0, 128, 192);

	//控件风格设定：重启提示弹窗
	lv_style_copy(&stylePage_sysRestartTips, &lv_style_plain_color);
	stylePage_sysRestartTips.body.main_color = LV_COLOR_WHITE;
	stylePage_sysRestartTips.body.grad_color = LV_COLOR_WHITE;
	stylePage_sysRestartTips.body.border.part = LV_BORDER_NONE;
	stylePage_sysRestartTips.body.radius = 6;
	stylePage_sysRestartTips.body.opa = LV_OPA_90;
	stylePage_sysRestartTips.body.padding.hor = 0;
	stylePage_sysRestartTips.body.padding.inner = 0;	

	lv_style_copy(&styleLabelCounter_sysRestartTips, &lv_style_plain);
	styleLabelCounter_sysRestartTips.text.font = &lv_font_arialNum_100;
	styleLabelCounter_sysRestartTips.text.color = LV_COLOR_RED;

	lv_style_copy(&styleLabelRef_sysRestartTips, &lv_style_plain);
	styleLabelRef_sysRestartTips.text.font = &lv_font_consola_17;
	styleLabelRef_sysRestartTips.text.color = LV_COLOR_BLACK;
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
	msgQh_wifiConfigCompleteTips = xQueueCreate(2, sizeof(uint8_t));

	xTaskCreate(task_guiSwitch_Detecting, //Task Function
				"guiDetect", //Task Name
				1024 * 4,	 //Stack Depth
				NULL,		 //Parameters
				CONFIG_MDF_TASK_DEFAULT_PRIOTY,	//Priority
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



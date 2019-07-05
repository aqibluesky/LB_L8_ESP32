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

#include "gui_businessReuse_reactionObjPage.h"
#include "gui_businessHome.h"

LV_FONT_DECLARE(lv_font_consola_17);
LV_FONT_DECLARE(lv_font_consola_19);

extern const char *btnm_str_devHeater[DEVICE_HEATER_OPREAT_ACTION_NUM + 1];
extern const char *btnm_str_devFans[DEVICE_FANS_OPREAT_ACTION_NUM + 1];
const char *btnm_str_devScenario[DEVSCENARIO_OPREATION_OBJ_NUM + 1] =  {"A", "B", "C", ""};

static uint8_t devDimmer_pageSet_brightnessOpreat_div = 0;
static uint8_t devCurtain_pageSet_positionOpreat_div = 0;

static lv_obj_t	*pageDetailSet_switchReaction_sw[3] = {NULL};
static lv_obj_t	*pageDetailSet_switchReaction_labelSwRef[3] = {NULL};
static lv_obj_t	*pageDetailSet_switchReaction_slider = NULL;
static lv_obj_t	*pageDetailSet_switchReaction_labelSliderRef = NULL;
static lv_obj_t	*pageDetailSet_switchReaction_btnm = NULL;
static lv_obj_t	*pageDetailSet_switchReaction_labelBtnmRef = NULL;

static char strTemp_labelObjRef[20] = {0};

static lv_style_t styleLabelRef_pageDetailSet;

static stt_devDataPonitTypedef deviceDataPoint_valSetTemp[PAGEREACTION_REUSE_GLOBAL_NUM] = {0};
static uint8_t currentDataPoint_valSetTemp_insert = 0;

static const lv_coord_t objSize_switchReaction_sw[2] = {48, 18};

static lv_res_t sw1FunCb_trigAction_switchReaction_valSet(lv_obj_t *sw){

	LV_OBJ_FREE_NUM_TYPE swFreeNum = lv_obj_get_free_num(sw);

//	switch(swFreeNum){
//	 
//		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

//		default:break;
//	}

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_oneBit.swVal_bit1 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_oneBit.swVal_bit1 = 0);

		}break;
		
		case devTypeDef_mulitSwTwoBit:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_twoBit.swVal_bit1 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_twoBit.swVal_bit1 = 0);

		}break;
		
		case devTypeDef_mulitSwThreeBit:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit1 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit1 = 0);

		}break;
		
		case devTypeDef_thermostat:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_thermostat.devThermostat_running_en = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_thermostat.devThermostat_running_en = 0);

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t sw2FunCb_trigAction_switchReaction_valSet(lv_obj_t *sw){

	LV_OBJ_FREE_NUM_TYPE swFreeNum = lv_obj_get_free_num(sw);

//	switch(swFreeNum){
//	 
//		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

//		default:break;
//	}

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwTwoBit:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_twoBit.swVal_bit2 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_twoBit.swVal_bit2 = 0);

		}break;
		
		case devTypeDef_mulitSwThreeBit:{

			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit2 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit2 = 0);

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t sw3FunCb_trigAction_switchReaction_valSet(lv_obj_t *sw){

	LV_OBJ_FREE_NUM_TYPE swFreeNum = lv_obj_get_free_num(sw);

//	switch(swFreeNum){
//	 
//		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

//		default:break;
//	}

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwThreeBit:{
			
			(lv_sw_get_state(sw))?
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit3 = 1):
				(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_mulitSwitch_threeBit.swVal_bit3 = 0);

		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t sliderFunCb_slidAction_switchReaction_valSet(lv_obj_t *slider){

	LV_OBJ_FREE_NUM_TYPE sliderFreeNum = lv_obj_get_free_num(slider);
	int16_t sliderVal = lv_slider_get_value(slider);

//	printf("page switchReaction detailSet, slider valSet:%d.\n", sliderVal);

	switch(currentDev_typeGet()){

		case devTypeDef_dimmer:{

			deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_dimmer.devDimmer_brightnessVal = //显示数据
					sliderVal * (DEVICE_DIMMER_BRIGHTNESS_MAX_VAL / devDimmer_pageSet_brightnessOpreat_div);

			sprintf(strTemp_labelObjRef, "brightness(%d%%):", deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_dimmer.devDimmer_brightnessVal);
			lv_label_set_text(pageDetailSet_switchReaction_labelSliderRef, strTemp_labelObjRef);

			switch(sliderFreeNum){ //真实数据
			 
				case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{

					deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_dimmer.devDimmer_brightnessVal = sliderVal;		

				}break;
				
				case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

				default:break;
			}

		}break;
		
		case devTypeDef_curtain:{

			deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_curtain.devCurtain_actEnumVal = //显示数据
					sliderVal * (DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL / devCurtain_pageSet_positionOpreat_div);

			sprintf(strTemp_labelObjRef, "position(%d%%):", deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_curtain.devCurtain_actEnumVal);
			lv_label_set_text(pageDetailSet_switchReaction_labelSliderRef, strTemp_labelObjRef);

			switch(sliderFreeNum){ //真实数据
			 
				case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{

					deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_curtain.devCurtain_actEnumVal = sliderVal;

				}break;
				
				case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

				default:break;
			}
			
		}break;

		default:break;
	}

	return LV_RES_OK;
}

static lv_res_t btnmFunCb_clickAction_switchReaction_valSet(lv_obj_t *btnm, const char *txt){

	LV_OBJ_FREE_NUM_TYPE btnmFreeNum = lv_obj_get_free_num(btnm);
	uint16_t toggleNum = lv_btnm_get_toggled(btnm);
	const char **btnmStrCmp_temp = NULL;
	uint8_t loopCmp_maxNum = 0;
	uint8_t loop = 0;

//	switch(btnmFreeNum){
//	 
//		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:{}break;
//		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{}break;

//		default:break;
//	}

	{ //强制获取当前toggle值，否则lv_btnm_get_toggled只能获取上一toggle值

		switch(currentDev_typeGet()){

			case devTypeDef_fans:{

				btnmStrCmp_temp = btnm_str_devFans; 
				loopCmp_maxNum = DEVICE_FANS_OPREAT_ACTION_NUM + 1;
			
			}break;

			case devTypeDef_scenario:{

				btnmStrCmp_temp = btnm_str_devScenario;
				loopCmp_maxNum = DEVSCENARIO_OPREATION_OBJ_NUM + 1;

			}break;

			case devTypeDef_heater:{

				btnmStrCmp_temp = btnm_str_devHeater; 
				loopCmp_maxNum = DEVICE_HEATER_OPREAT_ACTION_NUM + 1;
			
			}break;

			default:break;
		}

		for(loop = 0; loop < loopCmp_maxNum; loop ++){

			if(!strcmp(txt, btnmStrCmp_temp[loop])){

				toggleNum = loop;
				break;
			}
		}		
	}

//	printf("page switchReaction detailSet, btnm toggleNum:%d.\n", toggleNum);

	switch(currentDev_typeGet()){

		case devTypeDef_fans:{

			deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_fans.devFans_swEnumVal = toggleNum;	

			sprintf(strTemp_labelObjRef, "gear: %s", btnm_str_devFans[deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_fans.devFans_swEnumVal]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);
			lv_obj_refresh_style(pageDetailSet_switchReaction_labelBtnmRef);

		}break;
		
		case devTypeDef_scenario:{

			uint8_t scenarioIstDisp_Temp = 0;

			deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_scenario.devScenario_opNum = 1 << toggleNum;

			scenarioIstDisp_Temp = devDriverBussiness_scnarioSwitch_swVal2Insert(deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_scenario.devScenario_opNum);

			sprintf(strTemp_labelObjRef, "scenario: %s", btnm_str_devScenario[scenarioIstDisp_Temp - 1]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);			
			lv_obj_refresh_style(pageDetailSet_switchReaction_labelBtnmRef);

		}break;
		
		case devTypeDef_heater:{

			deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_heater.devHeater_swEnumVal = toggleNum;	
			
			sprintf(strTemp_labelObjRef, "gear: %s", btnm_str_devHeater[deviceDataPoint_valSetTemp[currentDataPoint_valSetTemp_insert].devType_heater.devHeater_swEnumVal]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);
			lv_obj_refresh_style(pageDetailSet_switchReaction_labelBtnmRef);

//			printf("testPoint, strDisp is:%s .\n", strTemp_labelObjRef);
			
		}break;

		default:break;
	}

	return LV_RES_OK;
}


void lvGui_businessReuse_objStyle_Init(void){

	lv_style_copy(&styleLabelRef_pageDetailSet, &lv_style_plain);
	styleLabelRef_pageDetailSet.text.font = &lv_font_consola_17;
	styleLabelRef_pageDetailSet.text.color = LV_COLOR_BLACK;
}

void lvGui_businessReuse_reactionObjPageElement_creat(lv_obj_t *obj_Parent, uint8_t pageObjIst, lv_coord_t cy, stt_devDataPonitTypedef *cfgDataCurrent){

	const uint8_t objSlider_opreationValDiv_default = 10;

	uint8_t pageObjIst_local = pageObjIst - PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE;
	lv_coord_t objPosOfs_x = 0;
			   
	switch(pageObjIst){
	 
		case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{

			objPosOfs_x = 50;
			
			devDimmer_pageSet_brightnessOpreat_div = DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV;
			devCurtain_pageSet_positionOpreat_div = DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV;

		}break;
		
		case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:{

			objPosOfs_x = 50;

			devDimmer_pageSet_brightnessOpreat_div = objSlider_opreationValDiv_default;
			devCurtain_pageSet_positionOpreat_div = objSlider_opreationValDiv_default;

		}break;
		
		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:
		case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{

			objPosOfs_x = 30;

			devDimmer_pageSet_brightnessOpreat_div = objSlider_opreationValDiv_default;
			devCurtain_pageSet_positionOpreat_div = objSlider_opreationValDiv_default;

		}break;

		default:break;
	}

	lvGui_businessReuse_objStyle_Init();

	memset(&deviceDataPoint_valSetTemp[pageObjIst_local], 0, sizeof(stt_devDataPonitTypedef));
	memcpy(&deviceDataPoint_valSetTemp[pageObjIst_local], cfgDataCurrent, sizeof(stt_devDataPonitTypedef));

	currentDataPoint_valSetTemp_insert = pageObjIst_local;

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:{

			pageDetailSet_switchReaction_labelSwRef[0] = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSwRef[0], &styleLabelRef_pageDetailSet);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[0], "switch_A: ");
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSwRef[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[0], obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x, cy + 30);

			pageDetailSet_switchReaction_sw[0] = lv_sw_create(obj_Parent, NULL);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[0], sw1FunCb_trigAction_switchReaction_valSet);
			lv_obj_set_protect(pageDetailSet_switchReaction_sw[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_sw[0], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_RIGHT_MID, 5, 0);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[0], pageObjIst);
			lv_obj_set_size(pageDetailSet_switchReaction_sw[0], objSize_switchReaction_sw[0], objSize_switchReaction_sw[1]);

			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_oneBit.swVal_bit1)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[0])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[0]));

		}break;
		
		case devTypeDef_mulitSwTwoBit:{

			pageDetailSet_switchReaction_labelSwRef[0] = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSwRef[0], &styleLabelRef_pageDetailSet);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[0], "switch_A: ");
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSwRef[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[0], obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x, cy + 15);
			pageDetailSet_switchReaction_labelSwRef[1] = lv_label_create(obj_Parent, pageDetailSet_switchReaction_labelSwRef[0]);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[1], "switch_B: ");
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[1], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);

			pageDetailSet_switchReaction_sw[0] = lv_sw_create(obj_Parent, NULL);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[0], sw1FunCb_trigAction_switchReaction_valSet);
			lv_obj_set_protect(pageDetailSet_switchReaction_sw[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_sw[0], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_RIGHT_MID, 5, 7);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[0], pageObjIst);
			lv_obj_set_size(pageDetailSet_switchReaction_sw[0], objSize_switchReaction_sw[0], objSize_switchReaction_sw[1]);
			pageDetailSet_switchReaction_sw[1] = lv_sw_create(obj_Parent, pageDetailSet_switchReaction_sw[0]);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[1], sw2FunCb_trigAction_switchReaction_valSet);
			lv_obj_align(pageDetailSet_switchReaction_sw[1], pageDetailSet_switchReaction_sw[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[1], pageObjIst);

			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_twoBit.swVal_bit1)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[0])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[0]));
			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_twoBit.swVal_bit2)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[1])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[1]));
				
		}break;
		
		case devTypeDef_mulitSwThreeBit:{

			pageDetailSet_switchReaction_labelSwRef[0] = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSwRef[0], &styleLabelRef_pageDetailSet);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[0], "switch_A: ");
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSwRef[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[0], obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x, cy);
			pageDetailSet_switchReaction_labelSwRef[1] = lv_label_create(obj_Parent, pageDetailSet_switchReaction_labelSwRef[0]);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[1], "switch_B: ");
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[1], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			pageDetailSet_switchReaction_labelSwRef[2] = lv_label_create(obj_Parent, pageDetailSet_switchReaction_labelSwRef[0]);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[2], "switch_C: ");
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[2], pageDetailSet_switchReaction_labelSwRef[1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
//			lv_obj_set_pos(pageDetailSet_switchReaction_labelSwRef[2], 40, 60);
//			lv_obj_set_top(pageDetailSet_switchReaction_labelSwRef[0], true);
//			lv_obj_set_top(pageDetailSet_switchReaction_labelSwRef[1], true);
//			lv_obj_set_top(pageDetailSet_switchReaction_labelSwRef[2], true);

			pageDetailSet_switchReaction_sw[0] = lv_sw_create(obj_Parent, NULL);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[0], sw1FunCb_trigAction_switchReaction_valSet);
			lv_obj_set_protect(pageDetailSet_switchReaction_sw[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_sw[0], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_RIGHT_MID, 5, 7);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[0], pageObjIst);
			lv_obj_set_size(pageDetailSet_switchReaction_sw[0], objSize_switchReaction_sw[0], objSize_switchReaction_sw[1]);
			pageDetailSet_switchReaction_sw[1] = lv_sw_create(obj_Parent, pageDetailSet_switchReaction_sw[0]);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[1], sw2FunCb_trigAction_switchReaction_valSet);
			lv_obj_align(pageDetailSet_switchReaction_sw[1], pageDetailSet_switchReaction_sw[0], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[1], pageObjIst);
			pageDetailSet_switchReaction_sw[2] = lv_sw_create(obj_Parent, pageDetailSet_switchReaction_sw[0]);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[2], sw3FunCb_trigAction_switchReaction_valSet);
			lv_obj_align(pageDetailSet_switchReaction_sw[2], pageDetailSet_switchReaction_sw[1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[2], pageObjIst);

			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_threeBit.swVal_bit1)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[0])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[0]));
			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_threeBit.swVal_bit2)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[1])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[1]));
			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_mulitSwitch_threeBit.swVal_bit3)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[2])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[2]));

		}break;

		case devTypeDef_thermostat:{

			pageDetailSet_switchReaction_labelSwRef[0] = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSwRef[0], &styleLabelRef_pageDetailSet);
			lv_label_set_text(pageDetailSet_switchReaction_labelSwRef[0], "thermostat open: ");
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSwRef[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSwRef[0], obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x, cy);
			
			pageDetailSet_switchReaction_sw[0] = lv_sw_create(obj_Parent, NULL);
			lv_sw_set_action(pageDetailSet_switchReaction_sw[0], sw1FunCb_trigAction_switchReaction_valSet);
			lv_obj_set_protect(pageDetailSet_switchReaction_sw[0], LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_sw[0], pageDetailSet_switchReaction_labelSwRef[0], LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
			lv_obj_set_free_num(pageDetailSet_switchReaction_sw[0], pageObjIst);
			lv_obj_set_size(pageDetailSet_switchReaction_sw[0], objSize_switchReaction_sw[0], objSize_switchReaction_sw[1]);

			(deviceDataPoint_valSetTemp[pageObjIst_local].devType_thermostat.devThermostat_running_en)?
				(lv_sw_on(pageDetailSet_switchReaction_sw[0])):
				(lv_sw_off(pageDetailSet_switchReaction_sw[0]));

		}break;
		
		case devTypeDef_dimmer:{

			uint8_t brightnessSliderDisp_Temp = 0;
			uint8_t brightnessLabelDisp_Temp = 0;

			switch(pageObjIst){ //显示数据预处理
			 
				case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{

					brightnessSliderDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_dimmer.devDimmer_brightnessVal;
					brightnessLabelDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_dimmer.devDimmer_brightnessVal *
													(DEVICE_DIMMER_BRIGHTNESS_MAX_VAL / devDimmer_pageSet_brightnessOpreat_div);

				}break;
				
				case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{

					brightnessSliderDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_dimmer.devDimmer_brightnessVal /
													(DEVICE_DIMMER_BRIGHTNESS_MAX_VAL / devDimmer_pageSet_brightnessOpreat_div);
					brightnessLabelDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_dimmer.devDimmer_brightnessVal;
					
				}break;

				default:break;
			}

			pageDetailSet_switchReaction_labelSliderRef = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSliderRef, &styleLabelRef_pageDetailSet);
			sprintf(strTemp_labelObjRef, "brightness(%d%%):", brightnessLabelDisp_Temp);
			lv_label_set_text(pageDetailSet_switchReaction_labelSliderRef, strTemp_labelObjRef);
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSliderRef, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSliderRef, obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x - 10, cy + 10);

			pageDetailSet_switchReaction_slider = lv_slider_create(obj_Parent, NULL);
			lv_obj_set_size(pageDetailSet_switchReaction_slider, 160, 30);
			lv_obj_set_protect(pageDetailSet_switchReaction_slider, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_slider, pageDetailSet_switchReaction_labelSliderRef, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_slider_set_action(pageDetailSet_switchReaction_slider, sliderFunCb_slidAction_switchReaction_valSet);
			lv_slider_set_range(pageDetailSet_switchReaction_slider, 0, devDimmer_pageSet_brightnessOpreat_div);
			lv_obj_set_free_num(pageDetailSet_switchReaction_slider, pageObjIst);

			lv_slider_set_value(pageDetailSet_switchReaction_slider, brightnessSliderDisp_Temp);

		}break;

		case devTypeDef_curtain:{

			uint8_t positionSliderDisp_Temp = 0;
			uint8_t positionLabelDisp_Temp = 0;

			switch(pageObjIst){ //显示数据处理

				case PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST:{

					positionSliderDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_curtain.devCurtain_actEnumVal;
					positionLabelDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_curtain.devCurtain_actEnumVal *
											  	(DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL / devCurtain_pageSet_positionOpreat_div);
		
				}break;

				case PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST:
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP:
				case PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX:{

					positionSliderDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_curtain.devCurtain_actEnumVal /
											  	(DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL / devCurtain_pageSet_positionOpreat_div);
					positionLabelDisp_Temp = deviceDataPoint_valSetTemp[pageObjIst_local].devType_curtain.devCurtain_actEnumVal;
												
				}break;

				default:break;
			}

			pageDetailSet_switchReaction_labelSliderRef = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelSliderRef, &styleLabelRef_pageDetailSet);
			sprintf(strTemp_labelObjRef, "position(%d%%):", positionLabelDisp_Temp);
			lv_label_set_text(pageDetailSet_switchReaction_labelSliderRef, strTemp_labelObjRef);
			lv_obj_set_protect(pageDetailSet_switchReaction_labelSliderRef, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelSliderRef, obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x - 10, cy + 10);
			
			pageDetailSet_switchReaction_slider = lv_slider_create(obj_Parent, NULL);
			lv_obj_set_size(pageDetailSet_switchReaction_slider, 160, 30);
			lv_obj_set_protect(pageDetailSet_switchReaction_slider, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_slider, pageDetailSet_switchReaction_labelSliderRef, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_slider_set_action(pageDetailSet_switchReaction_slider, sliderFunCb_slidAction_switchReaction_valSet);
			lv_slider_set_range(pageDetailSet_switchReaction_slider, 0, devCurtain_pageSet_positionOpreat_div);
			lv_obj_set_free_num(pageDetailSet_switchReaction_slider, pageObjIst);

			lv_slider_set_value(pageDetailSet_switchReaction_slider, positionSliderDisp_Temp);

		}break;
		
		case devTypeDef_fans:{

			pageDetailSet_switchReaction_labelBtnmRef = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelBtnmRef, &styleLabelRef_pageDetailSet);
			sprintf(strTemp_labelObjRef, "gear: %s", btnm_str_devFans[deviceDataPoint_valSetTemp[pageObjIst_local].devType_fans.devFans_swEnumVal]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);
			lv_obj_set_protect(pageDetailSet_switchReaction_labelBtnmRef, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelBtnmRef, obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x - 10, cy + 10);

			pageDetailSet_switchReaction_btnm = lv_btnm_create(obj_Parent, NULL);
			lv_obj_set_size(pageDetailSet_switchReaction_btnm, 160, 40);
			lv_obj_set_protect(pageDetailSet_switchReaction_btnm, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_btnm, pageDetailSet_switchReaction_labelBtnmRef, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_btnm_set_action(pageDetailSet_switchReaction_btnm, btnmFunCb_clickAction_switchReaction_valSet);
			lv_btnm_set_map(pageDetailSet_switchReaction_btnm, btnm_str_devFans);
			lv_obj_set_free_num(pageDetailSet_switchReaction_btnm, pageObjIst);

			lv_btnm_set_toggle(pageDetailSet_switchReaction_btnm, true, deviceDataPoint_valSetTemp[pageObjIst_local].devType_fans.devFans_swEnumVal);			

		}break;
		
		case devTypeDef_scenario:{

			uint8_t scenarioIstDisp_Temp = devDriverBussiness_scnarioSwitch_swVal2Insert(deviceDataPoint_valSetTemp[pageObjIst_local].devType_scenario.devScenario_opNum);

			pageDetailSet_switchReaction_labelBtnmRef = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelBtnmRef, &styleLabelRef_pageDetailSet);
			sprintf(strTemp_labelObjRef, "scenario: %s", btnm_str_devScenario[scenarioIstDisp_Temp - 1]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);
			lv_obj_set_protect(pageDetailSet_switchReaction_labelBtnmRef, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelBtnmRef, obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x - 10, cy + 10);
			
			pageDetailSet_switchReaction_btnm = lv_btnm_create(obj_Parent, NULL);
			lv_obj_set_size(pageDetailSet_switchReaction_btnm, 160, 40);
			lv_obj_set_protect(pageDetailSet_switchReaction_btnm, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_btnm, pageDetailSet_switchReaction_labelBtnmRef, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_btnm_set_action(pageDetailSet_switchReaction_btnm, btnmFunCb_clickAction_switchReaction_valSet);
			lv_btnm_set_map(pageDetailSet_switchReaction_btnm, btnm_str_devScenario);
			lv_obj_set_free_num(pageDetailSet_switchReaction_btnm, pageObjIst);

			lv_btnm_set_toggle(pageDetailSet_switchReaction_btnm, true, scenarioIstDisp_Temp - 1);	

		}break;
		
		case devTypeDef_heater:{

			pageDetailSet_switchReaction_labelBtnmRef = lv_label_create(obj_Parent, NULL);
			lv_label_set_style(pageDetailSet_switchReaction_labelBtnmRef, &styleLabelRef_pageDetailSet);
			sprintf(strTemp_labelObjRef, "gear: %s", btnm_str_devHeater[deviceDataPoint_valSetTemp[pageObjIst_local].devType_heater.devHeater_swEnumVal]);
			lv_label_set_text(pageDetailSet_switchReaction_labelBtnmRef, strTemp_labelObjRef);
			lv_obj_set_protect(pageDetailSet_switchReaction_labelBtnmRef, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_labelBtnmRef, obj_Parent, LV_ALIGN_IN_TOP_LEFT, objPosOfs_x - 10, cy + 10);
			
			pageDetailSet_switchReaction_btnm = lv_btnm_create(obj_Parent, NULL);
			lv_obj_set_size(pageDetailSet_switchReaction_btnm, 160, 40);
			lv_obj_set_protect(pageDetailSet_switchReaction_btnm, LV_PROTECT_POS);
			lv_obj_align(pageDetailSet_switchReaction_btnm, pageDetailSet_switchReaction_labelBtnmRef, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
			lv_btnm_set_action(pageDetailSet_switchReaction_btnm, btnmFunCb_clickAction_switchReaction_valSet);
			lv_btnm_set_map(pageDetailSet_switchReaction_btnm, btnm_str_devHeater);
			lv_obj_set_free_num(pageDetailSet_switchReaction_btnm, pageObjIst);

			lv_btnm_set_toggle(pageDetailSet_switchReaction_btnm, true, deviceDataPoint_valSetTemp[pageObjIst_local].devType_heater.devHeater_swEnumVal);

		}break;

		default:break;
	}
}

void lvGui_businessReuse_reactionObjPageElement_funValConfig_get(uint8_t pageIst, stt_devDataPonitTypedef *dataPointReaction){

	memcpy(dataPointReaction, &deviceDataPoint_valSetTemp[pageIst - PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE], sizeof(stt_devDataPonitTypedef));
}



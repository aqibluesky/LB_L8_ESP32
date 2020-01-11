#include "devDriver_scenario.h"

#include "os.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "dataTrans_localHandler.h"

#include "bussiness_timerSoft.h"

extern EventGroupHandle_t xEventGp_devApplication;

static const char *TAG = "lanbon_L8 - devDriverScenario";

static uint16_t scnarioSwitchDriver_clamDown_counter = 0;

uint8_t devDriverBussiness_scnarioSwitch_swVal2Insert(uint8_t swVal){

	switch(swVal){

		case 1:return 1;
		case 2:return 2;
		case 4:
		default:return 3;
	}
}

void devDriverBussiness_scnarioSwitch_moudleInit(void){

	
}

void devDriverBussiness_scnarioSwitch_moudleDeinit(void){

	
}

void devDriverBussiness_scnarioSwitch_driverClamDown_refresh(void){

	if(scnarioSwitchDriver_clamDown_counter)
		scnarioSwitchDriver_clamDown_counter --;
}

uint16_t devDriverBussiness_scnarioSwitch_driverClamDown_get(void){

	return scnarioSwitchDriver_clamDown_counter;
}

void devDriverBussiness_scnarioSwitch_dataParam_save(stt_scenarioSwitchData_nvsOpreat *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_scenario){

		enum_dataSaveObj objScenario_saveIst = saveObj_devScenario_paramDats_2;

		switch(param->dataRef.scenarioInsert_num){

			case 1:objScenario_saveIst = saveObj_devScenario_paramDats_0;break;
			case 2:objScenario_saveIst = saveObj_devScenario_paramDats_1;break;
			case 3:
			default:objScenario_saveIst = saveObj_devScenario_paramDats_2;break;
		}

//		printf("devSum:%d, macList set halfA[0]:"MACSTR",swVal:%02X.\n", param->dataRef.scenarioDevice_sum,
//																		 MAC2STR(param->dataHalf_A[0].unitDevMac),
//																		 param->dataHalf_A[0].unitDevOpreat_val);

//		printf("devSum:%d, macList set halfA[1]:"MACSTR",swVal:%02X.\n", param->dataRef.scenarioDevice_sum,
//																		 MAC2STR(param->dataHalf_A[1].unitDevMac),
//																		 param->dataHalf_A[1].unitDevOpreat_val);

//		printf("devSum:%d, macList set halfA[2]:"MACSTR",swVal:%02X.\n", param->dataRef.scenarioDevice_sum,
//																		 MAC2STR(param->dataHalf_A[2].unitDevMac),
//																		 param->dataHalf_A[2].unitDevOpreat_val);

		devSystemInfoLocalRecord_save(objScenario_saveIst, param);
	}
}

static void devDriverBussiness_scnarioSwitch_bussinessDataReq(uint8_t dstMac[6], uint8_t opreatVal){

	mdf_err_t ret = MDF_OK;
	
	const mlink_httpd_type_t type_L8mesh_cst = {
	
		.format = MLINK_HTTPD_FORMAT_HEX,
	};
	mwifi_data_type_t data_type = {
		
		.compression = true,
		.communicate = MWIFI_COMMUNICATE_UNICAST,
	};

	uint8_t meshDatsRequest_temp[2] = {0};

	meshDatsRequest_temp[0] = L8DEV_MESH_CMD_SCENARIO_CTRL;
	meshDatsRequest_temp[1] = opreatVal;

	memcpy(&(data_type.custom), &type_L8mesh_cst, sizeof(uint32_t));
	
	if(esp_mesh_get_layer() == MESH_ROOT){
	
		ret = mwifi_root_write((const uint8_t *)dstMac, 1, &data_type, meshDatsRequest_temp, 1 + 1, true);	//数据内容填充 -头命令长度1 + 操作值数据长度1
		MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 
	}
	else
	{
		ret = mwifi_write((const uint8_t *)dstMac, &data_type, meshDatsRequest_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
		MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> heartbeat mwifi_root_write", mdf_err_to_name(ret));
	}
}

void devDriverBussiness_scnarioSwitch_scenarioStatusReales(stt_devDataPonitTypedef *param){

	scnarioSwitchDriver_clamDown_counter = DEVSCENARIO_DRIVER_CALMDOWN_PERIOD; //UI按键冷却触发

	if(systemDevice_startUpTime_get() < 2)return; //场景开关不需要状态记录恢复业务逻辑，根据开机时间跳过既定业务

//	printf("watch point!!!.\n");

	xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_DEVDRV_SCENARIO);
}

void devDriverBussiness_scnarioSwitch_actionTrig(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	stt_devDataPonitTypedef devParam = {0};
	
	uint8_t dataReq_loop = 0;

	currentDev_dataPointGet(&devParam);

	if(swCurrentDevType == devTypeDef_scenario){

		if(mwifi_is_connected()){

			uint8_t scenarioDataParam_ist = 0;
			stt_scenarioSwitchData_nvsOpreat *scenarioParamData = NULL;

			scenarioDataParam_ist = devDriverBussiness_scnarioSwitch_swVal2Insert(devParam.devType_scenario.devScenario_opNum);

			if(!scenarioDataParam_ist)return; //参数无效，不予驱动

			scenarioParamData = nvsDataOpreation_devScenarioParam_get(scenarioDataParam_ist);

			printf("devSum:%d, macList get halfA[0]:"MACSTR",swVal:%02X.\n", scenarioParamData->dataRef.scenarioDevice_sum,
																			 MAC2STR(scenarioParamData->dataHalf_A[0].unitDevMac),
										   				  	   				 scenarioParamData->dataHalf_A[0].unitDevOpreat_val);

			if(scenarioParamData->dataRef.scenarioDevice_sum <= DEVSCENARIO_NVSDATA_HALFOPREAT_NUM){
			
				for(dataReq_loop = 0; dataReq_loop < scenarioParamData->dataRef.scenarioDevice_sum; dataReq_loop ++){
			
					devDriverBussiness_scnarioSwitch_bussinessDataReq(scenarioParamData->dataHalf_A[dataReq_loop].unitDevMac,
																	  scenarioParamData->dataHalf_A[dataReq_loop].unitDevOpreat_val);
				}
			}
			else
			{
				uint8_t dataReqLoop_reserve = scenarioParamData->dataRef.scenarioDevice_sum - DEVSCENARIO_NVSDATA_HALFOPREAT_NUM;

				for(dataReq_loop = 0; dataReq_loop < DEVSCENARIO_NVSDATA_HALFOPREAT_NUM; dataReq_loop ++){
				
					devDriverBussiness_scnarioSwitch_bussinessDataReq(scenarioParamData->dataHalf_A[dataReq_loop].unitDevMac,
																	  scenarioParamData->dataHalf_A[dataReq_loop].unitDevOpreat_val);
				}
				
				for(dataReq_loop = 0; dataReq_loop < dataReqLoop_reserve; dataReq_loop ++){
				
					devDriverBussiness_scnarioSwitch_bussinessDataReq(scenarioParamData->dataHalf_B[dataReq_loop].unitDevMac,
																	  scenarioParamData->dataHalf_B[dataReq_loop].unitDevOpreat_val);
				}
			}
			
			os_free(scenarioParamData);
		}
	}	
}
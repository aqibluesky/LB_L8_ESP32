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

static const char *TAG = "lanbon_L8 - devDriverScenario";

static uint16_t scnarioSwitchDriver_clamDown_counter = 0;

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

			case 0:objScenario_saveIst = saveObj_devScenario_paramDats_0;break;
			case 1:objScenario_saveIst = saveObj_devScenario_paramDats_1;break;
			case 2:
			default:objScenario_saveIst = saveObj_devScenario_paramDats_2;break;
		}

		devSystemInfoLocalRecord_save(objScenario_saveIst, param);
	}
}

static void devDriverBussiness_scnarioSwitch_bussinessDataReq(uint8_t dstMac[6], uint8_t opreatVal){

	mdf_err_t ret = MDF_OK;
	mwifi_data_type_t data_type = {
		
		.compression = true,
		.communicate = MWIFI_COMMUNICATE_UNICAST,
	};

	uint8_t meshDatsRequest_temp[2] = {0};

	meshDatsRequest_temp[0] = L8DEV_MESH_CMD_SCENARIO_CTRL;
	meshDatsRequest_temp[1] = opreatVal;
	
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

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	
	uint8_t dataReq_loop = 0;

	if(swCurrentDevType == devTypeDef_scenario){

		if(systemDevice_startUpTime_get() < 2)return; //场景开关不需要状态记录恢复业务逻辑，根据开机时间跳过既定业务

		scnarioSwitchDriver_clamDown_counter = DEVSCENARIO_DRIVER_CALMDOWN_PERIOD;

		if(mwifi_is_connected()){

			uint8_t scenarioDataParam_ist = 0;
			stt_scenarioSwitchData_nvsOpreat *scenarioParamData = NULL;

			switch(param->devType_scenario.devScenario_opNum){

				case 1:scenarioDataParam_ist = 1;break;
				case 2:scenarioDataParam_ist = 2;break;
				case 4:scenarioDataParam_ist = 3;break;
				default:return; //参数无效，不予驱动
			}

			scenarioParamData = nvsDataOpreation_devScenarioParam_get(scenarioDataParam_ist);

			if(scenarioParamData->dataRef.scenarioDevice_sum <= DEVSCENARIO_NVSDATA_HALFOPREAT_NUM){
			
				for(dataReq_loop = 0; dataReq_loop < scenarioParamData->dataRef.scenarioDevice_sum; dataReq_loop ++){
			
					devDriverBussiness_scnarioSwitch_bussinessDataReq(scenarioParamData->dataHalf_A[dataReq_loop].unitDevMac,
																	  scenarioParamData->dataHalf_A[dataReq_loop].unitDevOpreat_val);
				}
			}
			else{

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


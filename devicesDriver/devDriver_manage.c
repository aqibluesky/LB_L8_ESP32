#include "devDriver_manage.h"

#include "string.h"
#include "stdio.h"

#include "dataTrans_localHandler.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

extern void pageHome_buttonMain_imageRefresh(bool freshNoRecord);

extern EventGroupHandle_t xEventGp_devApplication;

static const char *TAG = "lanbon_L8 - devDriverManage";

static stt_devDataPonitTypedef lanbon_l8device_currentDataPoint = {0}; //数据点记录比对值，用于记录上一数据点
static stt_devDataPonitTypedef deviceDataPointRecord_lastTime = {0};
static devTypeDef_enum	lanbon_l8device_currentDevType = devTypeDef_mulitSwThreeBit;

static stt_devStatusRecord deviceStatusRecordIF_Flg = {0};

static void usrAppMutualCtrl_actionTrig(void);

devTypeDef_enum IRAM_ATTR currentDev_typeGet(void){ //需在中断内调用，加IRAM_ATTR

	return lanbon_l8device_currentDevType;
}

void currentDev_typeSet(devTypeDef_enum devType, bool nvsRecord_IF){

	lanbon_l8device_currentDevType = devType;
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devTypeDef, &lanbon_l8device_currentDevType);
}

void devStatusRecordIF_paramSet(stt_devStatusRecord *param, bool nvsRecord_IF){

	memcpy(&deviceStatusRecordIF_Flg, param, sizeof(stt_devStatusRecord));
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devStatusRecordIF, &deviceStatusRecordIF_Flg);
}

void devStatusRecordIF_paramGet(stt_devStatusRecord *param){

	memcpy(param, &deviceStatusRecordIF_Flg, sizeof(stt_devStatusRecord));
}

void currentDev_dataPointSet(stt_devDataPonitTypedef *param, bool nvsRecord_IF, bool mutualCtrlTrig_IF, bool statusUploadMedthod){

	//常规响应
	memcpy(&deviceDataPointRecord_lastTime, &lanbon_l8device_currentDataPoint, sizeof(stt_devDataPonitTypedef)); //设备状态数据点记录,当前状态转为历史状态，用于互控比对
	memcpy(&lanbon_l8device_currentDataPoint, param, sizeof(stt_devDataPonitTypedef)); //常规响应

	switch(lanbon_l8device_currentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:{

			devDriverBussiness_mulitBitSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_thermostat:{

			devDriverBussiness_thermostatSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_dimmer:{

			devDriverBussiness_dimmerSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_curtain:{

			devDriverBussiness_curtainSwitch_periphStatusRealesBySlide(param);

		}break;
		
		case devTypeDef_fans:{

			devDriverBussiness_fansSwitch_periphStatusReales(param);
		
		}break;
	
		case devTypeDef_scenario:{

			devDriverBussiness_scnarioSwitch_scenarioStatusReales(param);
		
		}break;
		
		case devTypeDef_heater:{

			devDriverBussiness_heaterSwitch_periphStatusReales(param);

		}break;
		
		default:break;
	}
	
	//其他业务更新
	usrAppParamSet_hbRealesInAdvance(statusUploadMedthod);
	pageHome_buttonMain_imageRefresh(false);
	if(nvsRecord_IF)usrAppDevStatusRecord_delayActionTrig(); //动作延时触发，避免高频存储
	if(mutualCtrlTrig_IF)usrAppMutualCtrl_actionTrig();
	usrApp_GreenMode_trig(); //绿色模式业务
	devScreenBkLight_weakUp(); //屏幕唤醒
}

void currentDev_extParamSet(void *param){

	uint8_t *dataParam = (uint8_t *)param;
	uint8_t devType_rcv = dataParam[4];

	if(devType_rcv == currentDev_typeGet()){

		switch(currentDev_typeGet()){

			case devTypeDef_curtain:{

				uint8_t orbitalPeriodTime_valSet = dataParam[0];
			
				devCurtain_currentOrbitalPeriodTimeSet(orbitalPeriodTime_valSet);	

			}break;
			
			case devTypeDef_heater:{

				uint16_t customDownCounter_valSet = ((uint16_t)(dataParam[0]) << 8) | 
													((uint16_t)(dataParam[1]) << 0);

				devDriverBussiness_heaterSwitch_closePeriodCustom_Set(customDownCounter_valSet, true);

			}break;

			default:break;
		}
	}
}

void currentDev_dataPointGet(stt_devDataPonitTypedef *param){

	memcpy(param, &lanbon_l8device_currentDataPoint, sizeof(stt_devDataPonitTypedef));
}

void funcation_usrAppMutualCtrlActionTrig(void){

	stt_devMutualGroupParam devMutualCtrlGroup_dataTemp[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	uint8_t mutualCtrlDevMacList[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP * MWIFI_ADDR_LEN] = {0};
	uint8_t mutualCtrlParam_data[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	uint8_t mutualCtrlGroup_insert[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	bool mutualCtrlAction_trigIf = false;

	devMutualCtrlGroupInfo_Get(devMutualCtrlGroup_dataTemp); //互控组信息数据获取

	//互控触发
	switch(lanbon_l8device_currentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:{

			uint8_t dataPointHexTemp_current = 0,
					dataPointHexTemp_record = 0;

			memcpy(&dataPointHexTemp_current, &lanbon_l8device_currentDataPoint, sizeof(uint8_t));
			memcpy(&dataPointHexTemp_record, &deviceDataPointRecord_lastTime, sizeof(uint8_t));

//			printf("mutual Info: currentData:%02X, recordData:%02X.\n", dataPointHexTemp_current,
//																	    dataPointHexTemp_record);

			for(uint8_t loopOpt = 0; loopOpt < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopOpt ++){ //互控位是否可用?

				if((dataPointHexTemp_record & (1 << loopOpt)) != (dataPointHexTemp_current & (1 << loopOpt))){

					if((devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
					   (devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){

						mutualCtrlGroup_insert[loopOpt] = devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert;
						(dataPointHexTemp_current & (1 << loopOpt))?
							(mutualCtrlParam_data[loopOpt] = 1):
							(mutualCtrlParam_data[loopOpt] = 0);

						mutualCtrlAction_trigIf = true;
					}
				}
			}
			
		}break;
		case devTypeDef_dimmer:{}break;
		case devTypeDef_fans:{}break;
		case devTypeDef_scenario:{}break;
		case devTypeDef_curtain:{}break;
		case devTypeDef_heater:{}break;

		default:break;
	}

	if(mutualCtrlAction_trigIf){

		if(mwifi_is_connected()){

#define DEVICE_MUTUAL_CTRL_REQ_DATALEN	3

			uint8_t dataMutualCtrlReq_temp[DEVICE_MUTUAL_CTRL_REQ_DATALEN] = {0};
			mdf_err_t ret = MDF_OK;
			mwifi_data_type_t data_type = {
				
				.compression = true,
				.communicate = MWIFI_COMMUNICATE_MULTICAST,
			};
			mlink_httpd_type_t type_L8mesh_cst = {
			
				.format = MLINK_HTTPD_FORMAT_HEX,
			};
			uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};

			esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
			
			memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

			for(uint8_t loopMutual = 0; loopMutual < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopMutual ++){

				if(!mutualCtrlGroup_insert[loopMutual])continue; //对应键位触发判断

				dataMutualCtrlReq_temp[0] = L8DEV_MESH_CMD_MUTUAL_CTRL;
				dataMutualCtrlReq_temp[1] = mutualCtrlGroup_insert[loopMutual];
				dataMutualCtrlReq_temp[2] = mutualCtrlParam_data[loopMutual];	

				usrAppMethod_mwifiMacAddrRemoveFromList(devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevMacList, 
														devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum, 
														sta_mac);
				devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum --;

				if(esp_mesh_get_layer() == MESH_ROOT){

					data_type.communicate = MWIFI_COMMUNICATE_MULTICAST;

					ret = mwifi_root_write(devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevMacList, 
										   devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum, 
										   &data_type, 
										   dataMutualCtrlReq_temp, 
										   sizeof(uint8_t) * DEVICE_MUTUAL_CTRL_REQ_DATALEN, 
										   true);
					MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mutualCtrl mwifi_translate", mdf_err_to_name(ret));
				}
				else
				{
					uint8_t loop = 0;
				
					data_type.communicate = MWIFI_COMMUNICATE_UNICAST;
				
					for(loop = 0; loop < devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum; loop ++){

						ret = mwifi_write(&(devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevMacList[loop * MWIFI_ADDR_LEN]), 
										  &data_type, 
										  dataMutualCtrlReq_temp, 
										  sizeof(uint8_t) * DEVICE_MUTUAL_CTRL_REQ_DATALEN, 
										  true);
						MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mutualCtrl loop:%d mwifi_translate", mdf_err_to_name(ret), loop);					
					}
				}
			}
		}
	}
}

static void usrAppMutualCtrl_actionTrig(void){ //触发互控所需要API的内存空间较大，所以通过事件传输到专用通信线程进行触发

	xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_MUTUALTRIG);
}

void devInfo_statusRecord_action(void){

	devSystemInfoLocalRecord_save(saveObj_swStatus, &lanbon_l8device_currentDataPoint);
}

void devDriverManageBussiness_initialition(void){

	static bool devDriver_elecDetect_initialition_Flg = false;
	static bool devDriver_tempDetect_initialition_Flg = false;
	static bool devDriver_tinfraActDetect_initialition_Flg = false;
	
	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(!devDriver_elecDetect_initialition_Flg){

		devDriver_elecDetect_initialition_Flg = true;
		
		devDriverBussiness_elecMeasure_periphInit();
	}

	if(!devDriver_tempDetect_initialition_Flg){

		devDriver_tempDetect_initialition_Flg = true;

		devDriverBussiness_temperatureMeasure_periphInit();		
	}

	if(!devDriver_tinfraActDetect_initialition_Flg){

		devDriver_tinfraActDetect_initialition_Flg = true;

		devDriverBussiness_infraActDetect_periphInit();
	}
	
	switch(swCurrentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:{

			devDriverBussiness_mulitBitSwitch_moudleInit();

		}break;
		
		case devTypeDef_dimmer:{

			devDriverBussiness_dimmerSwitch_moudleInit();
			
		}break;
		
		case devTypeDef_fans:{

			devDriverBussiness_fansSwitch_moudleInit();

		}break;
		
		case devTypeDef_scenario:{

			devDriverBussiness_scnarioSwitch_moudleInit();
		
		}break;
		
		case devTypeDef_curtain:{

			devDriverBussiness_curtainSwitch_moudleInit();

		}break;
		
		case devTypeDef_heater:{

			devDriverBussiness_heaterSwitch_moudleInit();

		}break;

		case devTypeDef_thermostat:{

			devDriverBussiness_thermostatSwitch_moudleInit();
		
		}break;
		
		default:break;
	}
}

void devDriverManageBussiness_deinitialition(void){

	devDriverBussiness_mulitBitSwitch_moudleDeinit();
	devDriverBussiness_dimmerSwitch_moudleInit();
	devDriverBussiness_curtainSwitch_moudleDeinit();
	devDriverBussiness_fansSwitch_moudleDeinit();
	devDriverBussiness_heaterSwitch_moudleDeinit();
	devDriverBussiness_thermostatSwitch_moudleDeinit();
	devDriverBussiness_scnarioSwitch_moudleDeinit();
}

void devDriverManageBussiness_deviceChangeRefresh(void){

	devDriverManageBussiness_deinitialition();
	devDriverManageBussiness_initialition();
}



















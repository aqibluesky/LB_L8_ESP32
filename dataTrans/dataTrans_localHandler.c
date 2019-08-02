#include "dataTrans_localHandler.h"

#include "os.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

#include "devDriver_manage.h"

extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;
extern uint8_t listNum_nodeDevDataManage;

static const uint8_t L8_meshDataCmdLen = 1; //L8 mesh内部数据传输 头命令 长度

static const char *TAG = "lanbon_L8 - meshDataHandler";

static TimerHandle_t timerHandler_dataTransPeriod = NULL;

void dataHandler_devNodeMeshData(const uint8_t *src_addr, const mlink_httpd_type_t *type,
                       					 const void *dataRx, size_t dataLen){

	const uint8_t MACADDR_INSRT_START_CMDCONTROL		 = DEV_HEX_PROTOCOL_APPLEN_CONTROL; 	 //MAC地址起始下标:普通控制
	const uint8_t MACADDR_INSRT_START_CMDTIMERSET		 = DEV_HEX_PROTOCOL_APPLEN_TIMERSET;	 //MAC地址起始下标:普通定时设置
	const uint8_t MACADDR_INSRT_START_CMDDELAYSET		 = DEV_HEX_PROTOCOL_APPLEN_DELAYSET;	 //MAC地址起始下标:延时设置
	const uint8_t MACADDR_INSRT_START_CMDGREENMODESET	 = DEV_HEX_PROTOCOL_APPLEN_GREENMODESET; //MAC地址起始下标:绿色模式设置
	const uint8_t MACADDR_INSRT_START_CMDNIGHTMODESET	 = DEV_HEX_PROTOCOL_APPLEN_NIGHTMODESET; //MAC地址起始下标:夜间模式设置

	const uint8_t devMacAddr_root[DEVICE_MAC_ADDR_APPLICATION_LEN] = MWIFI_ADDR_ROOT;

	uint8_t *dataRcv_temp = (uint8_t *)dataRx;

	uint8_t dataRespond_temp[DEVMQTT_MESH_DATAREQ_TEMPLEN] = {0};
	uint16_t dataRespond_tempLen = 0;
	uint8_t devMacBuff[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};

	switch(dataRcv_temp[0]){

		/*case R2N*--------------------------------------------------------------------------*/
		case L8DEV_MESH_CMD_CONTROL:{

			stt_devDataPonitTypedef dataVal_set = {0};
			
			memcpy(&dataVal_set, &dataRcv_temp[1], sizeof(uint8_t));
			currentDev_dataPointSet(&dataVal_set, true, true, true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdControl.\n");

		}break;

		case L8DEV_MESH_CMD_DEVLOCK:{

			uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();
			
			(dataRcv_temp[1])?
				(devRunningFlg_temp |= DEV_RUNNING_FLG_BIT_DEVLOCK):
				(devRunningFlg_temp &= (~DEV_RUNNING_FLG_BIT_DEVLOCK));
			
			currentDevRunningFlg_paramSet(devRunningFlg_temp, true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdDevLock.\n");

		}break;
		
		case L8DEV_MESH_CMD_SET_TIMER:{

			usrAppActTrigTimer_paramSet((usrApp_trigTimer *)&dataRcv_temp[1], true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdTimerSet.\n");

		}break;	
		
		case L8DEV_MESH_CMD_SET_DELAY:{

			usrAppParamSet_devDelayTrig(&dataRcv_temp[1]);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdDelaySet.\n");

		}break;		
		
		case L8DEV_MESH_CMD_SET_GREENMODE:{

			usrAppParamSet_devGreenMode(&dataRcv_temp[1], true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdGreenmodeSet.\n");

		}break;

		case L8DEV_MESH_CMD_SET_NIGHTMODE:{

			usrAppNightModeTimeTab_paramSet(&dataRcv_temp[1], true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdNightmodeSet.\n");

		}break;

		case L8DEV_MESH_CMD_SET_MUTUALCTRL:{
		
			uint8_t deviceMutualNum = (uint8_t)dataRcv_temp[1];
			uint8_t loop = 0;
			uint8_t cpyist = 0;
			
			struct stt_devMutualCtrlParam{
			
				uint8_t opreat_bit;
				uint8_t mutualGroup_insert[DEVICE_MUTUAL_CTRL_GROUP_NUM];
				uint8_t devMac[MWIFI_ADDR_LEN];
				
			}mutualGroupParamRcv_temp = {0};
			stt_devMutualGroupParam mutualGroupParamSet_temp = {0},
									mutualGroupParamSet_empty = {0};
			
			uint8_t dataParamUintTemp_length = sizeof(struct stt_devMutualCtrlParam);

			for(loop = 0; loop < deviceMutualNum; loop ++){ //互控设备Maclist获取

				memset(&mutualGroupParamRcv_temp, 0, dataParamUintTemp_length);
				memcpy(&mutualGroupParamRcv_temp, 
					   &(dataRcv_temp[loop * dataParamUintTemp_length + 2]), 
					   dataParamUintTemp_length);

				for(uint8_t loopA = 0; loopA < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopA ++){

					if(mutualGroupParamRcv_temp.opreat_bit & (1 << loopA)){

						if((mutualGroupParamRcv_temp.mutualGroup_insert[loopA] != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
						   (mutualGroupParamRcv_temp.mutualGroup_insert[loopA] != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){

							mutualGroupParamSet_temp.mutualCtrlGroup_insert = mutualGroupParamRcv_temp.mutualGroup_insert[loopA]; //单次设置之内，组号非无效，必相同
							memcpy(&(mutualGroupParamSet_temp.mutualCtrlDevMacList[(cpyist ++) * MWIFI_ADDR_LEN]),
								   mutualGroupParamRcv_temp.devMac,
								   MWIFI_ADDR_LEN);

							break;
						}
					}
				}		
			}
			mutualGroupParamSet_temp.mutualCtrlDevNum = cpyist;	

			esp_wifi_get_mac(ESP_IF_WIFI_STA, devMacBuff);
			for(loop = 0; loop < deviceMutualNum; loop ++){ //互控设置参数自身存储更新 或 分发

				memset(&mutualGroupParamRcv_temp, 0, dataParamUintTemp_length);
				memcpy(&mutualGroupParamRcv_temp, 
					   &(dataRcv_temp[loop * dataParamUintTemp_length + 2]), 
					   dataParamUintTemp_length);

				if(!memcmp(devMacBuff, mutualGroupParamRcv_temp.devMac, MWIFI_ADDR_LEN)){

					for(uint8_t loopA = 0; loopA < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopA ++){

						if(mutualGroupParamRcv_temp.opreat_bit & (1 << loopA)){

							if((mutualGroupParamRcv_temp.mutualGroup_insert[loopA] != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
							   (mutualGroupParamRcv_temp.mutualGroup_insert[loopA] != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){

								devMutualCtrlGroupInfo_Set(&mutualGroupParamSet_temp, 1 << loopA, true);
							}
							else
							{
								devMutualCtrlGroupInfo_Set(&mutualGroupParamSet_empty, 1 << loopA, true);
							}
						}
					}
				}
			}
		
			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdMutualCtrlSet.\n");
		
		}break; 

		case L8DEV_MESH_CMD_EXT_PARAM_SET:{

			currentDev_extParamSet(&dataRcv_temp[1]);
			
			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdExtParamSet.\n");

		}break;

		case L8DEV_MESH_CMD_SCENARIO_SET:{

			const uint8_t *dataRcv_kernel = &dataRcv_temp[1];

			uint8_t scenarioParam_ist = dataRcv_kernel[1];
			stt_scenarioSwitchData_nvsOpreat *scenarioParamData = nvsDataOpreation_devScenarioParam_get(scenarioParam_ist);
			uint8_t dataParamHalf_flg = dataRcv_kernel[2];
			
			if(dataParamHalf_flg == 0xA1){
			
				memset(scenarioParamData, 0, sizeof(stt_scenarioSwitchData_nvsOpreat));
				scenarioParamData->dataRef.scenarioDevice_sum = dataRcv_kernel[0];
				scenarioParamData->dataRef.scenarioInsert_num = scenarioParam_ist;
				if(scenarioParamData->dataRef.scenarioDevice_sum > DEVSCENARIO_NVSDATA_HALFOPREAT_NUM)
					memcpy(scenarioParamData->dataHalf_A, &(dataRcv_kernel[9]), sizeof(stt_scenarioUnitOpreatParam) * DEVSCENARIO_NVSDATA_HALFOPREAT_NUM);
				else
					memcpy(scenarioParamData->dataHalf_A, &(dataRcv_kernel[9]), sizeof(stt_scenarioUnitOpreatParam) * scenarioParamData->dataRef.scenarioDevice_sum);
			
				devDriverBussiness_scnarioSwitch_dataParam_save(scenarioParamData);
			}
			else
			if(dataParamHalf_flg == 0xA2){
			
				uint8_t scenarioDeviceSum_reserve = scenarioParamData->dataRef.scenarioDevice_sum - DEVSCENARIO_NVSDATA_HALFOPREAT_NUM;
			
				memcpy(scenarioParamData->dataHalf_B, &(dataRcv_kernel[9]), sizeof(stt_scenarioUnitOpreatParam) * scenarioDeviceSum_reserve);
			
				devDriverBussiness_scnarioSwitch_dataParam_save(scenarioParamData);
			}
			
			printf("scenario set cmd coming! sum:%d, ist:%d, part:%02X.\n", scenarioParamData->dataRef.scenarioDevice_sum,
																		 	scenarioParamData->dataRef.scenarioInsert_num,
																		 	dataParamHalf_flg);
			
			os_free(scenarioParamData);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdScenarioSet.\n");

		}break;

		case L8DEV_MESH_CMD_SCENARIO_CTRL:{

			uint8_t scenarioOpreatVal = dataRcv_temp[1];

			currentDev_dataPointSet((stt_devDataPonitTypedef *)&scenarioOpreatVal, true, false, false);
	
			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdScenarioCtrl.\n");

		}break;

		case L8DEV_MESH_CMD_DEVINFO_GET:{

			mdf_err_t ret				= MDF_OK;
			mwifi_data_type_t data_type = {
				
				.compression = true,
				.communicate = MWIFI_COMMUNICATE_UNICAST,
			};
			mlink_httpd_type_t type_L8mesh_cst = {
			
				.format = MLINK_HTTPD_FORMAT_HEX,
			};

			esp_wifi_get_mac(ESP_IF_WIFI_STA, devMacBuff);
			memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

			switch(dataRcv_temp[1]){
				
				case L8DEV_MESH_CMD_SET_TIMER:{

					dataRespond_temp[0] = L8DEV_MESH_CMD_SET_TIMER;
					usrAppActTrigTimer_paramGet((usrApp_trigTimer *)&dataRespond_temp[1]);
					memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDTIMERSET + 1], devMacBuff, DEVICE_MAC_ADDR_APPLICATION_LEN);
					dataRespond_tempLen = MACADDR_INSRT_START_CMDTIMERSET + DEVICE_MAC_ADDR_APPLICATION_LEN + 1;

					ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdTimerGet.\n");
				
				}break;
				
				case L8DEV_MESH_CMD_SET_DELAY:{

					dataRespond_temp[0] = L8DEV_MESH_CMD_SET_DELAY;
					usrAppParamGet_devDelayTrig(&dataRespond_temp[1]);
					memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDDELAYSET + 1], devMacBuff, DEVICE_MAC_ADDR_APPLICATION_LEN);
					dataRespond_tempLen = MACADDR_INSRT_START_CMDDELAYSET + DEVICE_MAC_ADDR_APPLICATION_LEN + 1;

					ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdDelayGet.\n");
				
				}break;
				
				case L8DEV_MESH_CMD_SET_GREENMODE:{

					dataRespond_temp[0] = L8DEV_MESH_CMD_SET_GREENMODE;
					usrAppParamGet_devGreenMode(&dataRespond_temp[1]);
					memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDGREENMODESET + 1], devMacBuff, DEVICE_MAC_ADDR_APPLICATION_LEN);
					dataRespond_tempLen = MACADDR_INSRT_START_CMDGREENMODESET + DEVICE_MAC_ADDR_APPLICATION_LEN + 1;
				
					ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdGreenmodeGet.\n");
				
				}break;
				
				case L8DEV_MESH_CMD_SET_NIGHTMODE:{

					dataRespond_temp[0] = L8DEV_MESH_CMD_SET_NIGHTMODE;
					usrAppNightModeTimeTab_paramGet((usrApp_trigTimer *)&dataRespond_temp[1]);
					memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDNIGHTMODESET + 1], devMacBuff, DEVICE_MAC_ADDR_APPLICATION_LEN);
					dataRespond_tempLen = MACADDR_INSRT_START_CMDNIGHTMODESET + DEVICE_MAC_ADDR_APPLICATION_LEN + 1;

					ESP_LOGI(TAG, "<R2N>mesh data rcv, cmdNightmodeGet.\n");
				
				}break;

			}

			if(dataRespond_tempLen){

				ret = mwifi_write(devMacAddr_root, &data_type, dataRespond_temp, dataRespond_tempLen, true);
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case L8DEV_MESH_CMD_CTRLOBJ_TEXTSET:{

			const uint8_t *dataRcv_kernel = &dataRcv_temp[1];

			stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
			char countryAbbreTemp[DATAMANAGE_LANGUAGE_ABBRE_MAXLEN] = {0};
			uint8_t objNum =  dataRcv_kernel[6];
			uint8_t textDataLen = dataRcv_kernel[7];
			const uint8_t dataTextCodeIstStart = 14;
			
			usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);
			
			memcpy(countryAbbreTemp, &(dataRcv_kernel[0]), sizeof(char) * DATAMANAGE_LANGUAGE_ABBRE_MAXLEN);
			dataTextObjDisp_temp.countryFlg = countryFlgGetByAbbre(countryAbbreTemp);
			
			switch(dataTextObjDisp_temp.countryFlg){
			
				case countryT_EnglishSerail:{
			
					memset(dataTextObjDisp_temp.dataBtnTextDisp[objNum], 0, GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE);
					memcpy(dataTextObjDisp_temp.dataBtnTextDisp[objNum], &(dataRcv_kernel[dataTextCodeIstStart]), textDataLen);
			
				}break;
			
				case countryT_Arabic:
				case countryT_Hebrew:{
			
					uint8_t dataChg_temp[GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE] = {0};
					uint8_t dataTransIst_temp = 0;
			
					memset(dataTextObjDisp_temp.dataBtnTextDisp[objNum], 0, GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE);
					memcpy(dataChg_temp, &(dataRcv_kernel[dataTextCodeIstStart]), textDataLen);
					for(uint8_t loop = 0; loop < (textDataLen / 2); loop ++){ //字序倒置<utf8编码2字节长度>
			
						dataTransIst_temp = textDataLen - (2 * (loop + 1));
						memcpy(&(dataTextObjDisp_temp.dataBtnTextDisp[objNum][loop * 2]), &(dataChg_temp[dataTransIst_temp]), 2);
					}
			
				}break;
			}
			
			usrAppHomepageBtnTextDisp_paramSet(&dataTextObjDisp_temp, true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmd btnTextSet.\n");

		}break;

		case L8DEV_MESH_CMD_CTRLOBJ_ICONSET:{

			const uint8_t *dataRcv_kernel = &dataRcv_temp[1];

			usrAppHomepageBtnIconNumDisp_paramSet((uint8_t *)dataRcv_kernel, true);

			printf("dataRcv:%02x, %02x, %02x.\n", dataRcv_kernel[0],
												  dataRcv_kernel[1],
												  dataRcv_kernel[2]);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmd btnIconSet.\n");

		}break;

		case L8DEV_MESH_CMD_UISET_THEMESTYLE:{

			uint8_t uiThemeStyleFlg_set = dataRcv_temp[1];

			extern void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF);
			
			usrAppHomepageThemeType_Set(uiThemeStyleFlg_set, true);

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmd guiHome themeSet.\n");

		}break;

		case L8DEV_MESH_CMD_NEIWORK_PARAM_CHG:{

			if(esp_mesh_get_layer() != MESH_ROOT){ //非根节点响应

				mwifi_config_t ap_config = {0x0};
				struct stt_paramWifiConfig{
				
					char router_ssid[32];
					char router_password[64];
					uint8_t router_bssid[6];
					
				}param_wifiConfig = {0};

				//wifi信息更改
				memcpy(&param_wifiConfig, &dataRcv_temp[1], sizeof(struct stt_paramWifiConfig));
				mdf_info_load("ap_config", &ap_config, sizeof(mwifi_config_t));
				memcpy(ap_config.router_ssid, param_wifiConfig.router_ssid, sizeof(char) * 32);
				memcpy(ap_config.router_password, param_wifiConfig.router_password, sizeof(char) * 64);
				memcpy(ap_config.router_bssid, param_wifiConfig.router_bssid, sizeof(uint8_t) * 6);
				memcpy(ap_config.mesh_id, param_wifiConfig.router_bssid, sizeof(uint8_t) * 6);
				mdf_info_save("ap_config", &ap_config, sizeof(mwifi_config_t));

				//倒计时重启触发
				usrApplication_systemRestartTrig(5);
			}

			ESP_LOGI(TAG, "<R2N>mesh data rcv, cmd wifi change.\n");

		}break;
						
		case L8DEV_MESH_CMD_DEVRESET:{}break;	

		case L8DEV_MESH_SYSTEMTIME_BOARDCAST:{

			if(esp_mesh_get_layer() != MESH_ROOT){ //根节点不理会广播时间数据

				deviceParamSet_timeZone((stt_timeZone *)&dataRcv_temp[L8_meshDataCmdLen], true); //时区设置
				usrAppDevCurrentSystemTime_paramSet((stt_localTime *)&dataRcv_temp[sizeof(stt_timeZone) + L8_meshDataCmdLen]); //时间设置

				ESP_LOGI(TAG, "<R2N>mesh data rcv, rootdev systime boardcast data.\n");
			}

		}break;

		/*case N2N*--------------------------------------------------------------------------*/
		case L8DEV_MESH_CMD_MUTUAL_CTRL:{

			/*
				byte1:互控组号
				byte2:互控数据
			*/

			uint8_t mutualCtrlGroup_insert = dataRcv_temp[1]; //互控组号
			uint8_t mutualCtrlGroupParam_data = dataRcv_temp[2];
			stt_devMutualGroupParam mutualCtrlGroupParam_temp = {0};
			uint8_t mutualCtrlGroupNum = 0; //互控数据数组索引
			bool mutualCtrlGroupCheck_res = false;

			mutualCtrlGroupCheck_res = devMutualCtrlGroupInfo_unitCheckByInsert(&mutualCtrlGroupParam_temp, &mutualCtrlGroupNum, mutualCtrlGroup_insert);

			if(mutualCtrlGroupCheck_res){

				devTypeDef_enum currentDevType = currentDev_typeGet();

				switch(currentDevType){
				
					case devTypeDef_mulitSwOneBit:
					case devTypeDef_mulitSwTwoBit:
					case devTypeDef_mulitSwThreeBit:{

						bool mutualCtrl_reserveIf = false;

						switch(mutualCtrlGroupNum){

							case 0:

								if(mutualCtrlTrigIf_A)
									mutualCtrl_reserveIf = true;

							break;
							
							case 1:

								if(mutualCtrlTrigIf_B)
									mutualCtrl_reserveIf = true;

							break;
							
							case 2:

								if(mutualCtrlTrigIf_C)
									mutualCtrl_reserveIf = true;

							break;

							default:break;
						}

						if(mutualCtrl_reserveIf){

							uint8_t devDataPoint_hex = 0;
							stt_devDataPonitTypedef devDataPoint = {0};

							currentDev_dataPointGet(&devDataPoint);
							memcpy(&devDataPoint_hex, &devDataPoint, sizeof(uint8_t));
							
							if(mutualCtrlGroupParam_data){
							
								devDataPoint_hex |= (1 << mutualCtrlGroupNum);
							}
							else
							{
								devDataPoint_hex &= ~(1 << mutualCtrlGroupNum);
							}

							currentDev_dataPointSet((stt_devDataPonitTypedef *)&devDataPoint_hex, true, false, true);
						}
						
					}break;

					case devTypeDef_curtain:
					case devTypeDef_dimmer:{

						if(mutualCtrlTrigIf_A)
							currentDev_dataPointSet((stt_devDataPonitTypedef *)&mutualCtrlGroupParam_data, true, false, true);

					}break;
					
					case devTypeDef_fans:{}break;
					case devTypeDef_scenario:{}break;
					case devTypeDef_heater:{}break;
				
					default:break;
				}
			}

			ESP_LOGI(TAG, "<R2N>mesh data rcv, mutualCtrl Trig.\n");
			
		}break;

		/*case N2R*--------------------------------------------------------------------------*/
		case L8DEV_MESH_HEARTBEAT_REQ:{

			if(esp_mesh_get_layer() == MESH_ROOT){ //根节点根据子节点上报信息进行链表管理

				stt_hbDataUpload devNode_hbDataTemp = {0};
				stt_nodeDev_hbDataManage *devNode = NULL;
				stt_nodeDev_hbDataManage devNode_temp = {0};

				memcpy(&devNode_hbDataTemp, (stt_hbDataUpload *)&dataRcv_temp[L8_meshDataCmdLen], sizeof(stt_hbDataUpload)); //数据加载
				devNode = L8devHbDataManageList_nodeGet(listHead_nodeDevDataManage, devNode_hbDataTemp.nodeDev_Mac, true); //链表节点获取
				if(NULL == devNode){

					printf("newNodeMac:"MACSTR"-creat.\n", MAC2STR(devNode_hbDataTemp.nodeDev_Mac)); 

					memcpy(&(devNode_temp.dataManage), &devNode_hbDataTemp, sizeof(stt_hbDataUpload));
					devNode_temp.nodeDevKeepAlive_counter = L8_NODEDEV_KEEPALIVE_PERIOD;
				
					L8devHbDataManageList_nodeCreat(listHead_nodeDevDataManage, &devNode_temp);	
					
				}
				else
				{
					printf("nodeMac:"MACSTR"-keepalive reales.\n", MAC2STR(devNode->dataManage.nodeDev_Mac));
					memcpy(&(devNode->dataManage), &devNode_hbDataTemp, sizeof(stt_hbDataUpload)); //节点状态信息更新
					devNode->nodeDevKeepAlive_counter = L8_NODEDEV_KEEPALIVE_PERIOD; //生命周期刷新
				}
			}

			ESP_LOGI(TAG, "<N2R>mesh data rcv, heartbeat data.\n");
		
		}break;
		
		default:break;
	}
}

void devHeartbeat_dataTrans_bussinessTrig(void){

	uint8_t dataRequest_temp[32] = {0}; //缓存大小要给够，不然数据传输会出问题
	mdf_err_t ret = MDF_OK;
	mwifi_data_type_t data_type = {
		
		.compression = true,
		.communicate = MWIFI_COMMUNICATE_UNICAST,
	};
	const mlink_httpd_type_t type_L8mesh_cst = {
	
		.format = MLINK_HTTPD_FORMAT_HEX,
	};
	
	memcpy(&(data_type.custom), &type_L8mesh_cst, sizeof(uint32_t));

	if(mwifi_is_connected() && esp_mesh_get_layer() != MESH_ROOT){ //是子节点则发心跳

		stt_hbDataUpload nodeDev_hbDataTemp = {0};
		const uint8_t meshRootAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_ROOT;

		L8devHeartbeatFunctionParamLoad(&nodeDev_hbDataTemp);
		
		dataRequest_temp[0] = L8DEV_MESH_HEARTBEAT_REQ;
		memcpy(&dataRequest_temp[L8_meshDataCmdLen], &nodeDev_hbDataTemp, sizeof(stt_hbDataUpload));

		ret = mwifi_write(meshRootAddr, &data_type, dataRequest_temp, sizeof(stt_hbDataUpload) + 1, true);
		MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> heartbeat mwifi_root_write", mdf_err_to_name(ret));

		printf("n2r hb trig.\n");
	}
	else
	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT){ //是根节点则广播时间

		const uint8_t boardcastAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;

		deviceParamGet_timeZone((stt_timeZone *)&dataRequest_temp[L8_meshDataCmdLen]); //设备时区填装
		usrAppDevCurrentSystemTime_paramGet((stt_localTime *)&dataRequest_temp[sizeof(stt_timeZone) + L8_meshDataCmdLen]); //设备系统时间填装

		dataRequest_temp[0] = L8DEV_MESH_SYSTEMTIME_BOARDCAST;

		ret = mwifi_root_write(boardcastAddr, 1, &data_type, dataRequest_temp, sizeof(stt_timeZone) + sizeof(stt_localTime) + L8_meshDataCmdLen, true);
        MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mwifi_root_write", mdf_err_to_name(ret));

		printf("r2n systime boradcast trig.\n");
	}
}





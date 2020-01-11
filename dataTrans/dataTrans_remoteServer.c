#include "dataTrans_remoteServer.h"

#include "os.h"

#include "mlink.h"
#include "mwifi.h"

#include "mdf_common.h"
#include "mqtt_client.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

#include "dataTrans_localHandler.h"
#include "dataTrans_meshUpgrade.h"

#include "devDriver_manage.h"

#define DEVMQTT_TOPIC_HEAD_A			"lanbon/a/"
#define DEVMQTT_TOPIC_HEAD_B			"lanbon/b/"

#define DEVMQTT_TOPIC_NUM_M2S		30
#define DEVMQTT_TOPIC_NUM_S2M		20
#define DEVMQTT_TOPIC_TEMP_LENGTH	100
#define DEVMQTT_TOPIC_CASE_LENGTH	50
#define DEVMQTT_DATA_RESPOND_LENGTH	100

extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;
extern uint8_t listNum_nodeDevDataManage;

extern EventGroupHandle_t xEventGp_devApplication;

extern void lvGui_wifiConfig_bussiness_configComplete_tipsTrig(void);

uint16_t dtCounter_preventSurge = 0;

static char mqttCfgParam_hostIpStr[32] = {0};

static const char *TAG = "lanbon_L8 - mqttRemote";

static const char cmdTopic_m2s_list[DEVMQTT_TOPIC_NUM_M2S][DEVMQTT_TOPIC_CASE_LENGTH] = {

	"/cmdControl", 
	"/cmdDevLock",
	"/cmdBkPicSet",
	"/cmdBkPicIstSet",
	"/cmdBtnTextPicSet",
	"/cmdBtnTextDispSet",
	"/cmdBtnIconDispSet",
	"/cmdTimerSet/normal", 
	"/cmdTimerSet/delay",
	"/cmdTimerSet/greenMode",
	"/cmdTimerSet/nightMode",

	"/cmdExtParamSet",
	
	"/cmdMutualSet",
	"/cmdScenario/opCtrl",
	"/cmdScenario/opSet",

	"/cmdDevLock/multiple", 
	"/cmdUiStyle/multiple",

	"/cmdSysParamCfg/overall",

	"/cmdWifiChg/overall",

	"/m2s/cmdQuery",

	"/cmdMeshUpgradeForce",
	"/cmdMeshUpgradeNotice",

	"/L8devLoginNotice_cfm",
};
enum{

	cmdTopicM2SInsert_cmdControl = 0,
	cmdTopicM2SInsert_cmdDevLock,
	cmdTopicM2SInsert_cmdBGroundPicSet,
	cmdTopicM2SInsert_cmdBGroundPicIstSet,
	cmdTopicM2SInsert_cmdBtnTextPicSet,
	cmdTopicM2SInsert_cmdBtnTextDispSet,
	cmdTopicM2SInsert_cmdBtnIconDispSet,
	cmdTopicM2SInsert_cmdTimerSet_normal,
	cmdTopicM2SInsert_cmdTimerSet_delay,
	cmdTopicM2SInsert_cmdTimerSet_greenMode,
	cmdTopicM2SInsert_cmdTimerSet_nightMode,

	cmdTopicM2SInsert_cmdExtParamSet,
	
	cmdTopicM2SInsert_cmdMutualSet,
	cmdTopicM2SInsert_cmdScenario_opCtrl,
	cmdTopicM2SInsert_cmdScenario_opSet,
	
	cmdTopicM2SInsert_cmdDevLock_multiple,
	cmdTopicM2SInsert_cmdUiStyle_multiple,

	cmdTopicM2SInsert_cmdSysParamCfg_overall,

	cmdTopicM2SInsert_cmdWifiChg_overall,

	cmdTopicM2SInsert_cmdQuery,

	cmdTopicM2SInsert_cmdMeshUpgradeForce,
	cmdTopicM2SInsert_cmdMeshUpgradeNotice,

	cmdTopicM2SInsert_L8devLoginNotice_cfm,
};

static const char cmdTopic_s2m_list[DEVMQTT_TOPIC_NUM_S2M][DEVMQTT_TOPIC_CASE_LENGTH] = {

	"/s2m/cmdStatusGet",
	"/s2m/cmdStatusGet/specified",
	"/s2m/cmdTimerGet/normal", 
	"/s2m/cmdTimerGet/delay",
	"/s2m/cmdTimerGet/greenMode",
	"/s2m/cmdTimerGet/nightMode",
	"/s2m/cmdMutualGet",
	"/s2m/cmdStatusSynchronous",

	"/cmdMeshUpgradeCheck",
};
enum{

	cmdTopicS2MInsert_cmdStatusGet = 0,
	cmdTopicS2MInsert_cmdStatusGet_specified,
	cmdTopicS2MInsert_cmdTimerGet_normal,
	cmdTopicS2MInsert_cmdTimerGet_delay,
	cmdTopicS2MInsert_cmdTimerGet_greenMode,
	cmdTopicS2MInsert_cmdTimerGet_nightMode,
	cmdTopicS2MInsert_cmdMutualGet,
	cmdTopicS2MInsert_cmdStatusSynchro,

	cmdTopicS2MInsert_cmdMeshUpgradeCheck,
};

static const char mqttTopicSpecial_elecsumReport[DEVMQTT_TOPIC_CASE_LENGTH] = "L8devElecsumReport";

static uint8_t	  counterDownRecord_loginConnectNotice = 0;
static const char mqttTopicSpecial_loginConnectNotice_req[DEVMQTT_TOPIC_CASE_LENGTH] = "L8devLoginNotice_req";

static char devMqtt_topicTemp[DEVMQTT_TOPIC_TEMP_LENGTH] = {0};

static uint8_t dataRespond_temp[DEVMQTT_DATA_RESPOND_LENGTH] = {0};

static esp_mqtt_client_handle_t usrAppClient = NULL;
static bool mqttClientElecsumRepot_reserveFlg = false;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

static esp_mqtt_client_config_t mqtt_cfg = {

	.transport = MQTT_TRANSPORT_OVER_TCP,
//	.host = "112.124.61.191",
	.event_handle = mqtt_event_handler,
//	.port = 8888,
	.username = "lanbon",
	.password = "lanbon2019.",

    .task_prio 	  = (CONFIG_MDF_TASK_DEFAULT_PRIOTY + 1),                
    .task_stack	  = (1024 * 10),                        
    .buffer_size  = (512 * 3),                     
};	

static bool remoteMqtt_connectFlg = false;

static uint8_t topicM2S_compareResult(char *strTarget, uint8_t strLen){

	uint8_t loopSearch = 0;
	uint8_t devRouterBssid[6] = {0};

	devRouterConnectBssid_Get(devRouterBssid);

	for(loopSearch = 0; loopSearch < DEVMQTT_TOPIC_NUM_M2S; loopSearch ++){

		memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
		sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																	 				   (char *)cmdTopic_m2s_list[loopSearch]);
//		if(loopSearch == 10){ //debug

//			printf("A:%s\n", devMqtt_topicTemp);
//			printf("B:%s\n", (char *)cmdTopic_m2s_list[loopSearch]);
//		}
		if(!memcmp(strTarget, devMqtt_topicTemp, strLen))break;

		memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
		sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_B"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																	 				   (char *)cmdTopic_m2s_list[loopSearch]);	
		if(!memcmp(strTarget, devMqtt_topicTemp, strLen))break;
	}

	if(loopSearch >= DEVMQTT_TOPIC_NUM_M2S){

		printf("topic search compare res:fail.\n");
//		printf("failed strTarget:'%s'.\n", strTarget);

	}else{

		printf("topic search compare res:%d.\n", loopSearch);
	}

	return loopSearch;
}

static void mqtt_remoteDataHandler(esp_mqtt_event_handle_t event, uint8_t cmdTopicM2S_num){

	const uint8_t MACADDR_INSRT_START_CMDCONTROL 		= DEV_HEX_PROTOCOL_APPLEN_CONTROL;		//MAC地址起始下标:普通控制
	const uint8_t MACADDR_INSRT_START_CMDDEVLOCK 		= DEV_HEX_PROTOCOL_APPLEN_DEVLOCK;		//MAC地址起始下标:设备锁定操作
	const uint8_t MACADDR_INSRT_START_CMDTIMERSET 		= DEV_HEX_PROTOCOL_APPLEN_TIMERSET; 	//MAC地址起始下标:普通定时设置
	const uint8_t MACADDR_INSRT_START_CMDDELAYSET 		= DEV_HEX_PROTOCOL_APPLEN_DELAYSET; 	//MAC地址起始下标:延时设置
	const uint8_t MACADDR_INSRT_START_CMDGREENMODESET 	= DEV_HEX_PROTOCOL_APPLEN_GREENMODESET; //MAC地址起始下标:绿色模式设置
	const uint8_t MACADDR_INSRT_START_CMDNIGHTMODESET 	= DEV_HEX_PROTOCOL_APPLEN_NIGHTMODESET; //MAC地址起始下标:夜间模式设置
	const uint8_t MACADDR_INSRT_START_CMDEXTPARAMSET	= DEV_HEX_PROTOCOL_APPLEN_EXTPARAMSET;  //MAC地址起始下标:额外其它参数设置

	const uint8_t MACADDR_INSRT_START_CMDQUERY = 1; //MAC地址起始下标

	mdf_err_t ret               = MDF_OK;
    mwifi_data_type_t data_type = {
		
        .compression = true,
        .communicate = MWIFI_COMMUNICATE_UNICAST,
    };
	mlink_httpd_type_t type_L8mesh_cst = {

		.format = MLINK_HTTPD_FORMAT_HEX,
	};

	bool data_sendToRoot_IF = false;

	uint8_t devRouterBssid[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};
	uint8_t devSelfMac[MWIFI_ADDR_LEN] = {0};

	if(dtCounter_preventSurge){

		dtCounter_preventSurge = USRDEF_MQTT_DTSURGE_PREVENT_KPTIME;
		printf("dt surgeTime still reserve.\n");
		
		return;
	}

	devRouterConnectBssid_Get(devRouterBssid);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);
	memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

	memset(dataRespond_temp, 0, DEVMQTT_DATA_RESPOND_LENGTH); //数据缓存清零
	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零

	devBeepTips_trig(3, 10, 150, 0, 1); //beeps

	switch(cmdTopicM2S_num){

		case cmdTopicM2SInsert_cmdControl:{

			const uint16_t dataComming_lengthLimit = 7; //数据长度最短限制

			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDCONTROL]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				stt_devDataPonitTypedef dataVal_set = {0};
				
				memcpy(&dataVal_set, (uint8_t *)(&event->data[0]), sizeof(uint8_t));
				currentDev_dataPointSet(&dataVal_set, true, true, true, false);
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_CONTROL;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDCONTROL); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDCONTROL], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDCONTROL + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdDevLock:{
			
			const uint16_t dataComming_lengthLimit = 7; //数据长度最短限制

			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDDEVLOCK]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

				(event->data[0])?
					(devRunningFlg_temp |= DEV_RUNNING_FLG_BIT_DEVLOCK):
					(devRunningFlg_temp &= (~DEV_RUNNING_FLG_BIT_DEVLOCK));

				currentDevRunningFlg_paramSet(devRunningFlg_temp, true);
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_DEVLOCK;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDDEVLOCK); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDDEVLOCK], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDDEVLOCK + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdBGroundPicSet:{

			const uint16_t dataComming_lengthLimit = 8; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[2]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				uint8_t  dataTransStatus = event->data[0] & 0x0f;
				uint8_t  dataPicIst = (event->data[0] >> 4) & 0x0f;
				uint8_t  *dataPic_ptr = NULL;
				uint16_t dataBagIst = event->data[1];
				uint8_t  *dataReales_ptr = (uint8_t *)&(event->data[8]);
				uint16_t dataReales_len = (uint16_t)(event->data_len) - 8;
				bool 	 dataBag_lastFrame_If = false;

				((dataTransStatus == 0x0B) || (dataTransStatus == 0x0C))?
					(dataBagIst += 200):
					(dataBagIst += 0);

				(dataTransStatus == 0x0C)?
					(dataBag_lastFrame_If = true):
					(dataBag_lastFrame_If = false);

//				printf("dataTransStatus:%d, dataBagIst:%d, dLen:%d\n", dataTransStatus, 
//																	   dataBagIst,
//																	   dataReales_len);
				
				dataPic_ptr = dataPtr_bGroundPic;
				
				gui_bussinessHome_bGroundPic_dataReales(dataPic_ptr, 
														dataReales_ptr, 
													 	dataReales_len, 
													 	dataBagIst, 
													 	dataBag_lastFrame_If);
			}
			else
			{

			}

		}break;

		case cmdTopicM2SInsert_cmdBGroundPicIstSet:{

			const uint16_t dataComming_lengthLimit = 7; //数据长度最短限制
			const uint8_t targetMacIstStart = 1; //数据包接收目标MAC地址起始索引
			
			if(event->data_len < dataComming_lengthLimit)break;
			
			(!memcmp(devSelfMac, &(event->data[targetMacIstStart]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				extern void usrAppHomepageBgroundPicOrg_Set(const uint8_t picIst, bool nvsRecord_IF, bool refresh_IF);
				usrAppHomepageBgroundPicOrg_Set((const uint8_t)event->data[0], true, true);
			}
			else
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_BGROUND_ISTSET;
				memcpy(&dataRespond_temp[1], &event->data[0], event->data_len); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[targetMacIstStart], 1,
									   &data_type, dataRespond_temp, event->data_len + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}
		
		}break;

//		case cmdTopicM2SInsert_cmdBtnTextPicSet:{

//			const uint16_t dataComming_lengthLimit = 8; //数据长度最短限制
//			
//			if(event->data_len < dataComming_lengthLimit)break;

//			(!memcmp(devSelfMac, &(event->data[2]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
//			if(data_sendToRoot_IF){

//				uint8_t dataTransStatus = event->data[0] & 0x0f;
//				uint8_t dataPicIst = (event->data[0] >> 4) & 0x0f;
//				uint8_t *dataPic_ptr = NULL;
//				uint8_t dataBagIst = event->data[1];
//				uint8_t *dataReales_ptr = (uint8_t *)&(event->data[8]);
//				uint16_t dataReales_len = (uint16_t)(event->data_len) - 8;
//				bool dataBag_lastFrame_If = false;

//				(dataTransStatus == 0x0A)?
//					(dataBag_lastFrame_If = false):
//					(dataBag_lastFrame_If = true);

//				printf("dataTransStatus:%d, dataBagIst:%d, dLen:%d\n", dataTransStatus, 
//																	   dataBagIst,
//																	   dataReales_len);
//				switch(dataPicIst){

//					case 0: dataPic_ptr = dataPtr_btnTextImg_sw_A; break;
//					case 1: dataPic_ptr = dataPtr_btnTextImg_sw_B; break;
//					case 2: dataPic_ptr = dataPtr_btnTextImg_sw_C; break;
//					default:break;
//				}
//				gui_bussinessHome_btnText_dataReales(dataPicIst, 
//													 dataPic_ptr, 
//													 dataReales_ptr, 
//													 dataReales_len, 
//													 dataBagIst, 
//													 dataBag_lastFrame_If);
//			}
//			else
//			{

//			}

//		}break;

		case cmdTopicM2SInsert_cmdBtnTextDispSet:{

			const uint16_t dataComming_lengthLimit = 14; //数据长度最短限制
			const uint8_t targetMacIstStart = 8; //数据包接收目标MAC地址起始索引
			
			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[targetMacIstStart]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				stt_dataDisp_guiBussinessHome_btnText dataTextObjDisp_temp = {0};
				char countryAbbreTemp[DATAMANAGE_LANGUAGE_ABBRE_MAXLEN] = {0};
				uint8_t objNum =  event->data[6];
				uint8_t textDataLen = event->data[7];
				const uint8_t dataTextCodeIstStart = 14;
			
				usrAppHomepageBtnTextDisp_paramGet(&dataTextObjDisp_temp);

				memcpy(countryAbbreTemp, &(event->data[0]), sizeof(char) * DATAMANAGE_LANGUAGE_ABBRE_MAXLEN);
				dataTextObjDisp_temp.countryFlg = countryFlgGetByAbbre(countryAbbreTemp);

				switch(dataTextObjDisp_temp.countryFlg){

					case countryT_Arabic:
					case countryT_Hebrew:{

						uint8_t dataChg_temp[GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE] = {0};
						uint8_t dataTransIst_temp = 0;

						memset(dataTextObjDisp_temp.dataBtnTextDisp[objNum], 0, GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE);
						memcpy(dataChg_temp, &(event->data[dataTextCodeIstStart]), textDataLen);
						for(uint8_t loop = 0; loop < (textDataLen / 2); loop ++){ //字序倒置<utf8编码2字节长度>

							dataTransIst_temp = textDataLen - (2 * (loop + 1));
							memcpy(&(dataTextObjDisp_temp.dataBtnTextDisp[objNum][loop * 2]), &(dataChg_temp[dataTransIst_temp]), 2);
						}

					}break;

					case countryT_EnglishSerail:
					default:{

						memset(dataTextObjDisp_temp.dataBtnTextDisp[objNum], 0, GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE);
						memcpy(dataTextObjDisp_temp.dataBtnTextDisp[objNum], &(event->data[dataTextCodeIstStart]), textDataLen);

					}break;
				}

				usrAppHomepageBtnTextDisp_paramSet(&dataTextObjDisp_temp, true);
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_CTRLOBJ_TEXTSET;
				memcpy(&dataRespond_temp[1], &event->data[0], event->data_len); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[targetMacIstStart], 1,
									   &data_type, dataRespond_temp, event->data_len + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}
			
		}break;

		case cmdTopicM2SInsert_cmdBtnIconDispSet:{

			const uint16_t dataComming_lengthLimit = 8; //数据长度最短限制
			const uint8_t targetMacIstStart = 3; //数据包接收目标MAC地址起始索引
		
			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[targetMacIstStart]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				usrAppHomepageBtnIconNumDisp_paramSet((uint8_t *)&event->data[0], true);
			}
			else
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_CTRLOBJ_ICONSET;
				memcpy(&dataRespond_temp[1], &event->data[0], event->data_len); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[targetMacIstStart], 1,
									   &data_type, dataRespond_temp, event->data_len + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdTimerSet_normal:{

			const uint16_t dataComming_lengthLimit = 30; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;

//			printf("timer0 setParam:[%02X %02X %02X].\n", event->data[0],
//														  event->data[1],
//														  event->data[2]);
			
			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDTIMERSET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				if(event->data_len >= (sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM)){

					usrAppActTrigTimer_paramSet((usrApp_trigTimer *)(event->data), true);
					
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) //红外定时数据另外存储

					const uint8_t dataIst = 30;

					if(event->data_len >= (dataComming_lengthLimit + 8)){

						devDriverBussiness_infraredSwitch_timerUpTrigIstTabSet((uint8_t *)(&event->data[dataIst]), true);

						printf("ir timerUpIstTab get:%d, %d, %d, %d, %d, %d, %d, %d.\n", (int)event->data[dataIst + 0],
																						 (int)event->data[dataIst + 1],
																						 (int)event->data[dataIst + 2],
																						 (int)event->data[dataIst + 3],
																						 (int)event->data[dataIst + 4],
																						 (int)event->data[dataIst + 5],
																						 (int)event->data[dataIst + 6],
																						 (int)event->data[dataIst + 7]);
					}
#endif
				}
			}
			else //数据不是给主机，则转发
			{
				uint8_t dataForward_loadIst = 0;
				uint8_t dataForward_len = 0;

				if(event->data_len > dataComming_lengthLimit){

					dataRespond_temp[dataForward_loadIst] = L8DEV_MESH_CMD_SET_TIMER;
					dataForward_loadIst += 1;
					memcpy(&dataRespond_temp[dataForward_loadIst], &event->data[0], MACADDR_INSRT_START_CMDTIMERSET); //数据内容填充
					dataForward_loadIst += MACADDR_INSRT_START_CMDTIMERSET;
					memcpy(&dataRespond_temp[dataForward_loadIst], &event->data[dataComming_lengthLimit], USRAPP_VALDEFINE_TRIGTIMER_NUM);
					dataForward_len = MACADDR_INSRT_START_CMDTIMERSET + USRAPP_VALDEFINE_TRIGTIMER_NUM + 1;
				}
				else
				{
					dataRespond_temp[dataForward_loadIst] = L8DEV_MESH_CMD_SET_TIMER;
					dataForward_loadIst += 1;
					memcpy(&dataRespond_temp[dataForward_loadIst], &event->data[0], MACADDR_INSRT_START_CMDTIMERSET); //数据内容填充
					dataForward_len = MACADDR_INSRT_START_CMDTIMERSET + 1;
				}

				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDTIMERSET], 1,
									   &data_type, dataRespond_temp, dataForward_len, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdTimerSet_delay:{

			const uint16_t dataComming_lengthLimit = 9; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;

//			printf("delayerSet param:[%02X][%02X][%02X]",
//					event->data[0],
//					event->data[1],
//					event->data[2]);

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDDELAYSET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){
			
				if(event->data_len >= (sizeof(uint16_t) + 1)){
				
					usrAppParamSet_devDelayTrig((uint8_t *)(event->data));
				}			
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_SET_DELAY;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDDELAYSET); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDDELAYSET], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDDELAYSET + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdTimerSet_greenMode:{

			const uint16_t dataComming_lengthLimit = 8; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;
			
			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDGREENMODESET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){
			
				if(event->data_len >= sizeof(uint16_t)){
				
					usrAppParamSet_devGreenMode((uint8_t *)(event->data), true);
				}
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_SET_GREENMODE;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDGREENMODESET); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDGREENMODESET], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDGREENMODESET + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdTimerSet_nightMode:{

			const uint16_t dataComming_lengthLimit = 11; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDNIGHTMODESET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){
			
				if(event->data_len >= (sizeof(usrApp_trigTimer) * 2)){

					uint8_t dataTemp[6] = {0};

					//数据处理调换
					dataTemp[0] = (uint8_t)event->data[0]; //全天
					dataTemp[1] = (uint8_t)event->data[2]; //时段1：时
					dataTemp[2] = (uint8_t)event->data[3]; //时段1：分
					dataTemp[3] = (uint8_t)event->data[1]; //非全天
					dataTemp[4] = (uint8_t)event->data[4]; //时段2：时
					dataTemp[5] = (uint8_t)event->data[5]; //时段2：分
					usrAppNightModeTimeTab_paramSet((usrApp_trigTimer *)(dataTemp), true);

//					ESP_LOGI(TAG, "nmData:%02X %02X %02X %02X %02X %02X.\n", dataTemp[0],
//																			 dataTemp[1],
//																			 dataTemp[2],
//																			 dataTemp[3],
//																			 dataTemp[4],
//																			 dataTemp[5]);
				}			
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_SET_NIGHTMODE;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDNIGHTMODESET); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDNIGHTMODESET], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDNIGHTMODESET + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdExtParamSet:{

			const uint16_t dataComming_lengthLimit = 10; //数据长度最短限制

			if(event->data_len < dataComming_lengthLimit)break;
			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDEXTPARAMSET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				currentDev_extParamSet(event->data);
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_EXT_PARAM_SET;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDEXTPARAMSET); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDEXTPARAMSET], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDEXTPARAMSET + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdMutualSet:{

			uint8_t deviceMutualNum = (uint8_t)event->data[0];
			uint8_t loop = 0;
			uint8_t cpyist = 0;

			const uint16_t dataComming_lengthLimit = 11;

			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			struct stt_devMutualCtrlParam{

				uint8_t opreat_bit;
				uint8_t mutualGroup_insert[DEVICE_MUTUAL_CTRL_GROUP_NUM];
				uint8_t devMac[MWIFI_ADDR_LEN];
				
			}mutualGroupParamRcv_temp = {0};
			stt_devMutualGroupParam mutualGroupParamSet_temp = {0},
									mutualGroupParamSet_empty = {0};

			uint8_t dataParamUintTemp_length = sizeof(struct stt_devMutualCtrlParam);

			if(event->data_len > (dataParamUintTemp_length * MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP)){

				MDF_LOGW("mqtt topic cmdMutualSet dataLen too long.\n");
				break;
			
			}
			else
			{
				
			}

			for(loop = 0; loop < deviceMutualNum; loop ++){ //互控设备Maclist获取

				memset(&mutualGroupParamRcv_temp, 0, dataParamUintTemp_length);
				memcpy(&mutualGroupParamRcv_temp, 
					   &(event->data[loop * dataParamUintTemp_length + 1]), 
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

			for(loop = 0; loop < deviceMutualNum; loop ++){ //互控设置参数自身存储更新 或 分发

				memset(&mutualGroupParamRcv_temp, 0, dataParamUintTemp_length);
				memcpy(&mutualGroupParamRcv_temp, 
					   &(event->data[loop * dataParamUintTemp_length + 1]), 
					   dataParamUintTemp_length);

				if(!memcmp(devSelfMac, mutualGroupParamRcv_temp.devMac, MWIFI_ADDR_LEN)){

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
				else
				{
					if(mutualGroupParamRcv_temp.opreat_bit){

						dataRespond_temp[0] = L8DEV_MESH_CMD_SET_MUTUALCTRL;
						memcpy(&dataRespond_temp[1], event->data, event->data_len); //数据内容填充

						ret = mwifi_root_write((const uint8_t *)mutualGroupParamRcv_temp.devMac, 1,
											   &data_type, dataRespond_temp, event->data_len + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
						MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
					}
				}
			}

		}break;

		case cmdTopicM2SInsert_cmdScenario_opCtrl:{

			uint8_t loop = 0;
			uint8_t scenarioUnit_num = event->data[0];
			struct stt_scenarioActionParam{

				uint8_t devMacAddr[MWIFI_ADDR_LEN];
				uint8_t opreatVal;
				
			}scenarioActionParam_unit = {0};
			uint8_t dataParamUintTemp_length = sizeof(struct stt_scenarioActionParam);

			const uint16_t dataComming_lengthLimit = 8;

			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			for(loop = 0; loop < scenarioUnit_num; loop ++){

				os_memset(&scenarioActionParam_unit, 0, dataParamUintTemp_length);
				os_memcpy(&scenarioActionParam_unit,
						  &(event->data[loop * dataParamUintTemp_length + 1]), 
						  dataParamUintTemp_length);

				if(!memcmp(devSelfMac, scenarioActionParam_unit.devMacAddr, MWIFI_ADDR_LEN)){

					stt_devDataPonitTypedef dataPointScenCtrl_temp = {0};
					
					memcpy(&dataPointScenCtrl_temp, &scenarioActionParam_unit.opreatVal, sizeof(stt_devDataPonitTypedef));
					
					switch(currentDev_typeGet()){
					
						case devTypeDef_thermostat:{
					
							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};
					
							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变
							dataPointScenCtrl_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							dataPointScenCtrl_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;
					
						}break;
					
						case devTypeDef_thermostatExtension:{
					
							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};
							uint8_t devThermostatExSwStatus_temp = 0;
					
							memcpy(&devThermostatExSwStatus_temp, &dataPointScenCtrl_temp, sizeof(uint8_t));
							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变
					
							/*bit0 -恒温器是否开启*/
							(devThermostatExSwStatus_temp & (1 >> 0))?
								(dataPointScenCtrl_temp.devType_thermostat.devThermostat_running_en = 1):
								(dataPointScenCtrl_temp.devType_thermostat.devThermostat_running_en = 0);
							dataPointScenCtrl_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							dataPointScenCtrl_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;
					
							/*bit2 -第二位开关值
							  bit1 -第一位开关值*/
							devThermostatExSwStatus_temp >>= 1;
							devThermostatExSwStatus_temp &= 0x03;
							devDriverBussiness_thermostatSwitch_exSwitchParamSet(devThermostatExSwStatus_temp);
					
						}break;
					
						default:break;
					}

					currentDev_dataPointSet(&dataPointScenCtrl_temp, true, false, false, false); //场景操作不触发互控
				}
				else
				{
					dataRespond_temp[0] = L8DEV_MESH_CMD_SCENARIO_CTRL; //mesh命令
					memcpy(&dataRespond_temp[1], &(scenarioActionParam_unit.opreatVal), 1); //数据
				
					ret = mwifi_root_write((const uint8_t *)scenarioActionParam_unit.devMacAddr, 1,
										 &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
					MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));					
				}
			}

		}break;

		case cmdTopicM2SInsert_cmdScenario_opSet:{

			const uint8_t targetMacIstStart = 3; //数据包接收目标MAC地址起始索引
			const uint16_t dataComming_lengthLimit = 9;
		
			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			if(!memcmp(devSelfMac, &(event->data[targetMacIstStart]), DEVICE_MAC_ADDR_APPLICATION_LEN)){

				uint8_t scenarioParam_ist = event->data[1];
				stt_scenarioSwitchData_nvsOpreat *scenarioParamData = nvsDataOpreation_devScenarioParam_get(scenarioParam_ist);
				uint8_t dataParamHalf_flg = event->data[2];

				if(dataParamHalf_flg == 0xA1){

					memset(scenarioParamData, 0, sizeof(stt_scenarioSwitchData_nvsOpreat));
					scenarioParamData->dataRef.scenarioDevice_sum = event->data[0];
					scenarioParamData->dataRef.scenarioInsert_num = scenarioParam_ist;
					if(scenarioParamData->dataRef.scenarioDevice_sum > DEVSCENARIO_NVSDATA_HALFOPREAT_NUM)
						memcpy(scenarioParamData->dataHalf_A, &(event->data[9]), sizeof(stt_scenarioUnitOpreatParam) * DEVSCENARIO_NVSDATA_HALFOPREAT_NUM);
					else
						memcpy(scenarioParamData->dataHalf_A, &(event->data[9]), sizeof(stt_scenarioUnitOpreatParam) * scenarioParamData->dataRef.scenarioDevice_sum);

					devDriverBussiness_scnarioSwitch_dataParam_save(scenarioParamData);
				}
				else
				if(dataParamHalf_flg == 0xA2){

					uint8_t scenarioDeviceSum_reserve = scenarioParamData->dataRef.scenarioDevice_sum - DEVSCENARIO_NVSDATA_HALFOPREAT_NUM;

					memcpy(scenarioParamData->dataHalf_B, &(event->data[9]), sizeof(stt_scenarioUnitOpreatParam) * scenarioDeviceSum_reserve);

					devDriverBussiness_scnarioSwitch_dataParam_save(scenarioParamData);
				}

				printf("scenario set cmd coming! sum:%d, ist:%d, part:%02X.\n", scenarioParamData->dataRef.scenarioDevice_sum,
																  			 	scenarioParamData->dataRef.scenarioInsert_num,
																            	dataParamHalf_flg);

				os_free(scenarioParamData);
			}
			else
			{
				char *dataMeshReq_bufTemp = (char *)os_zalloc(sizeof(char) * (event->data_len + 1)); //额定stack缓存不足，重新申请heap缓存
				dataMeshReq_bufTemp[0] = L8DEV_MESH_CMD_SCENARIO_SET; //mesh命令
				memcpy(&dataMeshReq_bufTemp[1], event->data, event->data_len); //数据
				
				ret = mwifi_root_write((const uint8_t *)&(event->data[targetMacIstStart]), 1,
									   &data_type, dataMeshReq_bufTemp, event->data_len + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
				MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));	

				os_free(dataMeshReq_bufTemp);
			}

		}break;

		case cmdTopicM2SInsert_cmdWifiChg_overall:{

			mwifi_config_t ap_config = {0x0};
			struct stt_paramWifiConfig{

				char router_ssid[32];
				char router_password[64];
				uint8_t router_bssid[6];
				
			}param_wifiConfig = {0};
			const uint8_t boardcastAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;

			//自身wifi信息修改
			memcpy(&param_wifiConfig, event->data, sizeof(struct stt_paramWifiConfig));
			mdf_info_load("ap_config", &ap_config, sizeof(mwifi_config_t));
			memcpy(ap_config.router_ssid, param_wifiConfig.router_ssid, sizeof(char) * 32);
			memcpy(ap_config.router_password, param_wifiConfig.router_password, sizeof(char) * 64);
			memcpy(ap_config.router_bssid, param_wifiConfig.router_bssid, sizeof(uint8_t) * 6);
			memcpy(ap_config.mesh_id, param_wifiConfig.router_bssid, sizeof(uint8_t) * 6);
			mdf_info_save("ap_config", &ap_config, sizeof(mwifi_config_t));

			//通知网内所有设备wifi信息修改
			dataRespond_temp[0] = L8DEV_MESH_CMD_NEIWORK_PARAM_CHG; //mesh命令
			memcpy(&dataRespond_temp[1], &param_wifiConfig, sizeof(struct stt_paramWifiConfig)); //数据
			
			ret = mwifi_root_write(boardcastAddr, 1, &data_type, dataRespond_temp, sizeof(struct stt_paramWifiConfig) + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
			MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 

			//倒计时重启触发
			usrApplication_systemRestartTrig(5);
			
		}break;

		case cmdTopicM2SInsert_cmdSysParamCfg_overall:{

			stt_timeZone timeZone_temp = {0};
			stt_mqttCfgParam mqttCfg_temp = {0};
		
			const uint16_t dataComming_lengthLimit = 8;
			
			const uint8_t boardcastAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
		
			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			//自身时区修改
			timeZone_temp.timeZone_H = (uint8_t)event->data[0];
			timeZone_temp.timeZone_M = (uint8_t)event->data[1];
			deviceParamSet_timeZone(&timeZone_temp, true);

			//自身mqtt配置信息修改
			memcpy(mqttCfg_temp.ip_remote, (uint8_t *)&event->data[2], sizeof(uint8_t) * 4);
			mqttCfg_temp.port_remote  = ((uint16_t)(event->data[6]) << 8) & 0xff00;
			mqttCfg_temp.port_remote |= ((uint16_t)(event->data[7]) << 0) & 0x00ff;
			mqttRemoteConnectCfg_paramSet(&mqttCfg_temp, true);

			//通知网内所有设备相关配置信息
			dataRespond_temp[0] = L8DEV_MESH_CMD_SYSTEM_PARAM_CHG; //mesh命令
			memcpy(&dataRespond_temp[1], event->data, event->data_len); //数据
			
			ret = mwifi_root_write(boardcastAddr, 1, &data_type, dataRespond_temp, event->data_len + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
			MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 

		}break;

		case cmdTopicM2SInsert_cmdDevLock_multiple:{

			uint8_t loop = 0;
			uint8_t scenarioUnit_num = event->data[0];
			struct stt_devLockOpreatParam{

				uint8_t devMacAddr[MWIFI_ADDR_LEN];
				uint8_t devLockVal;
				
			}devLockOpreatParam_unit = {0};
			uint8_t dataParamUintTemp_length = sizeof(struct stt_devLockOpreatParam);

			const uint16_t dataComming_lengthLimit = 7;
			
			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			if(scenarioUnit_num == 0xFF){ //全改

				const uint8_t boardcastAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
				uint8_t devLock_dats = event->data[7];
				uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();
				
				(devLock_dats)?
					(devRunningFlg_temp |= DEV_RUNNING_FLG_BIT_DEVLOCK):
					(devRunningFlg_temp &= (~DEV_RUNNING_FLG_BIT_DEVLOCK));
				
				currentDevRunningFlg_paramSet(devRunningFlg_temp, true);

				dataRespond_temp[0] = L8DEV_MESH_CMD_DEVLOCK; //mesh命令
				memcpy(&dataRespond_temp[1], &devLock_dats, 1); //数据
				
				ret = mwifi_root_write(boardcastAddr, 1,
									   &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 
			}
			else //选改
			{
				for(loop = 0; loop < scenarioUnit_num; loop ++){
				
					os_memset(&devLockOpreatParam_unit, 0, dataParamUintTemp_length);
					os_memcpy(&devLockOpreatParam_unit,
							  &(event->data[loop * dataParamUintTemp_length + 1]), 
							  dataParamUintTemp_length);
				
					if(!memcmp(devSelfMac, devLockOpreatParam_unit.devMacAddr, MWIFI_ADDR_LEN)){
				
						uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();
						
						(devLockOpreatParam_unit.devLockVal)?
							(devRunningFlg_temp |= DEV_RUNNING_FLG_BIT_DEVLOCK):
							(devRunningFlg_temp &= (~DEV_RUNNING_FLG_BIT_DEVLOCK));
						
						currentDevRunningFlg_paramSet(devRunningFlg_temp, true);
					}
					else
					{
						dataRespond_temp[0] = L8DEV_MESH_CMD_DEVLOCK; //mesh命令
						memcpy(&dataRespond_temp[1], &(devLockOpreatParam_unit.devLockVal), 1); //数据
				
						ret = mwifi_root_write((const uint8_t *)devLockOpreatParam_unit.devMacAddr, 1,
											 &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
						MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 				
					}
				}
			}

		}break;

		case cmdTopicM2SInsert_cmdUiStyle_multiple:{

			extern void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF);

			uint8_t loop = 0;
			uint8_t scenarioUnit_num = event->data[0];
			struct stt_uiThemeStyleParam{
			
				uint8_t devMacAddr[MWIFI_ADDR_LEN];
				uint8_t themeStyle_flg;
				
			}uiThemeStyleParam_unit = {0};
			uint8_t dataParamUintTemp_length = sizeof(struct stt_uiThemeStyleParam);
			
			const uint16_t dataComming_lengthLimit = 7;
			
			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			if(scenarioUnit_num == 0xFF){ //全改

				const uint8_t boardcastAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_BROADCAST;
				uint8_t themeStyleFlg = event->data[7];
			
				usrAppHomepageThemeType_Set(themeStyleFlg, true);
				
				dataRespond_temp[0] = L8DEV_MESH_CMD_UISET_THEMESTYLE; //mesh命令
				memcpy(&dataRespond_temp[1], &themeStyleFlg, 1); //数据
				
				ret = mwifi_root_write(boardcastAddr, 1,
									 &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}
			else //选改
			{
				for(loop = 0; loop < scenarioUnit_num; loop ++){
				
					os_memset(&uiThemeStyleParam_unit, 0, dataParamUintTemp_length);
					os_memcpy(&uiThemeStyleParam_unit,
							  &(event->data[loop * dataParamUintTemp_length + 1]), 
							  dataParamUintTemp_length);
				
					if(!memcmp(devSelfMac, uiThemeStyleParam_unit.devMacAddr, MWIFI_ADDR_LEN)){

						usrAppHomepageThemeType_Set(uiThemeStyleParam_unit.themeStyle_flg, true);
					}
					else
					{
						dataRespond_temp[0] = L8DEV_MESH_CMD_UISET_THEMESTYLE; //mesh命令
						memcpy(&dataRespond_temp[1], &(uiThemeStyleParam_unit.themeStyle_flg), 1); //数据
					
						ret = mwifi_root_write((const uint8_t *)uiThemeStyleParam_unit.devMacAddr, 1,
											 &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
						MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 				
					}
				}
			}
	
		}break;

		case cmdTopicM2SInsert_cmdMeshUpgradeForce:{

			const uint8_t numCheck_len = 127;
			uint8_t checkNum = numCheckMethod_customLanbon((uint8_t *)event->data, numCheck_len);

			if(checkNum == (uint8_t)event->data[numCheck_len]){

				const uint8_t targetMacIstStart = 0;
				const uint8_t versionNumIstStart = 6;
				const uint8_t severIpIstStart = 7;
				const uint8_t firewareNameIstStart = 11;

				usrAppUpgrade_firewareNameSet((const char *)&event->data[firewareNameIstStart]); //修改upgrade目标filename
				usrAppUpgrade_remoteIPset((uint8_t *)&event->data[severIpIstStart]); //IP设置

				usrApp_firewareUpgrade_trig(true, (uint8_t)currentDev_typeGet());
			}

		}break;

		case cmdTopicM2SInsert_cmdMeshUpgradeNotice:{

			const uint8_t numCheck_len = 127;
			uint8_t checkNum = numCheckMethod_customLanbon((uint8_t *)event->data, numCheck_len);

			if(checkNum == (uint8_t)event->data[numCheck_len]){ //异或校验

				const uint8_t targetMacIstStart = 0;
				const uint8_t versionNumIstStart = 6;
				const uint8_t severIpIstStart = 7;
				const uint8_t firewareNameIstStart = 11;
			
					  uint8_t firewareVersionNum = event->data[versionNumIstStart];

				usrAppUpgrade_firewareNameSet((const char*)&event->data[firewareNameIstStart]); //修改upgrade目标filename
				usrAppUpgrade_targetDevaddrSet((const uint8_t *)&(event->data[targetMacIstStart])); //修改upgrade目标mac
				usrAppUpgrade_remoteIPset((uint8_t *)&event->data[severIpIstStart]); //IP设置

				ESP_LOGI(TAG, "numCheck success, ver:%02X\n", firewareVersionNum);

				if(!memcmp(devSelfMac, (uint8_t *)&(event->data[targetMacIstStart]), MWIFI_ADDR_LEN)){

					extern void appUiElementSet_upgradeAvailable(bool val);

					if(firewareVersionNum > L8_DEVICE_VERSION){ //版本固件可用比较
					
						appUiElementSet_upgradeAvailable(true);
					}
					else
					{
						appUiElementSet_upgradeAvailable(false);
					}
				}
				else
				{
					dataRespond_temp[0] = L8DEV_MESH_CMD_FWARE_CHECK; //mesh命令
					memcpy(&dataRespond_temp[1], (uint8_t *)&firewareVersionNum, 1); //数据
					
					ret = mwifi_root_write((const uint8_t *)&(event->data[targetMacIstStart]), 1, &data_type, dataRespond_temp, 1 + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
					MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 	
				}
			}
			else
			{
				ESP_LOGI(TAG, "numCheck fail, target:%02X, should:%02X\n", (uint8_t)event->data[numCheck_len], checkNum);
			}

		}break;

		case cmdTopicM2SInsert_L8devLoginNotice_cfm:{

			printf("mqtt login notice confirm rep from serve, meshID[%02X%02X%02X%02X%02X%02X]\n", MAC2STR(&(event->data[1])));
			
			if(0x0A == (uint8_t)event->data[0]){ //命令确认
			
				uint8_t meshId_BsdCur[6] = {0};
				uint8_t *meshId_rcvCfm	 = (uint8_t *)&(event->data[1]); //接收到的确认meshID(即当前bssid)，下标1开始
			
				devRouterConnectBssid_Get(meshId_BsdCur);
			
				if(0 == memcmp(meshId_BsdCur, meshId_rcvCfm, 6)){ //当前设备么是ID确认(防止多个主机同时在进行确认)
			
					devRouterRecordBssid_Set(meshId_BsdCur, true);
			
					counterDownRecord_loginConnectNotice = 0;
			
					printf("mqtt login notice confirm success!\n");
				}
			}

		}break;

		case cmdTopicM2SInsert_cmdQuery:{
			
			enum{

				cmdQuery_deviceStatus = 0x11,
				cmdQuery_deviceStatus_specified = 0x12,
				cmdQuery_timerNormal = 0xA0,
				cmdQuery_delayTrig,
				cmdQuery_greenMode,
				cmdQuery_nightMode,
				cmdQuery_mutualCtrl = 0x42,
			};

			const uint16_t dataComming_lengthLimit = 7;

			if(event->data_len < dataComming_lengthLimit)break; //数据长度最短限制

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDQUERY]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if((uint8_t)event->data[0] == cmdQuery_mutualCtrl)data_sendToRoot_IF = true; //特殊查询，不进行mac地址核对
			if((uint8_t)event->data[0] == cmdQuery_deviceStatus)data_sendToRoot_IF = true; //特殊查询，不进行mac地址核对
			if((uint8_t)event->data[0] == cmdQuery_deviceStatus_specified)data_sendToRoot_IF = true; //特殊查询，不进行mac地址核对
			if(data_sendToRoot_IF){

				uint16_t mqttData_pbLen = 0;
			
				switch((uint8_t)event->data[0]){

					case cmdQuery_deviceStatus:{

						uint8_t *dataResp_devStatusInfo = NULL;
						uint8_t devUnitNum_temp = 0;
						uint16_t devUnitNumLimit_perPack = (USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH - 1) / sizeof(stt_devStatusInfoResp); //mqtt单包包含设备信息 数量限制
	
						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdStatusGet]);
						
						dataResp_devStatusInfo = L8devStatusInfoGet(listHead_nodeDevDataManage);
						devUnitNum_temp = dataResp_devStatusInfo[0];
						if(devUnitNum_temp == DEVLIST_MANAGE_LISTNUM_MASK_NULL){ //设备采集未就绪，设备还未足够运行一个心跳周期

							mqttData_pbLen = 1;
							printf("mqtt cmdQuery_devStatus respondNoReady res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataResp_devStatusInfo, mqttData_pbLen, 1, 0));
						}
						else
						{
							uint8_t dataRespLoop = devUnitNum_temp / devUnitNumLimit_perPack; //商
							uint8_t dataRespLastPack_devNum = devUnitNum_temp % devUnitNumLimit_perPack; //余
							uint8_t dataRespPerPackBuff[USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH] = {0};
							uint16_t dataRespPerPackInfoLen = 0;
							uint16_t dataRespLoadInsert = 1;

							//总信息长度超过一个包则拆分发送 -商包
							if(dataRespLoop){

								for(uint8_t loop = 0; loop < dataRespLoop; loop ++){
								
									dataRespPerPackInfoLen = sizeof(stt_devStatusInfoResp) * devUnitNumLimit_perPack;
									dataRespPerPackBuff[0] = dataResp_devStatusInfo[0];
									memcpy(&dataRespPerPackBuff[1], &dataResp_devStatusInfo[dataRespLoadInsert], dataRespPerPackInfoLen);
									dataRespLoadInsert += dataRespPerPackInfoLen;
									mqttData_pbLen = dataRespPerPackInfoLen + 1;
									
									printf("mqtt cmdQuery_devStatus respond loop%d res:0x%04X.\n", loop, esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
									memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
								}
							}

							//设备信息收尾数据包 -余包
							if(dataRespLastPack_devNum){

								dataRespPerPackInfoLen = sizeof(stt_devStatusInfoResp) * dataRespLastPack_devNum;
								dataRespPerPackBuff[0] = dataResp_devStatusInfo[0];
								memcpy(&dataRespPerPackBuff[1], &dataResp_devStatusInfo[dataRespLoadInsert],  dataRespPerPackInfoLen);
								dataRespLoadInsert += dataRespPerPackInfoLen;
								mqttData_pbLen = dataRespPerPackInfoLen + 1;
								printf("mqtt cmdQuery_devStatus respondTail res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
								memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
							}
						}
							
						os_free(dataResp_devStatusInfo); //设备信息获取缓存 释放

					}break;

					case cmdQuery_deviceStatus_specified:{

						const uint8_t dataIst_devsSpecifiedMAC = 1;
						struct stt_deviceStatusParam{
							
							uint8_t nodeDev_Type;
							uint8_t nodeDev_Status;
							uint16_t nodeDev_DevRunningFlg;
							stt_devTempParam2Hex nodeDev_dataTemprature;
							stt_devPowerParam2Hex nodeDev_dataPower;	
							uint8_t nodeDev_extFunParam[DEVPARAMEXT_DT_LEN];
							uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
						}deviceStatusParam_temp = {0};

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				     (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdStatusGet_specified]);

						if(!memcmp(devSelfMac, &(event->data[dataIst_devsSpecifiedMAC]), sizeof(uint8_t) * MWIFI_ADDR_LEN)){ //指定对象是否是自己

							//本机状态信息填装
							deviceStatusParam_temp.nodeDev_Type = (uint8_t)currentDev_typeGet();
							currentDev_dataPointGet((stt_devDataPonitTypedef *)&(deviceStatusParam_temp.nodeDev_Status));
							deviceStatusParam_temp.nodeDev_DevRunningFlg = currentDevRunningFlg_paramGet();
							devDriverBussiness_temperatureMeasure_getByHex(&(deviceStatusParam_temp.nodeDev_dataTemprature));
							devDriverBussiness_elecMeasure_valPowerGetByHex(&(deviceStatusParam_temp.nodeDev_dataPower));
							switch(currentDev_typeGet()){ //扩展数据填装
							
								case devTypeDef_curtain:
								case devTypeDef_moudleSwCurtain:{
							
									deviceStatusParam_temp.nodeDev_extFunParam[0] = devCurtain_currentPositionPercentGet();
							
								}break;
								
								case devTypeDef_heater:{
							
//									uint16_t heater_gearCur_period = devDriverBussiness_heaterSwitch_closePeriodCurrent_Get();
									uint16_t heater_gearCur_period = devDriverBussiness_heaterSwitch_closePeriodCustom_Get(); //移动端说只要自定义时间档就可以了
									uint16_t heater_timeRem_counter = devDriverBussiness_heaterSwitch_devParam_closeCounter_Get();
								
//									memcpy(&(deviceStatusParam_temp.nodeDev_extFunParam[0]), &heater_gearCur_period, sizeof(uint16_t));
//									memcpy(&(deviceStatusParam_temp.nodeDev_extFunParam[2]), &heater_timeRem_counter, sizeof(uint16_t));

									deviceStatusParam_temp.nodeDev_extFunParam[0] = (uint8_t)((heater_gearCur_period >> 8) & 0x00ff);
									deviceStatusParam_temp.nodeDev_extFunParam[1] = (uint8_t)((heater_gearCur_period >> 0) & 0x00ff);
									deviceStatusParam_temp.nodeDev_extFunParam[2] = (uint8_t)((heater_timeRem_counter >> 8) & 0x00ff);
									deviceStatusParam_temp.nodeDev_extFunParam[3] = (uint8_t)((heater_timeRem_counter >> 0) & 0x00ff);
									
								}break;

								case devTypeDef_thermostatExtension:{

									deviceStatusParam_temp.nodeDev_extFunParam[0] = devDriverBussiness_thermostatSwitch_exSwitchParamGetWithRcd();

								}break;
								
								case devTypeDef_mulitSwOneBit:
								case devTypeDef_mulitSwTwoBit:
								case devTypeDef_mulitSwThreeBit:
								case devTypeDef_moudleSwOneBit:
								case devTypeDef_moudleSwTwoBit:
								case devTypeDef_moudleSwThreeBit:
								case devTypeDef_dimmer:
								case devTypeDef_fans:
								case devTypeDef_scenario:
								case devTypeDef_socket:
								case devTypeDef_thermostat:
								
								default:{}break;
							}
							memcpy(deviceStatusParam_temp.nodeDev_Mac, 
								   devSelfMac,
								   sizeof(uint8_t) * MWIFI_ADDR_LEN);

							printf("MAC RCV[%02X %02X %02X %02X %02X %02X].\n", MAC2STR((uint8_t *)&(event->data[dataIst_devsSpecifiedMAC])));

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)|\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)|\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
							
							devBeepTips_trig(4, 8, 100, 40, 2); //beeps提示
#else
							lvGui_wifiConfig_bussiness_configComplete_tipsTrig();
#endif
						}
						else
						{
							stt_nodeDev_hbDataManage *nodeDevHbData_Param = L8devHbDataManageList_nodeGet(listHead_nodeDevDataManage, 
																		    (uint8_t *)&(event->data[dataIst_devsSpecifiedMAC]),
																		    false);

							if(nodeDevHbData_Param != NULL){

								deviceStatusParam_temp.nodeDev_Type = 			nodeDevHbData_Param->dataManage.nodeDev_Type;
								deviceStatusParam_temp.nodeDev_Status = 		nodeDevHbData_Param->dataManage.nodeDev_Status;
								deviceStatusParam_temp.nodeDev_DevRunningFlg = 	nodeDevHbData_Param->dataManage.nodeDev_runningFlg;
								memcpy(&(deviceStatusParam_temp.nodeDev_dataTemprature), &(nodeDevHbData_Param->dataManage.nodeDev_dataTemprature), sizeof(stt_devTempParam2Hex));
								memcpy(&(deviceStatusParam_temp.nodeDev_dataPower), 	 &(nodeDevHbData_Param->dataManage.nodeDev_dataPower),		sizeof(stt_devPowerParam2Hex));
								memcpy(deviceStatusParam_temp.nodeDev_extFunParam, 	 	 nodeDevHbData_Param->dataManage.nodeDev_extFunParam,		sizeof(uint8_t) * DEVPARAMEXT_DT_LEN);
								memcpy(deviceStatusParam_temp.nodeDev_Mac, 	 	 		 nodeDevHbData_Param->dataManage.nodeDev_Mac,				sizeof(uint8_t) * MWIFI_ADDR_LEN);

								os_free(nodeDevHbData_Param);

								dataRespond_temp[0] = L8DEV_MESH_CMD_SPEQUERY_NOTICE; //mesh命令 //数据
								
								ret = mwifi_root_write((const uint8_t *)&(event->data[dataIst_devsSpecifiedMAC]), 1, &data_type, dataRespond_temp, 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
								MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 

								printf("MAC RCV[%02X %02X %02X %02X %02X %02X].\n", MAC2STR((uint8_t *)&(event->data[dataIst_devsSpecifiedMAC])));
							}
							else
							{
								printf("mqtt cmdQuery_statusGet_specified MAC not found.\n");
								printf("MAC RCV[%02X %02X %02X %02X %02X %02X].\n", MAC2STR((uint8_t *)&(event->data[dataIst_devsSpecifiedMAC])));
							}
						}

						memcpy(dataRespond_temp, &deviceStatusParam_temp, sizeof(struct stt_deviceStatusParam));
						mqttData_pbLen = sizeof(struct stt_deviceStatusParam);
						printf("mqtt cmdQuery_statusGet_specified respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespond_temp, mqttData_pbLen, 1, 0));

					}break;

					case cmdQuery_timerNormal:{

						uint8_t dataLoad_ist = 0;

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																										 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_normal]);
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) //红外定时数据另外获取并添加

						usrAppActTrigTimer_paramGet((usrApp_trigTimer *)&dataRespond_temp[dataLoad_ist]);
						dataLoad_ist += MACADDR_INSRT_START_CMDTIMERSET;
						memcpy(&dataRespond_temp[dataLoad_ist], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						dataLoad_ist += DEVICE_MAC_ADDR_APPLICATION_LEN;
						devDriverBussiness_infraredSwitch_timerUpTrigIstTabGet(&dataRespond_temp[dataLoad_ist]);
						dataLoad_ist += USRAPP_VALDEFINE_TRIGTIMER_NUM;
						mqttData_pbLen = dataLoad_ist;
#else

						usrAppActTrigTimer_paramGet((usrApp_trigTimer *)&dataRespond_temp[dataLoad_ist]);
						dataLoad_ist += MACADDR_INSRT_START_CMDTIMERSET;
						memcpy(&dataRespond_temp[dataLoad_ist], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						dataLoad_ist += DEVICE_MAC_ADDR_APPLICATION_LEN;
						mqttData_pbLen = dataLoad_ist;
#endif
						printf("mqtt cmdQuery_timerNormal respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespond_temp, mqttData_pbLen, 1, 0));

					}break;

					case cmdQuery_delayTrig:{

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_delay]);

						usrAppParamGet_devDelayTrig(dataRespond_temp);
						memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDDELAYSET], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						mqttData_pbLen = MACADDR_INSRT_START_CMDDELAYSET + DEVICE_MAC_ADDR_APPLICATION_LEN;
						printf("mqtt cmdQuery_delayTrig respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespond_temp, mqttData_pbLen, 1, 0));
						
					}break;

					case cmdQuery_greenMode:{

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_greenMode]);
						
						usrAppParamGet_devGreenMode(dataRespond_temp);
						memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDGREENMODESET], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						mqttData_pbLen = MACADDR_INSRT_START_CMDGREENMODESET + DEVICE_MAC_ADDR_APPLICATION_LEN;
						printf("mqtt cmdQuery_greenMode respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespond_temp, mqttData_pbLen, 1, 0));

					}break;

					case cmdQuery_nightMode:{

						uint8_t dataTemp[6] = {0};

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_nightMode]);
						
						usrAppNightModeTimeTab_paramGet((usrApp_trigTimer *)&dataTemp);
						dataRespond_temp[0] = (uint8_t)dataTemp[0]; //全天
						dataRespond_temp[1] = (uint8_t)dataTemp[3]; //非全天
						dataRespond_temp[2] = (uint8_t)dataTemp[1]; //时段1：时
						dataRespond_temp[3] = (uint8_t)dataTemp[2]; //时段1：分
						dataRespond_temp[4] = (uint8_t)dataTemp[4]; //时段2：时
						dataRespond_temp[5] = (uint8_t)dataTemp[5]; //时段2：分

						memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDNIGHTMODESET], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						mqttData_pbLen = MACADDR_INSRT_START_CMDNIGHTMODESET + DEVICE_MAC_ADDR_APPLICATION_LEN;
						printf("mqtt cmdQuery_nightMode respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataRespond_temp, mqttData_pbLen, 1, 0));

					}break;

					case cmdQuery_mutualCtrl:{

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdMutualGet]);

						uint8_t mutualGroupIst_query = (uint8_t)event->data[7];
						stt_mutualCtrlInfoResp *dataResp_mutualInfo = L8devMutualCtrlInfo_Get(listHead_nodeDevDataManage, mutualGroupIst_query);
						mqttData_pbLen = sizeof(stt_mutualCtrlInfoResp);
						printf("mqtt cmdQuery_mutualCtrlInfo respond res:0x%04X.\n", 
								esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataResp_mutualInfo, mqttData_pbLen, 1, 0));
						os_free(dataResp_mutualInfo);

					}break;

					default:break;
				}
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_DEVINFO_GET;
				memcpy(&dataRespond_temp[1], &event->data[0], 1); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[1], 1,
									   &data_type, dataRespond_temp, 1 + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		default:break;
	}
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
	uint8_t devRouterBssid[6] = {0};

	mqttClientElecsumRepot_reserveFlg = false;
	
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:

			remoteMqtt_connectFlg = true;

//			usrAppClient = client; //更新client

			counterDownRecord_loginConnectNotice = USRDEF_MQTT_DEVCONNECT_NOTICE_LOOP_MAX;
			devRouterConnectBssid_Get(devRouterBssid);

			memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X/#", MAC2STR(devRouterBssid));
			msg_id = esp_mqtt_client_subscribe(client, devMqtt_topicTemp, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_B"MAC:%02X%02X%02X%02X%02X%02X/#", MAC2STR(devRouterBssid));
			msg_id = esp_mqtt_client_subscribe(client, devMqtt_topicTemp, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//			memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
//			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"%s", (char *)mqttTopicSpecial_elecsumReport);
//			msg_id = esp_mqtt_client_subscribe(client, devMqtt_topicTemp, 0);
//			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			usrApp_devRootStatusSynchronousInitiative(); //主机上线通知

//            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
//            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

//            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
//            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

//            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
//            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
			
            break;
        case MQTT_EVENT_DISCONNECTED:

			remoteMqtt_connectFlg = false;
		
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data", 0, 0, 0);
//            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

			dtCounter_preventSurge = USRDEF_MQTT_DTSURGE_PREVENT_KPTIME;
		
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
		
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//          printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//			printf("DATALEN=%d\r\n", event->data_len);

			uint8_t searchRes = topicM2S_compareResult(event->topic, event->topic_len);
			if(searchRes < DEVMQTT_TOPIC_NUM_M2S){ //常规主题索引处理
			
				mqtt_remoteDataHandler(event, searchRes);				
			}

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;

		case MQTT_EVENT_BEFORE_CONNECT:break;
    }

	mqttClientElecsumRepot_reserveFlg = true;
	
    return ESP_OK;
}

#define ELECSUM_REPORT_USR_DEBUG	0

#if(ELECSUM_REPORT_USR_DEBUG == 1)
void mqtt_rootDevRemoteDatatransLoop_elecSumReport(void){

	const uint8_t devSum = 6;
	const uint8_t devRouterBssid[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0xAA, 0xBB, 0xA1, 0xA2, 0xA3, 0xA4};
	uint8_t devNodeMAC[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0, 0, 0, 0, 0, 1};
	static float devElecSum_temp = 12345.67F;

	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT);
	else{

		return; //角色不对，不可用
	}

	if(!remoteMqtt_connectFlg)return; //远程连接不可用

	while(!mqttClientElecsumRepot_reserveFlg)vTaskDelay(1 / portTICK_PERIOD_MS); //等待mqttClient资源可用

	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零
	memset(dataRespond_temp, 0, DEVMQTT_DATA_RESPOND_LENGTH); //数据缓存清零
	
	sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"%s", (char *)mqttTopicSpecial_elecsumReport);

	dataRespond_temp[0] = devSum;
	dataRespond_temp[1] = 8;
	dataRespond_temp[2] = 0;
	memcpy(&dataRespond_temp[3], devRouterBssid, sizeof(uint8_t) * DEVICE_MAC_ADDR_APPLICATION_LEN);
	
	for(uint8_t loop = 0; loop < devSum; loop ++){

		memcpy(&dataRespond_temp[loop * 9 + 9], devNodeMAC, sizeof(uint8_t) * DEVICE_MAC_ADDR_APPLICATION_LEN);
		functionSpecialUsrApp_floatToHex(&dataRespond_temp[loop * 9 + 15], devElecSum_temp);
		devNodeMAC[5] ++;
	}

	devElecSum_temp += 1.01F;

	printf("mqtt cmdQuery_devElecsumInfo debugOpreat report res:0x%04X.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespond_temp, devSum * 9 + 9, 1, 0));
}

#else
void mqtt_rootDevRemoteDatatransLoop_elecSumReport(void){

	#define localUsrDef_elecRep_frameHeadLen	9

	uint8_t *dataReport_devElecsumInfo = NULL;
	uint8_t devUnitNum_temp = 0;
	uint16_t devUnitNumLimit_perPack = (USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH - localUsrDef_elecRep_frameHeadLen) / sizeof(stt_devUnitElecsumReport); //mqtt单包包含设备信息 数量限制
	uint16_t mqttData_pbLen = 0;

	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT);
	else{

		return; //角色不对，不可用
	}

	ESP_LOGI(TAG, "rootDev elecsumInfo report trig.");

	if(!remoteMqtt_connectFlg)return; //远程连接不可用

	while(!mqttClientElecsumRepot_reserveFlg)vTaskDelay(1 / portTICK_PERIOD_MS); //等待mqttClient资源可用

	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零
	sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"%s", (char *)mqttTopicSpecial_elecsumReport);

	dataReport_devElecsumInfo = L8devElecsumInfoGet(listHead_nodeDevDataManage);
	devUnitNum_temp = dataReport_devElecsumInfo[0];
	printf("elec devNum:%d.\n", dataReport_devElecsumInfo[0]);
	if(devUnitNum_temp == DEVLIST_MANAGE_LISTNUM_MASK_NULL){ //设备采集未就绪，设备还未足够运行一个心跳周期

		mqttData_pbLen = 1;
		printf("mqtt cmdQuery_devElecsumInfo reportNoReady res:0x%04X.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataReport_devElecsumInfo, mqttData_pbLen, 1, 0));
	}
	else
	{
		uint8_t dataRespLoop = devUnitNum_temp / devUnitNumLimit_perPack; //商
		uint8_t dataRespLastPack_devNum = devUnitNum_temp % devUnitNumLimit_perPack; //余
		uint8_t dataRespPerPackBuff[USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH] = {0};
		uint16_t dataRespPerPackInfoLen = 0;
		uint16_t dataRespLoadInsert = localUsrDef_elecRep_frameHeadLen;

		//总信息长度超过一个包则拆分发送 -商包
		if(dataRespLoop){

			for(uint8_t loop = 0; loop < dataRespLoop; loop ++){
			
				dataRespPerPackInfoLen = sizeof(stt_devUnitElecsumReport) * devUnitNumLimit_perPack;
				dataRespPerPackBuff[0] = devUnitNumLimit_perPack;
				memcpy(&dataRespPerPackBuff[1], &dataReport_devElecsumInfo[1], localUsrDef_elecRep_frameHeadLen - 1);
				memcpy(&dataRespPerPackBuff[localUsrDef_elecRep_frameHeadLen], &dataReport_devElecsumInfo[dataRespLoadInsert], dataRespPerPackInfoLen);
				dataRespLoadInsert += dataRespPerPackInfoLen;
				mqttData_pbLen = dataRespPerPackInfoLen + localUsrDef_elecRep_frameHeadLen;
				
				printf("mqtt cmdQuery_devElecsumInfo reportLoop%d res:0x%04X.\n", loop, esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
				memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
			}
		}

		//设备信息收尾数据包 -余包
		if(dataRespLastPack_devNum){

			dataRespPerPackInfoLen = sizeof(stt_devUnitElecsumReport) * dataRespLastPack_devNum;
			dataRespPerPackBuff[0] = dataRespLastPack_devNum;
			memcpy(&dataRespPerPackBuff[1], &dataReport_devElecsumInfo[1], localUsrDef_elecRep_frameHeadLen - 1);
			memcpy(&dataRespPerPackBuff[localUsrDef_elecRep_frameHeadLen], &dataReport_devElecsumInfo[dataRespLoadInsert], dataRespPerPackInfoLen);
			dataRespLoadInsert += dataRespPerPackInfoLen;
			mqttData_pbLen = dataRespPerPackInfoLen + localUsrDef_elecRep_frameHeadLen;
			
			printf("mqtt cmdQuery_devElecsumInfo reportTail res:0x%04X.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
			memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
		}
	}
		
	os_free(dataReport_devElecsumInfo); //设备信息获取缓存 释放
}

#endif

void mqtt_rootDevLoginConnectNotice_trig(void){

	if(!counterDownRecord_loginConnectNotice)return;
	else{

		counterDownRecord_loginConnectNotice --;
	}

	MDF_LOGI("mqtt connect notice loop:%d\n", counterDownRecord_loginConnectNotice);

	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT){ //根节点有效业务

		uint8_t meshId_BsdRcd[6] = {0},
				meshId_BsdCur[6] = {0};

		uint8_t dataRespPerPackBuff[USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH] = {0};
		uint16_t mqttData_pbLen = 0;

		memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零
		sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"%s", (char *)mqttTopicSpecial_loginConnectNotice_req);

		devRouterRecordBssid_Get(meshId_BsdRcd);
		devRouterConnectBssid_Get(meshId_BsdCur);

		memcpy(&dataRespPerPackBuff[0], meshId_BsdRcd, 6);
		memcpy(&dataRespPerPackBuff[6], meshId_BsdCur, 6);
		mqttData_pbLen = 12;

		printf("mqtt cmdMqttLoginConnect notice res:0x%04X.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
	}
}

void mqtt_remoteDataTrans(uint8_t dtCmd, uint8_t *data, uint16_t dataLen){

	enum{

		cmdQuery_deviceStatus = L8DEV_MESH_CMD_DEV_STATUS,
		cmdQuery_timerNormal = L8DEV_MESH_CMD_SET_TIMER,
		cmdQuery_delayTrig,
		cmdQuery_greenMode,
		cmdQuery_nightMode,
		cmdReport_statusSynchro = L8DEV_MESH_STATUS_SYNCHRO,
		cmdReport_firewareRsvCheck = L8DEV_MESH_CMD_FWARE_CHECK,
	};

	bool dtCmd_identify = false;

	uint8_t devRouterBssid[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};

	devRouterConnectBssid_Get(devRouterBssid);

	memset(dataRespond_temp, 0, DEVMQTT_DATA_RESPOND_LENGTH); //数据缓存清零
	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零

	if(remoteMqtt_connectFlg){ //远程连接是否可用？
		
		switch(dtCmd){

			case cmdQuery_deviceStatus:{

				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																			 				   	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdStatusGet]);
			}break;
		
			case cmdQuery_timerNormal:{
		
				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																			 				   	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_normal]);
			}break;
			
			case cmdQuery_delayTrig:{
		
				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																			 				   	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_delay]);
			}break;
		
			case cmdQuery_greenMode:{
		
				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																			 				   	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_greenMode]);
			}break;
		
			case cmdQuery_nightMode:{
		
				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																			 				   	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_nightMode]);
			}break;

			case cmdReport_statusSynchro:{

				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																							 	 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdStatusSynchro]);
			}break;

			case cmdReport_firewareRsvCheck:{
				
				dtCmd_identify = true;
				sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																								 (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdMeshUpgradeCheck]);
			}break;
		
			default:break;
		}
	}

	if(dtCmd_identify){

		memcpy(dataRespond_temp, data, dataLen);
		printf("mqtt normal dataTrans res:%d.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespond_temp, dataLen, 1, 0));
	}
}

void usrApp_devRootStatusSynchronousInitiative(void){

	uint8_t dataTrans_temp[1 + DEVPARAMEXT_DT_LEN + DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};
	uint16_t dataTrans_tempLen = 1 + DEVPARAMEXT_DT_LEN + DEVICE_MAC_ADDR_APPLICATION_LEN;
	
	currentDev_dataPointGetwithRecord((stt_devDataPonitTypedef *)&dataTrans_temp[0]);
	currentDev_extParamGet(&dataTrans_temp[1]);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, &dataTrans_temp[5]);

	mqtt_remoteDataTrans(L8DEV_MESH_STATUS_SYNCHRO, dataTrans_temp, dataTrans_tempLen);
}

void usrApp_deviceStatusSynchronousInitiative(void){

	if(mwifi_is_connected()){

		if(esp_mesh_get_layer() == MESH_ROOT){

			usrApp_devRootStatusSynchronousInitiative();
		}
		else
		{
			usrApp_devNodeStatusSynchronousInitiative();
		}
	}
}

void devFireware_upgradeReserveCheck(void){

	const uint8_t dataTransLength_limit = 12;

	uint8_t devRouterBssid[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};
	uint8_t devSelfMac[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};

	devRouterConnectBssid_Get(devRouterBssid);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);

	memset(dataRespond_temp, 0, DEVMQTT_DATA_RESPOND_LENGTH); //数据缓存清零
	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零

	//内容装填
	memcpy(&dataRespond_temp[0], devSelfMac, sizeof(uint8_t) * DEVICE_MAC_ADDR_APPLICATION_LEN); //MAC
	dataRespond_temp[6] = L8_DEVICE_VERSION; //版本号
	dataRespond_temp[dataTransLength_limit - 1] = numCheckMethod_customLanbon(dataRespond_temp, dataTransLength_limit - 1); //校验码

	if(mwifi_is_connected()){

		if(esp_mesh_get_layer() == MESH_ROOT){ //主机fireware check直接mqtt发出去
			
			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid), (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdMeshUpgradeCheck]);

			printf("mqtt firewareRsv check res:%d.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespond_temp, dataTransLength_limit, 1, 0));
		}
		else //子机fireware check则进行mdf转发
		{
			mdf_err_t ret = MDF_OK;
			mwifi_data_type_t data_type = {
				
				.compression = true,
				.communicate = MWIFI_COMMUNICATE_UNICAST,
			};
			const mlink_httpd_type_t type_L8mesh_cst = {
			
				.format = MLINK_HTTPD_FORMAT_HEX,
			};

			uint8_t dataRequest_temp[32] = {0}; //缓存大小要给够，不然数据传输会出问题

			const uint8_t meshRootAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_ROOT;

			memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

			dataRequest_temp[0] = L8DEV_MESH_CMD_FWARE_CHECK; //mesh转发头命令
			
			memcpy(&dataRequest_temp[1], dataRespond_temp, sizeof(uint8_t) * dataTransLength_limit); //下标0为nesh命令，全部向后挪一个字节

			ret = mwifi_write(meshRootAddr, &data_type, dataRequest_temp, sizeof(uint8_t) * dataTransLength_limit + 1, true);
			MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mdf firewareRsv check res", mdf_err_to_name(ret));
		}
	}
}

void devFireware_upgradeReserveCheck_trigByEvent(void){

	xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_DEVNODE_UPGRADE_CHECK);
}

void mqtt_app_start(void){

//    esp_mqtt_client_config_t mqtt_cfg = {
//        .uri = CONFIG_BROKER_URL,
//        .event_handle = mqtt_event_handler,
//        // .user_context = (void *)your_context
//    };

	stt_mqttCfgParam dtMqttCfg_temp = {0};

	mqttRemoteConnectCfg_paramGet(&dtMqttCfg_temp);
	sprintf(mqttCfgParam_hostIpStr, "%d.%d.%d.%d", dtMqttCfg_temp.ip_remote[0],
												   dtMqttCfg_temp.ip_remote[1],
												   dtMqttCfg_temp.ip_remote[2],
												   dtMqttCfg_temp.ip_remote[3]);
	mqtt_cfg.host = (const char *)mqttCfgParam_hostIpStr;
	mqtt_cfg.port = (uint32_t)dtMqttCfg_temp.port_remote;

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    usrAppClient = esp_mqtt_client_init((const esp_mqtt_client_config_t *)&mqtt_cfg);
    esp_mqtt_client_start(usrAppClient);
}

void mqtt_app_stop(void){

	esp_mqtt_client_stop(usrAppClient);

	remoteMqtt_connectFlg = false;
}






























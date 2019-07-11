#include "dataTrans_remoteServer.h"

#include "os.h"

#include "mlink.h"
#include "mwifi.h"

#include "mdf_common.h"
#include "mqtt_client.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

#include "dataTrans_localHandler.h"

#include "devDriver_manage.h"

#define DEVMQTT_TOPIC_HEAD_A			"lanbon/a/"
#define DEVMQTT_TOPIC_HEAD_B			"lanbon/b/"

#define DEVMQTT_TOPIC_NUM_M2S		20
#define DEVMQTT_TOPIC_NUM_S2M		10
#define DEVMQTT_TOPIC_TEMP_LENGTH	100
#define DEVMQTT_TOPIC_CASE_LENGTH	50
#define DEVMQTT_DATA_RESPOND_LENGTH	100

extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;
extern uint8_t listNum_nodeDevDataManage;

static const char *TAG = "lanbon_L8 - mqttRemote";

static const char cmdTopic_m2s_list[DEVMQTT_TOPIC_NUM_M2S][DEVMQTT_TOPIC_CASE_LENGTH] = {

	"/cmdControl", 
	"/cmdDevLock",
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

	"/cmdWifiChg/overall",

	"/m2s/cmdQuery",
};
enum{

	cmdTopicM2SInsert_cmdControl = 0,
	cmdTopicM2SInsert_cmdDevLock,
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

	cmdTopicM2SInsert_cmdWifiChg_overall,

	cmdTopicM2SInsert_cmdQuery,
};

static const char cmdTopic_s2m_list[DEVMQTT_TOPIC_NUM_S2M][DEVMQTT_TOPIC_CASE_LENGTH] = {

	"/s2m/cmdStatusGet",
	"/s2m/cmdTimerGet/normal", 
	"/s2m/cmdTimerGet/delay",
	"/s2m/cmdTimerGet/greenMode",
	"/s2m/cmdTimerGet/nightMode",
	"/s2m/cmdMutualGet",
};
enum{

	cmdTopicS2MInsert_cmdStatusGet = 0,
	cmdTopicS2MInsert_cmdTimerGet_normal,
	cmdTopicS2MInsert_cmdTimerGet_delay,
	cmdTopicS2MInsert_cmdTimerGet_greenMode,
	cmdTopicS2MInsert_cmdTimerGet_nightMode,
	cmdTopicS2MInsert_cmdMutualGet,
};

static const char mqttTopicSpecial_elecsumReport[DEVMQTT_TOPIC_CASE_LENGTH] = "L8devElecsumReport";

static char devMqtt_topicTemp[DEVMQTT_TOPIC_TEMP_LENGTH] = {0};

static uint8_t dataRespond_temp[DEVMQTT_DATA_RESPOND_LENGTH] = {0};

static esp_mqtt_client_handle_t usrAppClient = NULL;
static bool mqttClientElecsumRepot_reserveFlg = false;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

static const esp_mqtt_client_config_t mqtt_cfg = {

	.transport = MQTT_TRANSPORT_OVER_TCP,
	.host = "112.124.61.191",
	.event_handle = mqtt_event_handler,
	.port = 8888,
	.username = "lanbon",
	.password = "lanbon2019.",
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

	devRouterConnectBssid_Get(devRouterBssid);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);
	memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

	memset(dataRespond_temp, 0, DEVMQTT_DATA_RESPOND_LENGTH); //数据缓存清零
	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零

	switch(cmdTopicM2S_num){

		case cmdTopicM2SInsert_cmdControl:{

			const uint16_t dataComming_lengthLimit = 7; //数据长度最短限制

			if(event->data_len < dataComming_lengthLimit)break;

			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDCONTROL]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				stt_devDataPonitTypedef dataVal_set = {0};
				
				memcpy(&dataVal_set, (uint8_t *)(&event->data[0]), sizeof(uint8_t));
				currentDev_dataPointSet(&dataVal_set, true, true, true);
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

					case countryT_EnglishSerail:{

						memset(dataTextObjDisp_temp.dataBtnTextDisp[objNum], 0, GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE);
						memcpy(dataTextObjDisp_temp.dataBtnTextDisp[objNum], &(event->data[dataTextCodeIstStart]), textDataLen);

					}break;

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
			
			(!memcmp(devSelfMac, &(event->data[MACADDR_INSRT_START_CMDTIMERSET]), DEVICE_MAC_ADDR_APPLICATION_LEN))?(data_sendToRoot_IF = true):(data_sendToRoot_IF = false);
			if(data_sendToRoot_IF){

				if(event->data_len >= (sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM)){

					usrAppActTrigTimer_paramSet((usrApp_trigTimer *)(event->data), true);
				}
			}
			else //数据不是给主机，则转发
			{
				dataRespond_temp[0] = L8DEV_MESH_CMD_SET_TIMER;
				memcpy(&dataRespond_temp[1], &event->data[0], MACADDR_INSRT_START_CMDTIMERSET); //数据内容填充
				
				ret = mwifi_root_write((const uint8_t *)&event->data[MACADDR_INSRT_START_CMDTIMERSET], 1,
									   &data_type, dataRespond_temp, MACADDR_INSRT_START_CMDTIMERSET + 1, true); // +1代表mesh内部传输时添加第一字节为命令字节
				MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret));
			}

		}break;

		case cmdTopicM2SInsert_cmdTimerSet_delay:{

			const uint16_t dataComming_lengthLimit = 9; //数据长度最短限制
			
			if(event->data_len < dataComming_lengthLimit)break;

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
				
					usrAppNightModeTimeTab_paramSet((usrApp_trigTimer *)(event->data), true);
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

					currentDev_dataPointSet((stt_devDataPonitTypedef *)&(scenarioActionParam_unit.opreatVal), true, false, false); //场景操作不触发互控
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
			
			ret = mwifi_root_write(boardcastAddr, 1,
								 &data_type, dataRespond_temp, sizeof(struct stt_paramWifiConfig) + 1, true);  //数据内容填充 -头命令长度1 + 操作值数据长度1
			MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mqtt mwifi_root_write", mdf_err_to_name(ret)); 

			//倒计时重启触发
			usrApplication_systemRestartTrig(5);
			
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

		case cmdTopicM2SInsert_cmdQuery:{
			
			enum{

				cmdQuery_deviceStatus = 0x11,
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

					case cmdQuery_timerNormal:{

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_normal]);

						usrAppActTrigTimer_paramGet((usrApp_trigTimer *)&dataRespond_temp);
						memcpy(&dataRespond_temp[MACADDR_INSRT_START_CMDTIMERSET], devSelfMac, DEVICE_MAC_ADDR_APPLICATION_LEN);
						mqttData_pbLen = MACADDR_INSRT_START_CMDTIMERSET + DEVICE_MAC_ADDR_APPLICATION_LEN;
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

						sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X%s", MAC2STR(devRouterBssid),
																					 				   (char *)cmdTopic_s2m_list[cmdTopicS2MInsert_cmdTimerGet_nightMode]);
						
						usrAppNightModeTimeTab_paramGet((usrApp_trigTimer *)&dataRespond_temp);
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
						printf("mqtt cmdQuery_mutualCtrlInfo respond res:0x%04X.\n", esp_mqtt_client_publish(event->client, devMqtt_topicTemp, (const char*)dataResp_mutualInfo, mqttData_pbLen, 1, 0));
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

			devRouterConnectBssid_Get(devRouterBssid);

			memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_A"MAC:%02X%02X%02X%02X%02X%02X/#", MAC2STR(devRouterBssid));
			msg_id = esp_mqtt_client_subscribe(client, devMqtt_topicTemp, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH);
			sprintf(devMqtt_topicTemp, DEVMQTT_TOPIC_HEAD_B"MAC:%02X%02X%02X%02X%02X%02X/#", MAC2STR(devRouterBssid));
			msg_id = esp_mqtt_client_subscribe(client, devMqtt_topicTemp, 0);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

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
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			printf("DATALEN=%d\r\n", event->data_len);

			uint8_t searchRes = topicM2S_compareResult(event->topic, event->topic_len);
			if(searchRes < DEVMQTT_TOPIC_NUM_M2S){

				mqtt_remoteDataHandler(event, searchRes);				
			}

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }

	mqttClientElecsumRepot_reserveFlg = true;
	
    return ESP_OK;
}

void mqtt_rootDevRemoteDatatransLoop_elecSumReport(void){

	uint8_t *dataReport_devElecsumInfo = NULL;
	uint8_t devUnitNum_temp = 0;
	uint16_t devUnitNumLimit_perPack = (USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH - 1) / sizeof(stt_devUnitElecsumReport); //mqtt单包包含设备信息 数量限制
	uint16_t mqttData_pbLen = 0;

	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT);
	else{

		return; //角色不对，不可用
	}

	if(!remoteMqtt_connectFlg)return; //远程连接不可用

	while(!mqttClientElecsumRepot_reserveFlg)vTaskDelay(1 / portTICK_PERIOD_MS); //等待mqttClient资源可用

	memset(devMqtt_topicTemp, 0, DEVMQTT_TOPIC_TEMP_LENGTH); //主题缓存清零
	sprintf(devMqtt_topicTemp, "%s", (char *)mqttTopicSpecial_elecsumReport);

	dataReport_devElecsumInfo = L8devElecsumInfoGet(listHead_nodeDevDataManage);
	devUnitNum_temp = dataReport_devElecsumInfo[0];
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
		uint16_t dataRespLoadInsert = 1;

		//总信息长度超过一个包则拆分发送 -商包
		if(dataRespLoop){

			for(uint8_t loop = 0; loop < dataRespLoop; loop ++){
			
				dataRespPerPackInfoLen = sizeof(stt_devUnitElecsumReport) * devUnitNumLimit_perPack;
				dataRespPerPackBuff[0] = devUnitNumLimit_perPack;
				memcpy(&dataRespPerPackBuff[1], &dataReport_devElecsumInfo[dataRespLoadInsert], dataRespPerPackInfoLen);
				dataRespLoadInsert += dataRespPerPackInfoLen;
				mqttData_pbLen = dataRespPerPackInfoLen + 1;
				
				printf("mqtt cmdQuery_devElecsumInfo reportLoop%d res:0x%04X.\n", loop, esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
				memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
			}
		}

		//设备信息收尾数据包 -余包
		if(dataRespLastPack_devNum){

			dataRespPerPackInfoLen = sizeof(stt_devUnitElecsumReport) * dataRespLastPack_devNum;
			dataRespPerPackBuff[0] = dataRespLastPack_devNum;
			memcpy(&dataRespPerPackBuff[1], &dataReport_devElecsumInfo[dataRespLoadInsert], dataRespPerPackInfoLen);
			dataRespLoadInsert += dataRespPerPackInfoLen;
			mqttData_pbLen = dataRespPerPackInfoLen + 1;
			printf("mqtt cmdQuery_devElecsumInfo reportTail res:0x%04X.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespPerPackBuff, mqttData_pbLen, 1, 0)); //数据发送
			memset(dataRespPerPackBuff, 0, sizeof(uint8_t) * USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH); //数据发送缓存清空
		}
	}
		
	os_free(dataReport_devElecsumInfo); //设备信息获取缓存 释放
}

void mqtt_remoteDataTrans(uint8_t dtCmd, uint8_t *data, uint16_t dataLen){

	enum{

		cmdQuery_deviceStatus = 0x11,
		cmdQuery_timerNormal = 0xA0,
		cmdQuery_delayTrig,
		cmdQuery_greenMode,
		cmdQuery_nightMode,
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
		
			default:break;
		}
	}

	if(dtCmd_identify){

		memcpy(dataRespond_temp, data, dataLen);
		printf("mqtt normal dataTrans res:%d.\n", esp_mqtt_client_publish(usrAppClient, devMqtt_topicTemp, (const char*)dataRespond_temp, dataLen, 1, 0));
	}
}

void mqtt_app_start(void){

//    esp_mqtt_client_config_t mqtt_cfg = {
//        .uri = CONFIG_BROKER_URL,
//        .event_handle = mqtt_event_handler,
//        // .user_context = (void *)your_context
//    };

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

    usrAppClient = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(usrAppClient);
}

void mqtt_app_stop(void){

	esp_mqtt_client_stop(usrAppClient);

	remoteMqtt_connectFlg = false;
}






























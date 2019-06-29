#include "devDataManage.h"

#include "os.h"

#include "mdf_common.h"
#include "mwifi.h"
#include "mlink.h"

#include "devDriver_manage.h"
#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

LV_IMG_DECLARE(homepageCtrlObjIcon_1size32);
LV_IMG_DECLARE(homepageCtrlObjIcon_2size32);
LV_IMG_DECLARE(homepageCtrlObjIcon_3size32);

LV_IMG_DECLARE(btnIconHome_meeting);
LV_IMG_DECLARE(homepageCtrlObjIcon_1);LV_IMG_DECLARE(homepageCtrlObjIcon_2);LV_IMG_DECLARE(homepageCtrlObjIcon_3);
LV_IMG_DECLARE(homepageCtrlObjIcon_4);LV_IMG_DECLARE(homepageCtrlObjIcon_5);LV_IMG_DECLARE(homepageCtrlObjIcon_6);
LV_IMG_DECLARE(homepageCtrlObjIcon_7);LV_IMG_DECLARE(homepageCtrlObjIcon_8);LV_IMG_DECLARE(homepageCtrlObjIcon_9);
LV_IMG_DECLARE(homepageCtrlObjIcon_10);LV_IMG_DECLARE(homepageCtrlObjIcon_11);LV_IMG_DECLARE(homepageCtrlObjIcon_12);
LV_IMG_DECLARE(homepageCtrlObjIcon_13);LV_IMG_DECLARE(homepageCtrlObjIcon_14);LV_IMG_DECLARE(homepageCtrlObjIcon_15);
LV_IMG_DECLARE(homepageCtrlObjIcon_16);LV_IMG_DECLARE(homepageCtrlObjIcon_17);LV_IMG_DECLARE(homepageCtrlObjIcon_18);
LV_IMG_DECLARE(homepageCtrlObjIcon_19);LV_IMG_DECLARE(homepageCtrlObjIcon_20);LV_IMG_DECLARE(homepageCtrlObjIcon_21);
LV_IMG_DECLARE(homepageCtrlObjIcon_22);
LV_IMG_DECLARE(homepageCtrlObjIcon2_1);LV_IMG_DECLARE(homepageCtrlObjIcon2_2);LV_IMG_DECLARE(homepageCtrlObjIcon2_3);
LV_IMG_DECLARE(homepageCtrlObjIcon2_4);LV_IMG_DECLARE(homepageCtrlObjIcon2_5);LV_IMG_DECLARE(homepageCtrlObjIcon2_6);
LV_IMG_DECLARE(homepageCtrlObjIcon2_7);LV_IMG_DECLARE(homepageCtrlObjIcon2_8);LV_IMG_DECLARE(homepageCtrlObjIcon2_9);
LV_IMG_DECLARE(homepageCtrlObjIcon2_10);LV_IMG_DECLARE(homepageCtrlObjIcon2_11);LV_IMG_DECLARE(homepageCtrlObjIcon2_12);
LV_IMG_DECLARE(homepageCtrlObjIcon3_1);LV_IMG_DECLARE(homepageCtrlObjIcon3_2);LV_IMG_DECLARE(homepageCtrlObjIcon3_3);
LV_IMG_DECLARE(homepageCtrlObjIcon3_4);LV_IMG_DECLARE(homepageCtrlObjIcon3_5);LV_IMG_DECLARE(homepageCtrlObjIcon3_6);
LV_IMG_DECLARE(homepageCtrlObjIcon3_7);LV_IMG_DECLARE(homepageCtrlObjIcon3_8);

stt_nodeDev_hbDataManage *listHead_nodeDevDataManage = NULL;
uint8_t listNum_nodeDevDataManage = 0;
uint8_t devRunningTimeFromPowerUp_couter = 0; //设备启动时间 计时变量
uint8_t devRestartDelay_counter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8; //设备重启，倒计时延时执行时间

xQueueHandle msgQh_dataManagementHandle = NULL; //用于通知数据已被修改成功

//uint8_t *dataPtr_btnTextImg_sw_A = NULL;
//uint8_t *dataPtr_btnTextImg_sw_B = NULL;
//uint8_t *dataPtr_btnTextImg_sw_C = NULL;

static void devGuiBussinessHome_btnTextPic_save(uint8_t picIst, uint8_t *picData);

static bool usrAppOpreation_nvsFlashOpen_flg = false;

static const char *TAG = "lanbon_L8 - dataManage";

static const char *NVS_DATA_L8_PARTITION_NAME		= "L8_devDataRcord";

static const char *NVS_DATA_SYSINFO_RECORD			= "devSys_Info";   //nvs条目描述字符串限长16

static const char *DATA_SWSTATUS					= "devStatus";
static const char *DATA_DEV_STATUS_RECORD_IF		= "devStatusRecord";
static const char *DATA_INFO_TIMER_NORMAL			= "devTimerNormal";
static const char *DATA_INFO_TIMER_NIGHTMODE		= "devTimerMnight";
static const char *DATA_INFO_TIMER_GREENMODE		= "devTimerMgreen";
static const char *DATA_INFO_DEVRUNNINGFLG			= "devRunningFlg";
static const char *DATA_TIMEZONE					= "devTimeZone";
static const char *DATA_DEV_TYPEDEF					= "devTypeDef";
static const char *DATA_DEV_ROUTER_BSSID			= "devRouterBssid";
static const char *DATA_DEV_MUTUALCTRL_INFO			= "devMutualInfo";
static const char *DATA_DEV_GUIHOMEBTNTEXTDISP		= "devBtnTextDisp";
static const char *DATA_DEV_GUIHOMEBTNICONDISP		= "devBtnIconDisp";
static const char *DATA_DEV_GUIHOMEBTNTEXTPIC_A		= "devBtnTextPicA";
static const char *DATA_DEV_GUIHOMEBTNTEXTPIC_B		= "devBtnTextPicB";
static const char *DATA_DEV_GUIHOMEBTNTEXTPIC_C		= "devBtnTextPicC";
static const char *DATA_DEV_GUIHOMETHEMETYPE		= "devThemeType";
static const char *DATA_DEVCURTAIN_RUNNINGPARAM		= "devCurtainParam";
static const char *DATA_DEVDRVIPT_RECALIBRAPARAM	= "devDrviptParam";
static const char *DATA_DEVSCENARIO_DATA_PARAM_0	= "devScenDats_0";
static const char *DATA_DEVSCENARIO_DATA_PARAM_1	= "devScenDats_1";
static const char *DATA_DEVSCENARIO_DATA_PARAM_2	= "devScenDats_2";

static stt_dataDisp_guiBussinessHome_btnText dataBtnTextObjDisp_bussinessHome = {

	.countryFlg = countryT_Arabic,
	.dataBtnTextDisp[0] = "غرفة الاجتماعاتغرفة الاجتماعاتغرفة الاجتماعات",
	.dataBtnTextDisp[1] = "غرفة النوم",
	.dataBtnTextDisp[2] = "مرحاض",
};
static uint8_t dataBtnIconNumObjDisp_bussinessHome[GUIBUSSINESS_CTRLOBJ_MAX_NUM] = {12, 21, 31};

static bool listNodeDevOpreating_Flg = false; //子节点链表是否正在被进行管理操作

static bool meshNetworkParamReserve_Flg = false;

static int8_t devSignalStrength2ParentVal = -127; //设备信号强度
static uint16_t devMesh_currentNodeNum = 0; //当前所在mesh网络内节点数量

static bool devRouterOrMeshConnect_flg = false;

static uint16_t devCurrentRunningFlg = 0;

static uint8_t routerConnect_bssid[6] = {0};

static stt_devMutualGroupParam devMutualCtrl_group[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};

static char devIptdrvParam_recalibration = 'N';

uint8_t systemDevice_startUpTime_get(void){

	return devRunningTimeFromPowerUp_couter;
}

void gui_bussinessHome_btnText_dataReales(uint8_t picIst, uint8_t *picDataBuf, uint8_t *dataLoad, uint16_t dataLoad_len, uint8_t dataBagIst, bool lastFrame_If){

	if(picDataBuf == NULL)return;

	printf("btnText_dataReales.\n");

	const uint16_t dataIstBase = MQTT_REMOTE_DATATRANS_PIC_DATABAG_LENGTH / 4 * 3 * (uint16_t)dataBagIst;
	const uint16_t dataOpUnit = 4;
	uint16_t opLoop = 0;
	struct _pixelRGB_discr{

		uint16_t pData_B:5;
		uint16_t pData_G:6;		
		uint16_t pData_R:5;
	}pixelRGB565_opUnitTemp = {0};

	if(dataIstBase > GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * 3)return;

	for(opLoop = 0; opLoop < (dataLoad_len / dataOpUnit); opLoop ++){

		picDataBuf[dataIstBase + (3 * opLoop) + 2] = dataLoad[(dataOpUnit * opLoop) + 3];
		memset(&pixelRGB565_opUnitTemp, 0xff, sizeof(uint16_t));
		pixelRGB565_opUnitTemp.pData_R = dataLoad[(dataOpUnit * opLoop) + 2] / 8; //R:5bit
		pixelRGB565_opUnitTemp.pData_G = dataLoad[(dataOpUnit * opLoop) + 1] / 4; //G:6bit
		pixelRGB565_opUnitTemp.pData_B = dataLoad[(dataOpUnit * opLoop) + 0] / 8; //B:5bit
		memcpy(&picDataBuf[dataIstBase + (3 * opLoop)], &pixelRGB565_opUnitTemp, sizeof(uint16_t));
	}

	if(lastFrame_If){

		printf("last frame trig picSave.\n");
		devGuiBussinessHome_btnTextPic_save(picIst, picDataBuf);
	}
}

void usrAppHomepageBtnTextDisp_paramSet(stt_dataDisp_guiBussinessHome_btnText *param, bool nvsRecord_IF){

	stt_msgDats_dataManagementHandle sptr_msgQ_dmHandle = {0};

	sptr_msgQ_dmHandle.msgType = dataManagement_msgType_homePageCtrlObjTextChg;
	for(uint8_t loop = 0; loop < GUIBUSSINESS_CTRLOBJ_MAX_NUM; loop ++){ //缓存更新前作比较

		if(memcmp(param->dataBtnTextDisp[loop], dataBtnTextObjDisp_bussinessHome.dataBtnTextDisp[loop], sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE))
			sptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjTextChg.objChg_bitHold |= (1 << loop);
	}
	xQueueSend(msgQh_dataManagementHandle, &sptr_msgQ_dmHandle, 1 / portTICK_PERIOD_MS);

	memcpy(&dataBtnTextObjDisp_bussinessHome, param, sizeof(stt_dataDisp_guiBussinessHome_btnText));
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_btnTextDisp, &dataBtnTextObjDisp_bussinessHome);
}

void usrAppHomepageBtnTextDisp_paramGet(stt_dataDisp_guiBussinessHome_btnText *param){

	memcpy(param, &dataBtnTextObjDisp_bussinessHome, sizeof(stt_dataDisp_guiBussinessHome_btnText));
}

uint8_t countryFlgGetByAbbre(char countryAbbre[DATAMANAGE_LANGUAGE_ABBRE_MAXLEN]){

	const char countryAbbre_tab[DATAMANAGE_LANGUAGE_TAB_MAXNUM][DATAMANAGE_LANGUAGE_ABBRE_MAXLEN] = {

		"en", // 00 英语
		"ar", // 01 阿拉伯语
		"de", // 02 德语
		"es", // 03 西班牙
		"fr", // 04 法文
		"it", // 05 意大利
		"ja", // 06 日语
		"pl", // 07 波兰文
		"pt", // 08 葡萄牙语
		"th", // 09 泰文
		"tr", // 10 土耳其
		"iw", // 11 希伯来语
		"ru", // 12 俄文
		"vi", // 13 越南语
	};
	uint8_t res_flg = countryT_EnglishSerail;

	for(uint8_t loop = 0; loop < DATAMANAGE_LANGUAGE_TAB_MAXNUM; loop ++){

		if(!strcmp(countryAbbre_tab[loop], countryAbbre)){

			res_flg = loop;
			break;
		}
	}

	switch(res_flg){

		case 1:{

			res_flg = countryT_Arabic;

		}break;
	
		case 11:{

			res_flg = countryT_Hebrew;

		}break;

		default:{

			res_flg = countryT_EnglishSerail;
		
		}break;
	}

	return res_flg;
}

void usrAppHomepageBtnIconNumDisp_paramSet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM], bool nvsRecord_IF){

	stt_msgDats_dataManagementHandle sptr_msgQ_dmHandle = {0};

	sptr_msgQ_dmHandle.msgType = dataManagement_msgType_homePageCtrlObjIconChg;
	for(uint8_t loop = 0; loop < GUIBUSSINESS_CTRLOBJ_MAX_NUM; loop ++){ //缓存更新前作比较

		if(param[loop] != dataBtnIconNumObjDisp_bussinessHome[loop])
			sptr_msgQ_dmHandle.msgData_dmHandle.dataAb_hpCtrlObjIconChg.objChg_bitHold |= (1 << loop);
	}
	xQueueSend(msgQh_dataManagementHandle, &sptr_msgQ_dmHandle, 1 / portTICK_PERIOD_MS);

	memcpy(dataBtnIconNumObjDisp_bussinessHome, param, sizeof(uint8_t) * GUIBUSSINESS_CTRLOBJ_MAX_NUM);
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_btnIconDisp, dataBtnIconNumObjDisp_bussinessHome);
}

void usrAppHomepageBtnIconNumDisp_paramGet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM]){

	memcpy(param, dataBtnIconNumObjDisp_bussinessHome, sizeof(uint8_t) * GUIBUSSINESS_CTRLOBJ_MAX_NUM);
}

lv_img_dsc_t *usrAppHomepageBtnIconDisp_dataGet(uint8_t iconNum){

	const lv_img_dsc_t *res = NULL;

	switch(iconNum){

		case 1:		res = &homepageCtrlObjIcon_1; 	break;
		case 2:		res = &homepageCtrlObjIcon_2; 	break;
		case 3:		res = &homepageCtrlObjIcon_3; 	break;
		case 4:		res = &homepageCtrlObjIcon_4; 	break;
		case 5:		res = &homepageCtrlObjIcon_5; 	break;
		case 6:		res = &homepageCtrlObjIcon_6; 	break;
		case 7:		res = &homepageCtrlObjIcon_7; 	break;
		case 8:		res = &homepageCtrlObjIcon_8; 	break;
		case 9:		res = &homepageCtrlObjIcon_9; 	break;
		case 10:	res = &homepageCtrlObjIcon_10; 	break;
		case 11:	res = &homepageCtrlObjIcon_11; 	break;
		case 12:	res = &homepageCtrlObjIcon_12; 	break;
		case 13:	res = &homepageCtrlObjIcon_13; 	break;
		case 14:	res = &homepageCtrlObjIcon_14; 	break;
		case 15:	res = &homepageCtrlObjIcon_15; 	break;
		case 16:	res = &homepageCtrlObjIcon_16; 	break;
		case 17:	res = &homepageCtrlObjIcon_17; 	break;
		case 18:	res = &homepageCtrlObjIcon_18; 	break;
		case 19:	res = &homepageCtrlObjIcon_19; 	break;
		case 20:	res = &homepageCtrlObjIcon_20; 	break;
		case 21:	res = &homepageCtrlObjIcon_21; 	break;
		case 22:	res = &homepageCtrlObjIcon_22; 	break;

		case 23:	res = &homepageCtrlObjIcon2_1; 	break;
		case 24:	res = &homepageCtrlObjIcon2_2; 	break;
		case 25:	res = &homepageCtrlObjIcon2_3; 	break;
		case 26:	res = &homepageCtrlObjIcon2_4; 	break;
		case 27:	res = &homepageCtrlObjIcon2_5; 	break;
		case 28:	res = &homepageCtrlObjIcon2_6; 	break;
		case 29:	res = &homepageCtrlObjIcon2_7; 	break;
		case 30:	res = &homepageCtrlObjIcon2_8; 	break;
		case 31:	res = &homepageCtrlObjIcon2_9; 	break;
		case 32:	res = &homepageCtrlObjIcon2_10; break;
		case 33:	res = &homepageCtrlObjIcon2_11; break;
		case 34:	res = &homepageCtrlObjIcon2_12; break;

		case 35:	res = &homepageCtrlObjIcon3_1; 	break;
		case 36:	res = &homepageCtrlObjIcon3_2; 	break;
		case 37:	res = &homepageCtrlObjIcon3_3; 	break;
		case 38:	res = &homepageCtrlObjIcon3_4; 	break;
		case 39:	res = &homepageCtrlObjIcon3_5; 	break;
		case 40:	res = &homepageCtrlObjIcon3_6; 	break;
		case 41:	res = &homepageCtrlObjIcon3_7; 	break;
		case 42:	res = &homepageCtrlObjIcon3_8; 	break;
		
		default:res = &btnIconHome_meeting; break;
	}

	return res;
}

void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if){

	(reCalibra_if)?
		(devIptdrvParam_recalibration = 'Y'):
		(devIptdrvParam_recalibration = 'N');

	devSystemInfoLocalRecord_save(saveObj_devDriver_iptRecalibration_set, &devIptdrvParam_recalibration);
}

bool usrApp_devIptdrv_paramRecalibration_get(void){

	bool res = false;

	(devIptdrvParam_recalibration == 'Y')?
		(res = true):
		(res = false);

	return res;
}

void meshNetwork_connectReserve_IF_set(bool param){

	meshNetworkParamReserve_Flg = param;
}

bool meshNetwork_connectReserve_IF_get(void){

	return meshNetworkParamReserve_Flg;
}

void flgSet_gotRouterOrMeshConnect(bool valSet){

	devRouterOrMeshConnect_flg = valSet;
}

bool flgGet_gotRouterOrMeshConnect(void){

	return devRouterOrMeshConnect_flg;
}

void devRouterConnectBssid_Set(uint8_t bssid[6], bool nvsRecord_IF){

	memcpy(routerConnect_bssid, bssid, 6);
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_routerBssid, routerConnect_bssid);
}

void devRouterConnectBssid_Get(uint8_t bssid[6]){

	memcpy(bssid, routerConnect_bssid, 6);
}

void devMeshSignalVal_Reales(void){

	mesh_assoc_t mesh_assoc = {0x0};

	if(mwifi_is_connected()){

		esp_wifi_vnd_mesh_get(&mesh_assoc);

		devSignalStrength2ParentVal = mesh_assoc.rssi;
	}
	else
	{
		devSignalStrength2ParentVal = -127;
	}
}

int8_t devMeshSignalFromParentVal_Get(void){

	return devSignalStrength2ParentVal;
}

void devMeshNodeNum_Reales(void){

	devMesh_currentNodeNum = (uint16_t)esp_mesh_get_total_node_num();
}

uint16_t devMeshNodeNum_Get(void){

	return devMesh_currentNodeNum;
}

static void devGuiBussinessHome_btnTextPic_save(uint8_t picIst, uint8_t *picData){

	switch(picIst){

		case 0:{

			devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_btnTextPic_A, picData);

		}break;

		case 1:{

			devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_btnTextPic_B, picData);

		}break;

		case 2:{

			devSystemInfoLocalRecord_save(saveObj_devGuiBussinessHome_btnTextPic_C, picData);

		}break;

		default:break;
	}
}

void devMutualCtrlGroupInfo_Set(stt_devMutualGroupParam *mutualGroupParam, uint8_t opreatBit, bool nvsRecord_IF){

	if(opreatBit & (1 << 0))memcpy(&devMutualCtrl_group[0], mutualGroupParam, sizeof(stt_devMutualGroupParam));
	if(opreatBit & (1 << 1))memcpy(&devMutualCtrl_group[1], mutualGroupParam, sizeof(stt_devMutualGroupParam));
	if(opreatBit & (1 << 2))memcpy(&devMutualCtrl_group[2], mutualGroupParam, sizeof(stt_devMutualGroupParam));
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_mutualCtrlInfo, devMutualCtrl_group);
}

void devMutualCtrlGroupInfo_Get(stt_devMutualGroupParam mutualGroupParam[DEVICE_MUTUAL_CTRL_GROUP_NUM]){

	memcpy(mutualGroupParam, devMutualCtrl_group, sizeof(stt_devMutualGroupParam) * DEVICE_MUTUAL_CTRL_GROUP_NUM);
}

void devMutualCtrlGroupInfo_groupInsertGet(uint8_t groupInsert[DEVICE_MUTUAL_CTRL_GROUP_NUM]){

	for(uint8_t loop = 0; loop < DEVICE_MUTUAL_CTRL_GROUP_NUM; loop ++){

		groupInsert[loop] = devMutualCtrl_group[loop].mutualCtrlGroup_insert;
	}
}

bool devMutualCtrlGroupInfo_unitCheckByInsert(stt_devMutualGroupParam *mutualGroupParamUnit, uint8_t *groupNum, uint8_t paramInsert){

	bool infoGet_res = false;

	if((paramInsert == DEVICE_MUTUALGROUP_INVALID_INSERT_A) ||
	   (paramInsert == DEVICE_MUTUALGROUP_INVALID_INSERT_B)){

		infoGet_res = false;
	}
	else
	{
		for(uint8_t loop = 0; loop < DEVICE_MUTUAL_CTRL_GROUP_NUM; loop ++){
		
			if(devMutualCtrl_group[loop].mutualCtrlGroup_insert == paramInsert){
		
				memcpy(mutualGroupParamUnit, &devMutualCtrl_group[loop], sizeof(stt_devMutualGroupParam));
				*groupNum = loop;
				infoGet_res = true;
			}
		}
	}

	return infoGet_res;
}

uint16_t currentDevRunningFlg_paramGet(void){

	return devCurrentRunningFlg;
}

void currentDevRunningFlg_paramSet(uint16_t valFlg, bool nvsRecord_IF){

	devCurrentRunningFlg = valFlg;
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devRunning_flg, &devCurrentRunningFlg);
}

bool usrAppMethod_mwifiMacAddrRemoveFromList(uint8_t *macAddrList, uint8_t macAddrList_num, uint8_t macAddrRemove[MWIFI_ADDR_LEN]){

	uint8_t loop = 0;
	bool res_macAddrSearch = false;

	for(loop = 0; loop < macAddrList_num; loop ++){

		if(!res_macAddrSearch){

			if(!memcmp(&macAddrList[loop * MWIFI_ADDR_LEN], macAddrRemove, MWIFI_ADDR_LEN)){

				memset(&macAddrList[loop * MWIFI_ADDR_LEN], 0, MWIFI_ADDR_LEN);
				res_macAddrSearch = true;
			}
		}
		else
		{
			memcpy(&macAddrList[(loop - 1) * MWIFI_ADDR_LEN], &macAddrList[loop * MWIFI_ADDR_LEN], MWIFI_ADDR_LEN);
		}
	}

	if(res_macAddrSearch)memset(&macAddrList[(macAddrList_num - 1) * MWIFI_ADDR_LEN], 0, MWIFI_ADDR_LEN);

	return res_macAddrSearch;
}

stt_blufiConfigDevInfo_resp *devBlufiConfig_respInfoGet(void){

	stt_blufiConfigDevInfo_resp *devInfo = os_zalloc(sizeof(stt_blufiConfigDevInfo_resp));
	stt_devDataPonitTypedef devDataPoint = {0};

	currentDev_dataPointGet(&devDataPoint);

	devInfo->devTypedef = currentDev_typeGet();
	memcpy(&(devInfo->devStatus_valCurrent), &devDataPoint, sizeof(stt_devDataPonitTypedef)); //开关状态
	devInfo->devVersion = L8_DEVICE_VERSION; //版本
	devDriverBussiness_temperatureMeasure_getByHex(&(devInfo->devFuncInfo_temprature)); //温度
	devDriverBussiness_elecMeasure_valPowerGetByHex(&(devInfo->devFuncInfo_elecPower)); //功率

	printf("blufi config dataResp length:%d.\n", sizeof(stt_blufiConfigDevInfo_resp));

	return devInfo;
}

void L8devHbDataManageList_delSame(stt_nodeDev_hbDataManage *pHead){

    stt_nodeDev_hbDataManage *p,*q,*r;
    p = pHead->next; 

	listNodeDevOpreating_Flg = true;
	
    while(p != NULL)    
    {
        q = p;
        while(q->next != NULL) 
        {
            if(!memcmp(q->next->dataManage.nodeDev_Mac, p->dataManage.nodeDev_Mac, MWIFI_ADDR_LEN)) 
            {
                r = q->next; 
                q->next = r->next;   

				memcpy(&(q->dataManage), &(r->dataManage), sizeof(stt_hbDataUpload)); //后到优先更新
				q->nodeDevKeepAlive_counter = r->nodeDevKeepAlive_counter;
				
				free(r);
            }
            else
            {
            	q = q->next;
			}
        }

        p = p->next;
    }

	listNodeDevOpreating_Flg = false;
}

uint8_t L8devHbDataManageList_nodeNumDetect(stt_nodeDev_hbDataManage *pHead){

	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow;
	uint8_t loop = 0;

	listNodeDevOpreating_Flg = true;

	while(pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;

		loop ++;
	}

	listNodeDevOpreating_Flg = false;

	return loop;
}

uint8_t *L8devHbDataManageList_listGet(stt_nodeDev_hbDataManage *pHead){

	uint8_t listLen = L8devHbDataManageList_nodeNumDetect(pHead) + 1;
	uint8_t *listInfo = (uint8_t *)os_zalloc(sizeof(stt_hbDataUpload) * listLen);
	stt_nodeDev_hbDataManage *pAbove = pHead;
	uint8_t loop = 1;

	listNodeDevOpreating_Flg = true;

	listInfo[0] = listLen;
	memset(listInfo, 0, sizeof(stt_hbDataUpload) * listLen);

	while(pAbove->next != NULL){

		memcpy(&listInfo[sizeof(stt_hbDataUpload) * loop + 1], &(pAbove->next->dataManage), sizeof(stt_hbDataUpload));
		pAbove = pAbove->next;
		loop ++;
	}

	listNodeDevOpreating_Flg = false;

	return listInfo;
}

stt_mutualCtrlInfoResp *L8devMutualCtrlInfo_Get(stt_nodeDev_hbDataManage *pHead, uint8_t mutualCtrlGroupIst){

	stt_mutualCtrlInfoResp *devMutualInfo = (stt_mutualCtrlInfoResp *)os_zalloc(sizeof(stt_mutualCtrlInfoResp));
	uint8_t devMutualGroupIst_self[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	uint8_t devSelfMac[MWIFI_ADDR_LEN] = {0};
	stt_nodeDev_hbDataManage *pAbove = pHead;
	uint8_t loopCount = 0;

	if(devRunningTimeFromPowerUp_couter <= L8_DEV_LISTMANAGE_REALES_CONFIRM){
	
		devMutualInfo->devNum = DEVLIST_MANAGE_LISTNUM_MASK_NULL; //数量掩码，链表数据尚未就绪
	
		return devMutualInfo;
	}

	listNodeDevOpreating_Flg = true;

	esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);
	devMutualCtrlGroupInfo_groupInsertGet(devMutualGroupIst_self); //自身互控获取处理
	for(uint8_t loopA = 0; loopA < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopA ++){
	
		if(devMutualGroupIst_self[loopA] == mutualCtrlGroupIst){

			devMutualInfo->devNum = loopCount + 1;
			memcpy(devMutualInfo->unitDevMutualInfo[loopCount].devUnitMAC, 
				   devSelfMac,
				   MWIFI_ADDR_LEN * sizeof(uint8_t));
			devMutualInfo->unitDevMutualInfo[loopCount].devMutualBitIst = (1 << loopA);
			
			loopCount ++;
		}
	}	

	while(pAbove->next != NULL){

		for(uint8_t loopA = 0; loopA < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopA ++){

			if(pAbove->dataManage.nodeDev_mautualInfo[loopA] == mutualCtrlGroupIst){

				devMutualInfo->devNum = loopCount + 1;
				memcpy(devMutualInfo->unitDevMutualInfo[loopCount].devUnitMAC, 
					   pAbove->dataManage.nodeDev_Mac,
					   MWIFI_ADDR_LEN * sizeof(uint8_t));
				devMutualInfo->unitDevMutualInfo[loopCount].devMutualBitIst = (1 << loopA);

				loopCount ++;
			}
		}		
		
		pAbove = pAbove->next;

		if(loopCount >= MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP)break;
	}

	listNodeDevOpreating_Flg = false;

	return devMutualInfo; //谨记释放内存
}

uint8_t *L8devStatusInfoGet(stt_nodeDev_hbDataManage *pHead){ //仅获取链表单位子节点信息内的 除了 互控信息 以外的其他属性

	uint8_t *devStatusInfo = NULL;

	if(devRunningTimeFromPowerUp_couter <= L8_DEV_LISTMANAGE_REALES_CONFIRM){

		devStatusInfo = (uint8_t *)os_zalloc(sizeof(uint8_t) * 1);
		*devStatusInfo = DEVLIST_MANAGE_LISTNUM_MASK_NULL;

		return devStatusInfo;
	
	}else{

		uint8_t devList_num = (uint8_t)esp_mesh_get_total_node_num();
		uint8_t loopCount = 0;
		stt_nodeDev_hbDataManage *pAbove = pHead;
		stt_devStatusInfoResp statusInfo_dataUnitTemp = {0};
		uint8_t devSelfMac[MWIFI_ADDR_LEN] = {0};

		while(listNodeDevOpreating_Flg)vTaskDelay(1 / portTICK_PERIOD_MS);		
		listNodeDevOpreating_Flg = true;

		devStatusInfo = (uint8_t *)os_zalloc((sizeof(uint8_t) * 1) + (sizeof(stt_devStatusInfoResp) * (devList_num + 1))); //设备数量描述占1 Byte，本身设备状态信息占对应结构体 size Byte
		
		esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);

		//本机状态信息填装
		memcpy(statusInfo_dataUnitTemp.nodeDev_Mac, 
			   devSelfMac,
			   sizeof(uint8_t) * MWIFI_ADDR_LEN);
		currentDev_dataPointGet((stt_devDataPonitTypedef *)&(statusInfo_dataUnitTemp.nodeDev_Status));
		statusInfo_dataUnitTemp.nodeDev_Type = (uint8_t)currentDev_typeGet();
		statusInfo_dataUnitTemp.nodeDev_DevRunningFlg = currentDevRunningFlg_paramGet();
		devDriverBussiness_temperatureMeasure_getByHex(&(statusInfo_dataUnitTemp.nodeDev_dataTemprature));
		devDriverBussiness_elecMeasure_valPowerGetByHex(&(statusInfo_dataUnitTemp.nodeDev_dataPower));
		
		memcpy(&devStatusInfo[sizeof(stt_devStatusInfoResp) * loopCount + 1], //下标0为设备数量，所以从下标为 loopCount+1
			   &statusInfo_dataUnitTemp, 
			   sizeof(stt_devStatusInfoResp));

		loopCount ++;

		printf("mqtt statusQuery rootDev type:%02X.\n", statusInfo_dataUnitTemp.nodeDev_Type);

		//子设备状态信息填装
		while(pAbove->next != NULL){

			//单位缓存清空
			memset(&statusInfo_dataUnitTemp, 0, sizeof(stt_devStatusInfoResp));

			//单位缓存填装
			memcpy(statusInfo_dataUnitTemp.nodeDev_Mac, 
				   pAbove->next->dataManage.nodeDev_Mac,
				   sizeof(uint8_t) * MWIFI_ADDR_LEN);
			statusInfo_dataUnitTemp.nodeDev_Type = pAbove->next->dataManage.nodeDev_Type;
			statusInfo_dataUnitTemp.nodeDev_Status = pAbove->next->dataManage.nodeDev_Status;
			memcpy(&(statusInfo_dataUnitTemp.nodeDev_dataTemprature), 
				   &(pAbove->next->dataManage.nodeDev_dataTemprature),
				   sizeof(stt_devTempParam2Hex));
			memcpy(&(statusInfo_dataUnitTemp.nodeDev_dataPower), 
				   &(pAbove->next->dataManage.nodeDev_dataPower),
				   sizeof(stt_devPowerParam2Hex));
			
			//单位缓存填装进总数据队列缓存
			memcpy(&devStatusInfo[sizeof(stt_devStatusInfoResp) * loopCount + 1], //下标0为设备数量，所以从下标为 loopCount+1
				   &statusInfo_dataUnitTemp, 
				   sizeof(stt_devStatusInfoResp));

			pAbove = pAbove->next;
			loopCount ++;
		}

		devStatusInfo[0] = loopCount;

		listNodeDevOpreating_Flg = false;

		return devStatusInfo; //谨记释放内存
	}
}

uint8_t *L8devElecsumInfoGet(stt_nodeDev_hbDataManage *pHead){

	uint8_t *devElecsumInfo = NULL;

	if(devRunningTimeFromPowerUp_couter <= L8_DEV_LISTMANAGE_REALES_CONFIRM){

		devElecsumInfo= (uint8_t *)os_zalloc(sizeof(uint8_t) * 1);
		*devElecsumInfo = DEVLIST_MANAGE_LISTNUM_MASK_NULL;

		return devElecsumInfo;

	}else{

		uint8_t devList_num = (uint8_t)esp_mesh_get_total_node_num();
		uint8_t loopCount = 0;
		stt_nodeDev_hbDataManage *pAbove = pHead;
		stt_devUnitElecsumReport elecsumInfo_dataUnitTemp = {0};
		uint8_t devSelfMac[MWIFI_ADDR_LEN] = {0};

		while(listNodeDevOpreating_Flg)vTaskDelay(1 / portTICK_PERIOD_MS);		
		listNodeDevOpreating_Flg = true;

		devElecsumInfo = (uint8_t *)os_zalloc((sizeof(uint8_t) * 1) + (sizeof(stt_devUnitElecsumReport) * (devList_num + 1))); //设备数量描述占1 Byte，本身设备状态信息占对应结构体 size Byte

		esp_wifi_get_mac(ESP_IF_WIFI_STA, devSelfMac);

		//本机状态信息填装
		memcpy(elecsumInfo_dataUnitTemp.nodeDev_Mac, 
			   devSelfMac,
			   sizeof(uint8_t) * MWIFI_ADDR_LEN);
		devDriverBussiness_elecMeasure_valElecsumGetByHex(&(elecsumInfo_dataUnitTemp.nodeDev_dataElecsum));
		
		memcpy(&devElecsumInfo[sizeof(stt_devStatusInfoResp) * loopCount + 1], //下标0为设备数量，所以从下标为 loopCount+1
			   &elecsumInfo_dataUnitTemp, 
			   sizeof(stt_devUnitElecsumReport));

		loopCount ++;

		//子设备状态信息填装
		while(pAbove->next != NULL){

			//单位缓存清空
			memset(&elecsumInfo_dataUnitTemp, 0, sizeof(stt_devUnitElecsumReport));

			//单位缓存填装
			memcpy(elecsumInfo_dataUnitTemp.nodeDev_Mac, 
				   pAbove->next->dataManage.nodeDev_Mac,
				   sizeof(uint8_t) * MWIFI_ADDR_LEN);
			memcpy(&(elecsumInfo_dataUnitTemp.nodeDev_dataElecsum),
				   &(pAbove->next->dataManage.nodeDev_dataElecsum),
				   sizeof(stt_devUnitElecsumReport));
			
			//单位缓存填装进总数据队列缓存
			memcpy(&devElecsumInfo[sizeof(stt_devStatusInfoResp) * loopCount + 1], //下标0为设备数量，所以从下标为 loopCount+1
				   &elecsumInfo_dataUnitTemp, 
				   sizeof(stt_devUnitElecsumReport));

			pAbove = pAbove->next;
			loopCount ++;
		}

		devElecsumInfo[0] = loopCount;

		listNodeDevOpreating_Flg = false;

		return devElecsumInfo;
	}
}

void L8devHbDataManageList_listDestory(stt_nodeDev_hbDataManage *pHead){

	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow = pAbove;

	stt_nodeDev_hbDataManage *pTemp = NULL;

	listNodeDevOpreating_Flg = true;

	while(pAbove->next != NULL){

		pTemp 	= pAbove->next;
		pFollow->next = pTemp->next;
		free(pTemp);
		pAbove 	= pFollow;
		pFollow = pAbove;
	}

	listNodeDevOpreating_Flg = false;
}

uint8_t L8devHbDataManageList_nodeCreat(stt_nodeDev_hbDataManage *pHead, stt_nodeDev_hbDataManage *pNew){
	
	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow = NULL;
	uint8_t nCount = 0;

	listNodeDevOpreating_Flg = true;
	
	stt_nodeDev_hbDataManage *pNew_temp = (stt_nodeDev_hbDataManage *) os_zalloc(sizeof(stt_nodeDev_hbDataManage));
	memcpy(&(pNew_temp->dataManage), &(pNew->dataManage), sizeof(stt_hbDataUpload));
	pNew_temp->nodeDevKeepAlive_counter = pNew->nodeDevKeepAlive_counter;
	pNew_temp->next = NULL;
	
	while(pAbove->next != NULL){
	
		pFollow = pAbove;
		pAbove	= pFollow->next;

		nCount ++;
	}
	
	pAbove->next = pNew_temp;

	listNodeDevOpreating_Flg = false;
	
	return ++nCount;
}

stt_nodeDev_hbDataManage *L8devHbDataManageList_nodeGet(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method){	//method = 1,源节点地址返回，操作返回内存影响源节点信息; method = 0,映射信息地址返回，操作返回内存，不影响源节点信息.
	
	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow = NULL;
	
	stt_nodeDev_hbDataManage *pTemp = (stt_nodeDev_hbDataManage *)os_zalloc(sizeof(stt_nodeDev_hbDataManage));
	pTemp->next = NULL;

	listNodeDevOpreating_Flg = true;
	
	while(!(!memcmp(pAbove->dataManage.nodeDev_Mac, nodeDev_Mac, MWIFI_ADDR_LEN)) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(!memcmp(pAbove->dataManage.nodeDev_Mac, nodeDev_Mac, MWIFI_ADDR_LEN)){
		
		if(!method){
			
			memcpy(&(pTemp->dataManage), &(pAbove->dataManage), sizeof(stt_hbDataUpload));
			pTemp->nodeDevKeepAlive_counter = pAbove->nodeDevKeepAlive_counter;
		
		}else{
			
			free(pTemp);
			pTemp = pAbove;	
		}

		listNodeDevOpreating_Flg = false;
		
		return pTemp;
		
	}else{
		
		free(pTemp);

		listNodeDevOpreating_Flg = false;
		
		return NULL;
	}	
} 

bool L8devHbDataManageList_nodeRemove(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]){
	
	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow = NULL;
	
	stt_nodeDev_hbDataManage *pTemp = NULL;

	listNodeDevOpreating_Flg = true;
	
	while(!(!memcmp(pAbove->dataManage.nodeDev_Mac, nodeDev_Mac, MWIFI_ADDR_LEN)) && pAbove->next != NULL){
		
		pFollow = pAbove;
		pAbove	= pFollow->next;
	}
	
	if(!memcmp(pAbove->dataManage.nodeDev_Mac, nodeDev_Mac, MWIFI_ADDR_LEN)){
		
		pTemp = pAbove;
		pFollow->next = pAbove->next;
		free(pTemp);

		listNodeDevOpreating_Flg = false;

		return true;
		 
	}else{

		listNodeDevOpreating_Flg = false;

		return false;
	}
}

void L8devHbDataManageList_bussinessKeepAliveManagePeriod1s(stt_nodeDev_hbDataManage *pHead){

	stt_nodeDev_hbDataManage *pAbove = pHead;
	stt_nodeDev_hbDataManage *pFollow = pAbove;

	stt_nodeDev_hbDataManage *pTemp = NULL;

	if(listNodeDevOpreating_Flg)return;

	if(mwifi_is_connected()){

		if(esp_mesh_get_layer() == MESH_ROOT){
		
			while(pAbove->next != NULL){

				if(pAbove->next->nodeDevKeepAlive_counter){

					pAbove->next->nodeDevKeepAlive_counter --;
//					printf("nodeMac:"MACSTR"-lifeTime:%d.\n", MAC2STR(pAbove->next->dataManage.nodeDev_Mac), //释放前打印，否则读空指针，要崩
//															  pAbove->next->nodeDevKeepAlive_counter);

					pAbove  = pAbove->next;
					pFollow = pAbove;
				}
				else
				{
					printf("nodeMac:"MACSTR"-remove cause heartbeat miss.\n", MAC2STR(pAbove->next->dataManage.nodeDev_Mac));  //释放前打印，否则读空指针，要崩

					pTemp = pAbove->next;
					pFollow->next = pTemp->next;
					free(pTemp);

					pAbove = pFollow;
				}
			
			}
		}
		else
		{
			L8devHbDataManageList_listDestory(pHead);
		}
	}
}

void usrApplication_systemRestartTrig(uint8_t trigDelay){

	devRestartDelay_counter = trigDelay;
}

void devSystemInfoLocalRecord_preSaveTest(void){

	nvs_handle handle;

	stt_devDataPonitTypedef dataTemp_devInfo_swStatus = {0}; //数据缓存：开关状态
	usrApp_trigTimer dataTemp_trigTimerGroup[USRAPP_VALDEFINE_TRIGTIMER_NUM] = {0}; //数据缓存：普通定时

	ESP_ERROR_CHECK( nvs_flash_init_partition(NVS_DATA_L8_PARTITION_NAME));
	ESP_ERROR_CHECK( nvs_open_from_partition(NVS_DATA_L8_PARTITION_NAME, NVS_DATA_SYSINFO_RECORD, NVS_READWRITE, &handle) );
	
	ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_SWSTATUS, &dataTemp_devInfo_swStatus, sizeof(stt_devDataPonitTypedef)) );
	ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_INFO_TIMER_NORMAL, dataTemp_trigTimerGroup, sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM) );

	ESP_ERROR_CHECK( nvs_commit(handle) );

	nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition(NVS_DATA_L8_PARTITION_NAME));
}

stt_scenarioSwitchData_nvsOpreat *nvsDataOpreation_devScenarioParam_get(uint8_t scenarioIst){

	nvs_handle handle;

	uint32_t dataLength = 0;
	esp_err_t err;

	stt_scenarioSwitchData_nvsOpreat *dataParam = (stt_scenarioSwitchData_nvsOpreat *)os_zalloc(sizeof(stt_scenarioSwitchData_nvsOpreat));
	static const char *nvsOpreat_key = NULL;

	switch(scenarioIst){

		case 0:nvsOpreat_key = DATA_DEVSCENARIO_DATA_PARAM_0;break;
		case 1:nvsOpreat_key = DATA_DEVSCENARIO_DATA_PARAM_1;break;
		case 2:
		default:nvsOpreat_key = DATA_DEVSCENARIO_DATA_PARAM_2;break;
	}

	ESP_ERROR_CHECK( nvs_flash_init_partition(NVS_DATA_L8_PARTITION_NAME));
    ESP_ERROR_CHECK( nvs_open_from_partition(NVS_DATA_L8_PARTITION_NAME, NVS_DATA_SYSINFO_RECORD, NVS_READWRITE, &handle) );

	dataLength = sizeof(stt_scenarioSwitchData_nvsOpreat);
	err = nvs_get_blob(handle, nvsOpreat_key, dataParam, &dataLength);
	if(err == ESP_OK){

		ESP_LOGI(TAG,"nvs_data devScenario dataParam read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devScenario dataParam not found, maybe first running, err:0x%04X.\n", err);
	}

    nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition(NVS_DATA_L8_PARTITION_NAME));

	return dataParam;
}

void devSystemInfoLocalRecord_allErase(void){

	nvs_handle handle;

	ESP_ERROR_CHECK( nvs_flash_init_partition(NVS_DATA_L8_PARTITION_NAME) );
    ESP_ERROR_CHECK( nvs_open_from_partition(NVS_DATA_L8_PARTITION_NAME, NVS_DATA_SYSINFO_RECORD, NVS_READWRITE, &handle) );
	ESP_ERROR_CHECK( nvs_erase_all(handle) );
    nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition(NVS_DATA_L8_PARTITION_NAME) );
}

void devSystemInfoLocalRecord_initialize(void){

	nvs_handle handle;

	uint32_t dataLength = 0;
	esp_err_t err;

	stt_devDataPonitTypedef 				dataTemp_devInfo_swStatus 										= {0}; 	//数据缓存：开关状态
	usrApp_trigTimer 						dataTemp_trigTimerGroup[USRAPP_VALDEFINE_TRIGTIMER_NUM] 		= {0}; 	//数据缓存：普通定时
	uint8_t 								dataTemp_greenModePeriod[2] 									= {0}; 	//数据缓存：绿色模式
	usrApp_trigTimer 						dataTemp_nightModeTimeTab[2] 									= {0}; 	//数据缓存：夜间模式
	uint16_t 								dataTemp_devRunningFlg 											= 0; 	//数据缓存：设备运行标志
	stt_timeZone 							dataTemp_devTimeZone 											= {0};  //数据缓存：设备时区
	devTypeDef_enum 						dataTemp_devTypeDef;
	stt_devStatusRecord 					dataTemp_devStatusRecordIF 										= {0};
	uint8_t 								dataTemp_devRouterConnectBssid[DEVICE_MAC_ADDR_APPLICATION_LEN] = {0};
	stt_devMutualGroupParam 				dataTemp_devMutaulCtrlInfo[DEVICE_MUTUAL_CTRL_GROUP_NUM] 		= {0};
	stt_devCurtain_runningParam				dataTemp_devCurtainParam										= {0};
	stt_dataDisp_guiBussinessHome_btnText	dataTemp_homepageBtnTextDisp									= {0};
	uint8_t									dataTemp_homepageBtnIconNumDisp[GUIBUSSINESS_CTRLOBJ_MAX_NUM]	= {0};
	uint8_t									dataTemp_homepageThemeType										= 0;
	char 									dataTemp_devDrviptRecalibraParam								= 0;

	/*子设备管理表，单链初始化*/
	listHead_nodeDevDataManage = (stt_nodeDev_hbDataManage *)os_zalloc(sizeof(stt_nodeDev_hbDataManage));

	/*图片RAM，内存初始化*/
//	dataPtr_btnTextImg_sw_A = (uint8_t *)os_zalloc(GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE);
//	dataPtr_btnTextImg_sw_B = (uint8_t *)os_zalloc(GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE);
//	dataPtr_btnTextImg_sw_C = (uint8_t *)os_zalloc(GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE);

	msgQh_dataManagementHandle = xQueueCreate(4, sizeof(stt_msgDats_dataManagementHandle));

	ESP_ERROR_CHECK( nvs_flash_init_partition(NVS_DATA_L8_PARTITION_NAME));
    ESP_ERROR_CHECK( nvs_open_from_partition(NVS_DATA_L8_PARTITION_NAME, NVS_DATA_SYSINFO_RECORD, NVS_READWRITE, &handle) );
	
	/*掉电数据更新 --普通定时数据*/
	dataLength = sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM;
	err = nvs_get_blob(handle, DATA_INFO_TIMER_NORMAL, dataTemp_trigTimerGroup, &dataLength);
	if(err == ESP_OK){

		usrAppActTrigTimer_paramSet(dataTemp_trigTimerGroup, false);
		ESP_LOGI(TAG,"nvs_data devAppTrigTimer info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devAppTrigTimer info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --夜间模式定时数据*/
	dataLength = sizeof(usrApp_trigTimer) * 2;
	err = nvs_get_blob(handle, DATA_INFO_TIMER_NIGHTMODE, dataTemp_nightModeTimeTab, &dataLength);
	if(err == ESP_OK){

		usrAppNightModeTimeTab_paramSet(dataTemp_nightModeTimeTab, false);
		ESP_LOGI(TAG,"nvs_data nightModeTimeTab info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data nightModeTimeTab info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --绿色模式数据*/
	dataLength = sizeof(uint8_t) * 2;
	err = nvs_get_blob(handle, DATA_INFO_TIMER_GREENMODE, dataTemp_greenModePeriod, &dataLength);
	if(err == ESP_OK){

		uint8_t temp = 0;

		//16bits数据转8bits数组数据，倒序处理
		temp = dataTemp_greenModePeriod[1];
		dataTemp_greenModePeriod[1] = dataTemp_greenModePeriod[0];
		dataTemp_greenModePeriod[0] = temp;

		usrAppParamSet_devGreenMode(dataTemp_greenModePeriod, false);
		ESP_LOGI(TAG,"nvs_data devGreenModePeriod info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data evGreenModePeriod info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --设备运行标志*/
	dataLength = sizeof(uint16_t);
	err = nvs_get_blob(handle, DATA_INFO_DEVRUNNINGFLG, &dataTemp_devRunningFlg, &dataLength);
	if(err == ESP_OK){

		currentDevRunningFlg_paramSet(dataTemp_devRunningFlg, false);
		ESP_LOGI(TAG,"nvs_data devRunningFlg info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devRunningFlg info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --设备时区*/
	dataLength = sizeof(stt_timeZone);
	err = nvs_get_blob(handle, DATA_TIMEZONE, &dataTemp_devTimeZone, &dataLength);
	if(err == ESP_OK){

		deviceParamSet_timeZone(&dataTemp_devTimeZone, false);
		ESP_LOGI(TAG,"nvs_data devTimeZone info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devTimeZone info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --设备类型*/
	dataLength = sizeof(devTypeDef_enum);
	err = nvs_get_blob(handle, DATA_DEV_TYPEDEF, &dataTemp_devTypeDef, &dataLength);
	if(err == ESP_OK){

		currentDev_typeSet(dataTemp_devTypeDef, false);
		ESP_LOGI(TAG,"nvs_data devTypeDef info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devTypeDef info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --开关状态*/
	dataLength = sizeof(stt_devDataPonitTypedef);
    err = nvs_get_blob(handle, DATA_SWSTATUS, &dataTemp_devInfo_swStatus, &dataLength);
	if(err == ESP_OK){

//		currentDev_dataPointSet(&dataTemp_devInfo_swStatus, false, false, false);
		ESP_LOGI(TAG,"nvs_data devStatus info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devStatus info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --设备状态掉电记忆使能*/
	dataLength = sizeof(stt_devStatusRecord);
	err = nvs_get_blob(handle, DATA_DEV_STATUS_RECORD_IF, &dataTemp_devStatusRecordIF, &dataLength);
	if(err == ESP_OK){

		devStatusRecordIF_paramSet(&dataTemp_devStatusRecordIF, false);
		
		if(!dataTemp_devStatusRecordIF.devStatusOnOffRecord_IF){ //设备状态存储失能，设备状态开机直接清零

			memset(&dataTemp_devInfo_swStatus, 0, sizeof(stt_devDataPonitTypedef)); //记忆使能则数据保持，否则清空
		}	

		currentDev_dataPointSet(&dataTemp_devInfo_swStatus, false, false, true);
		
		ESP_LOGI(TAG,"nvs_data devStatusRecordIF info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devStatusRecordIF info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --历史路由器BSSID*/
	dataLength = sizeof(uint8_t) * 6;
	err = nvs_get_blob(handle, DATA_DEV_ROUTER_BSSID, dataTemp_devRouterConnectBssid, &dataLength);
	if(err == ESP_OK){

		devRouterConnectBssid_Set(dataTemp_devRouterConnectBssid, false);
		ESP_LOGI(TAG,"nvs_data devRouterConnectBssid info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devRouterConnectBssid info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --互控设置数据*/
	dataLength = sizeof(stt_devMutualGroupParam) * DEVICE_MUTUAL_CTRL_GROUP_NUM;
	err = nvs_get_blob(handle, DATA_DEV_MUTUALCTRL_INFO, dataTemp_devMutaulCtrlInfo, &dataLength);
	if(err == ESP_OK){

		devMutualCtrlGroupInfo_Set(&dataTemp_devMutaulCtrlInfo[0], 0x01, false);
		devMutualCtrlGroupInfo_Set(&dataTemp_devMutaulCtrlInfo[1], 0x02, false);
		devMutualCtrlGroupInfo_Set(&dataTemp_devMutaulCtrlInfo[2], 0x04, false);
		ESP_LOGI(TAG,"nvs_data devMutaulCtrl info read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devMutaulCtrl info not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --窗帘开关设备运行参数（轨道时间）*/
	dataLength = sizeof(stt_Curtain_motorAttr);
	err = nvs_get_blob(handle, DATA_DEVCURTAIN_RUNNINGPARAM, &dataTemp_devCurtainParam, &dataLength);
	if(err == ESP_OK){

		devCurtain_runningParamSet(&dataTemp_devCurtainParam, false);
		
		ESP_LOGI(TAG,"nvs_data devCurtain running param read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data devCurtain running param not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --home界面按键显示文字*/
	dataLength = sizeof(stt_dataDisp_guiBussinessHome_btnText);
	err = nvs_get_blob(handle, DATA_DEV_GUIHOMEBTNTEXTDISP, &dataTemp_homepageBtnTextDisp, &dataLength);
	if(err == ESP_OK){

		usrAppHomepageBtnTextDisp_paramSet(&dataTemp_homepageBtnTextDisp, false);
		
		ESP_LOGI(TAG,"nvs_data homepage btnTextDisp param read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data homepage btnTextDisp param not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --home界面按键显示图标索引*/
	dataLength = sizeof(uint8_t) * GUIBUSSINESS_CTRLOBJ_MAX_NUM;
	err = nvs_get_blob(handle, DATA_DEV_GUIHOMEBTNICONDISP, dataTemp_homepageBtnIconNumDisp, &dataLength);
	if(err == ESP_OK){

		usrAppHomepageBtnIconNumDisp_paramSet(dataTemp_homepageBtnIconNumDisp, false);
		
		ESP_LOGI(TAG,"nvs_data homepage btnIconNumDisp param read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data homepage btnIconNumDisp param not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --home界面主题类型*/
	dataLength = sizeof(uint8_t);
	err = nvs_get_blob(handle, DATA_DEV_GUIHOMETHEMETYPE, &dataTemp_homepageThemeType, &dataLength);
	if(err == ESP_OK){

		extern void usrAppHomepageThemeType_Set(const uint8_t themeType_flg, bool nvsRecord_IF);

		usrAppHomepageThemeType_Set(dataTemp_homepageThemeType, false);
		
		ESP_LOGI(TAG,"nvs_data homepage themeType param read success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data homepage themeType param not found, maybe first running, err:0x%04X.\n", err);
	}

	/*掉电数据更新 --屏幕校准使能*/
	dataLength = sizeof(char);
	err = nvs_get_blob(handle, DATA_DEVDRVIPT_RECALIBRAPARAM, &dataTemp_devDrviptRecalibraParam, &dataLength);
	if(err == ESP_OK){	

		devIptdrvParam_recalibration = dataTemp_devDrviptRecalibraParam;
		if(dataTemp_devDrviptRecalibraParam == 'Y'){ //当此生效，生效后参数复位

			dataTemp_devDrviptRecalibraParam = 'N';
			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVDRVIPT_RECALIBRAPARAM, &dataTemp_devDrviptRecalibraParam, sizeof(char)) );
		}
		
		ESP_LOGI(TAG,"nvs_data devDrvInpt paramRecalibra success.\n");
		
	}else{

		ESP_LOGI(TAG,"nvs_data  devDrvInpt paramRecalibra not found, maybe first running, err:0x%04X.\n", err);
	}


//	/*掉电数据更新 --home界面按键文件图片<pic_A>*/
//	if(dataPtr_btnTextImg_sw_A){

//		dataLength = sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE;
//		err = nvs_get_blob(handle, DATA_DEV_GUIHOMEBTNTEXTPIC_A, dataPtr_btnTextImg_sw_A, &dataLength);
//		if(err == ESP_OK){

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_A info read success.\n");
//			
//		}else{

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_A info not found, maybe first running, err:0x%04X.\n", err);
//		}
//	}

//	/*掉电数据更新 --home界面按键文件图片<pic_B>*/
//	if(dataPtr_btnTextImg_sw_A){

//		dataLength = sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE;
//		err = nvs_get_blob(handle, DATA_DEV_GUIHOMEBTNTEXTPIC_B, dataPtr_btnTextImg_sw_B, &dataLength);
//		if(err == ESP_OK){

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_B info read success.\n");
//			
//		}else{

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_B info not found, maybe first running, err:0x%04X.\n", err);
//		}
//	}

//	/*掉电数据更新 --home界面按键文件图片<pic_C>*/
//	if(dataPtr_btnTextImg_sw_A){

//		dataLength = sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE;
//		err = nvs_get_blob(handle, DATA_DEV_GUIHOMEBTNTEXTPIC_C, dataPtr_btnTextImg_sw_C, &dataLength);
//		if(err == ESP_OK){

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_C info read success.\n");
//			
//		}else{

//			ESP_LOGI(TAG,"nvs_data guiHome btnTextImg_sw_C info not found, maybe first running, err:0x%04X.\n", err);
//		}
//	}

    nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition(NVS_DATA_L8_PARTITION_NAME));

	devCurrentRunningFlg &= ~DEV_RUNNING_FLG_BIT_DELAY; //延时模式所有数据掉电清空，包括运行标志
}

void devSystemInfoLocalRecord_save(enum_dataSaveObj obj, void *dataSave){

    nvs_handle handle;

	while(usrAppOpreation_nvsFlashOpen_flg)vTaskDelay(2 / portTICK_RATE_MS);
	usrAppOpreation_nvsFlashOpen_flg = true;

	ESP_ERROR_CHECK( nvs_flash_init_partition(NVS_DATA_L8_PARTITION_NAME));
    ESP_ERROR_CHECK( nvs_open_from_partition(NVS_DATA_L8_PARTITION_NAME, NVS_DATA_SYSINFO_RECORD, NVS_READWRITE, &handle) );

	switch(obj){

		case saveObj_swStatus:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_SWSTATUS, dataSave, sizeof(stt_devDataPonitTypedef)) );
		
		}break;

		case saveObj_infoTimer_normal:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_INFO_TIMER_NORMAL, dataSave, sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM) );

		}break;

		case saveObj_infoTimer_nightMode:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_INFO_TIMER_NIGHTMODE, dataSave, sizeof(usrApp_trigTimer) * 2) );

		}break;

		case saveObj_infoTimer_greenMode:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_INFO_TIMER_GREENMODE, dataSave, sizeof(uint8_t) * 2) );

		}break;

		case saveObj_devRunning_flg:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_INFO_DEVRUNNINGFLG, dataSave, sizeof(uint16_t)) );

		}break;

		case saveObj_devTimeZone:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_TIMEZONE, dataSave, sizeof(stt_timeZone)) );

		}break;

		case saveObj_devTypeDef:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_TYPEDEF, dataSave, sizeof(devTypeDef_enum)) );

		}break;

		case saveObj_devStatusRecordIF:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_STATUS_RECORD_IF, dataSave, sizeof(stt_devStatusRecord)) );

		}break;

		case saveObj_routerBssid:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_ROUTER_BSSID, dataSave, sizeof(uint8_t) * DEVICE_MAC_ADDR_APPLICATION_LEN) );

		}break;

		case saveObj_mutualCtrlInfo:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_MUTUALCTRL_INFO, dataSave, sizeof(stt_devMutualGroupParam) * DEVICE_MUTUAL_CTRL_GROUP_NUM) );

		}break;

		case saveObj_devCurtain_runningParam:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVCURTAIN_RUNNINGPARAM, dataSave, sizeof(stt_devCurtain_runningParam)) );
		
		}break;

		case saveObj_devScenario_paramDats_0:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVSCENARIO_DATA_PARAM_0, dataSave, sizeof(stt_scenarioSwitchData_nvsOpreat)) );

		}break;

		case saveObj_devScenario_paramDats_1:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVSCENARIO_DATA_PARAM_1, dataSave, sizeof(stt_scenarioSwitchData_nvsOpreat)) );

		}break;

		case saveObj_devScenario_paramDats_2:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVSCENARIO_DATA_PARAM_2, dataSave, sizeof(stt_scenarioSwitchData_nvsOpreat)) );

		}break;

		case saveObj_devGuiBussinessHome_btnTextDisp:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMEBTNTEXTDISP, dataSave, sizeof(stt_dataDisp_guiBussinessHome_btnText)) );

		}break;

		case saveObj_devGuiBussinessHome_btnIconDisp:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMEBTNICONDISP, dataSave, sizeof(uint8_t) * GUIBUSSINESS_CTRLOBJ_MAX_NUM) );

		}break;

//		case saveObj_devGuiBussinessHome_btnTextPic_A:{

//			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMEBTNTEXTPIC_A, dataSave, sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE) );
//		
//		}break;

//		case saveObj_devGuiBussinessHome_btnTextPic_B:{

//			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMEBTNTEXTPIC_B, dataSave, sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE) );

//		}break;

//		case saveObj_devGuiBussinessHome_btnTextPic_C:{

//			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMEBTNTEXTPIC_C, dataSave, sizeof(uint8_t) * GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE) );

//		}break;

		case saveObj_devGuiBussinessHome_themeType:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEV_GUIHOMETHEMETYPE, dataSave, sizeof(uint8_t)) );

		}break;

		case saveObj_devDriver_iptRecalibration_set:{

			ESP_ERROR_CHECK( nvs_set_blob( handle, DATA_DEVDRVIPT_RECALIBRAPARAM, dataSave, sizeof(char)) );

		}break;

		default:break;
	}

    ESP_ERROR_CHECK( nvs_commit(handle) );

	nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition(NVS_DATA_L8_PARTITION_NAME));

	vTaskDelay(10 / portTICK_RATE_MS);

	usrAppOpreation_nvsFlashOpen_flg = false;
}


#include "dataTrans_meshUpgrade.h"

#include "os.h"

#include "mlink.h"
#include "mwifi.h"
#include "mupgrade.h"

#include "mdf_common.h"
#include "mqtt_client.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

#include "dataTrans_remoteServer.h"
#include "dataTrans_localHandler.h"

#include "devDriver_manage.h"

extern void lvGui_tipsFullScreen_generate(const char *strTips, uint8_t timeOut);
extern void lvGui_tipsFullScreen_generateAutoTime(const char *strTips);
extern void lvGui_tipsFullScreen_distruction(void);

extern TaskHandle_t taskHandle_communicateTrigBussiness;

extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;

extern EventGroupHandle_t xEventGp_devApplication;

static const char *TAG = "L8_OTA_upgrade";

static const char *severAddressHead = "http://112.124.61.191:8080/sm/image/device_firmware/";

static uint8_t upgradeUrlIP_temp[4] = {112, 124, 61, 191};
static char upgradeUrlAddress_temp[128]  = {0}; //url缓存
static char upgradeFirewareName_temp[64] = "hello-world(1.0.2).bin"; //fireware名称缓存
static char upgradeFirewareName_record[64] = {0};

static uint8_t *upgradeDestAddr = NULL;
static uint8_t upgradeDevNum = 0;

static uint8_t upgradeTarget_devType = 0;
static uint8_t upgradeDestAddr_temp[MWIFI_ADDR_LEN] = MWIFI_ADDR_NONE;
static bool upgradeMulti_if = false;

static bool flg_upgradeTrig = false;

static stt_statusParam_httpUpgrade rmOTA_statusParam = {0};

static void upgradeMeshOTA_applicationTask(void *arg){

    mdf_err_t ret       = MDF_OK;
    uint8_t *data       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    char name[32]       = {0x0};
    size_t total_size   = 0;
    int start_time      = 0;
    mwifi_data_type_t data_type = {.communicate = MWIFI_COMMUNICATE_MULTICAST};

	uint8_t *upgradeListInfo = NULL;

	esp_http_client_handle_t client = NULL;

    esp_http_client_config_t config = {

        .transport_type = HTTP_TRANSPORT_UNKNOWN,
    };

	flg_upgradeTrig = true; //远程升级已启动

	lvGui_tipsFullScreen_generateAutoTime("upgrade stand by. . .");

	vTaskDelay(3000 / portTICK_RATE_MS);
	vTaskSuspend(taskHandle_communicateTrigBussiness);

	tipsOpreatAutoSet_sysUpgrading();

	if(upgradeMulti_if){ //批量升级判定

		upgradeListInfo = L8DevListGet_Type1(listHead_nodeDevDataManage, upgradeTarget_devType);
		upgradeDestAddr = &upgradeListInfo[1]; //下标1开始为MAC列表
		upgradeDevNum = upgradeListInfo[0]; //下标0为数目

		MDF_LOGI("mulit upgrade num:%d\n", upgradeDevNum);
	}
	else
	{
		upgradeDestAddr = upgradeDestAddr_temp;
		upgradeDevNum = 1;
	}

    const char *upgStandBy_str = L8_MESH_CONMUNICATE_UPGSTBY_NOTICE_STR;
    ret = mwifi_root_write(upgradeDestAddr, upgradeDevNum, &data_type, upgStandBy_str, strlen(upgStandBy_str) + 1, true); //长度加上字符串本身的\0
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

	vTaskDelay(10000 / portTICK_RATE_MS);

	memset(&rmOTA_statusParam, 0, sizeof(stt_statusParam_httpUpgrade));
	rmOTA_statusParam.downloading_if = 1;

	if(strcmp(upgradeFirewareName_record, upgradeFirewareName_temp)){ //固件相同，不再重复下载

		memset(upgradeUrlAddress_temp, 0, sizeof(uint8_t) * 128);
		sprintf(upgradeUrlAddress_temp, "http://%d.%d.%d.%d:8080/sm/image/device_firmware/%s", upgradeUrlIP_temp[0], 
																							   upgradeUrlIP_temp[1], 
																							   upgradeUrlIP_temp[2],
																							   upgradeUrlIP_temp[3],
																							   upgradeFirewareName_temp);
		config.url = upgradeUrlAddress_temp;

		vTaskDelay(3000 / portTICK_RATE_MS);
		lvGui_tipsFullScreen_generate("firmware downloading. . .", 254); //全屏阻塞UI提示
		vTaskDelay(3000 / portTICK_RATE_MS);

	    client = esp_http_client_init(&config);
	    MDF_ERROR_GOTO(!client, EXIT, "Initialise HTTP connection");

	    start_time = xTaskGetTickCount();

	    MDF_LOGI("Open HTTP connection: %s", upgradeUrlAddress_temp);

	    do {
	        ret = esp_http_client_open(client, 0);

	        if (ret != MDF_OK) {
	            vTaskDelay(1000 / portTICK_RATE_MS);
	            MDF_LOGW("<%s> Connection service failed", mdf_err_to_name(ret));
	        }
	    } while (ret != MDF_OK);

	    total_size = esp_http_client_fetch_headers(client);
	    sscanf(upgradeUrlAddress_temp, "%*[^//]//%*[^/]/%[^.bin]", name);

	    ret = mupgrade_firmware_init(name, total_size);
	    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> Initialize the upgrade status", mdf_err_to_name(ret));

		rmOTA_statusParam.firwareTotal_size = total_size;

	    for (int recv_size = 0, size = 0;; recv_size += size) {
			
	        size = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
	        MDF_ERROR_GOTO(size < 0, EXIT, "<%s> Read data from http stream", mdf_err_to_name(ret));

			rmOTA_statusParam.firwareRecv_size = recv_size;

	        if(size > 0){

				char   strTips[32] 			= {0};
					   uint8_t percent 		= 0;
				static uint8_t percent_rcd 	= 0;

				percent = recv_size / (total_size / 100);
				if(percent_rcd != percent){

					extern void esp_task_wdt_feed(void);

					percent_rcd = percent;
					if(0 == (percent_rcd % 10)){

						sprintf(strTips, "firmware \ndownloading %d%%.", percent);
						lvGui_tipsFullScreen_generate(strTips, 254);

						MDF_LOGI("%s\n", strTips);

						esp_task_wdt_feed(); //task喂狗
						vTaskDelay(100 / portTICK_RATE_MS);
					}
				}
				
	            ret = mupgrade_firmware_download(data, size);
	            MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> Write firmware to flash", mdf_err_to_name(ret));

				datatransOpreation_heartbeatHold_set(120); //心跳挂起
	        }else
			if(recv_size == total_size){
				
	            MDF_LOGI("Connection closed, all data received");
	            break;
	        } 
			else
			{
				vTaskDelay(2000 / portTICK_RATE_MS);
				lvGui_tipsFullScreen_generate("firmware \ndownload fail", 15); //全屏阻塞UI提示
				vTaskDelay(2000 / portTICK_RATE_MS);
	            MDF_LOGW("<%s> esp_http_client_read", mdf_err_to_name(ret));
	            goto EXIT;
	        }
	    }

	    MDF_LOGI("The service download firmware is complete, Spend time: %ds",
	             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);

	    start_time = xTaskGetTickCount();

		lvGui_tipsFullScreen_generateAutoTime("download complete,\n firmware forwarding..."); //全屏阻塞UI提示
	}

	datatransOpreation_heartbeatHold_auto(); //心跳挂起

    ret = mupgrade_firmware_send(upgradeDestAddr, upgradeDevNum, NULL);
	if(ret != MDF_OK){

		vTaskDelay(2000 / portTICK_RATE_MS);
		lvGui_tipsFullScreen_generate("firmware forward fail", 15); //全屏阻塞UI提示
		vTaskDelay(2000 / portTICK_RATE_MS);
	}
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> mupgrade_firmware_send", mdf_err_to_name(ret));

	vTaskDelay(2000 / portTICK_RATE_MS);
	lvGui_tipsFullScreen_generate("firmware \nforward success", 15); //全屏阻塞UI提示
	vTaskDelay(2000 / portTICK_RATE_MS);

    MDF_LOGI("Firmware is sent to the device to complete, Spend time: %ds",
             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);

    const char *restart_str = L8_MESH_CONMUNICATE_RESTART_NOTICE_STR;
    ret = mwifi_root_write(upgradeDestAddr, upgradeDevNum, &data_type, restart_str, strlen(restart_str) + 1, true); //长度加上字符串本身的\0
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

	memset(upgradeFirewareName_record, 0, sizeof(uint8_t) * 64);
	strcpy(upgradeFirewareName_record, upgradeFirewareName_temp); //升级成功才更新记录

EXIT:
    MDF_FREE(data);

	memset(&rmOTA_statusParam, 0, sizeof(stt_statusParam_httpUpgrade));

	if(upgradeMulti_if){

		upgradeMulti_if = false;
		
		os_free(upgradeListInfo);
		upgradeListInfo = NULL;
		upgradeDestAddr = NULL;
	}
	
	if(client){

	    esp_http_client_close(client);
	    esp_http_client_cleanup(client);
	}

	vTaskDelay(2000 / portTICK_RATE_MS);
	lvGui_tipsFullScreen_distruction();
	vTaskDelay(2000 / portTICK_RATE_MS);
	flg_upgradeTrig = false;

	vTaskResume(taskHandle_communicateTrigBussiness);
	tipsOpreatSet_sysUpgrading(1);

	vTaskDelete(NULL);
}

void usrApp_httpUpgradeProgress_paramGet(stt_statusParam_httpUpgrade *param){

	memcpy(param, &rmOTA_statusParam, sizeof(stt_statusParam_httpUpgrade));
}

void usrAppUpgrade_firewareNameSet(const char *fName){
	
	memset(upgradeFirewareName_temp, 0, sizeof(uint8_t) * 64);

	strcpy(upgradeFirewareName_temp, fName);

	MDF_LOGI("mupgrade firmwareName chg to \"%s\"", fName);
}

void usrAppUpgrade_targetDevaddrSet(const uint8_t addr[MWIFI_ADDR_LEN]){

	memcpy(upgradeDestAddr_temp, addr, sizeof(uint8_t) * MWIFI_ADDR_LEN);

	MDF_LOGI("mupgrade targetMac chg to "MACSTR, MAC2STR(addr));
}

void usrAppUpgrade_remoteIPset(uint8_t ip[4]){

	memcpy(upgradeUrlIP_temp, ip, sizeof(uint8_t) * 4);

	MDF_LOGI("mupgrade targetIP chg to %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

void mwifiApp_firewareUpgrade_nodeNoticeToRoot(void){

	mwifi_data_type_t data_type = {
		
		.compression = true,
		.communicate = MWIFI_COMMUNICATE_MULTICAST,
	};
	const mlink_httpd_type_t type_L8mesh_cst = {

		.format = MLINK_HTTPD_FORMAT_HEX,
	};

	mdf_err_t ret = MDF_OK;

	const uint8_t mwifiRootAddr[MWIFI_ADDR_LEN] = MWIFI_ADDR_ROOT;
		  uint8_t dataTrans_temp[2] = {0};

	dataTrans_temp[0] = L8DEV_MESH_FWARE_UPGRADE;
	dataTrans_temp[1] = (uint8_t)currentDev_typeGet();

	memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));
	data_type.communicate = MWIFI_COMMUNICATE_UNICAST;	

	ret = mwifi_write(mwifiRootAddr, 
					  &data_type, 
					  dataTrans_temp, 
					  2, 
					  true);
	MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> nodeDev upgradeNotice: mwifi_translate", mdf_err_to_name(ret)); 

	printf("upgrade dttx devType:%02X.\n", dataTrans_temp[1]);
}

void upgradeMeshOTA_taskCreatAction(void){

	portBASE_TYPE res = MDF_FAIL;

	res = xTaskCreate(upgradeMeshOTA_applicationTask, "L8OTA_TASK", 6 * 1024, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);		
	MDF_LOGI("upgrade task creat res = %d.\n", res);
}

void usrApp_firewareUpgrade_trig(bool mulitUpgrade_if, uint8_t devtype){

//	const uint8_t targetDev_address[MWIFI_ADDR_LEN] = MWIFI_ADDR_ANY;
//	esp_wifi_get_mac(ESP_IF_WIFI_STA, targetDev_address);

	if(mwifi_is_connected()){

		upgradeTarget_devType = devtype;

		printf("upgrade devType:%02X.\n", upgradeTarget_devType);

		if(esp_mesh_get_layer() == MESH_ROOT){

			if(!flg_upgradeTrig){

				upgradeMulti_if = mulitUpgrade_if;

				xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_HOST_UPGRATE_TASK_CREAT); //事件通知触发远程升级任务启动
			}
			else
			{
				MDF_LOGI("upgrade task creat fail!!!\n");
			}
		}
		else
		{
			xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_DEVNODE_UPGRADE_REQUEST); //子设备向主设备发起升级申请
		}
	}
}






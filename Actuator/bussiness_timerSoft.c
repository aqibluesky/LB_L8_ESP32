#include "bussiness_timerSoft.h"

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

#include "lwip/apps/sntp.h"

#include "devDataManage.h"
#include "devDriver_manage.h"

extern stt_nodeDev_hbDataManage *listHead_nodeDevDataManage;
extern uint8_t devRunningTimeFromPowerUp_couter;
extern uint8_t devRestartDelay_counter;
extern uint8_t wifiConfigComplete_tipsStartCounter;
extern uint16_t ctrlObj_slidingCalmDownCounter;

EventGroupHandle_t xEventGp_devApplication = NULL;
xQueueHandle msgQh_systemRestartDelayCounterTips = NULL;

static const char* TAG = "lanbon_L8 - timerSoft";

static stt_localTime devSystemTime_current = { //系统时间初始化

	.time_Year 		= 2000,
	.time_Month 	= 1,
	.time_Day 		= 1,
	.time_Week 		= 1,
	.time_Hour 		= 0,
	.time_Minute 	= 0,
	.time_Second 	= 0,
};
static stt_timeZone devSystemTimeZone = {0, 0};
uint16_t devSysTimeKeep_counter = 0;

static time_t timeNow;
static struct tm timeInfo;

static usrApp_trigTimer usrApp_trigTimerGroup[USRAPP_VALDEFINE_TRIGTIMER_NUM] = {0};
static uint16_t timeUp_actionDone_flg = 0;

static usrApp_trigTimer usrApp_nightModeTimeTab[2] = {0};

static bool flg_stnp_running = false;

static TimerHandle_t timerHld_bussinessSoftTimer = NULL;

static uint8_t tipsLoopTimer_keeperCounter = 0;

static stt_timerLoop32BitCnt loopTimer_devHeartbeat = {DEV_HEARTBEAT_DATATRANS_PERIOD, 0}; //心跳计时器相关变量
static stt_timerLoop32BitCnt loopTimer_devElecsumReport = {DEV_ESUMREPORT_DATATRANS_PERIOD, 0}; //mqtt电量上报计时器相关变量

static struct stt_devStatusRecord_actionDelayParam{

	uint8_t timeCounter:7;
	uint8_t actionTrig_IF:1;
	
}devStatusRecord_actionDelayParam = {0};

EventGroupHandle_t xEventGp_tipsLoopTimer = NULL;

void deviceParamSet_timeZone(stt_timeZone *param, bool nvsRecord_IF){

	if(nvsRecord_IF){

		if(memcmp(param, &devSystemTimeZone, sizeof(stt_timeZone))){

			memcpy(&devSystemTimeZone, param, sizeof(stt_timeZone));
			devSystemInfoLocalRecord_save(saveObj_devTimeZone, &devSystemTimeZone);
		}
	}
	else{

		memcpy(&devSystemTimeZone, param, sizeof(stt_timeZone));
	}
}

void deviceParamGet_timeZone(stt_timeZone *param){

	memcpy(param, &devSystemTimeZone, sizeof(stt_timeZone));
}

void usrAppActTrigTimer_paramSet(usrApp_trigTimer param[USRAPP_VALDEFINE_TRIGTIMER_NUM], bool nvsRecord_IF){

	memcpy(usrApp_trigTimerGroup, param, sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM);
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_infoTimer_normal, (void *)usrApp_trigTimerGroup);
	timeUp_actionDone_flg = 0; //分钟禁止重复响应标志清空
}

void usrAppActTrigTimer_paramUnitSet(usrApp_trigTimer *param, uint8_t unitNum, bool nvsRecord_IF){

	memcpy(&usrApp_trigTimerGroup[unitNum], param, sizeof(usrApp_trigTimer));
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_infoTimer_normal, (void *)usrApp_trigTimerGroup);
	timeUp_actionDone_flg &= (1 << unitNum); //分钟禁止重复响应标志清空
}

void usrAppActTrigTimer_paramGet(usrApp_trigTimer param[USRAPP_VALDEFINE_TRIGTIMER_NUM]){

	memcpy(param, usrApp_trigTimerGroup, sizeof(usrApp_trigTimer) * USRAPP_VALDEFINE_TRIGTIMER_NUM);
}

void usrAppActTrigTimer_paramUnitGet(usrApp_trigTimer *param, uint8_t unitNum){

	memcpy(param, &usrApp_trigTimerGroup[unitNum], sizeof(usrApp_trigTimer));
}

void usrAppNightModeTimeTab_paramSet(usrApp_trigTimer param[2], bool nvsRecord_IF){

	memcpy(usrApp_nightModeTimeTab, param, sizeof(usrApp_trigTimer) * 2);
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_infoTimer_nightMode, usrApp_nightModeTimeTab);
}

void usrAppNightModeTimeTab_paramGet(usrApp_trigTimer param[2]){

	memcpy(param, usrApp_nightModeTimeTab, sizeof(usrApp_trigTimer) * 2);
}

void usrAppDevCurrentSystemTime_paramSet(stt_localTime *timeParam){

	memcpy(&devSystemTime_current, timeParam, sizeof(stt_localTime));
	
	devSysTimeKeep_counter = devSystemTime_current.time_Minute * 60 + devSystemTime_current.time_Second; //离线维持计时值更新
}

void usrAppDevCurrentSystemTime_paramGet(stt_localTime *timeParam){

	memcpy(timeParam, &devSystemTime_current, sizeof(stt_localTime));
}

void usrAppDevStatusRecord_delayActionTrig(void){

	devStatusRecord_actionDelayParam.timeCounter = DEVSTATUS_RECORD_ACTION_DELAY_TIME;
	devStatusRecord_actionDelayParam.actionTrig_IF = 1;
}

static void sntp_applicationStart(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "0.cn.pool.ntp.org");
	sntp_setservername(1, "1.cn.pool.ntp.org");
	sntp_setservername(2, "2.cn.pool.ntp.org");
    sntp_init();

	flg_stnp_running = true;
}

static void sntp_applicationStop(void){

	sntp_stop();
	
	flg_stnp_running = false;
}

static void devSystimeRealesFromSntp(struct tm sntpTime){

	if(sntpTime.tm_year < (2000 - 1900))return; //非法时间不进行更新

	devSystemTime_current.time_Year 	= sntpTime.tm_year + 1900;
	devSystemTime_current.time_Month 	= sntpTime.tm_mon + 1;
	devSystemTime_current.time_Day		= sntpTime.tm_mday;
	devSystemTime_current.time_Hour		= sntpTime.tm_hour;
	devSystemTime_current.time_Minute	= sntpTime.tm_min;
	devSystemTime_current.time_Second	= sntpTime.tm_sec;

	(sntpTime.tm_wday)?(devSystemTime_current.time_Week = sntpTime.tm_wday):(devSystemTime_current.time_Week = 7);

	devSysTimeKeep_counter = devSystemTime_current.time_Minute * 60 + devSystemTime_current.time_Second; //离线维持计时值更新
}

static void usrTimerUp_tipsTrig(uint8_t timerWitch){

	xEventGroupSetBits(xEventGp_tipsLoopTimer, 1 << timerWitch);
//	printf("timer:%d guiTips trig tx!\n", timerWitch);
}

void usrAppParamSet_hbRealesInAdvance(bool immediatelyIf){

	if(mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT)return; //根节点不参与心跳提前业务业务

	if(immediatelyIf){

		loopTimer_devHeartbeat.loopCounter = DEV_HEARTBEAT_DATATRANS_PERIOD - 600;
	}
	else
	{
		loopTimer_devHeartbeat.loopCounter = DEV_HEARTBEAT_DATATRANS_PERIOD - (esp_random() % 4000); //4000ms随机数
	}
}

static void bussiness_usrApp_actTrigTimer(void){

	uint16_t timeCompare_reference = (uint16_t)(devSystemTime_current.time_Hour) * 60 + (uint16_t)(devSystemTime_current.time_Minute);
	uint16_t devRunningFlg_temp = 0;
	bool devRunningTemp_timerNormal = false;
	bool devRunningTemp_nightMode = false;

	/*普通定时业务逻辑*/
	for(uint8_t loop = 0; loop < USRAPP_VALDEFINE_TRIGTIMER_NUM; loop ++){

		if(usrApp_trigTimerGroup[loop].tmUp_runningEn){

			uint16_t timeCompare_temp = (uint16_t)(usrApp_trigTimerGroup[loop].tmUp_Hour) * 60 + (uint16_t)(usrApp_trigTimerGroup[loop].tmUp_Minute);

			if(timeCompare_temp != timeCompare_reference){

				if(timeUp_actionDone_flg & (1 << loop))timeUp_actionDone_flg &= ~(1 << loop);
			}

			if(timeCompare_temp < timeCompare_reference){

				
			}
			else
			if(timeCompare_temp == timeCompare_reference){

				if(!(timeUp_actionDone_flg & (1 << loop))){ //一分钟内只能完成一次响应判断

					bool timerTrig_reserveFLG = false;
				
					timeUp_actionDone_flg |= (1 << loop);

					if(usrApp_trigTimerGroup[loop].tmUp_weekBitHold){

						if((1 << (devSystemTime_current.time_Week - 1)) & usrApp_trigTimerGroup[loop].tmUp_weekBitHold){

							ESP_LOGI(TAG, "loopTimer:%d up!!!.\n", loop + 1);
							timerTrig_reserveFLG = true;
						}
						
					}else{ //周占位为空，代表本组定时为一次性

						ESP_LOGI(TAG, "onceTimer:%d up!!!.\n", loop + 1);
						timerTrig_reserveFLG = true;

						usrApp_trigTimerGroup[loop].tmUp_runningEn = 0;
						devSystemInfoLocalRecord_save(saveObj_infoTimer_normal, usrApp_trigTimerGroup); //本地存储数据更新
					}

					if(timerTrig_reserveFLG){ //定时响应动作触发

						stt_devDataPonitTypedef devStatusValSet_temp = {0};

						usrTimerUp_tipsTrig(loop); //图标提示通知

						switch(currentDev_typeGet()){

							case devTypeDef_mulitSwOneBit:
							case devTypeDef_mulitSwTwoBit:
							case devTypeDef_mulitSwThreeBit:
							case devTypeDef_fans:
							case devTypeDef_scenario:
							case devTypeDef_heater:{

								uint8_t timerUp_trigVal = usrApp_trigTimerGroup[loop].tmUp_swValTrig;

								memcpy(&devStatusValSet_temp, &timerUp_trigVal, sizeof(uint8_t));

							}break;
							
							case devTypeDef_dimmer:{

								const uint8_t dataValReaction_coef = DEVICE_DIMMER_BRIGHTNESS_MAX_VAL / 
																	 DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV;
									
								uint8_t timerUp_trigVal = usrApp_trigTimerGroup[loop].tmUp_swValTrig * 
														  dataValReaction_coef;
								
								memcpy(&devStatusValSet_temp, &timerUp_trigVal, sizeof(uint8_t));

							}break;
							
							case devTypeDef_curtain:{

								const uint8_t dataValReaction_coef = DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL / 
																	 DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV;
									
								uint8_t timerUp_trigVal = usrApp_trigTimerGroup[loop].tmUp_swValTrig * 
														  dataValReaction_coef;

								memcpy(&devStatusValSet_temp, &timerUp_trigVal, sizeof(uint8_t));
								devStatusValSet_temp.devType_curtain.devCurtain_actMethod = 1; //位置定时，用滑条方式

							}break;

							default:break;
						}

						currentDev_dataPointSet(&devStatusValSet_temp, true, true, true);
					}
				}
			}
			else
			if(timeCompare_temp > timeCompare_reference){
			
				//bussiness reserve.
			}

			devRunningTemp_timerNormal = true;
		}
	}

	devRunningFlg_temp = currentDevRunningFlg_paramGet();
	if(devRunningTemp_timerNormal){ //运行缓存及本地存储更新

		if(!(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_TIMER))currentDevRunningFlg_paramSet(devRunningFlg_temp | DEV_RUNNING_FLG_BIT_TIMER, true);
	}
	else
	{
		if((devRunningFlg_temp & DEV_RUNNING_FLG_BIT_TIMER))currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_TIMER), true);
	}

	/*夜间模式业务逻辑*/
	if(usrApp_nightModeTimeTab[0].tmUp_runningEn | usrApp_nightModeTimeTab[0].tmUp_weekBitHold){ //第1数据段第一字节非零则表示 全天夜间

		devRunningTemp_nightMode = true;
		
	}else{

		if(usrApp_nightModeTimeTab[1].tmUp_runningEn | usrApp_nightModeTimeTab[1].tmUp_weekBitHold){ //第2数据段第一字节非零则表示 按时段夜间

			uint16_t minuteTempNightModeTimeTab_CalibrateTab_A = (uint16_t)(usrApp_nightModeTimeTab[0].tmUp_Hour) * 60 + (uint16_t)(usrApp_nightModeTimeTab[0].tmUp_Minute);
			uint16_t minuteTempNightModeTimeTab_CalibrateTab_B = (uint16_t)(usrApp_nightModeTimeTab[1].tmUp_Hour) * 60 + (uint16_t)(usrApp_nightModeTimeTab[1].tmUp_Minute);
			bool minuteTempNightModeTimeTab_reserveFlg = false;

			(minuteTempNightModeTimeTab_CalibrateTab_A < minuteTempNightModeTimeTab_CalibrateTab_B)?
				(minuteTempNightModeTimeTab_reserveFlg = false):
				(minuteTempNightModeTimeTab_reserveFlg = true);

			if(!minuteTempNightModeTimeTab_reserveFlg){ //时段顺序逻辑

				((timeCompare_reference >=	minuteTempNightModeTimeTab_CalibrateTab_A)&&
				 (timeCompare_reference <	minuteTempNightModeTimeTab_CalibrateTab_B))?
					(devRunningTemp_nightMode = true):(devRunningTemp_nightMode = false);
			}
			else //时段反序逻辑
			{
				((timeCompare_reference >=	minuteTempNightModeTimeTab_CalibrateTab_A)||
				 (timeCompare_reference <	minuteTempNightModeTimeTab_CalibrateTab_B))?
					(devRunningTemp_nightMode = true):(devRunningTemp_nightMode = false);
			}
		}
		else
		{
			devRunningTemp_nightMode = false;
		}
	}

	devRunningFlg_temp = currentDevRunningFlg_paramGet();
	if(devRunningTemp_nightMode){ //运行缓存及本地存储更新

		if(!(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_NIGHTMODE))currentDevRunningFlg_paramSet(devRunningFlg_temp | DEV_RUNNING_FLG_BIT_NIGHTMODE, true);
	}
	else
	{
		if((devRunningFlg_temp & DEV_RUNNING_FLG_BIT_NIGHTMODE))currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_NIGHTMODE), true);
	}
}

static void funCB_bussinessSoftTimer(void *timer){

	stt_devStatusRecord devStatusRecordFlg_temp = {0};

	static stt_timerLoop loopTimer_1second = {1000, 0};
	static stt_timerLoop loopTimer5s_sntpRefresh = {5 - 1, 0};	
	static stt_timerLoop loopTimerCst_debugPrint = {10 - 1, 0};
	static stt_timerLoop loopTimer6s_devParamRefresh = {6 - 1, 0};
	static stt_timerLoop loopTimer_devSelfLightDrvRefresh = {DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD, 0};

	/*毫秒业务*/
	if(loopTimer_devHeartbeat.loopCounter < loopTimer_devHeartbeat.loopPeriod)loopTimer_devHeartbeat.loopCounter ++; //设备信息周期心跳上传至根节点
	else{
	
		loopTimer_devHeartbeat.loopCounter = 0;
	
		xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_HEARTBEAT);
	}

	if(loopTimer_devElecsumReport.loopCounter < loopTimer_devElecsumReport.loopPeriod)loopTimer_devElecsumReport.loopCounter ++; //主机上报所有设备电量统计信息
	else{

		loopTimer_devElecsumReport.loopCounter = 0;

		xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_DEVEKECSUM_REPORT);
	}

	if(ctrlObj_slidingCalmDownCounter)ctrlObj_slidingCalmDownCounter --; //home界面控件滑动冷却计时业务

	devDriverBussiness_scnarioSwitch_driverClamDown_refresh(); //场景开关驱动冷却时间刷新

	/*100ms特殊*/
	if(loopTimer_devSelfLightDrvRefresh.loopCounter < loopTimer_devSelfLightDrvRefresh.loopPeriod)loopTimer_devSelfLightDrvRefresh.loopCounter ++;
	else{

		loopTimer_devSelfLightDrvRefresh.loopCounter = 0;

		//屏幕背光灯活力刷新业务
		devScreenBkLight_statusRefresh();
	}
	
	/*秒业务*/
	if(loopTimer_1second.loopCounter < loopTimer_1second.loopPeriod)loopTimer_1second.loopCounter ++;
	else{

		loopTimer_1second.loopCounter = 0;

		//设备重启，延时执行时间
		if(devRestartDelay_counter != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){

			if(devRestartDelay_counter){

				devRestartDelay_counter --;

				xQueueSend(msgQh_systemRestartDelayCounterTips, &devRestartDelay_counter, 1 / portTICK_PERIOD_MS);
			}
			else
			{
				devRestartDelay_counter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
				printf("system restart now!.\n");
				esp_restart();
			}
		}

		//wifi配置成功，提示时间
		if(wifiConfigComplete_tipsStartCounter != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){

			if(wifiConfigComplete_tipsStartCounter){

				wifiConfigComplete_tipsStartCounter --;
			}
			else
			{
				extern void lvGui_wifiConfig_bussiness_configComplete_tipsOver(void);
			
				wifiConfigComplete_tipsStartCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
				lvGui_wifiConfig_bussiness_configComplete_tipsOver();
			}
		}

		//设备启动时间计时
		if(devRunningTimeFromPowerUp_couter < (L8_DEV_LISTMANAGE_REALES_CONFIRM + 10))devRunningTimeFromPowerUp_couter ++;

		//debug信息周期性打印业务
		if(loopTimerCst_debugPrint .loopCounter < loopTimerCst_debugPrint .loopPeriod)loopTimerCst_debugPrint .loopCounter ++;
		else{

			loopTimerCst_debugPrint .loopCounter = 0;

//			printf("timer0 Info: H-%d, M-%d, bithold:%d.\n", usrApp_trigTimerGroup[0].tmUp_Hour,
//															 usrApp_trigTimerGroup[0].tmUp_Minute,
//															 usrApp_trigTimerGroup[0].tmUp_weekBitHold);
		}

		//子节点设备信息链表管理
		L8devHbDataManageList_bussinessKeepAliveManagePeriod1s(listHead_nodeDevDataManage);

		//设备本地时间自更新业务 --针对设备离线，无法获取sntp时间
		if(devSysTimeKeep_counter < 3600){

			
		
		}else{ //仅进行时、分、周，业务计算
		
			devSysTimeKeep_counter = 0;

			if(devSystemTime_current.time_Hour >= 23){

				devSystemTime_current.time_Hour = 0;
				(devSystemTime_current.time_Week > 7)?(devSystemTime_current.time_Week = 1):(devSystemTime_current.time_Week ++);
			
			}else{

				devSystemTime_current.time_Hour ++;
			}
		}

		devSystemTime_current.time_Minute = devSysTimeKeep_counter / 60;
		devSystemTime_current.time_Second = devSysTimeKeep_counter % 60;

		//设备关键参数周期性读取更新
		if(loopTimer6s_devParamRefresh.loopCounter < loopTimer6s_devParamRefresh.loopPeriod)loopTimer6s_devParamRefresh.loopCounter ++;
		else{

			loopTimer6s_devParamRefresh.loopCounter = 0;

			devMeshSignalVal_Reales();
			devMeshNodeNum_Reales();
		}

		//普通定时业务更新
		bussiness_usrApp_actTrigTimer();

		//提示性图标显示时间
		if(tipsLoopTimer_keeperCounter)tipsLoopTimer_keeperCounter --;

		//设备状态信息存储 动作延迟业务
		if(devStatusRecord_actionDelayParam.actionTrig_IF){

			if(devStatusRecord_actionDelayParam.timeCounter)devStatusRecord_actionDelayParam.timeCounter --;
			else{

				devStatusRecord_actionDelayParam.actionTrig_IF = 0; //触发标志清零

				devStatusRecordIF_paramGet(&devStatusRecordFlg_temp); //是否开启存储使能
				if(devStatusRecordFlg_temp.devStatusOnOffRecord_IF){

					devInfo_statusRecord_action(); //开关状态存储
				}
			}
		}

		//屏幕运行参数设置 存储动作延迟检测业务
		devScreenDriver_configParamSave_actionDetect();

		//sntp状态刷新业务
		if(loopTimer5s_sntpRefresh.loopCounter < loopTimer5s_sntpRefresh.loopPeriod)loopTimer5s_sntpRefresh.loopCounter ++;
		else{

			loopTimer5s_sntpRefresh.loopCounter = 0;

//			printf("sntp refresh.\n");

			ESP_LOGI(TAG, "DEV time: %04d/%02d/%02d -W_%d %02d:%02d:%02d.\n", devSystemTime_current.time_Year,
																			  devSystemTime_current.time_Month,
																			  devSystemTime_current.time_Day,
																			  devSystemTime_current.time_Week,
																			  devSystemTime_current.time_Hour,
																			  devSystemTime_current.time_Minute,
																			  devSystemTime_current.time_Second);

			if(flgGet_gotRouterOrMeshConnect() && (esp_mesh_get_layer() == MESH_ROOT)){

				if(!flg_stnp_running)sntp_applicationStart();
				else{

					char strftime_buf[64];

					time(&timeNow);
					// Set timezone to China Standard Time
					setenv("TZ", "CST-0", 1);
					tzset();
					localtime_r(&timeNow, &timeInfo);
					strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeInfo);
					
					if(timeNow){
					
						if(devSystemTimeZone.timeZone_H <= 12){
						
							timeNow += (3600UL * (long)devSystemTimeZone.timeZone_H + 60UL * (long)devSystemTimeZone.timeZone_M); //时区正
							
						}else
						if(devSystemTimeZone.timeZone_H > 12 && devSystemTimeZone.timeZone_H <= 24){
						
							timeNow -= (3600UL * (long)(devSystemTimeZone.timeZone_H - 12) + 60UL * (long)devSystemTimeZone.timeZone_M); //时区负
							
						}else
						if(devSystemTimeZone.timeZone_H == 30 || devSystemTimeZone.timeZone_H == 31){ 
							
							timeNow += (3600UL * (long)(devSystemTimeZone.timeZone_H - 17) + 60UL * (long)devSystemTimeZone.timeZone_M); //时区特殊
						}
					}
					localtime_r(&timeNow, &timeInfo);
					devSystimeRealesFromSntp(timeInfo);
					ESP_LOGI(TAG, "UTC time: %s", strftime_buf);
				}
			}
			else
			{
				if(flg_stnp_running)sntp_applicationStop();
			}
		}
	}
}

void usr_loopTimer_tipsKeeper_trig(void){

	tipsLoopTimer_keeperCounter = ICON_TIPSLOOPTIMER_APPEAR_TIME;
}

uint8_t usr_loopTimer_tipsKeeper_read(void){

	return tipsLoopTimer_keeperCounter;
}

void usrApp_bussinessSoftTimer_Init(void){

	timerHld_bussinessSoftTimer = xTimerCreate("usrApp_bussinessSoftTimer", 
											   portTICK_RATE_MS,
									           true, 
									           NULL, 
									           funCB_bussinessSoftTimer);

	xTimerStart(timerHld_bussinessSoftTimer, 0);

	xEventGp_tipsLoopTimer = xEventGroupCreate();
	xEventGp_devApplication = xEventGroupCreate();
	msgQh_systemRestartDelayCounterTips = xQueueCreate(2, sizeof(uint8_t)); //消息数据为当前倒计时时间
}




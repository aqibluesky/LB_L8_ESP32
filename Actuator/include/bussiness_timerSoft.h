/**
 * @file bussiness_timerSoft.h
 *
 */

#ifndef BUSSINESS_TIMERSOFT_H
#define BUSSINESS_TIMERSOFT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

/*********************
 *      DEFINES
 *********************/
#define USRAPP_VALDEFINE_TRIGTIMER_NUM			8

#define LOOPTIMEREVENT_FLG_BITHOLD_RESERVE		(0xFFFF)

#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER1UP		(1 << 0)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER2UP		(1 << 1)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER3UP		(1 << 2)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER4UP		(1 << 3)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER5UP		(1 << 4)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER6UP		(1 << 5)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER7UP		(1 << 6)
#define LOOPTIMEREVENT_FLG_BITHOLD_TIMER8UP		(1 << 7)
#define LOOPTIMEREVENT_FLG_BITHOLD_DELAYUP		(1 << 8)
#define LOOPTIMEREVENT_FLG_BITHOLD_GREENMODEUP	(1 << 9)
#define LOOPTIMEREVENT_FLG_BITHOLD_GUI_BLOCK	(1 << 10)
#define LOOPTIMEREVENT_FLG_BITHOLD_GUI_BLOCKCEL	(1 << 11)
#define LOOPTIMEREVENT_FLG_BITHOLD_FS_TIPS		(1 << 12)
#define LOOPTIMEREVENT_FLG_BITHOLD_FS_TIPS_CEL	(1 << 13)

#define DEV_MQTT_LOGIN_NOTICE_PERIOD			 (15 * 1000) //MQTT首次登陆等待回复通知周期，单位：ms
#define DEV_HEARTBEAT_DATATRANS_PERIOD_LIMIT_MIN (35 * 1000) //心跳周期，单位：ms
#define DEV_ESUMREPORT_DATATRANS_PERIOD			 (20 * 1000) //电量上报周期，单位：ms

#define DEVAPPLICATION_FLG_BITHOLD_RESERVE		 (0xFFFF)

#define DEVAPPLICATION_FLG_BITHOLD_HEARTBEAT			(1 << 0) //心跳事件
#define DEVAPPLICATION_FLG_BITHOLD_MUTUALTRIG			(1 << 1) //互控事件
#define DEVAPPLICATION_FLG_MQTT_LOGIN_NOTICE			(1 << 2) //MQTT登录通知事件
#define DEVAPPLICATION_FLG_BITHOLD_DEVEKECSUM_REPORT	(1 << 3) //MQTT电量信息定时上报事件
#define DEVAPPLICATION_FLG_BITHOLD_DEVDRV_SCENARIO		(1 << 4) //场景设备驱动动作
#define DEVAPPLICATION_FLG_BITHOLD_DEVSTATUS_SYNCHRO	(1 << 5) //设备状态主动同步
#define DEVAPPLICATION_FLG_DEVNODE_UPGRADE_REQUEST		(1 << 6) //子设备升级请求反向通知到主设备
#define DEVAPPLICATION_FLG_DEVNODE_UPGRADE_CHECK		(1 << 7) //设备固件升级可用检查
#define DEVAPPLICATION_FLG_HOST_UPGRATE_TASK_CREAT		(1 << 8) //主机设备触发远程升级时任务创建

#define ICON_TIPSLOOPTIMER_APPEAR_TIME			6	//闹钟响应提示图标显示时间			，单位：s	
#define DEVDP_RESPOND_ACTION_DELAY_TIME			1	//设备数据点触发 --避免高频响应导致crash
 
/**********************
 *      TYPEDEFS
 **********************/
 typedef struct{
 
	 uint16_t 	time_Year;
	 uint8_t 	time_Month;
	 uint8_t 	time_Week;
	 uint8_t 	time_Day;
	 uint8_t 	time_Hour;
	 uint8_t 	time_Minute;
	 uint8_t 	time_Second;
 }stt_localTime;

  typedef struct{
 
	 uint8_t timeZone_H;
	 uint8_t timeZone_M;
 }stt_timeZone;

 typedef struct{

	uint8_t tmUp_weekBitHold:7;	//周占位标志
 	uint8_t tmUp_runningEn:1;	//运行启动使能
 	uint8_t tmUp_Hour:5;		//定时：时
	uint8_t tmUp_swValTrig:3;	//定时触发值		
	uint8_t tmUp_Minute;		//定时：分
 }usrApp_trigTimer;

 typedef struct{

	uint8_t counterUP;
	uint8_t counterDN;

 }stt_kLongPreKeepParam;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void usrApp_bussinessSoftTimer_Init(void);
void deviceParamSet_timeZone(stt_timeZone *param, bool nvsRecord_IF);
void deviceParamGet_timeZone(stt_timeZone *param);
void usrAppActTrigTimer_paramClrReset(void);
void usrAppActTrigTimer_paramSet(usrApp_trigTimer param[USRAPP_VALDEFINE_TRIGTIMER_NUM], bool nvsRecord_IF);
void usrAppActTrigTimer_paramUnitSet(usrApp_trigTimer *param, uint8_t unitNum, bool nvsRecord_IF);
void usrAppActTrigTimer_paramGet(usrApp_trigTimer param[USRAPP_VALDEFINE_TRIGTIMER_NUM]);
void usrAppActTrigTimer_paramUnitGet(usrApp_trigTimer *param, uint8_t unitNum);
void usrAppNightModeTimeTab_paramClrReset(void);
void usrAppNightModeTimeTab_paramSet(usrApp_trigTimer param[2], bool nvsRecord_IF);
void usrAppNightModeTimeTab_paramGet(usrApp_trigTimer param[2]);
void usrAppParamSet_hbRealesInAdvance(bool immediatelyIf);
void usr_loopTimer_tipsKeeper_trig(void);
uint8_t usr_loopTimer_tipsKeeper_read(void);
void usr_guiBlockCounter_trig(uint8_t timeOut);
void usr_tipsFullScreen_trig(uint16_t timeOut);
void usrAppDevCurrentSystemTime_paramSet(stt_localTime *timeParam);
void usrAppDevCurrentSystemTime_paramGet(stt_localTime *timeParam);
void usrAppDevDpResp_actionDelayTrig_funcSet(void);
void lvGui_systemRestartCountingDown_trig(uint8_t secCount);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*BUSSINESS_TIMERSOFT_H*/




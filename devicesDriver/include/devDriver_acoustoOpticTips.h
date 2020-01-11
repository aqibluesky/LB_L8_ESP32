/**
 * @file devDriver_devSelfLight.h
 *
 */

#ifndef DEVDRIVER_DEVSELFLIGHT_H
#define DEVDRIVER_DEVSELFLIGHT_H

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
#define DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD			250			//灯光业务更新时间分量 -sTimer

#define DEVDRIVER_DEVBEEPS_REFRESH_PERIOD				0.00025F	//蜂鸣器业务更新时间分量 -hTime

#define DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV				(100) 		//背光分度

#define DEVDRIVER_DEVSELFLIGHT_SCR_PARAMSAVE_TIMEDELAY	3			//屏幕运行参数存储 动作延迟 时间 单位：s

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	atmosphereLightType_none = 0,
	atmosphereLightType_normalWithoutNet,
	atmosphereLightType_normalNetNode,
	atmosphereLightType_normalNetRoot,
	atmosphereLightType_dataSaveOpreat,
	atmosphereLightType_infraActDetectTrig,
}enum_atmosphereLightType;

typedef enum{

	screenBkLight_statusEmpty = 0,
	screenBkLight_statusHalf,
	screenBkLight_statusFull,
}enum_screenBkLight_status;

typedef struct{

	uint32_t timePeriod_devScreenBkLight_weakDown;
	uint8_t devScreenBkLight_brightness;
	uint8_t devScreenBkLight_brightnessSleep;
}stt_devScreenRunningParam;

typedef struct{

	uint8_t 	trig:1;
	uint8_t 	tones:3;
	uint8_t 	volume:4;
	uint8_t 	loop;

	uint16_t 	keepPeriod;
	uint16_t	pulsePeriod;
	
	uint16_t	runningCounter;
}stt_devBeepRunningParam;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void deviceHardwareAcoustoOptic_Init(void);
void bussiness_devLight_testApp(void);
enum_screenBkLight_status devScreenBkLight_brightnessGet(void);
void devAtmosphere_statusTips_trigSet(enum_atmosphereLightType tipsType);

void tipsOpreatSet_sysUpgrading(uint16_t valSet);
void tipsOpreatAutoSet_sysUpgrading(void);
void tipsSysUpgrading_realesRunning(void);

void devBeepTips_trig(uint8_t tones, uint8_t vol, uint16_t timeKeep, uint16_t timePulse, uint8_t loop);
bool devPirStatus_Get(void);
void devAcoustoOptic_statusRefresh(void);
uint8_t devAcoustoOptic_beepBussinessRefresh(void);
void devScreenBkLight_weakUp(void);

void devScreenDriver_configParamSave_actionDetect(void);
void devScreenDriver_configParam_set(stt_devScreenRunningParam *param, bool nvsRecord_IF);
void devScreenDriver_configParam_brightness_set(uint8_t brightnessVal, bool nvsRecord_IF);
void devScreenDriver_configParam_brightnessSleep_set(uint8_t brightnessSleepVal, bool nvsRecord_IF);
uint8_t devScreenDriver_configParam_brightness_get(void);
uint8_t devScreenDriver_configParam_brightnessSleep_get(void);
void devScreenDriver_configParam_screenLightTime_set(uint32_t timeVal, bool nvsRecord_IF);
uint32_t devScreenDriver_configParam_screenLightTime_get(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVSELFLIGHT_H*/



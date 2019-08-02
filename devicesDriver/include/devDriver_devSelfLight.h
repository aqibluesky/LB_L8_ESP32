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
#define DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD			250			//业务更新时间分量

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void deviceHardwareLight_Init(void);
void bussiness_devLight_testApp(void);
enum_screenBkLight_status devScreenBkLight_brightnessGet(void);
void devAtmosphere_statusTips_trigSet(enum_atmosphereLightType tipsType);
bool devPirStatus_Get(void);
void devScreenBkLight_statusRefresh(void);
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



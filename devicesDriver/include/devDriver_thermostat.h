/**
 * @file devDriver_devThermostat.h
 *
 */

#ifndef DEVDRIVER_DEVTHERMOSTAT_H
#define DEVDRIVER_DEVTHERMOSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDriver_manage.h"

/*********************
 *      DEFINES
 *********************/
#define DEVTHERMOSTAT_RUNNINGDETECT_PERIODLOOP_TIME		1.0F //硬件定时器内循环调用 热水器开关 设备运行逻辑业务 间隔

#define DEVICE_THERMOSTAT_ADJUST_EQUAL_RANGE			16   //温度可调节范围

#define DEVICE_THERMOSTAT_TEMPTARGET_DEFAULT			16	 //默认起始温度

/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	uint16_t deviceRunning_EN:1;
	uint16_t workModeInNight_IF:1;
	uint16_t temperatureVal_target:6;
	uint16_t temperatureVal_current:6;

	uint16_t deviceExSwstatus_rly1:1;
	uint16_t deviceExSwstatus_rly2:1;
}stt_thermostat_actAttr;

typedef struct{

	uint8_t swVal_bit1:1; 
	uint8_t swVal_bit2:1; 

	uint8_t rsv:4;

	uint8_t opCtrl_bit1:1; 
	uint8_t opCtrl_bit2:1; 
}stt_thermostatExSwParam;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_datapointDefaultParamLoad(void);
void devDriverBussiness_thermostatSwitch_moudleInit(void);
void devDriverBussiness_thermostatSwitch_moudleDeinit(void);
void IRAM_ATTR devDriverBussiness_thermostatSwitch_runningDetectLoop(void);
void devDriverBussiness_thermostatSwitch_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_thermostatSwitch_devParam_get(stt_thermostat_actAttr *param);

void devDriverBussiness_thermostatSwitch_exSwitchParamReales(uint8_t param);
void devDriverBussiness_thermostatSwitch_exSwitchParamSet(uint8_t param);
uint8_t devDriverBussiness_thermostatSwitch_exSwitchParamGet(void);
uint8_t devDriverBussiness_thermostatSwitch_exSwitchRcdParamGet(void);
uint8_t devDriverBussiness_thermostatSwitch_exSwitchParamGetWithRcd(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVTHERMOSTAT_H*/






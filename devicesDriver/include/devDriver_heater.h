/**
 * @file devDriver_devHeater.h
 *
 */

#ifndef DEVDRIVER_DEVHEATER_H
#define DEVDRIVER_DEVHEATER_H

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
#define DEVHEATER_RUNNINGDETECT_PERIODLOOP_TIME		0.001F //硬件定时器内循环调用 热水器开关 设备运行逻辑业务 间隔

#define DEVHEATER_COEFFICIENT_TIME_SECOND			(uint32_t)(1.0F / DEVHEATER_RUNNINGDETECT_PERIODLOOP_TIME) //参数操作时 秒系数 
#define DEVHEATER_REALY_SYNCHRONOUS_TIME			1100   //继电器同步间隔时间（大电流继电器向小电流继电器同步）单位：ms

#define DEVICE_HEATER_OPREAT_ACTION_NUM				5		//热水器总共有几个档

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	heaterOpreatAct_close = 0,
	heaterOpreatAct_open,
	heaterOpreatAct_closeAfter30Min,
	heaterOpreatAct_closeAfter60Min,
	heaterOpreatAct_closeAfterTimeCustom,
}stt_devHeater_opratAct;

typedef struct{

	uint32_t relaySynchronousDnCounter;
	
	uint32_t timeUp_period;
	uint32_t timeUp_counter;
	uint8_t  timeCount_En:1;

	uint32_t timeUp_period_customSet;
}stt_devHeater_actParam;

typedef struct{

	stt_devHeater_actParam timeCountParam;
	stt_devHeater_opratAct opreatAct;
}stt_Heater_actAttr;

typedef enum{

	msgType_devHeaterDriver_null = 0,
	msgType_devHeaterDriver_closeCounterChg,	
	msgType_devHeaterDriver_devActChg,		
}enum_msgType_devHeaterDriver;

typedef struct{

	enum_msgType_devHeaterDriver msgType;
	union unn_msgDats_devHeater{

		struct stt_data_closeCounter{
		
			uint32_t counter;
		}data_counterChg;

		struct stt_data_devAct{
		
			stt_devHeater_opratAct act;
		}data_opreatChg;

	}msgDats;
}stt_msgDats_devHeaterDriver;
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern xQueueHandle msgQh_devHeaterDriver;

void devDriverBussiness_heaterSwitch_moudleInit(void);
void devDriverBussiness_heaterSwitch_moudleDeinit(void);
void IRAM_ATTR devDriverBussiness_heaterSwitch_runningDetectLoop(void);
void devDriverBussiness_heaterSwitch_periphStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_heaterSwitch_devParam_Get(stt_Heater_actAttr *param);
uint32_t devDriverBussiness_heaterSwitch_devParam_closeCounter_Get(void);
uint32_t devDriverBussiness_heaterSwitch_closePeriodCustom_Get(void);
void devDriverBussiness_heaterSwitch_closePeriodCustom_Set(uint32_t valSet);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVHEATER_H*/





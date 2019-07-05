/**
 * @file devDriver_devCurtain.h
 *
 */

#ifndef DEVDRIVER_DEVCURTAIN_H
#define DEVDRIVER_DEVCURTAIN_H

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
#define DEVCURTAIN_RUNNINGDETECT_PERIODLOOP_TIME	0.001F

#define DEVICE_CURTAIN_ORBITAL_POSITION_MAX_VAL   	100 //轨道时间对应位置平分值最大值

#define DEVICE_CURTAIN_OPREATION_EXTRA_TIME			200 //窗帘触底操作回弹到停止状态的缓冲时间                    单位：ms

/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	uint32_t act_counter;
	uint32_t act_period;
	
}stt_devCurtain_runningParam;

typedef enum{
	
	curtainRunningStatus_cTact_stop = 2,
	curtainRunningStatus_cTact_close = 4,
	curtainRunningStatus_cTact_open = 1,

	curtainRunningStatus_cTact_custom = 9,
	
}stt_devCurtain_runningStatus;

typedef enum{

	msgType_devCurtainDriver_null = 0,
	msgType_devCurtainDriver_opreatStop,
	msgType_devCurtainDriver_orbitalChgingBySlider,
	msgType_devCurtainDriver_orbitalChgingByBtn,
}enum_msgType_devCurtainDriver;

typedef struct{

	enum_msgType_devCurtainDriver msgType;
	union unn_msgDats_devCurtain{

		struct stt_data_opreatStop{

			uint8_t opreatStop_sig:1;
		}data_opreatStop;

		struct stt_data_orbitalChgingByBtn{

			uint8_t orbitalPosPercent;
		}data_orbitalChgingByBtn;

		struct stt_data_orbitalChgingBySlider{

			uint8_t orbitalPosPercent;
		}data_orbitalChgingBySlider;

	}msgDats;
}stt_msgDats_devCurtainDriver;

typedef struct{ //窗帘数据结构体

	stt_devCurtain_runningParam devRunningParam;
	stt_devCurtain_runningStatus act;
}stt_Curtain_motorAttr;
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern xQueueHandle msgQh_devCurtainDriver;

void devDriverBussiness_curtainSwitch_moudleInit(void);
void devDriverBussiness_curtainSwitch_moudleDeinit(void);
void devCurtain_runningParamSet(stt_devCurtain_runningParam *param, bool nvsRecord_IF);
void devCurtain_currentOrbitalPeriodTimeSet(uint8_t timeVal);
bool IRAM_ATTR devDriverBussiness_curtainSwitch_devRunningDetect(void);
uint8_t devCurtain_currentPositionPercentGet(void);
stt_devCurtain_runningStatus devCurtain_currentRunningStatusGet(void);
void devCurtain_runningParamGet(stt_devCurtain_runningParam *param);
void devCurtain_ocrbitalTimeSaveInitiative(void);
void devDriverBussiness_curtainSwitch_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVCURTAIN_H*/





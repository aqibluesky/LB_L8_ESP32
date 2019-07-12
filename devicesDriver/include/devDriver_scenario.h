/**
 * @file devDriver_devScenario.h
 *
 */

#ifndef DEVDRIVER_DEVSCENARIO_H
#define DEVDRIVER_DEVSCENARIO_H

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
#define DEVSCENARIO_OPREATION_OBJ_NUM		3 		//场景开关 单个设备 最大操作场景个数
#define DEVSCENARIO_DRIVER_CALMDOWN_PERIOD	4000	//场景开关 驱动触发 冷却时间 单位：ms	

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_scnarioSwitch_moudleInit(void);
void devDriverBussiness_scnarioSwitch_moudleDeinit(void);
void devDriverBussiness_scnarioSwitch_driverClamDown_refresh(void);
uint16_t devDriverBussiness_scnarioSwitch_driverClamDown_get(void);
void devDriverBussiness_scnarioSwitch_dataParam_save(stt_scenarioSwitchData_nvsOpreat *param);
uint8_t devDriverBussiness_scnarioSwitch_swVal2Insert(uint8_t swVal);
void devDriverBussiness_scnarioSwitch_scenarioStatusReales(stt_devDataPonitTypedef *param);
void devDriverBussiness_scnarioSwitch_actionTrig(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVSCENARIO_H*/




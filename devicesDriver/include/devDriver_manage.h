/**
 * @file devDriver_Manage.h
 *
 */

#ifndef DEVDRIVER_MANAGE_H
#define DEVDRIVER_MANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDataManage.h"

#include "devDriver_devSelfLight.h"

#include "devDriver_curtain.h"
#include "devDriver_dimmer.h"
#include "devDriver_fans.h"
#include "devDriver_heater.h"
#include "devDriver_mulitBitSW.h"
#include "devDriver_scenario.h"
#include "devDriver_elecMeasure.h"
#include "devDriver_temperatureMeasure.h"
#include "devDriver_thermostat.h"
#include "devDriver_infraActDetect.h"

/*********************
 *      DEFINES
 *********************/
#define DEVICE_TYPE_LIST_NUM			9
#define HOMEPAGE_THEMETYPE_NUM			3

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
devTypeDef_enum IRAM_ATTR currentDev_typeGet(void);
void currentDev_typeSet(devTypeDef_enum devType, bool nvsRecord_IF);
void devStatusRecordIF_paramSet(stt_devStatusRecord *param, bool nvsRecord_IF);
void devStatusRecordIF_paramGet(stt_devStatusRecord *param);
void currentDev_dataPointGet(stt_devDataPonitTypedef *param);
void currentDev_dataPointSet(stt_devDataPonitTypedef *param, bool nvsRecord_IF, bool mutualCtrlTrig_IF, bool statusUploadMedthod);
void currentDev_extParamSet(void *param);
void devInfo_statusRecord_action(void);
void funcation_usrAppMutualCtrlActionTrig(void);
void devDriverManageBussiness_initialition(void);
void devDriverManageBussiness_deinitialition(void);
void devDriverManageBussiness_deviceChangeRefresh(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_MANAGE_H*/




















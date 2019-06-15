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
#define DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD		250

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

	uint8_t linkageWith_screenBkLight:1;
	uint8_t linkageWith_swRelay:1;
}stt_infraActDetectCombineFLG;

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVSELFLIGHT_H*/



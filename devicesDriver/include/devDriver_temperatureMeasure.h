/**
 * @file devDriver_temperature.h
 *
 */

#ifndef DEVDRIVER_TEMPERATUREMEASURE_H
#define DEVDRIVER_TEMPERATUREMEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDataManage.h"

/*********************
 *      DEFINES
 *********************/
#define DEVDRIVER_TEMPERATUREDETECT_PERIODLOOP_TIME		10.0F

#define DEVDRIVER_TEMPERATUREDETECT_ADC_CHECKTAB_LEN	96

/**********************
 *      TYPEDEFS
 **********************/
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_temperatureMeasure_periphInit(void);
float devDriverBussiness_temperatureMeasure_temperatureReales(void);
float devDriverBussiness_temperatureMeasure_get(void);
void devDriverBussiness_temperatureMeasure_getByHex(stt_devTempParam2Hex *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_TEMPERATUREMEASURE_H*/







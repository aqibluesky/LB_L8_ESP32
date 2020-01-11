/**
 * @file devDriver_devDimmer.h
 *
 */

#ifndef DEVDRIVER_DEVDIMMER_H
#define DEVDRIVER_DEVDIMMER_H

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
 #define DEVICE_DIMMER_BRIGHTNESS_MAX_VAL	100

/**********************
 *      TYPEDEFS
 **********************/
 typedef struct{ //

	uint16_t periodBeat_cfm; //
	uint16_t periodBeat_counter; //

	uint16_t pwm_actPeriod;
	
	uint16_t pwm_actEN:1; //
	uint16_t pwm_actCounter:15; //

	uint16_t couterSoureFreq_Debug;
	uint16_t periodSoureFreq_Debug;
}stt_Dimmer_attrFreq;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_dimmerSwitch_moudleInit(void);
void devDriverBussiness_dimmerSwitch_moudleDeinit(void);
void IRAM_ATTR devDriverBussiness_dimmerSwitch_decCounterReales(void);
void IRAM_ATTR devDriverBussiness_dimmerSwitch_debug(void);
void IRAM_ATTR devDriverBussiness_dimmerSwitch_currentParamGet(stt_Dimmer_attrFreq *param);
void IRAM_ATTR devDriverBussiness_dimmerSwitch_brightnessAdjDetectLoop(void);
uint8_t devDriverBussiness_dimmerSwitch_brightnessLastGet(void);
void devDriverBussiness_dimmerSwitch_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVDIMMER_H*/





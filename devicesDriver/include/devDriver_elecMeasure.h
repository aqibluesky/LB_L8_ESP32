/**
 * @file devDriver_elecMeasure.h
 *
 */

#ifndef DEVDRIVER_ELECMEASURE_H
#define DEVDRIVER_ELECMEASURE_H

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
#define DEVDRIVER_ELECMEASURE_FDETECT_PERIOD					5.0F

#define DEVDRIVER_ELECMEASURE_COEFFICIENT_POW					4.411065F
#define DEVDRIVER_ELECMEASURE_COEFFICIENT_COMPENSATION_POW		0.000001F

/**********************
 *      TYPEDEFS
 **********************/
 typedef struct{ //

	float eleParam_power; //
	float eleParamFun_powerPulseCount; //
	float eleParamFun_powerFreqVal; //
	
	float ele_Consum;
	
}stt_eleSocket_attrFreq;
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_elecMeasure_periphInit(void);
float devDriverBussiness_elecMeasure_powerCaculateReales(void);
float devDriverBussiness_elecMeasure_valElecPowerGet(void);
float devDriverBussiness_elecMeasure_valElecConsumGet(void);
void devDriverBussiness_elecMeasure_valPowerGetByHex(stt_devPowerParam2Hex *param);
void devDriverBussiness_elecMeasure_valElecsumGetByHex(stt_devElecsumParam2Hex *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_ELECMEASURE_H*/






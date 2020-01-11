/**
 * @file mechanical_bussinessOpreat.h
 *
 */

#ifndef TIPS_MECHANICAL_BUSSINESSOPREAT_H
#define TIPS_MECHANICAL_BUSSINESSOPREAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "iot_lvgl.h"

#include "devDriver_manage.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum{

	press_Null = 0,
	press_Short,
	press_ShortCnt, //
	press_LongA,
	press_LongB,
}keyCfrm_Type;

typedef struct{

	uint8_t param_combinationFunPreTrig_standBy_FLG:1; //
	uint8_t param_combinationFunPreTrig_standBy_keyVal:7; //
}param_combinationFunPreTrig;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devMechanicalOpreatPeriphInit(void);

void devOpreation_bussinessInit(void);
void devMechanicalOpreatTimeCounter_realesing(void);

void deviceTypeDefineByDcode_preScanning(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*TIPS_MECHANICAL_BUSSINESSOPREAT_H*/






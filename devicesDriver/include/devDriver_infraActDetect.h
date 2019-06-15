/**
 * @file devDriver_infraActDetect.h
 *
 */

#ifndef DEVDRIVER_INFRAACTDETECT_H
#define DEVDRIVER_INFRAACTDETECT_H

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
//#define NEC_DATATX_RELOAD(x)							((~x) << 8) | x)

#define DEVDRIVER_INFRAACTDETECT_PERIODLOOP_TIME		0.25F //频率探测周期

#define INFRAACTDETECTEVENT_FLG_BITHOLD_RESERVE			(0xFFFF)
#define INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN		(1 << 0)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct{

	float infraActDetect_PulseCount; //
	float infraActDetect_FreqVal; //

}stt_infraActDetect_attrFreq;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_infraActDetect_periphInit(void);
bool devDriverBussiness_infraActDetect_detectReales(void);
float devDriverBussiness_infraActDetect_freqRcvGet(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_INFRAACTDETECT_H*/

















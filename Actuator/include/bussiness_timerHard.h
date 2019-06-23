/**
 * @file bussiness_timerHard.h
 *
 */

#ifndef BUSSINESS_TIMERHARD_H
#define BUSSINESS_TIMERHARD_H

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

/**********************
 *      TYPEDEFS
 **********************/
typedef union{

	struct unn_msgAboutDevDelayTrig{

		stt_devDataPonitTypedef devStatusValSet;
		uint8_t nvsRecord_IF:1;
		uint8_t mutualCtrlTrig_IF:1;
		
		uint8_t devRunningFlgReales_IF:1;
		
	}msgDataAbout_DevDelayTrig;

	struct unn_msgAboutDevGreenMode{

		stt_devDataPonitTypedef devStatusValSet;
		uint8_t nvsRecord_IF:1;
		uint8_t mutualCtrlTrig_IF:1;

	}msgDataAbout_DevGreenMode;

}unn_msgDataFromHwTimer;

typedef struct{

	enum{

		msgType_null = 0,
		msgType_DevDelayTrig,
		msgType_DevGreenMode,
			
	}msgFromHwTimerType;

	unn_msgDataFromHwTimer msgData;

}stt_msgFromHwTimer;

enum{

	_driverDataType_debug = 0,
	_driverDataType_dimer,
	_driverDataType_curtain,
	_driverDataType_elecMeasure,
	_driverDataType_temprature,
	_driverDataType_infraActDetect,
	_driverDataType_thermostatDriving,
};

typedef struct{

	uint8_t driverDataType_discrib;

	union{

		struct{

			uint8_t dataDebug;
		}_debug_dats;

		struct{

			uint16_t freqSource;
			uint16_t freqLoad;
		}_dimmer_dats;

		struct{

			uint8_t curtainOrbitalTimeSave:1;
		}_curtain_dats;

		struct{

			uint8_t measurePeriod_notice:1;
		}_elecMeasure_dats;

		struct{

			uint8_t measurePeriod_notice:1;
		}_temperatureMeasure_dats;

		struct{

			uint8_t detectPeriod_notice:1;
		}_infraActDetect_dats;

		struct{

			uint8_t drivePeriod_notice:1;
		}_thermostatDriver_dats;
	}driverDats;
}stt_msgDriverHwTimer;
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void usrApp_bussinessHardTimer_Init(void);
void usrAppParamSet_devGreenMode(uint8_t paramCst[2], bool nvsRecord_IF);
void usrAppParamGet_devGreenMode(uint8_t paramCst[2]);
void usrApp_GreenMode_trig(void);
void usrAppParamSet_devDelayTrig(uint8_t paramCst[3]);
void usrAppParamGet_devDelayTrig(uint8_t paramCst[3]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*BUSSINESS_TIMERHARD_H*/





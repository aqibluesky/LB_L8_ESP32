/**
 * @file devDriver_infrared.h
 *
 */

#ifndef DEVDRIVER_INFRARED_H
#define DEVDRIVER_INFRARED_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDriver_manage.h"

#include "bussiness_timerSoft.h"

/*********************
 *      DEFINES
 *********************/
#define DEVINFRARED_RUNNINGDETECT_PERIODLOOP_TIME		   0.001F

#define DEVINFRARED_HXD019D_opsLearnningSTBY_TOUT		   2000    //
#define DEVINFRARED_HXD019D_opsLearnningSTBY_TOUTLOOP	   3	   //
#define DEVINFRARED_HXD019D_opsLearnning_TOUT			   20000   //
#define DEVINFRARED_HXD019D_opsSigSendSTBY_TOUT 		   300	   //
#define DEVINFRARED_HXD019D_resetOpreatTimeKeep			   200	   //

#define DEVINFRARED_OPREATCMD_CONTROL				  	   0x20    //
#define DEVINFRARED_OPREATCMD_LEARNNING				   	   0x21    //
#define DEVINFRARED_OPREATRES_CONTROL				   	   0x87    //
#define DEVINFRARED_OPREATRES_LEARNNING				   	   0x86    //

/**********************
 *      TYPEDEFS
 **********************/
 typedef enum{

	infraredSMStatus_null = 0, //
	infraredSMStatus_free, //
	infraredSMStatus_learnningSTBY, //
	infraredSMStatus_learnning, //
	infraredSMStatus_sigSendSTBY, //
	infraredSMStatus_sigSend, //
	infraredSMStatus_opStop //
}enumInfrared_status;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
uint8_t IRAM_ATTR devDriverBussiness_infraredSwitch_runningDetectLoop(void);

enumInfrared_status devDriverBussiness_infraredStatus_get(void);

uint8_t devDriverBussiness_infraredSwitch_currentOpreatNumGet(void);

void devDriverBussiness_infraredSwitch_timerUpTrigIstTabSet(uint8_t istTab[USRAPP_VALDEFINE_TRIGTIMER_NUM], bool nvsRecord_IF);
void devDriverBussiness_infraredSwitch_timerUpTrigIstTabGet(uint8_t istTab[USRAPP_VALDEFINE_TRIGTIMER_NUM]);

void devDriverBussiness_infraredSwitch_moudleInit(void);
void devDriverBussiness_infraredSwitch_moudleDeinit(void);
void devDriverBussiness_infraredSwitch_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_INFRARED_H*/


















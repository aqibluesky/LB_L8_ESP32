/**
 * @file dataTrans_remoteServer.h
 *
 */

#ifndef DATATRANS_REMOTESERVER_H
#define DATATRANS_REMOTESERVER_H

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
#define USRAPP_MQTT_REMOTEREQ_DATAPACK_PER_LENGTH		896	//MQTT数据传输单包长度限制

#define USRDEF_MQTT_DTSURGE_PREVENT_KPTIME				1500 //MQTT连接后残留信息消除等待时间

#define USRDEF_MQTT_DEVCONNECT_NOTICE_LOOP_MAX			10	//mqtt主机上线时，mac更换notice提示

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_rootDevRemoteDatatransLoop_elecSumReport(void);
void mqtt_rootDevLoginConnectNotice_trig(void);
void mqtt_remoteDataTrans(uint8_t dtCmd, uint8_t *data, uint16_t dataLen);
void usrApp_devRootStatusSynchronousInitiative(void);
void usrApp_deviceStatusSynchronousInitiative(void);
void devFireware_upgradeReserveCheck(void);
void devFireware_upgradeReserveCheck_trigByEvent(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DATATRANS_REMOTESERVER_H*/




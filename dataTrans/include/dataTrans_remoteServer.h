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

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_rootDevRemoteDatatransLoop_elecSumReport(void);
void mqtt_remoteDataTrans(uint8_t dtCmd, uint8_t *data, uint16_t dataLen);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DATATRANS_REMOTESERVER_H*/




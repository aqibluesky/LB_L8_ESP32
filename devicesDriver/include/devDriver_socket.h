/**
 * @file devDriver_socket.h
 *
 */

#ifndef DEVDRIVER_SOCKET_H
#define DEVDRIVER_SOCKET_H

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


/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_socketSwitch_moudleInit(void);
void devDriverBussiness_socketSwitch_moudleDeinit(void);
void devDriverBussiness_socketSwitch_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_SOCKET_H*/


















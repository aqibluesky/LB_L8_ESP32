/**
 * @file devDriver_devMulitBitSW.h
 *
 */

#ifndef DEVDRIVER_DEVMULITBITSW_H
#define DEVDRIVER_DEVMULITBITSW_H

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
 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void devDriverBussiness_mulitBitSwitch_moudleInit(void);
void devDriverBussiness_mulitBitSwitch_moudleDeinit(void);
void devDriverBussiness_mulitBitSwitch_periphStatusReales(stt_devDataPonitTypedef *param);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_DEVMULITBITSW_H*/





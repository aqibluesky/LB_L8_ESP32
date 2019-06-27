/**
 * @file dataTrans_localHandler.h
 *
 */

#ifndef DATATRANS_LOCALHANDLER_H
#define DATATRANS_LOCALHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

/*********************
 *      DEFINES
 *********************/
#define DEVMQTT_MESH_DATAREQ_TEMPLEN		64

#define L8DEV_MESH_CMD_CONTROL				0x10 /*R2N*/
#define L8DEV_MESH_CMD_DEVLOCK				0x17 /*R2N*/
#define L8DEV_MESH_CMD_DEVRESET				0x16 /*R2N*/
#define L8DEV_MESH_CMD_SET_MUTUALCTRL		0x41 /*R2N*/
#define L8DEV_MESH_CMD_SCENARIO_SET			0x45 /*R2N*/
#define L8DEV_MESH_CMD_SCENARIO_CTRL		0x47 /*R2N*/
#define L8DEV_MESH_CMD_CTRLOBJ_TEXTSET		0x50 /*R2N*/
#define L8DEV_MESH_CMD_CTRLOBJ_ICONSET		0x51 /*R2N*/
#define L8DEV_MESH_CMD_UISET_THEMESTYLE		0x52 /*R2N*/
#define L8DEV_MESH_CMD_EXT_PARAM_SET		0x53 /*R2N*/
#define	L8DEV_MESH_CMD_NEIWORK_PARAM_CHG	0x54 /*R2N*/

#define L8DEV_MESH_CMD_DEV_STATUS			0x11 /*R2N | N2R*/
#define L8DEV_MESH_CMD_SET_TIMER			0xA0 /*R2N | N2R*/
#define L8DEV_MESH_CMD_SET_DELAY			0xA1 /*R2N | N2R*/
#define L8DEV_MESH_CMD_SET_GREENMODE		0xA2 /*R2N | N2R*/
#define L8DEV_MESH_CMD_SET_NIGHTMODE		0xA3 /*R2N | N2R*/

#define L8DEV_MESH_CMD_MUTUAL_CTRL			0xEA /*N2N*/

#define L8DEV_MESH_CMD_DEVINFO_GET			0xFA /*R2N*/

#define L8DEV_MESH_SYSTEMTIME_BOARDCAST		0xFC /*R2N*/

#define L8DEV_MESH_HEARTBEAT_REQ			0xFB /*N2R*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void dataHandler_devNodeMeshData(const uint8_t *src_addr, const mlink_httpd_type_t *type, const void *dataRx, size_t dataLen);

void devHeartbeat_dataTrans_bussinessTrig(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DATATRANS_LOCALHANDLER_H*/




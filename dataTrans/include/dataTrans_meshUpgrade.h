/**
 * @file dataTrans_meshUpgrade.h
 *
 */

#ifndef DATATRANS_MESHUPGRADE_H
#define DATATRANS_MESHUPGRADE_H

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
#define L8_FIREWARE_UPGRADE_URL	"http://47.52.5.108:8080"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum __emStatus_upgrading{

	status_upgNull = -1,
	status_upgStandBy = 0, //准备
	status_upgTransit, //中转
	status_upgRunning, //正在进行
	status_upgDone, //完成
}statusEnum_devUpgrading;

typedef struct{

	uint8_t  downloading_if:1;
	uint32_t firwareTotal_size;
	uint32_t firwareRecv_size;
}stt_statusParam_httpUpgrade;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void mwifiApp_firewareUpgrade_nodeNoticeToRoot(void);
void usrAppUpgrade_remoteIPset(uint8_t ip[4]);
void usrAppUpgrade_firewareNameSet(const char *fName);
void usrAppUpgrade_targetDevaddrSet(const uint8_t addr[MWIFI_ADDR_LEN]);

void usrApp_httpUpgradeProgress_paramGet(stt_statusParam_httpUpgrade *param);
void upgradeMeshOTA_taskCreatAction(void);
void usrApp_firewareUpgrade_trig(bool mulitUpgrade_if, uint8_t devtype);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DATATRANS_MESHUPGRADE_H*/





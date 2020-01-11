/**
 * @file devDriver_Manage.h
 *
 */

#ifndef DEVDRIVER_MANAGE_H
#define DEVDRIVER_MANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "devDataManage.h"

#include "devDriver_acoustoOpticTips.h"

#include "devDriver_curtain.h"
#include "devDriver_dimmer.h"
#include "devDriver_fans.h"
#include "devDriver_heater.h"
#include "devDriver_mulitBitSW.h"
#include "devDriver_scenario.h"
#include "devDriver_elecMeasure.h"
#include "devDriver_temperatureMeasure.h"
#include "devDriver_thermostat.h"
#include "devDriver_infraActDetect.h"
#include "devDriver_infrared.h"
#include "devDriver_socket.h"

/*********************
 *      DEFINES
 *********************/
#define L8_DEVICE_DEVELOPE_VERSION_A			0x0A //--flash  4MB
#define L8_DEVICE_DEVELOPE_VERSION_B			0x0B //--flash  8MB
#define L8_DEVICE_DEVELOPE_VERSION_C			0x0C //--flash 16MB

#define L8_DEVICE_DEVELOPE_VERSION_DEF			L8_DEVICE_DEVELOPE_VERSION_B

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DRVMETHOD_BY_SLAVE_MCU_UART_TXD 	  (GPIO_NUM_12)
 #define DRVMETHOD_BY_SLAVE_MCU_UART_RXD 	  (GPIO_NUM_14)

 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)

	   typedef struct _stt_RMTest_pRcd{
	   
		   uint32_t timeRecord;
		   uint32_t relayActLoop;
	   }stt_RMTest_pRcd;

	   typedef struct _stt_relayMagTestParam{
	   
			   stt_RMTest_pRcd dataRcd;
	   
			   const uint16_t relayActPeriod; //翻转保持周期
			   uint16_t relayActCounter; //翻转计数
			   
			   uint16_t relayStatus_actFlg:3; //动作状态
			   uint16_t relayTest_EN:1; //测试开始/停止
	   }stt_relayMagTestParam;
  #endif
#else
  
#endif

#define DEVICE_TYPE_LIST_NUM					10
#define HOMEPAGE_THEMETYPE_NUM					3

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void currentDev_dataPointRecovery(stt_devDataPonitTypedef *param);
uint8_t deviceTypeVersionJudge(uint8_t devType);
bool deviceFistRunningJudge_get(void);
devTypeDef_enum IRAM_ATTR currentDev_typeGet(void);
void currentDev_typeSet(devTypeDef_enum devType, bool nvsRecord_IF);
void devStatusRecordIF_paramSet(stt_devStatusRecord *param, bool nvsRecord_IF);
void devStatusRecordIF_paramGet(stt_devStatusRecord *param);
void deviceDatapointSynchronousReport_actionTrig(void);
void devDriverParamChg_dataRealesTrig(bool nvsRecord_IF, 
										  	   bool mutualCtrlTrig_IF, 
										  	   bool statusUploadMedthod,
										  	   bool synchronousReport_IF);
void currentDev_dataPointGet(stt_devDataPonitTypedef *param);
void currentDev_dataPointRcdGet(stt_devDataPonitTypedef *param);
void currentDev_dataPointGetwithRecord(stt_devDataPonitTypedef *param);
void currentDev_dataPointSet(stt_devDataPonitTypedef *param, 
													   bool nvsRecord_IF, 
													   bool mutualCtrlTrig_IF, 
													   bool statusUploadMedthod, 
													   bool synchronousReport_IF);
void currentDev_extParamSet(void *param);
void currentDev_extParamGet(uint8_t paramTemp[DEVPARAMEXT_DT_LEN]);
void devDriverApp_responseAtionTrig_delay(void);
void funcation_usrAppMutualCtrlActionTrig(void);
void devDriverManageBussiness_initialition(void);
void devDriverManageBussiness_deinitialition(void);
void devDriverManageBussiness_deviceChangeRefresh(void);
bool devStatusDispMethod_landscapeIf_get(void);

void isrHandleFuncPcntUnit_regster(void (*funcHandle)(uint32_t evtStatus), uint8_t funcIst);

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)

  void debugTestMagRelay_paramSave(void);
  void debugTestMagRelay_paramSet(stt_RMTest_pRcd *param, bool nvsRecord_IF);
  void debugTestMagRelay_paramGet(stt_RMTest_pRcd *param);
 #endif
 
 void devDriverApp_statusExexuteBySlaveMcu(uint8_t status);
 void devDriverAppWithCompare_statusExexuteBySlaveMcu(uint8_t status);
 void devDriverAppFromISR_statusExexuteBySlaveMcu(uint8_t status);
 void devDriverApp_statusExexuteBySlaveMcu_byBit(uint8_t bitHold, bool status);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDRIVER_MANAGE_H*/




















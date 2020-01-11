#include "devDriver_manage.h"

#include "string.h"
#include "stdio.h"

#include "driver/pcnt.h"

#include "dataTrans_localHandler.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "bussiness_timerSoft.h"
#include "bussiness_timerHard.h"

extern void pageHome_buttonMain_imageRefresh(bool freshNoRecord);
extern void devStatusRunning_tipsRefresh(void);

extern EventGroupHandle_t xEventGp_devApplication;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
 #define UART_SLAVE_MCU_FIFO_RX_LEN		32

 #define UART_DEVDRV_PROTOCOL_HEAD_M	0xEF
 #define UART_DEVDRV_PROTOCOL_HEAD_S	0xEE	

 #define MCUDRV_PIN_74595_DIN			27		
 #define MCUDRV_PIN_74595_STK			12
 #define MCUDRV_PIN_74595_CLK			14

 #define MCUDRV_OPT_DIN(a)				gpio_set_level(MCUDRV_PIN_74595_DIN, (uint32_t)a)
 #define MCUDRV_OPT_STK(a)				gpio_set_level(MCUDRV_PIN_74595_STK, (uint32_t)a)
 #define MCUDRV_OPT_CLK(a)				gpio_set_level(MCUDRV_PIN_74595_CLK, (uint32_t)a)

 typedef struct __stt_devDrvBySmcuFormat{
 
	 uint8_t cmd;
	 uint8_t data;
	 
 }stt_devDrvBySmcuFormat;

 typedef struct __stt_uartProtocolFormat{

	uint8_t pHead;
	uint8_t command;
	uint8_t data;
	uint8_t pTailCheck;
	
}stt_protocolFmtUartDevDrv;

 static const uart_port_t uartPortSlaveMcu = UART_NUM_1;
 
 static xQueueHandle queueHandle_devDrvBySmcu = NULL;

 static volatile struct __stt_paramUartDataRcv{

	uint8_t dataRcv_toutFlg:1;
	uint8_t dataRcv[UART_SLAVE_MCU_FIFO_RX_LEN];
	uint8_t dataLenRcv;
 }paramSlaveMcuUartDataRcv = {0};

 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)

	volatile stt_relayMagTestParam paramMagRelayTest = {

		.relayActPeriod = 1000,
	};
 #endif
		
#else

#endif

static const char *TAG = "lanbon_L8 - devDriverManage";

static stt_devDataPonitTypedef lanbon_l8device_currentDataPoint = {0};
static stt_devDataPonitTypedef deviceDataPointRecord_lastTime = {0}; //数据点记录比对值，用于记录上一数据点
static devTypeDef_enum	lanbon_l8device_currentDevType = L8_DEVICE_TYPE_DEFULT;

static stt_devStatusRecord deviceStatusRecordIF_Flg = {

	.devStatusOnOffRecord_IF 	= 0,
	.devElecsumDisp_IF 		 	= 1,
	.devScreenLandscape_IF	 	= 0,
	.devUpgradeFirstRunning_FLG = 0,
};

static bool devDpTrig_funcFlg_nvsRecord = false;
static bool devDpTrig_funcFlg_mutualCtrlTrig = false;
static bool devDpTrig_funcFlg_statusUploadMedthod = false;
static bool devDpTrig_funcFlg_synchronousReport = false;

static pcnt_isr_handle_t isrPcnt_applicationHandle = NULL;
static uart_isr_handle_t isrUart_applicationHandle = NULL;

static void (*pcntDeviceDriver_handleFunc_list[8])(uint32_t evtStatus) = {NULL};

static void usrAppMutualCtrl_actionTrig(void);

static void IRAM_ATTR isrHandleFuncPcnt_deviceDriverApplication(void* arg){

    uint32_t intr_status = PCNT.int_st.val;
    int i;

    for (i = 0; i < PCNT_UNIT_MAX; i++) {
		
        if (intr_status & (BIT(i))) {
			
			pcntDeviceDriver_handleFunc_list[i](PCNT.status_unit[i].val);
			
            PCNT.int_clr.val = BIT(i);	
        }
    }
}

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)
	   
	void debugTestMagRelay_paramSet(stt_RMTest_pRcd *param, bool nvsRecord_IF){

		memcpy(&paramMagRelayTest.dataRcd, param, sizeof(stt_RMTest_pRcd));
		if(nvsRecord_IF)
			devSystemInfoLocalRecord_save(saveObj_paramMagRelayTest, &paramMagRelayTest.dataRcd);
	}

	void debugTestMagRelay_paramGet(stt_RMTest_pRcd *param){

		memcpy(param, &paramMagRelayTest.dataRcd, sizeof(stt_RMTest_pRcd));
	}

 	void debugTestMagRelay_paramSave(void){

		devSystemInfoLocalRecord_save(saveObj_paramMagRelayTest, &paramMagRelayTest.dataRcd);
	}
 #endif
 
static void IRAM_ATTR isrHandleFuncUart_deviceDriverBySlaveMcu(void* arg){

	volatile uart_dev_t *uart = &UART1;
	uint8_t recSize = 0;
	
	uart->int_clr.rxfifo_full = 1;
	uart->int_clr.frm_err = 1;
	
	if(uart->int_st.rxfifo_tout){ //检查是否产生超时中断
	
		uart->int_clr.rxfifo_tout = 1;
		
		paramSlaveMcuUartDataRcv.dataLenRcv = uart->status.rxfifo_cnt;
		
		if(paramSlaveMcuUartDataRcv.dataLenRcv){

			uint8_t loop = 0;
			
			paramSlaveMcuUartDataRcv.dataRcv_toutFlg = 1;
		
			while(uart->status.rxfifo_cnt)
			{
				paramSlaveMcuUartDataRcv.dataRcv[loop ++] = uart->fifo.rw_byte; 
			}
		} 
	} 
}

static uint8_t frame_Check(unsigned char frame_temp[], uint8_t check_num){
  
	unsigned char loop 		= 0;
	unsigned char val_Check = 0;
	
//	for(loop = 0; loop < check_num; loop ++){
//	
//		val_Check += frame_temp[loop];
//	}
//	
//	val_Check  = ~val_Check;
//	val_Check ^= 0xa7;
	
	for(loop = 0; loop < check_num; loop ++){
	
		val_Check ^= frame_temp[loop];
	}
	
	return val_Check;
}

static uint8_t ctrlRelayBy74595_optValGet(uint8_t relayStatus){

	uint8_t res = 0;

	struct __stt_74595optTemp{

		uint8_t relayMag1_pinOn:1;
		uint8_t relayMag1_pinOff:1;
		uint8_t relayMag2_pinOn:1;
		uint8_t relayMag2_pinOff:1;
		uint8_t relayMag3_pinOn:1;
		uint8_t lightCon_UD:1;
		uint8_t lightCon_LR:1;
		uint8_t relayMag3_pinOff:1;
	}optTemp_74595 = {0};

	(relayStatus & (1 << 0))?
		(optTemp_74595.relayMag1_pinOn = 1):
		(optTemp_74595.relayMag1_pinOff = 1);
	(relayStatus & (1 << 1))?
		(optTemp_74595.relayMag2_pinOn = 1):
		(optTemp_74595.relayMag2_pinOff = 1);
	(relayStatus & (1 << 2))?
		(optTemp_74595.relayMag3_pinOn = 1):
		(optTemp_74595.relayMag3_pinOff = 1);

	memcpy(&res, &optTemp_74595, sizeof(uint8_t));

	return res;
}

static void processTask_deviceDriverBySlaveMcu(void *arg){

	stt_devDrvBySmcuFormat rptr_drvByMcu = {0};

	static uint16_t localCounter = 0;

	for(;;){

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
//		if(paramSlaveMcuUartDataRcv.dataRcv_toutFlg){

//			paramSlaveMcuUartDataRcv.dataRcv_toutFlg = 0;

//			printf("slaveMcuUartDataRcv[len:%d].\n", paramSlaveMcuUartDataRcv.dataLenRcv);	
//		}

		if(xQueueReceive(queueHandle_devDrvBySmcu, &rptr_drvByMcu, 20 / portTICK_RATE_MS) == pdTRUE){

			stt_protocolFmtUartDevDrv dataTemp = {0};

			dataTemp.pHead 	 	= UART_DEVDRV_PROTOCOL_HEAD_M;
			dataTemp.command 	= rptr_drvByMcu.cmd;
			dataTemp.data 	 	= rptr_drvByMcu.data;
			dataTemp.pTailCheck	= frame_Check((uint8_t *)&dataTemp, 3);

			uart_write_bytes(uartPortSlaveMcu, (const char *)&dataTemp, sizeof(stt_protocolFmtUartDevDrv));	
			vTaskDelay(150 / portTICK_RATE_MS); //安全间隔150ms
		}

//		const int rxBytes = uart_read_bytes(uartPortSlaveMcu, 
//											paramSlaveMcuUartDataRcv.dataRcv, 
//											UART_SLAVE_MCU_FIFO_RX_LEN, 
//											20 / portTICK_RATE_MS);
//		if(rxBytes > 0){

//			printf("slaveMcuUartDataRcv[Len:%d].\n", rxBytes);	
//		}

//		if(localCounter)localCounter --;
//		else{

//			localCounter = 50;

//			uart_write_bytes(uartPortSlaveMcu, "hellow world.\n", 14);	
//		}
 #else
 
		if(xQueueReceive(queueHandle_devDrvBySmcu, &rptr_drvByMcu, 20 / portTICK_RATE_MS) == pdTRUE){

			if(2 == rptr_drvByMcu.cmd){

				uint8_t dataIn_temp = ctrlRelayBy74595_optValGet(rptr_drvByMcu.data);

				MCUDRV_OPT_STK(0);
				for(uint8_t loop = 0; loop < 8; loop ++){
				
					MCUDRV_OPT_CLK(0);
					(dataIn_temp & 0x80)?(MCUDRV_OPT_DIN(1)):(MCUDRV_OPT_DIN(0));
					vTaskDelay(1 / portTICK_RATE_MS);
					MCUDRV_OPT_CLK(1); 	
					dataIn_temp <<= 1;
				}
				MCUDRV_OPT_STK(1);

				vTaskDelay(100 / portTICK_RATE_MS); //安全间隔100ms

				MCUDRV_OPT_STK(0);
				MCUDRV_OPT_DIN(0);
				for(uint8_t loop = 0; loop < 8; loop ++){
					
					MCUDRV_OPT_CLK(0);
					vTaskDelay(1 / portTICK_RATE_MS);
					MCUDRV_OPT_CLK(1);
				}
				MCUDRV_OPT_STK(1);
			}
		}	
 #endif

 		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

void devDriverApp_statusExexuteBySlaveMcu(uint8_t status){

	stt_devDrvBySmcuFormat sptr_drvByMcu = {0};

	sptr_drvByMcu.cmd = 2;
	sptr_drvByMcu.data = status;
	
	if(queueHandle_devDrvBySmcu)
		xQueueSend(queueHandle_devDrvBySmcu, &sptr_drvByMcu, 100 / portTICK_RATE_MS);
}

void devDriverAppWithCompare_statusExexuteBySlaveMcu(uint8_t status){

	static uint8_t statusRecord = 0;

	if(statusRecord != status){

		statusRecord = status;

		devDriverApp_statusExexuteBySlaveMcu(status);
	}
}

void devDriverAppFromISR_statusExexuteBySlaveMcu(uint8_t status){

	static uint8_t statusRecord = 0;
	stt_devDrvBySmcuFormat sptr_drvByMcu = {0};

	if(statusRecord != status){

		statusRecord = status;

		sptr_drvByMcu.cmd = 2;
		sptr_drvByMcu.data = status;

		if(queueHandle_devDrvBySmcu)
			xQueueSendFromISR(queueHandle_devDrvBySmcu, &sptr_drvByMcu, NULL);
	}
}

void devDriverApp_statusExexuteBySlaveMcu_byBit(uint8_t bitHold, bool status){

	stt_devDrvBySmcuFormat sptr_drvByMcu = {0};

	sptr_drvByMcu.cmd = 2;
	memcpy(&sptr_drvByMcu.data , &lanbon_l8device_currentDataPoint, sizeof(uint8_t));
	
	(status)?
		(sptr_drvByMcu.data |=  (1 << bitHold)):
		(sptr_drvByMcu.data &= ~(1 << bitHold));

	switch(bitHold){

		case 0:{}break;

		case 1:{}break;
			
		case 2:{}break;

		default:break;
	}

	if(queueHandle_devDrvBySmcu)
		xQueueSend(queueHandle_devDrvBySmcu, &sptr_drvByMcu, 100 / portTICK_RATE_MS);
}

void devDriverApp_deviceTypeChangeInSlaveMcu(devTypeDef_enum devType){

	stt_devDrvBySmcuFormat sptr_drvByMcu = {0};

	sptr_drvByMcu.cmd = 1;
	sptr_drvByMcu.data = devType;

	if(queueHandle_devDrvBySmcu)
		xQueueSendToFront(queueHandle_devDrvBySmcu, &sptr_drvByMcu, 100 / portTICK_RATE_MS);
}

static void devDriverInit_drvBySlaveMcu_uart(void){

	uart_config_t uart_config = {

		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	uart_param_config(uartPortSlaveMcu, &uart_config);
	uart_set_pin(uartPortSlaveMcu, 
				 DRVMETHOD_BY_SLAVE_MCU_UART_TXD, 
				 DRVMETHOD_BY_SLAVE_MCU_UART_RXD, 
				 UART_PIN_NO_CHANGE, 
				 UART_PIN_NO_CHANGE);
	uart_driver_install(uartPortSlaveMcu, 
						129,
						0, 
						0, 
						NULL, 
						0); //安装驱动程序，使用idf自带中断服务函数
	uart_isr_free(uartPortSlaveMcu); //释放不使用idf自带中断服务函数
//	uart_isr_register(uartPortSlaveMcu, 
//					  isrHandleFuncUart_deviceDriverBySlaveMcu, 
//					  NULL, 
//					  ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM, 
//					  &isrUart_applicationHandle); //重新注册中断服务函数
//	uart_disable_intr_mask(uartPortSlaveMcu, 0x0003FFFF);
//	uart_enable_rx_intr(uartPortSlaveMcu); //使能中断接收
//	
//	uart_set_mode(uartPortSlaveMcu, UART_MODE_UART);
//	uart_set_rx_timeout(uartPortSlaveMcu, 20); //配置接收超时中断时间，单位为按照当前波特率传输1个bytes的时间
}

static void devDriverInit_drvBy74595(void){

	gpio_config_t io_conf = {0};
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << MCUDRV_PIN_74595_DIN) | 
						   (1ULL << MCUDRV_PIN_74595_STK) | 
						   (1ULL << MCUDRV_PIN_74595_CLK);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);	
}
#else

#endif

void isrHandleFuncPcntUnit_regster(void (*funcHandle)(uint32_t evtStatus), uint8_t funcIst){

	pcntDeviceDriver_handleFunc_list[funcIst] = funcHandle;
}

uint8_t deviceTypeVersionJudge(uint8_t devType){

	uint8_t typeVersion = 0;

	switch(devType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_scenario:
		case devTypeDef_curtain:
		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:{

			typeVersion = DEV_TYPES_PANEL_DEF_SHARE_MIX;

		}break;
			
		case devTypeDef_dimmer:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_DIMMER;

		}break;
			
		case devTypeDef_fans:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_FANS;
	
		}break;

		case devTypeDef_heater:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_HEATER;

		}break;
		
		case devTypeDef_infrared:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_INFRARED;

		}break;
			
		case devTypeDef_socket:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_SOCKET;

		}break;
			
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:
		case devTypeDef_moudleSwCurtain:{

			typeVersion = DEV_TYPES_PANEL_DEF_INDEP_MOUDLE;

		}break;

		default:{

			typeVersion = DEV_TYPES_PANEL_DEF_NULL;

		}break;
	}

	return typeVersion;
}

devTypeDef_enum IRAM_ATTR currentDev_typeGet(void){ //需在中断内调用，加IRAM_ATTR

	return lanbon_l8device_currentDevType;
}

void currentDev_typeSet(devTypeDef_enum devType, bool nvsRecord_IF){

	lanbon_l8device_currentDevType = devType;
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devTypeDef, &lanbon_l8device_currentDevType);
}

void devStatusRecordIF_paramSet(stt_devStatusRecord *param, bool nvsRecord_IF){

	memcpy(&deviceStatusRecordIF_Flg, param, sizeof(stt_devStatusRecord));
	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_devStatusRecordIF, &deviceStatusRecordIF_Flg);
}

void devStatusRecordIF_paramGet(stt_devStatusRecord *param){

	memcpy(param, &deviceStatusRecordIF_Flg, sizeof(stt_devStatusRecord));
}

void devInfo_statusRecord_action(void){

	if(deviceStatusRecordIF_Flg.devStatusOnOffRecord_IF){ //是否开启存储使能
	
		devSystemInfoLocalRecord_save(saveObj_swStatus, &lanbon_l8device_currentDataPoint);

		vTaskDelay(20 / portTICK_RATE_MS);
		switch(currentDev_typeGet()){ //其他补充需要存储的数据

			case devTypeDef_thermostatExtension:{

				uint8_t devThermostatExSwStatus_temp = devDriverBussiness_thermostatSwitch_exSwitchParamGet();

				devSystemInfoLocalRecord_save(saveObj_devThermostatExSw_statusDats, &devThermostatExSwStatus_temp);

			}break;

			default:{}break;
		}
	}
}

void deviceDatapointSynchronousReport_actionTrig(void){

	xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_DEVSTATUS_SYNCHRO);
}

bool devStatusDispMethod_landscapeIf_get(void){

	bool res = false;

	(deviceStatusRecordIF_Flg.devScreenLandscape_IF)?
		(res = true):
		(res = false);

	return res;
}

bool deviceFistRunningJudge_get(void){

	bool res = false;

	(!deviceStatusRecordIF_Flg.devUpgradeFirstRunning_FLG)?
		(res = true):
		(res = false);

	return res;
}

void devDriverApp_responseAtionTrig_instant(void){ //数据点触发即时响应动作

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	devStatusRunning_tipsRefresh(); //tips刷新
#else

	devScreenBkLight_weakUp(); //屏幕唤醒
	pageHome_buttonMain_imageRefresh(false); //UI控件刷新

	devAtmosphere_statusTips_trigSet(atmosphereLightType_dataSaveOpreat);
#endif

	devBeepTips_trig(3, 10, 150, 0, 1);

	usrApp_GreenMode_trig(); //绿色模式业务
	if(devDpTrig_funcFlg_mutualCtrlTrig)usrAppMutualCtrl_actionTrig(); //互控触发
}

void devDriverApp_responseAtionTrig_delay(void){ //数据点触发延时响应动作，避免高频动作

	usrAppParamSet_hbRealesInAdvance(devDpTrig_funcFlg_statusUploadMedthod); //心跳提前响应
	if(devDpTrig_funcFlg_nvsRecord)devInfo_statusRecord_action(); //状态记忆同步
	if(devDpTrig_funcFlg_synchronousReport)deviceDatapointSynchronousReport_actionTrig(); //状态同步上传
}

void devDriverApp_responseAtionTrig(void){

	devDriverApp_responseAtionTrig_instant(); //即时响应触发
	usrAppDevDpResp_actionDelayTrig_funcSet(); //延时响应参数刷新
}

void devDriverParamChg_dataRealesTrig(bool nvsRecord_IF, 
										  	   bool mutualCtrlTrig_IF, 
										  	   bool statusUploadMedthod,
										  	   bool synchronousReport_IF){

	//延时响应属性数据更新
	devDpTrig_funcFlg_nvsRecord 		  = nvsRecord_IF;
	devDpTrig_funcFlg_mutualCtrlTrig	  = mutualCtrlTrig_IF;
	devDpTrig_funcFlg_statusUploadMedthod = statusUploadMedthod;
	devDpTrig_funcFlg_synchronousReport   = synchronousReport_IF;

	//动作响应
	devDriverApp_responseAtionTrig();
}

void currentDev_dataPointRecovery(stt_devDataPonitTypedef *param){

	//仅数据更新，驱动不响应
	memcpy(&deviceDataPointRecord_lastTime, &lanbon_l8device_currentDataPoint, sizeof(stt_devDataPonitTypedef)); //设备状态数据点记录,当前状态转为历史状态，用于互控比对
	memcpy(&lanbon_l8device_currentDataPoint, param, sizeof(stt_devDataPonitTypedef)); //常规响应

	devDriverParamChg_dataRealesTrig(false, false, false, false);
}

void currentDev_dataPointSet(stt_devDataPonitTypedef *param, 
													   bool nvsRecord_IF, 
													   bool mutualCtrlTrig_IF, 
													   bool statusUploadMedthod,
													   bool synchronousReport_IF){

	//常规响应
	memcpy(&deviceDataPointRecord_lastTime, &lanbon_l8device_currentDataPoint, sizeof(stt_devDataPonitTypedef)); //设备状态数据点记录,当前状态转为历史状态，用于互控比对
	memcpy(&lanbon_l8device_currentDataPoint, param, sizeof(stt_devDataPonitTypedef)); //常规响应

	switch(lanbon_l8device_currentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:{

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU) && (DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)
			
			uint8_t opVal = 0;
			memcpy(&opVal, param, sizeof(stt_devDataPonitTypedef));
			devDriverApp_statusExexuteBySlaveMcu(opVal);
#else

			devDriverBussiness_mulitBitSwitch_periphStatusReales(param);			
#endif
		}break;

		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:{

//			uint8_t dataTemp = 0;

//			memcpy(&dataTemp, param, sizeof(uint8_t));
//			printf("thermostat datapoint set:%02X.\n", dataTemp);

			devDriverBussiness_thermostatSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_dimmer:{

			devDriverBussiness_dimmerSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_moudleSwCurtain:
		case devTypeDef_curtain:{

			devDriverBussiness_curtainSwitch_periphStatusReales(param);

		}break;

		case devTypeDef_infrared:{

			devDriverBussiness_infraredSwitch_periphStatusReales(param);

		}break;
		
		case devTypeDef_socket:{

			devDriverBussiness_socketSwitch_periphStatusReales(param);

		}break;
		
		case devTypeDef_fans:{

			devDriverBussiness_fansSwitch_periphStatusReales(param);
		
		}break;
	
		case devTypeDef_scenario:{

			devDriverBussiness_scnarioSwitch_scenarioStatusReales(param);
		
		}break;
		
		case devTypeDef_heater:{

			devDriverBussiness_heaterSwitch_periphStatusReales(param);

		}break;
		
		default:break;
	}

	devDriverParamChg_dataRealesTrig(nvsRecord_IF, 
								 	 mutualCtrlTrig_IF, 
								 	 statusUploadMedthod, 
								 	 synchronousReport_IF);
}

void currentDev_dataPointGet(stt_devDataPonitTypedef *param){

    memcpy(param, &lanbon_l8device_currentDataPoint, sizeof(stt_devDataPonitTypedef));
}

void currentDev_dataPointRcdGet(stt_devDataPonitTypedef *param){

    memcpy(param, &deviceDataPointRecord_lastTime, sizeof(stt_devDataPonitTypedef));
}

void currentDev_dataPointGetwithRecord(stt_devDataPonitTypedef *param){

	uint8_t dataPointHexTemp = 0,
			dataPointHexTemp_current = 0,
			dataPointHexTemp_record = 0;
			
	memcpy(&dataPointHexTemp_current, &lanbon_l8device_currentDataPoint, sizeof(uint8_t));
	memcpy(&dataPointHexTemp_record, &deviceDataPointRecord_lastTime, sizeof(uint8_t));

	dataPointHexTemp = dataPointHexTemp_current;

	switch(currentDev_typeGet()){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:{

			dataPointHexTemp &= 0x07; //除低三位以外全部清零
			
			for(uint8_t loopOpt = 0; loopOpt < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopOpt ++){ //有效位数
			
				if((dataPointHexTemp_record & (1 << loopOpt)) != (dataPointHexTemp_current & (1 << loopOpt))){
			
					dataPointHexTemp |= (1 << (loopOpt + 5)); //高三位对应动作位置位
				}
			}

		}break;

		case devTypeDef_curtain:
		case devTypeDef_moudleSwCurtain:
		case devTypeDef_heater:
		case devTypeDef_dimmer:
		case devTypeDef_infrared:
		case devTypeDef_socket:
		case devTypeDef_fans:
		case devTypeDef_scenario:
		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:
		default:{}break;
	}

	memcpy(param, &dataPointHexTemp, sizeof(uint8_t));

	printf("synchronous value reco(%02X).\n", dataPointHexTemp);
}

void currentDev_extParamSet(void *param){

	stt_devDataPonitTypedef devDataPoint = {0};
	
	uint8_t *dataParam = (uint8_t *)param; //扩展数据格式化
	uint8_t devType_rcv = dataParam[4]; //设备类型获取

	if(devType_rcv == currentDev_typeGet()){

		switch(currentDev_typeGet()){ //根据设备类型进行扩展数据设置操作

			case devTypeDef_curtain:
			case devTypeDef_moudleSwCurtain:{

				uint8_t orbitalPeriodTime_valSet = dataParam[0];
			
				devCurtain_currentOrbitalPeriodTimeSet(orbitalPeriodTime_valSet); //轨道时间设定
				devDataPoint.devType_curtain.devCurtain_actMethod = 0;
				devDataPoint.devType_curtain.devCurtain_actEnumVal =\
					curtainRunningStatus_cTact_close;
				currentDev_dataPointSet(&devDataPoint, false, false, false, false); //全关一次，使校准

			}break;
			
			case devTypeDef_heater:{

				extern void devHeater_bussinessDispRefresh_customGearSetTrig(void);
				uint16_t customDownCounter_valSet = ((uint16_t)(dataParam[0]) << 8) | 
													((uint16_t)(dataParam[1]) << 0);

				devDriverBussiness_heaterSwitch_closePeriodCustom_Set(customDownCounter_valSet, true);

				currentDev_dataPointGet(&devDataPoint); //重新触发，刷新时间显示
				if(4 == devDataPoint.devType_heater.devHeater_swEnumVal){

					devHeater_bussinessDispRefresh_customGearSetTrig();
					currentDev_dataPointSet(&devDataPoint, false, false, false, false);
				}
					
			}break;

			case devTypeDef_thermostatExtension:{

				uint8_t devThermostatExSwStatusSet_temp = dataParam[0];

				devDriverBussiness_thermostatSwitch_exSwitchParamSet(devThermostatExSwStatusSet_temp);
				devDriverParamChg_dataRealesTrig(true, true, true, false);

			}break;

			default:break;
		}
	}
}

void currentDev_extParamGet(uint8_t paramTemp[DEVPARAMEXT_DT_LEN]){

	switch(currentDev_typeGet()){ //扩展数据填装

		case devTypeDef_curtain:
		case devTypeDef_moudleSwCurtain:{

			paramTemp[0] = devCurtain_currentPositionPercentGet();

		}break;
		
		case devTypeDef_heater:{

			uint16_t heater_gearCur_period = devDriverBussiness_heaterSwitch_closePeriodCurrent_Get();
			uint16_t heater_timeRem_counter = devDriverBussiness_heaterSwitch_devParam_closeCounter_Get();

//			memcpy(&(paramTemp[0]), &heater_gearCur_period, sizeof(uint16_t));
//			memcpy(&(paramTemp[2]), &heater_timeRem_counter, sizeof(uint16_t));

			paramTemp[0] = (uint8_t)((heater_gearCur_period >> 8) & 0x00ff);
			paramTemp[1] = (uint8_t)((heater_gearCur_period >> 0) & 0x00ff);
			paramTemp[2] = (uint8_t)((heater_timeRem_counter >> 8) & 0x00ff);
			paramTemp[3] = (uint8_t)((heater_timeRem_counter >> 0) & 0x00ff);

		}break;

		case devTypeDef_infrared:{

			paramTemp[0] = devDriverBussiness_infraredSwitch_currentOpreatNumGet();

		}break;

		case devTypeDef_thermostatExtension:{

			paramTemp[0] = devDriverBussiness_thermostatSwitch_exSwitchParamGetWithRcd();

		}break;

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_dimmer:
		case devTypeDef_socket:
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:
		case devTypeDef_fans:
		case devTypeDef_scenario:
		case devTypeDef_thermostat:
		default:{ //其他类型没有特殊扩展数据的开关

			paramTemp[0] = L8_DEVICE_VERSION; //下标一写版本号

		}break;
	}
}

static void funcation_usrAppMutualCtrl_dataReq(uint8_t *dstDevMacList, uint8_t dstDevNum, uint8_t *dats, uint8_t dataLen){

	mwifi_data_type_t data_type = {
		
		.compression = true,
		.communicate = MWIFI_COMMUNICATE_MULTICAST,
	};
	const mlink_httpd_type_t type_L8mesh_cst = {

		.format = MLINK_HTTPD_FORMAT_HEX,
	};

	mdf_err_t ret = MDF_OK;

	memcpy(&data_type.custom, &type_L8mesh_cst, sizeof(uint32_t));

	if(esp_mesh_get_layer() == MESH_ROOT){
	
		data_type.communicate = MWIFI_COMMUNICATE_MULTICAST;
	
		ret = mwifi_root_write(dstDevMacList, 
							   dstDevNum, 
							   &data_type, 
							   dats, 
							   dataLen, 
							   true);
		MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mutualCtrl mwifi_translate", mdf_err_to_name(ret));
	}
	else
	{
		uint8_t loop = 0;
	
		data_type.communicate = MWIFI_COMMUNICATE_UNICAST;
	
		for(loop = 0; loop < dstDevNum; loop ++){
	
			ret = mwifi_write(&dstDevMacList[loop * MWIFI_ADDR_LEN], 
							  &data_type, 
							  dats, 
							  dataLen, 
							  true);
			MDF_ERROR_CHECK(ret != MDF_OK, ret, "<%s> mutualCtrl loop:%d mwifi_translate", mdf_err_to_name(ret), loop); 				
		}
	}
}

void funcation_usrAppMutualCtrlActionTrig(void){

	stt_devMutualGroupParam devMutualCtrlGroup_dataTemp[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	uint8_t mutualCtrlDevMacList[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP * MWIFI_ADDR_LEN] = {0};
	uint8_t mutualCtrlParam_data[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	uint8_t mutualCtrlGroup_insert[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
	bool mutualCtrlAction_trigIf = false;

	devMutualCtrlGroupInfo_Get(devMutualCtrlGroup_dataTemp); //互控组信息数据获取

	//互控触发
	switch(lanbon_l8device_currentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:{

			uint8_t dataPointHexTemp_current = 0,
					dataPointHexTemp_record = 0;

			memcpy(&dataPointHexTemp_current, &lanbon_l8device_currentDataPoint, sizeof(uint8_t));
			memcpy(&dataPointHexTemp_record, &deviceDataPointRecord_lastTime, sizeof(uint8_t));

//			printf("mutual Info: currentData:%02X, recordData:%02X.\n", dataPointHexTemp_current,
//																	    dataPointHexTemp_record);

			for(uint8_t loopOpt = 0; loopOpt < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopOpt ++){ //互控位是否可用?

				if((dataPointHexTemp_record & (1 << loopOpt)) != (dataPointHexTemp_current & (1 << loopOpt))){

					if((devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
					   (devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){

						mutualCtrlGroup_insert[loopOpt] = devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert;
						(dataPointHexTemp_current & (1 << loopOpt))?
							(mutualCtrlParam_data[loopOpt] = 1):
							(mutualCtrlParam_data[loopOpt] = 0);

						mutualCtrlAction_trigIf = true;
					}
				}
			}
			
		}break;

		case devTypeDef_curtain:
		case devTypeDef_moudleSwCurtain:
		case devTypeDef_dimmer:{

			if((devMutualCtrlGroup_dataTemp[0].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
			   (devMutualCtrlGroup_dataTemp[0].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){
			
				mutualCtrlGroup_insert[0] = devMutualCtrlGroup_dataTemp[0].mutualCtrlGroup_insert;
			
				mutualCtrlAction_trigIf = true;
			}

		}break;

		case devTypeDef_thermostatExtension:{

			const uint8_t muutalCtrlGroupNum = 2;
			uint8_t dataPointHexTemp_current = devDriverBussiness_thermostatSwitch_exSwitchParamGet(),
					dataPointHexTemp_record = devDriverBussiness_thermostatSwitch_exSwitchRcdParamGet();

			for(uint8_t loopOpt = 0; loopOpt < muutalCtrlGroupNum; loopOpt ++){ //互控位是否可用?
			
				if((dataPointHexTemp_record & (1 << loopOpt)) != (dataPointHexTemp_current & (1 << loopOpt))){
			
					if((devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_A) &&
					   (devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert != DEVICE_MUTUALGROUP_INVALID_INSERT_B)){
			
						mutualCtrlGroup_insert[loopOpt] = devMutualCtrlGroup_dataTemp[loopOpt].mutualCtrlGroup_insert;
						(dataPointHexTemp_current & (1 << loopOpt))?
							(mutualCtrlParam_data[loopOpt] = 1):
							(mutualCtrlParam_data[loopOpt] = 0);
			
						mutualCtrlAction_trigIf = true;
					}
				}
			}
					
		}break;

//		case devTypeDef_infrared:{}break;
//		case devTypeDef_socket:{}break;
//		case devTypeDef_moudleSwThreeBit:{}break;
//		case devTypeDef_fans:{}break;
//		case devTypeDef_scenario:{}break;
//		case devTypeDef_heater:{}break;

		default:break;
	}

	if(mutualCtrlAction_trigIf){

		if(mwifi_is_connected()){

#define DEVICE_MUTUAL_CTRL_REQ_DATALEN	3

			uint8_t dataMutualCtrlReq_temp[DEVICE_MUTUAL_CTRL_REQ_DATALEN] = {0};
			uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};

			esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

			switch(lanbon_l8device_currentDevType){

				case devTypeDef_mulitSwOneBit:
				case devTypeDef_mulitSwTwoBit:
				case devTypeDef_mulitSwThreeBit:
				case devTypeDef_moudleSwOneBit:
				case devTypeDef_moudleSwTwoBit:
				case devTypeDef_moudleSwThreeBit:
				case devTypeDef_thermostatExtension:{

					for(uint8_t loopMutual = 0; loopMutual < DEVICE_MUTUAL_CTRL_GROUP_NUM; loopMutual ++){
					
						if(!mutualCtrlGroup_insert[loopMutual])continue; //对应键位触发判断
					
						dataMutualCtrlReq_temp[0] = L8DEV_MESH_CMD_MUTUAL_CTRL;
						dataMutualCtrlReq_temp[1] = mutualCtrlGroup_insert[loopMutual];
						dataMutualCtrlReq_temp[2] = mutualCtrlParam_data[loopMutual];	
					
						usrAppMethod_mwifiMacAddrRemoveFromList(devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevMacList, 
																devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum, 
																sta_mac);
						devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum --; //数目减掉自身
					
						funcation_usrAppMutualCtrl_dataReq(devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevMacList,
														   devMutualCtrlGroup_dataTemp[loopMutual].mutualCtrlDevNum,
														   dataMutualCtrlReq_temp,
														   DEVICE_MUTUAL_CTRL_REQ_DATALEN);
					}

				}break;

				case devTypeDef_curtain:
				case devTypeDef_moudleSwCurtain:
				case devTypeDef_dimmer:{

					dataMutualCtrlReq_temp[0] = L8DEV_MESH_CMD_MUTUAL_CTRL;
					dataMutualCtrlReq_temp[1] = mutualCtrlGroup_insert[0];
					memcpy(&dataMutualCtrlReq_temp[2], &lanbon_l8device_currentDataPoint, sizeof(uint8_t));

					usrAppMethod_mwifiMacAddrRemoveFromList(devMutualCtrlGroup_dataTemp[0].mutualCtrlDevMacList, 
															devMutualCtrlGroup_dataTemp[0].mutualCtrlDevNum, 
															sta_mac);
					devMutualCtrlGroup_dataTemp[0].mutualCtrlDevNum --; //数目减掉自身

					funcation_usrAppMutualCtrl_dataReq(devMutualCtrlGroup_dataTemp[0].mutualCtrlDevMacList,
													   devMutualCtrlGroup_dataTemp[0].mutualCtrlDevNum,
													   dataMutualCtrlReq_temp,
													   DEVICE_MUTUAL_CTRL_REQ_DATALEN);
				}break;

//				case devTypeDef_infrared:{}break;
//				case devTypeDef_socket:{}break;
//				case devTypeDef_moudleSwThreeBit:{}break;
//				case devTypeDef_fans:{}break;
//				case devTypeDef_scenario:{}break;
//				case devTypeDef_thermostat:{}break;
//				case devTypeDef_heater:{}break;

				default:break;
			}
		}
	}
}

static void usrAppMutualCtrl_actionTrig(void){ //触发互控所需要API的内存空间较大，所以通过事件传输到专用通信线程进行触发

	xEventGroupSetBits(xEventGp_devApplication, DEVAPPLICATION_FLG_BITHOLD_MUTUALTRIG);
}

void devDriverManageBussiness_initialition(void){

	esp_err_t res = ESP_OK;

	static bool devDriverBySlaveMCU_statusRecovery_flg = false;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)	
	static bool devDriverBySlaveMCU_initialition_flg = false;
	
#else
	

#endif

	static bool isrFunc_pcntIntr_register_flg = false;
	static bool devDriver_elecDetect_initialition_Flg = false;
	static bool devDriver_tempDetect_initialition_Flg = false;
	static bool devDriver_tinfraActDetect_initialition_Flg = false;
	
	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(!isrFunc_pcntIntr_register_flg){

		isrFunc_pcntIntr_register_flg = true;

		//pcntIsr 注册在所有pcnt驱动初始化之前进行
	    res = pcnt_isr_register(isrHandleFuncPcnt_deviceDriverApplication, 
	    						NULL, 
	    						ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM, 
	    						&isrPcnt_applicationHandle);
		printf(">>>>>>>>>>pcnt isr reg res:%d.\n", res);
	}

	if(!devDriver_elecDetect_initialition_Flg){

		devDriver_elecDetect_initialition_Flg = true;
		
		devDriverBussiness_elecMeasure_periphInit();
	}

	if(!devDriver_tempDetect_initialition_Flg){

		devDriver_tempDetect_initialition_Flg = true;

		devDriverBussiness_temperatureMeasure_periphInit();		
	}

	if(!devDriver_tinfraActDetect_initialition_Flg){

		devDriver_tinfraActDetect_initialition_Flg = true;

		devDriverBussiness_infraActDetect_periphInit();
	}

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)	
	if(!devDriverBySlaveMCU_initialition_flg){

		devDriverBySlaveMCU_initialition_flg = true;

 #if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)
 
		devDriverInit_drvBySlaveMcu_uart();
 #else

 		devDriverInit_drvBy74595();
 #endif

		queueHandle_devDrvBySmcu = xQueueCreate(10, sizeof(stt_devDrvBySmcuFormat)); //协MCU通信进程消息队列创建
		if(NULL == queueHandle_devDrvBySmcu)printf("queueHandle_devDrvBySmcu creat fail !!!\n");

		xTaskCreate(processTask_deviceDriverBySlaveMcu, "pTask_uartSMcu", 1024 * 2, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY + 1, NULL); //协MCU通信进程创建
	}

	devDriverApp_deviceTypeChangeInSlaveMcu(swCurrentDevType); //类型切换时通知

#else

#endif

	switch(swCurrentDevType){

		case devTypeDef_mulitSwOneBit:
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_moudleSwOneBit:
		case devTypeDef_moudleSwTwoBit:
		case devTypeDef_moudleSwThreeBit:{

			devDriverBussiness_mulitBitSwitch_moudleInit();

		}break;
		
		case devTypeDef_dimmer:{

			devDriverBussiness_dimmerSwitch_moudleInit();
			
		}break;
		
		case devTypeDef_fans:{

			devDriverBussiness_fansSwitch_moudleInit();

		}break;
		
		case devTypeDef_scenario:{

			devDriverBussiness_scnarioSwitch_moudleInit();
		
		}break;
		
		case devTypeDef_curtain:
		case devTypeDef_moudleSwCurtain:{

			devDriverBussiness_curtainSwitch_moudleInit();

		}break;

		case devTypeDef_infrared:{

			devDriverBussiness_infraredSwitch_moudleInit();

		}break;
		
		case devTypeDef_socket:{

			devDriverBussiness_socketSwitch_moudleInit();

		}break;
		
		case devTypeDef_heater:{

			devDriverBussiness_heaterSwitch_moudleInit();

		}break;

		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:{

			devDriverBussiness_thermostatSwitch_moudleInit();
		
		}break;
		
		default:break;
	}

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) //红外转发器强制不做状态恢复
	devDriverBySlaveMCU_statusRecovery_flg = true;
#endif
	if(!devDriverBySlaveMCU_statusRecovery_flg){ //记忆状态恢复，硬件初始化晚于掉电数据恢复

		devDriverBySlaveMCU_statusRecovery_flg = true;

		currentDev_dataPointSet(&lanbon_l8device_currentDataPoint, false, false, true, false);

		switch(swCurrentDevType){

			case devTypeDef_thermostatExtension:{

				devDriverBussiness_thermostatSwitch_exSwitchParamSet(devDriverBussiness_thermostatSwitch_exSwitchParamGet());

			}break;

			default:{}break;
		}
	}
}

void devDriverManageBussiness_deinitialition(void){

	devDriverBussiness_mulitBitSwitch_moudleDeinit();
	devDriverBussiness_dimmerSwitch_moudleInit();
	devDriverBussiness_curtainSwitch_moudleDeinit();
	devDriverBussiness_fansSwitch_moudleDeinit();
	devDriverBussiness_heaterSwitch_moudleDeinit();
	devDriverBussiness_thermostatSwitch_moudleDeinit();
	devDriverBussiness_infraredSwitch_moudleDeinit();
	devDriverBussiness_socketSwitch_moudleDeinit();
	devDriverBussiness_scnarioSwitch_moudleDeinit();
}

void devDriverManageBussiness_deviceChangeRefresh(void){

	devDriverManageBussiness_deinitialition();
	devDriverManageBussiness_initialition();
}



















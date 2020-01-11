#include "devDriver_heater.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "devDriver_manage.h"

#define DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY1     	27
#define DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY2     	14
#define DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY3     	12

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DEVDRIVER_HEATER_ACTION_POWERSTART()		devDriverApp_statusExexuteBySlaveMcu(0x01)
 #define DEVDRIVER_HEATER_ACTION_POWERSYNCH()		devDriverApp_statusExexuteBySlaveMcu(0x07)
 #define DEVDRIVER_HEATER_ACTION_POWEROFF()			devDriverApp_statusExexuteBySlaveMcu(0x00)
#else

 #define DEVDRIVER_HEATER_ACTION_POWERSTART()		gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY1, (uint32_t)1)
 #define DEVDRIVER_HEATER_ACTION_POWERSYNCH()		gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY2, (uint32_t)1);\
													gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY3, (uint32_t)1)
 #define DEVDRIVER_HEATER_ACTION_POWEROFF()			gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
#endif

static bool devDriver_moudleInitialize_Flg = false;

static stt_Heater_actAttr devParam_heater = {

	.timeCountParam.relaySynchronousDnCounter = 0,
	.timeCountParam.timeUp_period			  = 0,
	.timeCountParam.timeUp_counter			  = 0,
	.timeCountParam.timeCount_En			  = 0,
	.timeCountParam.timeUp_period_customSet   = 10 * DEVHEATER_COEFFICIENT_TIME_SECOND, //默认自定义延时关闭时间为60s
};

xQueueHandle msgQh_devHeaterDriver = NULL;

static void devDriverBussiness_heaterSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_heater)return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY1) | 
						   (1ULL << DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY2) |
						   (1ULL << DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY3);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_heaterSwitch_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY1);
	gpio_reset_pin(DEVDRIVER_HEATER_GPIO_OUTPUT_IO_RLY2);
}

void devDriverBussiness_heaterSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_heater)return;
	if(devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
				
#else
	
	devDriverBussiness_heaterSwitch_periphInit();	
#endif

	msgQh_devHeaterDriver = xQueueCreate(1, sizeof(stt_msgDats_devHeaterDriver)); //队列长度为1，保证实时性

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_heaterSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
							
#else

	devDriverBussiness_heaterSwitch_periphDeinit();
#endif

	vQueueDelete(msgQh_devHeaterDriver);
	msgQh_devHeaterDriver = NULL;
	
	devDriver_moudleInitialize_Flg = false;
}

void IRAM_ATTR devDriverBussiness_heaterSwitch_runningDetectLoop(void){

	static stt_devHeater_opratAct opreatAct_localRecord = heaterOpreatAct_close;
	stt_msgDats_devHeaterDriver sptr_msgQ_devHeaterDriver = {0};

	if(devParam_heater.timeCountParam.relaySynchronousDnCounter != COUNTER_DISENABLE_MASK_SPECIALVAL_U32){

		if(devParam_heater.timeCountParam.relaySynchronousDnCounter)devParam_heater.timeCountParam.relaySynchronousDnCounter --;
		else{

			devParam_heater.timeCountParam.relaySynchronousDnCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U32;
			
			DEVDRIVER_HEATER_ACTION_POWERSYNCH();
		}
	}

	if(devParam_heater.timeCountParam.timeCount_En){

		if(devParam_heater.timeCountParam.timeUp_counter < devParam_heater.timeCountParam.timeUp_period)devParam_heater.timeCountParam.timeUp_counter ++;
		else{

			devParam_heater.timeCountParam.timeCount_En = 0;
			devParam_heater.timeCountParam.timeUp_counter = 0;
		
			devParam_heater.opreatAct = heaterOpreatAct_close; //自主更新驱动层设备运行状态

			DEVDRIVER_HEATER_ACTION_POWEROFF();
		}
	}

	switch(devParam_heater.opreatAct){

		case heaterOpreatAct_close:{
	
			if(opreatAct_localRecord != devParam_heater.opreatAct){

				sptr_msgQ_devHeaterDriver.msgType = msgType_devHeaterDriver_devActChg;
				sptr_msgQ_devHeaterDriver.msgDats.data_opreatChg.act = heaterOpreatAct_close;
				xQueueOverwriteFromISR(msgQh_devHeaterDriver, &sptr_msgQ_devHeaterDriver, NULL);
			}

		}break;
		
		case heaterOpreatAct_open:{

			if(opreatAct_localRecord != devParam_heater.opreatAct){
			
				sptr_msgQ_devHeaterDriver.msgType = msgType_devHeaterDriver_devActChg;
				sptr_msgQ_devHeaterDriver.msgDats.data_opreatChg.act = heaterOpreatAct_open;
				xQueueOverwriteFromISR(msgQh_devHeaterDriver, &sptr_msgQ_devHeaterDriver, NULL);
			}

		}break;
		
		case heaterOpreatAct_closeAfter30Min:
		case heaterOpreatAct_closeAfter60Min:
		case heaterOpreatAct_closeAfterTimeCustom:{

			if(!(devParam_heater.timeCountParam.timeUp_counter % (uint32_t)DEVHEATER_COEFFICIENT_TIME_SECOND)){

				sptr_msgQ_devHeaterDriver.msgType = msgType_devHeaterDriver_closeCounterChg;
				sptr_msgQ_devHeaterDriver.msgDats.data_counterChg.counter = 
					(devParam_heater.timeCountParam.timeUp_period - devParam_heater.timeCountParam.timeUp_counter) / 
					(uint32_t)DEVHEATER_COEFFICIENT_TIME_SECOND;
				xQueueSendFromISR(msgQh_devHeaterDriver, &sptr_msgQ_devHeaterDriver, NULL);
			}

		}break;

		default:break;
	}

	opreatAct_localRecord = devParam_heater.opreatAct;
}

void devDriverBussiness_heaterSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	static stt_devHeater_opratAct opreatAct_localRecord = heaterOpreatAct_close;

	if(swCurrentDevType == devTypeDef_heater){

		devParam_heater.opreatAct = param->devType_heater.devHeater_swEnumVal;
		printf("heater opreatAct set:%d.\n", devParam_heater.opreatAct);

		switch(devParam_heater.opreatAct){

			case heaterOpreatAct_close:{

				devParam_heater.timeCountParam.relaySynchronousDnCounter = COUNTER_DISENABLE_MASK_SPECIALVAL_U32;
				devParam_heater.timeCountParam.timeCount_En = 0;
				devParam_heater.timeCountParam.timeUp_counter = 0;

				if(opreatAct_localRecord != heaterOpreatAct_close)DEVDRIVER_HEATER_ACTION_POWEROFF(); //动作放入运行监测业务逻辑内执行

			}break;
			
			case heaterOpreatAct_open:{

				devParam_heater.timeCountParam.relaySynchronousDnCounter = DEVHEATER_REALY_SYNCHRONOUS_TIME;
				devParam_heater.timeCountParam.timeCount_En = 0;
				devParam_heater.timeCountParam.timeUp_counter = 0;

				if(opreatAct_localRecord == heaterOpreatAct_close)DEVDRIVER_HEATER_ACTION_POWERSTART(); //非关闭状态下，不进行重复操作

			}break;
			
			case heaterOpreatAct_closeAfter30Min:{

				devParam_heater.timeCountParam.relaySynchronousDnCounter = DEVHEATER_REALY_SYNCHRONOUS_TIME;
				devParam_heater.timeCountParam.timeCount_En = 1;
				devParam_heater.timeCountParam.timeUp_period = 30 * 60 * DEVHEATER_COEFFICIENT_TIME_SECOND;
				devParam_heater.timeCountParam.timeUp_counter = 0;

				if(opreatAct_localRecord == heaterOpreatAct_close)DEVDRIVER_HEATER_ACTION_POWERSTART(); //非关闭状态下，不进行重复操作

			}break;
			
			case heaterOpreatAct_closeAfter60Min:{

				devParam_heater.timeCountParam.relaySynchronousDnCounter = DEVHEATER_REALY_SYNCHRONOUS_TIME;
				devParam_heater.timeCountParam.timeCount_En = 1;
				devParam_heater.timeCountParam.timeUp_period = 60 * 60 * DEVHEATER_COEFFICIENT_TIME_SECOND;
				devParam_heater.timeCountParam.timeUp_counter = 0;

				if(opreatAct_localRecord == heaterOpreatAct_close)DEVDRIVER_HEATER_ACTION_POWERSTART(); //非关闭状态下，不进行重复操作

			}break;
			
			case heaterOpreatAct_closeAfterTimeCustom:{

				devParam_heater.timeCountParam.relaySynchronousDnCounter = DEVHEATER_REALY_SYNCHRONOUS_TIME;
				devParam_heater.timeCountParam.timeCount_En = 1;
				devParam_heater.timeCountParam.timeUp_period = devParam_heater.timeCountParam.timeUp_period_customSet;
				devParam_heater.timeCountParam.timeUp_counter = 0;

				if(opreatAct_localRecord == heaterOpreatAct_close)DEVDRIVER_HEATER_ACTION_POWERSTART(); //非关闭状态下，不进行重复操作

			}break;

			default:break;
		}

		opreatAct_localRecord = devParam_heater.opreatAct;
	}
}

void devDriverBussiness_heaterSwitch_devParam_Get(stt_Heater_actAttr *param){

	memcpy(param, &devParam_heater, sizeof(stt_Heater_actAttr));
}

uint32_t devDriverBussiness_heaterSwitch_devParam_closeCounter_Get(void){

	return devParam_heater.timeCountParam.timeUp_counter / DEVHEATER_COEFFICIENT_TIME_SECOND;
}

uint32_t devDriverBussiness_heaterSwitch_closePeriodCurrent_Get(void){

	return devParam_heater.timeCountParam.timeUp_period / DEVHEATER_COEFFICIENT_TIME_SECOND;
}

uint32_t devDriverBussiness_heaterSwitch_closePeriodCustom_Get(void){

	return devParam_heater.timeCountParam.timeUp_period_customSet / DEVHEATER_COEFFICIENT_TIME_SECOND;
}

void devDriverBussiness_heaterSwitch_closePeriodCustom_Set(uint32_t valSet, bool nvsRecord_IF){

	devParam_heater.timeCountParam.timeUp_period_customSet = 
	valSet * DEVHEATER_COEFFICIENT_TIME_SECOND;

	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_devHeater_customTimeParam, &valSet);
	}
}




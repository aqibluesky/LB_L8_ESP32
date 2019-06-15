#include "devDriver_curtain.h"

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

#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1     	27
#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2     	14
#define DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3     	12
#define DEVDRIVER_CURTAIN_ACTION_OPEN()				gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)1);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
#define DEVDRIVER_CURTAIN_ACTION_STOP()				gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
#define DEVDRIVER_CURTAIN_ACTION_CLOSE()			gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1, (uint32_t)1);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)

static bool devDriver_moudleInitialize_Flg = false;

static stt_Curtain_motorAttr devParam_curtain = {0};
static uint32_t curtainOrbitalPositionTimeSet = 0;

xQueueHandle msgQh_devCurtainDriver = NULL;
	
void devCurtain_runningParamSet(stt_devCurtain_runningParam *param, bool nvsRecord_IF){ //轨道时间为ms，注意设置存储时进行千倍乘

	memcpy(&(devParam_curtain.devRunningParam), param, sizeof(stt_devCurtain_runningParam));

	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam));
	}
}

void devCurtain_runningParamGet(stt_devCurtain_runningParam *param){ //轨道时间为ms，注意设置获取时进行千倍除

	memcpy(param, &(devParam_curtain.devRunningParam), sizeof(stt_devCurtain_runningParam));
}

uint8_t devCurtain_currentPositionPercentGet(void){

	return (uint8_t)((devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period);
}

stt_devCurtain_runningStatus devCurtain_currentRunningStatusGet(void){

	return devParam_curtain.act;
}

void devCurtain_ocrbitalTimeSaveInitiative(void){

	devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam)); //位置记录执行
}

void devDriverBussiness_curtainSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_curtain)return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1) | 
						   (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2) | 
						   (1ULL << DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

void devDriverBussiness_curtainSwitch_periphDeinit(void){

	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY1);
	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY2);
	gpio_reset_pin( DEVDRIVER_CURTAIN_GPIO_OUTPUT_IO_RLY3);
}

void devDriverBussiness_curtainSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_curtain)return;
	if(devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_curtainSwitch_periphInit();

	msgQh_devCurtainDriver = xQueueCreate(1, sizeof(stt_msgDats_devCurtainDriver)); //队列长度为1，保证实时性

	devParam_curtain.devRunningParam.act_period = 5000;

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_curtainSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_curtainSwitch_periphDeinit();

	vQueueDelete(msgQh_devCurtainDriver);
	msgQh_devCurtainDriver = NULL;
	
	devDriver_moudleInitialize_Flg = false;
}

bool IRAM_ATTR devDriverBussiness_curtainSwitch_devRunningDetect(void){ //开关设备运行监测，每ms调用

	bool devCurtainOrbitalTimeSave_IF = false;
	stt_msgDats_devCurtainDriver sptr_msgQ_devCurtainDriver = {0};
	static stt_devCurtain_runningStatus act_localRecord = curtainRunningStatus_cTact_stop;

	if(devParam_curtain.act == curtainRunningStatus_cTact_stop){

		if(act_localRecord != devParam_curtain.act){

			act_localRecord = devParam_curtain.act;
			 //返停触发消息发送以便进行UI更新
			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_opreatStop;
			sptr_msgQ_devCurtainDriver.msgDats.data_opreatStop.opreatStop_sig = 1;
			xQueueOverwriteFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
//			xQueueSendToFrontFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
		
	}else{

		act_localRecord = devParam_curtain.act;
	}

	if((devParam_curtain.act == curtainRunningStatus_cTact_close) ||
	   (devParam_curtain.act == curtainRunningStatus_cTact_open)){

		static uint8_t orbitalPercent_localRecord = 0;
		uint8_t orbitalPercent_temp = (devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period;

		if(orbitalPercent_localRecord != orbitalPercent_temp){

			orbitalPercent_localRecord = orbitalPercent_temp;

			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalChgingByBtn;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingByBtn.orbitalPosPercent = orbitalPercent_localRecord;
			xQueueSendFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
	}

	if(devParam_curtain.act == curtainRunningStatus_cTact_custom){

		static uint8_t orbitalPercent_localRecord = 0;
		uint8_t orbitalPercent_temp = (devParam_curtain.devRunningParam.act_counter * 100) / devParam_curtain.devRunningParam.act_period;
		
		if(orbitalPercent_localRecord != orbitalPercent_temp){
		
			orbitalPercent_localRecord = orbitalPercent_temp;
		
			sptr_msgQ_devCurtainDriver.msgType = msgType_devCurtainDriver_orbitalChgingBySlider;
			sptr_msgQ_devCurtainDriver.msgDats.data_orbitalChgingBySlider.orbitalPosPercent = orbitalPercent_localRecord;
			xQueueSendFromISR(msgQh_devCurtainDriver, &sptr_msgQ_devCurtainDriver, NULL);
		}
	}

	switch(devParam_curtain.act){

		case curtainRunningStatus_cTact_stop:{

			static uint32_t curtainOrbitalPosTime_localRecord = 0;

			if(curtainOrbitalPosTime_localRecord != devParam_curtain.devRunningParam.act_counter){ //轨道位置时间不同时才进行存储操作

				curtainOrbitalPosTime_localRecord = devParam_curtain.devRunningParam.act_counter;

				devCurtainOrbitalTimeSave_IF = true;
			
//				devSystemInfoLocalRecord_save(saveObj_devCurtain_runningParam, &(devParam_curtain.devRunningParam)); //位置记录执行 --中断内无法进行nvs操作，通过返回标志位devCurtainOrbitalTimeSave_IF后使用消息队列进行nvs存储信号传输
			}

			curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_counter;

			DEVDRIVER_CURTAIN_ACTION_STOP();

		}break;
		
		case curtainRunningStatus_cTact_close:{

			if(devParam_curtain.devRunningParam.act_counter != 0)devParam_curtain.devRunningParam.act_counter --;
			else{

				if(devParam_curtain.devRunningParam.act_period){ //轨道时间不为零则返停

					devParam_curtain.devRunningParam.act_counter = 0;
					devParam_curtain.act = curtainRunningStatus_cTact_stop;
				}
			}

			DEVDRIVER_CURTAIN_ACTION_CLOSE();

		}break;
		
		case curtainRunningStatus_cTact_open:{

			if(devParam_curtain.devRunningParam.act_counter < devParam_curtain.devRunningParam.act_period)devParam_curtain.devRunningParam.act_counter ++;
			else{

				if(devParam_curtain.devRunningParam.act_period){ //轨道时间不为零则返停
					
					devParam_curtain.devRunningParam.act_counter = devParam_curtain.devRunningParam.act_period;
					devParam_curtain.act = curtainRunningStatus_cTact_stop;
				}
			}

			DEVDRIVER_CURTAIN_ACTION_OPEN();

		}break;

		case curtainRunningStatus_cTact_custom:{

			if(curtainOrbitalPositionTimeSet > devParam_curtain.devRunningParam.act_period)
				curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_period;

			if(devParam_curtain.devRunningParam.act_counter < curtainOrbitalPositionTimeSet){

				devParam_curtain.devRunningParam.act_counter ++;

				DEVDRIVER_CURTAIN_ACTION_OPEN();
			}
			else
			if(devParam_curtain.devRunningParam.act_counter > curtainOrbitalPositionTimeSet){

				devParam_curtain.devRunningParam.act_counter --;

				DEVDRIVER_CURTAIN_ACTION_CLOSE();
			}
			else{

				devParam_curtain.devRunningParam.act_counter = curtainOrbitalPositionTimeSet;
				devParam_curtain.act = curtainRunningStatus_cTact_stop;
			}
			
		}break;

		default:break;
	}

	return devCurtainOrbitalTimeSave_IF;
}

void devDriverBussiness_curtainSwitch_periphStatusRealesByBtn(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_curtain){

		devParam_curtain.act = param->devType_curtain.devCurtain_actEnumVal;

		if((devParam_curtain.act == curtainRunningStatus_cTact_open) && //满开触底操作
		   (devParam_curtain.devRunningParam.act_counter == devParam_curtain.devRunningParam.act_period) &&
		   (devParam_curtain.devRunningParam.act_period != 0)){

			devParam_curtain.devRunningParam.act_counter -= DEVICE_CURTAIN_OPREATION_EXTRA_TIME;
			
		}else
		if((devParam_curtain.act == curtainRunningStatus_cTact_close) && //满关触底操作
		   (devParam_curtain.devRunningParam.act_counter == 0) &&
		   (devParam_curtain.devRunningParam.act_period != 0)){

			devParam_curtain.devRunningParam.act_counter += DEVICE_CURTAIN_OPREATION_EXTRA_TIME;
		}
	}
}

void devDriverBussiness_curtainSwitch_periphStatusRealesBySlide(uint8_t positionPercentVal){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_curtain){

		devParam_curtain.act = curtainRunningStatus_cTact_custom;
		curtainOrbitalPositionTimeSet = devParam_curtain.devRunningParam.act_period / 100 * positionPercentVal;
		printf("curtain orbital set:%d.\n", curtainOrbitalPositionTimeSet);
	}
}




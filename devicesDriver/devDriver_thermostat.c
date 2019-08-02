#include "devDriver_thermostat.h"

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

#define DEVDRIVER_THERMOSTAT_GPIO_OUTPUT_IO_RLY     	27

#define DEVDRIVER_THERMOSTAT_ADJUST_START()				gpio_set_level(DEVDRIVER_THERMOSTAT_GPIO_OUTPUT_IO_RLY, (uint32_t)1)
#define DEVDRIVER_THERMOSTAT_ADJUST_STOP()				gpio_set_level(DEVDRIVER_THERMOSTAT_GPIO_OUTPUT_IO_RLY, (uint32_t)0)

static bool devDriver_moudleInitialize_Flg = false;

static stt_thermostat_actAttr devParam_thermostat = {

	.workModeInNight_IF = 0,
	.temperatureVal_target = 16,
	.temperatureVal_current = 16,
};

static void devDriverBussiness_thermostatSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_heater)return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_THERMOSTAT_GPIO_OUTPUT_IO_RLY);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_thermostatSwitch_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_THERMOSTAT_GPIO_OUTPUT_IO_RLY);
}

void devDriverBussiness_thermostatSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_thermostat)return;
	if(devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_thermostatSwitch_periphInit();

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_thermostatSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_thermostatSwitch_periphDeinit();
	
	devDriver_moudleInitialize_Flg = false;
}

void IRAM_ATTR devDriverBussiness_thermostatSwitch_runningDetectLoop(void){

	uint8_t adjustTrig_shortVal = 0;

	float tempratureRead_temp = devDriverBussiness_temperatureMeasure_get();

	devParam_thermostat.temperatureVal_current = (uint16_t)tempratureRead_temp;

	if(!devParam_thermostat.deviceRunning_EN)return; //设备启用失能，直接返回

	(devParam_thermostat.workModeInNight_IF)?
		(adjustTrig_shortVal = 1):
		(adjustTrig_shortVal = 5);

	if(devParam_thermostat.temperatureVal_current < (devParam_thermostat.temperatureVal_target - adjustTrig_shortVal)){

		DEVDRIVER_THERMOSTAT_ADJUST_START();
	}
	else //施密特触发器
	if(devParam_thermostat.temperatureVal_current > devParam_thermostat.temperatureVal_target){

		DEVDRIVER_THERMOSTAT_ADJUST_STOP();
	}
}

void devDriverBussiness_thermostatSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_thermostat){

		devParam_thermostat.deviceRunning_EN = 
			param->devType_thermostat.devThermostat_running_en;

		devParam_thermostat.workModeInNight_IF = 
			param->devType_thermostat.devThermostat_nightMode_en;

		devParam_thermostat.temperatureVal_target = 
			param->devType_thermostat.devThermostat_tempratureTarget;

		if(devParam_thermostat.temperatureVal_target < 16)
			devParam_thermostat.temperatureVal_target = 16;
	}
}

void devDriverBussiness_thermostatSwitch_devParam_get(stt_thermostat_actAttr *param){

	memcpy(param, &devParam_thermostat, sizeof(stt_thermostat_actAttr));
}



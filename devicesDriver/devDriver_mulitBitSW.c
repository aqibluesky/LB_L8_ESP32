#include "devDriver_mulitBitSW.h"

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

#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1     	27
#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2     	14
#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3     	12

static bool devDriver_moudleInitialize_Flg = false;

static void devDriverBussiness_mulitBitSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	
	//bit mask of the pins that you want to set
	switch(swCurrentDevType){

		case devTypeDef_mulitSwOneBit:

			io_conf.pin_bit_mask = (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1);
			
			break;
		
		case devTypeDef_mulitSwTwoBit:

			io_conf.pin_bit_mask = (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1) | 
								   (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2);
		
			break;
			
		case devTypeDef_mulitSwThreeBit:

			io_conf.pin_bit_mask = (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1) | 
								   (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2) | 
								   (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3);
			
			break;
			
		default:break;
	}
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_mulitBitSwitch_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1);
	gpio_reset_pin(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2);
	gpio_reset_pin(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3);
}

void devDriverBussiness_mulitBitSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if((swCurrentDevType != devTypeDef_mulitSwOneBit) &&
	   (swCurrentDevType != devTypeDef_mulitSwTwoBit) &&
	   (swCurrentDevType != devTypeDef_mulitSwThreeBit))return;
	if(devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_mulitBitSwitch_periphInit();

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_mulitBitSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_mulitBitSwitch_periphDeinit();

	devDriver_moudleInitialize_Flg = false;
}

void devDriverBussiness_mulitBitSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	switch(swCurrentDevType){

		case devTypeDef_mulitSwOneBit:{

			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1, 
						   (uint32_t)param->devType_mulitSwitch_oneBit.swVal_bit1);

		}break;
		
		case devTypeDef_mulitSwTwoBit:{

			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1, 
						   (uint32_t)param->devType_mulitSwitch_twoBit.swVal_bit1);
			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2, 
						   (uint32_t)param->devType_mulitSwitch_twoBit.swVal_bit2);

		}break;
		
		case devTypeDef_mulitSwThreeBit:{

			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1, 
						   (uint32_t)param->devType_mulitSwitch_threeBit.swVal_bit1);
			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2, 
						   (uint32_t)param->devType_mulitSwitch_threeBit.swVal_bit2);
			gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3, 
						   (uint32_t)param->devType_mulitSwitch_threeBit.swVal_bit3);

		}break;
		
		default:break;
	}
} 



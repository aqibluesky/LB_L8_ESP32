#include "devDriver_fans.h"

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

#define DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1     	27
#define DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2     	14
#define DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3     	12

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DEVDRIVER_FANS_ACTION_STOP()						devDriverApp_statusExexuteBySlaveMcu(0x00)
 #define DEVDRIVER_FANS_ACTION_GEAR1()						devDriverApp_statusExexuteBySlaveMcu(0x01)
 #define DEVDRIVER_FANS_ACTION_GEAR2()						devDriverApp_statusExexuteBySlaveMcu(0x02)
 #define DEVDRIVER_FANS_ACTION_GEAR3()						devDriverApp_statusExexuteBySlaveMcu(0x04)
#else

 #define DEVDRIVER_FANS_ACTION_STOP()				gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
 #define DEVDRIVER_FANS_ACTION_GEAR1()				gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1, (uint32_t)1);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
 #define DEVDRIVER_FANS_ACTION_GEAR2()				gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2, (uint32_t)1);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3, (uint32_t)0)
 #define DEVDRIVER_FANS_ACTION_GEAR3()				gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2, (uint32_t)0);\
													gpio_set_level(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3, (uint32_t)1)
#endif

static bool devDriver_moudleInitialize_Flg = false;

static stt_Fans_actAttr devParam_fans = {0};

static void devDriverBussiness_fansSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_fans)return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1) | 
						   (1ULL << DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2) | 
						   (1ULL << DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_fansSwitch_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY1);
	gpio_reset_pin(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY2);
	gpio_reset_pin(DEVDRIVER_FANS_GPIO_OUTPUT_IO_RLY3);
}

void devDriverBussiness_fansSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_fans)return;
	if(devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
		
#else

	devDriverBussiness_fansSwitch_periphInit();
#endif

	devDriver_moudleInitialize_Flg = true;	
}

void devDriverBussiness_fansSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
						
#else

	devDriverBussiness_fansSwitch_periphDeinit();
#endif
	
	devDriver_moudleInitialize_Flg = false;
}

void devDriverBussiness_fansSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_fans){

		devParam_fans.opreatAct = param->devType_fans.devFans_swEnumVal; 
		printf("fans opreatAct set:%d.\n", devParam_fans.opreatAct);

		switch(devParam_fans.opreatAct){

			case fansOpreatAct_stop:{

				DEVDRIVER_FANS_ACTION_STOP();

			}break;
			
			case fansOpreatAct_firstGear:{

				DEVDRIVER_FANS_ACTION_GEAR1();

			}break;
			
			case fansOpreatAct_secondGear:{

				DEVDRIVER_FANS_ACTION_GEAR2();

			}break;
			
			case fansOpreatAct_thirdGear:{

				DEVDRIVER_FANS_ACTION_GEAR3();

			}break;
			
			default:break;
		}
	}
}


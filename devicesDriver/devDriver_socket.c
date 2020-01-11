#include "devDriver_socket.h"

/* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "devDriver_manage.h"

#define DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_RLY	   		27
#define DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_UPOWER	   	14

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)


#else

 #define DEVDRIVER_SOCKET_RELAY_STATUS_SET(a)		gpio_set_level(DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_RLY, (uint32_t)a);\
 													gpio_set_level(DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_UPOWER, (uint32_t)a);
#endif

static const char *TAG = "lanbon_L8 - Socket driver";

static bool devDriver_moudleInitialize_Flg = false;

static void devDriverBussiness_socketSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	gpio_config_t io_conf = {0};

	if(swCurrentDevType != devTypeDef_socket)return;
	
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_RLY) |
						   (1ULL << DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_UPOWER);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_socketSwitch_periphDeinit(void){

	gpio_reset_pin(DEVDRIVER_SOCKET_GPIO_OUTPUT_IO_RLY);
}

void devDriverBussiness_socketSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_socket)return;
	if(devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else

	devDriverBussiness_socketSwitch_periphInit();
#endif

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_socketSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
#else

	devDriverBussiness_socketSwitch_periphDeinit();
#endif

	devDriver_moudleInitialize_Flg = false;
}

void devDriverBussiness_socketSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_socket){

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)


#else

		DEVDRIVER_SOCKET_RELAY_STATUS_SET(param->devType_socket.devSocket_opSw);
#endif
	}
}





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

#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1	   12
#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2	   14
#define DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3	   27

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(a)		devDriverApp_statusExexuteBySlaveMcu_byBit(0, a)
 #define DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(a)		devDriverApp_statusExexuteBySlaveMcu_byBit(1, a)
 #define DEVDRIVER_MULITPLESW_RELAY3_STATUS_SET(a)		devDriverApp_statusExexuteBySlaveMcu_byBit(2, a)
#else

 #define DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(a)		gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1, (uint32_t)a)
 #define DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(a)		gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2, (uint32_t)a)
 #define DEVDRIVER_MULITPLESW_RELAY3_STATUS_SET(a)		gpio_set_level(DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY3, (uint32_t)a)			
#endif

static const char *TAG = "lanbon_L8 - MultiSw driver";

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
		case devTypeDef_moudleSwOneBit:

			io_conf.pin_bit_mask = (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1);
			
			break;
		
		case devTypeDef_mulitSwTwoBit:
		case devTypeDef_moudleSwTwoBit:	

			io_conf.pin_bit_mask = (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY1) | 
								   (1ULL << DEVDRIVER_MULITPLESW_GPIO_OUTPUT_IO_RLY2);
		
			break;
			
		case devTypeDef_mulitSwThreeBit:
		case devTypeDef_moudleSwThreeBit:

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
	   (swCurrentDevType != devTypeDef_mulitSwThreeBit) && 
	   (swCurrentDevType != devTypeDef_moudleSwOneBit) &&
	   (swCurrentDevType != devTypeDef_moudleSwTwoBit) &&
	   (swCurrentDevType != devTypeDef_moudleSwThreeBit))return;
	if(devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

#else

	devDriverBussiness_mulitBitSwitch_periphInit();
#endif

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_mulitBitSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
								
#else

	devDriverBussiness_mulitBitSwitch_periphDeinit();
#endif

	devDriver_moudleInitialize_Flg = false;
}

void devDriverBussiness_mulitBitSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	uint8_t statusVal_opreat = 0;
	
	memcpy(&statusVal_opreat, param, sizeof(uint8_t));

	MDF_LOGI("Ctrl val:%02X\n", statusVal_opreat);

//#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

//	if(statusVal_opreat & 0xE0){ //是否为语音控制操作（高三位有效？）

//		uint8_t statusTemp = 0;

//		currentDev_dataPointGet((stt_devDataPonitTypedef *)&statusTemp);

//		switch(swCurrentDevType){

//			case devTypeDef_mulitSwOneBit:
//			case devTypeDef_moudleSwOneBit:{

//				statusTemp &= 0x01;
//				if(param->devType_mulitSwitch_oneBit.opCtrl_bit1){

//					(param->devType_mulitSwitch_oneBit.swVal_bit1)?
//						(statusTemp |=  (1 << 0)):
//						(statusTemp &= ~(1 << 0));
//				}
//			
//			}break;

//			case devTypeDef_mulitSwTwoBit:
//			case devTypeDef_moudleSwTwoBit:{

//				statusTemp &= 0x03;
//				if(param->devType_mulitSwitch_twoBit.opCtrl_bit1){

//					(param->devType_mulitSwitch_twoBit.swVal_bit1)?
//						(statusTemp |=  (1 << 0)):
//						(statusTemp &= ~(1 << 0));
//				}
//				
//				if(param->devType_mulitSwitch_twoBit.opCtrl_bit2){

//					(param->devType_mulitSwitch_twoBit.swVal_bit2)?
//						(statusTemp |=  (1 << 1)):
//						(statusTemp &= ~(1 << 1));
//				}

//			}break;

//			case devTypeDef_mulitSwThreeBit:
//			case devTypeDef_moudleSwThreeBit:{

//				statusTemp &= 0x07;
//				if(param->devType_mulitSwitch_threeBit.opCtrl_bit1){

//					(param->devType_mulitSwitch_threeBit.swVal_bit1)?
//						(statusTemp |=  (1 << 0)):
//						(statusTemp &= ~(1 << 0));
//				}
//				
//				if(param->devType_mulitSwitch_threeBit.opCtrl_bit2){

//					(param->devType_mulitSwitch_threeBit.swVal_bit2)?
//						(statusTemp |=  (1 << 1)):
//						(statusTemp &= ~(1 << 1));
//				}
//				
//				if(param->devType_mulitSwitch_threeBit.opCtrl_bit3){

//					(param->devType_mulitSwitch_threeBit.swVal_bit3)?
//						(statusTemp |=  (1 << 2)):
//						(statusTemp &= ~(1 << 2));	
//				}

//			}break;

//			default:break;
//		}

//		devDriverAppWithCompare_statusExexuteBySlaveMcu(statusTemp);

//		memcpy(param, &statusTemp, sizeof(stt_devDataPonitTypedef));
//		currentDev_dataPointRecovery(param); //按位操作数据点恢复
//	}
//	else
//	{
//		switch(swCurrentDevType){

//			case devTypeDef_mulitSwOneBit:
//			case devTypeDef_moudleSwOneBit:{

//				devDriverApp_statusExexuteBySlaveMcu(statusVal_opreat & 0x01);
//			
//			}break;

//			case devTypeDef_mulitSwTwoBit:
//			case devTypeDef_moudleSwTwoBit:{

//				devDriverApp_statusExexuteBySlaveMcu(statusVal_opreat & 0x03);

//			}break;

//			case devTypeDef_mulitSwThreeBit:
//			case devTypeDef_moudleSwThreeBit:{

//				devDriverApp_statusExexuteBySlaveMcu(statusVal_opreat & 0x07);

//			}break;

//			default:break;
//		}
//	}

//#else

	if(statusVal_opreat & 0xE0){ //是否为语音控制操作（高三位有效？）

		stt_devDataPonitTypedef dataPoint_temp = {0};

		currentDev_dataPointRcdGet(&dataPoint_temp);

		switch(swCurrentDevType){
		
			case devTypeDef_mulitSwOneBit:
			case devTypeDef_moudleSwOneBit:{

				if(param->devType_mulitSwitch_oneBit.opCtrl_bit1){

					DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_oneBit.swVal_bit1);
					dataPoint_temp.devType_mulitSwitch_oneBit.swVal_bit1 =\
						param->devType_mulitSwitch_oneBit.swVal_bit1;
				}
					
			}break;
			
			case devTypeDef_mulitSwTwoBit:
			case devTypeDef_moudleSwTwoBit:{

				if(param->devType_mulitSwitch_twoBit.opCtrl_bit1){

					DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_twoBit.swVal_bit1);
					dataPoint_temp.devType_mulitSwitch_twoBit.swVal_bit1 =\
						param->devType_mulitSwitch_twoBit.swVal_bit1;
				}
					
				if(param->devType_mulitSwitch_twoBit.opCtrl_bit2){

					DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(param->devType_mulitSwitch_twoBit.swVal_bit2);	
					dataPoint_temp.devType_mulitSwitch_twoBit.swVal_bit2 =\
						param->devType_mulitSwitch_twoBit.swVal_bit2;
				}
								
			}break;
			
			case devTypeDef_mulitSwThreeBit:
			case devTypeDef_moudleSwThreeBit:{

				if(param->devType_mulitSwitch_threeBit.opCtrl_bit1){

					DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit1);
					dataPoint_temp.devType_mulitSwitch_threeBit.swVal_bit1 =\
						param->devType_mulitSwitch_threeBit.swVal_bit1;
				}
					
				if(param->devType_mulitSwitch_threeBit.opCtrl_bit2){

					DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit2);
					dataPoint_temp.devType_mulitSwitch_threeBit.swVal_bit2 =\
						param->devType_mulitSwitch_threeBit.swVal_bit2;
				}
					
				if(param->devType_mulitSwitch_threeBit.opCtrl_bit3){

					DEVDRIVER_MULITPLESW_RELAY3_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit3);
					dataPoint_temp.devType_mulitSwitch_threeBit.swVal_bit3 =\
						param->devType_mulitSwitch_threeBit.swVal_bit3;
				}
					
			}break;
			
			default:break;
		}

		dataPoint_temp.devType_mulitSwitch_threeBit.opCtrl_bit1 =\
		dataPoint_temp.devType_mulitSwitch_threeBit.opCtrl_bit2 =\
		dataPoint_temp.devType_mulitSwitch_threeBit.opCtrl_bit3 = 0;
		currentDev_dataPointRecovery(&dataPoint_temp); //按位操作数据点恢复
	}
	else
	{
		switch(swCurrentDevType){
		
			case devTypeDef_mulitSwOneBit:
			case devTypeDef_moudleSwOneBit:{
		
				DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_oneBit.swVal_bit1);
		
			}break;
			
			case devTypeDef_mulitSwTwoBit:
			case devTypeDef_moudleSwTwoBit:{
		
				DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_twoBit.swVal_bit1);
				DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(param->devType_mulitSwitch_twoBit.swVal_bit2);
		
			}break;
			
			case devTypeDef_mulitSwThreeBit:
			case devTypeDef_moudleSwThreeBit:{
		
				DEVDRIVER_MULITPLESW_RELAY1_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit1);
				DEVDRIVER_MULITPLESW_RELAY2_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit2);
				DEVDRIVER_MULITPLESW_RELAY3_STATUS_SET(param->devType_mulitSwitch_threeBit.swVal_bit3);
				
			}break;
			
			default:break;
		}
	}
//#endif
} 



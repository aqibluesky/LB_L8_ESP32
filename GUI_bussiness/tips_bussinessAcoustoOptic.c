#include "tips_bussinessAcoustoOptic.h"

/* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "mdf_common.h"
#include "mwifi.h"
#include "mlink.h"

#include "devDriver_manage.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

 #define DEV_INFRARED_STATUS_TIPS_PIN_R		(32)
 #define DEV_INFRARED_STATUS_TIPS_PIN_B		(33)

 #define tipsInfrared_LR_set(x)				gpio_set_level(DEV_INFRARED_STATUS_TIPS_PIN_R, x)
 #define tipsInfrared_LB_set(x)				gpio_set_level(DEV_INFRARED_STATUS_TIPS_PIN_B, x)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

 #define SOCKETS_SPECIFICATION_AMERICA		0x0A //美规
 #define SOCKETS_SPECIFICATION_BRITISH		0x0B //英规
 #define SOCKETS_SPECIFICATION_GENERAL		0x0C //常规
 #define SOCKETS_SPECIFICATION_SAFRICA		0x0D //南非

 #define DEV_SOCKET_SPECIFICATION			SOCKETS_SPECIFICATION_AMERICA

 #define DEV_SOCKET_STATUS_TIPS_PIN_R	   (32)
 #define DEV_SOCKET_STATUS_TIPS_PIN_B	   (33)

 #define tipsSocket_LR_set(x)			   gpio_set_level(DEV_SOCKET_STATUS_TIPS_PIN_R, x)
 #define tipsSocket_LB_set(x)			   gpio_set_level(DEV_SOCKET_STATUS_TIPS_PIN_B, x)

 #if(DEV_SOCKET_SPECIFICATION == SOCKETS_SPECIFICATION_AMERICA)

  #define tipsSocket_LR_ON()			   tipsSocket_LR_set(1);tipsSocket_LB_set(0)
  #define tipsSocket_LB_ON()			   tipsSocket_LR_set(0);tipsSocket_LB_set(1)
  #define tipsSocket_ALL_OFF()			   tipsSocket_LR_set(0);tipsSocket_LB_set(0)

 #elif(DEV_SOCKET_SPECIFICATION == SOCKETS_SPECIFICATION_GENERAL)

  #define tipsSocket_LR_ON()			   tipsSocket_LR_set(0);tipsSocket_LB_set(1)
  #define tipsSocket_LB_ON()			   tipsSocket_LR_set(1);tipsSocket_LB_set(0)
  #define tipsSocket_ALL_ON()			   tipsSocket_LR_set(0);tipsSocket_LB_set(0)
  #define tipsSocket_ALL_OFF()			   tipsSocket_LR_set(1);tipsSocket_LB_set(1)
  
 #endif

 enum{
 
	 devSocketLedTipsAct_mode_null = 0,
	 devSocketLedTipsAct_mode_R,
	 devSocketLedTipsAct_mode_B,
	 devSocketLedTipsAct_mode_RB
 }devSocketLedTipsAction_param = {0};

 bool devSocketStatusTip_nwkFlg = false,
	  devSocketStatusTip_swFlg = false;

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

 #define DEV_MOUDLE_STATUS_TIPS_PIN			(2)

 #define tipsSocket_L_set(x)			   	gpio_set_level(DEV_MOUDLE_STATUS_TIPS_PIN, x)
 
#endif

static enum_tipsNetworkStatus 			tipsNetworkStatus 			 = ntStatus_noneNet;
static enum_tipsDevNoneScrRunningStatus tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
static uint16_t 						tipsAbnormalCounter 		 = 0;

void devTipsByLed_driverReales(void){

	const uint16_t loopPeriod_1s = (uint16_t)(1.0F / DEVDRIVER_EX_LEDTIPS_REFRESH_PERIOD);
	static uint16_t loopCounter_1s = 0;

	const float actPeriodValFoloat_A = 0.500F / DEVDRIVER_EX_LEDTIPS_REFRESH_PERIOD,
				actPeriodValFoloat_B = 0.020F / DEVDRIVER_EX_LEDTIPS_REFRESH_PERIOD,
				actPeriodValFoloat_C = 0.150F / DEVDRIVER_EX_LEDTIPS_REFRESH_PERIOD;
	uint16_t actPeriodValInt = 0;
	static struct{
	
		uint16_t actCounter:15;
		uint16_t lightLevel:1;
	}tipsRunningParam = {0};

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	switch(devDriverBussiness_infraredStatus_get()){

		case infraredSMStatus_learnning:
		case infraredSMStatus_sigSend:{

			tipsInfrared_LR_set(0);

		}break;

		default:{

			tipsInfrared_LR_set(1);

		}break;
	}

	switch(tipsDevNoneScr_runningStatus){

		case tipsRunningStatus_normal:{

			switch(tipsNetworkStatus){
			
				case ntStatus_noneNet:{ //灭
			
					tipsInfrared_LB_set(1);
			
				}break;
			
				case ntStatus_offline:{ //闪
			
					if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
					else{
			
						actPeriodValInt = (uint16_t)actPeriodValFoloat_A;
						tipsRunningParam.actCounter = actPeriodValInt;
					
						tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
					
						tipsInfrared_LB_set(tipsRunningParam.lightLevel);
					}
			
				}break;
			
				case ntStatus_online:{ //亮
			
					tipsInfrared_LB_set(0);
			
				}break;
			
				default:break;
			}

		}break;
		
		case tipsRunningStatus_upgrading:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{

				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_B;
					tipsRunningParam.actCounter = actPeriodValInt;
				
					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
				
					tipsInfrared_LB_set(tipsRunningParam.lightLevel);
				}
			}

		}break;
		
		case tipsRunningStatus_funcTrig:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{
			
				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_C;
					tipsRunningParam.actCounter = actPeriodValInt;
				
					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
				
					tipsInfrared_LB_set(tipsRunningParam.lightLevel);
				}
			}

		}break;

		default:{

			tipsAbnormalCounter = 0;
		
		}break;
	}
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
	static uint8_t colorCounter = 0;
	const uint8_t colorPeriod = 100;

 #if(DEV_SOCKET_SPECIFICATION == SOCKETS_SPECIFICATION_AMERICA)
	if(colorCounter > colorPeriod)colorCounter = 0;
	else{

		colorCounter ++;

		switch(devSocketLedTipsAction_param){
			
			case devSocketLedTipsAct_mode_R:{

				tipsSocket_LR_ON();

			}break;
			
			case devSocketLedTipsAct_mode_B:{

				tipsSocket_LB_ON();

			}break;
			
			case devSocketLedTipsAct_mode_RB:{

				if(colorCounter % 2){

					tipsSocket_LB_ON();
				}
				else
				{
					tipsSocket_LR_ON();
				}
				
			}break;

			case devSocketLedTipsAct_mode_null:
			default:{

				tipsSocket_ALL_OFF();

			}break;
		}
	}
	
 #elif(DEV_SOCKET_SPECIFICATION == SOCKETS_SPECIFICATION_GENERAL)
	if(colorCounter > colorPeriod)colorCounter = 0;
	else{

		colorCounter ++;

		switch(devSocketLedTipsAction_param){

			case devSocketLedTipsAct_mode_R:{

				tipsSocket_LR_ON();

			}break;

			case devSocketLedTipsAct_mode_B:{

				tipsSocket_LB_ON();

			}break;

			case devSocketLedTipsAct_mode_RB:{

				tipsSocket_ALL_ON();

			}break;

			case devSocketLedTipsAct_mode_null:
			default:{

				tipsSocket_ALL_OFF();

			}break;
		}
	}

 #endif

	switch(tipsDevNoneScr_runningStatus){

		case tipsRunningStatus_normal:{

			switch(tipsNetworkStatus){

				case ntStatus_noneNet:{ //灭

					devSocketStatusTip_nwkFlg = false;

				}break;

				case ntStatus_offline:{ //闪

					if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
					else{
					
						actPeriodValInt = (uint16_t)actPeriodValFoloat_A;
						tipsRunningParam.actCounter = actPeriodValInt;

						tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
						devSocketStatusTip_nwkFlg = tipsRunningParam.lightLevel;
					}

				}break;

				case ntStatus_online:{ //亮

					devSocketStatusTip_nwkFlg = true;

				}break;

				default:{}break;
			}

		}break;

		case tipsRunningStatus_upgrading:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{

				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_B;
					tipsRunningParam.actCounter = actPeriodValInt;

					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
					devSocketStatusTip_nwkFlg = tipsRunningParam.lightLevel;
				}
			}

		}break;

		case tipsRunningStatus_funcTrig:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{

				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_C;
					tipsRunningParam.actCounter = actPeriodValInt;

					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
					devSocketStatusTip_nwkFlg = tipsRunningParam.lightLevel;
				}
			}
			
		}break;

		default:{}break;
	}

	if(devSocketStatusTip_swFlg | devSocketStatusTip_nwkFlg){

		if(devSocketStatusTip_swFlg & devSocketStatusTip_nwkFlg){

			devSocketLedTipsAction_param = devSocketLedTipsAct_mode_RB;
		}
		else
		if(devSocketStatusTip_swFlg){

			devSocketLedTipsAction_param = devSocketLedTipsAct_mode_R;
		}
		else
		if(devSocketStatusTip_nwkFlg){

			devSocketLedTipsAction_param = devSocketLedTipsAct_mode_B;
		}
	}
	else
	{
		devSocketLedTipsAction_param = devSocketLedTipsAct_mode_null;
	}

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	switch(tipsDevNoneScr_runningStatus){

		case tipsRunningStatus_normal:{

			switch(tipsNetworkStatus){
			
				case ntStatus_noneNet:{ //灭
			
					tipsSocket_L_set(1);
			
				}break;
			
				case ntStatus_offline:{ //闪
			
					if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
					else{
			
						actPeriodValInt = (uint16_t)actPeriodValFoloat_A;
						tipsRunningParam.actCounter = actPeriodValInt;
					
						tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
					
						tipsSocket_L_set(tipsRunningParam.lightLevel);
					}
			
				}break;
			
				case ntStatus_online:{ //亮
			
					tipsSocket_L_set(0);
			
				}break;
			
				default:break;
			}

		}break;
		
		case tipsRunningStatus_upgrading:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{

				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_B;
					tipsRunningParam.actCounter = actPeriodValInt;
				
					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
				
					tipsSocket_L_set(tipsRunningParam.lightLevel);
				}
			}

		}break;
		
		case tipsRunningStatus_funcTrig:{

			if(!tipsAbnormalCounter)tipsDevNoneScr_runningStatus = tipsRunningStatus_normal;
			else{
			
				if(tipsRunningParam.actCounter)tipsRunningParam.actCounter --;
				else{
				
					actPeriodValInt = (uint16_t)actPeriodValFoloat_C;
					tipsRunningParam.actCounter = actPeriodValInt;
				
					tipsRunningParam.lightLevel = !tipsRunningParam.lightLevel;
				
					tipsSocket_L_set(tipsRunningParam.lightLevel);
				}
			}

		}break;

		default:{

			tipsAbnormalCounter = 0;
		
		}break;
	}

#endif

	if(loopCounter_1s < loopPeriod_1s)loopCounter_1s ++;
	else{

		loopCounter_1s = 0;

		if(tipsAbnormalCounter)tipsAbnormalCounter --;
	}
}

void devTipsStatusRunning_abnormalTrig(enum_tipsDevNoneScrRunningStatus status, uint16_t time){

	tipsDevNoneScr_runningStatus = status;

	if(tipsRunningStatus_normal == status){

		tipsAbnormalCounter = 0;
	}
	else
	{
		tipsAbnormalCounter = time;
	}
}

void devStatusRunning_tipsRefresh(void){

	uint8_t devMesh_statusRole_temp = roleMeshStatus_noConnect;
	static uint8_t devMesh_statusRole_record = roleMeshStatus_noConnect;

	stt_devDataPonitTypedef devDataPoint_temp = {0};

	currentDev_dataPointGet(&devDataPoint_temp);

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	(devDataPoint_temp.devType_socket.devSocket_opSw)?
		(devSocketStatusTip_swFlg = true):
		(devSocketStatusTip_swFlg = false);

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

#endif

	if(meshNetwork_connectReserve_IF_get()){

		if(flgGet_gotRouterOrMeshConnect()){

			tipsNetworkStatus = ntStatus_online;
		}
		else
		{
			tipsNetworkStatus = ntStatus_offline;
		}
	}
	else
	{
		tipsNetworkStatus = ntStatus_noneNet;
	}

	if(mwifi_is_connected()){
	
		if(esp_mesh_get_layer() == MESH_ROOT){
	
			devMesh_statusRole_temp = roleMeshStatus_root;
		}
		else
		{
			devMesh_statusRole_temp = roleMeshStatus_node;
		}
	}
	else
	{
		devMesh_statusRole_temp = roleMeshStatus_noConnect;
	}
	
	if(devMesh_statusRole_record != devMesh_statusRole_temp){

		devMesh_statusRole_record = devMesh_statusRole_temp;

		uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

		switch(devMesh_statusRole_record){

			case roleMeshStatus_root:{

				devRunningFlg_temp |= DEV_RUNNING_FLG_BIT_DEVHOSTIF;
			
			}break;

			case roleMeshStatus_node:{

				devRunningFlg_temp &= ~DEV_RUNNING_FLG_BIT_DEVHOSTIF;

			}break;

			case roleMeshStatus_noConnect:{

				devRunningFlg_temp &= ~DEV_RUNNING_FLG_BIT_DEVHOSTIF;

			}break;

			default:break;
		}

		currentDevRunningFlg_paramSet(devRunningFlg_temp, true);
	}
}

void devTipsAcoustoOpticPeriphInit(void){

	gpio_config_t io_conf = {0};

	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)	

	io_conf.pin_bit_mask = (1ULL << DEV_INFRARED_STATUS_TIPS_PIN_R) | 
						   (1ULL << DEV_INFRARED_STATUS_TIPS_PIN_B);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 0;

	devBeepTips_trig(3, 10, 150, 0, 1); //红外转发器不存在状态恢复，此处补充开机提示音
	
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	io_conf.pin_bit_mask = (1ULL << DEV_SOCKET_STATUS_TIPS_PIN_R) | 
						   (1ULL << DEV_SOCKET_STATUS_TIPS_PIN_B);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 0;
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	io_conf.pin_bit_mask = (1ULL << DEV_MOUDLE_STATUS_TIPS_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 1;
#endif
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	devTipsStatusRunning_abnormalTrig(tipsRunningStatus_funcTrig, 1);
}






#include "devDriver_devSelfLight.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/ledc.h"

#include "devDriver_manage.h"

#define LEDC_DEV_RESERVE_NUM   					(6)
#define DEVLEDC_TIMER							LEDC_TIMER_0
#define DEVLEDC_MODE           					LEDC_HIGH_SPEED_MODE
#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_R			(16)
#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_G			(32)
#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_B			(17)
#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_LRen		(26)
#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_UDen		(33)
#define DEVLEDC_PIN_SCREEN_BK_LIGHT				(5)
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_R		LEDC_CHANNEL_0
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_G		LEDC_CHANNEL_1
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_B		LEDC_CHANNEL_2
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_LRen	LEDC_CHANNEL_3
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_UDen	LEDC_CHANNEL_4
#define DEVLEDC_CHANNEL_SCREEN_BK_LIGHT			LEDC_CHANNEL_5
#define DEVLEDC_DUTY_DEFAULT      				(8000)
#define DEVLEDC_COLOR_DUTY_DIV					(8000 / 255)
#define DEVLEDC_FADE_TIME_DEFAULT 				(1000)

#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R		0
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G		1
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B		2
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen	3
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen	4
#define DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT		5

#define DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI	(1000 / DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD)

#define DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN	20 * DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI	//屏幕半亮灭活检测时间
#define DEVVALUE_DEFAULT_PERIOD_SCRBKLT_SHUTDOWN	(DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN + (10 * DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI)) //屏幕全灭灭活检测时间

#define DEVLEDC_ATMOSPHERELED_COLORSET(r,g,b)		ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel, r * DEVLEDC_COLOR_DUTY_DIV);\
            										ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel);\
													ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel, g * DEVLEDC_COLOR_DUTY_DIV);\
            										ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel);\
													ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel, b * DEVLEDC_COLOR_DUTY_DIV);\
            										ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel)
#define DEVLEDC_ATMOSPHERELED_COLORCHG(r,g,b)		ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel, r * DEVLEDC_COLOR_DUTY_DIV, DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD - 50);\
            										ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel, LEDC_FADE_NO_WAIT);\
													ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel, g * DEVLEDC_COLOR_DUTY_DIV, DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD - 50);\
            										ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel, LEDC_FADE_NO_WAIT);\
													ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel, b * DEVLEDC_COLOR_DUTY_DIV, DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD - 50);\
            										ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel, LEDC_FADE_NO_WAIT)

#define DEVLEDC_ATMOSPHERELED_BRIGHTNESSSET(lr,ud)	ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].channel, DEVLEDC_DUTY_DEFAULT - (lr * DEVLEDC_COLOR_DUTY_DIV));\
            										ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].channel);\
													ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].channel, DEVLEDC_DUTY_DEFAULT - (ud * DEVLEDC_COLOR_DUTY_DIV));\
            										ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].channel)

extern EventGroupHandle_t xEventGp_infraActDetect;

static const struct _stt_atmosphere_colorTab{

	uint32_t colorDuty_r;	
	uint32_t colorDuty_g;	
	uint32_t colorDuty_b;	
}atmosphere_colorTab[5] = {

	{255,  64,   0},
	{128,   0, 214},
	{  0, 100, 256},
	{ 50, 200,   0},
	{ 50, 	0, 150},
};

static uint16_t timePeriod_devScreenBkLight_weakDown = DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN; //屏幕半亮灭活检测计时变量
static uint16_t timePeriod_devScreenBkLight_shutDown = DEVVALUE_DEFAULT_PERIOD_SCRBKLT_SHUTDOWN; //屏幕全灭灭活检测计时变量
static uint16_t timeCounter_devScreenBkLight_keepAlive = 0; //屏幕背光活力检查时间
static bool 	screenBkLight_shutDownFLG = false; //屏幕背光熄灭标志
static bool		screenBkLight_initializedFLG = false;

static enum_screenBkLight_status devScreenBklightCurrent_status = screenBkLight_statusEmpty;
static enum_atmosphereLightType devAtmosphereCurrent_status = atmosphereLightType_normalWithoutNet;
static enum_atmosphereLightType devAtmosphereStatus_record = atmosphereLightType_none;

static stt_infraActDetectCombineFLG devLinkageCombine_FLG = {

	.linkageWith_screenBkLight = 0,
	.linkageWith_swRelay = 0
};

static ledc_channel_config_t devLight_lecCfgParam[LEDC_DEV_RESERVE_NUM] = {

	{
		.channel	= DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_R,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_ATMOSPHERE_LIGHT_R,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
	{
		.channel	= DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_G,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_ATMOSPHERE_LIGHT_G,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
	{
		.channel	= DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_B,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_ATMOSPHERE_LIGHT_B,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
	{
		.channel	= DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_LRen,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_ATMOSPHERE_LIGHT_LRen,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
	{
		.channel	= DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_UDen,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_ATMOSPHERE_LIGHT_UDen,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
	{
		.channel	= DEVLEDC_CHANNEL_SCREEN_BK_LIGHT,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_SCREEN_BK_LIGHT,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
};

static void devScreenBkLight_brightnessSet(enum_screenBkLight_status val){

	if(!screenBkLight_initializedFLG)return;

	devScreenBklightCurrent_status = val;

	switch(devScreenBklightCurrent_status){

		case screenBkLight_statusEmpty:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
									0, 
									DEVLEDC_FADE_TIME_DEFAULT);

		}break;

		case screenBkLight_statusHalf:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
									DEVLEDC_DUTY_DEFAULT / 10, 
									DEVLEDC_FADE_TIME_DEFAULT);

		}break;

		case screenBkLight_statusFull:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
									DEVLEDC_DUTY_DEFAULT, 
									DEVLEDC_FADE_TIME_DEFAULT / 2);

		}break;

		default:{

			return;
			
		}break;
	}

	ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode,
                    devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
                    LEDC_FADE_NO_WAIT);
}

void deviceHardwareLight_Init(void){

	/*
	 * Prepare and set configuration of timers
	 * that will be used by LED Controller
	 */
	ledc_timer_config_t devLight_timer = {
		.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
		.freq_hz = 5000,					  // frequency of PWM signal
		.speed_mode = DEVLEDC_MODE, 		  // timer mode
		.timer_num = DEVLEDC_TIMER			  // timer index
	};
	// Set configuration of timer0 for high speed channels
	ledc_timer_config(&devLight_timer);

//	// Prepare and set configuration of timer1 for low speed channels
//	devLight_timer.speed_mode = LEDC_LOW_SPEED_MODE;
//	devLight_timer.timer_num = DEVLEDC_TIMER;
//	ledc_timer_config(&devLight_timer);

	// Set LED Controller with previously prepared configuration
	for (uint8_t ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
		ledc_channel_config(&devLight_lecCfgParam[ch]);
	}

	// Initialize fade service.
	ledc_fade_func_install(0);
	screenBkLight_initializedFLG = true; //初始化完成标志置位
	devScreenBkLight_brightnessSet(screenBkLight_statusFull);

	DEVLEDC_ATMOSPHERELED_BRIGHTNESSSET(240, 200);	
}

enum_screenBkLight_status devScreenBkLight_brightnessGet(void){

	return devScreenBklightCurrent_status;
}

void devScreenBkLight_weakUp(void){

	if(devScreenBkLight_brightnessGet() != screenBkLight_statusFull)devScreenBkLight_brightnessSet(screenBkLight_statusFull);
	
	timeCounter_devScreenBkLight_keepAlive = 0;
	screenBkLight_shutDownFLG = false;
}

void devScreenBkLight_statusRefresh(void){

	static uint16_t atmosphereLight_transitionCount = 0; //氛围灯转场时间
	static uint16_t atmosphereLight_statusKeepCount = 0;
	EventBits_t infraActDetect_etBits = 0;
		
	//红外动作探测业务
	infraActDetect_etBits = xEventGroupWaitBits(xEventGp_infraActDetect, 
											 	INFRAACTDETECTEVENT_FLG_BITHOLD_RESERVE,
												pdTRUE,
												pdFALSE,
												0);
	if((infraActDetect_etBits & INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN) == INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN){

//		printf("infraActDetect trig!.\n");
//		devAtmosphere_statusTips_trigSet(atmosphereLightType_infraActDetectTrig); //氛围灯触发

		if(devLinkageCombine_FLG.linkageWith_screenBkLight){

			devScreenBkLight_weakUp();
		}

		if(devLinkageCombine_FLG.linkageWith_swRelay){

			
		}
	}

	//背光灯业务
	if(timeCounter_devScreenBkLight_keepAlive < timePeriod_devScreenBkLight_shutDown){

		timeCounter_devScreenBkLight_keepAlive ++;

		if(timeCounter_devScreenBkLight_keepAlive > timePeriod_devScreenBkLight_weakDown){

			devScreenBkLight_brightnessSet(screenBkLight_statusHalf);
		}
			
	}else{

		if(!screenBkLight_shutDownFLG){

			devScreenBkLight_brightnessSet(screenBkLight_statusEmpty);

			screenBkLight_shutDownFLG = true;
		}
	}

	//氛围灯业务
	if(devAtmosphereStatus_record != devAtmosphereCurrent_status){ //转场效果

		devAtmosphereStatus_record = devAtmosphereCurrent_status;

		switch(devAtmosphereCurrent_status){ 
		
			case atmosphereLightType_normalWithoutNet:
			case atmosphereLightType_normalNetNode:
			case atmosphereLightType_normalNetRoot:{

				atmosphereLight_transitionCount = 2 * (DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD / DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD);
			
			}break;
			
			case atmosphereLightType_dataSaveOpreat:{

				atmosphereLight_transitionCount = 2 * (DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD / DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD);

			}break;

			case atmosphereLightType_infraActDetectTrig:{

				atmosphereLight_transitionCount = 2 * (DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD / DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD);

			}break;
		
			default:break;
		}
	}

	if(atmosphereLight_transitionCount){

		atmosphereLight_transitionCount --;

		switch(devAtmosphereCurrent_status){
		
			case atmosphereLightType_normalWithoutNet:{

				switch(atmosphereLight_transitionCount){

					case 1:{

						DEVLEDC_ATMOSPHERELED_COLORCHG(atmosphere_colorTab[0].colorDuty_r,
													   atmosphere_colorTab[0].colorDuty_g,
													   atmosphere_colorTab[0].colorDuty_b);
					}break;

					case 0:{
					
					}break;

					default:break;
				}
	
			}break;
			
			case atmosphereLightType_normalNetNode:{
				
				switch(atmosphereLight_transitionCount){
					
					case 1:{
					
						DEVLEDC_ATMOSPHERELED_COLORCHG(atmosphere_colorTab[1].colorDuty_r,
													   atmosphere_colorTab[1].colorDuty_g,
													   atmosphere_colorTab[1].colorDuty_b);
					}break;

					case 0:{
					
					}break;
				
					default:break;
				}

			}break;
			
			case atmosphereLightType_normalNetRoot:{
		
				switch(atmosphereLight_transitionCount){
					
					case 1:{
					
						DEVLEDC_ATMOSPHERELED_COLORCHG(atmosphere_colorTab[2].colorDuty_r,
													   atmosphere_colorTab[2].colorDuty_g,
													   atmosphere_colorTab[2].colorDuty_b);
					}break;

					case 0:{
						
					
					}break;
				
					default:break;
				}

			}break;
			
			case atmosphereLightType_dataSaveOpreat:{
		
				switch(atmosphereLight_transitionCount){
					
					case 1:{
					
						DEVLEDC_ATMOSPHERELED_COLORCHG(atmosphere_colorTab[3].colorDuty_r,
													   atmosphere_colorTab[3].colorDuty_g,
													   atmosphere_colorTab[3].colorDuty_b);
					}break;

					case 0:{

						devAtmosphereCurrent_status = atmosphereLightType_none;
					
					}break;
					
					default:break;
				}

			}break;

			case atmosphereLightType_infraActDetectTrig:{
		
				switch(atmosphereLight_transitionCount){
					
					case 1:{
					
						DEVLEDC_ATMOSPHERELED_COLORCHG(atmosphere_colorTab[4].colorDuty_r,
													   atmosphere_colorTab[4].colorDuty_g,
													   atmosphere_colorTab[4].colorDuty_b);
					}break;

					case 0:{

						devAtmosphereCurrent_status = atmosphereLightType_none;
					
					}break;
					
					default:break;
				}

			}break;
			
			default:break;
		}

//		printf("atmosphereLight_transitionCount:%d.\n", atmosphereLight_transitionCount);
	}
	else
	{
		switch(devAtmosphereCurrent_status){ //保持效果
		
			case atmosphereLightType_normalWithoutNet:{
		
				DEVLEDC_ATMOSPHERELED_COLORSET(atmosphere_colorTab[0].colorDuty_r,
											   atmosphere_colorTab[0].colorDuty_g,
											   atmosphere_colorTab[0].colorDuty_b);
			}break;
			
			case atmosphereLightType_normalNetNode:{
		
				DEVLEDC_ATMOSPHERELED_COLORSET(atmosphere_colorTab[1].colorDuty_r,
											   atmosphere_colorTab[1].colorDuty_g,
											   atmosphere_colorTab[1].colorDuty_b);
			}break;
				
			case atmosphereLightType_normalNetRoot:{
		
				DEVLEDC_ATMOSPHERELED_COLORSET(atmosphere_colorTab[2].colorDuty_r,
											   atmosphere_colorTab[2].colorDuty_g,
											   atmosphere_colorTab[2].colorDuty_b);
			}break;
				
			case atmosphereLightType_dataSaveOpreat:{

				atmosphereLight_statusKeepCount --;
		
				DEVLEDC_ATMOSPHERELED_COLORSET(atmosphere_colorTab[3].colorDuty_r,
											   atmosphere_colorTab[3].colorDuty_g,
											   atmosphere_colorTab[3].colorDuty_b);

				if(!atmosphereLight_statusKeepCount)
					devAtmosphereCurrent_status = atmosphereLightType_none;
											   
			}break;
		
			default:break;
		}
	}

	//关键状态自检业务
	switch(devAtmosphereCurrent_status){ 
			
		case atmosphereLightType_dataSaveOpreat:{


		}break;
	
		default:{

			//系统关键状态自检
			if(mwifi_is_connected()){
			
				if(esp_mesh_get_layer() == MESH_ROOT){
			
					devAtmosphereCurrent_status = atmosphereLightType_normalNetRoot;
				}
				else
				{
					devAtmosphereCurrent_status = atmosphereLightType_normalNetNode;
				}
			}
			else
			{
				devAtmosphereCurrent_status = atmosphereLightType_normalWithoutNet;
			}

		}break;
	}
}

void devAtmosphere_statusTips_trigSet(enum_atmosphereLightType tipsType){

	devAtmosphereCurrent_status = tipsType;

	if(tipsType == atmosphereLightType_dataSaveOpreat)
		devAtmosphereStatus_record = atmosphereLightType_none;
}

void bussiness_devLight_testApp(void)
{
	uint8_t ch = 0;

    while (1) {
        printf("1. LEDC fade up to duty = %d\n", DEVLEDC_DUTY_DEFAULT);
        for (ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
            ledc_set_fade_with_time(devLight_lecCfgParam[ch].speed_mode,
                    devLight_lecCfgParam[ch].channel, DEVLEDC_DUTY_DEFAULT, DEVLEDC_FADE_TIME_DEFAULT);
            ledc_fade_start(devLight_lecCfgParam[ch].speed_mode,
                    devLight_lecCfgParam[ch].channel, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(DEVLEDC_FADE_TIME_DEFAULT / portTICK_PERIOD_MS);

        printf("2. LEDC fade down to duty = 0\n");
        for (ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
            ledc_set_fade_with_time(devLight_lecCfgParam[ch].speed_mode,
                    devLight_lecCfgParam[ch].channel, 0, DEVLEDC_FADE_TIME_DEFAULT);
            ledc_fade_start(devLight_lecCfgParam[ch].speed_mode,
                    devLight_lecCfgParam[ch].channel, LEDC_FADE_NO_WAIT);
        }
        vTaskDelay(DEVLEDC_FADE_TIME_DEFAULT * 2 / portTICK_PERIOD_MS);

        printf("3. LEDC set duty = %d without fade\n", DEVLEDC_DUTY_DEFAULT);
        for (ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
            ledc_set_duty(devLight_lecCfgParam[ch].speed_mode, devLight_lecCfgParam[ch].channel, DEVLEDC_DUTY_DEFAULT);
            ledc_update_duty(devLight_lecCfgParam[ch].speed_mode, devLight_lecCfgParam[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("4. LEDC set duty = 0 without fade\n");
        for (ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
            ledc_set_duty(devLight_lecCfgParam[ch].speed_mode, devLight_lecCfgParam[ch].channel, 0);
            ledc_update_duty(devLight_lecCfgParam[ch].speed_mode, devLight_lecCfgParam[ch].channel);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



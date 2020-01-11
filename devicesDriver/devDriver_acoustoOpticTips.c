#include "devDriver_acoustoOpticTips.h"

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

#if(L8_DEVICE_DEVELOPE_VERSION_DEF == L8_DEVICE_DEVELOPE_VERSION_A)

 #define LEDC_DEV_RESERVE_NUM   				(6)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_LRen		(26)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_UDen		(33)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_R			(16)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_B			(17)

 #define DEVLEDC_ATMOSPHERELED_BRIGHTNESSSET(lr,ud)	 ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].channel, DEVLEDC_DUTY_DEFAULT - (lr * DEVLEDC_COLOR_DUTY_DIV));\
													 ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen].channel);\
													 ledc_set_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].channel, DEVLEDC_DUTY_DEFAULT - (ud * DEVLEDC_COLOR_DUTY_DIV));\
													 ledc_update_duty(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen].channel)
													 
#elif(L8_DEVICE_DEVELOPE_VERSION_DEF == L8_DEVICE_DEVELOPE_VERSION_B)

 #define LEDC_DEV_RESERVE_NUM				    (4)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_R		    (26)
 #define DEVLEDC_PIN_ATMOSPHERE_LIGHT_B		    (33)

#endif
#define DEVDRIVER_BEEPS_GPIO_OUTPUT_PIN			(15)
#define DEVPWMBEEPS_TIMER						LEDC_TIMER_1
#define DEVPWMBEEPS_MODE           				LEDC_HIGH_SPEED_MODE
#define DEVPWMBEEPS_CHANNEL_OUTPUT				LEDC_CHANNEL_7
#define DEVPWMBEEPS_DUTY_DEFAULT      			(100) 	//基础占空比分量

#define DEVLEDC_PIN_ATMOSPHERE_LIGHT_G			(32)
#define DEVLEDC_PIN_SCREEN_BK_LIGHT				(5)
#define DEVLEDC_TIMER							LEDC_TIMER_0
#define DEVLEDC_MODE           					LEDC_HIGH_SPEED_MODE
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_R		LEDC_CHANNEL_0
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_G		LEDC_CHANNEL_1
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_B		LEDC_CHANNEL_2
#define DEVLEDC_CHANNEL_SCREEN_BK_LIGHT			LEDC_CHANNEL_3
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_LRen	LEDC_CHANNEL_4
#define DEVLEDC_CHANNEL_ATMOSPHERE_LIGHT_UDen	LEDC_CHANNEL_5
#define DEVLEDC_DUTY_DEFAULT      				(8000) 	//基础占空比分量
#define DEVLEDC_SCREEN_BRIGHTNESS_DUTY_BASIC	(DEVLEDC_DUTY_DEFAULT / 10 * 1) //最低亮度分度
#define DEVLEDC_SCREEN_BRIGHTNESS_DUTY_LIMITADJ	(DEVLEDC_DUTY_DEFAULT - DEVLEDC_SCREEN_BRIGHTNESS_DUTY_BASIC) //亮度占空比调节范围
#define DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT	(DEVLEDC_SCREEN_BRIGHTNESS_DUTY_LIMITADJ / DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV) //单分度对应占空比

#define DEVLEDC_COLOR_DUTY_DIV					(DEVLEDC_DUTY_DEFAULT / 255)

#define DEVLEDC_FADE_TIME_DEFAULT 				(1000)

#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R		0
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G		1
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B		2
#define DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT		3
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_LRen	4
#define DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_UDen	5

#define DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI	(1000 / DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD)

#define DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN	20	//屏幕半亮灭活检测时间 单位：s
#define DEVVALUE_DEFAULT_SCRBKLT_SHUTDOWN_TIMEDELAY	5	//屏幕全灭滞后时间 单位：s

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
#define DEVLEDC_ATMOSPHERELED_COLORCHG_X(r,g,b,x)	ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel, r * DEVLEDC_COLOR_DUTY_DIV, x);\
													ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_R].channel, LEDC_FADE_NO_WAIT);\
													ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel, g * DEVLEDC_COLOR_DUTY_DIV, x);\
													ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_G].channel, LEDC_FADE_NO_WAIT);\
													ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel, b * DEVLEDC_COLOR_DUTY_DIV, x);\
													ledc_fade_start(devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].speed_mode, devLight_lecCfgParam[DEVLEDC_PARAMNUM_ATMOSPHERE_LIGHT_B].channel, LEDC_FADE_NO_WAIT)

#define DEVDRIVER_BEEPS_PIN_LOSEF()					ledc_set_duty(devBeeps_pwmCfgParam.speed_mode, devBeeps_pwmCfgParam.channel, 0);\
            										ledc_update_duty(devBeeps_pwmCfgParam.speed_mode, devBeeps_pwmCfgParam.channel)
#define DEVDRIVER_BEEPS_DUTY_SET(x)					ledc_set_duty(devBeeps_pwmCfgParam.speed_mode, devBeeps_pwmCfgParam.channel, DEVPWMBEEPS_DUTY_DEFAULT / 10 * x);\
            										ledc_update_duty(devBeeps_pwmCfgParam.speed_mode, devBeeps_pwmCfgParam.channel)
#define DEVDRIVER_BEEPS_FREQ_SET(x)					ledc_set_freq(DEVPWMBEEPS_MODE, DEVPWMBEEPS_TIMER, 300 * x + 500)

extern EventGroupHandle_t xEventGp_infraActDetect;

static const struct _stt_atmosphere_colorTab{

	uint32_t colorDuty_r;	
	uint32_t colorDuty_g;	
	uint32_t colorDuty_b;	
}atmosphere_colorTab[5] = {

	{255,   0,   0},
	{128,   0, 214},
	{  0, 100, 256},
	{ 50, 200,   0},
	{ 50, 	0, 150},
};

static stt_devScreenRunningParam devScreenConfigParam = {

	.timePeriod_devScreenBkLight_weakDown = DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN,
	.devScreenBkLight_brightness = 64,
	.devScreenBkLight_brightnessSleep = 8,
};
static stt_devBeepRunningParam devBeepRunningParam = {0};

static uint8_t  devScreenConfigParam_nvsSave_timeDelay_count = 0;	//存储延迟计时变量
static uint32_t timePeriod_devScreenBkLight_shutDown = DEVVALUE_DEFAULT_PERIOD_SCRBKLT_WEAKDOWN + DEVVALUE_DEFAULT_SCRBKLT_SHUTDOWN_TIMEDELAY; //屏幕全灭灭活检测计时变量
static uint32_t timeCounter_devScreenBkLight_keepAlive = 0; //屏幕背光活力计时器变量
static bool 	screenBkLight_shutDownFLG = false; //屏幕背光熄灭标志
static bool		screenBkLight_initializedFLG = false;

static enum_screenBkLight_status devScreenBklightCurrent_status = screenBkLight_statusEmpty;
static enum_atmosphereLightType devAtmosphereCurrent_status = atmosphereLightType_normalWithoutNet;
static enum_atmosphereLightType devAtmosphereStatus_record = atmosphereLightType_none;

static uint16_t tpisCounter_systemUpgrading = 0;

static const ledc_channel_config_t devBeeps_pwmCfgParam = {

	.channel	= DEVPWMBEEPS_CHANNEL_OUTPUT,
	.duty		= 0,
	.gpio_num	= DEVDRIVER_BEEPS_GPIO_OUTPUT_PIN,
	.speed_mode = DEVPWMBEEPS_MODE,
	.hpoint 	= 0,
	.timer_sel	= DEVPWMBEEPS_TIMER
};

static const ledc_channel_config_t devLight_lecCfgParam[LEDC_DEV_RESERVE_NUM] = {

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
		.channel	= DEVLEDC_CHANNEL_SCREEN_BK_LIGHT,
		.duty		= 0,
		.gpio_num	= DEVLEDC_PIN_SCREEN_BK_LIGHT,
		.speed_mode = DEVLEDC_MODE,
		.hpoint 	= 0,
		.timer_sel	= DEVLEDC_TIMER
	},
#if(L8_DEVICE_DEVELOPE_VERSION_DEF == L8_DEVICE_DEVELOPE_VERSION_A)
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
#endif
};

static void devScreenBkLight_brightnessSet(enum_screenBkLight_status val){

	uint32_t pwmHalf_brightness = 0;
	uint8_t compensateWe_pwmHalf_brightness = devScreenConfigParam.devScreenBkLight_brightness - devScreenConfigParam.devScreenBkLight_brightnessSleep;

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	return;
#endif

	if(!screenBkLight_initializedFLG)return;

	if(compensateWe_pwmHalf_brightness > 0){
	
		pwmHalf_brightness = (uint32_t)(devScreenConfigParam.devScreenBkLight_brightnessSleep + (compensateWe_pwmHalf_brightness / 5)) * DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT;
	}
	else
	{
		pwmHalf_brightness = (uint32_t)(devScreenConfigParam.devScreenBkLight_brightness) * DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT;
	}

	devScreenBklightCurrent_status = val;

	switch(devScreenBklightCurrent_status){

		case screenBkLight_statusEmpty:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
									(uint32_t)(devScreenConfigParam.devScreenBkLight_brightnessSleep) * DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT,
									DEVLEDC_FADE_TIME_DEFAULT);

		}break;

		case screenBkLight_statusHalf:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
//									DEVLEDC_SCREEN_BRIGHTNESS_DUTY_BASIC, 
									pwmHalf_brightness,
									DEVLEDC_FADE_TIME_DEFAULT);

		}break;

		case screenBkLight_statusFull:{

			ledc_set_fade_with_time(devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].speed_mode, 
									devLight_lecCfgParam[DEVLEDC_PARAMNUM_SCREEN_BK_LIGHT].channel, 
//									(uint32_t)(devScreenConfigParam.devScreenBkLight_brightness) * DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT + DEVLEDC_SCREEN_BRIGHTNESS_DUTY_BASIC, 
									(uint32_t)(devScreenConfigParam.devScreenBkLight_brightness) * DEVLEDC_SCREEN_BRIGHTNESS_ADJ_PERCENT, 
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

void deviceHardwareAcoustoOptic_Init(void){

	gpio_config_t io_conf = {0};
	
	/*
	 * Prepare and set configuration of timers
	 * that will be used by LED Controller
	 */
	const ledc_timer_config_t devLight_timer = {
	
		.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
		.freq_hz = 5000,					  // frequency of PWM signal
		.speed_mode = DEVLEDC_MODE, 		  // timer mode
		.timer_num = DEVLEDC_TIMER			  // timer index
	},
	devBeeps_timer = {

		.duty_resolution = LEDC_TIMER_7_BIT, // resolution of PWM duty
		.freq_hz = 1000,					  // frequency of PWM signal
		.speed_mode = DEVPWMBEEPS_MODE, 	  // timer mode
		.timer_num = DEVPWMBEEPS_TIMER		  // timer index
	};

//	//BEEPS引脚初始化
//	//disable interrupt
//	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//	//set as output mode
//	io_conf.mode = GPIO_MODE_OUTPUT;
//	//bit mask of the pins that you want to set
//	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_BEEPS_GPIO_OUTPUT_PIN);
//	//disable pull-down mode
//	io_conf.pull_down_en = 1;
//	//disable pull-up mode
//	io_conf.pull_up_en = 0;
//	//configure GPIO with the given settings
//	gpio_config(&io_conf);

	// PWM初始化
	// Set configuration of timer0 for high speed channels
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
	
#else
	
	ledc_timer_config(&devLight_timer);
#endif

	ledc_timer_config(&devBeeps_timer);

//	// Prepare and set configuration of timer1 for low speed channels
//	devLight_timer.speed_mode = LEDC_LOW_SPEED_MODE;
//	devLight_timer.timer_num = DEVLEDC_TIMER;
//	ledc_timer_config(&devLight_timer);

	// Set LED Controller with previously prepared configuration
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
		
#else
		
	for (uint8_t ch = 0; ch < LEDC_DEV_RESERVE_NUM; ch++) {
		ledc_channel_config(&devLight_lecCfgParam[ch]);
	}	
#endif

	ledc_channel_config(&devBeeps_pwmCfgParam);

	// Initialize fade service.
	ledc_fade_func_install(0);
	screenBkLight_initializedFLG = true; //初始化完成标志置位
	devScreenBkLight_brightnessSet(screenBkLight_statusEmpty);
//	devBeepTips_trig(4, 8, 100, 40, 1);
//	vTaskDelay(1000 / portTICK_PERIOD_MS);
//	DEVLEDC_ATMOSPHERELED_BRIGHTNESSSET(240, 200);	
}

enum_screenBkLight_status devScreenBkLight_brightnessGet(void){

	return devScreenBklightCurrent_status;
}

void tipsOpreatSet_sysUpgrading(uint16_t valSet){

	tpisCounter_systemUpgrading = valSet;
}

void tipsOpreatAutoSet_sysUpgrading(void){

	uint8_t meshNodeNum = (uint8_t)esp_mesh_get_total_node_num();

	if(meshNodeNum < 30)tpisCounter_systemUpgrading = 180;
	else{

		tpisCounter_systemUpgrading = meshNodeNum / 10 * 60; //每10个一分钟
	}
}

void tipsSysUpgrading_realesRunning(void){

	if(tpisCounter_systemUpgrading)tpisCounter_systemUpgrading --;
}

void devBeepTips_trig(uint8_t tones, 
						   uint8_t vol,
						   uint16_t timeKeep, 
						   uint16_t timePulse, 
						   uint8_t loop){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();
	
	if(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_NIGHTMODE){ //夜间模式蜂鸣器失能

		devBeepRunningParam.trig = 0;
		return;
	}

	if(devBeepRunningParam.trig)return;

	devBeepRunningParam.tones = tones;
	devBeepRunningParam.volume = vol;
	devBeepRunningParam.keepPeriod = timeKeep;
	devBeepRunningParam.pulsePeriod = timePulse;
	devBeepRunningParam.loop = loop;

	devBeepRunningParam.runningCounter = 0;
	devBeepRunningParam.trig = 1;

	DEVDRIVER_BEEPS_FREQ_SET(devBeepRunningParam.tones);
	DEVDRIVER_BEEPS_DUTY_SET(devBeepRunningParam.volume);
}

void devScreenBkLight_weakUp(void){

	if(devScreenBkLight_brightnessGet() != screenBkLight_statusFull)devScreenBkLight_brightnessSet(screenBkLight_statusFull);
	
	timeCounter_devScreenBkLight_keepAlive = 0;
	screenBkLight_shutDownFLG = false;
}

uint8_t devAcoustoOptic_beepBussinessRefresh(void){

	static enum statusMac_beepsRun{

		beepsRunStatus_stop = 0,
		beepsRunStatus_run,
		beepsRunStatus_pulse
	}statusBeepsRunnig = beepsRunStatus_stop;
	bool beepsEn = false;
	static bool beepsEn_rcd = false;

	//蜂鸣器业务
	if(devBeepRunningParam.trig){

		if(devBeepRunningParam.loop){

			if(devBeepRunningParam.runningCounter){

				devBeepRunningParam.runningCounter --;

				if(beepsRunStatus_run == statusBeepsRunnig){

					beepsEn = true;
				}
				else
				{
					beepsEn = false;
				}
			}
			else
			{
				switch(statusBeepsRunnig){

					case beepsRunStatus_stop:{

						statusBeepsRunnig = beepsRunStatus_run;
						devBeepRunningParam.runningCounter = devBeepRunningParam.keepPeriod * 4;

					}break;
					
					case beepsRunStatus_run:{

						statusBeepsRunnig = beepsRunStatus_pulse;
						devBeepRunningParam.runningCounter = devBeepRunningParam.pulsePeriod * 4;

					}break;
					
					case beepsRunStatus_pulse:{

						statusBeepsRunnig = beepsRunStatus_stop;
						devBeepRunningParam.runningCounter = 0;

						devBeepRunningParam.loop --;
					
					}break;

					default:{

						statusBeepsRunnig = beepsRunStatus_stop;
						devBeepRunningParam.runningCounter = 0;

						devBeepRunningParam.trig = 0;
					}
				}
			}
		}
		else
		{
			devBeepRunningParam.trig = 0;
		}
	}
	else
	{
		beepsEn = false;
	}

	if(beepsEn_rcd != beepsEn){

		beepsEn_rcd = beepsEn;
		if(beepsEn){

			DEVDRIVER_BEEPS_DUTY_SET(devBeepRunningParam.volume);
		}
		else
		{
			DEVDRIVER_BEEPS_PIN_LOSEF();
		}
	}

	return statusBeepsRunnig;
}

void devAcoustoOptic_statusRefresh(void){

	static uint16_t atmosphereLight_transitionCount = 0; //氛围灯转场时间
	static uint16_t atmosphereLight_statusKeepCount = 0;
		   uint16_t devRunningFlg_temp = 0;
	static bool     devNightmodeFlg_record = false;
	EventBits_t infraActDetect_etBits = 0;

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET) ||\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
	
	return;
#endif

	devRunningFlg_temp = currentDevRunningFlg_paramGet();

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
	
	//红外动作探测业务
	infraActDetect_etBits = xEventGroupWaitBits(xEventGp_infraActDetect, 
											 	INFRAACTDETECTEVENT_FLG_BITHOLD_RESERVE,
												pdTRUE,
												pdFALSE,
												0);
	if((infraActDetect_etBits & INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN) == INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN){

		stt_paramLinkageConfig linkageConfigParamGet_temp = {0};

		devSystemOpration_linkageConfig_paramGet(&linkageConfigParamGet_temp);

//		printf("infraActDetect trig!.\n");
//		devAtmosphere_statusTips_trigSet(atmosphereLightType_infraActDetectTrig); //氛围灯触发

		if(linkageConfigParamGet_temp.linkageRunning_proxmity_en){

			if(linkageConfigParamGet_temp.linkageReaction_proxmity_scrLightTrigIf){

				devScreenBkLight_weakUp();
			}

			if(linkageConfigParamGet_temp.linkageReaction_proxmity_trigEn){

				currentDev_dataPointSet(&(linkageConfigParamGet_temp.linkageReaction_proxmity_swVal), true, true, true, false);
			}
		}
	}

	//背光灯业务
	if(timeCounter_devScreenBkLight_keepAlive < (devScreenConfigParam.timePeriod_devScreenBkLight_weakDown * DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI)){

		if(devScreenConfigParam.timePeriod_devScreenBkLight_weakDown != COUNTER_DISENABLE_MASK_SPECIALVAL_U16) //常亮档位设置检测
			timeCounter_devScreenBkLight_keepAlive ++;

		if(timeCounter_devScreenBkLight_keepAlive > ((devScreenConfigParam.timePeriod_devScreenBkLight_weakDown - DEVVALUE_DEFAULT_SCRBKLT_SHUTDOWN_TIMEDELAY) * DEVLEDC_DRIVER_REFRESH_TIME_PERIOD_COEFFI)){

			devScreenBkLight_brightnessSet(screenBkLight_statusHalf);
		}
			
	}else{

		if(!screenBkLight_shutDownFLG){

			devScreenBkLight_brightnessSet(screenBkLight_statusEmpty);

			screenBkLight_shutDownFLG = true;
		}
	}

	//氛围灯业务
	if(tpisCounter_systemUpgrading){ //升级提示 --强优先
	
		static bool lightFlash_statusRecord = false;

		lightFlash_statusRecord = !lightFlash_statusRecord;

		if(lightFlash_statusRecord){

			DEVLEDC_ATMOSPHERELED_COLORCHG(255, 0, 0);
		}
		else
		{
			DEVLEDC_ATMOSPHERELED_COLORCHG(0, 0, 0);
		}
	}
	else // --弱优先提示
	{
		if(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_NIGHTMODE){ //夜间模式氛围灯关闭
	
			if(false == devNightmodeFlg_record){ //氛围灯失能
	
				devNightmodeFlg_record = true;
	
				DEVLEDC_ATMOSPHERELED_COLORCHG(0, 0, 0);
			}
		}
		else //非夜间模式氛围灯有效
		{
			if(devNightmodeFlg_record)devNightmodeFlg_record = false;

			if(screenBkLight_shutDownFLG){ //触摸已被释放

				const uint8_t tabSin[] = {

					0, 17, 34, 52, 69, 87, 104, 121, 139, 156, 173, 190, 207, 224, 241, 255
				};
				const uint8_t breathActPeriod = 10;
				static struct stt_paramBreath{

					uint8_t counter;
					uint8_t period;
					uint8_t dir:1;
				}breathCounter = {0, 10};
				static uint8_t random[3] = {0};
				uint16_t bLight_temp[3] = {0};
				uint8_t loop;

				if(breathCounter.counter < breathCounter.period)breathCounter.counter ++;
				else{

					breathCounter.counter = 0;
					breathCounter.dir = !breathCounter.dir;

					if(breathCounter.dir){

						for(loop = 0; loop < 3; loop ++)random[loop] = (uint8_t)(esp_random() % 16);
						bLight_temp[0] = tabSin[random[0]] / 4 * 2;
						bLight_temp[1] = tabSin[random[1]] / 4 * 3;
						bLight_temp[2] = tabSin[random[2]] / 4 * 4;
						DEVLEDC_ATMOSPHERELED_COLORCHG_X(bLight_temp[0],
														 bLight_temp[1],
														 bLight_temp[2],
														 DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD * (breathActPeriod - 1));
						breathCounter.period = breathActPeriod; 
					}
					else
					{
						DEVLEDC_ATMOSPHERELED_COLORCHG_X(0, 0, 0, DEVDRIVER_DEVSELFLIGHT_REFRESH_PERIOD * (breathActPeriod - 1));
						breathCounter.period = breathActPeriod * 3; 
					}
				}

//				printf("r:%d, g:%d, b:%d.\n", bLight_temp[0], bLight_temp[1], bLight_temp[2]);
			}
			else //触摸占用
			{
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
			
				if(atmosphereLight_transitionCount){ //atmosphereLight_transitionCount非零，需要运行转场效果
			
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
			
//					printf("atmosphereLight_transitionCount:%d.\n", atmosphereLight_transitionCount);
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
					
						default:{ //全灭
			
							DEVLEDC_ATMOSPHERELED_COLORSET(0, 0, 0);
			
						}break;
					}
				}

			}
		}
	}
}

void devAtmosphere_statusTips_trigSet(enum_atmosphereLightType tipsType){

	devAtmosphereCurrent_status = tipsType;

	if(tipsType == atmosphereLightType_dataSaveOpreat)
		devAtmosphereStatus_record = atmosphereLightType_none;
}

static void devScreenDriver_configParamSave_actionTrig(void){

	devSystemInfoLocalRecord_save(saveObj_devDriver_screenRunningParam_set, &devScreenConfigParam);
}

void devScreenDriver_configParamSave_actionDetect(void){ //检测是否有延迟存储要求，

	if(devScreenConfigParam_nvsSave_timeDelay_count != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){

		if(devScreenConfigParam_nvsSave_timeDelay_count)devScreenConfigParam_nvsSave_timeDelay_count --;
		else{

			devScreenConfigParam_nvsSave_timeDelay_count = COUNTER_DISENABLE_MASK_SPECIALVAL_U8;
			devScreenDriver_configParamSave_actionTrig();
		}
	}
}

void devScreenDriver_configParam_brightness_set(uint8_t brightnessVal, bool nvsRecord_IF){

	printf("brightness set val:%d.\n", brightnessVal);

	if(brightnessVal > DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV)
		brightnessVal = DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV;

	devScreenConfigParam.devScreenBkLight_brightness = brightnessVal;
	devScreenBkLight_brightnessSet(screenBkLight_statusFull);	

	if(nvsRecord_IF)
		devScreenConfigParam_nvsSave_timeDelay_count = DEVDRIVER_DEVSELFLIGHT_SCR_PARAMSAVE_TIMEDELAY;
}

void devScreenDriver_configParam_brightnessSleep_set(uint8_t brightnessSleepVal, bool nvsRecord_IF){

	printf("brightnessSleep set val:%d.\n", brightnessSleepVal);

	if(brightnessSleepVal > DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV)
		brightnessSleepVal = DEVLEDC_SCREEN_BRIGHTNESS_LEVEL_DIV;

	devScreenConfigParam.devScreenBkLight_brightnessSleep = brightnessSleepVal;
	devScreenBkLight_brightnessSet(screenBkLight_statusFull);	

	if(nvsRecord_IF)
		devScreenConfigParam_nvsSave_timeDelay_count = DEVDRIVER_DEVSELFLIGHT_SCR_PARAMSAVE_TIMEDELAY;
}

uint8_t devScreenDriver_configParam_brightness_get(void){

	return devScreenConfigParam.devScreenBkLight_brightness;
}

uint8_t devScreenDriver_configParam_brightnessSleep_get(void){

	return devScreenConfigParam.devScreenBkLight_brightnessSleep;
}

void devScreenDriver_configParam_screenLightTime_set(uint32_t timeVal, bool nvsRecord_IF){

	devScreenConfigParam.timePeriod_devScreenBkLight_weakDown = timeVal;
	devScreenBkLight_weakUp();

	if(nvsRecord_IF)
		devScreenConfigParam_nvsSave_timeDelay_count = DEVDRIVER_DEVSELFLIGHT_SCR_PARAMSAVE_TIMEDELAY;
}

uint32_t devScreenDriver_configParam_screenLightTime_get(void){

	return devScreenConfigParam.timePeriod_devScreenBkLight_weakDown;
}

void devScreenDriver_configParam_set(stt_devScreenRunningParam *param, bool nvsRecord_IF){

	memcpy(&devScreenConfigParam, param, sizeof(stt_devScreenRunningParam));

	if(nvsRecord_IF)
		devScreenConfigParam_nvsSave_timeDelay_count = DEVDRIVER_DEVSELFLIGHT_SCR_PARAMSAVE_TIMEDELAY;
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



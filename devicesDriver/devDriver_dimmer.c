#include "devDriver_dimmer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#define DEVDRIVER_DIMMER_GPIO_IO_OUTP     	 12

#define DEVDRIVER_DIMMER_PCNT_TEST_UNIT      PCNT_UNIT_1
#define DEVDRIVER_DIMMER_PCNT_H_LIM_VAL      1
#define DEVDRIVER_DIMMER_PCNT_L_LIM_VAL     -10
#define DEVDRIVER_DIMMER_PCNT_THRESH1_VAL    5
#define DEVDRIVER_DIMMER_PCNT_THRESH0_VAL   -5
#define DEVDRIVER_DIMMER_PCNT_INPUT_SIG_IO   27  // Pulse Input GPIO
#define DEVDRIVER_DIMMER_PCNT_INPUT_CTRL_IO  36  // Control GPIO HIGH=count up, LOW=count down

static bool devDriver_moudleInitialize_Flg = false;

static stt_Dimmer_attrFreq devParam_dimmer = {0};
static struct _paramDynamic_dimmerAdj{

	uint16_t valBrightness_target;

}paramDynamic_dimmerAdj = {0};
static pcnt_isr_handle_t user_isr_handle = NULL;

static void IRAM_ATTR pcnt_isr_handler(void* arg){
	
	uint32_t intr_status = PCNT.int_st.val;
	int	loop = 0;

	for (loop = 0; loop < PCNT_UNIT_MAX; loop++){

		switch(intr_status & (BIT(loop))){

			case PCNT_STATUS_H_LIM:{

//				devParam_dimmer.periodBeat_cfm = devParam_dimmer.periodBeat_counter;
//				devParam_dimmer.periodBeat_counter = 0;

//				devParam_dimmer.pwm_actEN = 1;
				
			}break;

			default:break;
		}

		if(intr_status & (BIT(loop)))
			PCNT.int_clr.val = BIT(loop);
	}
	
	devParam_dimmer.periodBeat_cfm = devParam_dimmer.periodBeat_counter;
	devParam_dimmer.periodBeat_counter = 0;
	
	devParam_dimmer.pwm_actEN = 1;
	
//	/*debug code*/
//	devParam_dimmer.couterSoureFreq_Debug ++;
}

void IRAM_ATTR devDriverBussiness_dimmerSwitch_debug(void){

	pcnt_get_counter_value(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, (int16_t *)&(devParam_dimmer.periodSoureFreq_Debug));
}

void IRAM_ATTR devDriverBussiness_dimmerSwitch_decCounterReales(void){ 

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();
	static uint32_t gpio_out_put_status = 0;
//	uint16_t freq_periodBeatHalf = devParam_dimmer.periodBeat_cfm / 2;

	if(swCurrentDevType != devTypeDef_dimmer)return;

	devParam_dimmer.periodBeat_counter ++;

	if(devParam_dimmer.pwm_actEN){

		devParam_dimmer.pwm_actCounter ++;
		if((devParam_dimmer.pwm_actCounter < devParam_dimmer.pwm_actPeriod)){
//		if((devParam_dimmer.pwm_actCounter < devParam_dimmer.pwm_actPeriod) &&
//		   (devParam_dimmer.pwm_actCounter < freq_periodBeatHalf)){

		   if(gpio_out_put_status != 1){
		   
			   gpio_set_level(DEVDRIVER_DIMMER_GPIO_IO_OUTP, 1);
			   gpio_out_put_status = 1;
		   }
		}
		else
		{
			devParam_dimmer.pwm_actCounter = 0;
			devParam_dimmer.pwm_actEN = 0;
			if(gpio_out_put_status != 0){
	
				gpio_set_level(DEVDRIVER_DIMMER_GPIO_IO_OUTP, 0);
				gpio_out_put_status = 0;
			}
		}
	}
	else
	{
		if(gpio_out_put_status != 0){
		
			gpio_set_level(DEVDRIVER_DIMMER_GPIO_IO_OUTP, 0);
			gpio_out_put_status = 0;
		}
	}
}

void IRAM_ATTR devDriverBussiness_dimmerSwitch_currentParamGet(stt_Dimmer_attrFreq *param){

	memcpy(param, &devParam_dimmer, sizeof(stt_Dimmer_attrFreq));
}

static void devDriverBussiness_dimmerSwitch_loadInit(void){

	gpio_config_t io_conf = {0};

	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_DIMMER_GPIO_IO_OUTP);
	//disable pull-down mode
	io_conf.pull_down_en = 1;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

static void devDriverBussiness_dimmerSwitch_loadDeinit(void){

	gpio_reset_pin(DEVDRIVER_DIMMER_GPIO_IO_OUTP);
}

static void devDriverBussiness_dimmerSwitch_pcntInit(void){

    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = DEVDRIVER_DIMMER_PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = DEVDRIVER_DIMMER_PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = DEVDRIVER_DIMMER_PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = DEVDRIVER_DIMMER_PCNT_H_LIM_VAL,
        .counter_l_lim = DEVDRIVER_DIMMER_PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, 300);
    pcnt_filter_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);

    /* Set threshold 0 and 1 values and enable events to watch */
//    pcnt_set_event_value(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, 
//    					 PCNT_EVT_THRES_1, 
//    					 DEVDRIVER_DIMMER_PCNT_THRESH1_VAL);

//    pcnt_event_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_1);
	
//    pcnt_set_event_value(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, 
//						 PCNT_EVT_THRES_0, 
//						 DEVDRIVER_DIMMER_PCNT_THRESH0_VAL);
//	
//    pcnt_event_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_0);
	
//    /* Enable events on zero, maximum and minimum limit values */
//    pcnt_event_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, PCNT_EVT_ZERO);
    pcnt_event_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
//    pcnt_event_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);
    pcnt_counter_clear(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);

    /* Register ISR handler and enable interrupts for PCNT unit */
    pcnt_isr_register(pcnt_isr_handler, NULL, 0, &user_isr_handle);
    pcnt_intr_enable(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);
}

static void devDriverBussiness_dimmerSwitch_pcntDeinit(void){

	pcnt_isr_handler_remove(DEVDRIVER_DIMMER_PCNT_TEST_UNIT);
}

static void devDriverBussiness_dimmerSwitch_periphInit(void){

	devDriverBussiness_dimmerSwitch_loadInit();
	devDriverBussiness_dimmerSwitch_pcntInit();
}

static void devDriverBussiness_dimmerSwitch_periphDeinit(void){

	devDriverBussiness_dimmerSwitch_loadDeinit();
	devDriverBussiness_dimmerSwitch_pcntDeinit();
}

void devDriverBussiness_dimmerSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_dimmer)return;
	if(devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_dimmerSwitch_periphInit();

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_dimmerSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_dimmerSwitch_periphDeinit();

	devDriver_moudleInitialize_Flg = false;
}

void IRAM_ATTR devDriverBussiness_dimmerSwitch_brightnessAdjDetectLoop(void){

	if(devParam_dimmer.pwm_actPeriod == paramDynamic_dimmerAdj.valBrightness_target)return;

	if(devParam_dimmer.pwm_actPeriod < paramDynamic_dimmerAdj.valBrightness_target)devParam_dimmer.pwm_actPeriod ++;
	if(devParam_dimmer.pwm_actPeriod > paramDynamic_dimmerAdj.valBrightness_target)devParam_dimmer.pwm_actPeriod --;
}

void devDriverBussiness_dimmerSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_dimmer){

		paramDynamic_dimmerAdj.valBrightness_target = (devParam_dimmer.periodBeat_cfm / 100) * param->devType_dimmer.devDimmer_brightnessVal;

//		if(devParam_dimmer.periodBeat_cfm > 500){ //过零检测单周期过长，则进行低亮度补偿

//			uint16_t brightnessCompensation = devParam_dimmer.periodBeat_cfm / 100 * 12; //12%亮度补偿

//			(param->devType_dimmer.devDimmer_brightnessVal > 0)?
//				(paramDynamic_dimmerAdj.valBrightness_target = (brightnessCompensation + (devParam_dimmer.periodBeat_cfm - brightnessCompensation) / 100) * param->devType_dimmer.devDimmer_brightnessVal):
//				(paramDynamic_dimmerAdj.valBrightness_target = 0);
//		}
//		devParam_dimmer.pwm_actPeriod = (devParam_dimmer.periodBeat_cfm / 100) * param->devType_dimmer.devDimmer_brightnessVal;
	}
} 



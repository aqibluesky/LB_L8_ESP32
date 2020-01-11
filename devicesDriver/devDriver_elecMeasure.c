#include "devDriver_elecMeasure.h"

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

#include "devDriver_manage.h"

#include "bussiness_timerSoft.h"

#define DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT      PCNT_UNIT_2
#define DEVDRIVER_ELECMEASURE_PCNT_H_LIM_VAL      20000
#define DEVDRIVER_ELECMEASURE_PCNT_L_LIM_VAL     -10
#define DEVDRIVER_ELECMEASURE_PCNT_THRESH1_VAL    5
#define DEVDRIVER_ELECMEASURE_PCNT_THRESH0_VAL   -5
#define DEVDRIVER_ELECMEASURE_PCNT_INPUT_SIG_IO   35  // Pulse Input GPIO
#define DEVDRIVER_ELECMEASURE_PCNT_INPUT_CTRL_IO  36  // Control GPIO HIGH=count up, LOW=count down

static stt_eleSocket_attrFreq devParam_ElecDetect = {0};

static void IRAM_ATTR pcnt_isr_handler(uint32_t evtStatus){

	if(evtStatus & PCNT_STATUS_H_LIM){

		
	}
}

void devDriverBussiness_elecMeasure_paramProcess(void){ //调用周期:DEVDRIVER_ELECMEASURE_PARAM_PROCESS_PERIOD s

	stt_localTime timeNow = {0};
	const float processPeriod_paramTemp = DEVDRIVER_ELECMEASURE_PARAM_PROCESS_PERIOD;
	static stt_timerLoop_ext paramElecSumSave = {(120 / (uint16_t)processPeriod_paramTemp), 0};

	usrAppDevCurrentSystemTime_paramGet(&timeNow);

	if(0 == timeNow.time_Minute){

		if(devParam_ElecDetect.ele_Consum > 1.0F)devParam_ElecDetect.ele_Consum = 0.0F;
	}

	if(paramElecSumSave.loopCounter < paramElecSumSave.loopPeriod)paramElecSumSave.loopCounter ++;
	else{

		paramElecSumSave.loopCounter = 0;
		
		devDriverBussiness_elecMeasure_elecSumSave2Flash();
	}
}

float devDriverBussiness_elecMeasure_powerCaculateReales(void){

	int16_t freq_elecMeansure = 0;
	float   elecSum_calcuTemp = 0.0F;

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_HEATER) //热水器电量功能暂隐
#else

	return 0.0F;
#endif

	pcnt_get_counter_value(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, &freq_elecMeansure);

//	printf("pcntCount:%d.\n", freq_elecMeansure);

	devParam_ElecDetect.eleParamFun_powerPulseCount = (float)freq_elecMeansure;

	devParam_ElecDetect.eleParamFun_powerFreqVal = 
		devParam_ElecDetect.eleParamFun_powerPulseCount / DEVDRIVER_ELECMEASURE_FDETECT_PERIOD;
	devParam_ElecDetect.eleParam_power = 
		devParam_ElecDetect.eleParamFun_powerFreqVal *
		(DEVDRIVER_ELECMEASURE_COEFFICIENT_POW - (DEVDRIVER_ELECMEASURE_COEFFICIENT_COMPENSATION_POW * devParam_ElecDetect.eleParamFun_powerFreqVal));

	if(devParam_ElecDetect.eleParamFun_powerFreqVal < 0.00001F)devParam_ElecDetect.eleParamFun_powerFreqVal = 0.00001F;

	elecSum_calcuTemp = 1.00F * (devParam_ElecDetect.eleParamFun_powerPulseCount * devParam_ElecDetect.eleParam_power / (1000.00F * 3600.00F * devParam_ElecDetect.eleParamFun_powerFreqVal));
	
	devParam_ElecDetect.ele_Consum += elecSum_calcuTemp;
	devParam_ElecDetect.ele_Consum_localRecord += elecSum_calcuTemp;

	pcnt_counter_clear(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);
		
	return devParam_ElecDetect.eleParam_power;
}

void devDriverBussiness_elecMeasure_elecSumRealesFromFlash(uint32_t param){

	devParam_ElecDetect.ele_Consum_localRecord = (float)param / DEVDRIVER_ELECMEASURE_TRANSFOR_PRECISION;
}

void devDriverBussiness_elecMeasure_elecSumSave2Flash(void){

	uint32_t elecSum_temp = (uint32_t)(devParam_ElecDetect.ele_Consum_localRecord * DEVDRIVER_ELECMEASURE_TRANSFOR_PRECISION);
	devSystemInfoLocalRecord_save(saveObj_devEelcSum, &elecSum_temp);	
}

void devDriverBussiness_elecMeasure_elecSumResetClear(void){

	uint32_t elecSum_temp = 1;
	devParam_ElecDetect.ele_Consum_localRecord = 0.0001F;
	devSystemInfoLocalRecord_save(saveObj_devEelcSum, &elecSum_temp);	
}

float devDriverBussiness_elecMeasure_valElecPowerGet(void){

	return devParam_ElecDetect.eleParam_power;
}

float devDriverBussiness_elecMeasure_valElecConsumGet(void){

	return devParam_ElecDetect.ele_Consum_localRecord;
}

void devDriverBussiness_elecMeasure_valPowerGetByHex(stt_devPowerParam2Hex *param){

	const float decimal_prtCoefficient = 100.0F; //小数计算偏移倍数 --100倍对应十进制两位

	uint16_t dataInteger_prt = (uint16_t)devParam_ElecDetect.eleParam_power & 0xFFFF;
	uint8_t dataDecimal_prt = (uint8_t)((devParam_ElecDetect.eleParam_power - (float)dataInteger_prt) * decimal_prtCoefficient);

	//只可能为正数，不做负数处理

	param->integer_h8bit = (uint8_t)((dataInteger_prt & 0xFF00) >> 8);
	param->integer_l8bit = (uint8_t)((dataInteger_prt & 0x00FF) >> 0);
	param->decimal_8bit = dataDecimal_prt;
}

void devDriverBussiness_elecMeasure_valElecsumGetByHex(stt_devElecsumParam2Hex *param){

	const float decimal_prtCoefficient = 100.0F; //小数计算偏移倍数 --100倍对应十进制两位

	uint16_t dataInteger_prt = (uint16_t)devParam_ElecDetect.ele_Consum & 0xFFFF;
	uint8_t dataDecimal_prt = (uint8_t)((devParam_ElecDetect.ele_Consum - (float)dataInteger_prt) * decimal_prtCoefficient);

	//只可能为正数，不做负数处理
	param->integer_h8bit = (uint8_t)((dataInteger_prt & 0xFF00) >> 8);
	param->integer_l8bit = (uint8_t)((dataInteger_prt & 0x00FF) >> 0);
	param->decimal_8bit = dataDecimal_prt;
}

static void devDriverBussiness_elecMeasure_pcntInit(void){

    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = DEVDRIVER_ELECMEASURE_PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = DEVDRIVER_ELECMEASURE_PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = DEVDRIVER_ELECMEASURE_PCNT_H_LIM_VAL,
        .counter_l_lim = DEVDRIVER_ELECMEASURE_PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, 300);
    pcnt_filter_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);

    /* Set threshold 0 and 1 values and enable events to watch */
//    pcnt_set_event_value(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, 
//    					 PCNT_EVT_THRES_1, 
//    					 DEVDRIVER_ELECMEASURE_PCNT_THRESH1_VAL);

//    pcnt_event_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_1);
	
//    pcnt_set_event_value(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, 
//						 PCNT_EVT_THRES_0, 
//						 DEVDRIVER_ELECMEASURE_PCNT_THRESH0_VAL);
//	
//    pcnt_event_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_0);
	
//    /* Enable events on zero, maximum and minimum limit values */
//    pcnt_event_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, PCNT_EVT_ZERO);
//    pcnt_event_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
//    pcnt_event_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);
    pcnt_counter_clear(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);

    /* Register ISR handler and enable interrupts for PCNT unit */
	isrHandleFuncPcntUnit_regster(pcnt_isr_handler, DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT); //pcnt应用处理函数注册，实际pcnt中断函数在devDriver_manage.c中
    pcnt_intr_enable(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(DEVDRIVER_ELECMEASURE_PCNT_TEST_UNIT);
}

void devDriverBussiness_elecMeasure_periphInit(void){

	devDriverBussiness_elecMeasure_pcntInit();
}



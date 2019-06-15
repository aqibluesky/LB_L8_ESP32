#include "bussiness_timerHard.h"

#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "devDriver_manage.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#define DEVVAL_DEFAULT_HWTIMER_DIVIDER         16  //  Hardware timer clock divider
#define DEVVAL_DEFAULT_HWTIMER_SCALE           (TIMER_BASE_CLK / DEVVAL_DEFAULT_HWTIMER_DIVIDER)  // convert counter value to seconds
#define DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC   (0.00001F)   // sample test interval for the first timer
#define DEVVAL_DEFAULT_HWTIMER_INTERVAL1_SEC   (1.00F)   // sample test interval for the second timer
#define DEVVAL_DEFAULT_TMWITHOUT_RELOAD   	 	0       // testing will be done without auto reload
#define DEVVAL_DEFAULT_TMWITH_RELOAD      	 	1       // testing will be done with auto reload

extern uint16_t devSysTimeKeep_counter;

static xQueueHandle queueHandle_hwTimer = NULL;
static xQueueHandle queueDriver_hwTimer = NULL;

static stt_timerLoop_ext devTimeLoopper_devGreenMode = {0};
static uint16_t devCounterParam_devDelayTrig = 0;
static uint8_t devDelayTrig_status = 0;

void usrAppParamSet_devGreenMode(uint8_t paramCst[2], bool nvsRecord_IF){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

	if(devTimeLoopper_devGreenMode.loopPeriod == (((uint16_t)paramCst[0] << 8) | ((uint16_t)paramCst[1] & 0x00FF)))return;

	devTimeLoopper_devGreenMode.loopCounter = 0;
	devTimeLoopper_devGreenMode.loopPeriod = ((uint16_t)paramCst[0] << 8) | ((uint16_t)paramCst[1] & 0x00FF);

	if(nvsRecord_IF)devSystemInfoLocalRecord_save(saveObj_infoTimer_greenMode, &(devTimeLoopper_devGreenMode.loopPeriod));

	if(devTimeLoopper_devGreenMode.loopPeriod)
		currentDevRunningFlg_paramSet(devRunningFlg_temp | DEV_RUNNING_FLG_BIT_GREENMODE, nvsRecord_IF);
	else
		currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_GREENMODE), nvsRecord_IF);
}

void usrAppParamGet_devGreenMode(uint8_t paramCst[2]){

	paramCst[0] = (uint8_t)(devTimeLoopper_devGreenMode.loopPeriod >> 8);
	paramCst[1] = (uint8_t)(devTimeLoopper_devGreenMode.loopPeriod & 0x00FF);
}

void usrApp_GreenMode_trig(void){

	devTimeLoopper_devGreenMode.loopCounter = 0;
}

void usrAppParamSet_devDelayTrig(uint8_t paramCst[3]){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

	devCounterParam_devDelayTrig = ((uint16_t)paramCst[0] << 8) | (uint16_t)paramCst[1]; //延时时间
	devDelayTrig_status = paramCst[2]; //延时响应值

	if(devCounterParam_devDelayTrig)
		currentDevRunningFlg_paramSet(devRunningFlg_temp | DEV_RUNNING_FLG_BIT_DELAY, true);
	else
		currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), true);
}

void usrAppParamGet_devDelayTrig(uint8_t paramCst[3]){

	paramCst[0] = (uint8_t)(devCounterParam_devDelayTrig >> 8);
	paramCst[1] = (uint8_t)(devCounterParam_devDelayTrig & 0x00FF);
	paramCst[2] = devDelayTrig_status;
}

static void IRAM_ATTR bussinessHwTimer_loop_1s(void){ //内部存储操作无法在中断内进行，所以需要通过消息队列向外传递存储信息

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();
	stt_devDataPonitTypedef devStatusValSet_temp = {0};
	stt_msgFromHwTimer sptr_msgQ_hwTimer = {0};

	//系统离线本地时间保持业务
	devSysTimeKeep_counter ++;

	//延时触发业务计时
	if(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_DELAY){

		if(devCounterParam_devDelayTrig)devCounterParam_devDelayTrig --;
		else{

//			currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), true);
//			memcpy(&devStatusValSet_temp, (stt_devDataPonitTypedef *)&devDelayTrig_status, sizeof(stt_devDataPonitTypedef));
//			currentDev_dataPointSet(&devStatusValSet_temp, true, true, true);

			memcpy(&devStatusValSet_temp, (stt_devDataPonitTypedef *)&devDelayTrig_status, sizeof(stt_devDataPonitTypedef));

			sptr_msgQ_hwTimer.msgFromHwTimerType = msgType_DevDelayTrig;
			sptr_msgQ_hwTimer.msgData.msgDataAbout_DevDelayTrig.nvsRecord_IF = 1;
			sptr_msgQ_hwTimer.msgData.msgDataAbout_DevDelayTrig.mutualCtrlTrig_IF = 1;
			sptr_msgQ_hwTimer.msgData.msgDataAbout_DevDelayTrig.devRunningFlgReales_IF = 1;
			memcpy(&(sptr_msgQ_hwTimer.msgData.msgDataAbout_DevDelayTrig.devStatusValSet), 
				   &devStatusValSet_temp,
				   sizeof(stt_devDataPonitTypedef));
			
			xQueueSendFromISR(queueHandle_hwTimer, &sptr_msgQ_hwTimer, NULL);
		}
	}

	//绿色模式计时业务
	if(devRunningFlg_temp & DEV_RUNNING_FLG_BIT_GREENMODE){

		if(devTimeLoopper_devGreenMode.loopCounter < devTimeLoopper_devGreenMode.loopPeriod)devTimeLoopper_devGreenMode.loopCounter ++;
		else{

			stt_devDataPonitTypedef devStatus_current = {0};
			const stt_devDataPonitTypedef devStatus_empty = {0};

			currentDev_dataPointGet(&devStatus_current);

			if(memcmp(&devStatus_current, &devStatus_empty, sizeof(stt_devDataPonitTypedef))){ //非零触发执行

				memset(&devStatusValSet_temp, 0, sizeof(stt_devDataPonitTypedef));

				sptr_msgQ_hwTimer.msgFromHwTimerType = msgType_DevGreenMode;
				sptr_msgQ_hwTimer.msgData.msgDataAbout_DevGreenMode.nvsRecord_IF = 1;
				sptr_msgQ_hwTimer.msgData.msgDataAbout_DevGreenMode.mutualCtrlTrig_IF = 1;
				memcpy(&(sptr_msgQ_hwTimer.msgData.msgDataAbout_DevGreenMode.devStatusValSet), 
					   &devStatusValSet_temp,
					   sizeof(stt_devDataPonitTypedef));

				xQueueSendFromISR(queueHandle_hwTimer, &sptr_msgQ_hwTimer, NULL);
			}
		}
	}
}

static void IRAM_ATTR bussinessHwTimer_devDriverWork(void){

	stt_msgDriverHwTimer sptr_msgQ_hwTimer = {0};
	static uint32_t loopCounter_debug = 0;
	static uint32_t loopCounter_dimmerFdebug = 0;
	const uint32_t loopPeriod_dimmerFdebug = (uint32_t)(1.0F / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_elecFdetect = 0;
	const uint32_t loopPeriod_elecFdetect = (uint32_t)(DEVDRIVER_ELECMEASURE_FDETECT_PERIOD / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_tempTdetect = 0;
	const uint32_t loopPeriod_tempTdetect = (uint32_t)(DEVDRIVER_TEMPERATUREDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_infraActDetect = 0;
	const uint32_t loopPeriod_infraActDetect = (uint32_t)(DEVDRIVER_INFRAACTDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static stt_timerLoop loopCounter_dimmerParamAdj = {

		.loopPeriod = 200,
		.loopCounter = 0,
	};
	static stt_timerLoop loopCounter_curtainRunningDetect = {
	
		.loopPeriod = (const uint16_t)(DEVCURTAIN_RUNNINGDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC),
		.loopCounter = 0,
	};
	static stt_timerLoop loopCounter_heaterRunningDetect = {

		.loopPeriod = (const uint16_t)(DEVHEATER_RUNNINGDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC),
		.loopCounter = 0,
	};
	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(loopCounter_debug < (uint32_t)(1.0F / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC))loopCounter_debug ++;
	else{

		loopCounter_debug = 0;

		sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_debug;
		sptr_msgQ_hwTimer.driverDats._debug_dats.dataDebug = 0;
		xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
	}

	switch(swCurrentDevType){

		case devTypeDef_dimmer:{

			devDriverBussiness_dimmerSwitch_decCounterReales();

			if(loopCounter_dimmerParamAdj.loopCounter < loopCounter_dimmerParamAdj.loopPeriod)loopCounter_dimmerParamAdj.loopCounter ++;
			else{

				loopCounter_dimmerParamAdj.loopCounter = 0;

				devDriverBussiness_dimmerSwitch_brightnessAdjDetectLoop();
			}

			if(loopCounter_dimmerFdebug < loopPeriod_dimmerFdebug)loopCounter_dimmerFdebug ++;
			else
			{			
				loopCounter_dimmerFdebug = 0;

				stt_Dimmer_attrFreq devDimmerParam_temp = {0};

				devDriverBussiness_dimmerSwitch_debug();
				devDriverBussiness_dimmerSwitch_currentParamGet(&devDimmerParam_temp);
				sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_dimer;
				sptr_msgQ_hwTimer.driverDats._dimmer_dats.freqSource = devDimmerParam_temp.periodSoureFreq_Debug;
				sptr_msgQ_hwTimer.driverDats._dimmer_dats.freqLoad = devDimmerParam_temp.periodBeat_cfm;
				xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
			}
			
		}break;

		case devTypeDef_curtain:{

			if(loopCounter_curtainRunningDetect.loopCounter < loopCounter_curtainRunningDetect.loopPeriod)loopCounter_curtainRunningDetect.loopCounter ++;
			else{

				loopCounter_curtainRunningDetect.loopCounter = 0;

				if(devDriverBussiness_curtainSwitch_devRunningDetect() == true){

					sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_curtain;
					sptr_msgQ_hwTimer.driverDats._curtain_dats.curtainOrbitalTimeSave = 1;
					xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
				}
			}
		
		}break;

		case devTypeDef_heater:{

			if(loopCounter_heaterRunningDetect.loopCounter < loopCounter_heaterRunningDetect.loopPeriod)loopCounter_heaterRunningDetect.loopCounter ++;
			else{

				loopCounter_heaterRunningDetect.loopCounter = 0;

				devDriverBussiness_heaterSwitch_runningDetectLoop();
			}
		
		}break;

		default:break;
	}

	if(loopCounter_elecFdetect < loopPeriod_elecFdetect)loopCounter_elecFdetect ++;
	else{

		loopCounter_elecFdetect = 0;

		sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_elecMeasure;
		sptr_msgQ_hwTimer.driverDats._elecMeasure_dats.measurePeriod_notice = 1;
		xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
	}

	if(loopCounter_tempTdetect < loopPeriod_tempTdetect)loopCounter_tempTdetect ++;
	else{

		loopCounter_tempTdetect = 0;
		
		sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_temprature;
		sptr_msgQ_hwTimer.driverDats._elecMeasure_dats.measurePeriod_notice = 1;
		xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
	}

	if(loopCounter_infraActDetect < loopPeriod_infraActDetect)loopCounter_infraActDetect ++;
	else{

		loopCounter_infraActDetect = 0;

		sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_infraActDetect;
		sptr_msgQ_hwTimer.driverDats._infraActDetect_dats.detectPeriod_notice = 1;
		xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
	}
}


/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR devHardTimer_group0_isr(void *para)
{
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[timer_idx].update = 1;
//    uint64_t timer_counter_value = ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32 | TIMERG0.hw_timer[timer_idx].cnt_low;

    /* Clear the interrupt and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {

        TIMERG0.int_clr_timers.t0 = 1;
//        timer_counter_value += (uint64_t) (DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC * DEVVAL_DEFAULT_HWTIMER_SCALE);
//        TIMERG0.hw_timer[timer_idx].alarm_high = (uint32_t) (timer_counter_value >> 32);
//        TIMERG0.hw_timer[timer_idx].alarm_low = (uint32_t) timer_counter_value;

		bussinessHwTimer_devDriverWork();
		
    } else if ((intr_status & BIT(timer_idx)) && timer_idx == TIMER_1) {
		
        TIMERG0.int_clr_timers.t1 = 1;

		bussinessHwTimer_loop_1s();
		
    } else {
		
        
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
static void devHardTimer_init(int timer_idx, 
    							   bool auto_reload, 
    							   double timer_interval_sec){
    							   
    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = DEVVAL_DEFAULT_HWTIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, 
    					  timer_idx, 
    					  timer_interval_sec * DEVVAL_DEFAULT_HWTIMER_SCALE);
	
    timer_enable_intr(TIMER_GROUP_0, 
					  timer_idx);
	
    timer_isr_register(TIMER_GROUP_0, 
					   timer_idx, 
					   devHardTimer_group0_isr, 
        			   (void *)timer_idx, 
        			   ESP_INTR_FLAG_LEVEL2, //Level
        			   NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

static void taskFunction_dataHandleFromHwTimer(void *arg){

	stt_msgFromHwTimer rptr_msgQ_fromHwTimer = {0};
	stt_msgDriverHwTimer rptr_msgQ_driverHwTimer = {0};

	for(;;){

		if(xQueueReceive(queueHandle_hwTimer, &rptr_msgQ_fromHwTimer, 10 / portTICK_RATE_MS) == pdTRUE){

			switch(rptr_msgQ_fromHwTimer.msgFromHwTimerType){

				case msgType_DevDelayTrig:{

					currentDev_dataPointSet(&(rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.devStatusValSet),
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.nvsRecord_IF,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.mutualCtrlTrig_IF,
											true);

					if(rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.devRunningFlgReales_IF){

						uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

						currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), true);
					}

				}break;

				case msgType_DevGreenMode:{

					currentDev_dataPointSet(&(rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.devStatusValSet),
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.nvsRecord_IF,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.mutualCtrlTrig_IF, 
											true);

				}break;

				default:break;
			}

			memset(&rptr_msgQ_fromHwTimer, 0, sizeof(stt_msgFromHwTimer));
		}

		if(xQueueReceive(queueDriver_hwTimer, &rptr_msgQ_driverHwTimer, 10 / portTICK_RATE_MS) == pdTRUE){

			switch(rptr_msgQ_driverHwTimer.driverDataType_discrib){

				case _driverDataType_debug:{ //秒周期测试

					devTypeDef_enum swCurrentDevType = currentDev_typeGet();

					switch(swCurrentDevType){

						case devTypeDef_curtain:{

							stt_devCurtain_runningParam devCurtainRunningParam = {0};

							devCurtain_runningParamGet(&devCurtainRunningParam);
							printf("orbital period:%d, orbital counter:%d.\n", (int)devCurtainRunningParam.act_period,
																			   (int)devCurtainRunningParam.act_counter);
							
						}break;

						default:break;
					}

//					printf("hw test devF:0.0001 done.\n");

				}break;

				case _driverDataType_dimer:{

					printf("freqSource: %dbp, freqLoad: %dbp.\n", rptr_msgQ_driverHwTimer.driverDats._dimmer_dats.freqSource,
																  rptr_msgQ_driverHwTimer.driverDats._dimmer_dats.freqLoad);

				}break;

				case _driverDataType_curtain:{

					if(rptr_msgQ_driverHwTimer.driverDats._curtain_dats.curtainOrbitalTimeSave){

						devCurtain_ocrbitalTimeSaveInitiative();
					}

				}break;

				case _driverDataType_elecMeasure:{

					float powerDetect_temp = devDriverBussiness_elecMeasure_powerCaculateReales();
					static float powerDetect_record = 0.0F;

					if(powerDetect_record != powerDetect_temp){

						powerDetect_record = powerDetect_temp;
						printf("devPower detectVal:%.3fW.\n", powerDetect_temp);
					}

				}break;

				case _driverDataType_temprature:{

					float temperature_temp = devDriverBussiness_temperatureMeasure_temperatureReales();
					float temperature_record = 0.0F;

					if(temperature_record != temperature_temp){

						temperature_record = temperature_temp;
						printf("devTemperature detectVal:%.02f.\n", temperature_temp);
					}
				
				}break;

				case _driverDataType_infraActDetect:{

					bool val_infraActDetect = devDriverBussiness_infraActDetect_detectReales();
					float freqDectect_temp = devDriverBussiness_infraActDetect_freqRcvGet();
					static float freqDectect_record = 0.0F;

					if(freqDectect_record != freqDectect_temp){

						freqDectect_record = freqDectect_temp;

//						printf("infraAct freqDetect:%.2f.\n", freqDectect_record);
					}

//					if(val_infraActDetect){

//						printf("infra act trig.\n");
//					}
		
				}break;

				default:break;
			}

			memset(&rptr_msgQ_driverHwTimer, 0, sizeof(stt_msgDriverHwTimer));
		}
	}

	vTaskDelete(NULL);
}

void usrApp_bussinessHardTimer_Init(void){

	queueHandle_hwTimer = xQueueCreate(6, sizeof(stt_msgFromHwTimer));
	queueDriver_hwTimer = xQueueCreate(6, sizeof(stt_msgDriverHwTimer));

	devHardTimer_init(TIMER_1, 
					  DEVVAL_DEFAULT_TMWITH_RELOAD, 
					  DEVVAL_DEFAULT_HWTIMER_INTERVAL1_SEC);

	devHardTimer_init(TIMER_0, 
					  DEVVAL_DEFAULT_TMWITH_RELOAD, 
					  DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);

	xTaskCreate(taskFunction_dataHandleFromHwTimer, 
				"Hwtimer_evt_task", 
				2048, 
				NULL, 
				CONFIG_MDF_TASK_DEFAULT_PRIOTY, 
				NULL);
}



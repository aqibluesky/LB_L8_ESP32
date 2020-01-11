#include "bussiness_timerHard.h"

#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "bussiness_timerSoft.h"

#include "devDriver_manage.h"

#include "tips_bussinessAcoustoOptic.h"

#define DEVVAL_DEFAULT_HWTIMER_DIVIDER         16  //  Hardware timer clock divider
#define DEVVAL_DEFAULT_HWTIMER_SCALE           (TIMER_BASE_CLK / DEVVAL_DEFAULT_HWTIMER_DIVIDER)  // convert counter value to seconds
#define DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC   (0.00001F)   // sample test interval for the first timer
#define DEVVAL_DEFAULT_HWTIMER_INTERVAL1_SEC   (1.00F)   // sample test interval for the second timer
#define DEVVAL_DEFAULT_TMWITHOUT_RELOAD   	 	0       // testing will be done without auto reload
#define DEVVAL_DEFAULT_TMWITH_RELOAD      	 	1       // testing will be done with auto reload

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)

	extern volatile stt_relayMagTestParam paramMagRelayTest;
 #endif
#endif

extern uint16_t devSysTimeKeep_counter;

extern EventGroupHandle_t xEventGp_tipsLoopTimer;

static xQueueHandle queueHandle_hwTimer = NULL;
static xQueueHandle queueDriver_hwTimer = NULL;

static stt_timerLoop_ext devTimeLoopper_devGreenMode = {0};
static uint16_t devCounterParam_devDelayTrig = 0;
static uint8_t devDelayTrig_status = 0;

void usrAppParamClrReset_devGreenMode(void){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

	devTimeLoopper_devGreenMode.loopPeriod = 0;
	devSystemInfoLocalRecord_save(saveObj_infoTimer_greenMode, &(devTimeLoopper_devGreenMode.loopPeriod));
	currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_GREENMODE), true);
}

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

void usrAppParamClrReset_devDelayTrig(void){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

	devCounterParam_devDelayTrig = 0;
	devDelayTrig_status = 0;
	currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), false);
}

void usrAppParamSet_devDelayTrig(uint8_t paramCst[3]){

	uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

	devCounterParam_devDelayTrig = ((uint16_t)paramCst[0] << 8) | (uint16_t)paramCst[1]; //延时时间
	devDelayTrig_status = paramCst[2]; //延时响应值

	if(devCounterParam_devDelayTrig)
		currentDevRunningFlg_paramSet(devRunningFlg_temp | DEV_RUNNING_FLG_BIT_DELAY, false);
	else
		currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), false);
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
//			currentDev_dataPointSet(&devStatusValSet_temp, true, true, true, false);

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
			stt_devDataPonitTypedef devStatus_empty = {0};

			currentDev_dataPointGet(&devStatus_current);
			switch(currentDev_typeGet()){
				
				case devTypeDef_curtain:
				case devTypeDef_moudleSwCurtain:{ //窗帘静止时指定值

					stt_devCurtain_runningParam curtainRunningParam = {0};

					devCurtain_runningParamGet(&curtainRunningParam);
					if(!curtainRunningParam.act_counter){

						devStatus_empty.devType_curtain.devCurtain_actEnumVal = curtainRunningStatus_cTact_stop; //关
					}
					
				}break;
			
				default:break;
			}

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
	const uint32_t loopPeriod_devBeepsRunning = (uint32_t)(DEVDRIVER_DEVBEEPS_REFRESH_PERIOD / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_devBeepsRunning = 0;
	const uint32_t loopPeriod_devExTipsLedRunning = (uint32_t)(DEVDRIVER_EX_LEDTIPS_REFRESH_PERIOD / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_devExTipsLedRunning = 0;
	const uint32_t loopPeriod_dimmerFdebug = (uint32_t)(1.0F / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC);
	static uint32_t loopCounter_elecParamProcess = 0;
	const uint32_t loopPeriod_elecParamProcess = (uint32_t)(DEVDRIVER_ELECMEASURE_PARAM_PROCESS_PERIOD / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC); //测量数据处理
	static uint32_t loopCounter_elecFdetect = 0;
	const uint32_t loopPeriod_elecFdetect = (uint32_t)(DEVDRIVER_ELECMEASURE_FDETECT_PERIOD / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC); //测量数据计算
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
	static stt_timerLoop loopCounter_thermostatRunningDetect = {
	
		.loopPeriod = (const uint16_t)(DEVTHERMOSTAT_RUNNINGDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC),
		.loopCounter = 0,
	};
	static stt_timerLoop loopCounter_infraredRunningDetect = {
	
		.loopPeriod = (const uint16_t)(DEVINFRARED_RUNNINGDETECT_PERIODLOOP_TIME / DEVVAL_DEFAULT_HWTIMER_INTERVAL0_SEC),
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

		case devTypeDef_curtain:
		case devTypeDef_moudleSwCurtain:{

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

		case devTypeDef_thermostat:
		case devTypeDef_thermostatExtension:{

			if(loopCounter_thermostatRunningDetect.loopCounter < loopCounter_thermostatRunningDetect.loopPeriod)loopCounter_thermostatRunningDetect.loopCounter ++;
			else{

				loopCounter_thermostatRunningDetect.loopCounter = 0;

				sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_thermostatDriving;
				sptr_msgQ_hwTimer.driverDats._thermostatDriver_dats.drivePeriod_notice = 1;
				xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
			}

		}break;

		case devTypeDef_infrared:{

			static stt_timerLoop devInfrared_localDebugTimer = {
			
				.loopPeriod = (const uint16_t)(1.0F / DEVINFRARED_RUNNINGDETECT_PERIODLOOP_TIME),
				.loopCounter = 0,
			};

			if(loopCounter_infraredRunningDetect.loopCounter < loopCounter_infraredRunningDetect.loopPeriod)loopCounter_infraredRunningDetect.loopCounter ++;
			else{

				uint8_t dataDebug_temp = 0;

				loopCounter_infraredRunningDetect.loopCounter = 0;

				dataDebug_temp = devDriverBussiness_infraredSwitch_runningDetectLoop();

				/*调试业务*/
				if(devInfrared_localDebugTimer.loopCounter < devInfrared_localDebugTimer.loopPeriod)devInfrared_localDebugTimer.loopCounter ++;
				else{

					devInfrared_localDebugTimer.loopCounter = 0;

					sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_infraredRunning;
					sptr_msgQ_hwTimer.driverDats._infraredDriver_dats.debugPeriod_notice = 1;
					sptr_msgQ_hwTimer.driverDats._infraredDriver_dats.debugData = dataDebug_temp;
					xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
				}
			}
		}

		default:break;
	}

	if(loopCounter_devBeepsRunning < loopPeriod_devBeepsRunning)loopCounter_devBeepsRunning ++;
	else{

		uint8_t statusCode = 0;
		static uint8_t statusCodeRecord = 0;

		loopCounter_devBeepsRunning = 0;

		statusCode = devAcoustoOptic_beepBussinessRefresh();

		if(statusCodeRecord != statusCode){

			statusCodeRecord = statusCode;
			
			sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_beepsParamDebug;
			sptr_msgQ_hwTimer.driverDats._beepsDriver_dats.dataStatus = statusCode;
			xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
		}
	}

	if(loopCounter_devExTipsLedRunning < loopPeriod_devExTipsLedRunning)loopCounter_devExTipsLedRunning ++;
	else{

		loopCounter_devExTipsLedRunning = 0;

		devTipsByLed_driverReales();
	}

	if(loopCounter_elecParamProcess < loopPeriod_elecParamProcess)loopCounter_elecParamProcess ++;
	else{

		loopCounter_elecParamProcess = 0;

		sptr_msgQ_hwTimer.driverDataType_discrib = _driverDataType_elecMeasure;
		sptr_msgQ_hwTimer.driverDats._elecMeasure_dats.processPeriod_notice = 1;
		xQueueSendFromISR(queueDriver_hwTimer, &sptr_msgQ_hwTimer, NULL);
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

					stt_devDataPonitTypedef devStatusValSet_temp = {0};

					memcpy(&devStatusValSet_temp, &rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.devStatusValSet, sizeof(stt_devDataPonitTypedef));
					
					switch(currentDev_typeGet()){
						
						case devTypeDef_curtain:
						case devTypeDef_moudleSwCurtain:{
					
							devStatusValSet_temp.devType_curtain.devCurtain_actMethod = 1; //位置定时，用滑条方式
						
						}break;

						case devTypeDef_thermostat:{

							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};

							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变
							devStatusValSet_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							devStatusValSet_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;

						}break;

						case devTypeDef_thermostatExtension:{

							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};
							uint8_t devThermostatExSwStatus_temp = 0;

							memcpy(&devThermostatExSwStatus_temp, &devStatusValSet_temp, sizeof(uint8_t));
							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变

							/*bit0 -恒温器是否开启*/
							(devThermostatExSwStatus_temp & (1 >> 0))?
								(devStatusValSet_temp.devType_thermostat.devThermostat_running_en = 1):
								(devStatusValSet_temp.devType_thermostat.devThermostat_running_en = 0);
							devStatusValSet_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							devStatusValSet_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;

							/*bit2 -第二位开关值
							  bit1 -第一位开关值*/
							devThermostatExSwStatus_temp >>= 1;
							devThermostatExSwStatus_temp &= 0x03;
							devDriverBussiness_thermostatSwitch_exSwitchParamSet(devThermostatExSwStatus_temp);

						}break;
					
						default:break;
					}

					currentDev_dataPointSet(&devStatusValSet_temp,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.nvsRecord_IF,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.mutualCtrlTrig_IF,
											true,
											true);

					if(rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevDelayTrig.devRunningFlgReales_IF){

						uint16_t devRunningFlg_temp = currentDevRunningFlg_paramGet();

						currentDevRunningFlg_paramSet(devRunningFlg_temp & (~DEV_RUNNING_FLG_BIT_DELAY), true);
					}

					xEventGroupSetBits(xEventGp_tipsLoopTimer, LOOPTIMEREVENT_FLG_BITHOLD_DELAYUP);

				}break;

				case msgType_DevGreenMode:{

					stt_devDataPonitTypedef devStatusValSet_temp = {0};

					memcpy(&devStatusValSet_temp, &rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.devStatusValSet, sizeof(stt_devDataPonitTypedef));

					switch(currentDev_typeGet()){
						
						case devTypeDef_curtain:
						case devTypeDef_moudleSwCurtain:{

							devStatusValSet_temp.devType_curtain.devCurtain_actEnumVal = curtainRunningStatus_cTact_close; //全关

						}break;

						case devTypeDef_thermostat:{

							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};

							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变
							devStatusValSet_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							devStatusValSet_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;

						}break;

						case devTypeDef_thermostatExtension:{

							stt_devDataPonitTypedef devDataPointCurrent_temp = {0};

							devDriverBussiness_thermostatSwitch_exSwitchParamSet(0); //额外两个开关也关闭

							currentDev_dataPointGet(&devDataPointCurrent_temp); //只改运行使能参数，其他参数不变
							devStatusValSet_temp.devType_thermostat.devThermostat_nightMode_en =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_nightMode_en;
							devStatusValSet_temp.devType_thermostat.devThermostat_tempratureTarget =\
							devDataPointCurrent_temp.devType_thermostat.devThermostat_tempratureTarget;

						}break;

						default:break;
					}

					currentDev_dataPointSet(&devStatusValSet_temp,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.nvsRecord_IF,
											(bool)rptr_msgQ_fromHwTimer.msgData.msgDataAbout_DevGreenMode.mutualCtrlTrig_IF, 
											true,
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

					//磁保持继电器测试
#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)
 #if(DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)

 					if(paramMagRelayTest.relayTest_EN)
						paramMagRelayTest.dataRcd.timeRecord ++;

					if(0 == (paramMagRelayTest.dataRcd.timeRecord % 10)) //定时存储
						debugTestMagRelay_paramSave();
 #endif
#endif

					switch(swCurrentDevType){

						case devTypeDef_curtain:
						case devTypeDef_moudleSwCurtain:{

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

//					printf("freqSource: %dbp, freqLoad: %dbp.\n", rptr_msgQ_driverHwTimer.driverDats._dimmer_dats.freqSource,
//																  rptr_msgQ_driverHwTimer.driverDats._dimmer_dats.freqLoad);

				}break;

				case _driverDataType_curtain:{

					if(rptr_msgQ_driverHwTimer.driverDats._curtain_dats.curtainOrbitalTimeSave){

						devCurtain_ocrbitalTimeSaveInitiative();
					}

				}break;

				case _driverDataType_elecMeasure:{

					if(rptr_msgQ_driverHwTimer.driverDats._elecMeasure_dats.measurePeriod_notice){

						float powerDetect_temp = devDriverBussiness_elecMeasure_powerCaculateReales();
						static float powerDetect_record = 0.0F;

						if(powerDetect_record != powerDetect_temp){

							powerDetect_record = powerDetect_temp;
//							printf("devPower detectVal:%.3fW.\n", powerDetect_temp);
						}
					}

					if(rptr_msgQ_driverHwTimer.driverDats._elecMeasure_dats.processPeriod_notice){

						devDriverBussiness_elecMeasure_paramProcess();				
					}

				}break;

				case _driverDataType_temprature:{

					float temperature_temp = devDriverBussiness_temperatureMeasure_temperatureReales();
					float temperature_record = 0.0F;

					if(temperature_record != temperature_temp){

						temperature_record = temperature_temp;
//						printf("devTemperature detectVal:%.02f.\n", temperature_temp);
					}
				
				}break;

				case _driverDataType_infraActDetect:{

//					bool val_infraActDetect = devDriverBussiness_infraActDetect_detectReales();
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

				case _driverDataType_thermostatDriving:{

					devDriverBussiness_thermostatSwitch_runningDetectLoop();

				}break;

				case _driverDataType_infraredRunning:{

					if(rptr_msgQ_driverHwTimer.driverDats._infraredDriver_dats.debugPeriod_notice){

//						printf("infrared debug data:%d.\n", rptr_msgQ_driverHwTimer.driverDats._infraredDriver_dats.debugData);
					}

				}break;

				case _driverDataType_beepsParamDebug:{

//					printf("beepsStage:%d.\n", rptr_msgQ_driverHwTimer.driverDats._beepsDriver_dats.dataStatus);

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



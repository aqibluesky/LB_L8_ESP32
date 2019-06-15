#include "devDriver_infraActDetect.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

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

//CHOOSE SELF TEST OR NORMAL TEST
#define DEVDRIVER_INFRAACTDETECT_RMT_RX_SELF_TEST   0

/******************************************************/
/*****                SELF TEST:                  *****/
/*Connect RMT_TX_GPIO_NUM with RMT_RX_GPIO_NUM        */
/*TX task will send NEC data with carrier disabled    */
/*RX task will print NEC data it receives.            */
/******************************************************/
#if DEVDRIVER_INFRAACTDETECT_RMT_RX_SELF_TEST
#define DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL  0   /*!< Data bit is active high for self test mode */
#define DEVDRIVER_INFRAACTDETECT_RMT_TX_CARRIER_EN    0   /*!< Disable carrier for self test mode  */
#else
//Test with infrared LED, we have to enable carrier for transmitter
//When testing via IR led, the receiver waveform is usually active-low.
#define DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL  0   /*!< If we connect with a IR receiver, the data is active low */
#define DEVDRIVER_INFRAACTDETECT_RMT_TX_CARRIER_EN    1   /*!< Enable carrier for IR transmitter test with IR led */
#endif

#define DEVDRIVER_INFRAACTDETECT_DATA_ADDR_DECTECT_REQ	0x0012
#define DEVDRIVER_INFRAACTDETECT_DATA_CMD_DECTECT_REQ	0x0034

#define DEVDRIVER_INFRAACTDETECT_RMT_TX_CHANNEL    		1     														/*!< RMT channel for transmitter */
#define DEVDRIVER_INFRAACTDETECT_RMT_TX_GPIO_NUM  		13     														/*!< GPIO number for transmitter signal */
#define DEVDRIVER_INFRAACTDETECT_RMT_CLK_DIV      		100    														/*!< RMT counter clock divider */
#define DEVDRIVER_INFRAACTDETECT_RMT_TICK_10_US    		(80000000/DEVDRIVER_INFRAACTDETECT_RMT_CLK_DIV/100000)  	/*!< RMT counter value for 10 us.(Source clock is APB clock) */

#define DEVDRIVER_INFRAACTDETECT_NEC_HEADER_HIGH_US   	9000                         								/*!< NEC protocol header: positive 9ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_HEADER_LOW_US     	4500                         								/*!< NEC protocol header: negative 4.5ms*/
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_HIGH_US   	560                          								/*!< NEC protocol data bit 1: positive 0.56ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_LOW_US    	(2250-DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_HIGH_US)   		/*!< NEC protocol data bit 1: negative 1.69ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_HIGH_US  	560                          								/*!< NEC protocol data bit 0: positive 0.56ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_LOW_US   	(1120-DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_HIGH_US)  		/*!< NEC protocol data bit 0: negative 0.56ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_END           	560                         								/*!< NEC protocol end: positive 0.56ms */
#define DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN        	320            			   									/*!< NEC parse margin time */
 
#define DEVDRIVER_INFRAACTDETECT_NEC_ITEM_DURATION(d)  	((d & 0x7fff)*10/DEVDRIVER_INFRAACTDETECT_RMT_TICK_10_US) 	/*!< Parse duration time from memory register value */
#define DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM   	3  															/*!< NEC code item number: header + 32bit data + end */
#define DEVDRIVER_INFRAACTDETECT_RMT_TX_DATA_NUM  		34   														/*!< NEC tx test data number */
#define DEVDRIVER_INFRAACTDETECT_rmt_item32_tIMEOUT_US  9500   														/*!< RMT receiver timeout value(us) */

#define DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT      	PCNT_UNIT_3
#define DEVDRIVER_INFRAACTDETECT_PCNT_H_LIM_VAL      	20000
#define DEVDRIVER_INFRAACTDETECT_PCNT_L_LIM_VAL     	-20000
#define DEVDRIVER_INFRAACTDETECT_PCNT_THRESH1_VAL    	5
#define DEVDRIVER_INFRAACTDETECT_PCNT_THRESH0_VAL   	-5
#define DEVDRIVER_INFRAACTDETECT_PCNT_INPUT_SIG_IO   	34  // Pulse Input GPIO
#define DEVDRIVER_INFRAACTDETECT_PCNT_INPUT_CTRL_IO  	36  // Control GPIO HIGH=count up, LOW=count down

EventGroupHandle_t xEventGp_infraActDetect = NULL;

static stt_infraActDetect_attrFreq devParam_infraActDetect = {0};
static pcnt_isr_handle_t user_isr_handle = NULL;

static const char* NEC_TAG = "infraActDet";

static int channel_necAppTx = 0L;
static uint16_t cmd_tx = 0;
static uint16_t addr_tx = 0;
static int nec_tx_num = 0;
static size_t size_tx = 0;
static rmt_item32_t* item_tx = NULL;
static int item_num_tx = 0;

/*
 * @brief Build register value of waveform for NEC one data bit
 */
static inline void nec_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
{
    item->level0 = 1;
    item->duration0 = (high_us) / 10 * DEVDRIVER_INFRAACTDETECT_RMT_TICK_10_US;
    item->level1 = 0;
    item->duration1 = (low_us) / 10 * DEVDRIVER_INFRAACTDETECT_RMT_TICK_10_US;
}

/*
 * @brief Generate NEC header value: active 9ms + negative 4.5ms
 */
static void nec_fill_item_header(rmt_item32_t* item)
{
    nec_fill_item_level(item, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_LOW_US);
}

/*
 * @brief Generate NEC data bit 1: positive 0.56ms + negative 1.69ms
 */
static void nec_fill_item_bit_one(rmt_item32_t* item)
{
    nec_fill_item_level(item, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_LOW_US);
}

/*
 * @brief Generate NEC data bit 0: positive 0.56ms + negative 0.56ms
 */
static void nec_fill_item_bit_zero(rmt_item32_t* item)
{
    nec_fill_item_level(item, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_LOW_US);
}

/*
 * @brief Generate NEC end signal: positive 0.56ms
 */
static void nec_fill_item_end(rmt_item32_t* item)
{
    nec_fill_item_level(item, DEVDRIVER_INFRAACTDETECT_NEC_BIT_END, 0x7fff);
}

/*
 * @brief Check whether duration is around target_us
 */
inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us)
{
    if(( DEVDRIVER_INFRAACTDETECT_NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us))
        && ( DEVDRIVER_INFRAACTDETECT_NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
        return true;
    } else {
        return false;
    }
}

/*
 * @brief Check whether this value represents an NEC header
 */
static bool nec_header_if(rmt_item32_t* item)
{

//	struct _stt_necHeaderCheck{

//		uint8_t activeLevel_res:1;
//		uint8_t necHeadRangeHi_res:1;
//		uint8_t necHeadRangeLo_res:1;

//	}data_necHeaderCheckRes = {0};

//	(item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)?
//		(data_necHeaderCheckRes.activeLevel_res = 1):
//		(data_necHeaderCheckRes.activeLevel_res = 0);
//	(nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necHeaderCheckRes.necHeadRangeHi_res = 1):
//		(data_necHeaderCheckRes.necHeadRangeHi_res = 0);
//	(nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necHeaderCheckRes.necHeadRangeLo_res = 1):
//		(data_necHeaderCheckRes.necHeadRangeLo_res = 0);

//	printf("nec head check res:    activeLevel_res[%d], necHeadRangeHi_res[%d], necHeadRangeLo_res[%d].\n", data_necHeaderCheckRes.activeLevel_res,
//																									  	 data_necHeaderCheckRes.necHeadRangeHi_res,
//																									  	 data_necHeaderCheckRes.necHeadRangeLo_res);
//	if(data_necHeaderCheckRes.activeLevel_res &&
//	   data_necHeaderCheckRes.necHeadRangeHi_res &&
//	   data_necHeaderCheckRes.necHeadRangeLo_res){

//		return true;
//	}																							  	 

    if((item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_HEADER_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)) {
        return true;
    }

    return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 1
 */
static bool nec_bit_one_if(rmt_item32_t* item)
{

//	struct _stt_necBit1Check{

//		uint8_t activeLevel_res:1;
//		uint8_t necBit1RangeHi_res:1;
//		uint8_t necBit1RangeLo_res:1;

//	}data_necBit1CheckRes = {0};

//	(item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)?
//		(data_necBit1CheckRes.activeLevel_res = 1):
//		(data_necBit1CheckRes.activeLevel_res = 0);
//	(nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necBit1CheckRes.necBit1RangeHi_res = 1):
//		(data_necBit1CheckRes.necBit1RangeHi_res = 0);
//	(nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necBit1CheckRes.necBit1RangeLo_res = 1):
//		(data_necBit1CheckRes.necBit1RangeLo_res = 0);

//	printf("nec bitOne check res:  activeLevel_res[%d], necBit1RangeHi_res[%d], necBit1RangeLo_res[%d].\n", data_necBit1CheckRes.activeLevel_res,
//																									  	   data_necBit1CheckRes.necBit1RangeHi_res,
//																									  	   data_necBit1CheckRes.necBit1RangeLo_res);
//	if(data_necBit1CheckRes.activeLevel_res &&
//	   data_necBit1CheckRes.necBit1RangeHi_res &&
//	   data_necBit1CheckRes.necBit1RangeLo_res){

//		return true;
//	}	

    if((item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ONE_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)) {
        return true;
    }
		
    return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 0
 */
static bool nec_bit_zero_if(rmt_item32_t* item)
{
//	struct _stt_necBit0Check{

//		uint8_t activeLevel_res:1;
//		uint8_t necBit0RangeHi_res:1;
//		uint8_t necBit0RangeLo_res:1;

//	}data_necBit0CheckRes = {0};

//	(item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)?
//		(data_necBit0CheckRes.activeLevel_res = 1):
//		(data_necBit0CheckRes.activeLevel_res = 0);
//	(nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necBit0CheckRes.necBit0RangeHi_res = 1):
//		(data_necBit0CheckRes.necBit0RangeHi_res = 0);
//	(nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN))?
//		(data_necBit0CheckRes.necBit0RangeLo_res = 1):
//		(data_necBit0CheckRes.necBit0RangeLo_res = 0);

//	printf("nec bitZero check res: activeLevel_res[%d], necBit0RangeHi_res[%d], necBit0RangeLo_res[%d].\n", data_necBit0CheckRes.activeLevel_res,
//																									  	 	data_necBit0CheckRes.necBit0RangeHi_res,
//																									  	 	data_necBit0CheckRes.necBit0RangeLo_res);
//	if(data_necBit0CheckRes.activeLevel_res &&
//	   data_necBit0CheckRes.necBit0RangeHi_res &&
//	   data_necBit0CheckRes.necBit0RangeLo_res){

//		return true;
//	}	

    if((item->level0 == DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL && item->level1 != DEVDRIVER_INFRAACTDETECT_RMT_RX_ACTIVE_LEVEL)
        && nec_check_in_range(item->duration0, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_HIGH_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)
        && nec_check_in_range(item->duration1, DEVDRIVER_INFRAACTDETECT_NEC_BIT_ZERO_LOW_US, DEVDRIVER_INFRAACTDETECT_NEC_BIT_MARGIN)) {
        return true;
    }
		
    return false;
}

/*
 * @brief Parse NEC 32 bit waveform to address and command.
 */
static int nec_parse_items(rmt_item32_t* item, int item_num, uint16_t* addr, uint16_t* data)
{
    int w_len = item_num;
    if(w_len < DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM) {
        return -1;
    }
    int i = 0, j = 0;
    if(!nec_header_if(item++)) {

//		printf("nec header err.\n");
	
        return -1;
    }
    uint16_t addr_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            addr_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            addr_t |= (0 << j);
        } else {

//			printf("nec addr bit_zero err.\n");
		
            return -1;
        }
        item++;
        i++;
    }
    uint16_t data_t = 0;
    for(j = 0; j < 16; j++) {
        if(nec_bit_one_if(item)) {
            data_t |= (1 << j);
        } else if(nec_bit_zero_if(item)) {
            data_t |= (0 << j);
        } else {

//			printf("nec data bit_zero err.\n");
		
            return -1;
        }
        item++;
        i++;
    }
    *addr = addr_t;
    *data = data_t;
    return i;
}

/*
 * @brief Build NEC 32bit waveform.
 */
static int nec_build_items(int channel, rmt_item32_t* item, int item_num, uint16_t addr, uint16_t cmd_data)
{
    int i = 0, j = 0;
    if(item_num < DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM) {
        return -1;
    }
    nec_fill_item_header(item++);
    i++;
    for(j = 0; j < 16; j++) {
        if(addr & 0x1) {
            nec_fill_item_bit_one(item);
        } else {
            nec_fill_item_bit_zero(item);
        }
        item++;
        i++;
        addr >>= 1;
    }
    for(j = 0; j < 16; j++) {
        if(cmd_data & 0x1) {
            nec_fill_item_bit_one(item);
        } else {
            nec_fill_item_bit_zero(item);
        }
        item++;
        i++;
        cmd_data >>= 1;
    }
    nec_fill_item_end(item);
    i++;
    return i;
}

static void IRAM_ATTR pcnt_isr_handler(void* arg){

	uint32_t intr_status = PCNT.int_st.val;
	int	loop = 0;

	for (loop = 0; loop < PCNT_UNIT_MAX; loop++){

		switch(intr_status & (BIT(loop))){

			case PCNT_STATUS_H_LIM:{

				
			}break;

			default:break;
		}

		if(intr_status & (BIT(loop)))
			PCNT.int_clr.val = BIT(loop);
	}
}

static void nec_tx_actionTrigLoop(void){

	static int opOffset = 0;
	int i = 0;

	i = nec_build_items(channel_necAppTx, item_tx + opOffset, item_num_tx - opOffset, ((~addr_tx) << 8) | addr_tx, ((~cmd_tx) << 8) |  cmd_tx);
	if(i < 0){
	
		//To send data according to the waveform items.
		rmt_write_items(channel_necAppTx, item_tx, item_num_tx, true);
		//Wait until sending is done.
		rmt_wait_tx_done(channel_necAppTx, portMAX_DELAY);
		//before we free the data, make sure sending is already done.
//		free(item_tx);
//		item_tx = NULL;

		opOffset = 0;

//		vTaskDelay(10 / portTICK_PERIOD_MS);

//        ESP_LOGI(NEC_TAG, "RMT TX DATA");
		size_tx = (sizeof(rmt_item32_t) * DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM * nec_tx_num);
//		item_tx = (rmt_item32_t*) malloc(size_tx);
		item_num_tx = DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM * nec_tx_num;
		memset((void*) item_tx, 0, size_tx);
	}
	else
	{
		addr_tx = DEVDRIVER_INFRAACTDETECT_DATA_ADDR_DECTECT_REQ;
		cmd_tx  = DEVDRIVER_INFRAACTDETECT_DATA_CMD_DECTECT_REQ;
	
		opOffset += i;
		
		return;
	}
}

static void devDriverBussiness_infraActDetect_signalGenerate(void){

	nec_tx_actionTrigLoop();
}

bool devDriverBussiness_infraActDetect_detectReales(void){

	const float freq_detectComfirm_max = 390.0F;
	const float freq_detectComfirm_min = 80.0F;	

	bool res = false;
	int16_t freq_infraActDetect = 0;

	pcnt_get_counter_value(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, &freq_infraActDetect);
	devParam_infraActDetect.infraActDetect_PulseCount = (float)freq_infraActDetect;
	devParam_infraActDetect.infraActDetect_FreqVal = devParam_infraActDetect.infraActDetect_PulseCount / 
													 DEVDRIVER_INFRAACTDETECT_PERIODLOOP_TIME;

	pcnt_counter_clear(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);

	devDriverBussiness_infraActDetect_signalGenerate();

	//施密特触发
	if(devParam_infraActDetect.infraActDetect_FreqVal > freq_detectComfirm_max){

		xEventGroupSetBits(xEventGp_infraActDetect, INFRAACTDETECTEVENT_FLG_BITHOLD_TRIGHAPPEN);
	}
	else
	if(devParam_infraActDetect.infraActDetect_FreqVal < freq_detectComfirm_min)
	{
		
	}
													 
	return res;
}

float devDriverBussiness_infraActDetect_freqRcvGet(void){

	return devParam_infraActDetect.infraActDetect_FreqVal;
}

/*
 * @brief RMT transmitter initialization
 */
static void nec_tx_init(void)
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = DEVDRIVER_INFRAACTDETECT_RMT_TX_CHANNEL;
    rmt_tx.gpio_num = DEVDRIVER_INFRAACTDETECT_RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = DEVDRIVER_INFRAACTDETECT_RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 80;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = DEVDRIVER_INFRAACTDETECT_RMT_TX_CARRIER_EN;
    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

void devDriverBussiness_infraActDetect_necTxInit(void){

	nec_tx_init();
	channel_necAppTx = DEVDRIVER_INFRAACTDETECT_RMT_TX_CHANNEL;
	nec_tx_num = DEVDRIVER_INFRAACTDETECT_RMT_TX_DATA_NUM;
	size_tx = (sizeof(rmt_item32_t) * DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM * nec_tx_num);
	item_tx = (rmt_item32_t*) malloc(size_tx);
	item_num_tx = DEVDRIVER_INFRAACTDETECT_NEC_DATA_ITEM_NUM * nec_tx_num;
	memset((void*) item_tx, 0, size_tx);
}

void devDriverBussiness_infraActDetect_pcntRxInit(void){

    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = DEVDRIVER_INFRAACTDETECT_PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = DEVDRIVER_INFRAACTDETECT_PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = DEVDRIVER_INFRAACTDETECT_PCNT_H_LIM_VAL,
        .counter_l_lim = DEVDRIVER_INFRAACTDETECT_PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, 300);
    pcnt_filter_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);

    /* Set threshold 0 and 1 values and enable events to watch */
//    pcnt_set_event_value(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, 
//    					 PCNT_EVT_THRES_1, 
//    					 DEVDRIVER_INFRAACTDETECT_PCNT_THRESH1_VAL);

//    pcnt_event_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_1);
	
//    pcnt_set_event_value(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, 
//						 PCNT_EVT_THRES_0, 
//						 DEVDRIVER_INFRAACTDETECT_PCNT_THRESH0_VAL);
//	
//    pcnt_event_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, 
//					  PCNT_EVT_THRES_0);
	
//    /* Enable events on zero, maximum and minimum limit values */
//    pcnt_event_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, PCNT_EVT_ZERO);
//    pcnt_event_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, PCNT_EVT_H_LIM);
//    pcnt_event_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);
    pcnt_counter_clear(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);

    /* Register ISR handler and enable interrupts for PCNT unit */
    pcnt_isr_register(pcnt_isr_handler, NULL, 0, &pcnt_isr_handler);
    pcnt_intr_enable(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(DEVDRIVER_INFRAACTDETECT_PCNT_TEST_UNIT);
}

//void devDriverBussiness_infraActDetect_gpioTxInit(void){

//	gpio_config_t io_conf = {0};

//	//disable interrupt
//	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//	//set as output mode
//	io_conf.mode = GPIO_MODE_OUTPUT;
//	//bit mask of the pins that you want to set
//	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_INFRAACTDETECT_CON_GPIONUM_SIG_TX);
//	//disable pull-down mode
//	io_conf.pull_down_en = 0;
//	//disable pull-up mode
//	io_conf.pull_up_en = 0;
//	//configure GPIO with the given settings
//	gpio_config(&io_conf);
//}

void devDriverBussiness_infraActDetect_periphInit(void){

	devDriverBussiness_infraActDetect_necTxInit();
	devDriverBussiness_infraActDetect_pcntRxInit();

	xEventGp_infraActDetect = xEventGroupCreate();
}


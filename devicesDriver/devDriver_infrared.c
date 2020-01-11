#include "devDriver_infrared.h"

#include "os.h"

#include "driver/i2c.h"

/* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"

#define SENSOR_DS18B20_DATA_PIN						(25)
#define SENSOR_DS18B20_PINCFG_PUTO()				gpio_set_direction(SENSOR_DS18B20_DATA_PIN, GPIO_MODE_OUTPUT)		
#define SENSOR_DS18B20_PINCFG_PUTI()				gpio_set_direction(SENSOR_DS18B20_DATA_PIN, GPIO_MODE_INPUT)
#define SENSOR_DS18B20_DATA_SET(x)					gpio_set_level(SENSOR_DS18B20_DATA_PIN, (uint32_t)x)
#define SENSOR_DS18B20_DATA_GET()					gpio_get_level(SENSOR_DS18B20_DATA_PIN)

#define DEVDRIVER_INFRARED_IIC_SDA					(14)		
#define DEVDRIVER_INFRARED_IIC_SCL					(27)
#define DEVDRIVER_INFRARED_GPIO_HXD019D_RESET		(5)
#define DEVDRIVER_INFRARED_GPIO_HXD019D_BUSY		(13)

#define HXD019D_PIN_RESET_LEVEL						0

#define HXD019D_PIN_SDA_PINCFG_PUTO()				gpio_set_direction(DEVDRIVER_INFRARED_IIC_SDA, GPIO_MODE_OUTPUT)
#define HXD019D_PIN_SDA_PINCFG_PUTI()				gpio_set_direction(DEVDRIVER_INFRARED_IIC_SDA, GPIO_MODE_INPUT)

#define HXD019D_PIN_SDA_GET()						gpio_get_level(DEVDRIVER_INFRARED_IIC_SDA)
#define HXD019D_PIN_BUSY_GET()						gpio_get_level(DEVDRIVER_INFRARED_GPIO_HXD019D_BUSY)

#define HXD019D_PIN_SDA_SET(x)						gpio_set_level(DEVDRIVER_INFRARED_IIC_SDA, (uint32_t)x)
#define HXD019D_PIN_SCL_SET(x)						gpio_set_level(DEVDRIVER_INFRARED_IIC_SCL, (uint32_t)x)
#define HXD019D_PIN_RESET_SET(x)					gpio_set_level(DEVDRIVER_INFRARED_GPIO_HXD019D_RESET, (uint32_t)x)

static const char *TAG = "lanbon_L8 - Infrared driver";

static TaskHandle_t tHandle_devInfraredProcess = NULL;

static uint8_t irData_temp[DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN] = {0};

static bool devDriver_moudleInitialize_Flg = false;

static enumInfrared_status theadStatus_infrared = infraredSMStatus_null;
static uint8_t infrared_opToutLoop = 0;

static uint8_t infrared_currentOpreatRes = 0; //
static uint8_t infrared_currentOpreatinsert = 0;

static uint8_t infrared_timerTrigIstNumTab[USRAPP_VALDEFINE_TRIGTIMER_NUM] = {0};

static volatile uint16_t infraredAct_timeCounter = 0;
static volatile uint16_t ds18b20_loopDetect_timeCounter = 0;

static esp_err_t devInfrared_gpio_init(void){

	esp_err_t ret = ERR_OK;
	gpio_config_t io_conf = {0};

	//bit mask of the pins that you want to set
	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_INFRARED_GPIO_HXD019D_RESET) |
						   (1ULL << DEVDRIVER_INFRARED_IIC_SCL) |
						   (1ULL << DEVDRIVER_INFRARED_IIC_SDA) |
						   (1ULL << SENSOR_DS18B20_DATA_PIN);
	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	ret = gpio_config(&io_conf);
	if(ret)return ret;

	io_conf.pin_bit_mask = (1ULL << DEVDRIVER_INFRARED_GPIO_HXD019D_BUSY);
	//set as output mode
	io_conf.mode = GPIO_MODE_INPUT;	
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 1;
	ret = gpio_config(&io_conf);

	return ret;
}

static uint8_t GetACKSign(void) {

	uint8_t ACKSign = 0;

	HXD019D_PIN_SDA_PINCFG_PUTI();
	ets_delay_us(135);

	HXD019D_PIN_SCL_SET(1);
	ets_delay_us(135);
	ACKSign = (uint8_t)HXD019D_PIN_SDA_GET();			
	ets_delay_us(135);
	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(135);

	return ACKSign;
}

static void SendACKSign(void){

	HXD019D_PIN_SDA_PINCFG_PUTO();
	ets_delay_us(30);
	HXD019D_PIN_SDA_SET(0);
	ets_delay_us(30);

	HXD019D_PIN_SCL_SET(1);		
	ets_delay_us(30);		
	HXD019D_PIN_SCL_SET(0);
}

static void I2COpen(void)
{	
	HXD019D_PIN_SDA_PINCFG_PUTO();

	HXD019D_PIN_SDA_SET(1);
	HXD019D_PIN_SCL_SET(1);
}

static void I2CClose(void)   
{
	HXD019D_PIN_SDA_PINCFG_PUTO();

	HXD019D_PIN_SDA_SET(1);
	HXD019D_PIN_SCL_SET(1);
}

static void I2CStart(void) 
{
	HXD019D_PIN_SDA_PINCFG_PUTO();
	HXD019D_PIN_SDA_SET(1);
	HXD019D_PIN_SCL_SET(1);
	ets_delay_us(65);		

	HXD019D_PIN_SDA_SET(0);
	ets_delay_us(320);		

	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(65);
}

static void I2CStop(void)
{
	HXD019D_PIN_SDA_PINCFG_PUTO();
	HXD019D_PIN_SDA_SET(0);
	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(30);

	HXD019D_PIN_SCL_SET(1);
	ets_delay_us(30);

	HXD019D_PIN_SDA_SET(1);
	ets_delay_us(30);
}

static void I2CWriteData(uint8_t bData)
{
	uint8_t loop = 0,
		    ACKSign = 0,
		    bitOpreat = 0;

	HXD019D_PIN_SDA_PINCFG_PUTO();
	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(30);
	
	for(loop = 0; loop <= 7; loop ++){

		ets_delay_us(30);
		
		bitOpreat = (bData >> (7 - loop)) & 0x01;

		if(bitOpreat)
		HXD019D_PIN_SDA_SET(1);
		else
		HXD019D_PIN_SDA_SET(0);

		ets_delay_us(30);
		HXD019D_PIN_SCL_SET(1);
		ets_delay_us(30);
		HXD019D_PIN_SCL_SET(0);
	}
	
	ACKSign = GetACKSign();
}

static void writeI2C(char *data2, uint8_t count)
{
	uint8_t i = 0;
	uint8_t j = 0;
	char iBuffer = 0;

	I2COpen();
	ets_delay_us(30);

	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(65);
	HXD019D_PIN_SCL_SET(1);
	
	vTaskDelay(20 / portTICK_PERIOD_MS);

	I2CStart();
	ets_delay_us(30);
	
	for(i = 0; i < count; i ++)	
	{
		iBuffer = data2[i];
		I2CWriteData(iBuffer);
		ets_delay_us(30);
	}

	ets_delay_us(30);

	I2CStop();
	ets_delay_us(30);

	I2CClose();
	ets_delay_us(30);
}

static void Learn_start(void)
{
		
	I2COpen();
	ets_delay_us(30);

	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(65);
	HXD019D_PIN_SCL_SET(1);
	
	vTaskDelay(20 / portTICK_PERIOD_MS);

	I2CStart();
	ets_delay_us(30);
	
	I2CWriteData(0x30);
	ets_delay_us(30);

	I2CWriteData(0x20);
	ets_delay_us(30);
	
	I2CWriteData(0x50);
	ets_delay_us(65);

	I2CStop();
	ets_delay_us(30);

	I2CClose();
	ets_delay_us(30);
}

static void I2CReadData(uint8_t* pbData)
{
	uint8_t readdata = 0;
	uint8_t i=8;

	HXD019D_PIN_SDA_PINCFG_PUTI();
	while (i--)
	{
		readdata <<= 1;

		HXD019D_PIN_SCL_SET(1);
		ets_delay_us(30);

		readdata |= HXD019D_PIN_SDA_GET();

		HXD019D_PIN_SCL_SET(0);
		ets_delay_us(65);	
	}
	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(30);

	*pbData = readdata;

	SendACKSign();
	ets_delay_us(65);
}

static void readI2C(char* readtempbuf) 
{
	uint8_t bValue;
	uint8_t i=0;
	uint8_t checksum;

	I2COpen();
	ets_delay_us(30);

	HXD019D_PIN_SCL_SET(0);
	ets_delay_us(65);
	HXD019D_PIN_SCL_SET(1);
	
	vTaskDelay(20 / portTICK_PERIOD_MS);

	I2CStart();
	ets_delay_us(30);

	//----------------------------------------
	//write
	I2CWriteData(0x30);
	ets_delay_us(30);
	//address point
	I2CWriteData(0x62);
	ets_delay_us(30);

	//---------------------------------------
	//read
	I2CStart();
	ets_delay_us(30);

	I2CWriteData(0x31);
	ets_delay_us(30);

	I2CReadData(&bValue);	
	ets_delay_us(30);	
	
	if(bValue != 0x00)
	{
		I2CStop();
		ets_delay_us(30);
		I2CClose();
		ets_delay_us(30);
	}

	i = 0;
	readtempbuf[i] = bValue;
	checksum = 0xc3;

	for(i = 1; i < 230; i++)
	{
		I2CReadData(&bValue);
		ets_delay_us(30);
		readtempbuf[i] = bValue;
		checksum += bValue;
	}
	
	I2CReadData(&bValue);		
	ets_delay_us(30);

	I2CStop();
	ets_delay_us(30);
	I2CClose();
	ets_delay_us(30);
}

static esp_err_t devInfrared_IICopreat_irControl(void){

	esp_err_t ret = ERR_OK;

	writeI2C((char *)irData_temp, 232);

	return ret;
}

static esp_err_t devInfrared_IICopreat_irLearn_stepA(void){

	esp_err_t ret = ERR_OK;

	Learn_start();

	return ret;
}

static esp_err_t devInfrared_IICopreat_irLearn_stepB(void){

	esp_err_t ret = ERR_OK;

	uint8_t check_Num = 0,
			 loopTemp = 0;
	
	readI2C((char *)irData_temp);
	
	for(loopTemp = 1; loopTemp < 230; loopTemp ++)check_Num += irData_temp[loopTemp];
	check_Num += 0x03;
	check_Num += 0x30;
	for(loopTemp = 229; loopTemp > 0; loopTemp --)irData_temp[loopTemp + 1] = irData_temp[loopTemp];
	irData_temp[0] = 0x30;
	irData_temp[1] = 0x03;
	irData_temp[231] = check_Num;
	
	nvsDataOpreation_devInfraredParam_set(irData_temp, infrared_currentOpreatinsert);
	theadStatus_infrared = infraredSMStatus_free; 
	
	infrared_currentOpreatRes = DEVINFRARED_OPREATRES_LEARNNING; 
	deviceDatapointSynchronousReport_actionTrig(); //状态同步通知

	return ret;
}

static void devInfrared_opreatAct_learnningStart(uint8_t opInsert){

	HXD019D_PIN_RESET_SET(!HXD019D_PIN_RESET_LEVEL); //

	if(theadStatus_infrared == infraredSMStatus_learnning){

		/*正在等待学习状态下，作用时间更新*/
		infraredAct_timeCounter = DEVINFRARED_HXD019D_opsLearnning_TOUT;
	}
	else
	{		
		devInfrared_IICopreat_irLearn_stepA();

		infraredAct_timeCounter = DEVINFRARED_HXD019D_opsLearnningSTBY_TOUT;
		infrared_opToutLoop = DEVINFRARED_HXD019D_opsLearnningSTBY_TOUTLOOP;

		theadStatus_infrared = infraredSMStatus_learnningSTBY; //注意顺序，顺序在最后最佳	
		infrared_currentOpreatinsert = opInsert;
	}
}

static void devInfrared_opreatAct_remoteControlStart(uint8_t opInsert){

	stt_infraredSwitchData_nvsOpreat *dataTemp = NULL;

	HXD019D_PIN_RESET_SET(!HXD019D_PIN_RESET_LEVEL);

	dataTemp = nvsDataOpreation_devInfraredParam_get(opInsert);	//控制数据缓存读取
	memcpy(irData_temp, dataTemp->ir_dataOpreat_buf, sizeof(uint8_t) * DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN);
	infrared_currentOpreatinsert = opInsert;

	theadStatus_infrared = infraredSMStatus_sigSendSTBY;

	os_free(dataTemp);
	dataTemp = NULL;
}

static bool sensorDs18b20_opreat_rst(void){

	bool ret = false;

	SENSOR_DS18B20_PINCFG_PUTO();
	SENSOR_DS18B20_DATA_SET(0);
	ets_delay_us(600);
	SENSOR_DS18B20_DATA_SET(1);
	ets_delay_us(60);

	SENSOR_DS18B20_PINCFG_PUTI();
	ret = SENSOR_DS18B20_DATA_GET();
	ets_delay_us(400);

	SENSOR_DS18B20_PINCFG_PUTO();
	SENSOR_DS18B20_DATA_SET(1);
	
	return ret ;
}

static void sensorDs18b20_opreat_writeBit(bool dBit){

	SENSOR_DS18B20_PINCFG_PUTO();
	SENSOR_DS18B20_DATA_SET(0);
	ets_delay_us(2);
	SENSOR_DS18B20_DATA_SET((uint32_t)dBit);
	ets_delay_us(60);
	SENSOR_DS18B20_DATA_SET(1);	
}

static void sensorDs18b20_opreat_writebyte(uint8_t dByte)
{
	uint8_t loop = 0;

	for(loop = 0; loop < 8; loop ++){

		sensorDs18b20_opreat_writeBit(dByte & 0x01);
		dByte >>= 1;
	}
}

static bool sensorDs18b20_opreat_readBit(void)
{
	bool dBit = false;

	SENSOR_DS18B20_PINCFG_PUTO();
	SENSOR_DS18B20_DATA_SET(0);
	ets_delay_us(2);
	SENSOR_DS18B20_DATA_SET(1);
	ets_delay_us(6);

	SENSOR_DS18B20_PINCFG_PUTI();
	dBit = SENSOR_DS18B20_DATA_GET();
	ets_delay_us(40);

	SENSOR_DS18B20_PINCFG_PUTO();
	SENSOR_DS18B20_DATA_SET(1);
	
	return dBit;
}

static uint8_t sensorDs18b20_opreat_readByte(void)
{
	uint8_t dByte = 0,
			dTemp = 0;
	uint8_t loop = 0;

	for(loop = 0; loop < 8; loop ++){

		dTemp = (uint8_t)sensorDs18b20_opreat_readBit();
		dByte = (dTemp << 7) | (dByte >> 1);
	}
	
	return dByte;
}

static void sensorDs18b20_opreat_sweap(void){

	sensorDs18b20_opreat_rst();
	ets_delay_us(2);
	sensorDs18b20_opreat_writebyte(0xcc);
	sensorDs18b20_opreat_writebyte(0x44);
}

static uint16_t sensorDs18b20_opreat_tempRead(void){

	uint16_t a = 0,
			 b = 0,
			 t = 0,
			 temp = 0;
	bool flag = false;
	float ftemp = 0.0F;
	
	sensorDs18b20_opreat_sweap();
	
	sensorDs18b20_opreat_rst();
	ets_delay_us(2);
	
	sensorDs18b20_opreat_writebyte(0xcc);
	sensorDs18b20_opreat_writebyte(0xbe);
	a = sensorDs18b20_opreat_readByte();
	b = sensorDs18b20_opreat_readByte();
	
	if(b & 0xfc){
		
		 temp = b;
		 temp = temp << 8;
		 temp |= a;
		 temp = ((~temp) + 1);
		 ftemp = temp * 0.0625F * 100.0F + 0.5F;
		 t = ftemp;
		 flag = true;
	}
	else
	{
	   	ftemp = ((b * 256) + a) * 0.0625F;
	    t = (uint16_t)(ftemp * 100.0F + 0.5F);
		flag = false;
	}

	ESP_LOGI(TAG, "ds18b20 temp:%.1f c\n", ftemp);
	
	return t;
}

static void devInfrared_opreatAct_stop(void){

	HXD019D_PIN_RESET_SET(HXD019D_PIN_RESET_LEVEL);
	
	memset(irData_temp, 0, sizeof(uint8_t) * DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN);
	infrared_currentOpreatinsert = 0;

	infraredAct_timeCounter = 0;
	infrared_opToutLoop = 0;

	infraredAct_timeCounter = DEVINFRARED_HXD019D_resetOpreatTimeKeep; //
	theadStatus_infrared = infraredSMStatus_opStop;
}

static void devDriverTask_infraredSwitch_process(void *arg){

	const uint16_t sensorDs18b20_detectPeriod = 10000;

	static enumInfrared_status localTheadStatus_infrared = infraredSMStatus_null;
	static struct{

		uint16_t toutTrig_flg:1;
		uint16_t toutTrigTherhold:15;
	}paramTimer_irLearnTout = {

		.toutTrig_flg 	  = 0,
		.toutTrigTherhold = 2000,
	};

	for(;;){

		if(ds18b20_loopDetect_timeCounter)ds18b20_loopDetect_timeCounter --;
		else{

			ds18b20_loopDetect_timeCounter = sensorDs18b20_detectPeriod;

			sensorDs18b20_opreat_tempRead();
		}

		if(localTheadStatus_infrared != theadStatus_infrared){

			localTheadStatus_infrared = theadStatus_infrared;
			ESP_LOGI(TAG, "infrared status chg:%d.\n", localTheadStatus_infrared);
		}

		switch(theadStatus_infrared){

			case infraredSMStatus_free:{

				paramTimer_irLearnTout.toutTrig_flg = 0;

			}break;
			
			case infraredSMStatus_learnningSTBY:{

//				ESP_LOGI(TAG, "[learnning sty]:infrared_opToutLoop:%d.\n", infrared_opToutLoop);

				infrared_currentOpreatRes = 0; //操作状态清空

				if(infrared_opToutLoop){

					if(0 == HXD019D_PIN_BUSY_GET()){
					
						theadStatus_infrared = infraredSMStatus_learnning; //
						infraredAct_timeCounter = DEVINFRARED_HXD019D_opsLearnning_TOUT; //
					}
					else
					{
						if(!infraredAct_timeCounter){ //
						
							devInfrared_IICopreat_irLearn_stepA();
							infraredAct_timeCounter = DEVINFRARED_HXD019D_opsLearnningSTBY_TOUT;
							infrared_opToutLoop --;
						}
					}
				}
				else
				{
					theadStatus_infrared = infraredSMStatus_free;
				}	
				
			}break;
			
			case infraredSMStatus_learnning:{

				if(infraredAct_timeCounter){

					if(HXD019D_PIN_BUSY_GET()){

						devBeepTips_trig(5, 8, 100, 50, 2);
						infraredAct_timeCounter = 0;
						infrared_opToutLoop = 0;

						devInfrared_IICopreat_irLearn_stepB();
					}
					else
					if(!paramTimer_irLearnTout.toutTrig_flg){ //学习等待超时beeps警告

						if(infraredAct_timeCounter < paramTimer_irLearnTout.toutTrigTherhold){

							paramTimer_irLearnTout.toutTrig_flg = 1;

							devBeepTips_trig(5, 8, paramTimer_irLearnTout.toutTrigTherhold, 0, 1);
						}
					}
				}
				else
				{
					devInfrared_opreatAct_stop();
					ESP_LOGI(TAG, "infrared learning time out.\n");
				}
				
			}break;
			
			case infraredSMStatus_sigSendSTBY:{

				if(!infraredAct_timeCounter)theadStatus_infrared = infraredSMStatus_sigSend;

			}break;
			
			case infraredSMStatus_sigSend:{

				devInfrared_IICopreat_irControl();

				theadStatus_infrared = infraredSMStatus_free;
				infrared_currentOpreatRes = DEVINFRARED_OPREATCMD_CONTROL;
				
			}break;
			
			case infraredSMStatus_opStop:{

				if(!infraredAct_timeCounter){
				
					theadStatus_infrared = infraredSMStatus_free;
					HXD019D_PIN_RESET_SET(!HXD019D_PIN_RESET_LEVEL);
				}

			}break;
			
			default:{

				theadStatus_infrared = infraredSMStatus_free;

			}break;
		}

		 vTaskDelay(pdMS_TO_TICKS(50));
	}

	vTaskDelete(NULL);
}

static void devDriverBussiness_infraredSwitch_periphInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_infrared)return;

	ESP_ERROR_CHECK(devInfrared_gpio_init());
}

static void devDriverBussiness_infraredSwitch_periphDeinit(void){
	
	gpio_reset_pin( DEVDRIVER_INFRARED_IIC_SDA);
	gpio_reset_pin( DEVDRIVER_INFRARED_IIC_SCL);
	gpio_reset_pin( DEVDRIVER_INFRARED_GPIO_HXD019D_RESET);
	gpio_reset_pin( DEVDRIVER_INFRARED_GPIO_HXD019D_BUSY);
}

void devDriverBussiness_infraredSwitch_timerUpTrigIstTabSet(uint8_t istTab[USRAPP_VALDEFINE_TRIGTIMER_NUM], bool nvsRecord_IF){

	memcpy(infrared_timerTrigIstNumTab, istTab, sizeof(uint8_t) * USRAPP_VALDEFINE_TRIGTIMER_NUM);
	if(nvsRecord_IF){

		devSystemInfoLocalRecord_save(saveObj_devInfrared_timerUpIstTab, infrared_timerTrigIstNumTab);
	}
}

void devDriverBussiness_infraredSwitch_timerUpTrigIstTabGet(uint8_t istTab[USRAPP_VALDEFINE_TRIGTIMER_NUM]){

	memcpy(istTab, infrared_timerTrigIstNumTab, sizeof(uint8_t) * USRAPP_VALDEFINE_TRIGTIMER_NUM);
}

uint8_t IRAM_ATTR devDriverBussiness_infraredSwitch_runningDetectLoop(void){

	if(infraredAct_timeCounter)infraredAct_timeCounter --;
	if(ds18b20_loopDetect_timeCounter)ds18b20_loopDetect_timeCounter --;

	return (infraredAct_timeCounter / 1000);
}

uint8_t devDriverBussiness_infraredSwitch_currentOpreatNumGet(void){

	ESP_LOGI(TAG, "infrared opreat res get:%d.\n", infrared_currentOpreatRes);

	return infrared_currentOpreatRes;
}

void devDriverBussiness_infraredSwitch_moudleInit(void){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType != devTypeDef_infrared)return;
	if(devDriver_moudleInitialize_Flg)return;

	esp_log_level_set(TAG, ESP_LOG_INFO);

	devDriverBussiness_infraredSwitch_periphInit();
	
	xTaskCreate(devDriverTask_infraredSwitch_process, 
				"devInfraredI2C_task", 
				1024 * 4, 
				NULL, 
				CONFIG_MDF_TASK_DEFAULT_PRIOTY + 3, 
				&tHandle_devInfraredProcess);

	devDriver_moudleInitialize_Flg = true;
}

void devDriverBussiness_infraredSwitch_moudleDeinit(void){

	if(!devDriver_moudleInitialize_Flg)return;

	devDriverBussiness_infraredSwitch_periphDeinit();
	vTaskDelete(tHandle_devInfraredProcess);

	devDriver_moudleInitialize_Flg = false;
}

enumInfrared_status devDriverBussiness_infraredStatus_get(void){

	return theadStatus_infrared;
}

void devDriverBussiness_infraredSwitch_periphStatusReales(stt_devDataPonitTypedef *param){

	devTypeDef_enum swCurrentDevType = currentDev_typeGet();

	if(swCurrentDevType == devTypeDef_infrared){

		ESP_LOGI(TAG, "infrared ir insert:%d.\n", param->devType_infrared.devInfrared_irIst);

		if(param->devType_infrared.devInfrared_actCmd){

			devInfrared_opreatAct_learnningStart(param->devType_infrared.devInfrared_irIst);
		}	
		else
		{
			devInfrared_opreatAct_remoteControlStart(param->devType_infrared.devInfrared_irIst);
		}
	}
}

























#include "devDriver_temperatureMeasure.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"

#include "mlink.h"
#include "mwifi.h"
#include "mdf_common.h"

#include "devDriver_manage.h"

#define DEVDRIVER_TEMPERATUREMEASURE_FILTER_COEFFIE		0.05F

#define DEVDRIVER_TEMPERATUREMEASURE_DEFAULT_VREF       1100
#define DEVDRIVER_TEMPERATUREMEASURE_NO_OF_SAMPLES		64

#define DEVDRIVER_TEMPERATUREMEASURE_NEGATIVE_BOUND		32767.0F	//数据转HEX传输时正负转换界限值

static float devParam_temprature = 0.0F;

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_3;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

const struct _stt_ntcCheckTab{ //NTC对照表

	int16_t tempratureVal;
	uint16_t adcDectectVal;
	
}ntcResTemperature_checkTab[DEVDRIVER_TEMPERATUREDETECT_ADC_CHECKTAB_LEN] = {

	{-30, 0x0048}, {-29, 0x004C}, {-28, 0x0051}, {-27, 0x0056}, {-26, 0x005C}, {-25, 0x0061}, {-24, 0x0067}, {-23, 0x006D}, {-22, 0x0074}, {-21, 0x007A}, {-20, 0x0081}, {-19, 0x0089}, 
	{-18, 0x0091}, {-17, 0x0099}, {-16, 0x00A1}, {-15, 0x00AA}, {-14, 0x00B4}, {-13, 0x00BE}, {-12, 0x00C8}, {-11, 0x00D2}, {-10, 0x00DE}, {-9,  0x00E9}, {-8,  0x00F5}, {-7,  0x0102}, 
	{-6,  0x010F}, {-5,  0x011D}, {-4,  0x012B}, {-3,  0x0139}, {-2,  0x0149}, {-1,  0x0158}, {0,   0x0169}, {1,   0x017A}, {2,   0x018B}, {3,   0x019E}, {4,   0x01B1}, {5,   0x01C4},
	{6,   0x01D8}, {7,   0x01ED}, {8,   0x0202}, {9,   0x0218}, {10,  0x022F}, {11,  0x0246}, {12,  0x025E}, {13,  0x0277}, {14,  0x0290}, {15,  0x02AA}, {16,  0x02C5}, {17,  0x02E0},  
	{18,  0x02FC}, {19,  0x0319}, {20,  0x0336}, {21,  0x0354}, {22,  0x0372}, {23,  0x0391}, {24,  0x03B1}, {25,  0x03D1}, {26,  0x03F1}, {27,  0x0413}, {28,  0x0434}, {29,  0x0457},  
	{30,  0x0479}, {31,  0x049C}, {32,  0x04C0}, {33,  0x04E4}, {34,  0x0508}, {35,  0x052D}, {36,  0x0552}, {37,  0x0577}, {38,  0x059D}, {39,  0x05C3}, {40,  0x05E9}, {41,  0x060F},  
	{42,  0x0636}, {43,  0x065C}, {44,  0x0683}, {45,  0x06AA}, {46,  0x06D0}, {47,  0x06F7}, {48,  0x071E}, {49,  0x0745}, {50,  0x076B}, {51,  0x0792}, {52,  0x07B8}, {53,  0x07DE},  
	{54,  0x0804}, {55,  0x082A}, {56,  0x0850}, {57,  0x0875},	{58,  0x089A}, {59,  0x08BF}, {60,  0x08E3}, {61,  0x0908}, {62,  0x092B}, {63,  0x094F}, {64,  0x0972}, {65,  0x0994},  
	{66,  0x09B7}, {67,  0x09D8}, {68,  0x09FA}, {69,  0x0A1B}, {70,  0x0A3B}, {71,  0x0A5B}, {72,  0x0A7A}, {73,  0x0A99}, {74,  0x0AB8}, {75,  0x0AD6}, {76,  0x0AF3}, {77,  0x0B10},  
	{78,  0x0B2D}, {79,  0x0B49}, {80,  0x0B64}, {81,  0x0B7F}, {82,  0x0B99}, {83,  0x0BB3}, {84,  0x0BCD}, {85,  0x0BE6}, {86,  0x0BFE}, {87,  0x0C16}, {88,  0x0C2D}, {89,  0x0C44},  
	{90,  0x0C5B}, {91,  0x0C70}, {92,  0x0C86}, {93,  0x0C9B}, {94,  0x0CAF}, {95,  0x0CC3}, {96,  0x0CD7}, {97,  0x0CEA}, {98,  0x0CFD}, {99,  0x0D0F}, {100, 0x0D21}, {101, 0x0D33}, 
	{102, 0x0D44}, {103, 0x0D54}, {104, 0x0D65}, {105, 0x0D74}, {106, 0x0D84}, {107, 0x0D93}, {108, 0x0DA2}, {109, 0x0DB0}, {110, 0x0DBE}, {111, 0x0DCC}, {112, 0x0DD9}, {113, 0x0DE6}, 
	{114, 0x0DF3}, {115, 0x0DFF}, {116, 0x0E0B}, {117, 0x0E17}, {118, 0x0E22}, {119, 0x0E2E}, {120, 0x0E38}, {121, 0x0E43}, {122, 0x0E4E}, {123, 0x0E58}, {124, 0x0E61}, {125, 0x0E6B}, 
	{126, 0x0E74}, {127, 0x0E7D}, {128, 0x0E86}, {129, 0x0E8F}, {130, 0x0E97}, {131, 0x0EA0}, {132, 0x0EA8}, {133, 0x0EAF}, {134, 0x0EB7}, {135, 0x0EBF}, {136, 0x0EC6}, {137, 0x0ECD}, 
	{138, 0x0ED4}, {139, 0x0EDA}, {140, 0x0EE1}, {141, 0x0EE7}, {142, 0x0EED}, {143, 0x0EF4}, {144, 0x0EF9}, {145, 0x0EFF}, {146, 0x0F05}, {147, 0x0F0A}, {148, 0x0F10}, {149, 0x0F15}, 
	{150, 0x0F1A}, {151, 0x0F1F}, {152, 0x0F24}, {153, 0x0F28}, {154, 0x0F2D}, {155, 0x0F31}, {156, 0x0F36}, {157, 0x0F3A}, {158, 0x0F3E}, {159, 0x0F42}, {160, 0x0F46}, {161, 0x0F4A}, 
	{162, 0x0F4E}, {163, 0x0F51}, {164, 0x0F55}, {165, 0x0F58}, {166, 0x0F5C}, {167, 0x0F5F}, {168, 0x0F63}, {169, 0x0F66}, {170, 0x0F69}, {171, 0x0F6C}, {172, 0x0F6F}, {173, 0x0F72}, 
	{174, 0x0F74}, {175, 0x0F77}, {176, 0x0F7A}, {177, 0x0F7C}, {178, 0x0F7F}, {179, 0x0F81}, {180, 0x0F84}, {181, 0x0F86}, {182, 0x0F89}, {183, 0x0F8B}, {184, 0x0F8D}, {185, 0x0F8F}, 
	{186, 0x0F91}, {187, 0x0F93}, {188, 0x0F95}, {189, 0x0F97}, {190, 0x0F99}, {191, 0x0F9B}, {192, 0x0F9D}, {193, 0x0F9F}, {194, 0x0FA1}, {195, 0x0FA2}, {196, 0x0FA4}, {197, 0x0FA6}, 
	{198, 0x0FA7}, {199, 0x0FA9}, {200, 0x0FAA}, {201, 0x0FAC}, {202, 0x0FAD}, {203, 0x0FAF}, {204, 0x0FB0}, {205, 0x0FB2}, {206, 0x0FB3}, {207, 0x0FB4}, {208, 0x0FB6}, {209, 0x0FB7}, 
	{210, 0x0FB8}, {211, 0x0FB9}, {212, 0x0FBA}, {213, 0x0FBC}, {214, 0x0FBD}, {215, 0x0FBE}, {216, 0x0FBF}, {217, 0x0FC0}, {218, 0x0FC1}, {219, 0x0FC2}, {220, 0x0FC3}, {221, 0x0FC4}, 
	{222, 0x0FC5}, {223, 0x0FC6}, {224, 0x0FC7}, {225, 0x0FC8}, {226, 0x0FC9}, {227, 0x0FCA}, {228, 0x0FCA}, {229, 0x0FCB}, {230, 0x0FCC}, {231, 0x0FCD}, {232, 0x0FCE}, {233, 0x0FCE}, 
	{234, 0x0FCF}, {235, 0x0FD0}, {236, 0x0FD0}, {237, 0x0FD1}, {238, 0x0FD2}, {239, 0x0FD3}, {240, 0x0FFF},
};

static float temperatureParsing_fromNTCtab(uint16_t adcDectectVal){

	float temperatureRes = 0.0F;
	uint16_t insertTemp = DEVDRIVER_TEMPERATUREDETECT_ADC_CHECKTAB_LEN / 2;
	uint16_t insertHalfTemp = DEVDRIVER_TEMPERATUREDETECT_ADC_CHECKTAB_LEN / 2;
	uint16_t loopOp = 0;

	//范围限定
	if(adcDectectVal < ntcResTemperature_checkTab[3].adcDectectVal)return -273.15F;
	if(adcDectectVal > ntcResTemperature_checkTab[DEVDRIVER_TEMPERATUREDETECT_ADC_CHECKTAB_LEN - 3].adcDectectVal)return 1000.0F;

	//二分查表
	do{
	
		loopOp ++;

		insertHalfTemp /= 2;

		if(adcDectectVal < ntcResTemperature_checkTab[insertTemp].adcDectectVal){

			insertTemp -= insertHalfTemp;
		}
		else
		{
			insertTemp += insertHalfTemp;
		}

	}while(insertHalfTemp);

	//区间线性计算
	if(adcDectectVal == ntcResTemperature_checkTab[insertTemp].adcDectectVal){
		
		temperatureRes = (float)ntcResTemperature_checkTab[insertTemp].tempratureVal;
	}
	else
	{
		
		float divisorTemp = 0.0F;
		
		if(adcDectectVal < ntcResTemperature_checkTab[insertTemp].adcDectectVal){
			
			if(adcDectectVal == ntcResTemperature_checkTab[insertTemp - 1].adcDectectVal)
				temperatureRes = (float)ntcResTemperature_checkTab[insertTemp - 1].tempratureVal;
			else{
				
				divisorTemp = (float)(ntcResTemperature_checkTab[insertTemp].tempratureVal - ntcResTemperature_checkTab[insertTemp - 1].tempratureVal) /
							  (float)(ntcResTemperature_checkTab[insertTemp].adcDectectVal - ntcResTemperature_checkTab[insertTemp - 1].adcDectectVal);
				temperatureRes = (float)(adcDectectVal - ntcResTemperature_checkTab[insertTemp - 1].adcDectectVal) *  divisorTemp + (float)ntcResTemperature_checkTab[insertTemp - 1].tempratureVal;
			}
		}
		else
		{
			if(adcDectectVal == ntcResTemperature_checkTab[insertTemp + 1].adcDectectVal)
				temperatureRes = (float)ntcResTemperature_checkTab[insertTemp + 1].tempratureVal;
			else{
				
				divisorTemp = (float)(ntcResTemperature_checkTab[insertTemp + 1].tempratureVal - ntcResTemperature_checkTab[insertTemp].tempratureVal) /
							  (float)(ntcResTemperature_checkTab[insertTemp + 1].adcDectectVal - ntcResTemperature_checkTab[insertTemp].adcDectectVal);
				temperatureRes = (float)(adcDectectVal - ntcResTemperature_checkTab[insertTemp].adcDectectVal) *  divisorTemp + (float)ntcResTemperature_checkTab[insertTemp].tempratureVal;
			}
		}
	}
	
//	printf("insert check:%d.\n", insertTemp);
//	printf("loopOp:%d.\n", loopOp);
//	printf("insertHalf check:%d.\n", insertHalfTemp);
	
	return temperatureRes;
}

static void check_efuse(void){
	
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type){

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static void devDriverBussiness_temperatureMeasure_adcInit(void){

	//Check if Two Point or Vref are burned into eFuse
	check_efuse();

	//Configure ADC
	if (unit == ADC_UNIT_1) {
		adc1_config_width(ADC_WIDTH_BIT_12);
		adc1_config_channel_atten(channel, atten);
	} else {
		adc2_config_channel_atten((adc2_channel_t)channel, atten);
	}

	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEVDRIVER_TEMPERATUREMEASURE_DEFAULT_VREF, adc_chars);
	print_char_val_type(val_type);
}

void devDriverBussiness_temperatureMeasure_periphInit(void){

	devDriverBussiness_temperatureMeasure_adcInit();
}

float devDriverBussiness_temperatureMeasure_temperatureReales(void){

	static bool tempMeasure_startFlg = false;
	static float tempSample_record = 0.0F;
	float tempSample_new = 0.0F;
	uint32_t adc_reading = 0;
	
	//Multisampling
	for(int i = 0; i < DEVDRIVER_TEMPERATUREMEASURE_NO_OF_SAMPLES; i++){

		int raw;
		
		if (unit == ADC_UNIT_1) {

			raw = adc1_get_raw((adc1_channel_t)channel);
			adc_reading += raw;
			
		} else {
		
			adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
			adc_reading += raw;
		}
	}
	adc_reading /= DEVDRIVER_TEMPERATUREMEASURE_NO_OF_SAMPLES;
	//Convert adc_reading to voltage in mV
	uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

	tempSample_new = temperatureParsing_fromNTCtab((uint16_t)adc_reading);

	if(!tempMeasure_startFlg){ //起始采样

		if(tempSample_new < 45.0F){ //首次采样为样本值，必须相对标准

			tempMeasure_startFlg = true;

			tempSample_record = tempSample_new;
		}
	}
	else
	{
		if(((tempSample_new - tempSample_record) > 10.0F) || //±10.0F大波动废值丢弃
		   ((tempSample_record - tempSample_new) > 10.0F)){

			printf("temprature data not qualified.\n");

			return devParam_temprature;
		}
	}

	printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

	//一阶滞后
	tempSample_record = DEVDRIVER_TEMPERATUREMEASURE_FILTER_COEFFIE * tempSample_new + (1.0F - DEVDRIVER_TEMPERATUREMEASURE_FILTER_COEFFIE) * tempSample_record;
	devParam_temprature = tempSample_record;

	return devParam_temprature;
}

float devDriverBussiness_temperatureMeasure_get(void){

	return devParam_temprature;
}

void devDriverBussiness_temperatureMeasure_getByHex(stt_devTempParam2Hex *param){

	const float decimal_prtCoefficient = 100.0F; //小数计算偏移倍数 --100倍对应十进制两位
	float tempratureCaculate_temp = devParam_temprature + DEVDRIVER_TEMPERATUREMEASURE_NEGATIVE_BOUND;
	
	uint16_t dataInteger_prt = (uint16_t)tempratureCaculate_temp & 0xFFFF;
	uint8_t dataDecimal_prt = (uint8_t)((tempratureCaculate_temp - (float)dataInteger_prt) * decimal_prtCoefficient);

	if(devParam_temprature < 0.0F)dataDecimal_prt = 99 - dataDecimal_prt; //针对负数处理

	param->integer_h8bit = (uint8_t)((dataInteger_prt & 0xFF00) >> 8);
	param->integer_l8bit = (uint8_t)((dataInteger_prt & 0x00FF) >> 0);
	param->decimal_8bit = dataDecimal_prt;
}



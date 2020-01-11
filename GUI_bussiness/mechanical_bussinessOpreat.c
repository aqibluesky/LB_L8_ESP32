#include "mechanical_bussinessOpreat.h"

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

#include "tips_bussinessAcoustoOptic.h"

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

 #define DEV_INFRARED_USR_KEY_PIN		(4)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

 #define DEV_SOCKET_USR_KEY_PIN			(4)	

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

 #define DEV_MOUDLE_USR_KEY_PIN			(4)

 #define DEV_MOUDLE_OPRLY_K1_PIN		(5)
 #define DEV_MOUDLE_OPRLY_K2_PIN	    (18)
 #define DEV_MOUDLE_OPRLY_K3_PIN	    (19)

 #define DEV_MOUDLE_DCODE_1_PIN			(13)
 #define DEV_MOUDLE_DCODE_2_PIN			(25)
 #define DEV_MOUDLE_DCODE_3_PIN			(34)
 #define DEV_MOUDLE_DCODE_4_PIN			(32)
 #define DEV_MOUDLE_DCODE_5_PIN			(33)
 #define DEV_MOUDLE_DCODE_6_PIN			(21)

 #define DEV_MOUDLE_DCODE_FLGVAL_FUNC_A	 0x20
 #define DEV_MOUDLE_DCODE_FLGVAL_FUNC_B  0x10
 #define DEV_MOUDLE_DCODE_FLGVAL_FUNC_C  0x08
 #define DEV_MOUDLE_DCODE_FLGVAL_FUNC_D  0x04
 #define DEV_MOUDLE_DCODE_FLGVAL_FUNC_E  0x03
#endif

static uint8_t 	val_dcode_Local = 0;

static uint16_t edCounter_usrKey = COUNTER_DISENABLE_MASK_SPECIALVAL_U16,
		 		edCounter_button = COUNTER_DISENABLE_MASK_SPECIALVAL_U16,
		 		edCntLoop_button = COUNTER_DISENABLE_MASK_SPECIALVAL_U16,
		 		toutCnt_3S5S	 = COUNTER_DISENABLE_MASK_SPECIALVAL_U16,
		 		edCounter_dCoder = COUNTER_DISENABLE_MASK_SPECIALVAL_U16;

static uint8_t  buttonContinue_rcd = 1;

static param_combinationFunPreTrig param_combinationFunTrigger_3S1L = {0},
								   param_combinationFunTrigger_3S5S = {0};

static void mechOpreatDevDatapoint_opreatNormal(uint8_t devCtrlVal){

	stt_devDataPonitTypedef devDataPoint_getTemp = {0};
	stt_devDataPonitTypedef devDataPoint_setTemp = {0};
	bool devOpReserve_flg = false;

	currentDev_dataPointGet(&devDataPoint_getTemp);
	memcpy(&devDataPoint_setTemp, &devDataPoint_getTemp, sizeof(stt_devDataPonitTypedef));

	switch(currentDev_typeGet()){

		case devTypeDef_moudleSwOneBit:{

			if(devCtrlVal & (1 << 0)){

				devDataPoint_setTemp.devType_mulitSwitch_oneBit.swVal_bit1 =\
					!devDataPoint_getTemp.devType_mulitSwitch_oneBit.swVal_bit1;
				devOpReserve_flg = true;
			}
				
		}break;
		
		case devTypeDef_moudleSwTwoBit:{

			if(devCtrlVal & (1 << 0)){

				devDataPoint_setTemp.devType_mulitSwitch_oneBit.swVal_bit1 =\
					!devDataPoint_getTemp.devType_mulitSwitch_oneBit.swVal_bit1;
				devOpReserve_flg = true;
			}
			if(devCtrlVal & (1 << 1)){

				devDataPoint_setTemp.devType_mulitSwitch_twoBit.swVal_bit2 =\
					!devDataPoint_getTemp.devType_mulitSwitch_twoBit.swVal_bit2;
				devOpReserve_flg = true;
			}
		}break;
		
		case devTypeDef_moudleSwThreeBit:{

			if(devCtrlVal & (1 << 0)){

				devDataPoint_setTemp.devType_mulitSwitch_oneBit.swVal_bit1 =\
					!devDataPoint_getTemp.devType_mulitSwitch_oneBit.swVal_bit1;
				devOpReserve_flg = true;
			}

			if(devCtrlVal & (1 << 1)){
				
				devDataPoint_setTemp.devType_mulitSwitch_twoBit.swVal_bit2 =\
					!devDataPoint_getTemp.devType_mulitSwitch_twoBit.swVal_bit2;
				devOpReserve_flg = true;
			}

			if(devCtrlVal & (1 << 2)){

				devDataPoint_setTemp.devType_mulitSwitch_threeBit.swVal_bit3 =\
					!devDataPoint_getTemp.devType_mulitSwitch_threeBit.swVal_bit3;
				devOpReserve_flg = true;
			}
			
		}break;
		
		case devTypeDef_moudleSwCurtain:{

			if(devCtrlVal & (1 << 0)){
			
				devDataPoint_setTemp.devType_curtain.devCurtain_actMethod = 0;
				devDataPoint_setTemp.devType_curtain.devCurtain_actEnumVal =\
					curtainRunningStatus_cTact_open;
				devOpReserve_flg = true;
			}
			
			if(devCtrlVal & (1 << 1)){
				
				devDataPoint_setTemp.devType_curtain.devCurtain_actMethod = 0;
				devDataPoint_setTemp.devType_curtain.devCurtain_actEnumVal =\
					curtainRunningStatus_cTact_stop;
				devOpReserve_flg = true;
			}
			
			if(devCtrlVal & (1 << 2)){
			
				devDataPoint_setTemp.devType_curtain.devCurtain_actMethod = 0;
				devDataPoint_setTemp.devType_curtain.devCurtain_actEnumVal =\
					curtainRunningStatus_cTact_close;
				devOpReserve_flg = true;
			}

		}break;

		default:break;
	}

	if(devOpReserve_flg){

		currentDev_dataPointSet(&devDataPoint_setTemp, true, true, true, true);
	}
}

static void mechOpreatUsrKey_opreatShort(void){

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	/*调试业务*/
//	static bool optFlg = false;

//	optFlg = !optFlg;

//	stt_devDataPonitTypedef devDataPoint_temp = {0};

//	devDataPoint_temp.devType_infrared.devInfrared_actCmd = optFlg;
//	devDataPoint_temp.devType_infrared.devInfrared_irIst = 127;

//	currentDev_dataPointSet(&devDataPoint_temp, false, false, true, true);

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	stt_devDataPonitTypedef devDataPoint_getTemp = {0};
	stt_devDataPonitTypedef devDataPoint_setTemp = {0};

	currentDev_dataPointGet(&devDataPoint_getTemp);
	devDataPoint_setTemp.devType_socket.devSocket_opSw =\
		!devDataPoint_getTemp.devType_socket.devSocket_opSw;
	currentDev_dataPointSet(&devDataPoint_setTemp, true, true, true, true);

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

#endif
}

static void mechOpreatUsrKey_opreatLongA(void){

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

#endif

	mdf_info_erase("ESP-MDF");
	usrApplication_systemRestartTrig(3);
	devTipsStatusRunning_abnormalTrig(tipsRunningStatus_funcTrig, 2);
	devBeepTips_trig(4, 8, 500, 100, 2);
}

static void mechOpreatUsrKey_opreatLongB(void){

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

#endif

	devBeepTips_trig(4, 8, 100, 40, 2);
}

static uint8_t mechOpreatDetect_dcodeValGet(void){

	uint8_t valDcode = 0;

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	if(!gpio_get_level(DEV_MOUDLE_DCODE_6_PIN))valDcode |= 1 << 0;
	else valDcode &= ~(1 << 0);

	if(!gpio_get_level(DEV_MOUDLE_DCODE_5_PIN))valDcode |= 1 << 1;
	else valDcode &= ~(1 << 1);

	if(!gpio_get_level(DEV_MOUDLE_DCODE_4_PIN))valDcode |= 1 << 2;
	else valDcode &= ~(1 << 2);

	if(!gpio_get_level(DEV_MOUDLE_DCODE_3_PIN))valDcode |= 1 << 3;
	else valDcode &= ~(1 << 3);

	if(!gpio_get_level(DEV_MOUDLE_DCODE_2_PIN))valDcode |= 1 << 4;
	else valDcode &= ~(1 << 4);

	if(!gpio_get_level(DEV_MOUDLE_DCODE_1_PIN))valDcode |= 1 << 5;
	else valDcode &= ~(1 << 5);
#endif

	return valDcode;
}

static void mechOpreatDetect_dcodeScanning(void){

	const  uint8_t	comfirm_Period	= 200;	//

	uint8_t  val_dcode_differ	 	= 0,
			 val_dcodeTmp		 	= 0;
	
	bool	 val_CHG				= false;

	static uint8_t 	val_dcodeCfm	= 0;
	
	val_dcodeTmp = mechOpreatDetect_dcodeValGet();
	
	if(val_dcode_Local != val_dcodeTmp){

		if(val_dcodeCfm != val_dcodeTmp){

			val_dcodeCfm = val_dcodeTmp;
			
			edCounter_dCoder = comfirm_Period;
		}
		else
		{
			if(!edCounter_dCoder){

				val_CHG	= true;
				
				edCounter_dCoder = COUNTER_DISENABLE_MASK_SPECIALVAL_U16;
			}
		}
	}
	else
	{
		val_dcodeCfm = val_dcode_Local;
	}

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	if(val_CHG){
		
		val_CHG				= false;
	
		val_dcode_differ 	= val_dcode_Local ^ val_dcodeCfm;
		val_dcode_Local 	= val_dcodeCfm	  = val_dcodeTmp;
		
		devBeepTips_trig(4, 8, 100, 40, 2);

		printf("dcode chg:%02X.\n", val_dcode_Local);
		
		if(val_dcode_differ & DEV_MOUDLE_DCODE_FLGVAL_FUNC_A){
		
			if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_A){
			

			}else{
			

			}
		}
		
		if(val_dcode_differ & DEV_MOUDLE_DCODE_FLGVAL_FUNC_B){

			stt_devStatusRecord devStatusRecordFlg_temp = {0};

			devStatusRecordIF_paramGet(&devStatusRecordFlg_temp);
			if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_B){

				devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 1;
				
			}else{
			
				devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 0;
			}
			devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, false);
		}
		
		if(val_dcode_differ & DEV_MOUDLE_DCODE_FLGVAL_FUNC_C){
		
			if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_C){

				
				
			}else{
			
				
			}
		}
		
		if(val_dcode_differ & DEV_MOUDLE_DCODE_FLGVAL_FUNC_D){
		
			if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_D){

				
				
			}else{
			
				
			}
		}

		if(val_dcode_differ & DEV_MOUDLE_DCODE_FLGVAL_FUNC_E){

			uint8_t funcTemp = val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_E;
			switch(funcTemp){

				case 0: currentDev_typeSet(devTypeDef_moudleSwCurtain, false);
						break;
				case 1: currentDev_typeSet(devTypeDef_moudleSwOneBit, false);
						break;
				case 2: currentDev_typeSet(devTypeDef_moudleSwTwoBit, false);
						break;
				case 3: currentDev_typeSet(devTypeDef_moudleSwThreeBit, false);
						break;
						
				default:break;
			}

			devDriverManageBussiness_deviceChangeRefresh(); //驱动更新
			devSystemInfoLocalRecord_normalClear();

//			printf("devType:%02X.\n", currentDev_typeGet());
		}
	}	
#endif
}

static void mechButtonOpreatFuncTrig_longA_keep(uint8_t statusPad){

}

static void mechButtonOpreatFuncTrig_normal(uint8_t statusPad, keyCfrm_Type statusCfm){

	if((press_Short == statusCfm) || press_ShortCnt == statusCfm){

		mechOpreatDevDatapoint_opreatNormal(statusPad);
	}

	switch(statusCfm){

		case press_Short:{

			printf("touchPad:%02X, shortPress.\n", (int)statusPad);

		}break;
		
		case press_ShortCnt:{

			printf("touchPad:%02X, cntPress.\n", (int)statusPad);

			buttonContinue_rcd ++;

			if(3 == buttonContinue_rcd){
			
				param_combinationFunTrigger_3S1L.param_combinationFunPreTrig_standBy_FLG = 1; //
				param_combinationFunTrigger_3S1L.param_combinationFunPreTrig_standBy_keyVal = statusPad; //
				
			}else{
			
				memset(&param_combinationFunTrigger_3S1L, 0, sizeof(param_combinationFunPreTrig)); //
			} 

		}break;
		
		case press_LongA:{

			if((param_combinationFunTrigger_3S1L.param_combinationFunPreTrig_standBy_FLG) && 
			   (statusPad == param_combinationFunTrigger_3S1L.param_combinationFunPreTrig_standBy_keyVal)){ //
						
				memset(&param_combinationFunTrigger_3S1L, 0, sizeof(param_combinationFunPreTrig)); 

				printf("combination fun<3S1L> trig!\n");
			}
			else
			{
				printf("touchPad:%02X, longPress_A.\n", (int)statusPad);
			}

		}break;
		
		case press_LongB:{}break;

		default:break;
	}

	if(statusCfm != press_ShortCnt){

		buttonContinue_rcd = 1;
		memset(&param_combinationFunTrigger_3S1L, 0, sizeof(param_combinationFunPreTrig));

		if(statusCfm != press_Short)
			memset(&param_combinationFunTrigger_3S5S, 0, sizeof(param_combinationFunPreTrig));
	}
}

static void mechButtonOpreatFuncTrig_trigContinue(uint8_t statusPad, uint8_t loopCount){

	printf("touchPad:%02X, %02Xtime pressOver.\n", (int)statusPad, (int)loopCount);

	switch(loopCount){

		case 3:{
			
			param_combinationFunTrigger_3S5S.param_combinationFunPreTrig_standBy_FLG = 1; //
			param_combinationFunTrigger_3S5S.param_combinationFunPreTrig_standBy_keyVal = statusPad;  //
			toutCnt_3S5S = 3000;  //
		}break;

		case 5:{

			if((param_combinationFunTrigger_3S5S.param_combinationFunPreTrig_standBy_FLG) && 
			   (statusPad == param_combinationFunTrigger_3S5S.param_combinationFunPreTrig_standBy_keyVal)){ //
			
				memset(&param_combinationFunTrigger_3S5S, 0, sizeof(param_combinationFunPreTrig)); //

				printf("combination fun<3S5S> trig!\n");
			}
		}break;

		default:break;
	}
}

static uint8_t mechOpreatDetect_buttonValGet(void){

	uint8_t valButton = 0;

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	if(!gpio_get_level(DEV_MOUDLE_OPRLY_K1_PIN))valButton |= 1 << 0;
	else valButton &= ~(1 << 0);

	if(!gpio_get_level(DEV_MOUDLE_OPRLY_K2_PIN))valButton |= 1 << 1;
	else valButton &= ~(1 << 1);

	if(!gpio_get_level(DEV_MOUDLE_OPRLY_K3_PIN))valButton |= 1 << 2;
	else valButton &= ~(1 << 2);
#endif

	return valButton;
}

static void mechOpreatDetect_buttonScanning(void){

	static uint8_t 	touchPad_temp 		= 0;
	static bool 	keyPress_FLG 		= false;
	
	static bool 	funTrigFLG_LongA 	= false;
	static bool 	funTrigFLG_LongB 	= false;
	
	const  uint16_t	touchCfrmTaky_period= 350,
					touchCfrmLoop_Short = 20,	//
					touchCfrmLoop_LongA = 8000,	//
					touchCfrmLoop_LongB = 15000,	//
					touchCfrmLoop_MAX	= 60000;//
	
	static uint8_t 	pressContinueGet 	= 0;
	       uint8_t 	pressContinueCfm 	= 0;
	
		   uint16_t conterTemp 			= 0; //
	
	if(!toutCnt_3S5S)memset(&param_combinationFunTrigger_3S5S, 0, sizeof(param_combinationFunPreTrig)); //
	
	if(mechOpreatDetect_buttonValGet()){
		
		if(!keyPress_FLG){
		
			keyPress_FLG = true;
			edCounter_button = touchCfrmLoop_MAX;
			edCntLoop_button = touchCfrmTaky_period;  //
			touchPad_temp = mechOpreatDetect_buttonValGet();
		}
		else
		{
			if(touchPad_temp == mechOpreatDetect_buttonValGet()){ //
				
				conterTemp = touchCfrmLoop_MAX - edCounter_button;
			
				if(conterTemp > touchCfrmLoop_LongA && conterTemp <= touchCfrmLoop_LongB){
				
					mechButtonOpreatFuncTrig_longA_keep(touchPad_temp); //
					
					if(!funTrigFLG_LongA){
					
						funTrigFLG_LongA = true;
						mechButtonOpreatFuncTrig_normal(touchPad_temp, press_LongA);
					}
				}
				if(conterTemp > touchCfrmLoop_LongB && conterTemp <= touchCfrmLoop_MAX){
				
					if(!funTrigFLG_LongB){
					
						funTrigFLG_LongB = true;
						mechButtonOpreatFuncTrig_normal(touchPad_temp, press_LongB);
					}
				}
				
			}
			else
			{
				if((touchCfrmLoop_MAX - edCounter_button) < touchCfrmLoop_Short){ //
				
					edCounter_button = touchCfrmLoop_MAX;
					edCntLoop_button = touchCfrmTaky_period;  //
					touchPad_temp = mechOpreatDetect_buttonValGet();
				}
			}
		}
	}
	else
	{
		if(keyPress_FLG){
		
			conterTemp = touchCfrmLoop_MAX - edCounter_button;
			if(conterTemp > touchCfrmLoop_Short && conterTemp <= touchCfrmLoop_LongA){
			
				if(edCntLoop_button)pressContinueGet ++;
				if(pressContinueGet <= 1)mechButtonOpreatFuncTrig_normal(touchPad_temp, press_Short); //
				else mechButtonOpreatFuncTrig_normal(touchPad_temp, press_ShortCnt);
			}

			edCounter_button = COUNTER_DISENABLE_MASK_SPECIALVAL_U16;
		}
	
		if(!edCntLoop_button && pressContinueGet){
		
			pressContinueCfm = pressContinueGet;
			pressContinueGet = 0;
			
			if(pressContinueCfm >= 2){
				
				mechButtonOpreatFuncTrig_trigContinue(touchPad_temp, pressContinueCfm);
				pressContinueCfm = 0;
			}
			
			touchPad_temp = 0;
		}

		if(funTrigFLG_LongA){funTrigFLG_LongA = false;}
		if(funTrigFLG_LongB){funTrigFLG_LongB = false;}
			
		keyPress_FLG = false;
	}
}

static bool mechOpreatDetect_usrKeyValGet(void){

	bool valKey = false;

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
	valKey = !gpio_get_level(DEV_INFRARED_USR_KEY_PIN);
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
	valKey = !gpio_get_level(DEV_SOCKET_USR_KEY_PIN);
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)
	valKey = !gpio_get_level(DEV_MOUDLE_USR_KEY_PIN);
#endif
	
	return valKey;
}

static void mechOpreatDetect_usrKeyScanning(void){

	const uint16_t keyPressCfmPeriod_MAX 	= 60000,
				   keyPressCfmPeriod_short 	= 20,
				   keyPressCfmPeriod_longA 	= 8000,
				   keyPressCfmPeriod_longB 	= 20000;
		  uint16_t keyPressCfmCounterTemp 	= 0;
	static bool kPress_flg = false,
				longA_flg  = false,
				longB_flg  = false;

	if(mechOpreatDetect_usrKeyValGet()){

		if(!kPress_flg){

			kPress_flg = true;

			edCounter_usrKey = keyPressCfmPeriod_MAX;
		}	

		keyPressCfmCounterTemp = keyPressCfmPeriod_MAX - edCounter_usrKey;

		if((keyPressCfmCounterTemp >= keyPressCfmPeriod_longA) && 
		   (keyPressCfmCounterTemp <= keyPressCfmPeriod_longB) && 
		   (!longA_flg)){

			longA_flg = true;

			printf("usrKey longA press.\n");

			mechOpreatUsrKey_opreatLongA();
		}

		if((keyPressCfmCounterTemp >= keyPressCfmPeriod_longB) && 
		   (keyPressCfmCounterTemp <= keyPressCfmPeriod_MAX) && 
		   (!longB_flg)){

			longB_flg = true;

			printf("usrKey longB press.\n");

			mechOpreatUsrKey_opreatLongB();
		}	
	}
	else
	{
		if(kPress_flg){

			kPress_flg = false;

			keyPressCfmCounterTemp = keyPressCfmPeriod_MAX - edCounter_usrKey;

			if((keyPressCfmCounterTemp >= keyPressCfmPeriod_short) && 
			   (keyPressCfmCounterTemp <= keyPressCfmPeriod_longA)){

				printf("usrKey short press.\n");

				mechOpreatUsrKey_opreatShort();
			}

			longA_flg = false;
			longB_flg = false;
			edCounter_usrKey = COUNTER_DISENABLE_MASK_SPECIALVAL_U16;
		}
	}
}

void devMechanicalOpreatTimeCounter_realesing(void){

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)|\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)|\
   (L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	COUNTER_COUNTING_DOWN_LEGAL_U16(edCounter_usrKey)
	COUNTER_COUNTING_DOWN_LEGAL_U16(edCounter_button)
	COUNTER_COUNTING_DOWN_LEGAL_U16(edCntLoop_button)
	COUNTER_COUNTING_DOWN_LEGAL_U16(toutCnt_3S5S)
	COUNTER_COUNTING_DOWN_LEGAL_U16(edCounter_dCoder)
#endif
}

void devMechanicalOpreatDetecting(void){

	mechOpreatDetect_dcodeScanning();
	mechOpreatDetect_buttonScanning();
	mechOpreatDetect_usrKeyScanning();
}

static void task_devOpreating_detecting(void *pvParameter){

	for(;;){

		devStatusRunning_tipsRefresh();
		devMechanicalOpreatDetecting();
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
}

void deviceTypeDefineByDcode_preScanning(void){

	val_dcode_Local = mechOpreatDetect_dcodeValGet();

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_A){


	}

	if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_B){ //状态记忆

		stt_devStatusRecord devStatusRecordFlg_temp = {0};

		devStatusRecordFlg_temp.devStatusOnOffRecord_IF = 1;
		devStatusRecordIF_paramSet(&devStatusRecordFlg_temp, false);
	}

	if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_C){


	}

	if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_D){


	}

	if(val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_E){ //开关类型

		uint8_t funcTemp = val_dcode_Local & DEV_MOUDLE_DCODE_FLGVAL_FUNC_E;
		switch(funcTemp){
		
			case 0: currentDev_typeSet(devTypeDef_moudleSwCurtain, false);
					break;
			case 1: currentDev_typeSet(devTypeDef_moudleSwOneBit, false);
					break;
			case 2: currentDev_typeSet(devTypeDef_moudleSwTwoBit, false);
					break;
			case 3: currentDev_typeSet(devTypeDef_moudleSwThreeBit, false);
					break;
			default:break;
		}
	}
	
#endif
}

void devMechanicalOpreatPeriphInit(void){

	gpio_config_t io_conf = {0};

	//disable interrupt
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_INPUT;
	//bit mask of the pins that you want to set
#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)	

	io_conf.pin_bit_mask = (1ULL << DEV_INFRARED_USR_KEY_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 1;
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	io_conf.pin_bit_mask = (1ULL << DEV_SOCKET_USR_KEY_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 1;
#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	io_conf.pin_bit_mask = (1ULL << DEV_MOUDLE_USR_KEY_PIN)  |\
						   (1ULL << DEV_MOUDLE_OPRLY_K1_PIN) |\
						   (1ULL << DEV_MOUDLE_OPRLY_K2_PIN) |\
						   (1ULL << DEV_MOUDLE_OPRLY_K3_PIN) |\
						   (1ULL << DEV_MOUDLE_DCODE_1_PIN)  |\
						   (1ULL << DEV_MOUDLE_DCODE_2_PIN)  |\
						   (1ULL << DEV_MOUDLE_DCODE_3_PIN)  |\
						   (1ULL << DEV_MOUDLE_DCODE_4_PIN)  |\
						   (1ULL << DEV_MOUDLE_DCODE_5_PIN)  |\
						   (1ULL << DEV_MOUDLE_DCODE_6_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en 	 = 1;
#endif
	//configure GPIO with the given settings
	gpio_config(&io_conf);
}

void devOpreation_bussinessInit(void){

#if(L8_DEVICE_TYPE_PANEL_DEF != DEV_TYPES_PANEL_DEF_INDEP_MOUDLE) //已被提前初始化
	devMechanicalOpreatPeriphInit();
#endif
	devTipsAcoustoOpticPeriphInit();

	mutualCtrlTrigIf_A =\
	mutualCtrlTrigIf_B =\
	mutualCtrlTrigIf_C = true;

	xTaskCreate(task_devOpreating_detecting, //Task Function
				"devOpreatDetect", //Task Name
				1024 * 4,	 //Stack Depth
				NULL,		 //Parameters
				CONFIG_MDF_TASK_DEFAULT_PRIOTY, //Priority	
				NULL);		 //Task Handler
}












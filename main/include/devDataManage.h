/**
 * @file devDataManage.h
 *
 */

#ifndef DEVDATAMANAGE_H
#define DEVDATAMANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "esp_types.h"

#include "mwifi.h"

#include "lvgl.h"

#include "devDataManage.h"

/*********************
 *      DEFINES
 *********************/
//#define L8_DEVICE_VERSION						0x08
#define L8_DEVICE_VERSION						0x18 //开关设备版本号（设备版本号高四位自增向后迭代，高四位为0，语音控制没有高位区分，要慢一点）

#define L8_DEVICE_VERSION_REF_DISCRIPTION		"L8-v1.0.16[202001091606]" //固件说明描述版本号

#define DEVICE_MAC_ADDR_APPLICATION_LEN			6

#define DEVLIST_MANAGE_LISTNUM_MASK_NULL		0xFF	//设备数量为空时 掩码

#define L8_MESH_CONMUNICATE_RESTART_NOTICE_STR	"restart"
#define L8_MESH_CONMUNICATE_UPGSTBY_NOTICE_STR	"upgrade stand by"

#define DATAMANAGE_LANGUAGE_TAB_MAXNUM			20
#define DATAMANAGE_LANGUAGE_ABBRE_MAXLEN		6

#define COUNTER_DISENABLE_MASK_SPECIALVAL_U32	4294967295 	//32位计时counter失效掩码值
#define COUNTER_DISENABLE_MASK_SPECIALVAL_U16	65535		//16位计时counter失效掩码值
#define COUNTER_DISENABLE_MASK_SPECIALVAL_U8	255			//8位计时counter失效掩码值

#define COUNTER_COUNTING_DOWN_LEGAL_U32(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U32){if(x)x --;}	
#define COUNTER_COUNTING_DOWN_LEGAL_U16(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U16){if(x)x --;}	
#define COUNTER_COUNTING_DOWN_LEGAL_U8(x)		if(x != COUNTER_DISENABLE_MASK_SPECIALVAL_U8){if(x)x --;}	

#define DEVICE_MUTUAL_CTRL_GROUP_NUM			3		//设备互控组数量
#define MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP	5		//单个互控组内最多设备数量
#define DEVICE_MUTUALGROUP_INVALID_INSERT_A		0		//互控组号 无效值A:0
#define DEVICE_MUTUALGROUP_INVALID_INSERT_B		0xFF	//互控组号 无效值A:0xFF

#define L8_DEVICE_TYPEDEFINE_BASE_ADDR		(0x49)

#define TOUCHEVENT_FLG_BITHOLD_RESERVE		(0x00FF)

#define TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG	(1 << 0)
#define TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP	(1 << 1)

#define FTIC_EXTRA_REGISTERVAL_GES_mUP		0x10
#define FTIC_EXTRA_REGISTERVAL_GES_mDN		0x18
#define FTIC_EXTRA_REGISTERVAL_GES_mLT		0x14
#define FTIC_EXTRA_REGISTERVAL_GES_mRT		0x1C
#define FTIC_EXTRA_REGISTERVAL_GES_zIN		0x48
#define FTIC_EXTRA_REGISTERVAL_GES_zOUT		0x49

#define DEV_RUNNING_FLG_BIT_TIMER			(1 << 0)
#define DEV_RUNNING_FLG_BIT_DELAY			(1 << 1)
#define DEV_RUNNING_FLG_BIT_GREENMODE		(1 << 2)
#define DEV_RUNNING_FLG_BIT_NIGHTMODE		(1 << 3)
#define DEV_RUNNING_FLG_BIT_DEVLOCK			(1 << 4)
#define DEV_RUNNING_FLG_BIT_DEVHOSTIF		(1 << 5)
#define DEV_RUNNING_FLG_BIT_RESERVE_3		(1 << 6)
#define DEV_RUNNING_FLG_BIT_RESERVE_4		(1 << 8)
#define DEV_RUNNING_FLG_BIT_RESERVE_5		(1 << 9)
#define DEV_RUNNING_FLG_BIT_RESERVE_6		(1 << 10)
#define DEV_RUNNING_FLG_BIT_RESERVE_7		(1 << 11)
#define DEV_RUNNING_FLG_BIT_RESERVE_8		(1 << 12)
#define DEV_RUNNING_FLG_BIT_RESERVE_9		(1 << 13)
#define DEV_RUNNING_FLG_BIT_RESERVE_10		(1 << 14)
#define DEV_RUNNING_FLG_BIT_RESERVE_11		(1 << 15)

#define DEV_HEX_PROTOCOL_APPLEN_CONTROL			1	//16进制字节格式协议应用数据长度:普通控制
#define DEV_HEX_PROTOCOL_APPLEN_DEVLOCK			1	//16进制字节格式协议应用数据长度:设备锁定操作
#define DEV_HEX_PROTOCOL_APPLEN_TIMERSET		24	//16进制字节格式协议应用数据长度:普通定时设置
#define DEV_HEX_PROTOCOL_APPLEN_DELAYSET		3	//16进制字节格式协议应用数据长度:延时设置
#define DEV_HEX_PROTOCOL_APPLEN_GREENMODESET	2	//16进制字节格式协议应用数据长度:绿色模式设置
#define DEV_HEX_PROTOCOL_APPLEN_NIGHTMODESET	6	//16进制字节格式协议应用数据长度:夜间模式设置
#define DEV_HEX_PROTOCOL_APPLEN_EXTPARAMSET		5	//16进制字节格式协议应用数据长度:开关除普通控制外的额外参数设置

#define L8_NODEDEV_KEEPALIVE_PERIOD				249	//mesh内部心跳子节点生命周期

#define L8_DEV_LISTMANAGE_REALES_CONFIRM		40	//mesh内部心跳管理表确认可用时间，单位：s

#define DEV_TIMER_OPREATION_OBJSLIDER_VAL_DIV	5	//设备设置定时业务开关响应值时，滑动条操作分量

#define DEV_MESH_PSD_DEFAULT()					{"psd456456123"}

#define GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE		40
#define GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE	   (120 * 20) //图片动态内存大小定义
#define MQTT_REMOTE_DATATRANS_PIC_DATABAG_LENGTH		976

#define GUI_BUSSINESS_HOME_BGROUND_PIC_PIXEL_SIZE	   (240 * 320) //背景底图像素大小

#define MQTT_REMOTE_DATATRANS_PIC_DATABAG_LENGTH_S		768
#define MQTT_REMOTE_DATATRANS_PARAM_IP_DEF				{112, 124, 61, 191}
#define MQTT_REMOTE_DATATRANS_PARAM_PORT_DEF			8888

/********************************
 *     DEVIVE VERSION DEFINES 
 ********************************/	
#define PROGMA_FLASH_PARTITIONS_OFFSET_A		0x00
#define PROGMA_FLASH_PARTITIONS_OFFSET_B		0x20
#define DEVICE_FLASH_PARTITIONS_OFFSET			PROGMA_FLASH_PARTITIONS_OFFSET_B

#define DEVICE_DRIVER_METHOD_BY_SLAVE_MCU		0x01
#define DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE	0x02
	
#define DEVICE_DRIVER_DEFINITION				DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU)

 #define DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST		0
#endif

 #define DEV_TYPES_PANEL_DEF_NULL				0

#if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SELF_HARDWARE) //--是否为磁保持继电器版本
 #define DEV_TYPES_PANEL_DEF_ALL_RESERVE		0xFA  //面板类型定义：所有可选
 #define DEV_TYPES_PANEL_DEF_SHARE_MIX			(0x01 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：六合一（多位开关和窗帘等 ）
 #define DEV_TYPES_PANEL_DEF_INDEP_FANS			(0x02 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅风扇 
 #define DEV_TYPES_PANEL_DEF_INDEP_DIMMER		(0x03 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅调光
 #define DEV_TYPES_PANEL_DEF_INDEP_HEATER		(0x04 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅热水器

 #define DEV_TYPES_PANEL_DEF_INDEP_INFRARED		(0x11 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅红外转发器
 #define DEV_TYPES_PANEL_DEF_INDEP_SOCKET		(0x12 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅插座
 #define DEV_TYPES_PANEL_DEF_INDEP_MOUDLE		(0x13 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅三位模块
 
#else
 #define DEV_TYPES_PANEL_DEF_ALL_RESERVE 	    0xFB  //面板类型定义：所有可选
 #define DEV_TYPES_PANEL_DEF_SHARE_MIX		    (0x05 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：六合一（多位开关和窗帘等 ）
 #define DEV_TYPES_PANEL_DEF_INDEP_FANS		    (0x06 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅风扇 
 #define DEV_TYPES_PANEL_DEF_INDEP_DIMMER	    (0x07 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅调光
 #define DEV_TYPES_PANEL_DEF_INDEP_HEATER	    (0x08 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅热水器

 #define DEV_TYPES_PANEL_DEF_INDEP_INFRARED		(0x18 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅红外转发器
 #define DEV_TYPES_PANEL_DEF_INDEP_SOCKET		(0x19 + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅插座
 #define DEV_TYPES_PANEL_DEF_INDEP_MOUDLE		(0x1A + DEVICE_FLASH_PARTITIONS_OFFSET) //面板类型定义：仅三位模块 
 
#endif

#define L8_DEVICE_TYPE_PANEL_DEF				DEV_TYPES_PANEL_DEF_SHARE_MIX

#if(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_ALL_RESERVE)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwThreeBit

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_SHARE_MIX)

 #if(DEVICE_DRIVER_DEFINITION == DEVICE_DRIVER_METHOD_BY_SLAVE_MCU) && (DRVMETHOD_BY_SLAVE_MCU_RELAY_TEST == 1)
	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwTwoBit
 #else
 	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_mulitSwThreeBit
 #endif

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_FANS)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_fans

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_DIMMER)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_dimmer

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_HEATER)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_heater

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_INFRARED)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_infrared

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_SOCKET)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_socket

#elif(L8_DEVICE_TYPE_PANEL_DEF == DEV_TYPES_PANEL_DEF_INDEP_MOUDLE)

	#define	L8_DEVICE_TYPE_DEFULT				devTypeDef_moudleSwCurtain

#endif

/**********************
 *      TYPEDEFS
 **********************/
 enum{ //窗帘专用动作描述值枚举
 
	 devCurtain_actEnum_Open = 0,
	 devCurtain_actEnum_Puase,
	 devCurtain_actEnum_Close,
 }devCurtain_actEnum;
 
 enum{ //风扇专用动作描述值枚举
 
	 devFans_swEnum_Close = 0,
	 devFans_swEnum_sw1,
	 devFans_swEnum_sw2,
	 devFans_swEnum_sw3,
 }devFans_swEnum;

 enum{ //区分语言部分文字国家标识

	countryT_EnglishSerail = 0,
	countryT_Arabic,
	countryT_Hebrew,
 }countryTextPtrEnum;

 enum{ //节点状态
 
	 roleMeshStatus_noConnect = 0,
	 roleMeshStatus_root,
	 roleMeshStatus_node,
 };

 typedef union{
 
	 struct{
 
		 uint8_t swVal_bit1:1; 

		 uint8_t rsv:6;

		 uint8_t opCtrl_bit1:1; 
	 }devType_mulitSwitch_oneBit;
 
	 struct{

		 uint8_t swVal_bit1:1; 
		 uint8_t swVal_bit2:1; 

		 uint8_t rsv:4;

		 uint8_t opCtrl_bit1:1; 
		 uint8_t opCtrl_bit2:1; 
	 }devType_mulitSwitch_twoBit;
 
	 struct{
 
		 uint8_t swVal_bit1:1; 
		 uint8_t swVal_bit2:1; 
		 uint8_t swVal_bit3:1; 

		 uint8_t rsv:2;

		 uint8_t opCtrl_bit1:1; 
		 uint8_t opCtrl_bit2:1; 
		 uint8_t opCtrl_bit3:1; 
	 }devType_mulitSwitch_threeBit;
 
	 struct{
 
		 uint8_t devCurtain_actEnumVal:7;
		 uint8_t devCurtain_actMethod:1; // 1:滑条， 0:按钮
	 }devType_curtain;

	 struct{
 
		 uint8_t devHeater_swEnumVal;
	 }devType_heater;

	 struct{

	 	 uint8_t devThermostat_running_en:1; //bit0
	 	 uint8_t devThermostat_nightMode_en:1; //bit1
		 uint8_t devThermostat_tempratureTarget:6; //bit2-7
	 }devType_thermostat;

	 struct{
 
		 uint8_t devScenario_opNum;
	 }devType_scenario;

	 struct{
 
		 uint8_t devFans_swEnumVal;
	 }devType_fans;
 
	 struct{
 
		 uint8_t devDimmer_brightnessVal;
	 }devType_dimmer;

	 struct{

		 uint8_t devInfrared_irIst:7;
		 uint8_t devInfrared_actCmd:1; // 1:学习， 0:控制
	 }devType_infrared;

	 struct{
		
		 uint8_t devSocket_opSw:1;
	 }devType_socket;
 }stt_devDataPonitTypedef;
 
typedef enum{

	devTypeDef_mulitSwOneBit = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 1),
	devTypeDef_mulitSwTwoBit,
	devTypeDef_mulitSwThreeBit,
	devTypeDef_dimmer,
	devTypeDef_fans,
	devTypeDef_scenario,
	devTypeDef_curtain,
	devTypeDef_infrared,
	devTypeDef_socket,
	devTypeDef_moudleSwOneBit,
	devTypeDef_moudleSwTwoBit,
	devTypeDef_moudleSwThreeBit,
	devTypeDef_moudleSwCurtain,
	devTypeDef_thermostat = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x1E),
	devTypeDef_heater = (L8_DEVICE_TYPEDEFINE_BASE_ADDR + 0x1F),
	devTypeDef_thermostatExtension,
//	devTypeDef_curtainExtension,
}devTypeDef_enum;

typedef enum{

	dataManagement_msgType_null = 0,
	dataManagement_msgType_homePageCtrlObjTextChg,
	dataManagement_msgType_homePageCtrlObjIconChg,
	dataManagement_msgType_homePageThemeTypeChg,
	dataManagement_msgType_homePagePicGroundChg
}enumMsgType_dataManagement;

typedef struct{

	enumMsgType_dataManagement msgType;

	union _unn_dataDmHandle{

		struct _sttData_hpCtrlObjTextChg{

			uint8_t objChg_bitHold;
		}dataAb_hpCtrlObjTextChg;

		struct _sttData_hpCtrlObjIconChg{

			uint8_t objChg_bitHold;
		}dataAb_hpCtrlObjIconChg;

		struct _sttData_hpThemeTypeChg{

			uint8_t themeTypeChg_notice:1;
		}dataAb_hpThemeTypeChg;	

		struct _sttData_hpPicGroundChg{

			uint8_t picGroundChg_notice:1;
		}dataAb_hpPicGroundChg;	

	}msgData_dmHandle;

}stt_msgDats_dataManagementHandle;

typedef struct{

	uint8_t linkageRunning_proxmity_en:1;
	uint8_t linkageRunning_temprature_en:1;

	int8_t linkageCondition_tempratureVal;

	stt_devDataPonitTypedef linkageReaction_proxmity_swVal;
	uint8_t 				linkageReaction_proxmity_trigEn:1;
	
	uint8_t 				linkageReaction_proxmity_scrLightTrigIf:1;
	stt_devDataPonitTypedef linkageReaction_temprature_swVal;
	
}stt_paramLinkageConfig;

typedef struct{

	const uint16_t loopPeriod;
	uint16_t loopCounter;
}stt_timerLoop;

typedef struct{

	uint16_t loopPeriod;
	uint16_t loopCounter;
}stt_timerLoop_ext;

typedef struct{

	const uint32_t loopPeriod;
	uint32_t loopCounter;
}stt_timerLoop32BitCnt;

typedef struct{

	uint32_t loopPeriod;
	uint32_t loopCounter;
}stt_timerLoop32BitCnt_ext;

typedef struct{

	uint8_t mutualCtrlGroup_insert; //互控组号
	uint8_t mutualCtrlDevNum; //该互控组内设备数量
	uint8_t mutualCtrlDevMacList[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP * MWIFI_ADDR_LEN]; //该互控组内设备MAC地址列表
}stt_devMutualGroupParam;

typedef struct{

	uint8_t routerInfo_ssid[32];
	uint8_t routerInfo_psd[64];
}stt_routerCfgInfo;

typedef struct{

	uint8_t  ip_remote[4];
	uint16_t port_remote;
}stt_mqttCfgParam;

typedef struct{

	uint8_t devNum;
	uint8_t mGroupIst;
	struct stt_mutualInfoUnitDev{

		uint8_t devUnitMAC[MWIFI_ADDR_LEN];
		uint8_t devMutualBitIst;
		
	}unitDevMutualInfo[MAXNUM_OF_DEVICE_IN_SINGLE_MUTUALGROUP];
}stt_mutualCtrlInfoResp;

 typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devTempParam2Hex;

typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devPowerParam2Hex;

typedef struct{

	uint8_t  integer_h8bit;
	uint8_t  integer_l8bit;
	uint8_t  decimal_8bit;
}stt_devElecsumParam2Hex;

typedef struct{

	uint8_t devTypedef;
	uint8_t devVersion;
	stt_devDataPonitTypedef devStatus_valCurrent;
	stt_devTempParam2Hex devFuncInfo_temprature;
	stt_devPowerParam2Hex devFuncInfo_elecPower;
}stt_blufiConfigDevInfo_resp;

#define DEVPARAMEXT_DT_LEN	4
typedef struct{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	uint8_t nodeDev_Type;
	uint8_t nodeDev_Status;
	uint16_t nodeDev_DevRunningFlg;
	stt_devTempParam2Hex nodeDev_dataTemprature;
	stt_devPowerParam2Hex nodeDev_dataPower;	
	uint8_t nodeDev_extFunParam[DEVPARAMEXT_DT_LEN];
}stt_devStatusInfoResp; //与stt_hbDataUpload不同，只获取需要的属性进行回复

typedef struct{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	stt_devElecsumParam2Hex nodeDev_dataElecsum;
}stt_devUnitElecsumReport;

typedef struct{

	uint8_t devStatusOnOffRecord_IF:1; //是否记忆开关状态
	uint8_t devElecsumDisp_IF:1; //电路参数显示类别（1：显示电流 0：显示功率）
	uint8_t devScreenLandscape_IF:1; //是否横屏（1：横屏 0：竖屏）
	uint8_t devUpgradeFirstRunning_FLG:1; //是否为第一次启动
	uint8_t devStatusData_reserve:4; //reserve
}stt_devStatusRecord;

typedef struct{

	uint8_t unitDevMac[6];
	uint8_t unitDevOpreat_val;
}stt_scenarioUnitOpreatParam;

typedef struct{

	uint8_t scenarioInsert_num; //场景索引
	uint8_t scenarioDevice_sum; //场景内设备数量
}stt_scenarioSwitchData_nvsOpreatRef; 
/*单个场景数据最大存储长度：sizeof(stt_scenarioSwitchData_nvsOpreatRef) + 128 + 128*/ //数据说明头 + 数据上半部 + 数据下半部

#define DEVSCENARIO_NVSDATA_HALFOPREAT_NUM		128 //场景数据半部存储数量
typedef struct{

	stt_scenarioSwitchData_nvsOpreatRef dataRef;
	stt_scenarioUnitOpreatParam dataHalf_A[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
	stt_scenarioUnitOpreatParam dataHalf_B[DEVSCENARIO_NVSDATA_HALFOPREAT_NUM];
}stt_scenarioSwitchData_nvsOpreat;

#define DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN 	232 //红外转单条指令操作数据缓存
typedef struct{

	uint8_t ir_dataOpreat_buf[DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN];
}stt_infraredSwitchData_nvsOpreat;

typedef struct{

	stt_infraredSwitchData_nvsOpreat dats;
	char 							 nvsKeyWord[16];
}stt_devInfraredNvsOpreatParam;

#define GUIBUSSINESS_CTRLOBJ_MAX_NUM 3
typedef struct{

	uint8_t countryFlg;
	uint8_t dataBtnTextDisp[GUIBUSSINESS_CTRLOBJ_MAX_NUM][GUI_BUSSINESS_HOME_BTNTEXT_STR_UTF8_SIZE];
}stt_dataDisp_guiBussinessHome_btnText;

typedef struct{

	uint8_t    valGestrue;

	uint8_t    touchNum;
	lv_point_t touchPosition[5];
}stt_touchEveInfo;

typedef struct{

	uint8_t bGround_keyTheme_ist;
	uint8_t bGround_picOrg_ist;
}stt_bGroundThemeParam;

typedef enum{

	saveObj_swStatus = 0,
	saveObj_infoTimer_normal,
	saveObj_infoTimer_nightMode,
	saveObj_infoTimer_greenMode,
	saveObj_devTimeZone,
	saveObj_devEelcSum,
	saveObj_devRunning_flg,
	saveObj_devTypeDef,
	saveObj_devStatusRecordIF,
	saveObj_routerBssid,
	saveObj_routerBssidRcd,
	saveObj_mutualCtrlInfo,
	saveObj_devDriver_linkageConfigParam_set,
	saveObj_devGuiBussinessHome_btnTextDisp,
	saveObj_devGuiBussinessHome_btnIconDisp,
	saveObj_devGuiBussinessHome_btnTextPic_A,
	saveObj_devGuiBussinessHome_btnTextPic_B,
	saveObj_devGuiBussinessHome_btnTextPic_C,
	saveObj_devGuiBussinessHome_bGroundPic,
	saveObj_devGuiBussinessHome_themeParam,

	saveObj_devDriver_iptRecalibration_set,
	saveObj_devDriver_screenRunningParam_set,

	saveObj_devCurtain_runningParam,

	saveObj_devInfrared_paramDats,
	saveObj_devInfrared_timerUpIstTab,

	saveObj_devThermostatExSw_statusDats,

	saveObj_devScenario_paramDats_0,
	saveObj_devScenario_paramDats_1,
	saveObj_devScenario_paramDats_2,

	saveObj_dtMqttCfgParam,

	saveObj_routerConfigInfo,

	saveObj_devHeater_customTimeParam,

	saveObj_paramMagRelayTest,

}enum_dataSaveObj;

typedef enum{

	caliStatus_noCfm = 0,
	caliStatus_Y,
	caliStatus_N,
}enum_touchCaliRsv;

/*-------------------------------心跳数据管理链表 数据结构--------------------------------------*/
typedef struct struct_hbDataUpload{

	uint8_t nodeDev_Mac[MWIFI_ADDR_LEN];
	uint8_t nodeDev_Type;
	uint8_t nodeDev_Status;
	uint8_t nodeDev_mautualInfo[DEVICE_MUTUAL_CTRL_GROUP_NUM];
	uint16_t nodeDev_runningFlg;
	stt_devTempParam2Hex nodeDev_dataTemprature;
	stt_devPowerParam2Hex nodeDev_dataPower;
	stt_devElecsumParam2Hex nodeDev_dataElecsum;
	uint8_t nodeDev_extFunParam[DEVPARAMEXT_DT_LEN];

}stt_hbDataUpload;

typedef struct struct_nodeDev_hbDataManage{

	stt_hbDataUpload dataManage;
	uint16_t nodeDevKeepAlive_counter;

	struct struct_nodeDev_hbDataManage *next;
	
}stt_nodeDev_hbDataManage;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern xQueueHandle msgQh_dataManagementHandle;

extern bool mutualCtrlTrigIf_A; 
extern bool mutualCtrlTrigIf_B;
extern bool mutualCtrlTrigIf_C;

extern const char meshPsd_default[20];

extern uint8_t *dataPtr_bGroundPic;
extern uint8_t bGroundPic_reserveFlg;

//extern uint8_t *dataPtr_btnTextImg_sw_A;
//extern uint8_t *dataPtr_btnTextImg_sw_B;
//extern uint8_t *dataPtr_btnTextImg_sw_C;

uint8_t numCheckMethod_customLanbon(uint8_t *dats, uint8_t len);

uint8_t systemDevice_startUpTime_get(void);

void gui_bussinessHome_btnText_dataReales(uint8_t picIst, uint8_t *picDataBuf, uint8_t *dataLoad, uint16_t dataLoad_len, uint8_t dataBagIst, bool lastFrame_If);
void gui_bussinessHome_bGroundPic_dataReales(uint8_t *picDataBuf, uint8_t *dataLoad, uint16_t dataLoad_len, uint16_t dataBagIst, bool lastFrame_If);

void flgSet_gotRouterOrMeshConnect(bool valSet);
bool flgGet_gotRouterOrMeshConnect(void);

void devRouterRecordBssid_Set(uint8_t bssid[6], bool nvsRecord_IF);
void devRouterRecordBssid_Get(uint8_t bssid[6]);

void devRouterConnectBssid_Set(uint8_t bssid[6], bool nvsRecord_IF);
void devRouterConnectBssid_Get(uint8_t bssid[6]);

void usrApp_devIptdrv_paramRecalibration_set(bool reCalibra_if);
enum_touchCaliRsv usrApp_devIptdrv_paramRecalibration_get(void);

void usrAppHomepageBtnTextDisp_paramSet(stt_dataDisp_guiBussinessHome_btnText *param, bool nvsRecord_IF);
void usrAppHomepageBtnTextDisp_defaultLoad(devTypeDef_enum devType, bool nvsRecord_IF);
void usrAppHomepageBtnTextDisp_paramGet(stt_dataDisp_guiBussinessHome_btnText *param);
uint8_t countryFlgGetByAbbre(char *countryAbbre);

void usrAppHomepageBtnIconNumDisp_paramSet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM], bool nvsRecord_IF);
void usrAppHomepageBtnIconNumDisp_defaultLoad(devTypeDef_enum devType, bool nvsRecord_IF);
void usrAppHomepageBtnIconNumDisp_paramGet(uint8_t param[GUIBUSSINESS_CTRLOBJ_MAX_NUM]);
lv_img_dsc_t *usrAppHomepageBtnIconDisp_dataGet(uint8_t iconNum);

void devMeshSignalVal_Reales(void);
int8_t devMeshSignalFromParentVal_Get(void);
void devMeshNodeNum_Reales(void);
uint16_t devMeshNodeNum_Get(void);

void devMutualCtrlGroupInfo_Clr(void);
void devMutualCtrlGroupInfo_Set(stt_devMutualGroupParam *mutualGroupParam, uint8_t opreatBit, bool nvsRecord_IF);
void devMutualCtrlGroupInfo_Get(stt_devMutualGroupParam mutualGroupParam[DEVICE_MUTUAL_CTRL_GROUP_NUM]);
void devMutualCtrlGroupInfo_groupInsertGet(uint8_t groupInsert[DEVICE_MUTUAL_CTRL_GROUP_NUM]);
bool devMutualCtrlGroupInfo_unitCheckByInsert(stt_devMutualGroupParam *mutualGroupParamUnit, uint8_t *groupNum, uint8_t paramInsert);

void devSystemOpration_linkageConfig_paramSet(stt_paramLinkageConfig *param, bool nvsRecord_IF);
void devSystemOpration_linkageConfig_paramGet(stt_paramLinkageConfig *param);

uint16_t currentDevRunningFlg_paramGet(void);
void currentDevRunningFlg_paramSet(uint16_t valFlg, bool nvsRecord_IF);

void meshNetwork_connectReserve_IF_set(bool param);
bool meshNetwork_connectReserve_IF_get(void);

void mqttRemoteConnectCfg_paramSet(stt_mqttCfgParam *param, bool nvsRecord_IF);
void mqttRemoteConnectCfg_paramGet(stt_mqttCfgParam *param);

void currentRouterCfgInfo_paramSet(stt_routerCfgInfo *param, bool nvsRecord_IF);
void currentRouterCfgInfo_paramGet(stt_routerCfgInfo *param);

stt_blufiConfigDevInfo_resp *devBlufiConfig_respInfoGet(void);

void nvsDataOpreation_devInfraredParam_set(uint8_t irDats[DEVINFRARED_HXD019D_IIC_DATA_BUF_LEN], uint8_t cmdInsert);
stt_infraredSwitchData_nvsOpreat *nvsDataOpreation_devInfraredParam_get(uint8_t cmdInsert);

stt_scenarioSwitchData_nvsOpreat *nvsDataOpreation_devScenarioParam_get(uint8_t scenarioIst);

void devSystemInfoLocalRecord_preSaveTest(void);
void devSystemInfoLocalRecord_normalClear(void);
void devSystemInfoLocalRecord_allErase(void);
void devSystemInfoLocalRecord_initialize(void);
void devSystemInfoLocalRecord_save(enum_dataSaveObj obj, void *dataSave);

void L8devHbDataManageList_delSame(stt_nodeDev_hbDataManage *head);
uint8_t L8devHbDataManageList_nodeNumDetect(stt_nodeDev_hbDataManage *pHead);
uint8_t *L8devHbDataManageList_listGet(stt_nodeDev_hbDataManage *pHead);
stt_mutualCtrlInfoResp *L8devMutualCtrlInfo_Get(stt_nodeDev_hbDataManage *pHead, uint8_t mutualCtrlGroupIst);
uint8_t *L8DevListGet_Type1(stt_nodeDev_hbDataManage *pHead, uint8_t targetDevType);
uint8_t *L8devStatusInfoGet(stt_nodeDev_hbDataManage *pHead);
uint8_t *L8devElecsumInfoGet(stt_nodeDev_hbDataManage *pHead);
void L8devHbDataManageList_listDestory(stt_nodeDev_hbDataManage *pHead);
uint8_t L8devHbDataManageList_nodeCreat(stt_nodeDev_hbDataManage *pHead, stt_nodeDev_hbDataManage *pNew);
stt_nodeDev_hbDataManage *L8devHbDataManageList_nodeGet(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN], bool method);
bool L8devHbDataManageList_nodeRemove(stt_nodeDev_hbDataManage *pHead, uint8_t nodeDev_Mac[MWIFI_ADDR_LEN]);
void L8devHbDataManageList_bussinessKeepAliveManagePeriod1s(stt_nodeDev_hbDataManage *pHead);

void L8devHeartbeatFunctionParamLoad(stt_hbDataUpload *nodeDev_dataTemp);

bool usrAppMethod_mwifiMacAddrRemoveFromList(uint8_t *macAddrList, uint8_t macAddrList_num, uint8_t macAddrRemove[MWIFI_ADDR_LEN]);

void usrApplication_systemRestartTrig(uint8_t trigDelay);

void functionSpecialUsrApp_floatToHex(stt_devElecsumParam2Hex *param, float fData);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DEVDATAMANAGE_H*/




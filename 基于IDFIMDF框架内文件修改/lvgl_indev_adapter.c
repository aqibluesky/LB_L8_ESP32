// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* SDK Includes */
#include "sdkconfig.h"
#include "esp_log.h"

#ifdef CONFIG_LVGL_GUI_ENABLE

/*C Includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// /* freertos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"

/* I2C Includes */
#include "driver/i2c.h"
#include "iot_ft5x06.h"
#include "iot_i2c_bus.h"

/* lvgl includes */
#include "iot_lvgl.h"

/* lvgl calibration includes */
#include "lv_calibration.h"

/* FT5x06 Include */
#include "FT5x06.h"

/* usrApp Include */
#include "devDriver_manage.h"

extern EventGroupHandle_t xEventGp_screenTouch;
extern stt_touchEveInfo devTouchGetInfo;

static ft5x06_handle_t dev = NULL;

static void write_reg(uint8_t reg, uint8_t val)
{
    iot_ft5x06_write(dev, reg, 1, &val);
}

static uint8_t read_byte(uint8_t reg)
{
    uint8_t data;
    iot_ft5x06_read(dev, reg, 1, &data);
    return data;
}

static uint16_t read_word(uint8_t reg)
{
    uint8_t data[2];
    uint16_t result;
    iot_ft5x06_read(dev, reg, 2, &data);
    result = data[0] << 8 | data[1];
    return result;
}

/*Function pointer to read data. Return 'true' if there is still data to be read (buffered)*/
static bool ex_tp_read(lv_indev_data_t *data)
{
	uint8_t touchPointNum = 0;
	uint8_t touch_gestrue = 0;
    static lv_coord_t x = 0xFFFF, y = 0xFFFF;
	static lv_point_t pointRecord[3] = {0}; //-参数对应业务保留，逻辑触发失能
	static bool touchReales_flg = true;

	enum_screenBkLight_status bkLightStatus_temp = devScreenBkLight_brightnessGet();
	
    data->state = LV_INDEV_STATE_REL;
    // please be sure that your touch driver every time return old (last clcked) value. 
    data->point.x = x;
    data->point.y = y;

	touchPointNum = read_byte(FT5x06_TOUCH_POINTS);
	touch_gestrue = read_byte(FT5x06_GESTURE_ID); //手势寄存器暂时无效，应该是硬件不支持

	if(touch_gestrue){

		devTouchGetInfo.valGestrue = touch_gestrue;
		if(xEventGp_screenTouch)xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
	}

	// Only take a reading if we are touched.
	if (touchPointNum & 0x07){

		devScreenBkLight_weakUp();		
		if(bkLightStatus_temp != screenBkLight_statusEmpty){

			devTouchGetInfo.touchNum = touchPointNum;
			for(uint8_t loop = 0; loop < touchPointNum; loop ++){
	
				devTouchGetInfo.touchPosition[loop].x = (lv_coord_t)(read_word(FT5x06_TOUCH1_XH + 6 * loop) & 0x0fff);
				devTouchGetInfo.touchPosition[loop].y = (lv_coord_t)read_word(FT5x06_TOUCH1_YH + 6 * loop);
			}
			if(xEventGp_screenTouch)xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_TOUCHTRIG);
			
			/* Get the X, Y, values */
			data->point.x = (lv_coord_t)(read_word(FT5x06_TOUCH1_XH) & 0x0fff);
			data->point.y = (lv_coord_t)read_word(FT5x06_TOUCH1_YH);
			data->state = LV_INDEV_STATE_PR;
	
			// Apply calibration, rotation
			// Transform the co-ordinates
			if (lvgl_calibration_transform(&(data->point))) {
				lv_coord_t t;
				// Rescale X,Y if we are using self-calibration
#ifdef CONFIG_LVGL_DISP_ROTATE_0
					data->point.x = LV_HOR_RES - 1 - data->point.x;
					data->point.y = LV_VER_RES - 1 - data->point.y;
#elif defined(CONFIG_LVGL_DISP_ROTATE_90)
					t = data->point.x;
					data->point.x = LV_HOR_RES - 1 - data->point.y;
					data->point.y = t;
#elif defined(CONFIG_LVGL_DISP_ROTATE_180)
					data->point.x = LV_HOR_RES - 1 - data->point.x;
					data->point.y = LV_VER_RES - 1 - data->point.y;
#elif defined(CONFIG_LVGL_DISP_ROTATE_270)
					t = data->point.y;
					data->point.y = LV_VER_RES - 1 - data->point.x;
					data->point.x = t;
#endif
				x = data->point.x;
				y = data->point.y;
			}
			else //未进行校正业务
			{
//				x = data->point.x;
//				y = data->point.y;
//			
				data->point.x = LV_HOR_RES - 1 - data->point.x;
				data->point.y = LV_VER_RES - 1 - data->point.y;
			}

			//自定义手势解析 -业务保留，逻辑触发失能
//			if(memcmp(&pointRecord[0], &data->point, sizeof(lv_point_t))){

//				enum{

//					usrAppGuestrue_null = 0,
//					usrAppGuestrue_up,
//					usrAppGuestrue_down,
//					usrAppGuestrue_right,
//					usrAppGuestrue_left,
//					
//				}usrAppGuestrueDetect = usrAppGuestrue_null;

//				lv_coord_t diffVal_x = 0,
//					       diffVal_y = 0;

//				memcpy(&pointRecord[1], &pointRecord[0], sizeof(lv_point_t) * 2);
//				memcpy(&pointRecord[0], &data->point, sizeof(lv_point_t));

//				diffVal_x = pointRecord[1].x - pointRecord[0].x;
//				diffVal_y = pointRecord[1].y - pointRecord[0].y;

//				if(diffVal_x < 15 && diffVal_x > -15){ //预测

//					if((pointRecord[0].y - pointRecord[1].y) > 3){

//						usrAppGuestrueDetect = usrAppGuestrue_down;
//					}
//					else
//					if((pointRecord[0].y - pointRecord[1].y) < -3){

//						usrAppGuestrueDetect = usrAppGuestrue_up;
//					}
//				}
//				else
//				if(diffVal_y < 15 && diffVal_y > -15){
//				
//					if((pointRecord[0].x - pointRecord[1].x) > 3){
//					
//						usrAppGuestrueDetect = usrAppGuestrue_right;
//					}
//					else
//					if((pointRecord[0].x - pointRecord[1].x) < -3){
//					
//						usrAppGuestrueDetect = usrAppGuestrue_left;
//					}
//				}
//				else{

//					usrAppGuestrueDetect = usrAppGuestrue_null;
//				}

//				if(touchReales_flg){

//					switch(usrAppGuestrueDetect){ //精测

//						case usrAppGuestrue_down:{

//							if((pointRecord[1].y - pointRecord[2].y) > 3 ){

//							   touchReales_flg = false;
//							   memset(pointRecord, 0, sizeof(lv_point_t) * 3);

//							   devTouchGetInfo.valGestrue = FTIC_EXTRA_REGISTERVAL_GES_mDN;
//							   xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
//							}

//						}break;
//						
//						case usrAppGuestrue_up:{

//							if((pointRecord[1].y - pointRecord[2].y) < -3 ){

//							   touchReales_flg = false;
//							   memset(pointRecord, 0, sizeof(lv_point_t) * 3);
//							
//							   devTouchGetInfo.valGestrue = FTIC_EXTRA_REGISTERVAL_GES_mUP;
//							   xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
//							}

//						}break;
//						
//						case usrAppGuestrue_right:{

//							if((pointRecord[1].x - pointRecord[2].x) > 3 ){

//							   touchReales_flg = false;
//							   memset(pointRecord, 0, sizeof(lv_point_t) * 3);
//							
//							   devTouchGetInfo.valGestrue = FTIC_EXTRA_REGISTERVAL_GES_mRT;
//							   xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
//							}

//						}break;
//						
//						case usrAppGuestrue_left:{

//							if((pointRecord[1].x - pointRecord[2].x) < -3 ){

//							   	touchReales_flg = false;
//							   	memset(pointRecord, 0, sizeof(lv_point_t) * 3);
//							
//							   	devTouchGetInfo.valGestrue = FTIC_EXTRA_REGISTERVAL_GES_mLT;
//							   	if(xEventGp_screenTouch){

//									xEventGroupSetBits(xEventGp_screenTouch, TOUCHEVENT_FLG_BITHOLD_GESTRUEHAP);
//							   	}
//							}
//							
//						}break;
//						
//						default:break;
//					}
//				}
//			}
			
//			// Apply calibration, rotation --redefine by Lanbon
//			// Transform the co-ordinates
//			if (lvgl_calibration_transform(&(data->point))) {
//				
//				lv_coord_t t;
//				// Rescale X,Y if we are using self-calibration
//#ifdef defined(CONFIG_LVGL_DISP_ROTATE_0)
//				data->point.x = LV_HOR_RES - 1 - data->point.x;
//				data->point.y = LV_VER_RES - 1 - data->point.y;
//#elif defined(CONFIG_LVGL_DISP_ROTATE_90)
//				t = data->point.y;
//				data->point.y = LV_VER_RES - 1 - data->point.x;
//				data->point.x = t;
//#elif CONFIG_LVGL_DISP_ROTATE_180
//				data->point.x = data->point.x;
//				data->point.y = data->point.y;
//#elif defined(CONFIG_LVGL_DISP_ROTATE_270)
//				t = data->point.x;
//				data->point.x = LV_HOR_RES - 1 - data->point.y;
//				data->point.y = t;
//#endif
//				x = data->point.x;
//				y = data->point.y;
//			}
		}
	}
	else{

		touchReales_flg = true;
		memset(pointRecord, 0, sizeof(lv_point_t) * 3);
	}
		
    return false;
}

/* Input device interface */
/* Initialize your touchpad */
lv_indev_drv_t lvgl_indev_init()
{
    i2c_bus_handle_t i2c_bus = NULL;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_LVGL_TOUCH_SDA_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = CONFIG_LVGL_TOUCH_SCL_GPIO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 200000,
    };
    i2c_bus = iot_i2c_bus_create(CONFIG_LVGL_TOUCH_IIC_NUM, &conf);
    dev = iot_ft5x06_create(i2c_bus, FT5X06_ADDR_DEF);

    // Init default values. (From NHD-3.5-320240MF-ATXL-CTP-1 datasheet)
    // Valid touching detect threshold --def: 70
    write_reg(FT5x06_ID_G_THGROUP, 0x08);

    // valid touching peak detect threshold --def: 60
    write_reg(FT5x06_ID_G_THPEAK, 15);

    // Touch focus threshold --def: 16
    write_reg(FT5x06_ID_G_THCAL, 0xE9);

    // threshold when there is surface water --def: 60
    write_reg(FT5x06_ID_G_THWATER, 25);

    // threshold of temperature compensation --def: 10
    write_reg(FT5x06_ID_G_THTEMP, 20);

    // Touch difference threshold --def: 20
    write_reg(FT5x06_ID_G_THDIFF, 0x20);

    // Delay to enter 'Monitor' status (s) --def: 02
    write_reg(FT5x06_ID_G_TIME_ENTER_MONITOR, 0x02);

    // Period of 'Active' status (ms) --def: 12
    write_reg(FT5x06_ID_G_PERIODACTIVE, 0x03);

    // Timer to enter 'idle' when in 'Monitor' (ms) --def: 40
    write_reg(FT5x06_ID_G_PERIODMONITOR, 0x28);

    lv_indev_drv_t indev_drv;      /*Descriptor of an input device driver*/
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER; /*The touchpad is pointer type device*/
    indev_drv.read = ex_tp_read;            /*Library ready your touchpad via this function*/

    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
    return indev_drv;
}

#endif /* CONFIG_LVGL_GUI_ENABLE */

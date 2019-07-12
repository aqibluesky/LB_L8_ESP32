/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mdf_common.h"
#include "mwifi.h"
#include "mlink.h"
#include "mupgrade.h"

#include "mespnow.h"
#include "mconfig_blufi.h"
#include "mconfig_chain.h"

#include "os.h"

#include "tcpip_adapter.h"

//#include "light_driver.h"

#include "lcd_adapter.h"

#include "mqtt_client.h"

#include "QR_Encode.h"

/* lvgl includes */
#include "iot_lvgl.h"

/* lvgl test includes */
#include "lv_test_theme.h"

#include "devDriver_manage.h"

#include "devDriver_devSelfLight.h"
#include "bussiness_timerHard.h"
#include "bussiness_timerSoft.h"

#include "gui_businessHome.h"

#include "dataTrans_remoteServer.h"
#include "dataTrans_localHandler.h"

#define LIGHT_TID                     (1)
#define LIGHT_RESTART_COUNT_RESET     (3)
#define LIGHT_RESTART_TIMEOUT_MS      (3000)
#define LIGHT_STORE_RESTART_COUNT_KEY "light_count"

#define EVENT_GROUP_TRIGGER_HANDLE     BIT0
#define EVENT_GROUP_TRIGGER_RECV       BIT1

LV_IMG_DECLARE(testPic);
LV_IMG_DECLARE(testPic_P);
LV_IMG_DECLARE(ttA);
LV_IMG_DECLARE(night_Sight);

extern EventGroupHandle_t xEventGp_devApplication;

static const unsigned int dataSaveLen_test = 1024;

static struct
{
    lv_img_header_t header;
    uint32_t data_size;
    uint8_t * data;
	
}testImg_data = {

	.header.always_zero = 0,
	.header.w = 120,
	.header.h = 30,
	.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
	.data_size = GUI_BUSSINESS_HOME_BTNTEXT_PIC_PIXEL_SIZE * LV_IMG_PX_SIZE_ALPHA_BYTE,
	.data = NULL,
};

const uint8_t sssssT[960 * LV_IMG_PX_SIZE_ALPHA_BYTE] = {

	0
};
	
/**
 * @brief The value of the cid corresponding to each attribute of the light
 */
enum light_cid {
    LIGHT_CID_STATUS            = 0,
    LIGHT_CID_HUE               = 1,
    LIGHT_CID_SATURATION        = 2,
    LIGHT_CID_VALUE             = 3,
    LIGHT_CID_COLOR_TEMPERATURE = 4,
    LIGHT_CID_BRIGHTNESS        = 5,
    LIGHT_CID_MODE              = 6,
};

enum light_status {
    LIGHT_STATUS_ONOFF000              = 0,
    LIGHT_STATUS_ONOFF001              = 1,
	LIGHT_STATUS_ONOFF010			   = 2,
	LIGHT_STATUS_ONOFF011 			   = 3,
	LIGHT_STATUS_ONOFF100			   = 4,
	LIGHT_STATUS_ONOFF101			   = 5,
	LIGHT_STATUS_ONOFF110			   = 6,
	LIGHT_STATUS_ONOFF111			   = 7,

    LIGHT_STATUS_SWITCH            = 8,
    LIGHT_STATUS_HUE               = 9,
    LIGHT_STATUS_BRIGHTNESS        = 10,
    LIGHT_STATUS_COLOR_TEMPERATURE = 11,
};

static const char *TAG                          = "lanbon_L8 - usrMain";
static TaskHandle_t g_root_write_task_handle    = NULL;
static TaskHandle_t g_root_read_task_handle     = NULL;
static EventGroupHandle_t g_event_group_trigger = NULL;

/* LVGL Object */
static lv_obj_t *chart = NULL;
static lv_obj_t *gauge = NULL;
static lv_chart_series_t *series = NULL;

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init_custom(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "LANBON_DEVELOP002",
            .password = "Lanbon22*#",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
//    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void nvs_write_data_to_flash(void)
{
    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *DATA1 = "param 1";
    static const char *DATA2 = "param 2";
    static const char *DATA3 = "param 3";
	static const char *DATA4 = "image_data";
	static const char *DATA5 = "image_data1";

    int32_t value_for_store = 666;

    wifi_config_t wifi_config_to_store = {
        .sta = {
            .ssid = "store_ssid:hello_kitty",
            .password = "store_password:1234567890",
        },
    };

	ESP_ERROR_CHECK( nvs_flash_init_partition("L8_devDataRcord"));
    printf("set size:%u\r\n", sizeof(wifi_config_to_store));
    ESP_ERROR_CHECK( nvs_open_from_partition("L8_devDataRcord", NVS_CUSTOMER, NVS_READWRITE, &handle) );
    ESP_ERROR_CHECK( nvs_set_str( handle, DATA1, "i am a string.") );
    ESP_ERROR_CHECK( nvs_set_i32( handle, DATA2, value_for_store) );
    ESP_ERROR_CHECK( nvs_set_blob( handle, DATA3, &wifi_config_to_store, sizeof(wifi_config_to_store)) );
	ESP_ERROR_CHECK( nvs_set_blob( handle, DATA4, ttA.data, sizeof(uint8_t) * 960 * LV_IMG_PX_SIZE_ALPHA_BYTE) );
//	ESP_ERROR_CHECK( nvs_set_blob( handle, DATA5, &(testPic.data[2400]), sizeof(uint8_t) * 2400) )

    ESP_ERROR_CHECK( nvs_commit(handle) );
	nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition("L8_devDataRcord"));

}

void nvs_read_data_from_flash(void)
{
    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *DATA1 = "param 1";
    static const char *DATA2 = "param 2";
    static const char *DATA3 = "param 3";
	static const char *DATA4 = "image_data";
	static const char *DATA5 = "image_data1";

    uint32_t str_length = 32;
    char str_data[32] = {0};
    int32_t value = 0;
    wifi_config_t wifi_config_stored;
    memset(&wifi_config_stored, 0x0, sizeof(wifi_config_stored));
    uint32_t len = sizeof(wifi_config_stored);

	uint32_t dataLen_test = sizeof(uint8_t) * 960 * LV_IMG_PX_SIZE_ALPHA_BYTE;

	testImg_data.data = (uint8_t *)os_zalloc(dataLen_test);

//	memcpy(testImg_data.data, ttA.data, 960 * LV_IMG_PX_SIZE_ALPHA_BYTE * sizeof(uint8_t));

	lv_obj_t * wp = lv_img_create(lv_scr_act(), NULL);

	ESP_ERROR_CHECK( nvs_flash_init_partition("L8_devDataRcord"));

    ESP_ERROR_CHECK( nvs_open_from_partition("L8_devDataRcord", NVS_CUSTOMER, NVS_READWRITE, &handle) );

    ESP_ERROR_CHECK ( nvs_get_str(handle, DATA1, str_data, &str_length) );
    ESP_ERROR_CHECK ( nvs_get_i32(handle, DATA2, &value) );
    ESP_ERROR_CHECK ( nvs_get_blob(handle, DATA3, &wifi_config_stored, &len) );
	
	ESP_ERROR_CHECK ( nvs_get_blob(handle, DATA4, testImg_data.data, &dataLen_test) );

    printf("[data1]: %s len:%u\r\n", str_data, str_length);
    printf("[data2]: %d\r\n", value);
    printf("[data3]: ssid:%s passwd:%s\r\n", wifi_config_stored.sta.ssid, wifi_config_stored.sta.password);
	
    nvs_close(handle);
	ESP_ERROR_CHECK( nvs_flash_deinit_partition("L8_devDataRcord"));

//	memcpy(sssssT, ttA.data, 960 * LV_IMG_PX_SIZE_ALPHA_BYTE * sizeof(uint8_t));

//	memcpy(testImg_data.data, ttA.data, 960 * LV_IMG_PX_SIZE_ALPHA_BYTE * sizeof(uint8_t));

//	testImg_data.data = ttA.data;

	lv_img_set_src(wp, (lv_img_dsc_t *)&testImg_data);
	lv_obj_set_pos(wp, 0, 10);
}

static void littlevgl_usrTest(void){

//	nvs_write_data_to_flash();
//	nvs_read_data_from_flash();

	lvGui_businessInit();
}

static lv_res_t on_led_switch_toggled(lv_obj_t *sw)
{
    ESP_LOGI(TAG, "Hello");
    return LV_RES_OK;
}

static void littlevgl_demo(void)
{
    lv_obj_t *scr = lv_obj_create(NULL, NULL);
    lv_scr_load(scr);

    lv_theme_t *th = lv_theme_alien_init(100, NULL);
    lv_theme_set_current(th);

    lv_obj_t *tabview = lv_tabview_create(lv_scr_act(), NULL);

    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, SYMBOL_LOOP);
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, SYMBOL_HOME);
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, SYMBOL_SETTINGS);
    lv_tabview_set_tab_act(tabview, 1, false);

    chart = lv_chart_create(tab2, NULL);
    lv_obj_set_size(chart, 300, 150);
    lv_chart_set_point_count(chart, 20);
    lv_obj_align(chart, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(chart, (lv_chart_type_t)(LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE));
    lv_chart_set_series_opa(chart, LV_OPA_70);
    lv_chart_set_series_width(chart, 4);
    lv_chart_set_range(chart, 0, 100);
    series = lv_chart_add_series(chart, LV_COLOR_RED);

    static lv_color_t needle_colors[] = {LV_COLOR_RED};
    gauge = lv_gauge_create(tab1, NULL);
    lv_gauge_set_needle_count(gauge,
                              sizeof(needle_colors) / sizeof(needle_colors[0]), needle_colors);
    lv_obj_align(gauge, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_gauge_set_value(gauge, 0, 50);

    char name[10];
    int i;
    lv_obj_t *labels[3];
    lv_obj_t *switches[3];
    for (i = 0; i < 3; i++)
    {
        labels[i] = lv_label_create(tab3, NULL);
        sprintf(name, "LED%d", i + 1);
        lv_label_set_text(labels[i], name);
    }
    lv_obj_align(labels[0], NULL, LV_ALIGN_IN_TOP_MID, -40, 20);
    for (i = 1; i < 3; i++)
    {
        lv_obj_align(labels[i], labels[i - 1], LV_ALIGN_OUT_BOTTOM_MID, 0, 35);
    }
    for (i = 0; i < 3; i++)
    {
        switches[i] = lv_sw_create(tab3, NULL);
        lv_obj_align(switches[i], labels[i], LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        lv_sw_set_action(switches[i], on_led_switch_toggled);
    }
}

static mdf_err_t wifi_init()
{
    mdf_err_t ret          = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    tcpip_adapter_init();
    MDF_ERROR_ASSERT(esp_event_loop_init(NULL, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief Timed printing system information
 */
static void show_system_info_timercb(void *timer)
{
    uint8_t primary                 = 0;
    wifi_second_chan_t second       = 0;
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    mesh_assoc_t mesh_assoc         = {0x0};
    wifi_sta_list_t wifi_sta_list   = {0x0};
	uint8_t mutualGroupInsert_temp[DEVICE_MUTUAL_CTRL_GROUP_NUM] = {0};
//	lv_mem_monitor_t lv_mon			= {0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_wifi_vnd_mesh_get(&mesh_assoc);
    esp_mesh_get_parent_bssid(&parent_bssid);
	devMutualCtrlGroupInfo_groupInsertGet(mutualGroupInsert_temp);
//	lv_mem_monitor(&lv_mon);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: " MACSTR ", parent bssid: " MACSTR
             ", parent rssi: %d, node num: %d, free heap: %u, mutualGroupInsert:[%d]-[%d]-[%d].\n", 
             primary,
             esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
             mesh_assoc.rssi, esp_mesh_get_total_node_num(), esp_get_free_heap_size(),
             mutualGroupInsert_temp[0], mutualGroupInsert_temp[1], mutualGroupInsert_temp[2]);

//	MDF_LOGI("lv mem used:%6d(%3d%%), lv mem biggest free:%6d.\n",
//			 (int)lv_mon.total_size - (int)lv_mon.free_size,
//			 lv_mon.used_pct,
//			 (int)lv_mon.free_biggest_size);

    for (int i = 0; i < wifi_sta_list.num; i++) {
        MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
    }

#ifdef CONFIG_LIGHT_MEMORY_DEBUG

    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
#endif /**< CONFIG_LIGHT_MEMORY_DEBUG */
}

static void restart_count_erase_timercb(void *timer)
{
    if (!xTimerStop(timer, portMAX_DELAY)) {
        MDF_LOGE("xTimerStop timer: %p", timer);
    }

    if (!xTimerDelete(timer, portMAX_DELAY)) {
        MDF_LOGE("xTimerDelete timer: %p", timer);
    }

    mdf_info_erase(LIGHT_STORE_RESTART_COUNT_KEY);
    MDF_LOGD("Erase restart count");
}

static int restart_count_get()
{
    mdf_err_t ret             = MDF_OK;
    TimerHandle_t timer       = NULL;
    uint32_t restart_count    = 0;
    RESET_REASON reset_reason = rtc_get_reset_reason(0);

    mdf_info_load(LIGHT_STORE_RESTART_COUNT_KEY, &restart_count, sizeof(uint32_t));

    if (reset_reason != POWERON_RESET && reset_reason != RTCWDT_RTC_RESET) {
        restart_count = 0;
        MDF_LOGW("restart reason: %d", reset_reason);
    }

    /**< If the device restarts within the instruction time,
         the event_mdoe value will be incremented by one */
    restart_count++;
    ret = mdf_info_save(LIGHT_STORE_RESTART_COUNT_KEY, &restart_count, sizeof(uint32_t));
    MDF_ERROR_CHECK(ret != ESP_OK, ret, "Save the number of restarts within the set time");

    timer = xTimerCreate("restart_count_erase", LIGHT_RESTART_TIMEOUT_MS / portTICK_RATE_MS,
                         false, NULL, restart_count_erase_timercb);
    MDF_ERROR_CHECK(!timer, ret, "xTaskCreate, timer: %p", timer);

    xTimerStart(timer, 0);

    return restart_count;
}

static bool light_restart_is_exception()
{
    mdf_err_t ret                      = ESP_OK;
    ssize_t coredump_len               = 0;
    esp_partition_iterator_t part_itra = NULL;

    part_itra = esp_partition_find(ESP_PARTITION_TYPE_DATA,
                                   ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL);
    MDF_ERROR_CHECK(!part_itra, false, "<%s> esp_partition_find fail", mdf_err_to_name(ret));

    const esp_partition_t *coredump_part = esp_partition_get(part_itra);
    MDF_ERROR_CHECK(!coredump_part, false, "<%s> esp_partition_get fail", mdf_err_to_name(ret));

    ret = esp_partition_read(coredump_part, sizeof(ssize_t), &coredump_len, sizeof(ssize_t));
    MDF_ERROR_CHECK(ret, false, "<%s> esp_partition_read fail", mdf_err_to_name(ret));

    if (coredump_len <= 0) {
        return false;
    }

    /**< erase all coredump partition */
    ret = esp_partition_erase_range(coredump_part, 0, coredump_part->size);
    MDF_ERROR_CHECK(ret, false, "<%s> esp_partition_erase_range fail", mdf_err_to_name(ret));

    return true;
}

static mdf_err_t get_network_config(mwifi_init_config_t *init_config, mwifi_config_t *ap_config)
{
    MDF_PARAM_CHECK(init_config);
    MDF_PARAM_CHECK(ap_config);

    mconfig_data_t *mconfig_data        = NULL;
    mconfig_blufi_config_t blufi_config = {
        .company_id = 0x02E5, /**< Espressif Incorporated */
        .tid        = LIGHT_TID,
    };

    MDF_ERROR_ASSERT(mconfig_chain_slave_init());

    /**
     * @brief Switch to master mode to configure the network for other devices
     */
    uint8_t sta_mac[6] = {0};
    MDF_ERROR_ASSERT(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac));
    sprintf(blufi_config.name, "LB8_%02X%02X%02X%02X%02X%02X", sta_mac[0], 
															   sta_mac[1],
															   sta_mac[2],
															   sta_mac[3],
															   sta_mac[4],
															   sta_mac[5]);
    MDF_LOGI("BLE name: %s", blufi_config.name);

    MDF_ERROR_ASSERT(mconfig_blufi_init(&blufi_config));
    MDF_ERROR_ASSERT(mconfig_queue_read(&mconfig_data, portMAX_DELAY));
    MDF_ERROR_ASSERT(mconfig_chain_slave_deinit());
    MDF_ERROR_ASSERT(mconfig_blufi_deinit());

    memcpy(ap_config, &mconfig_data->config, sizeof(mwifi_config_t));
    memcpy(init_config, &mconfig_data->init_config, sizeof(mwifi_init_config_t));

    mdf_info_save("init_config", init_config, sizeof(mwifi_init_config_t));
    mdf_info_save("ap_config", ap_config, sizeof(mwifi_config_t));

    /**
     * @brief Switch to master mode to configure the network for other devices
     */
    if (mconfig_data->whitelist_size > 0) {
        for (int i = 0; i < mconfig_data->whitelist_size / sizeof(mconfig_whitelist_t); ++i) {
            MDF_LOGD("count: %d, data: " MACSTR,
                     i, MAC2STR((uint8_t *)mconfig_data->whitelist_data + i * sizeof(mconfig_whitelist_t)));
        }

        MDF_ERROR_ASSERT(mconfig_chain_master(mconfig_data, 60000 / portTICK_RATE_MS));
    }

    MDF_FREE(mconfig_data);

    return MDF_OK;
}

static mdf_err_t mlink_set_value(uint16_t cid, void *arg)
{
    int value = *((int *)arg);

    switch (cid) {
        case LIGHT_CID_STATUS:
			
            switch (value) {
				case  LIGHT_STATUS_ONOFF000:
				case  LIGHT_STATUS_ONOFF001:
				case  LIGHT_STATUS_ONOFF010:
				case  LIGHT_STATUS_ONOFF011:		
				case  LIGHT_STATUS_ONOFF100:		
				case  LIGHT_STATUS_ONOFF101:			 
				case  LIGHT_STATUS_ONOFF110:			
				case  LIGHT_STATUS_ONOFF111:{			

					stt_devDataPonitTypedef dataVal_set = {0};

					dataVal_set.devType_mulitSwitch_threeBit.swVal_bit1 = (uint8_t)value >> 0;
					dataVal_set.devType_mulitSwitch_threeBit.swVal_bit2 = (uint8_t)value >> 1;
					dataVal_set.devType_mulitSwitch_threeBit.swVal_bit3 = (uint8_t)value >> 2;
					currentDev_dataPointSet(&dataVal_set, true, true, true);

//					light_driver_set_switch(value);

				}break;

//                case LIGHT_STATUS_SWITCH:
//                    light_driver_set_switch(!light_driver_get_switch());
//                    break;

//                case LIGHT_STATUS_HUE: {
//                    uint16_t hue = light_driver_get_hue();
//                    hue = (hue + 60) % 360;

//                    light_driver_set_saturation(100);
//                    light_driver_set_hue(hue);
//                    break;
//                }

//                case LIGHT_STATUS_BRIGHTNESS: {
//                    if (light_driver_get_mode() == MODE_HSV) {
//                        uint8_t value = (light_driver_get_value() + 20) % 100;
//                        light_driver_set_value(value);
//                    } else {
//                        uint8_t brightness = (light_driver_get_brightness() + 20) % 100;
//                        light_driver_set_brightness(brightness);
//                    }

//                    break;
//                }

//                case LIGHT_STATUS_COLOR_TEMPERATURE: {
//                    uint8_t color_temperature = (light_driver_get_color_temperature() + 20) % 100;

//                    if (!light_driver_get_brightness()) {
//                        light_driver_set_brightness(30);
//                    }

//                    light_driver_set_color_temperature(color_temperature);

//                    break;
//                }

                default:
                    break;
            }

            break;

//        case LIGHT_CID_MODE:
//            switch (value) {
//                case MODE_BRIGHTNESS_INCREASE:
//                    light_driver_fade_brightness(100);
//                    break;

//                case MODE_BRIGHTNESS_DECREASE:
//                    light_driver_fade_brightness(0);
//                    break;

//                case MODE_HUE_INCREASE:
//                    light_driver_set_saturation(100);
//                    light_driver_fade_hue(360);
//                    break;

//                case MODE_HUE_DECREASE:
//                    light_driver_set_saturation(100);
//                    light_driver_fade_hue(0);
//                    break;

//                case MODE_WARM_INCREASE:
//                    if (!light_driver_get_brightness()) {
//                        light_driver_set_brightness(30);
//                    }

//                    light_driver_fade_warm(100);
//                    break;

//                case MODE_WARM_DECREASE:
//                    if (!light_driver_get_brightness()) {
//                        light_driver_set_brightness(30);
//                    }

//                    light_driver_fade_warm(0);
//                    break;

//                case MODE_NONE:
//                    light_driver_fade_stop();
//                    break;

//                default:
//                    break;
//            }

//            break;

//        case LIGHT_CID_HUE:
//            light_driver_set_hue(value);
//            break;

//        case LIGHT_CID_SATURATION:
//            light_driver_set_saturation(value);
//            break;

//        case LIGHT_CID_VALUE:
//            light_driver_set_value(value);
//            break;

//        case LIGHT_CID_COLOR_TEMPERATURE:
//            light_driver_set_color_temperature(value);
//            break;

//        case LIGHT_CID_BRIGHTNESS:
//            light_driver_set_brightness(value);
//            break;

        default:
            MDF_LOGE("No support cid: %d", cid);
            return MDF_FAIL;
    }

    MDF_LOGD("cid: %d, value: %d", cid, value);

    return MDF_OK;
}

static mdf_err_t mlink_get_value(uint16_t cid, void *arg)
{
//    int *value = (int *)arg;

//    switch (cid) {
//        case LIGHT_CID_STATUS:
//            *value = light_driver_get_switch();
//            break;

//        case LIGHT_CID_HUE:
//            *value = light_driver_get_hue();
//            break;

//        case LIGHT_CID_SATURATION:
//            *value = light_driver_get_saturation();
//            break;

//        case LIGHT_CID_VALUE:
//            *value = light_driver_get_value();
//            break;

//        case LIGHT_CID_COLOR_TEMPERATURE:
//            *value = light_driver_get_color_temperature();
//            break;

//        case LIGHT_CID_BRIGHTNESS:
//            *value = light_driver_get_brightness();
//            break;

//        case LIGHT_CID_MODE:
//            *value = light_driver_get_mode();
//            break;

//        default:
//            MDF_LOGE("No support cid: %d", cid);
//            return MDF_FAIL;
//    }

//    MDF_LOGV("cid: %d, value: %d", cid, *value);

    return MDF_OK;
}

static void root_write_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = NULL;
    size_t size   = 0;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type      = {0};
	mlink_httpd_type_t *httpd_type 	 = NULL;

    MDF_LOGI("root_write_task is running");

    while (mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT) {
        size = MWIFI_PAYLOAD_LEN * 4;
        MDF_FREE(data);
        ret = mwifi_root_read(src_addr, &data_type, &data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_read", mdf_err_to_name(ret));

        if (data_type.upgrade) {
            ret = mupgrade_root_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_handle", mdf_err_to_name(ret));
            continue;
        }

		httpd_type = (mlink_httpd_type_t *)&(data_type.custom);
//		MDF_LOGD("root receive dataType:%d.\n", httpd_type->format);
		if(httpd_type->format == MLINK_HTTPD_FORMAT_HEX){

			uint8_t *dataRcv_temp = (uint8_t *)data;

			if(dataRcv_temp[0] == L8DEV_MESH_HEARTBEAT_REQ){ //心跳截留，非心跳转发

				dataHandler_devNodeMeshData(src_addr, httpd_type, data, size);
			}
			else
			{
				mqtt_remoteDataTrans(dataRcv_temp[0], &dataRcv_temp[1], size - 1);
			}

			continue;
		}

        MDF_LOGD("Root receive, addr: " MACSTR ", size: %d, data: %.*s",
                 MAC2STR(src_addr), size, size, data);

        switch (data_type.protocol) {
            case MLINK_PROTO_HTTPD: {
                mlink_httpd_t httpd_data  = {
                    .size       = size,
                    .data       = data,
                    .addrs_num  = 1,
                    .addrs_list = src_addr,
                };
                memcpy(&httpd_data.type, &data_type.custom, sizeof(httpd_data.type));

                ret = mlink_httpd_write(&httpd_data, portMAX_DELAY);
                MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_write", mdf_err_to_name(ret));

                break;
            }

            case MLINK_PROTO_NOTICE: {
                ret = mlink_notice_write(data, size, src_addr);
                MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_write", mdf_err_to_name(ret));
                break;
            }

            default:
                MDF_LOGW("Does not support the protocol: %d", data_type.protocol);
                break;
        }
    }

    MDF_LOGW("root_write_task is exit");

    MDF_FREE(data);
    g_root_write_task_handle = NULL;
    vTaskDelete(NULL);
}

static void root_read_task(void *arg)
{
    mdf_err_t ret               = MDF_OK;
    mlink_httpd_t *httpd_data   = NULL;
    mwifi_data_type_t data_type = {
        .compression = true,
        .communicate = MWIFI_COMMUNICATE_MULTICAST,
    };

    MDF_LOGI("root_read_task is running");

    while (mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT) {
        if (httpd_data) {
            MDF_FREE(httpd_data->addrs_list);
            MDF_FREE(httpd_data->data);
            MDF_FREE(httpd_data);
        }

        ret = mlink_httpd_read(&httpd_data, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK || !httpd_data, "<%s> mwifi_root_read", mdf_err_to_name(ret));
        MDF_LOGD("Root receive, addrs_num: %d, addrs_list: " MACSTR ", size: %d, data: %.*s",
                 httpd_data->addrs_num, MAC2STR(httpd_data->addrs_list),
                 httpd_data->size, httpd_data->size, httpd_data->data);

        memcpy(&data_type.custom, &httpd_data->type, sizeof(mlink_httpd_type_t));
        ret = mwifi_root_write(httpd_data->addrs_list, httpd_data->addrs_num,
                               &data_type, httpd_data->data, httpd_data->size, true);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_write", mdf_err_to_name(ret));
    }

    MDF_LOGW("root_read_task is exit");

    if (httpd_data) {
        MDF_FREE(httpd_data->addrs_list);
        MDF_FREE(httpd_data->data);
        MDF_FREE(httpd_data);
    }

    g_root_read_task_handle = NULL;
    vTaskDelete(NULL);
}

void request_handle_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    uint8_t *data = NULL;
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        MDF_FREE(data);
        ret = mwifi_read(src_addr, &data_type, &data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> Receive a packet targeted to self over the mesh network",
                           mdf_err_to_name(ret));

        if (data_type.upgrade) {
            ret = mupgrade_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_handle", mdf_err_to_name(ret));

            continue;
        }

        MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %.*s", MAC2STR(src_addr), size, size, data);

        mlink_httpd_type_t *httpd_type = (mlink_httpd_type_t *)&data_type.custom;

		if(httpd_type->format == MLINK_HTTPD_FORMAT_HEX){

			dataHandler_devNodeMeshData(src_addr, httpd_type, data, size);
		}
		else
		if(httpd_type->format ==  MLINK_HTTPD_FORMAT_JSON){

	        ret = mlink_handle(src_addr, httpd_type, data, size);
	        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mlink_handle", mdf_err_to_name(ret));			
		}

        if (httpd_type->from == MLINK_HTTPD_FROM_DEVICE) {
            data_type.protocol = MLINK_PROTO_NOTICE;
            ret = mwifi_write(NULL, &data_type, "status", strlen("status"), true);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mlink_handle", mdf_err_to_name(ret));
        }
    }

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event_loop_cb, event: 0x%x", event);
    mdf_err_t ret = MDF_OK;

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:

			flgSet_gotRouterOrMeshConnect(true);
		
            MDF_LOGI("Parent is connected on station interface");
//            light_driver_breath_stop();
            break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:

			flgSet_gotRouterOrMeshConnect(false);
		
            MDF_LOGI("Parent is disconnected on station interface");
            break;

        case MDF_EVENT_MWIFI_FIND_NETWORK: {
            MDF_LOGI("the root connects to another router with the same SSID");
            mwifi_config_t ap_config  = {0x0};
            wifi_second_chan_t second = 0;

            mdf_info_load("ap_config", &ap_config, sizeof(mwifi_config_t));
            esp_wifi_get_channel(&ap_config.channel, &second);
            mwifi_set_config(&ap_config);
            mdf_info_save("ap_config", &ap_config, sizeof(mwifi_config_t));
            break;
        }

        case MDF_EVENT_MWIFI_ROUTING_TABLE_ADD:
        case MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE: {
            MDF_LOGI("total_num: %d", esp_mesh_get_total_node_num());

            uint8_t sta_mac[MWIFI_ADDR_LEN] = {0x0};
            MDF_ERROR_ASSERT(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac));

            ret = mlink_notice_write("http", strlen("http"), sta_mac);
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_write", mdf_err_to_name(ret));
            break;
        }

        case MDF_EVENT_MWIFI_ROOT_GOT_IP: {
            MDF_LOGI("Root obtains the IP address");

	        tcpip_adapter_ip_info_t ip_info;
	        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
			MDF_LOGI("<MESH_EVENT_ROOT_GOT_IP>sta ip: " IPSTR ", mask: " IPSTR ", gw: " IPSTR,
			IP2STR(&ip_info.ip),
			IP2STR(&ip_info.netmask),
			IP2STR(&ip_info.gw));

			meshNetwork_connectReserve_IF_set(true);
			flgSet_gotRouterOrMeshConnect(true);

            ret = mlink_notice_init();
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_notice_init", mdf_err_to_name(ret));

            uint8_t sta_mac[MWIFI_ADDR_LEN] = {0x0};
            MDF_ERROR_ASSERT(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac));

            ret = mlink_notice_write("http", strlen("http"), sta_mac);
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_write", mdf_err_to_name(ret));

            ret = mlink_httpd_start();
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_start", mdf_err_to_name(ret));

            if (!g_root_write_task_handle) {
                xTaskCreate(root_write_task, "root_write", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, &g_root_write_task_handle);
            }

            if (!g_root_read_task_handle) {
                xTaskCreate(root_read_task, "root_read", 8 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, &g_root_read_task_handle);
            }

			mqtt_app_start();

            break;
        }

        case MDF_EVENT_MWIFI_ROOT_LOST_IP:
            MDF_LOGI("Root loses the IP address");

			flgSet_gotRouterOrMeshConnect(false);

			mqtt_app_stop();

            ret = mlink_notice_deinit();
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_notice_deinit", mdf_err_to_name(ret));

            ret = mlink_httpd_stop();
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mlink_httpd_stop", mdf_err_to_name(ret));
            break;

        case MDF_EVENT_MCONFIG_BLUFI_STA_DISCONNECTED:
//            light_driver_breath_start(128, 128, 0); /**< yellow blink */
            break;

        case MDF_EVENT_MCONFIG_BLUFI_STA_CONNECTED:
//            light_driver_breath_start(255, 128, 0); /**< orange blink */
            break;

        case MDF_EVENT_MCONFIG_BLUFI_FINISH:
        case MDF_EVENT_MCONFIG_CHAIN_FINISH:
//            light_driver_breath_start(0, 255, 0); /**< green blink */
            break;

        case MDF_EVENT_MUPGRADE_STARTED:
            MDF_LOGI("Enter upgrade mode");
//            light_driver_breath_start(0, 0, 128); /**< blue blink */
            vTaskDelay(3000 / portTICK_RATE_MS);
//            light_driver_breath_stop();
            break;

        case MDF_EVENT_MUPGRADE_STATUS: {
            MDF_LOGI("The upgrade progress is: %d%%", (int)ctx);
            mwifi_data_type_t data_type = {
                .protocol = MLINK_PROTO_NOTICE,
            };
            ret = mwifi_write(NULL, &data_type, "ota_status", strlen("ota_status"), true);
            MDF_ERROR_BREAK(ret != MDF_OK, "<%s> mwifi_write", esp_err_to_name(ret));
            break;
        }

        case MDF_EVENT_MUPGRADE_FINISH:
            MDF_LOGI("Upgrade completed waiting for restart");
//            light_driver_breath_start(0, 0, 255); /**< blue blink */
            break;

        case MDF_EVENT_MLINK_SYSTEM_RESET:
            MDF_LOGW("Erase information saved in flash and system restart");

            ret = mdf_info_erase(MDF_SPACE_NAME);
            MDF_ERROR_BREAK(ret != 0, "Erase the information");

            esp_restart();
            break;

        case MDF_EVENT_MLINK_SYSTEM_REBOOT:
            MDF_LOGW("Restart PRO and APP CPUs");
            esp_restart();
            break;

        case MDF_EVENT_MLINK_SET_STATUS:
            if (!g_event_group_trigger) {
                g_event_group_trigger = xEventGroupCreate();
            }

            xEventGroupSetBits(g_event_group_trigger, EVENT_GROUP_TRIGGER_HANDLE);
            break;

        case MDF_EVENT_MESPNOW_RECV:
            if ((int)ctx == MESPNOW_TRANS_PIPE_CONTROL) {
                xEventGroupSetBits(g_event_group_trigger, EVENT_GROUP_TRIGGER_RECV);
            }

            break;

        default:
            break;
    }

    return MDF_OK;
}

static void trigger_handle_task(void *arg)
{
    mdf_err_t ret = MDF_OK;

    if (!g_event_group_trigger) {
        g_event_group_trigger = xEventGroupCreate();
    }

    for (;;) {
		
        EventBits_t uxBits = xEventGroupWaitBits(g_event_group_trigger,
                             					 EVENT_GROUP_TRIGGER_RECV | EVENT_GROUP_TRIGGER_HANDLE,
                             					 pdTRUE, 
                             					 pdFALSE, 
                             					 2 / portTICK_RATE_MS);

        if (uxBits & EVENT_GROUP_TRIGGER_RECV) {
            uint8_t *data       = NULL;
            uint8_t *addrs_list = NULL;
            size_t addrs_num    = 0;
            size_t size         = 0;
            mwifi_data_type_t data_type = {
                .protocol = MLINK_PROTO_HTTPD,
            };

            mlink_httpd_type_t httpd_type = {
                .format = MLINK_HTTPD_FORMAT_JSON,
                .from   = MLINK_HTTPD_FROM_DEVICE,
                .resp   = false,
            };

            memcpy(&data_type.custom, &httpd_type, sizeof(mlink_httpd_type_t));

            ret = mlink_espnow_read(&addrs_list, &addrs_num, &data, &size, portMAX_DELAY);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mlink_espnow_read", esp_err_to_name(ret));

            MDF_LOGI("Mlink espnow read data: %.*s", size, data);

            for (int i = 0; i < addrs_num; ++i) {
                ret = mwifi_write(addrs_list  + 6 * i, &data_type, data, size, true);
                MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_write", esp_err_to_name(ret));
            }

            MDF_FREE(data);
            MDF_FREE(addrs_list);
        }

        if (uxBits & EVENT_GROUP_TRIGGER_HANDLE) {
            ret = mlink_trigger_handle(MLINK_COMMUNICATE_MESH);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mlink_trigger_handle", mdf_err_to_name(ret));
        }

		uxBits = xEventGroupWaitBits(xEventGp_devApplication, 
									 DEVAPPLICATION_FLG_BITHOLD_RESERVE,
									 pdTRUE,
									 pdFALSE,
									 2 / portTICK_RATE_MS);

		if(uxBits & DEVAPPLICATION_FLG_BITHOLD_HEARTBEAT){

//			ESP_LOGI(TAG, "heartbeat trig.");

			devHeartbeat_dataTrans_bussinessTrig();
			vTaskDelay(20 / portTICK_RATE_MS);
		}

		if(uxBits & DEVAPPLICATION_FLG_BITHOLD_MUTUALTRIG){

//			ESP_LOGI(TAG, "mutualCtrl trig.");

			funcation_usrAppMutualCtrlActionTrig(); 
			vTaskDelay(20 / portTICK_RATE_MS);
		}

		if(uxBits & DEVAPPLICATION_FLG_BITHOLD_DEVEKECSUM_REPORT){

//			ESP_LOGI(TAG, "decElecsum info report trig.");

//			mqtt_rootDevRemoteDatatransLoop_elecSumReport();
			vTaskDelay(20 / portTICK_RATE_MS);
		}

		if(uxBits & DEVAPPLICATION_FLG_BITHOLD_DEVDRV_SCENARIO){

			ESP_LOGI(TAG, "scenario driver action trig.");

			devDriverBussiness_scnarioSwitch_actionTrig();
			vTaskDelay(20 / portTICK_RATE_MS);
		}

		vTaskDelay(2 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

static void developer_debugTask(void *arg){

//    gpio_config_t io_conf;
//	
//    //disable interrupt
//    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//    //set as output mode
//    io_conf.mode = GPIO_MODE_OUTPUT;
//    //disable pull-down mode
//    io_conf.pull_down_en = 0;
//    //disable pull-up mode
//    io_conf.pull_up_en = 0;
//    //bit mask of the pins that you want to set
//    io_conf.pin_bit_mask = GPIO_SEL_16 | GPIO_SEL_17 | GPIO_SEL_32;
//    //configure GPIO with the given settings
//    gpio_config(&io_conf);

//    //set as output mode
//    io_conf.mode = GPIO_MODE_INPUT;
//    //bit mask of the pins that you want to set
//    io_conf.pin_bit_mask = GPIO_SEL_26;
//    //configure GPIO with the given settings
//    gpio_config(&io_conf);

//	for(;;){

//		if(gpio_get_level(GPIO_NUM_26)){

//			gpio_set_level(GPIO_NUM_16, 0);
//			gpio_set_level(GPIO_NUM_17, 0);
//			gpio_set_level(GPIO_NUM_32, 0);

//		}else{

//			gpio_set_level(GPIO_NUM_16, 1);
//			gpio_set_level(GPIO_NUM_17, 1);
//			gpio_set_level(GPIO_NUM_32, 1);
//		}

//		vTaskDelay(100 / portTICK_PERIOD_MS);

//		gpio_set_level(GPIO_NUM_16, 0);
//		gpio_set_level(GPIO_NUM_17, 0);
//		gpio_set_level(GPIO_NUM_32, 0);
//		vTaskDelay(500 / portTICK_PERIOD_MS);

//		gpio_set_level(GPIO_NUM_16, 1);
//		gpio_set_level(GPIO_NUM_17, 1);
//		gpio_set_level(GPIO_NUM_32, 1);
//		vTaskDelay(500 / portTICK_PERIOD_MS);
//	}

//	deviceHardwareLight_Init();
//	bussiness_devLight_testApp();

	vTaskDelete(NULL);
}

void app_main()
{
    char name[32]                       = {0};
    uint8_t sta_mac[6]                  = {0};
    mwifi_config_t ap_config            = {0x0};
    mwifi_init_config_t init_config     = {0x0};
//    light_driver_config_t driver_config = {
//        .gpio_red        = CONFIG_LIGHT_GPIO_RED,
//        .gpio_green      = CONFIG_LIGHT_GPIO_GREEN,
//        .gpio_blue       = CONFIG_LIGHT_GPIO_BLUE,
//        .gpio_cold       = CONFIG_LIGHT_GPIO_COLD,
//        .gpio_warm       = CONFIG_LIGHT_GPIO_WARM,
//        .fade_period_ms  = CONFIG_LIGHT_FADE_PERIOD_MS,
//        .blink_period_ms = CONFIG_LIGHT_BLINK_PERIOD_MS,
//    };

//	devSystemInfoLocalRecord_preSaveTest();
	devSystemInfoLocalRecord_initialize();

	deviceHardwareLight_Init();
	devDriverManageBussiness_initialition();
	usrApp_bussinessHardTimer_Init();
	usrApp_bussinessSoftTimer_Init();

	/* Initialize LittlevGL GUI */
	lvgl_init();

	littlevgl_usrTest();

    TimerHandle_t timer = xTimerCreate("show_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, show_system_info_timercb);

    xTimerStart(timer, 0);

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

//    nvs_flash_init();
//    wifi_init_custom();
//    mqtt_app_start();

    /**
     * @brief Initialize wifi
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mespnow_init());

    /**
     * @brief Light driver initialization
     */
//    MDF_ERROR_ASSERT(light_driver_init(&driver_config));

    /**
     * @brief Continuous power off and restart more than three times to reset the device
     */
    if (restart_count_get() >= LIGHT_RESTART_COUNT_RESET) {
        MDF_LOGW("Erase information saved in flash");
        mdf_info_erase(MDF_SPACE_NAME);
    }

    /**
     * @brief Indicate the status of the device by means of a light
     */
    if (mdf_info_load("init_config", &init_config, sizeof(mwifi_init_config_t)) == MDF_OK
            && mdf_info_load("ap_config", &ap_config, sizeof(mwifi_config_t)) == MDF_OK) {
            
        if (light_restart_is_exception()) {
//            light_driver_breath_start(255, 0, 0); /**< red blink */
        } else {
//            light_driver_set_switch(true);
        }

		meshNetwork_connectReserve_IF_set(true);
		
    } else {
//        light_driver_breath_start(255, 255, 0); /**< yellow blink */
        MDF_ERROR_ASSERT(get_network_config(&init_config, &ap_config));
        MDF_LOGI("mconfig, ssid: %s, password: %s, mesh_id: " MACSTR,
                 ap_config.router_ssid, ap_config.router_password,
                 MAC2STR(ap_config.mesh_id));

		meshNetwork_connectReserve_IF_set(false);
    }

    /**
     * @brief Note that once BT controller memory is released, the process cannot be reversed.
     *        It means you can not use the bluetooth mode which you have released by this function.
     *        it can release the .bss, .data and other section to heap
     */
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    /**
     * @brief Configure MLink (LAN communication module)
     */
    MDF_ERROR_ASSERT(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac));
    snprintf(name, sizeof(name), "light_%02x%02x", sta_mac[4], sta_mac[5]);
    MDF_ERROR_ASSERT(mlink_add_device(LIGHT_TID, name, CONFIG_LIGHT_VERSION));
    MDF_ERROR_ASSERT(mlink_add_characteristic(0, "on", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 3, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(1, "hue", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 360, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(2, "saturation", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 100, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(3, "value", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 100, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(4, "color_temperature", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 100, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(5, "brightness", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RWT, 0, 100, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic(6, "mode", CHARACTERISTIC_FORMAT_INT, CHARACTERISTIC_PERMS_RW, 1, 3, 1));
    MDF_ERROR_ASSERT(mlink_add_characteristic_handle(mlink_get_value, mlink_set_value));

    mlink_trigger_init();
    xTaskCreate(trigger_handle_task, "trigger_handle", 1024 * 8,  NULL, 4, NULL);

    /**
     * @brief Initialize esp-mesh
     */
    MDF_ERROR_ASSERT(mwifi_init(&init_config));
    MDF_ERROR_ASSERT(mwifi_set_config(&ap_config));
    MDF_ERROR_ASSERT(mwifi_start());

	xTaskCreate(developer_debugTask, "developer_debugTask", 1024 * 4, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    /**
     * @brief Data transfer between wifi mesh devices
     */
    xTaskCreate(request_handle_task, "request_handle", 8 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
}

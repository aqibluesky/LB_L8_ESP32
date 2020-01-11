deps_config := \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/app_trace/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/aws_iot/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/bt/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/driver/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/efuse/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp32/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp_adc_cal/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp_event/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp_http_client/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp_http_server/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esp_https_ota/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/espcoredump/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/ethernet/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/fatfs/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/freemodbus/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/freertos/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/heap/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/i2c_devices/i2c_bus/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/spi_devices/lcd/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/libsodium/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/log/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/hmi/lvgl_gui/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/lwip/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/mbedtls/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/mcommon/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/mconfig/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/mdns/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/mespnow/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/mqtt/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/mupgrade/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/components/mwifi/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/nvs_flash/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/openssl/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/pthread/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/spi_flash/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/spiffs/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/tcpip_adapter/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/unity/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/vfs/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/wear_levelling/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/wifi_provisioning/Kconfig \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/app_update/Kconfig.projbuild \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/bootloader/Kconfig.projbuild \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/esptool_py/Kconfig.projbuild \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/examples/lanbon_L8/main/Kconfig.projbuild \
	C:/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/components/partition_table/Kconfig.projbuild \
	/c/usr_Space/WORK/prj_workSpace/esp-mdf/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_TARGET)" "esp32"
include/config/auto.conf: FORCE
endif
ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;

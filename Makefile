#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := $(notdir $(shell pwd))
EXTRA_COMPONENT_DIRS += $(shell pwd)/light_driver
EXTRA_COMPONENT_DIRS += $(shell pwd)/GUI_bussiness
EXTRA_COMPONENT_DIRS += $(shell pwd)/GUI_material
EXTRA_COMPONENT_DIRS += $(shell pwd)/devicesDriver
EXTRA_COMPONENT_DIRS += $(shell pwd)/dataTrans
EXTRA_COMPONENT_DIRS += $(shell pwd)/Actuator
EXTRA_COMPONENT_DIRS += $(shell pwd)/QRCODE

include $(MDF_PATH)/project.mk

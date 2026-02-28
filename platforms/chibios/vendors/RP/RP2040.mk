#
# Raspberry Pi RP2040 specific drivers
##############################################################################
COMMON_VPATH += $(PLATFORM_PATH)/$(PLATFORM_KEY)/$(DRIVER_DIR)/vendor/$(MCU_FAMILY)/$(MCU_SERIES)

ifeq ($(strip $(WS2812_DRIVER)), vendor)
    OPT_DEFS += -DRP_DMA_REQUIRED=TRUE
endif

#
# ChibiOS RP2040 startup configuration
##############################################################################
ADEFS  += -DCRT0_VTOR_INIT=1 \
		  -DCRT0_EXTRA_CORES_NUMBER=0 \
          -DCRT0_INIT_VECTORS=1

CFLAGS += -DNDEBUG

#
# Shared ChibiOS-Contrib drivers (I2C, PWM, ADC) and vendor includes
##############################################################################
include $(PLATFORM_PATH)/$(PLATFORM_KEY)/vendors/$(MCU_FAMILY)/_common.mk

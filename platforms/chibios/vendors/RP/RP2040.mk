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
# ChibiOS-Contrib drivers not yet in trunk: I2C and PWM
##############################################################################
PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1/hal_i2c_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1

PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1/hal_pwm_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1

PLATFORM_RP2040_PATH := $(PLATFORM_PATH)/$(PLATFORM_KEY)/vendors/$(MCU_FAMILY)
EXTRAINCDIRS += $(PLATFORM_RP2040_PATH)

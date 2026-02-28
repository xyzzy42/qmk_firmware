#
# Shared ChibiOS-Contrib drivers for RP2040 and RP2350
##############################################################################

# ChibiOS-Contrib drivers not yet in trunk: I2C, PWM, and ADC
PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1/hal_i2c_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1

PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1/hal_pwm_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1

PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/ADCv1/hal_adc_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/ADCv1

# Shared vendor include path (for _pin_defs.h, etc.)
PLATFORM_RP_VENDOR_PATH := $(PLATFORM_PATH)/$(PLATFORM_KEY)/vendors/$(MCU_FAMILY)
EXTRAINCDIRS += $(PLATFORM_RP_VENDOR_PATH)

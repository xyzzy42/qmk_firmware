#
# Raspberry Pi RP2350 specific drivers
##############################################################################
COMMON_VPATH += $(PLATFORM_PATH)/$(PLATFORM_KEY)/$(DRIVER_DIR)/vendor/$(MCU_FAMILY)/$(MCU_SERIES)

ifeq ($(strip $(WS2812_DRIVER)), vendor)
    OPT_DEFS += -DRP_DMA_REQUIRED=TRUE
endif

#
# ChibiOS RP2350 startup configuration
##############################################################################
# RP2350 uses the ARMv8-M-ML port with imagedef for boot
ADEFS  += -DCRT0_VTOR_INIT=1 \
          -DCRT0_EXTRA_CORES_NUMBER=0 \
          -DCRT0_INIT_VECTORS=1

# RP2350-specific defines required for ARMv8-M-ML port compatibility
# - CORTEX_SIMPLIFIED_PRIORITY: Use PendSV instead of SVC for context switch
# - CH_DBG_ENABLE_STACK_CHECK: Save/restore PSPLIM during context switch
# These are required by ChibiOS startup_rp2350.mk
OPT_DEFS += -DCORTEX_SIMPLIFIED_PRIORITY=TRUE -DCH_DBG_ENABLE_STACK_CHECK=TRUE
ADEFS    += -DCORTEX_SIMPLIFIED_PRIORITY=TRUE -DCH_DBG_ENABLE_STACK_CHECK=TRUE

# Enable RCP (Redundancy Coprocessor) at CP7 in addition to FPU at CP10/CP11
# Default CRT0_CPACR_INIT is 0x00F00000 (FPU only)
# 0x00F0C000 = FPU (bits 20-23) + RCP (bits 14-15)
OPT_DEFS += -DCRT0_CPACR_INIT=0x00F0C000
ADEFS    += -DCRT0_CPACR_INIT=0x00F0C000

CFLAGS += -DNDEBUG \
          -Wno-error=discarded-qualifiers \
          -Wno-error=array-bounds

#
# ChibiOS-Contrib drivers not yet in trunk: I2C and PWM
##############################################################################
PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1/hal_i2c_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/I2Cv1

PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1/hal_pwm_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/PWMv1

PLATFORM_RP2350_PATH := $(PLATFORM_PATH)/$(PLATFORM_KEY)/vendors/$(MCU_FAMILY)
EXTRAINCDIRS += $(PLATFORM_RP2350_PATH)

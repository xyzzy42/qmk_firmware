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

CFLAGS += -DPICO_NO_FPGA_CHECK \
          -DNDEBUG \
          -Wno-error=discarded-qualifiers \
          -Wno-error=array-bounds

#
# ChibiOS provides native RP2350 HAL drivers (clocks, pll, xosc, gpio)
# and pico_bindings for minimal Pico SDK API compatibility
##############################################################################
EXTRAINCDIRS += $(CHIBIOS)/os/various/pico_bindings/dumb/include

#
# USB driver from chibios-contrib (required for keyboard operation)
# USBDv1 works for both RP2040 and RP2350 since they have the same USB controller.
# The rp2040_usb.h header provides hardware register definitions.
##############################################################################
PLATFORM_SRC += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/USBDv1/hal_usb_lld.c
EXTRAINCDIRS += $(CHIBIOS_CONTRIB)/os/hal/ports/RP/LLD/USBDv1

#
# RP2350 does not require Pico SDK - ChibiOS provides all necessary drivers
# Flash operations use ChibiOS HAL or bootrom direct access
##############################################################################

PLATFORM_RP2350_PATH := $(PLATFORM_PATH)/$(PLATFORM_KEY)/vendors/$(MCU_FAMILY)

PLATFORM_SRC += $(PLATFORM_RP2350_PATH)/pico_sdk_shims.c

# ChibiOS 8.0 hal_safety.c is included via hal.mk (non-smart build includes it)
# so we don't need to add it here explicitly

EXTRAINCDIRS += $(PLATFORM_RP2350_PATH)

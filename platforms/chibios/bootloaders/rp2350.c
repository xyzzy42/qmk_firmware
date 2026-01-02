// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "hal.h"
#include "bootloader.h"
#include "gpio.h"
#include "wait.h"

#if !defined(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED)
#    define RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK 0U
#else
#    define RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK (1U << RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED)
#endif

// RP2350 bootrom functions
// These are accessed via the bootrom lookup table
typedef void (*reset_usb_boot_fn)(uint32_t gpio_activity_pin_mask, uint32_t disable_interface_mask);

// RP2350 ROM table addresses - the ROM stores a pointer to the function table at this address
#define RP2350_ROM_TABLE_LOOKUP_ADDR 0x00000014

// Helper to read a value from a fixed ROM address, avoiding compiler warnings
static inline uint32_t rom_read32(uint32_t addr) {
    uint32_t result;
    __asm__ volatile ("ldr %0, [%1]" : "=r" (result) : "r" (addr));
    return result;
}

static reset_usb_boot_fn get_reset_usb_boot(void) {
    // RP2350 bootrom function table lookup
    // The bootrom functions are at fixed addresses in the RP2350
    // Function code for reset_usb_boot is 'UB' (0x5542)
    uint32_t table_addr = rom_read32(RP2350_ROM_TABLE_LOOKUP_ADDR);
    uint16_t *lookup = (uint16_t *)table_addr;

    // Search for the function
    while (*lookup != 0) {
        if (*lookup == 0x5542) {  // 'UB' = USB boot
            return (reset_usb_boot_fn)(*(uint32_t *)(lookup + 1));
        }
        lookup += 3;  // Each entry is 6 bytes
    }
    return NULL;
}

__attribute__((weak)) void mcu_reset(void) {
    NVIC_SystemReset();
}

void bootloader_jump(void) {
    reset_usb_boot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK, 0U);
    }
    // If we can't find the function, fall back to system reset
    NVIC_SystemReset();
}

void enter_bootloader_mode_if_requested(void) {}

#if defined(RP2350_BOOTLOADER_DOUBLE_TAP_RESET)
#    if !defined(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT)
#        define RP2350_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
#    endif

// Needs to be located in a RAM section that is never initialized on boot to
// preserve its value on reset
static volatile uint32_t __attribute__((section(".ram0.bootloader_magic"))) magic_location;
const uint32_t                                                              magic_token = 0xCAFEB0BA;

// Enable DWT cycle counter for timing (needed before chSysInit runs)
static void enable_dwt_cyccnt(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// We can not use the __early_init / enter_bootloader_mode_if_requested hook as
// we depend on an already initialized system with usable memory regions and
// populated function pointer tables to the optimized math functions in the
// bootrom. This function is called just prior to main.
void __late_init(void) {
    // All clocks have to be enabled before jumping to the bootloader function,
    // otherwise the bootrom will be stuck infinitely.
    // Use ChibiOS clock initialization
    rp_clock_init();

    // Enable DWT cycle counter - required for wait_us() on Cortex-M33
    // (normally enabled in chSysInit, but we run before main())
    enable_dwt_cyccnt();

    if (magic_location != magic_token) {
        magic_location = magic_token;
        // ChibiOS is not initialized at this point, so sleeping is only
        // possible via busy waiting.
        wait_us(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT * 1000U);
        magic_location = 0;
        return;
    }

    magic_location = 0;
    reset_usb_boot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK, 0U);
    }
}

#endif

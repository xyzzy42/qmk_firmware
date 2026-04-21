// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "hal.h"
#include "bootloader.h"
#include "gpio.h"
#include "wait.h"

#if defined(RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED)
#    define BOOTLOADER_FLAGS    RP_ROM_BOOTSEL_GPIO_PIN_SPECIFIED
#    define BOOTLOADER_LED      RP2350_BOOTLOADER_DOUBLE_TAP_RESET_LED
#else
#    define BOOTLOADER_FLAGS    0
#    define BOOTLOADER_LED      0 /* not used */
#endif

// RP2350 bootrom lookup function
typedef void *(*rom_table_lookup_fn)(uint32_t code, uint32_t mask);
// This function is looked up via the above function
typedef void (*reboot_fn)(uint32_t flags, uint32_t delay_ms, uint32_t p0, uint32_t p1);

// RP2350 ROM table addresses - the ROM stores a pointer to the function table at this address
#define RP2350_ROM_TABLE_LOOKUP_ADDR 0x00000014
// The ROM stores a pointer here to a function that looks up pointers the above table
#define RP2350_ROM_TABLE_LOOKUP_FUNC 0x00000016

static reboot_fn get_reset_usb_boot(void) {
    // GCC doesn't know the ROM table exists, so flags this as an out of bounds array access
    _Pragma("GCC diagnostic push");
    _Pragma("GCC diagnostic ignored \"-Warray-bounds\"");
    uint16_t lookup_func_addr = *(const uint16_t *)RP2350_ROM_TABLE_LOOKUP_FUNC;
    _Pragma("GCC diagnostic pop");

    // RP2350 bootrom function lookup
    rom_table_lookup_fn lookup = (const void *)(uintptr_t)lookup_func_addr;

    // Gets reboot function or returns NULL on error
    return lookup(RP_ROM_FUNC_REBOOT, RP_ROM_RT_FLAG_FUNC_ARM_NONSEC);
}

__attribute__((weak)) void mcu_reset(void) {
    NVIC_SystemReset();
}

void bootloader_jump(void) {
    reboot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP_ROM_REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL | RP_ROM_REBOOT2_FLAG_NO_RETURN_ON_SUCCESS, 0, BOOTLOADER_FLAGS, BOOTLOADER_LED);
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

    reboot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP_ROM_REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL, 0, BOOTLOADER_FLAGS, BOOTLOADER_LED);
    }
}

#endif

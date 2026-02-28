// Copyright 2022 Stefan Kerkmann
// SPDX-License-Identifier: GPL-2.0-or-later

#include "hal.h"
#include "bootloader.h"
#include "gpio.h"
#include "wait.h"

#if !defined(RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED)
#    define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK 0U
#else
#    define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK (1U << RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED)
#endif

// RP2040 bootrom functions - accessed via ROM table lookup
typedef void (*reset_usb_boot_fn)(uint32_t gpio_activity_pin_mask, uint32_t disable_interface_mask);

// RP2040 ROM table addresses (16-bit pointers stored in ROM)
#define RP2040_ROM_FUNC_TABLE_ADDR   0x00000014
#define RP2040_ROM_TABLE_LOOKUP_ADDR 0x00000018

// Helper to read a 16-bit value from a fixed ROM address
static inline uint16_t rom_read16(uint32_t addr) {
    uint16_t result;
    __asm__ volatile ("ldrh %0, [%1]" : "=r" (result) : "r" (addr));
    return result;
}

static reset_usb_boot_fn get_reset_usb_boot(void) {
    // RP2040 bootrom stores 16-bit pointers at fixed addresses:
    // 0x14: pointer to function lookup table
    // 0x18: pointer to rom_table_lookup helper function
    typedef void *(*rom_table_lookup_fn)(uint16_t *table, uint32_t code);

    uint16_t *func_table = (uint16_t *)(uint32_t)rom_read16(RP2040_ROM_FUNC_TABLE_ADDR);
    rom_table_lookup_fn rom_table_lookup = (rom_table_lookup_fn)(uint32_t)rom_read16(RP2040_ROM_TABLE_LOOKUP_ADDR);

    // Function code for reset_usb_boot: 'UB' encoded as per RP2040 bootrom spec
    return (reset_usb_boot_fn)rom_table_lookup(func_table, 'U' | ('B' << 8));
}

__attribute__((weak)) void mcu_reset(void) {
    NVIC_SystemReset();
}

void bootloader_jump(void) {
    reset_usb_boot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK, 0U);
    }
    // If we can't find the function, fall back to system reset
    NVIC_SystemReset();
}

void enter_bootloader_mode_if_requested(void) {}

#if defined(RP2040_BOOTLOADER_DOUBLE_TAP_RESET)
#    if !defined(RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT)
#        define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
#    endif

// Needs to be located in a RAM section that is never initialized on boot to
// preserve its value on reset
static volatile uint32_t __attribute__((section(".ram0.bootloader_magic"))) magic_location;
const uint32_t                                                              magic_token = 0xCAFEB0BA;

// We can not use the __early_init / enter_bootloader_mode_if_requested hook as
// we depend on an already initialized system with usable memory regions and
// populated function pointer tables to the optimized math functions in the
// bootrom. This function is called just prior to main.
void __late_init(void) {
    // All clocks have to be enabled before jumping to the bootloader function,
    // otherwise the bootrom will be stuck infinitely.
    rp_clock_init();

    if (magic_location != magic_token) {
        magic_location = magic_token;
        // ChibiOS is not initialized at this point, so sleeping is only
        // possible via busy waiting.
        wait_us(RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT * 1000U);
        magic_location = 0;
        return;
    }

    magic_location = 0;
    reset_usb_boot_fn reset_usb_boot = get_reset_usb_boot();
    if (reset_usb_boot) {
        reset_usb_boot(RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED_MASK, 0U);
    }
}

#endif

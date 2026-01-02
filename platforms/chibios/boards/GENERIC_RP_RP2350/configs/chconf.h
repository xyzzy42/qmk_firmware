// Copyright 2025 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// RP2350 single-core mode (ARMv8-M-ML port doesn't support SMP yet)
#define CH_CFG_SMP_MODE                     FALSE
#define CH_CFG_ST_RESOLUTION                32
#define CH_CFG_ST_FREQUENCY                 1000000
#define CH_CFG_INTERVALS_SIZE               32
#define CH_CFG_TIME_TYPES_SIZE              32
#define CH_CFG_ST_TIMEDELTA                 20

#include_next <chconf.h>

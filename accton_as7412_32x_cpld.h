/*
 * Accton AS7412-32X CPLD Platform Definitions
 *
 * Copyright 2016 Cumulus Networks, Inc.
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 */

#ifndef ACCTON_7412_CPLD_H__
#define ACCTON_7412_CPLD_H__

/* the as7412 actually has two CPLDs: main and fan
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed
 * into each register definition. When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                        (6)
#define MAIN_CPLD_ID                          (2)
#define FAN_CPLD_ID                           (1)
#define CPLD_IDX_MASK                         (3 << CPLD_IDX_SHIFT)
#define MAIN_CPLD_IDX                         (MAIN_CPLD_ID << CPLD_IDX_SHIFT)
#define FAN_CPLD_IDX                          (FAN_CPLD_ID << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS                  (2)

#define GET_CPLD_IDX(A)                       (((A & CPLD_IDX_MASK) >>\
CPLD_IDX_SHIFT) - 1)
#define STRIP_CPLD_IDX(A)                     (A & ~CPLD_IDX_MASK)

#define MAIN_CPLD_REG(A)                      (A | MAIN_CPLD_IDX)
#define FAN_CPLD_REG(A)                       (A | FAN_CPLD_IDX)

#define ACT7412_CPLD_VERSION_REG              MAIN_CPLD_REG(0x00)

#define ACT7412_CPLD_PSU_STATUS_REG           MAIN_CPLD_REG(0x02)
  #define ACT7412_PSU1_MASK                   (0x2a)
  #define ACT7412_PSU2_MASK                   (0x15)
  #define ACT7412_PSU2_PRESENT_L              BIT(0)
  #define ACT7412_PSU1_PRESENT_L              BIT(1)
  #define ACT7412_PSU2_POWER_GOOD             BIT(2)
  #define ACT7412_PSU1_POWER_GOOD             BIT(3)
  #define ACT7412_PSU2_ALERT_L                BIT(4)
  #define ACT7412_PSU1_ALERT_L                BIT(5)

#define ACT7412_CPLD_FAN_PRESENT_REG          FAN_CPLD_REG(0x0f)
  #define ACT7412_FAN_TRAY1_PRESENT_L         BIT(0)
  #define ACT7412_FAN_TRAY2_PRESENT_L         BIT(1)
  #define ACT7412_FAN_TRAY3_PRESENT_L         BIT(2)
  #define ACT7412_FAN_TRAY4_PRESENT_L         BIT(3)
  #define ACT7412_FAN_TRAY5_PRESENT_L         BIT(4)
  #define ACT7412_FAN_TRAY6_PRESENT_L         BIT(5)

#define ACT7412_CPLD_PWM_REG                  FAN_CPLD_REG(0x11)
  #define ACT7412_FAN_PWM_MULTIPLIER          (17)

#define ACT7412_CPLD_FAN1_SPEED_REG           FAN_CPLD_REG(0x12)
#define ACT7412_CPLD_FAN3_SPEED_REG           FAN_CPLD_REG(0x13)
#define ACT7412_CPLD_FAN5_SPEED_REG           FAN_CPLD_REG(0x14)
#define ACT7412_CPLD_FAN7_SPEED_REG           FAN_CPLD_REG(0x15)
#define ACT7412_CPLD_FAN9_SPEED_REG           FAN_CPLD_REG(0x16)
#define ACT7412_CPLD_FAN11_SPEED_REG          FAN_CPLD_REG(0x17)
#define ACT7412_CPLD_FAN2_SPEED_REG           FAN_CPLD_REG(0x22)
#define ACT7412_CPLD_FAN4_SPEED_REG           FAN_CPLD_REG(0x23)
#define ACT7412_CPLD_FAN6_SPEED_REG           FAN_CPLD_REG(0x24)
#define ACT7412_CPLD_FAN8_SPEED_REG           FAN_CPLD_REG(0x25)
#define ACT7412_CPLD_FAN10_SPEED_REG          FAN_CPLD_REG(0x26)
#define ACT7412_CPLD_FAN12_SPEED_REG          FAN_CPLD_REG(0x27)
  #define ACT7412_FAN_SPEED_MULTIPLIER        (100)

#define ACT7412_CPLD_FAN1_FAN4_LED_REG        FAN_CPLD_REG(0x1c)
  #define ACT7412_FAN4_GREEN_LED              BIT(0)
  #define ACT7412_FAN4_RED_LED                BIT(1)
  #define ACT7412_FAN3_GREEN_LED              BIT(2)
  #define ACT7412_FAN3_RED_LED                BIT(3)
  #define ACT7412_FAN2_GREEN_LED              BIT(4)
  #define ACT7412_FAN2_RED_LED                BIT(5)
  #define ACT7412_FAN1_GREEN_LED              BIT(6)
  #define ACT7412_FAN1_RED_LED                BIT(7)

#define ACT7412_CPLD_FAN5_FAN6_LED_REG        FAN_CPLD_REG(0x1d)
  #define ACT7412_FAN6_GREEN_LED              BIT(0)
  #define ACT7412_FAN6_RED_LED                BIT(1)
  #define ACT7412_FAN5_GREEN_LED              BIT(2)
  #define ACT7412_FAN5_RED_LED                BIT(3)

#define ACT7412_CPLD_FAN_WATCHDOG_REG         FAN_CPLD_REG(0x33)
  #define ACT7412_FAN_WATCHDOG_ENABLE         BIT(0)

#endif  /* ACCTON_7412_CPLD_H__ */


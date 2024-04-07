/*
 * Accton AS7312-54X CPLD Platform Definitions
 *
 * Copyright 2017 Cumulus Networks, Inc.
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

#ifndef ACCTON_7312_CPLD_H__
#define ACCTON_7312_CPLD_H__

/* the as7312 has four CPLDs
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed
 * into each register definition. When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                        (8)
#define CPLD1_ID                              (1)
#define CPLD2_ID                              (2)
#define CPLD3_ID                              (3)
#define CPLD4_ID                              (0)
#define CPLD_IDX_MASK                         (3 << CPLD_IDX_SHIFT)
#define CPLD1_IDX                             (CPLD1_ID << CPLD_IDX_SHIFT)
#define CPLD2_IDX                             (CPLD2_ID << CPLD_IDX_SHIFT)
#define CPLD3_IDX                             (CPLD3_ID << CPLD_IDX_SHIFT)
#define CPLD4_IDX                             (CPLD4_ID << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS                  (4)

#define GET_CPLD_IDX(A)                       ((A & CPLD_IDX_MASK) >>\
CPLD_IDX_SHIFT)
#define STRIP_CPLD_IDX(A)                     (A & ~CPLD_IDX_MASK)

#define CPLD1_REG(A)                          (A | CPLD1_IDX)
#define CPLD2_REG(A)                          (A | CPLD2_IDX)
#define CPLD3_REG(A)                          (A | CPLD3_IDX)
#define CPLD4_REG(A)                          (A | CPLD4_IDX)

/*
 * CPLD 1 register definitions
 */
#define ACT7312_BOARD_INFO_REG                CPLD1_REG(0x00)

#define ACT7312_CPLD1_VERSION_REG             CPLD1_REG(0x01)

#define ACT7312_PSU_STATUS_REG                CPLD1_REG(0x02)
  #define CPLD_PSU1_MASK                      (0x15)
  #define CPLD_PSU1_ALERT_L                   BIT(4)
  #define CPLD_PSU1_POWER_GOOD                BIT(2)
  #define CPLD_PSU1_PRESENT_L                 BIT(0)
  #define CPLD_PSU2_MASK                      (0x2a)
  #define CPLD_PSU2_ALERT_L                   BIT(5)
  #define CPLD_PSU2_POWER_GOOD                BIT(3)
  #define CPLD_PSU2_PRESENT_L                 BIT(1)

#define ACT7312_SYS_LED1_REG                  CPLD1_REG(0x41)
  #define ACT7312_LOC_LED_AMBER               (0)
  #define ACT7312_LOC_LED_OFF                 BIT(7)
  #define ACT7312_LOC_LED_MASK                BIT(7)
  #define ACT7312_DIAG_LED_MASK               (BIT(1) | BIT(0))
  #define ACT7312_DIAG_LED_AMBER              (0)
  #define ACT7312_DIAG_LED_RED                BIT(0)
  #define ACT7312_DIAG_LED_GREEN              BIT(1)
  #define ACT7312_DIAG_LED_OFF                (BIT(1) | BIT(0))

#define ACT7312_SYS_LED_RED_REG               CPLD1_REG(0x43)
#define ACT7312_SYS_LED_GREEN_REG             CPLD1_REG(0x44)
#define ACT7312_SYS_LED_BLUE_REG              CPLD1_REG(0x45)

/*
 * CPLD 2 register definitions
 */
#define ACT7312_CPLD2_VERSION_REG             CPLD2_REG(0x01)

#define ACT7312_SFP_1_8_PRESENT_REG           CPLD2_REG(0x09)
#define ACT7312_SFP_9_16_PRESENT_REG          CPLD2_REG(0x0a)
#define ACT7312_SFP_17_24_PRESENT_REG         CPLD2_REG(0x0b)

#define ACT7312_SFP_1_8_TX_FAULT_REG          CPLD2_REG(0x0c)
#define ACT7312_SFP_9_16_TX_FAULT_REG         CPLD2_REG(0x0d)
#define ACT7312_SFP_17_24_TX_FAULT_REG        CPLD2_REG(0x0e)

#define ACT7312_SFP_1_8_TX_DISABLE_REG        CPLD2_REG(0x0f)
#define ACT7312_SFP_9_16_TX_DISABLE_REG       CPLD2_REG(0x10)
#define ACT7312_SFP_17_24_TX_DISABLE_REG      CPLD2_REG(0x11)

#define ACT7312_SFP_1_8_RX_LOSS_REG           CPLD2_REG(0x12)
#define ACT7312_SFP_9_16_RX_LOSS_REG          CPLD2_REG(0x13)
#define ACT7312_SFP_17_24_RX_LOSS_REG         CPLD2_REG(0x14)

#define ACT7312_QSFP_49_52_RESET_REG          CPLD2_REG(0x17)
#define ACT7312_QSFP_49_52_PRESENT_REG        CPLD2_REG(0x18)

#define ACT7312_100G_LED_GREEN1_REG           CPLD2_REG(0x20)
#define ACT7312_100G_LED_RED1_REG             CPLD2_REG(0x21)
#define ACT7312_100G_LED_BLUE1_REG            CPLD2_REG(0x22)

#define ACT7312_25G_LED_GREEN1_REG            CPLD2_REG(0x23)
#define ACT7312_25G_LED_RED1_REG              CPLD2_REG(0x24)
#define ACT7312_25G_LED_BLUE1_REG             CPLD2_REG(0x25)

#define ACT7312_40G_LED_GREEN1_REG            CPLD2_REG(0x26)
#define ACT7312_40G_LED_RED1_REG              CPLD2_REG(0x27)
#define ACT7312_40G_LED_BLUE1_REG             CPLD2_REG(0x28)

#define ACT7312_10G_LED_GREEN1_REG            CPLD2_REG(0x29)
#define ACT7312_10G_LED_RED1_REG              CPLD2_REG(0x2a)
#define ACT7312_10G_LED_BLUE1_REG             CPLD2_REG(0x2b)

/*
 * CPLD 3 register definitions
 */
#define ACT7312_CPLD3_VERSION_REG             CPLD3_REG(0x01)

#define ACT7312_SFP_25_32_PRESENT_REG         CPLD3_REG(0x09)
#define ACT7312_SFP_33_40_PRESENT_REG         CPLD3_REG(0x0a)
#define ACT7312_SFP_41_48_PRESENT_REG         CPLD3_REG(0x0b)

#define ACT7312_SFP_25_32_TX_FAULT_REG        CPLD3_REG(0x0c)
#define ACT7312_SFP_33_40_TX_FAULT_REG        CPLD3_REG(0x0d)
#define ACT7312_SFP_41_48_TX_FAULT_REG        CPLD3_REG(0x0e)

#define ACT7312_SFP_25_32_TX_DISABLE_REG      CPLD3_REG(0x0f)
#define ACT7312_SFP_33_40_TX_DISABLE_REG      CPLD3_REG(0x10)
#define ACT7312_SFP_41_48_TX_DISABLE_REG      CPLD3_REG(0x11)

#define ACT7312_SFP_25_32_RX_LOSS_REG         CPLD3_REG(0x12)
#define ACT7312_SFP_33_40_RX_LOSS_REG         CPLD3_REG(0x13)
#define ACT7312_SFP_41_48_RX_LOSS_REG         CPLD3_REG(0x14)

#define ACT7312_QSFP_53_54_RESET_REG          CPLD3_REG(0x17)
#define ACT7312_QSFP_53_54_PRESENT_REG        CPLD3_REG(0x18)

#define ACT7312_100G_LED_GREEN2_REG           CPLD3_REG(0x20)
#define ACT7312_100G_LED_RED2_REG             CPLD3_REG(0x21)
#define ACT7312_100G_LED_BLUE2_REG            CPLD3_REG(0x22)

#define ACT7312_25G_LED_GREEN2_REG            CPLD3_REG(0x23)
#define ACT7312_25G_LED_RED2_REG              CPLD3_REG(0x24)
#define ACT7312_25G_LED_BLUE2_REG             CPLD3_REG(0x25)

#define ACT7312_40G_LED_GREEN2_REG            CPLD3_REG(0x26)
#define ACT7312_40G_LED_RED2_REG              CPLD3_REG(0x27)
#define ACT7312_40G_LED_BLUE2_REG             CPLD3_REG(0x28)

#define ACT7312_10G_LED_GREEN2_REG            CPLD3_REG(0x29)
#define ACT7312_10G_LED_RED2_REG              CPLD3_REG(0x2a)
#define ACT7312_10G_LED_BLUE2_REG             CPLD3_REG(0x2b)

/*
 * CPLD 4 (fan) register definitions
 */
#define ACT7312_CPLD4_BOARD_INFO_REG          CPLD4_REG(0x00)
#define ACT7312_CPLD4_VERSION_REG             CPLD4_REG(0x01)

#define ACT7312_FAN_TRAY_PRESENT_REG          CPLD4_REG(0x0f)
#define ACT7312_FAN_TRAY_DIR_REG              CPLD4_REG(0x10)

#define ACT7312_FAN_PWM_REG                   CPLD4_REG(0x11)
  #define FAN_PWM_MASK                        (0x0f)
  #define FAN_PWM_MULTIPLIER                  (16)

#define ACT7312_FAN1_SPEED_REG                CPLD4_REG(0x12)
#define ACT7312_FAN2_SPEED_REG                CPLD4_REG(0x13)
#define ACT7312_FAN3_SPEED_REG                CPLD4_REG(0x14)
#define ACT7312_FAN4_SPEED_REG                CPLD4_REG(0x15)
#define ACT7312_FAN5_SPEED_REG                CPLD4_REG(0x16)
#define ACT7312_FAN6_SPEED_REG                CPLD4_REG(0x17)

#define ACT7312_FANR1_SPEED_REG               CPLD4_REG(0x22)
#define ACT7312_FANR2_SPEED_REG               CPLD4_REG(0x23)
#define ACT7312_FANR3_SPEED_REG               CPLD4_REG(0x24)
#define ACT7312_FANR4_SPEED_REG               CPLD4_REG(0x25)
#define ACT7312_FANR5_SPEED_REG               CPLD4_REG(0x26)
#define ACT7312_FANR6_SPEED_REG               CPLD4_REG(0x27)
  #define FAN_SPEED_MULTIPLIER                (100)

#define ACT7312_FAN_WDOG_TIMER_REG            CPLD4_REG(0x31)

#define ACT7312_CPLD_FAN_WATCHDOG_REG         CPLD4_REG(0x33)
  #define FAN_WATCHDOG_ENABLE                 1

#endif  /* ACCTON_7312_CPLD_H__ */


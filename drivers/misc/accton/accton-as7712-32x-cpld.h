/*
 * Accton AS7712-32X CPLD Platform Definitions
 *
 * Copyright 2016 Cumulus Networks, Inc.
 * Nikhil Dhar <ndhar@cumulusnetworks.com>
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
 *
 */

#ifndef ACCTON_7712_CPLD_H__
#define ACCTON_7712_CPLD_H__

#define ACCTON_AS7712_CPLD_STRING_NAME_SIZE   30
#define CPLD_IDX_SHIFT                        (7)
#define CPLD1_ID                              (1)
#define CPLD2_ID                              (2)
#define CPLD3_ID                              (3)
#define FAN_CPLD_ID                           (4)
#define CPLD_IDX_MASK                         (7 << CPLD_IDX_SHIFT)
#define CPLD1_IDX                             (CPLD1_ID << CPLD_IDX_SHIFT)
#define CPLD2_IDX                             (CPLD2_ID << CPLD_IDX_SHIFT)
#define CPLD3_IDX                             (CPLD3_ID << CPLD_IDX_SHIFT)
#define FAN_CPLD_IDX                          (FAN_CPLD_ID << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS                  (4)

#define GET_CPLD_IDX(A)            (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT) - 1)
#define STRIP_CPLD_IDX(A)          (A & ~CPLD_IDX_MASK)

/************************************************************
 *              CPLD1 register definitions
 ************************************************************/
#define CPLD_BOARD_INFO_REG                   (0x00 | CPLD1_IDX)
#define CPLD_BOARD_INFO_PCB_ID_MASK           (0x0c)
#define CPLD_BOARD_INFO_PCB_VER_MASK          (0x03)
#define CPLD_CPLD1_VERSION_REG                (0x01 | CPLD1_IDX)
/* PSU status register */
#define CPLD_CPLD1_PSU_STATUS_REG             (0x02 | CPLD1_IDX)
#define CPLD_CPLD1_PSU2_STATUS_AC_ALERT_MASK  (0x10)
#define CPLD_CPLD1_PSU1_STATUS_AC_ALERT_MASK  (0x20)
#define CPLD_CPLD1_PSU2_STATUS_12V_GOOD_MASK  (0x04)
#define CPLD_CPLD1_PSU1_STATUS_12V_GOOD_MASK  (0x08)
#define CPLD_CPLD1_PSU2_STATUS_PRESENT_MASK   (0x01)
#define CPLD_CPLD1_PSU1_STATUS_PRESENT_MASK   (0x02)
/* QSFP reset registers */
#define CPLD_CPLD1_QSFP_1_8_RESET_REG         (0x04 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_PORT_1_RESET          BIT(0)
#define CPLD_CPLD1_QSFP_PORT_2_RESET          BIT(1)
#define CPLD_CPLD1_QSFP_PORT_3_RESET          BIT(2)
#define CPLD_CPLD1_QSFP_PORT_4_RESET          BIT(3)
#define CPLD_CPLD1_QSFP_PORT_5_RESET          BIT(4)
#define CPLD_CPLD1_QSFP_PORT_6_RESET          BIT(5)
#define CPLD_CPLD1_QSFP_PORT_7_RESET          BIT(6)
#define CPLD_CPLD1_QSFP_PORT_8_RESET          BIT(7)

#define CPLD_CPLD1_QSFP_9_16_RESET_REG        (0x05 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_PORT_9_RESET          BIT(0)
#define CPLD_CPLD1_QSFP_PORT_10_RESET         BIT(1)
#define CPLD_CPLD1_QSFP_PORT_11_RESET         BIT(2)
#define CPLD_CPLD1_QSFP_PORT_12_RESET         BIT(3)
#define CPLD_CPLD1_QSFP_PORT_13_RESET         BIT(4)
#define CPLD_CPLD1_QSFP_PORT_14_RESET         BIT(5)
#define CPLD_CPLD1_QSFP_PORT_15_RESET         BIT(6)
#define CPLD_CPLD1_QSFP_PORT_16_RESET         BIT(7)

#define CPLD_CPLD1_QSFP_17_24_RESET_REG       (0x06 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_PORT_17_RESET         BIT(0)
#define CPLD_CPLD1_QSFP_PORT_18_RESET         BIT(1)
#define CPLD_CPLD1_QSFP_PORT_19_RESET         BIT(2)
#define CPLD_CPLD1_QSFP_PORT_20_RESET         BIT(3)
#define CPLD_CPLD1_QSFP_PORT_21_RESET         BIT(4)
#define CPLD_CPLD1_QSFP_PORT_22_RESET         BIT(5)
#define CPLD_CPLD1_QSFP_PORT_23_RESET         BIT(6)
#define CPLD_CPLD1_QSFP_PORT_24_RESET         BIT(7)

#define CPLD_CPLD1_QSFP_25_32_RESET_REG       (0x07 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_PORT_25_RESET         BIT(0)
#define CPLD_CPLD1_QSFP_PORT_26_RESET         BIT(1)
#define CPLD_CPLD1_QSFP_PORT_27_RESET         BIT(2)
#define CPLD_CPLD1_QSFP_PORT_28_RESET         BIT(3)
#define CPLD_CPLD1_QSFP_PORT_29_RESET         BIT(4)
#define CPLD_CPLD1_QSFP_PORT_30_RESET         BIT(5)
#define CPLD_CPLD1_QSFP_PORT_31_RESET         BIT(6)
#define CPLD_CPLD1_QSFP_PORT_32_RESET         BIT(7)
/* System reset registers */
#define CPLD_CPLD1_SYSTEM_RESET_5_REG         (0x08 | CPLD1_IDX)
#define CPLD_CPLD1_SYSTEM_RESET_6_REG         (0x09 | CPLD1_IDX)
#define CPLD_CPLD1_SYSTEM_RESET_7_REG         (0x0a | CPLD1_IDX)
#define CPLD_CPLD1_SYSTEM_RESET_LOCK_REG      (0x0b | CPLD1_IDX)
/* QSFP interrupt status registers */
#define CPLD_CPLD1_QSFP_1_8_INTERRUPT_STATUS_REG     (0x10 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_9_16_INTERRUPT_STATUS_REG    (0x11 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_17_24_INTERRUPT_STATUS_REG   (0x12 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_25_32_INTERRUPT_STATUS_REG   (0x13 | CPLD1_IDX)
/* System interrupt status registers */
#define CPLD_CPLD1_INTERRUPT_STATUS_5_REG            (0x14 | CPLD1_IDX)
#define CPLD_CPLD1_INTERRUPT_STATUS_LM75_1           BIT(0)
#define CPLD_CPLD1_INTERRUPT_STATUS_LM75_2           BIT(1)
#define CPLD_CPLD1_INTERRUPT_STATUS_LM75_3           BIT(2)
#define CPLD_CPLD1_INTERRUPT_STATUS_CPLD_2           BIT(3)
#define CPLD_CPLD1_INTERRUPT_STATUS_CPLD_3           BIT(4)
#define CPLD_CPLD1_INTERRUPT_STATUS_FPGA             BIT(5)
#define CPLD_CPLD1_INTERRUPT_STATUS_FAN              BIT(6)
#define CPLD_CPLD1_INTERRUPT_STATUS_IDT8V89307       BIT(7)
#define CPLD_CPLD1_INTERRUPT_STATUS_6_REG            (0x15 | CPLD1_IDX)
#define CPLD_CPLD1_INTERRUPT_STATUS_MAC              BIT(0)
#define CPLD_CPLD1_INTERRUPT_STATUS_MAC_PCIE         BIT(1)
/* QSFP interrupt mask registers */
#define CPLD_CPLD1_QSFP_1_8_INTERRUPT_MASK_REG       (0x20 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_9_16_INTERRUPT_MASK_REG      (0x21 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_17_24_INTERRUPT_MASK_REG     (0x22 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_25_32_INTERRUPT_MASK_REG     (0x23 | CPLD1_IDX)
/* System interrupt mask registers */
#define CPLD_CPLD1_INTERRUPT_MASK_5_REG              (0x24 | CPLD1_IDX)
#define CPLD_CPLD1_INTERRUPT_MASK_LM75_1             BIT(0)
#define CPLD_CPLD1_INTERRUPT_MASK_LM75_2             BIT(1)
#define CPLD_CPLD1_INTERRUPT_MASK_LM75_3             BIT(2)
#define CPLD_CPLD1_INTERRUPT_MASK_CPLD_2             BIT(3)
#define CPLD_CPLD1_INTERRUPT_MASK_CPLD_3             BIT(4)
#define CPLD_CPLD1_INTERRUPT_MASK_FPGA               BIT(5)
#define CPLD_CPLD1_INTERRUPT_MASK_FAN                BIT(6)
#define CPLD_CPLD1_INTERRUPT_MASK_IDT8V89307         BIT(7)
#define CPLD_CPLD1_INTERRUPT_MASK_6_REG              (0x25 | CPLD1_IDX)
#define CPLD_CPLD1_INTERRUPT_MASK_MAC                BIT(0)
#define CPLD_CPLD1_INTERRUPT_MASK_MAC_PCIE           BIT(1)
/* QSFP present registers */
#define CPLD_CPLD1_QSFP_1_8_PRESENT_REG              (0x30 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_9_16_PRESENT_REG             (0x31 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_17_24_PRESENT_REG            (0x32 | CPLD1_IDX)
#define CPLD_CPLD1_QSFP_25_32_PRESENT_REG            (0x33 | CPLD1_IDX)
/* System led definitions */
#define CPLD_CPLD1_SYSTEM_LED_1_REG                  (0x41 | CPLD1_IDX)
#define CPLD_CPLD1_LOC_LED_MASK                      (0x80)
#define CPLD_CPLD1_LOC_BLUE_LED_OFF                  (0x80)
#define CPLD_CPLD1_LOC_BLUE_LED_ON                   (0x00)
#define CPLD_CPLD1_DIAG_LED_MASK                     (0x03)
#define CPLD_CPLD1_DIAG_RED_LED_ON                   (0x01)
#define CPLD_CPLD1_DIAG_LED_OFF                      (0x03)
#define CPLD_CPLD1_DIAG_GREEN_LED_ON                 (0x02)
/************************************************************
 *              CPLD2 register definitions
 ************************************************************/
#define CPLD_CPLD2_VERSION_REG                     (0x01 | CPLD2_IDX)

/************************************************************
 *              CPLD3 register definitions
 ************************************************************/
#define CPLD_CPLD3_VERSION_REG                     (0x01 | CPLD3_IDX)

/************************************************************
 *              FAN CPLD register definitions
 ************************************************************/
#define FAN_CPLD_BOARD_INFO_REG                    (0x00 | FAN_CPLD_IDX)
#define FAN_CPLD_BOARD_INFO_VER_ID_MASK            (0xe0)
#define FAN_CPLD_BOARD_INFO_BOARD_ID_MASK          (0x03)
#define FAN_CPLD_VERSION_REG                       (0x01 | FAN_CPLD_IDX)
#define FAN_CPLD_RESET_REG                         (0x04 | FAN_CPLD_IDX)
#define FAN_CPLD_INTERRUPT_STATUS_REG              (0x05 | FAN_CPLD_IDX)
#define FAN_CPLD_INTERRUPT_MASK_REG                (0x06 | FAN_CPLD_IDX)
/* Fan module present register */
#define FAN_CPLD_FAN_MOD_PRESENT_REG               (0x0f | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_1_PRESENT                     BIT(0)
#define FAN_CPLD_FAN_2_PRESENT                     BIT(1)
#define FAN_CPLD_FAN_3_PRESENT                     BIT(2)
#define FAN_CPLD_FAN_4_PRESENT                     BIT(3)
#define FAN_CPLD_FAN_5_PRESENT                     BIT(4)
#define FAN_CPLD_FAN_6_PRESENT                     BIT(5)
/* Fan direction register */
#define FAN_CPLD_FAN_MOD_DIRECTION_REG             (0x10 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_1_DIRECTION                   BIT(0)
#define FAN_CPLD_FAN_2_DIRECTION                   BIT(1)
#define FAN_CPLD_FAN_3_DIRECTION                   BIT(2)
#define FAN_CPLD_FAN_4_DIRECTION                   BIT(3)
#define FAN_CPLD_FAN_5_DIRECTION                   BIT(4)
#define FAN_CPLD_FAN_6_DIRECTION                   BIT(5)
/* Fan module PWM */
#define FAN_CPLD_FAN_MOD_PWM_REG                   (0x11 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_MOD_PWM_MASK                  (0x0f)
/* Fan module speed */
#define FAN_CPLD_FAN_1_SPEED_REG                   (0x12 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_2_SPEED_REG                   (0x13 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_3_SPEED_REG                   (0x14 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_4_SPEED_REG                   (0x15 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_5_SPEED_REG                   (0x16 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_6_SPEED_REG                   (0x17 | FAN_CPLD_IDX)
/* Fan 1 to 4 LED display */
#define FAN_CPLD_FAN_1_4_LED_DISPLAY_REG           (FAN_CPLD_IDX | 0x1c)
#define FAN_CPLD_FAN_4_GREEN                       BIT(0)
#define FAN_CPLD_FAN_4_RED                         BIT(1)
#define FAN_CPLD_FAN_3_GREEN                       BIT(2)
#define FAN_CPLD_FAN_3_RED                         BIT(3)
#define FAN_CPLD_FAN_2_GREEN                       BIT(4)
#define FAN_CPLD_FAN_2_RED                         BIT(5)
#define FAN_CPLD_FAN_1_GREEN                       BIT(6)
#define FAN_CPLD_FAN_1_RED                         BIT(7)
/* Fan 5 and 6 LED display */
#define FAN_CPLD_FAN_5_6_LED_DISPLAY_REG           (FAN_CPLD_IDX | 0x1d)
#define FAN_CPLD_FAN_6_GREEN                       BIT(0)
#define FAN_CPLD_FAN_6_RED                         BIT(1)
#define FAN_CPLD_FAN_5_GREEN                       BIT(2)
#define FAN_CPLD_FAN_5_RED                         BIT(3)
/* Rear Fan module speed */
#define FAN_CPLD_REAR_FAN_1_SPEED_REG              (0x22 | FAN_CPLD_IDX)
#define FAN_CPLD_REAR_FAN_2_SPEED_REG              (0x23 | FAN_CPLD_IDX)
#define FAN_CPLD_REAR_FAN_3_SPEED_REG              (0x24 | FAN_CPLD_IDX)
#define FAN_CPLD_REAR_FAN_4_SPEED_REG              (0x25 | FAN_CPLD_IDX)
#define FAN_CPLD_REAR_FAN_5_SPEED_REG              (0x26 | FAN_CPLD_IDX)
#define FAN_CPLD_REAR_FAN_6_SPEED_REG              (0x27 | FAN_CPLD_IDX)
/* Fan module power enable/disable */
#define FAN_CPLD_FAN_POWER_EN_REG                  (0x30 | FAN_CPLD_IDX)
#define FAN_CPLD_FAN_1_POWER_EN                    BIT(0)
#define FAN_CPLD_FAN_2_POWER_EN                    BIT(1)
#define FAN_CPLD_FAN_3_POWER_EN                    BIT(2)
#define FAN_CPLD_FAN_4_POWER_EN                    BIT(3)
#define FAN_CPLD_FAN_5_POWER_EN                    BIT(4)
#define FAN_CPLD_FAN_6_POWER_EN                    BIT(5)
/* Fan CPLD watchdog enable/disable */
#define FAN_CPLD_WD_DISABLE_REG			  (0x33 | FAN_CPLD_IDX)
#define FAN_CPLD_WD_ENABLE			  (0x1)
#define FAN_CPLD_WD_DISABLE			  (0x0)
#define FAN_CPLD_WD_STATE_NAME_SIZE		  20
#endif

/*
 * DELL S6010 CPLD Platform Definitions
 *
 * Copyright (C) 2016 Cumulus Networks, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef DELL_S6010_CPLD_H__
#define DELL_S6010_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from S6010-ON_HW_SPEC_REV09_20151209.pdf
 */
#define DELL_S6010_CPLD_STRING_NAME_SIZE  30

/* the S6010 actually has three CPLDs: system, master, slave
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed
 * into each register definition. When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */

#define CPLD_IDX_SHIFT                   (6)
#define SYSTEM_CPLD_ID                   (1)
#define MASTER_CPLD_ID                   (2)
#define SLAVE_CPLD_ID                    (3)
#define CPLD_IDX_MASK                    (3 << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX                  (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX                  (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define SLAVE_CPLD_IDX                   (SLAVE_CPLD_ID << CPLD_IDX_SHIFT)
#define NO_CPLD_I2C_CLIENTS              (3)

#define GET_CPLD_IDX(A)                  (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                (A & ~CPLD_IDX_MASK)

/* Master CPLD, LED control register 1 */
#define CPLD_LED_CONTROL_1_REG           (0x07 | MASTER_CPLD_IDX)
#define CPLD_SYSTEM_LED_MASK             (0x30)
#define CPLD_SYSTEM_LED_GREEN_BLINK      (0x00)
#define CPLD_SYSTEM_LED_GREEN            (0x10)
#define CPLD_SYSTEM_LED_YELLOW           (0x20)
#define CPLD_SYSTEM_LED_YELLOW_BLINK     (0x30)
#define CPLD_BEACON_LED_MASK             (0x08)
#define CPLD_BEACON_LED_OFF              (0x00)
#define CPLD_BEACON_LED_BLUE_BLINK       (0x08)
#define CPLD_POWER_LED_MASK              (0x06)
#define CPLD_POWER_LED_OFF               (0x00)
#define CPLD_POWER_LED_YELLOW            (0x02)
#define CPLD_POWER_LED_GREEN             (0x04)
#define CPLD_POWER_LED_YELLOW_BLINK      (0x06)
#define CPLD_MASTER_LED_MASK             (0x01)
#define CPLD_MASTER_LED_OFF              (0x01)
#define CPLD_MASTER_LED_GREEN            (0x00)

/* Master CPLD, Fan status register 1 */
#define CPLD_FAN_STATUS_1_REG		 (0x08 | MASTER_CPLD_IDX)
#define CPLD_FAN_TRAY_2_PRESENT_L        (1 << 7)
#define CPLD_FAN_TRAY_1_PRESENT_L        (1 << 6)
#define CPLD_FAN_TRAY_3_LED_MASK         (0x30)
#define CPLD_FAN_TRAY_3_LED_OFF          (0x00)
#define CPLD_FAN_TRAY_3_LED_GREEN        (0x10)
#define CPLD_FAN_TRAY_3_LED_YELLOW       (0x20)
#define CPLD_FAN_TRAY_2_LED_MASK         (0x0C)
#define CPLD_FAN_TRAY_2_LED_OFF          (0x00)
#define CPLD_FAN_TRAY_2_LED_GREEN        (0x04)
#define CPLD_FAN_TRAY_2_LED_YELLOW       (0x08)
#define CPLD_FAN_TRAY_1_LED_MASK         (0x03)
#define CPLD_FAN_TRAY_1_LED_OFF          (0x00)
#define CPLD_FAN_TRAY_1_LED_GREEN        (0x01)
#define CPLD_FAN_TRAY_1_LED_YELLOW       (0x02)

/* Master CPLD, fan status register 2 */
#define CPLD_FAN_STATUS_2_REG		     (0x09 | MASTER_CPLD_IDX)
#define CPLD_FRONT_FAN_LED_MASK		     (0x18)
#define CPLD_FRONT_FAN_LED_OFF		     (0x00)
#define CPLD_FRONT_FAN_LED_GREEN         (0x10)
#define CPLD_FRONT_FAN_LED_YELLOW        (0x08)
#define CPLD_FRONT_FAN_LED_YELLOW_BLINK	 (0x18)
#define CPLD_PORT_LED_MASK		         (0x20)
#define CPLD_PORT_LED_OFF		         (0x00)
#define CPLD_PORT_LED_ON		         (0x20)

/* Master CPLD, fan status register 2/2 */
#define CPLD_FAN_STATUS_2_2_REG		 (0x19 | MASTER_CPLD_IDX)
#define CPLD_FAN_TRAY_5_PRESENT_L        (1 << 6)
#define CPLD_FAN_TRAY_4_PRESENT_L        (1 << 5)
#define CPLD_FAN_TRAY_3_PRESENT_L        (1 << 4)
#define CPLD_FAN_TRAY_4_LED_MASK         (0x03)
#define CPLD_FAN_TRAY_4_LED_OFF          (0x00)
#define CPLD_FAN_TRAY_4_LED_GREEN        (0x01)
#define CPLD_FAN_TRAY_4_LED_YELLOW       (0x02)
#define CPLD_FAN_TRAY_5_LED_MASK         (0x0C)
#define CPLD_FAN_TRAY_5_LED_OFF          (0x00)
#define CPLD_FAN_TRAY_5_LED_GREEN        (0x04)
#define CPLD_FAN_TRAY_5_LED_YELLOW       (0x08)


/* Master CPLD, QSFP module mode control register 1 */
#define CPLD_QSFP_17_24_MOD_SELECT_REG   (0x0a | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_24_MOD_SELECT     (1 << 7)
#define CPLD_QSFP_PORT_23_MOD_SELECT     (1 << 6)
#define CPLD_QSFP_PORT_22_MOD_SELECT     (1 << 5)
#define CPLD_QSFP_PORT_21_MOD_SELECT     (1 << 4)
#define CPLD_QSFP_PORT_20_MOD_SELECT     (1 << 3)
#define CPLD_QSFP_PORT_19_MOD_SELECT     (1 << 2)
#define CPLD_QSFP_PORT_18_MOD_SELECT     (1 << 1)
#define CPLD_QSFP_PORT_17_MOD_SELECT     (1 << 0)

/* Master CPLD, QSFP module mode control register 2 */
#define CPLD_QSFP_25_32_MOD_SELECT_REG   (0x0b | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_32_MOD_SELECT     (1 << 7)
#define CPLD_QSFP_PORT_31_MOD_SELECT     (1 << 6)
#define CPLD_QSFP_PORT_30_MOD_SELECT     (1 << 5)
#define CPLD_QSFP_PORT_29_MOD_SELECT     (1 << 4)
#define CPLD_QSFP_PORT_28_MOD_SELECT     (1 << 3)
#define CPLD_QSFP_PORT_27_MOD_SELECT     (1 << 2)
#define CPLD_QSFP_PORT_26_MOD_SELECT     (1 << 1)
#define CPLD_QSFP_PORT_25_MOD_SELECT     (1 << 0)

/* Slave CPLD, QSFP LP mode control register 1 */
#define CPLD_QSFP_1_8_MOD_SELECT_REG     (0x00 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_8_MOD_SELECT      (1 << 7)
#define CPLD_QSFP_PORT_7_MOD_SELECT      (1 << 6)
#define CPLD_QSFP_PORT_6_MOD_SELECT      (1 << 5)
#define CPLD_QSFP_PORT_5_MOD_SELECT      (1 << 4)
#define CPLD_QSFP_PORT_4_MOD_SELECT      (1 << 3)
#define CPLD_QSFP_PORT_3_MOD_SELECT      (1 << 2)
#define CPLD_QSFP_PORT_2_MOD_SELECT      (1 << 1)
#define CPLD_QSFP_PORT_1_MOD_SELECT      (1 << 0)

/* Slave CPLD, QSFP LP mode control register 2 */
#define CPLD_QSFP_9_16_MOD_SELECT_REG    (0x01 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_16_MOD_SELECT     (1 << 7)
#define CPLD_QSFP_PORT_15_MOD_SELECT     (1 << 6)
#define CPLD_QSFP_PORT_14_MOD_SELECT     (1 << 5)
#define CPLD_QSFP_PORT_13_MOD_SELECT     (1 << 4)
#define CPLD_QSFP_PORT_12_MOD_SELECT     (1 << 3)
#define CPLD_QSFP_PORT_11_MOD_SELECT     (1 << 2)
#define CPLD_QSFP_PORT_10_MOD_SELECT     (1 << 1)
#define CPLD_QSFP_PORT_9_MOD_SELECT      (1 << 0)
/* Master CPLD, QSFP LP mode control register 1 */
#define CPLD_QSFP_17_24_LP_MODE_REG      (0x0c | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_24_LP_MODE        (1 << 7)
#define CPLD_QSFP_PORT_23_LP_MODE        (1 << 6)
#define CPLD_QSFP_PORT_22_LP_MODE        (1 << 5)
#define CPLD_QSFP_PORT_21_LP_MODE        (1 << 4)
#define CPLD_QSFP_PORT_20_LP_MODE        (1 << 3)
#define CPLD_QSFP_PORT_19_LP_MODE        (1 << 2)
#define CPLD_QSFP_PORT_18_LP_MODE        (1 << 1)
#define CPLD_QSFP_PORT_17_LP_MODE        (1 << 0)

/* Master CPLD, QSFP LP mode control register 2 */
#define CPLD_QSFP_25_32_LP_MODE_REG      (0x0d | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_32_LP_MODE        (1 << 7)
#define CPLD_QSFP_PORT_31_LP_MODE        (1 << 6)
#define CPLD_QSFP_PORT_30_LP_MODE        (1 << 5)
#define CPLD_QSFP_PORT_29_LP_MODE        (1 << 4)
#define CPLD_QSFP_PORT_28_LP_MODE        (1 << 3)
#define CPLD_QSFP_PORT_27_LP_MODE        (1 << 2)
#define CPLD_QSFP_PORT_26_LP_MODE        (1 << 1)
#define CPLD_QSFP_PORT_25_LP_MODE        (1 << 0)

/* Slave CPLD, QSFP LP mode control register 1 */
#define CPLD_QSFP_1_8_LP_MODE_REG        (0x02 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_1_LP_MODE         (1 << 7)
#define CPLD_QSFP_PORT_2_LP_MODE         (1 << 6)
#define CPLD_QSFP_PORT_3_LP_MODE         (1 << 5)
#define CPLD_QSFP_PORT_4_LP_MODE         (1 << 4)
#define CPLD_QSFP_PORT_5_LP_MODE         (1 << 3)
#define CPLD_QSFP_PORT_6_LP_MODE         (1 << 2)
#define CPLD_QSFP_PORT_7_LP_MODE         (1 << 1)
#define CPLD_QSFP_PORT_8_LP_MODE         (1 << 0)

/* Slave CPLD, QSFP LP mode control register 2 */
#define CPLD_QSFP_9_16_LP_MODE_REG       (0x03 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_9_LP_MODE         (1 << 7)
#define CPLD_QSFP_PORT_10_LP_MODE        (1 << 6)
#define CPLD_QSFP_PORT_11_LP_MODE        (1 << 5)
#define CPLD_QSFP_PORT_12_LP_MODE        (1 << 4)
#define CPLD_QSFP_PORT_13_LP_MODE        (1 << 3)
#define CPLD_QSFP_PORT_14_LP_MODE        (1 << 2)
#define CPLD_QSFP_PORT_15_LP_MODE        (1 << 1)
#define CPLD_QSFP_PORT_16_LP_MODE        (1 << 0)
/* Master CPLD, QSFP presence control 1 register */
#define CPLD_QSFP_17_24_PRESENT_REG      (0x0e | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_24_PRESENT_L      (1 << 7)
#define CPLD_QSFP_PORT_23_PRESENT_L      (1 << 6)
#define CPLD_QSFP_PORT_22_PRESENT_L      (1 << 5)
#define CPLD_QSFP_PORT_21_PRESENT_L      (1 << 4)
#define CPLD_QSFP_PORT_20_PRESENT_L      (1 << 3)
#define CPLD_QSFP_PORT_19_PRESENT_L      (1 << 2)
#define CPLD_QSFP_PORT_18_PRESENT_L      (1 << 1)
#define CPLD_QSFP_PORT_17_PRESENT_L      (1 << 0)

/* Master CPLD, QSFP presence control 2 register */
#define CPLD_QSFP_25_32_PRESENT_REG      (0x0f | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_32_PRESENT_L      (1 << 7)
#define CPLD_QSFP_PORT_31_PRESENT_L      (1 << 6)
#define CPLD_QSFP_PORT_30_PRESENT_L      (1 << 5)
#define CPLD_QSFP_PORT_29_PRESENT_L      (1 << 4)
#define CPLD_QSFP_PORT_28_PRESENT_L      (1 << 3)
#define CPLD_QSFP_PORT_27_PRESENT_L      (1 << 2)
#define CPLD_QSFP_PORT_26_PRESENT_L      (1 << 1)
#define CPLD_QSFP_PORT_25_PRESENT_L      (1 << 0)

/* Slave CPLD, QSFP presence control 1 register */
#define CPLD_QSFP_1_8_PRESENT_REG        (0x04 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_8_PRESENT_L       (1 << 7)
#define CPLD_QSFP_PORT_7_PRESENT_L       (1 << 6)
#define CPLD_QSFP_PORT_6_PRESENT_L       (1 << 5)
#define CPLD_QSFP_PORT_5_PRESENT_L       (1 << 4)
#define CPLD_QSFP_PORT_4_PRESENT_L       (1 << 3)
#define CPLD_QSFP_PORT_3_PRESENT_L       (1 << 2)
#define CPLD_QSFP_PORT_2_PRESENT_L       (1 << 1)
#define CPLD_QSFP_PORT_1_PRESENT_L       (1 << 0)

/* Master CPLD, QSFP presence control 2 register */
#define CPLD_QSFP_9_16_PRESENT_REG       (0x05 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_16_PRESENT_L      (1 << 7)
#define CPLD_QSFP_PORT_15_PRESENT_L      (1 << 6)
#define CPLD_QSFP_PORT_14_PRESENT_L      (1 << 5)
#define CPLD_QSFP_PORT_13_PRESENT_L      (1 << 4)
#define CPLD_QSFP_PORT_12_PRESENT_L      (1 << 3)
#define CPLD_QSFP_PORT_11_PRESENT_L      (1 << 2)
#define CPLD_QSFP_PORT_10_PRESENT_L      (1 << 1)
#define CPLD_QSFP_PORT_9_PRESENT_L       (1 << 0)
/* Master CPLD, QSFP reset control 1 register */
#define CPLD_QSFP_17_24_RESET_REG        (0x10 | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_24_RESET_L        (1 << 7)
#define CPLD_QSFP_PORT_23_RESET_L        (1 << 6)
#define CPLD_QSFP_PORT_22_RESET_L        (1 << 5)
#define CPLD_QSFP_PORT_21_RESET_L        (1 << 4)
#define CPLD_QSFP_PORT_20_RESET_L        (1 << 3)
#define CPLD_QSFP_PORT_19_RESET_L        (1 << 2)
#define CPLD_QSFP_PORT_18_RESET_L        (1 << 1)
#define CPLD_QSFP_PORT_17_RESET_L        (1 << 0)

/* Master CPLD, QSFP reset control 2 register */
#define CPLD_QSFP_25_32_RESET_REG        (0x11 | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_32_RESET_L        (1 << 7)
#define CPLD_QSFP_PORT_31_RESET_L        (1 << 6)
#define CPLD_QSFP_PORT_30_RESET_L        (1 << 5)
#define CPLD_QSFP_PORT_29_RESET_L        (1 << 4)
#define CPLD_QSFP_PORT_28_RESET_L        (1 << 3)
#define CPLD_QSFP_PORT_27_RESET_L        (1 << 2)
#define CPLD_QSFP_PORT_26_RESET_L        (1 << 1)
#define CPLD_QSFP_PORT_25_RESET_L        (1 << 0)

/* Slave CPLD, QSFP reset control 1 register */
#define CPLD_QSFP_1_8_RESET_REG          (0x06 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_8_RESET_L         (1 << 7)
#define CPLD_QSFP_PORT_7_RESET_L         (1 << 6)
#define CPLD_QSFP_PORT_6_RESET_L         (1 << 5)
#define CPLD_QSFP_PORT_5_RESET_L         (1 << 4)
#define CPLD_QSFP_PORT_4_RESET_L         (1 << 3)
#define CPLD_QSFP_PORT_3_RESET_L         (1 << 2)
#define CPLD_QSFP_PORT_2_RESET_L         (1 << 1)
#define CPLD_QSFP_PORT_1_RESET_L         (1 << 0)

/* Slave CPLD, QSFP reset control 2 register */
#define CPLD_QSFP_9_16_RESET_REG         (0x07 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_16_RESET_L        (1 << 7)
#define CPLD_QSFP_PORT_15_RESET_L        (1 << 6)
#define CPLD_QSFP_PORT_14_RESET_L        (1 << 5)
#define CPLD_QSFP_PORT_13_RESET_L        (1 << 4)
#define CPLD_QSFP_PORT_12_RESET_L        (1 << 3)
#define CPLD_QSFP_PORT_11_RESET_L        (1 << 2)
#define CPLD_QSFP_PORT_10_RESET_L        (1 << 1)
#define CPLD_QSFP_PORT_9_RESET_L         (1 << 0)

/* Master CPLD, QSFP interrupt control 1 register */
#define CPLD_QSFP_17_24_INTERRUPT_REG    (0x12 | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_24_INTERRUPT_L    (1 << 7)
#define CPLD_QSFP_PORT_23_INTERRUPT_L    (1 << 6)
#define CPLD_QSFP_PORT_22_INTERRUPT_L    (1 << 5)
#define CPLD_QSFP_PORT_21_INTERRUPT_L    (1 << 4)
#define CPLD_QSFP_PORT_20_INTERRUPT_L    (1 << 3)
#define CPLD_QSFP_PORT_19_INTERRUPT_L    (1 << 2)
#define CPLD_QSFP_PORT_18_INTERRUPT_L    (1 << 1)
#define CPLD_QSFP_PORT_17_INTERRUPT_L    (1 << 0)

/* Master CPLD, QSFP interrupt control 2 register */
#define CPLD_QSFP_25_32_INTERRUPT_REG    (0x13 | MASTER_CPLD_IDX)
#define CPLD_QSFP_PORT_32_INTERRUPT_L    (1 << 7)
#define CPLD_QSFP_PORT_31_INTERRUPT_L    (1 << 6)
#define CPLD_QSFP_PORT_30_INTERRUPT_L    (1 << 5)
#define CPLD_QSFP_PORT_29_INTERRUPT_L    (1 << 4)
#define CPLD_QSFP_PORT_28_INTERRUPT_L    (1 << 3)
#define CPLD_QSFP_PORT_27_INTERRUPT_L    (1 << 2)
#define CPLD_QSFP_PORT_26_INTERRUPT_L    (1 << 1)
#define CPLD_QSFP_PORT_25_INTERRUPT_L    (1 << 0)

/* Slave CPLD, QSFP interrupt control 1 register */
#define CPLD_QSFP_1_8_INTERRUPT_REG     (0x08 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_8_INTERRUPT_L    (1 << 7)
#define CPLD_QSFP_PORT_7_INTERRUPT_L    (1 << 6)
#define CPLD_QSFP_PORT_6_INTERRUPT_L    (1 << 5)
#define CPLD_QSFP_PORT_5_INTERRUPT_L    (1 << 4)
#define CPLD_QSFP_PORT_4_INTERRUPT_L    (1 << 3)
#define CPLD_QSFP_PORT_3_INTERRUPT_L    (1 << 2)
#define CPLD_QSFP_PORT_2_INTERRUPT_L    (1 << 1)
#define CPLD_QSFP_PORT_1_INTERRUPT_L    (1 << 0)

/* Slave CPLD, QSFP interrupt control 2 register */
#define CPLD_QSFP_9_16_INTERRUPT_REG    (0x09 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_PORT_16_INTERRUPT_L   (1 << 7)
#define CPLD_QSFP_PORT_15_INTERRUPT_L   (1 << 6)
#define CPLD_QSFP_PORT_14_INTERRUPT_L   (1 << 5)
#define CPLD_QSFP_PORT_13_INTERRUPT_L   (1 << 4)
#define CPLD_QSFP_PORT_12_INTERRUPT_L   (1 << 3)
#define CPLD_QSFP_PORT_11_INTERRUPT_L   (1 << 2)
#define CPLD_QSFP_PORT_10_INTERRUPT_L   (1 << 1)
#define CPLD_QSFP_PORT_9_INTERRUPT_L    (1 << 0)

/* Master CPLD, Power Supply Status Register */
#define CPLD_QSFP_1_32_MUX_SELECT_REG    (0x1a | MASTER_CPLD_IDX)
#define CPLD_POWER_SUPPLY_STATUS_REG     (0x03 | MASTER_CPLD_IDX)
#define CPLD_PSU1_MASK                   (0xE0)
#define CPLD_PSU1_PRESENT_L              (1 << 7)
#define CPLD_PSU1_GOOD_L                 (1 << 6)
#define CPLD_PSU1_ERROR                  (1 << 5)
#define CPLD_PSU2_MASK                   (0x0E)
#define CPLD_PSU2_PRESENT_L              (1 << 3)
#define CPLD_PSU2_GOOD_L                 (1 << 2)
#define CPLD_PSU2_ERROR                  (1 << 1)
#define CPLD_SYSTEM_REV_REG               (0x08 | SYSTEM_CPLD_IDX)
#define CPLD_MASTER_REV_REG               (0x18 | MASTER_CPLD_IDX)
#define CPLD_SLAVE_REV_REG		          (0x0b | SLAVE_CPLD_IDX)
#define CPLD_SYSTEM_HW_REV_REG            (0    | SYSTEM_CPLD_IDX)
#define CPLD_MASTER_HW_REV_REG            (0x01 | MASTER_CPLD_IDX)

#endif

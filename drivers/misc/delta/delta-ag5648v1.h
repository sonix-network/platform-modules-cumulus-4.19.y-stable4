/*
 * Delta AG5648V1 CPLD Platform Definitions
 *
 * Copyright (C) 2018 Cumulus Networks, Inc.  All Rights Reserved.
 * David Yen <dhyen@cumulusnetworks.com>
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

#ifndef DELTA_AG5648V1_CPLD_H__
#define DELTA_AG5648V1_CPLD_H__

#define CPLD_STRING_NAME_SIZE  30
#define SMB_I801_NAME "SMBus I801 adapter"
#define SMB_ISMT_NAME "SMBus iSMT adapter"

/*
 *  The iSMT bus is connected to the following devices:
 *
 *    pca9548 8-channel mux (0x70)
 *       0 system cpld (0x31)
 *         master cpld (0x35)
 *         slave cpld (0x39)
 *         temp sensor (0x4d)
 *         board eeprom (0x53)
 *       1 temp sensor (0x4b)
 *         temp sensor (0x4c)
 *         temp sensor (0x49)
 *         temp sensor (0x4e)
 *         temp sensor (0x4f)
 *         fan tray 1 eeprom (0x51)
 *         fan tray 2 eeprom (0x52)
 *         fan tray 3 eeprom (0x53)
 *         fan tray 4 eeprom (0x54)
 *         fan controller 1 (0x4d)
 *       2 sfp28 and qsfp8 modules (0x50)
 *       3 fan controller 2 (0x4d)
 */

enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS,
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,
	I2C_FIRST_SFF_PORT = 21,
};

enum {
	_INV_IDX = -1,
	_PRESENT_IDX = 0,
	_TX_DISABLE_IDX,
	_RX_LOS_IDX,
	_TX_FAULT_IDX,
};

/* CPLDs */
#define NUM_CPLD_DEVICES                  (3)
#define SYSTEM_CPLD_ID                    (1)
#define MASTER_CPLD_ID                    (2)
#define SLAVE_CPLD_ID                     (3)

#define CPLD_IDX_SHIFT                    (6)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX                   (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX                   (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define SLAVE_CPLD_IDX                    (SLAVE_CPLD_ID << CPLD_IDX_SHIFT)
#define MAX_REGS                          (7)

#define GET_CPLD_ID(A)            (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT) - 1)
#define STRIP_CPLD_IDX(A)         (A & ~CPLD_IDX_MASK)

/* SFF muxes */
#define PORT_LABEL_SIZE                   (8)
#define SFF_MUXES                         (1)
#define NUM_SFF_PORTS                     (54)
#define FIRST_SFP28_PORT                  (1)
#define NUM_SFP28_PORTS                   (48)
#define FIRST_QSFP28_PORT                 (49)
#define NUM_QSFP28_PORTS                  (6)

/*
 * Register info from AG5648_HW_SPEC_REV13_20161124.pdf
 */

/* System CPLD register definitions */
#define SYSTEM_HW_REVISION_REG            (0x00 | SYSTEM_CPLD_IDX)
#define  SYSTEM_BOARD_STAGE_MASK          (0xf0)
#define  SYSTEM_PLATFORM_TYPE_MASK        (0x03)

#define SYSTEM_SOFTWARE_RESET_REG         (0x01 | SYSTEM_CPLD_IDX)

#define POWER_RAILS_ENABLE_REG            (0x02 | SYSTEM_CPLD_IDX)

#define BIOS_EEPROM_SPI_WP_CONTROL_REG    (0x03 | SYSTEM_CPLD_IDX)

#define POWER_STATUS_1_REG                (0x04 | SYSTEM_CPLD_IDX)
#define  PG_DDR_VTT_MASK                  BIT(7)
#define  PG_PVDDR_MASK                    BIT(6)
#define  PG_PWR_CORE_MASK                 BIT(5)
#define  PG_V1P1_MASK                     BIT(4)
#define  PG_V1P0_MASK                     BIT(3)
#define  PG_3V3_MASK                      BIT(2)
#define  PG_1V8_MASK                      BIT(1)
#define  PG_1V35_MASK                     BIT(0)

#define POWER_STATUS_2_REG                (0x05 | SYSTEM_CPLD_IDX)

#define INTERRUPT_ALARM_REG               (0x06 | SYSTEM_CPLD_IDX)
#define  VRHOT_VCCP_MASK                  BIT(2)
#define  CPU_THERMTRIP_MASK               BIT(1)
#define  TEMP_ALERT_MASK                  BIT(0)

#define WATCH_DOG_FUNCTION_REG            (0x07 | SYSTEM_CPLD_IDX)
#define  WD_TIMER_MASK                    (7 << 4)
#define  WD_ENABLE_MASK                   BIT(3)
#define  WDI_FLAG_MASK                    BIT(0)
#define SYSTEM_CPLD_REVISION_REG          (0x08 | SYSTEM_CPLD_IDX)

/* Master CPLD register definitions */
#define MASTER_HW_REVISION_REG            (0x01 | MASTER_CPLD_IDX)
#define  MASTER_BOARD_STAGE_MASK          (0xf0)
#define  MASTER_PLATFORM_TYPE_MASK        (0x0f)

#define MASTER_SOFTWARE_RESET_REG         (0x02 | MASTER_CPLD_IDX)

#define POWER_SUPPLY_STATUS_REG           (0x03 | MASTER_CPLD_IDX)
#define  PS1_PS_N_MASK                    BIT(7)
#define  PS1_PG_N_MASK                    BIT(6)
#define  PS1_INT_N_MASK                   BIT(5)
#define  PS1_ON_N_MASK                    BIT(4)
#define  PS2_PS_N_MASK                    BIT(3)
#define  PS2_PG_N_MASK                    BIT(2)
#define  PS2_INT_N_MASK                   BIT(1)
#define  PS2_ON_N_MASK                    BIT(0)

#define POWER_ENABLE_REG                  (0x04 | MASTER_CPLD_IDX)

#define POWER_STATUS_REG                  (0x05 | MASTER_CPLD_IDX)

#define INTERRUPT_1_REG                   (0x06 | MASTER_CPLD_IDX)

#define INTERRUPT_1_MASK_REG              (0x07 | MASTER_CPLD_IDX)

#define SFP28_44_37_MODULE_ABSENT_REG     (0x08 | MASTER_CPLD_IDX)
#define  SFP28_MOD_P44_MASK               BIT(7)
#define  SFP28_MOD_P43_MASK               BIT(6)
#define  SFP28_MOD_P42_MASK               BIT(5)
#define  SFP28_MOD_P41_MASK               BIT(4)
#define  SFP28_MOD_P40_MASK               BIT(3)
#define  SFP28_MOD_P39_MASK               BIT(2)
#define  SFP28_MOD_P38_MASK               BIT(1)
#define  SFP28_MOD_P37_MASK               BIT(0)

#define SFP28_48_45_MODULE_ABSENT_REG     (0x09 | MASTER_CPLD_IDX)
#define  SFP28_MOD_P48_MASK               BIT(7)
#define  SFP28_MOD_P47_MASK               BIT(6)
#define  SFP28_MOD_P46_MASK               BIT(5)
#define  SFP28_MOD_P45_MASK               BIT(4)

#define SFP28_44_37_MODULE_TX_DISABLE_REG (0x0A | MASTER_CPLD_IDX)
#define  SFP28_TXDIS_P44_MASK             BIT(7)
#define  SFP28_TXDIS_P43_MASK             BIT(6)
#define  SFP28_TXDIS_P42_MASK             BIT(5)
#define  SFP28_TXDIS_P41_MASK             BIT(4)
#define  SFP28_TXDIS_P40_MASK             BIT(3)
#define  SFP28_TXDIS_P39_MASK             BIT(2)
#define  SFP28_TXDIS_P38_MASK             BIT(1)
#define  SFP28_TXDIS_P37_MASK             BIT(0)

#define SFP28_48_45_MODULE_TX_DISABLE_REG (0x0B | MASTER_CPLD_IDX)
#define  SFP28_TXDIS_P48_MASK             BIT(7)
#define  SFP28_TXDIS_P47_MASK             BIT(6)
#define  SFP28_TXDIS_P46_MASK             BIT(5)
#define  SFP28_TXDIS_P45_MASK             BIT(4)

#define SFP28_44_37_MODULE_RX_LOS_REG     (0x0C | MASTER_CPLD_IDX)
#define  SFP28_RXLOS_P44_MASK             BIT(7)
#define  SFP28_RXLOS_P43_MASK             BIT(6)
#define  SFP28_RXLOS_P42_MASK             BIT(5)
#define  SFP28_RXLOS_P41_MASK             BIT(4)
#define  SFP28_RXLOS_P40_MASK             BIT(3)
#define  SFP28_RXLOS_P39_MASK             BIT(2)
#define  SFP28_RXLOS_P38_MASK             BIT(1)
#define  SFP28_RXLOS_P37_MASK             BIT(0)

#define SFP28_48_45_MODULE_RX_LOS_REG     (0x0D | MASTER_CPLD_IDX)
#define  SFP28_RXLOS_P48_MASK             BIT(7)
#define  SFP28_RXLOS_P47_MASK             BIT(6)
#define  SFP28_RXLOS_P46_MASK             BIT(5)
#define  SFP28_RXLOS_P45_MASK             BIT(4)

#define SFP28_44_37_MODULE_TX_FAULT_REG   (0x0E | MASTER_CPLD_IDX)
#define  SFP28_TXFAULT_P44_MASK           BIT(7)
#define  SFP28_TXFAULT_P43_MASK           BIT(6)
#define  SFP28_TXFAULT_P42_MASK           BIT(5)
#define  SFP28_TXFAULT_P41_MASK           BIT(4)
#define  SFP28_TXFAULT_P40_MASK           BIT(3)
#define  SFP28_TXFAULT_P39_MASK           BIT(2)
#define  SFP28_TXFAULT_P38_MASK           BIT(1)
#define  SFP28_TXFAULT_P37_MASK           BIT(0)

#define SFP28_48_45_MODULE_TX_FAULT_REG   (0x0F | MASTER_CPLD_IDX)
#define  SFP28_TXFAULT_P48_MASK           BIT(7)
#define  SFP28_TXFAULT_P47_MASK           BIT(6)
#define  SFP28_TXFAULT_P46_MASK           BIT(5)
#define  SFP28_TXFAULT_P45_MASK           BIT(4)

#define QSFP28_MODULE_MODE_SELECT_REG     (0x10 | MASTER_CPLD_IDX)
#define  QSFP28_54_MODESEL_MASK           BIT(5)
#define  QSFP28_53_MODESEL_MASK           BIT(4)
#define  QSFP28_52_MODESEL_MASK           BIT(3)
#define  QSFP28_51_MODESEL_MASK           BIT(2)
#define  QSFP28_50_MODESEL_MASK           BIT(1)
#define  QSFP28_49_MODESEL_MASK           BIT(0)

#define QSFP28_LP_MODE_CONTROL_REG        (0x11 | MASTER_CPLD_IDX)
#define  QSFP28_54_LP_MODE_MASK           BIT(5)
#define  QSFP28_53_LP_MODE_MASK           BIT(4)
#define  QSFP28_52_LP_MODE_MASK           BIT(3)
#define  QSFP28_51_LP_MODE_MASK           BIT(2)
#define  QSFP28_50_LP_MODE_MASK           BIT(1)
#define  QSFP28_49_LP_MODE_MASK           BIT(0)

#define QSFP28_PRESENCE_STATUS_REG        (0x12 | MASTER_CPLD_IDX)
#define  QSFP28_54_PRESENT_N_MASK         BIT(5)
#define  QSFP28_53_PRESENT_N_MASK         BIT(4)
#define  QSFP28_52_PRESENT_N_MASK         BIT(3)
#define  QSFP28_51_PRESENT_N_MASK         BIT(2)
#define  QSFP28_50_PRESENT_N_MASK         BIT(1)
#define  QSFP28_49_PRESENT_N_MASK         BIT(0)

#define QSFP28_RESET_CONTROL_REG          (0x13 | MASTER_CPLD_IDX)
#define  QSFP28_54_RST_N_MASK             BIT(5)
#define  QSFP28_53_RST_N_MASK             BIT(4)
#define  QSFP28_52_RST_N_MASK             BIT(3)
#define  QSFP28_51_RST_N_MASK             BIT(2)
#define  QSFP28_50_RST_N_MASK             BIT(1)
#define  QSFP28_49_RST_N_MASK             BIT(0)

#define QSFP28_INTERRUPT_STATUS_REG       (0x14 | MASTER_CPLD_IDX)
#define  QSFP28_54_INT_N_MASK             BIT(5)
#define  QSFP28_53_INT_N_MASK             BIT(4)
#define  QSFP28_52_INT_N_MASK             BIT(3)
#define  QSFP28_51_INT_N_MASK             BIT(2)
#define  QSFP28_50_INT_N_MASK             BIT(1)
#define  QSFP28_49_INT_N_MASK             BIT(0)

#define TOMAHAWK_ROV_AVS_CONTROL_REG      (0x15 | MASTER_CPLD_IDX)
#define  ROV_MASK                         (0x0f)

#define DEBUG_LED_1_REG                   (0x16 | MASTER_CPLD_IDX)
#define  DEBUG_LED_MASK                   (0x07)

#define MASTER_CPLD_REVISION_REG          (0x17 | MASTER_CPLD_IDX)

#define SFP28_QSFP28_I2C_MUX_SELECT_REG   (0x18 | MASTER_CPLD_IDX)
#define  SFP28_QSFP28_I2C_MUX6_MASK       BIT(5)
#define  SFP28_QSFP28_I2C_MUX5_MASK       BIT(4)
#define  SFP28_QSFP28_I2C_MUX4_MASK       BIT(3)
#define  SFP28_QSFP28_I2C_MUX3_MASK       BIT(2)
#define  SFP28_QSFP28_I2C_MUX2_MASK       BIT(1)
#define  SFP28_QSFP28_I2C_MUX1_MASK       BIT(0)

/* Slave CPLD register definitions */
#define SLAVE_CPLD_REVISION_REG           (0x01 | SLAVE_CPLD_IDX)

#define INTERRUPT_2_REG                   (0x02 | SLAVE_CPLD_IDX)

#define INTERRUPT_2_MASK_REG              (0x03 | SLAVE_CPLD_IDX)

#define LED_CONTROL_1_REG                 (0x04 | SLAVE_CPLD_IDX)
#define  FRONT_FAN_LED_MASK               (0xc0)
#define  FRONT_FAN_LED_OFF                (0x00)
#define  FRONT_FAN_LED_AMBER              (0x40)
#define  FRONT_FAN_LED_GREEN              (0x80)
#define  FRONT_FAN_LED_BLINKING_YELLOW    (0xc0)
#define  SYSTEM_LED_MASK                  (0x30)
#define  SYSTEM_LED_BLINKING_GREEN        (0x00)
#define  SYSTEM_LED_GREEN                 (0x10)
#define  SYSTEM_LED_AMBER                 (0x20)
#define  SYSTEM_LED_BLINKING_AMBER        (0x30)
#define  POWER_LED_MASK                   (0x06)
#define  POWER_LED_OFF                    (0x00)
#define  POWER_LED_AMBER                  (0x02)
#define  POWER_LED_GREEN                  (0x04)
#define  POWER_LED_BLINKING_AMBER         (0x06)

#define FAN_STATUS_1_REG                  (0x05 | SLAVE_CPLD_IDX)
#define  FAN_4_LED_MASK                   (0xC0)
#define  FAN_4_LED_OFF                    (0x00)
#define  FAN_4_LED_GREEN                  (0x40)
#define  FAN_4_LED_AMBER                  (0x80)
#define  FAN_3_LED_MASK                   (0x30)
#define  FAN_3_LED_OFF                    (0x00)
#define  FAN_3_LED_GREEN                  (0x10)
#define  FAN_3_LED_AMBER                  (0x20)
#define  FAN_2_LED_MASK                   (0x0C)
#define  FAN_2_LED_OFF                    (0x00)
#define  FAN_2_LED_GREEN                  (0x04)
#define  FAN_2_LED_AMBER                  (0x08)
#define  FAN_1_LED_MASK                   (0x03)
#define  FAN_1_LED_OFF                    (0x00)
#define  FAN_1_LED_GREEN                  (0x01)
#define  FAN_1_LED_AMBER                  (0x02)

#define FAN_STATUS_2_REG                  (0x06 | SLAVE_CPLD_IDX)
#define  FAN_ALERT_N_MASK                 (0x10)
#define  FAN_TRAY4_PRES_N_MASK            (0x08)
#define  FAN_TRAY3_PRES_N_MASK            (0x04)
#define  FAN_TRAY2_PRES_N_MASK            (0x02)
#define  FAN_TRAY1_PRES_N_MASK            (0x01)

#define EEPROM_WRITE_PROTECT_REG          (0x07 | SLAVE_CPLD_IDX)
#define  FAN4_EEPROM_WP_MASK              (0x08)
#define  FAN3_EEPROM_WP_MASK              (0x04)
#define  FAN2_EEPROM_WP_MASK              (0x02)
#define  FAN1_EEPROM_WP_MASK              (0x01)

/* Mod_ABS */
#define SFP28_8_1_MODULE_ABSENT_REG       (0x08 | SLAVE_CPLD_IDX)
#define  SFP28_MOD_P8_MASK                BIT(7)
#define  SFP28_MOD_P7_MASK                BIT(6)
#define  SFP28_MOD_P6_MASK                BIT(5)
#define  SFP28_MOD_P5_MASK                BIT(4)
#define  SFP28_MOD_P4_MASK                BIT(3)
#define  SFP28_MOD_P3_MASK                BIT(2)
#define  SFP28_MOD_P2_MASK                BIT(1)
#define  SFP28_MOD_P1_MASK                BIT(0)

#define SFP28_16_9_MODULE_ABSENT_REG      (0x09 | SLAVE_CPLD_IDX)
#define  SFP28_MOD_P16_MASK               BIT(7)
#define  SFP28_MOD_P15_MASK               BIT(6)
#define  SFP28_MOD_P14_MASK               BIT(5)
#define  SFP28_MOD_P13_MASK               BIT(4)
#define  SFP28_MOD_P12_MASK               BIT(3)
#define  SFP28_MOD_P11_MASK               BIT(2)
#define  SFP28_MOD_P10_MASK               BIT(1)
#define  SFP28_MOD_P9_MASK                BIT(0)

#define SFP28_24_17_MODULE_ABSENT_REG     (0x0a | SLAVE_CPLD_IDX)
#define  SFP28_MOD_P24_MASK               BIT(7)
#define  SFP28_MOD_P23_MASK               BIT(6)
#define  SFP28_MOD_P22_MASK               BIT(5)
#define  SFP28_MOD_P21_MASK               BIT(4)
#define  SFP28_MOD_P20_MASK               BIT(3)
#define  SFP28_MOD_P19_MASK               BIT(2)
#define  SFP28_MOD_P18_MASK               BIT(1)
#define  SFP28_MOD_P17_MASK               BIT(0)

#define SFP28_32_25_MODULE_ABSENT_REG     (0x0b | SLAVE_CPLD_IDX)
#define  SFP28_MOD_P32_MASK               BIT(7)
#define  SFP28_MOD_P31_MASK               BIT(6)
#define  SFP28_MOD_P30_MASK               BIT(5)
#define  SFP28_MOD_P29_MASK               BIT(4)
#define  SFP28_MOD_P28_MASK               BIT(3)
#define  SFP28_MOD_P27_MASK               BIT(2)
#define  SFP28_MOD_P26_MASK               BIT(1)
#define  SFP28_MOD_P25_MASK               BIT(0)

#define SFP28_36_33_MODULE_ABSENT_REG     (0x0c | SLAVE_CPLD_IDX)
#define  SFP28_MOD_P36_MASK               BIT(3)
#define  SFP28_MOD_P35_MASK               BIT(2)
#define  SFP28_MOD_P34_MASK               BIT(1)
#define  SFP28_MOD_P33_MASK               BIT(0)

/* Tx_Disable */
#define SFP28_8_1_MODULE_TX_DISABLE_REG   (0x0d | SLAVE_CPLD_IDX)
#define  SFP28_TXDIS_P8_MASK              BIT(7)
#define  SFP28_TXDIS_P7_MASK              BIT(6)
#define  SFP28_TXDIS_P6_MASK              BIT(5)
#define  SFP28_TXDIS_P5_MASK              BIT(4)
#define  SFP28_TXDIS_P4_MASK              BIT(3)
#define  SFP28_TXDIS_P3_MASK              BIT(2)
#define  SFP28_TXDIS_P2_MASK              BIT(1)
#define  SFP28_TXDIS_P1_MASK              BIT(0)

#define SFP28_16_9_MODULE_TX_DISABLE_REG  (0x0e | SLAVE_CPLD_IDX)
#define  SFP28_TXDIS_P16_MASK             BIT(7)
#define  SFP28_TXDIS_P15_MASK             BIT(6)
#define  SFP28_TXDIS_P14_MASK             BIT(5)
#define  SFP28_TXDIS_P13_MASK             BIT(4)
#define  SFP28_TXDIS_P12_MASK             BIT(3)
#define  SFP28_TXDIS_P11_MASK             BIT(2)
#define  SFP28_TXDIS_P10_MASK             BIT(1)
#define  SFP28_TXDIS_P9_MASK              BIT(0)

#define SFP28_24_17_MODULE_TX_DISABLE_REG (0x0f | SLAVE_CPLD_IDX)
#define  SFP28_TXDIS_P24_MASK             BIT(7)
#define  SFP28_TXDIS_P23_MASK             BIT(6)
#define  SFP28_TXDIS_P22_MASK             BIT(5)
#define  SFP28_TXDIS_P21_MASK             BIT(4)
#define  SFP28_TXDIS_P20_MASK             BIT(3)
#define  SFP28_TXDIS_P19_MASK             BIT(2)
#define  SFP28_TXDIS_P18_MASK             BIT(1)
#define  SFP28_TXDIS_P17_MASK             BIT(0)

#define SFP28_32_25_MODULE_TX_DISABLE_REG (0x10 | SLAVE_CPLD_IDX)
#define  SFP28_TXDIS_P32_MASK             BIT(7)
#define  SFP28_TXDIS_P31_MASK             BIT(6)
#define  SFP28_TXDIS_P30_MASK             BIT(5)
#define  SFP28_TXDIS_P29_MASK             BIT(4)
#define  SFP28_TXDIS_P28_MASK             BIT(3)
#define  SFP28_TXDIS_P27_MASK             BIT(2)
#define  SFP28_TXDIS_P26_MASK             BIT(1)
#define  SFP28_TXDIS_P25_MASK             BIT(0)

#define SFP28_36_33_MODULE_TX_DISABLE_REG (0x11 | SLAVE_CPLD_IDX)
#define  SFP28_TXDIS_P36_MASK             BIT(3)
#define  SFP28_TXDIS_P35_MASK             BIT(2)
#define  SFP28_TXDIS_P34_MASK             BIT(1)
#define  SFP28_TXDIS_P33_MASK             BIT(0)

/* Rx_LOS */
#define SFP28_8_1_MODULE_RX_LOS_REG       (0x12 | SLAVE_CPLD_IDX)
#define  SFP28_RXLOS_P8_MASK              BIT(7)
#define  SFP28_RXLOS_P7_MASK              BIT(6)
#define  SFP28_RXLOS_P6_MASK              BIT(5)
#define  SFP28_RXLOS_P5_MASK              BIT(4)
#define  SFP28_RXLOS_P4_MASK              BIT(3)
#define  SFP28_RXLOS_P3_MASK              BIT(2)
#define  SFP28_RXLOS_P2_MASK              BIT(1)
#define  SFP28_RXLOS_P1_MASK              BIT(0)

#define SFP28_16_9_MODULE_RX_LOS_REG      (0x13 | SLAVE_CPLD_IDX)
#define  SFP28_RXLOS_P16_MASK             BIT(7)
#define  SFP28_RXLOS_P15_MASK             BIT(6)
#define  SFP28_RXLOS_P14_MASK             BIT(5)
#define  SFP28_RXLOS_P13_MASK             BIT(4)
#define  SFP28_RXLOS_P12_MASK             BIT(3)
#define  SFP28_RXLOS_P11_MASK             BIT(2)
#define  SFP28_RXLOS_P10_MASK             BIT(1)
#define  SFP28_RXLOS_P9_MASK              BIT(0)

#define SFP28_24_17_MODULE_RX_LOS_REG     (0x14 | SLAVE_CPLD_IDX)
#define  SFP28_RXLOS_P24_MASK             BIT(7)
#define  SFP28_RXLOS_P23_MASK             BIT(6)
#define  SFP28_RXLOS_P22_MASK             BIT(5)
#define  SFP28_RXLOS_P21_MASK             BIT(4)
#define  SFP28_RXLOS_P20_MASK             BIT(3)
#define  SFP28_RXLOS_P19_MASK             BIT(2)
#define  SFP28_RXLOS_P18_MASK             BIT(1)
#define  SFP28_RXLOS_P17_MASK             BIT(0)

#define SFP28_32_25_MODULE_RX_LOS_REG     (0x15 | SLAVE_CPLD_IDX)
#define  SFP28_RXLOS_P32_MASK             BIT(7)
#define  SFP28_RXLOS_P31_MASK             BIT(6)
#define  SFP28_RXLOS_P30_MASK             BIT(5)
#define  SFP28_RXLOS_P29_MASK             BIT(4)
#define  SFP28_RXLOS_P28_MASK             BIT(3)
#define  SFP28_RXLOS_P27_MASK             BIT(2)
#define  SFP28_RXLOS_P26_MASK             BIT(1)
#define  SFP28_RXLOS_P25_MASK             BIT(0)

#define SFP28_36_33_MODULE_RX_LOS_REG     (0x16 | SLAVE_CPLD_IDX)
#define  SFP28_RXLOS_P36_MASK             BIT(3)
#define  SFP28_RXLOS_P35_MASK             BIT(2)
#define  SFP28_RXLOS_P34_MASK             BIT(1)
#define  SFP28_RXLOS_P33_MASK             BIT(0)

/* Tx_Fault */
#define SFP28_8_1_MODULE_TX_FAULT_REG     (0x17 | SLAVE_CPLD_IDX)
#define  SFP28_TXFAULT_P8_MASK            BIT(7)
#define  SFP28_TXFAULT_P7_MASK            BIT(6)
#define  SFP28_TXFAULT_P6_MASK            BIT(5)
#define  SFP28_TXFAULT_P5_MASK            BIT(4)
#define  SFP28_TXFAULT_P4_MASK            BIT(3)
#define  SFP28_TXFAULT_P3_MASK            BIT(2)
#define  SFP28_TXFAULT_P2_MASK            BIT(1)
#define  SFP28_TXFAULT_P1_MASK            BIT(0)

#define SFP28_16_9_MODULE_TX_FAULT_REG    (0x18 | SLAVE_CPLD_IDX)
#define  SFP28_TXFAULT_P16_MASK           BIT(7)
#define  SFP28_TXFAULT_P15_MASK           BIT(6)
#define  SFP28_TXFAULT_P14_MASK           BIT(5)
#define  SFP28_TXFAULT_P13_MASK           BIT(4)
#define  SFP28_TXFAULT_P12_MASK           BIT(3)
#define  SFP28_TXFAULT_P11_MASK           BIT(2)
#define  SFP28_TXFAULT_P10_MASK           BIT(1)
#define  SFP28_TXFAULT_P9_MASK            BIT(0)

#define SFP28_24_17_MODULE_TX_FAULT_REG   (0x19 | SLAVE_CPLD_IDX)
#define  SFP28_TXFAULT_P24_MASK           BIT(7)
#define  SFP28_TXFAULT_P23_MASK           BIT(6)
#define  SFP28_TXFAULT_P22_MASK           BIT(5)
#define  SFP28_TXFAULT_P21_MASK           BIT(4)
#define  SFP28_TXFAULT_P20_MASK           BIT(3)
#define  SFP28_TXFAULT_P19_MASK           BIT(2)
#define  SFP28_TXFAULT_P18_MASK           BIT(1)
#define  SFP28_TXFAULT_P17_MASK           BIT(0)

#define SFP28_32_25_MODULE_TX_FAULT_REG   (0x1a | SLAVE_CPLD_IDX)
#define  SFP28_TXFAULT_P32_MASK           BIT(7)
#define  SFP28_TXFAULT_P31_MASK           BIT(6)
#define  SFP28_TXFAULT_P30_MASK           BIT(5)
#define  SFP28_TXFAULT_P29_MASK           BIT(4)
#define  SFP28_TXFAULT_P28_MASK           BIT(3)
#define  SFP28_TXFAULT_P27_MASK           BIT(2)
#define  SFP28_TXFAULT_P26_MASK           BIT(1)
#define  SFP28_TXFAULT_P25_MASK           BIT(0)

#define SFP28_36_33_MODULE_TX_FAULT_REG   (0x1b | SLAVE_CPLD_IDX)
#define  SFP28_TXFAULT_P36_MASK           BIT(3)
#define  SFP28_TXFAULT_P35_MASK           BIT(2)
#define  SFP28_TXFAULT_P34_MASK           BIT(1)
#define  SFP28_TXFAULT_P33_MASK           BIT(0)

#endif  /* DELTA_AG5648V1_CPLD_H__ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Accton AS5835 CPLD Definitions
 *
 * Copyright (C) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
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

#ifndef ACCTON_AS5835_CPLD_H__
#define ACCTON_AS5835_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * This family of platforms has five CPLD devices
 *
 *------------------------------------------------------------------------------
 */

#define AS5835_54X_PLATFORM_NAME "as5835_54x_platform"
#define AS5835_54T_PLATFORM_NAME "as5835_54t_platform"

#define AS5835_CPUCPLD_NAME	 "as5835_cpucpld"
#define AS5835_CPLD1_NAME	 "as5835_cpld1"
#define AS5835_CPLD2_NAME	 "as5835_cpld2"
#define AS5835_CPLD3_NAME	 "as5835_cpld3"
#define AS5835_CPLD4_NAME	 "as5835_cpld4"

/*------------------------------------------------------------------------------
 *
 *			       CPU CPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define ACCTON_AS5835_CPUCPLD_BOARD_INFO_REG				0x00
#  define ACCTON_AS5835_CPUCPLD_CPU_ID_BIT				4
#  define ACCTON_AS5835_CPUCPLD_PCB_VERSION_MSB				2
#  define ACCTON_AS5835_CPUCPLD_PCB_VERSION_LSB				0

#define ACCTON_AS5835_CPUCPLD_VERSION_REG				0x01
#  define ACCTON_AS5835_CPUCPLD_VERSION_MSB				7
#  define ACCTON_AS5835_CPUCPLD_VERSION_LSB				0

#define ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_1_REG			0x10
#  define ACCTON_AS5835_CPUCPLD_SPI_CS_SELECT_BIT			4

#define ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG			0x11
#  define ACCTON_AS5835_CPUCPLD_WATCHDOG_ENABLE_BIT			4
#  define ACCTON_AS5835_CPUCPLD_UPDATE_BOOT_DEV_SEL_BIT			3
#  define ACCTON_AS5835_CPUCPLD_BOOT_DEV_SEL_BIT			2
#  define ACCTON_AS5835_CPUCPLD_WATCHDOG_TRIGGER_BIT			1
#  define ACCTON_AS5835_CPUCPLD_TEST_MODE_BIT				0

/* note: the following register will automatically return to "H" after 500ms */
#define ACCTON_AS5835_CPUCPLD_SYSTEM_STATE_CONTROL_REG			0x20
#  define ACCTON_AS5835_CPUCPLD_SYSTEM_SLEEP_BIT			3
#  define ACCTON_AS5835_CPUCPLD_SYSTEM_RESET_BIT			2
#  define ACCTON_AS5835_CPUCPLD_SYSTEM_POWER_OFF_BIT			1
#  define ACCTON_AS5835_CPUCPLD_SYSTEM_POWER_RESET_BIT			0

/* note: the following register will automatically return to "H" after 500ms */
#define ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG			0x21
#  define ACCTON_AS5835_CPUCPLD_BMC_LPC_DEBUG_RST_BIT			6
#  define ACCTON_AS5835_CPUCPLD_TPM_RESET_BIT				5
#  define ACCTON_AS5835_CPUCPLD_LTB_RESET_BIT				4
#  define ACCTON_AS5835_CPUCPLD_M_2_SSD_2_RESET_BIT			3
#  define ACCTON_AS5835_CPUCPLD_M_2_SSD_1_RESET_BIT			2
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_JTAG_RESET_BIT		1
#  define ACCTON_AS5835_CPUCPLD_PCA9548_RESET_BIT			0

/* note: the following register will automatically return to "H" after 500ms */
#define ACCTON_AS5835_CPUCPLD_RESET_DEVICE_2_REG			0x22
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_RESET_BIT		2
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_MAX_RESET_BIT		1
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_P1014_RESET_BIT		0

#define ACCTON_AS5835_CPUCPLD_RESET_DEVICE_3_REG			0x23
#  define ACCTON_AS5835_CPUCPLD_POWER_BUTTON_BIT			1
#  define ACCTON_AS5835_CPUCPLD_RESET_BUTTON_BIT			0

#define ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG			0x30
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC_BIT			4
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_CPU_BIT			3
#  define ACCTON_AS5835_CPUCPLD_NMI_EVENT_FROM_BMC_BIT			2
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_CPLD23_INTERRUPT_BIT		1
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_INTERRUPT_BIT	0

#define ACCTON_AS5835_CPUCPLD_INTERRUPT_IF_REG				0x31
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC_BIT			4
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_CPU_BIT			3

#define ACCTON_AS5835_CPUCPLD_INTERRUPT_MASK_REG			0x32
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC_OR_CPU_MASK_MSB	4
#  define ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC_OR_CPU_MASK_LSB	3
#  define ACCTON_AS5835_CPUCPLD_NMI_EVENT_FROM_BMC_MASK_BIT		2
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_CPLD23_INTERRUPT_MASK_BIT	1
#  define ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_INTERRUPT_MASK_BIT	0

#define ACCTON_AS5835_CPUCPLD_UART_SELECTION_REG			0x40
#  define ACCTON_AS5835_CPUCPLD_UART1_SELECTION_BIT			1
#  define ACCTON_AS5835_CPUCPLD_UART0_SELECTION_BIT			0

#define ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG			0x41
#  define ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVNN_BIT			4
#  define ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVCCP_BIT			3
#  define ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVCCRAM_BIT		2
#  define ACCTON_AS5835_CPUCPLD_CPU_PROC_HOT_BIT			1
#  define ACCTON_AS5835_CPUCPLD_CPU_THERMAL_TRIP_BIT			0

#define ACCTON_AS5835_CPUCPLD_BMC_STATUS_REG				0x42
#  define ACCTON_AS5835_CPUCPLD_BMC_USB1_PWRFAULT_BIT			1
#  define ACCTON_AS5835_CPUCPLD_BMC_USB1_DRVVBUS_BIT			0

#define ACCTON_AS5835_CPUCPLD_BMC_CONTROL_SPI_FLASH_MASK_REG		0x43
#  define ACCTON_AS5835_CPUCPLD_BMC_CONTROL_SPI_SOURCE_MASK_BIT		0

#define ACCTON_AS5835_CPUCPLD_USB_PROTECT_REG				0x44
#  define ACCTON_AS5835_CPUCPLD_USB3_OC_BIT				1
#  define ACCTON_AS5835_CPUCPLD_USB1_PWRFAULT_BIT			0

/*------------------------------------------------------------------------------
 *
 *				 CPLD1 Registers
 *
 *-----------------------------------------------------------------------------
 */

#define ACCTON_AS5835_CPLD1_BOARD_ID_REG				0x00
#  define ACCTON_AS5835_CPLD1_BOARD_ID_MSB				3
#  define ACCTON_AS5835_CPLD1_BOARD_ID_LSB				0

#define ACCTON_AS5835_CPLD1_VERSION_REG					0x01
#  define ACCTON_AS5835_CPLD1_VERSION_MSB				3
#  define ACCTON_AS5835_CPLD1_VERSION_LSB				0

#define ACCTON_AS5835_CPLD1_PSU_STATUS_REG				0x02
#  define ACCTON_AS5835_CPLD1_PSU2_AC_ALERT_BIT				6
#  define ACCTON_AS5835_CPLD1_PSU2_12V_PG_BIT				5
#  define ACCTON_AS5835_CPLD1_PSU2_PRESENT_BIT				4
#  define ACCTON_AS5835_CPLD1_PSU1_AC_ALERT_BIT				2
#  define ACCTON_AS5835_CPLD1_PSU1_12V_PG_BIT				1
#  define ACCTON_AS5835_CPLD1_PSU1_PRESENT_BIT				0

#define ACCTON_AS5835_CPLD1_VID_STATUS_REG				0x03
#  define ACCTON_AS5835_CPLD1_SVID_STATUS_MSB				7
#  define ACCTON_AS5835_CPLD1_SVID_STATUS_LSB				0

#define ACCTON_AS5835_CPLD1_RESET_CONTROL_REG				0x04
#  define ACCTON_AS5835_CPLD1_88E1512_PHY_RST_BIT			5
#  define ACCTON_AS5835_CPLD1_RESET_PCIE_BIT				4
#  define ACCTON_AS5835_CPLD1_RESET_MAC_BIT				3
#  define ACCTON_AS5835_CPLD1_I210_PHY_RST_BIT				2
#  define ACCTON_AS5835_CPLD1_I2C_SW_RESET_N_BIT			1

#define ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_12_9_REG		0x05
#  define ACCTON_AS5835_CPLD1_PHY12_RESET_BIT				3
#  define ACCTON_AS5835_CPLD1_PHY11_RESET_BIT				2
#  define ACCTON_AS5835_CPLD1_PHY10_RESET_BIT				1
#  define ACCTON_AS5835_CPLD1_PHY9_RESET_BIT				0

#define ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG		0x06
#  define ACCTON_AS5835_CPLD1_PHY8_RESET_BIT				7
#  define ACCTON_AS5835_CPLD1_PHY7_RESET_BIT				6
#  define ACCTON_AS5835_CPLD1_PHY6_RESET_BIT				5
#  define ACCTON_AS5835_CPLD1_PHY5_RESET_BIT				4
#  define ACCTON_AS5835_CPLD1_PHY4_RESET_BIT				3
#  define ACCTON_AS5835_CPLD1_PHY3_RESET_BIT				2
#  define ACCTON_AS5835_CPLD1_PHY2_RESET_BIT				1
#  define ACCTON_AS5835_CPLD1_PHY1_RESET_BIT				0

#define ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG			0x07
#  define ACCTON_AS5835_CPLD1_LM75_INT_CPU_BIT				7
#  define ACCTON_AS5835_CPLD1_MAC_INT_BIT				5
#  define ACCTON_AS5835_CPLD1_CPLD23_INT_L_BIT				4
#  define ACCTON_AS5835_CPLD1_88E1512_INT_BIT				3
#  define ACCTON_AS5835_CPLD1_LM75_INT_CH3_BIT				2
#  define ACCTON_AS5835_CPLD1_LM75_INT_CH2_BIT				1
#  define ACCTON_AS5835_CPLD1_LM75_INT_CH1_BIT				0

#define ACCTON_AS5835_CPLD1_SYSTEM_LED_STATUS_REG			0x0a
#  define ACCTON_AS5835_CPLD1_LOC_B_MSB					5
#  define ACCTON_AS5835_CPLD1_LOC_B_LSB					4
#  define ACCTON_AS5835_CPLD1_DIAG_MSB					3
#  define ACCTON_AS5835_CPLD1_DIAG_LSB					2
#  define ACCTON_AS5835_CPLD1_FAN_MSB					1
#  define ACCTON_AS5835_CPLD1_FAN_LSB					0

#define ACCTON_AS5835_CPLD1_PSU_LED_STATUS_REG				0x0b
#  define ACCTON_AS5835_CPLD1_PSU2_MSB					3
#  define ACCTON_AS5835_CPLD1_PSU2_LSB					2
#  define ACCTON_AS5835_CPLD1_PSU1_MSB					1
#  define ACCTON_AS5835_CPLD1_PSU1_LSB					0

#define ACCTON_AS5835_CPLD1_OTHER_1_REG					0x0e
#  define ACCTON_AS5835_MDC_MDIO_SEL1_BIT				3
#  define ACCTON_AS5835_MDC_MDIO_SEL0_BIT				2
#  define ACCTON_AS5835_CPLD_SELECT_BIT					0

/* note: the spec says 0x15...	but is it really supposed to be 0x0f ? */
#define ACCTON_AS5835_CPLD1_OTHER_2_REG					0x15
#  define ACCTON_AS5835_PCIE_WAKE_L_BIT					1
#  define ACCTON_AS5835_PCIE_INTR_L_BIT					0

/*------------------------------------------------------------------------------
 *
 *				 CPLD2 Registers
 *
 *------------------------------------------------------------------------------
 */

#define ACCTON_AS5835_CPLD2_VERSION_REG					0x01
#  define ACCTON_AS5835_CPLD2_VERSION_MSB				3
#  define ACCTON_AS5835_CPLD2_VERSION_LSB				0

#define NUM_CPLD2_SFP_REGS						5

#define ACCTON_AS5835_CPLD2_SFP_PRESENT_REG				0x08
#define ACCTON_AS5835_CPLD2_SFP_TX_FAULT_REG				0x0d
#define ACCTON_AS5835_CPLD2_SFP_TX_DISABLE_REG				0x12
#define ACCTON_AS5835_CPLD2_SFP_RX_LOS_REG				0x17

/*------------------------------------------------------------------------------
 *
 *				 CPLD3 Registers
 *
 *------------------------------------------------------------------------------
 */

#define ACCTON_AS5835_CPLD3_VERSION_REG					0x01
#  define ACCTON_AS5835_CPLD3_VERSION_MSB				3
#  define ACCTON_AS5835_CPLD3_VERSION_LSB				0

#define NUM_CPLD3_SFP_REGS						2

#define ACCTON_AS5835_CPLD3_SFP_PRESENT_REG				0x06
#define ACCTON_AS5835_CPLD3_SFP_TX_FAULT_REG				0x09
#define ACCTON_AS5835_CPLD3_SFP_TX_DISABLE_REG				0x0c
#define ACCTON_AS5835_CPLD3_SFP_RX_LOS_REG				0x0f

#define ACCTON_AS5835_CPLD3_QSFP_INT_REG				0x12
#  define ACCTON_AS5835_CPLD3_QSFP_INT_MSB				5
#  define ACCTON_AS5835_CPLD3_QSFP_INT_LSB				0

#define ACCTON_AS5835_CPLD3_QSFP_MODULE_INT_REG				0x13
#  define ACCTON_AS5835_CPLD3_QSFP_MODULE_INT_MSB			5
#  define ACCTON_AS5835_CPLD3_QSFP_MODULE_INT_LSB			0

#define ACCTON_AS5835_CPLD3_QSFP_PRESENT_REG				0x14
#  define ACCTON_AS5835_CPLD3_QSFP_PRESENT_MSB				5
#  define ACCTON_AS5835_CPLD3_QSFP_PRESENT_LSB				0

#define ACCTON_AS5835_CPLD3_QSFP_MOD_RST_REG				0x15
#  define ACCTON_AS5835_CPLD3_QSFP_MOD_RST_MSB				5
#  define ACCTON_AS5835_CPLD3_QSFP_MOD_RST_LSB				0

#define ACCTON_AS5835_CPLD3_QSFP_LPMODE_REG				0x16
#  define ACCTON_AS5835_CPLD3_QSFP_LPMODE_MSB				5
#  define ACCTON_AS5835_CPLD3_QSFP_LPMODE_LSB				0

/*------------------------------------------------------------------------------
 *
 *				 CPLD4 Registers
 *
 *------------------------------------------------------------------------------
 */

#define ACCTON_AS5835_CPLD4_VERSION_REG					0x01
#  define ACCTON_AS5835_CPLD4_VERSION_MSB				3
#  define ACCTON_AS5835_CPLD4_VERSION_LSB				0

#define ACCTON_AS5835_CPLD4_FAN_PRESENT_REG				0x02
#  define ACCTON_AS5835_CPLD4_FAN5_PRESENT_BIT				4
#  define ACCTON_AS5835_CPLD4_FAN4_PRESENT_BIT				3
#  define ACCTON_AS5835_CPLD4_FAN3_PRESENT_BIT				2
#  define ACCTON_AS5835_CPLD4_FAN2_PRESENT_BIT				1
#  define ACCTON_AS5835_CPLD4_FAN1_PRESENT_BIT				0

#define ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG				0x03
#  define ACCTON_AS5835_CPLD4_FAN5_DIRECTION_BIT			4
#  define ACCTON_AS5835_CPLD4_FAN4_DIRECTION_BIT			3
#  define ACCTON_AS5835_CPLD4_FAN3_DIRECTION_BIT			2
#  define ACCTON_AS5835_CPLD4_FAN2_DIRECTION_BIT			1
#  define ACCTON_AS5835_CPLD4_FAN1_DIRECTION_BIT			0

#define ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG			0x04
#  define ACCTON_AS5835_CPLD4_FAN5_FAULT_BIT				4
#  define ACCTON_AS5835_CPLD4_FAN4_FAULT_BIT				3
#  define ACCTON_AS5835_CPLD4_FAN3_FAULT_BIT				2
#  define ACCTON_AS5835_CPLD4_FAN2_FAULT_BIT				1
#  define ACCTON_AS5835_CPLD4_FAN1_FAULT_BIT				0

#define ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG			0x05
#  define ACCTON_AS5835_CPLD4_FANR5_FAULT_BIT				4
#  define ACCTON_AS5835_CPLD4_FANR4_FAULT_BIT				3
#  define ACCTON_AS5835_CPLD4_FANR3_FAULT_BIT				2
#  define ACCTON_AS5835_CPLD4_FANR2_FAULT_BIT				1
#  define ACCTON_AS5835_CPLD4_FANR1_FAULT_BIT				0

#define ACCTON_AS5835_CPLD4_FAN_PWM_CYCLE_STATUS_REG			0x06
#  define ACCTON_AS5835_CPLD4_PWM_MSB					4
#  define ACCTON_AS5835_CPLD4_PWM_LSB					0

#define ACCTON_AS5835_CPLD4_FAN1_SPEED_REG				0x07
#  define ACCTON_AS5835_CPLD4_FAN1_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FAN1_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN2_SPEED_REG				0x08
#  define ACCTON_AS5835_CPLD4_FAN2_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FAN2_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN3_SPEED_REG				0x09
#  define ACCTON_AS5835_CPLD4_FAN3_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FAN3_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN4_SPEED_REG				0x0a
#  define ACCTON_AS5835_CPLD4_FAN4_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FAN4_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN5_SPEED_REG				0x0b
#  define ACCTON_AS5835_CPLD4_FAN5_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FAN5_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FANR1_SPEED_REG				0x0c
#  define ACCTON_AS5835_CPLD4_FANR1_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FANR1_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FANR2_SPEED_REG				0x0d
#  define ACCTON_AS5835_CPLD4_FANR2_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FANR2_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FANR3_SPEED_REG				0x0e
#  define ACCTON_AS5835_CPLD4_FANR3_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FANR3_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FANR4_SPEED_REG				0x0f
#  define ACCTON_AS5835_CPLD4_FANR4_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FANR4_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FANR5_SPEED_REG				0x10
#  define ACCTON_AS5835_CPLD4_FANR5_SPEED_MSB				4
#  define ACCTON_AS5835_CPLD4_FANR5_SPEED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN_4_1_LED_REG				0x11
#  define ACCTON_AS5835_CPLD4_FAN4_LED_MSB				7
#  define ACCTON_AS5835_CPLD4_FAN4_LED_LSB				6
#  define ACCTON_AS5835_CPLD4_FAN3_LED_MSB				5
#  define ACCTON_AS5835_CPLD4_FAN3_LED_LSB				4
#  define ACCTON_AS5835_CPLD4_FAN2_LED_MSB				3
#  define ACCTON_AS5835_CPLD4_FAN2_LED_LSB				2
#  define ACCTON_AS5835_CPLD4_FAN1_LED_MSB				1
#  define ACCTON_AS5835_CPLD4_FAN1_LED_LSB				0

#define ACCTON_AS5835_CPLD4_FAN_5_LED_REG				0x12
#  define ACCTON_AS5835_CPLD4_FAN5_LED_MSB				1
#  define ACCTON_AS5835_CPLD4_FAN5_LED_LSB				0

#endif	/* ACCTON_AS5835_CPLD_H__ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Delta AGV848v1 CPLD Definitions
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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

#ifndef DELTA_AGV848V1_CPLD_H__
#define DELTA_AGV848V1_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * This platform has five CPLD devices
 *
 *------------------------------------------------------------------------------
 */

#define AGV848V1_PLATFORM_NAME "agv848v1_platform"
#define AGV848V1_CPUPLD_NAME   "agv848v1_cpupld"
#define AGV848V1_SWPLD1_NAME   "agv848v1_swpld1"
#define AGV848V1_SWPLD2_NAME   "agv848v1_swpld2"
#define AGV848V1_SWPLD3_NAME   "agv848v1_swpld3"
#define AGV848V1_SWPLD4_NAME   "agv848v1_swpld4"

/*------------------------------------------------------------------------------
 *
 * This platform has two i2c busses:
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 *  SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 *
 *------------------------------------------------------------------------------
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

	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	I2C_MUX3_BUS0,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	I2C_MUX4_BUS0,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	I2C_MUX5_BUS0,
	I2C_MUX5_BUS1,
	I2C_MUX5_BUS2,
	I2C_MUX5_BUS3,
	I2C_MUX5_BUS4,
	I2C_MUX5_BUS5,
	I2C_MUX5_BUS6,
	I2C_MUX5_BUS7,

	I2C_MUX6_BUS0,
	I2C_MUX6_BUS1,
	I2C_MUX6_BUS2,
	I2C_MUX6_BUS3,
	I2C_MUX6_BUS4,
	I2C_MUX6_BUS5,
	I2C_MUX6_BUS6,
	I2C_MUX6_BUS7,

	I2C_MUX7_BUS0,
	I2C_MUX7_BUS1,
	I2C_MUX7_BUS2,
	I2C_MUX7_BUS3,
	I2C_MUX7_BUS4,
	I2C_MUX7_BUS5,
	I2C_MUX7_BUS6,
	I2C_MUX7_BUS7,

	I2C_MUX8_BUS0,
	I2C_MUX8_BUS1,
	I2C_MUX8_BUS2,
	I2C_MUX8_BUS3,
	I2C_MUX8_BUS4,
	I2C_MUX8_BUS5,
	I2C_MUX8_BUS6,
	I2C_MUX8_BUS7,

	I2C_MUX9_BUS0,
	I2C_MUX9_BUS1,
	I2C_MUX9_BUS2,
	I2C_MUX9_BUS3,
	I2C_MUX9_BUS4,
	I2C_MUX9_BUS5,
	I2C_MUX9_BUS6,
	I2C_MUX9_BUS7,

	I2C_MUX10_BUS0,
	I2C_MUX10_BUS1,
	I2C_MUX10_BUS2,
	I2C_MUX10_BUS3,
	I2C_MUX10_BUS4,
	I2C_MUX10_BUS5,
	I2C_MUX10_BUS6,
	I2C_MUX10_BUS7,
};

/*------------------------------------------------------------------------------
 *
 *			       CPUPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AGV848V1_SYS_CPLD_REV_REG			                 0x00
#  define DELTA_AGV848V1_MJR_REV_MSB					 7
#  define DELTA_AGV848V1_MJR_REV_LSB					 4
#  define DELTA_AGV848V1_MNR_REV_MSB					 3
#  define DELTA_AGV848V1_MNR_REV_LSB					 0

#define DELTA_AGV848V1_SYS_CPLD_GPR_REG				         0x01
#  define DELTA_AGV848V1_SCRTCH_REG_MSB			                 7
#  define DELTA_AGV848V1_SCRTCH_REG_LSB			                 0

#define DELTA_AGV848V1_CPU_BRD_REV_TYPE_REG			         0x02
#  define DELTA_AGV848V1_BRD_REV_MSB			                 7
#  define DELTA_AGV848V1_BRD_REV_LSB			                 4
#  define DELTA_AGV848V1_BRD_TYPE_MSB				         3
#  define DELTA_AGV848V1_BRD_TYPE_LSB				         0

#define DELTA_AGV848V1_SYS_SRR_REG				         0x03
#  define DELTA_AGV848V1_SPI_CS_SEL_BIT			                 5
#  define DELTA_AGV848V1_RST_BIOS_SWITCH_BIT		                 4

#define DELTA_AGV848V1_SYS_EEPROM_WP_REG			         0x04
#  define DELTA_AGV848V1_SYSTM_ID_EEPROM_WP_BIT			         4
#  define DELTA_AGV848V1_SPI_WP_GBE_BIT				         3
#  define DELTA_AGV848V1_SPI_BIOS_WP_BIT			         2
#  define DELTA_AGV848V1_SPI_BAK_BIOS_WP_BIT			         1

#define DELTA_AGV848V1_SYS_IRQ_REG				         0x05
#  define DELTA_AGV848V1_LPC_CLK_FAIL_IRQ_EN_BIT		         7
#  define DELTA_AGV848V1_VRHOT_VCCP_IRQ_EN_BIT			         6
#  define DELTA_AGV848V1_CPU_THERMTRIP_IRQ_EN_BIT		         5
#  define DELTA_AGV848V1_TEMP_ALERT_IRQ_EN_BIT			         4
#  define DELTA_AGV848V1_LPC_CLK_FAIL_IRQ_BIT			         3
#  define DELTA_AGV848V1_VRHOT_VCCP_IRQ_BIT			         2
#  define DELTA_AGV848V1_CPU_THERMTRIP_IRQ_BIT			         1
#  define DELTA_AGV848V1_TEMP_ALERT_IRQ_BIT				 0

#define DELTA_AGV848V1_SYS_WD_REG			                 0x06
#  define DELTA_AGV848V1_WD_TIMER_MSB			                 6
#  define DELTA_AGV848V1_WD_TIMER_LSB			                 4
#  define DELTA_AGV848V1_WD_EN_BIT			                 3
#  define DELTA_AGV848V1_WD_PUNCH_BIT			                 0

#define DELTA_AGV848V1_REBOOT_CAUSE_REG			                 0x08
#  define DELTA_AGV848V1_COLD_RESET_BIT			                 7
#  define DELTA_AGV848V1_WARM_RESET_BIT			                 6
#  define DELTA_AGV848V1_WD_FAIL_BIT			                 4
#  define DELTA_AGV848V1_BIOS_SWITCHOVER_BIT			         3
#  define DELTA_AGV848V1_BOOT_FAIL_BIT			                 2
#  define DELTA_AGV848V1_CPU_PWR_ERR_BIT			         0

#define DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG		                 0x09
#  define DELTA_AGV848V1_V1P5_EN_BIT			                 7
#  define DELTA_AGV848V1_PWR_VDDR_EN_BIT		                 6
#  define DELTA_AGV848V1_PWR_CORE_EN_BIT		                 5
#  define DELTA_AGV848V1_V1P1_EN_BIT			                 4
#  define DELTA_AGV848V1_V1P0_EN_BIT			                 3
#  define DELTA_AGV848V1_V3P3_EN_BIT			                 2
#  define DELTA_AGV848V1_REG_1V8_EN_BIT			                 1
#  define DELTA_AGV848V1_REG_1V35_EN_BIT		                 0

#define DELTA_AGV848V1_CPU_PWR_STATUS_1_REG		                 0x0a
#  define DELTA_AGV848V1_PG_DDR_VTT_BIT			                 7
#  define DELTA_AGV848V1_PG_PVDDR_BIT			                 6
#  define DELTA_AGV848V1_PG_PWR_CORE_BIT		                 5
#  define DELTA_AGV848V1_PG_V1P1_BIT			                 4
#  define DELTA_AGV848V1_PG_V1P0_BIT			                 3
#  define DELTA_AGV848V1_PG_3V3_BIT			                 2
#  define DELTA_AGV848V1_PG_1V8_BIT			                 1
#  define DELTA_AGV848V1_PG_1V35_BIT			                 0

#define DELTA_AGV848V1_CPU_PWR_RAIL_MARGINS_CONTROL_REG	                 0x10
#  define DELTA_AGV848V1_CPU_PWR_MARGIN_MSB		                 1
#  define DELTA_AGV848V1_CPU_PWR_MARGIN_LSB		                 0

/*------------------------------------------------------------------------------
 *
 *			       SWPLD1 Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AGV848V1_SYSTEM_PRODUCT_ID_REG			         0x00
#  define DELTA_AGV848V1_BOARD_ID_MSB					 3
#  define DELTA_AGV848V1_BOARD_ID_LSB					 0

#define DELTA_AGV848V1_SYSTEM_DEVICE_VERSION_REG		         0x01
#  define DELTA_AGV848V1_DEV_VER_MSB					 7
#  define DELTA_AGV848V1_DEV_VER_LSB					 0

#define DELTA_AGV848V1_HW_RESET_CONTROL_REG				 0x02
#  define DELTA_AGV848V1_PCIE_PERST_N_BIT				 6
#  define DELTA_AGV848V1_CPLD_LPCRST_BIT				 5
#  define DELTA_AGV848V1_BMC_RSTN_BIT					 4
#  define DELTA_AGV848V1_RST_MAC_N_BIT					 3
#  define DELTA_AGV848V1_RST_UCONSOLE_BIT				 2
#  define DELTA_AGV848V1_OOB_PHY_RSTN_I210_AT_BIT			 1
#  define DELTA_AGV848V1_OOB_RST_USB_HUB_BIT				 0

#define DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG			         0x03
#  define DELTA_AGV848V1_CPLD_SWPLD2_INT_BIT				 7
#  define DELTA_AGV848V1_CPLD_SWPLD3_INT_BIT				 6
#  define DELTA_AGV848V1_PCIE_INT_BIT					 5
#  define DELTA_AGV848V1_PS2_PWR_INT_BIT				 4
#  define DELTA_AGV848V1_PS1_PWR_INT_BIT				 3
#  define DELTA_AGV848V1_FANB_THERMAL_INT_BIT				 2
#  define DELTA_AGV848V1_FANB_ALERT_BIT					 1
#  define DELTA_AGV848V1_SMB_ALERT_BIT					 0

#define DELTA_AGV848V1_HW_INTERRUPT_MASK_REG			         0x04
#  define DELTA_AGV848V1_CPLD2_INT_MASK_BIT				 7
#  define DELTA_AGV848V1_CPLD3_INT_MASK_BIT				 6
#  define DELTA_AGV848V1_PCIE_INTR_MASK_BIT				 5
#  define DELTA_AGV848V1_PS2_PWR_INT_MASK_BIT				 4
#  define DELTA_AGV848V1_PS1_PWR_INT_MASK_BIT				 3
#  define DELTA_AGV848V1_FANB_THERMAL_INTN_MASK_BIT			 2
#  define DELTA_AGV848V1_FANB_ALERTN_MASK_BIT				 1
#  define DELTA_AGV848V1_SMB_ALERT_MASK_BIT				 0

#define DELTA_AGV848V1_HW_MISC_CONTROL_REG			         0x05
#  define DELTA_AGV848V1_EEPROM_WP_BIT					 5
#  define DELTA_AGV848V1_OOB_DEV_OFF_N_BIT				 4
#  define DELTA_AGV848V1_PCIE_WAKE_N_BIT				 3
#  define DELTA_AGV848V1_OOB_PCIE_WAKE_N_BIT				 2
#  define DELTA_AGV848V1_OOB_RS232_INVALID_L_BIT			 1
#  define DELTA_AGV848V1_USB_SUSPEND_BIT				 0

#define DELTA_AGV848V1_SYS_LED_CONTROL_1_REG			         0x06
#  define DELTA_AGV848V1_STATUS_LED_MSB					 6
#  define DELTA_AGV848V1_STATUS_LED_LSB					 4
#  define DELTA_AGV848V1_FAN_LED_MSB					 2
#  define DELTA_AGV848V1_FAN_LED_LSB					 0

#define DELTA_AGV848V1_SYS_LED_CONTROL_2_REG			         0x07
#  define DELTA_AGV848V1_PWR_LED_MSB					 6
#  define DELTA_AGV848V1_PWR_LED_LSB					 4
#  define DELTA_AGV848V1_RPS_LED_MSB					 2
#  define DELTA_AGV848V1_RPS_LED_LSB					 0

#define DELTA_AGV848V1_FAN_TRAY_LED_CONTROL_REG			         0x08
#  define DELTA_AGV848V1_FAN1_LED_MSB					 7
#  define DELTA_AGV848V1_FAN1_LED_LSB					 6
#  define DELTA_AGV848V1_FAN2_LED_MSB					 5
#  define DELTA_AGV848V1_FAN2_LED_LSB					 4
#  define DELTA_AGV848V1_FAN3_LED_MSB					 3
#  define DELTA_AGV848V1_FAN3_LED_LSB					 2
#  define DELTA_AGV848V1_FAN4_LED_MSB					 1
#  define DELTA_AGV848V1_FAN4_LED_LSB					 0

#define DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_1_REG		         0x09
#  define DELTA_AGV848V1_PCA_RST_FAN_N_BIT				 1
#  define DELTA_AGV848V1_PCA_RST_I2C1_BIT				 0

#define DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG		         0x0a
#  define DELTA_AGV848V1_PCA_RST_I2C2_6_BIT				 6
#  define DELTA_AGV848V1_PCA_RST_I2C2_5_BIT				 5
#  define DELTA_AGV848V1_PCA_RST_I2C2_4_BIT				 4
#  define DELTA_AGV848V1_PCA_RST_I2C2_3_BIT				 3
#  define DELTA_AGV848V1_PCA_RST_I2C2_2_BIT				 2
#  define DELTA_AGV848V1_PCA_RST_I2C2_1_BIT				 1
#  define DELTA_AGV848V1_PCA_RST_I2C2_0_BIT				 0

#define DELTA_AGV848V1_POWER_SUPPLY_ENABLE_REG			         0x0b
#  define DELTA_AGV848V1_PS2_PSON_N_BIT					 1
#  define DELTA_AGV848V1_PS1_PSON_N_BIT					 0

#define DELTA_AGV848V1_POWER_SUPPLY_STATUS_REG			         0x0c
#  define DELTA_AGV848V1_PS1_PRESENT_N_BIT				 5
#  define DELTA_AGV848V1_PS1_PWOK_BIT					 4
#  define DELTA_AGV848V1_PS2_PRESENT_N_BIT				 1
#  define DELTA_AGV848V1_PS2_PWOK_BIT					 0

#define DELTA_AGV848V1_POWER_RAIL_ENABLE_REG			         0x0d
#  define DELTA_AGV848V1_OOB_USB2_POE_N_BIT				 5
#  define DELTA_AGV848V1_EN_VCC_3V3_BIT					 4
#  define DELTA_AGV848V1_EN_VCC_MAC_1V8_BIT				 3
#  define DELTA_AGV848V1_EN_VCC_MAC_1V2_BIT				 2
#  define DELTA_AGV848V1_EN_VCC_AVS_0V81_BIT				 1
#  define DELTA_AGV848V1_EN_VCC_MAC_0V8_BIT				 0

#define DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG			         0x0e
#  define DELTA_AGV848V1_OOB_USB2_OVC_BIT				 6
#  define DELTA_AGV848V1_PG_VCC_STB_5V0_BIT				 5
#  define DELTA_AGV848V1_PG_VCC_STB_3V3_BIT				 4
#  define DELTA_AGV848V1_PG_VCC_3V3_BIT					 3
#  define DELTA_AGV848V1_PG_BMC_1V15_BIT				 2
#  define DELTA_AGV848V1_PG_BMC_1V2_BIT					 1
#  define DELTA_AGV848V1_PG_BMC_2V5_BIT					 0

#define DELTA_AGV848V1_POWER_RAIL_STATUS_2_REG			         0x0f
#  define DELTA_AGV848V1_PG_VCC_AVS_0V81_BIT				 3
#  define DELTA_AGV848V1_PG_VCC_MAC_0V8_BIT				 2
#  define DELTA_AGV848V1_PG_VCC_MAC_1V2_BIT				 1
#  define DELTA_AGV848V1_PG_VCC_MAC_1V8_BIT				 0

#define DELTA_AGV848V1_POWER_RAIL_MARGINS_CONTROL_REG		         0x10
#  define DELTA_AGV848V1_SYS_PWR_1V_MAC_AVS_BIT				 7
#  define DELTA_AGV848V1_SYS_PWR_1V_MARGIN_MSB				 3
#  define DELTA_AGV848V1_SYS_PWR_1V_MARGIN_LSB				 2
#  define DELTA_AGV848V1_SYS_PWR_MARGIN_MSB				 1
#  define DELTA_AGV848V1_SYS_PWR_MARGIN_LSB				 0

#define DELTA_AGV848V1_HW_RESET_CONTROL_2_REG			         0x11
#  define DELTA_AGV848V1_RSTN_SWPLD2_BIT				 1
#  define DELTA_AGV848V1_RSTN_SWPLD3_BIT				 0

#define DELTA_AGV848V1_SFP28_TX_DISABLE_1_REG			         0x12
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_8_1_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_8_1_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_DISABLE_2_REG			         0x13
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_16_9_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_16_9_LSB			 0

#define DELTA_AGV848V1_BCM_AVS_VID_CONTROL_REG			         0x14
#  define DELTA_AGV848V1_VID_MSB					 7
#  define DELTA_AGV848V1_VID_LSB					 0

#define DELTA_AGV848V1_CPLD_MAC_AVS_REG				         0x16
#  define DELTA_AGV848V1_CPLD_MAC_AVS_MSB				 7
#  define DELTA_AGV848V1_CPLD_MAC_AVS_LSB				 0

#define DELTA_AGV848V1_SYSTEM_HW_VERSION_REG			         0xf6
#  define DELTA_AGV848V1_HW_VER_MSB					 7
#  define DELTA_AGV848V1_HW_VER_LSB					 0

#define DELTA_AGV848V1_SYSTEM_CPLD_VERSION_REG			         0xf7
#  define DELTA_AGV848V1_SWPLD1_VER_MSB					 7
#  define DELTA_AGV848V1_SWPLD1_VER_LSB					 0

/*------------------------------------------------------------------------------
 *
 *			       SWPLD2 Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AGV848V1_SFP28_TX_DISABLE_3_REG			         0x00
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_24_17_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_24_17_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_DISABLE_4_REG			         0x01
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_32_25_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_32_25_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_DISABLE_5_REG			         0x02
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_40_33_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_40_33_LSB			 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_1_REG		         0x03
#  define DELTA_AGV848V1_SFP28_MOD_ABS_8_1_MSB				 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_8_1_LSB				 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_2_REG		         0x04
#  define DELTA_AGV848V1_SFP28_MOD_ABS_16_9_MSB				 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_16_9_LSB				 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_3_REG		         0x05
#  define DELTA_AGV848V1_SFP28_MOD_ABS_24_17_MSB			 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_24_17_LSB			 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_4_REG		         0x06
#  define DELTA_AGV848V1_SFP28_MOD_ABS_32_25_MSB			 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_32_25_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_1_REG		         0x07
#  define DELTA_AGV848V1_SFP28_TX_FAULT_8_1_MSB				 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_8_1_LSB				 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_2_REG		         0x08
#  define DELTA_AGV848V1_SFP28_TX_FAULT_16_9_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_16_9_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_3_REG		         0x09
#  define DELTA_AGV848V1_SFP28_TX_FAULT_24_17_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_24_17_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_4_REG		         0x0a
#  define DELTA_AGV848V1_SFP28_TX_FAULT_32_25_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_32_25_LSB			 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_1_REG		         0x0b
#  define DELTA_AGV848V1_SFP28_RX_LOS_8_1_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_8_1_LSB				 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_2_REG		         0x0c
#  define DELTA_AGV848V1_SFP28_RX_LOS_16_9_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_16_9_LSB				 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_3_REG		         0x0d
#  define DELTA_AGV848V1_SFP28_RX_LOS_24_17_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_24_17_LSB				 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_4_REG		         0x0e
#  define DELTA_AGV848V1_SFP28_RX_LOS_32_25_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_32_25_LSB				 0

#define DELTA_AGV848V1_SWPLD2_VERSION_REG			         0x10
#  define DELTA_AGV848V1_SWPLD2_VER_MSB					 7
#  define DELTA_AGV848V1_SWPLD2_VER_LSB					 0

#define DELTA_AGV848V1_SFP28_LED_TEST_CONTROL_REG		         0x11
#  define DELTA_AGV848V1_SFP28_LEDCTRL_MSB				 2
#  define DELTA_AGV848V1_SFP28_LEDCTRL_LSB				 0

/*------------------------------------------------------------------------------
 *
 *			       SWPLD3 Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AGV848V1_SFP28_TX_DISABLE_6_REG			         0x00
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_48_41_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_DISABLE_48_41_LSB			 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_5_REG		         0x01
#  define DELTA_AGV848V1_SFP28_MOD_ABS_40_33_MSB			 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_40_33_LSB			 0

#define DELTA_AGV848V1_SFP28_ABSENT_STATUS_6_REG		         0x02
#  define DELTA_AGV848V1_SFP28_MOD_ABS_48_41_MSB			 7
#  define DELTA_AGV848V1_SFP28_MOD_ABS_48_41_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_5_REG		         0x03
#  define DELTA_AGV848V1_SFP28_TX_FAULT_40_33_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_40_33_LSB			 0

#define DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_6_REG		         0x04
#  define DELTA_AGV848V1_SFP28_TX_FAULT_48_41_MSB			 7
#  define DELTA_AGV848V1_SFP28_TX_FAULT_48_41_LSB			 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_5_REG		         0x05
#  define DELTA_AGV848V1_SFP28_RX_LOS_40_33_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_40_33_LSB				 0

#define DELTA_AGV848V1_SFP28_RX_LOS_STATUS_6_REG		         0x06
#  define DELTA_AGV848V1_SFP28_RX_LOS_48_41_MSB				 7
#  define DELTA_AGV848V1_SFP28_RX_LOS_48_41_LSB				 0

#define DELTA_AGV848V1_QSFP28_MODULE_RESET_REG			         0x07
#  define DELTA_AGV848V1_QSFP28_RSTN_56_49_MSB				 7
#  define DELTA_AGV848V1_QSFP28_RSTN_56_49_LSB				 0

#define DELTA_AGV848V1_QSFP28_ABSENT_STATUS_REG			         0x08
#  define DELTA_AGV848V1_QSFP28_MOD_ABS_56_49_MSB			 7
#  define DELTA_AGV848V1_QSFP28_MOD_ABS_56_49_LSB			 0

#define DELTA_AGV848V1_QSFP28_INTERRUPT_REG			         0x09
#  define DELTA_AGV848V1_QSFP28_MOD_INTN_56_49_MSB			 7
#  define DELTA_AGV848V1_QSFP28_MOD_INTN_56_49_LSB			 0

#define DELTA_AGV848V1_QSFP28_LP_MODE_REG			         0x0a
#  define DELTA_AGV848V1_QSFP28_LPMODE_EN_56_49_MSB			 7
#  define DELTA_AGV848V1_QSFP28_LPMODE_EN_56_49_LSB			 0

#define DELTA_AGV848V1_SWPLD3_VERSION_REG			         0x10
#  define DELTA_AGV848V1_SWPLD3_VER_MSB					 7
#  define DELTA_AGV848V1_SWPLD3_VER_LSB					 0

#define DELTA_AGV848V1_QSFP28_LED_TEST_CONTROL_REG		         0x11
#  define DELTA_AGV848V1_QSFP28_LEDCTRL_MSB				 2
#  define DELTA_AGV848V1_QSFP28_LEDCTRL_LSB				 0

/*------------------------------------------------------------------------------
 *
 *			       SWPLD4 Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AGV848V1_SWPLD4_VERSION_REG			         0x10
#  define DELTA_AGV848V1_SWPLD4_VER_MSB					 7
#  define DELTA_AGV848V1_SWPLD4_VER_LSB					 0

#define DELTA_AGV848V1_CPLD4_SFP28_LED_TEST_CONTROL_REG		         0x11
#  define DELTA_AGV848V1_SWPLD4_SFP28_LEDCTRL_MSB			 2
#  define DELTA_AGV848V1_SWPLD4_SFP28_LEDCTRL_LSB			 0

#endif	/* DELTA_AGV848V1_CPLD_H__ */

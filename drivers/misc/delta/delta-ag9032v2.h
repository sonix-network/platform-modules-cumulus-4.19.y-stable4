/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Delta AG9032v2 CPLD Definitions
 *
 * Copyright (C) 2018, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved.
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

#ifndef DELTA_AG9032V2_CPLD_H__
#define DELTA_AG9032V2_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * This platform has two CPLD devices and three virtual muxes
 *
 *------------------------------------------------------------------------------
 */

#define AG9032V2_PLATFORM_NAME   "ag9032v2_platform"
#define AG9032V2_SYSTEMCPLD_NAME "ag9032v2_systemcpld"
#define AG9032V2_MASTERCPLD_NAME "ag9032v2_mastercpld"
#define AG9032V2_SFFMUX_NAME     "ag9032v2_sffmux"
#define AG9032V2_SLAVE1CPLD_NAME "ag9032v2_slave1cpld"
#define AG9032V2_SLAVE2CPLD_NAME "ag9032v2_slave2cpld"

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
	CPLD_QSFP_MUX_BUS1 = 11,
	CPLD_QSFP_MUX_BUS2,
	CPLD_QSFP_MUX_BUS3,
	CPLD_QSFP_MUX_BUS4,
	CPLD_QSFP_MUX_BUS5,
	CPLD_QSFP_MUX_BUS6,
	CPLD_QSFP_MUX_BUS7,
	CPLD_QSFP_MUX_BUS8,
	CPLD_QSFP_MUX_BUS9,
	CPLD_QSFP_MUX_BUS10,
	CPLD_QSFP_MUX_BUS11,
	CPLD_QSFP_MUX_BUS12,
	CPLD_QSFP_MUX_BUS13,
	CPLD_QSFP_MUX_BUS14,
	CPLD_QSFP_MUX_BUS15,
	CPLD_QSFP_MUX_BUS16,
	CPLD_QSFP_MUX_BUS17,
	CPLD_QSFP_MUX_BUS18,
	CPLD_QSFP_MUX_BUS19,
	CPLD_QSFP_MUX_BUS20,
	CPLD_QSFP_MUX_BUS21,
	CPLD_QSFP_MUX_BUS22,
	CPLD_QSFP_MUX_BUS23,
	CPLD_QSFP_MUX_BUS24,
	CPLD_QSFP_MUX_BUS25,
	CPLD_QSFP_MUX_BUS26,
	CPLD_QSFP_MUX_BUS27,
	CPLD_QSFP_MUX_BUS28,
	CPLD_QSFP_MUX_BUS29,
	CPLD_QSFP_MUX_BUS30,
	CPLD_QSFP_MUX_BUS31,
	CPLD_QSFP_MUX_BUS32,
};

/*------------------------------------------------------------------------------
 *
 *			       Struct for CPLD mux control
 *
 *------------------------------------------------------------------------------
 */

struct cpld_item {
	struct i2c_client *cpld_client;
	struct mutex cpld_mutex; /* mutex for fanmux and psumux select */
};

/*------------------------------------------------------------------------------
 *
 *			       System CPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AG9032V2_SYS_CPLD_REV_REG			                 0x00
#  define DELTA_AG9032V2_MJR_REV_MSB					 7
#  define DELTA_AG9032V2_MJR_REV_LSB					 4
#  define DELTA_AG9032V2_MNR_REV_MSB					 3
#  define DELTA_AG9032V2_MNR_REV_LSB					 0

#define DELTA_AG9032V2_SYS_CPLD_GPR_REG				         0x01
#  define DELTA_AG9032V2_SCRTCH_REG_MSB			                 7
#  define DELTA_AG9032V2_SCRTCH_REG_LSB			                 0

#define DELTA_AG9032V2_CPU_BRD_REV_TYPE_REG			         0x02
#  define DELTA_AG9032V2_BRD_REV_MSB			                 7
#  define DELTA_AG9032V2_BRD_REV_LSB			                 4
#  define DELTA_AG9032V2_BRD_TYPE_MSB				         3
#  define DELTA_AG9032V2_BRD_TYPE_LSB				         0

#define DELTA_AG9032V2_SYS_SRR_REG				         0x03
#  define DELTA_AG9032V2_SPI_CS_SEL_BIT			                 5
#  define DELTA_AG9032V2_RST_BIOS_SWITCH_BIT		                 4
#  define DELTA_AG9032V2_CPLD_UPGRADE_RST_BIT		                 1

#define DELTA_AG9032V2_SYS_EEPROM_WP_REG			         0x04
#  define DELTA_AG9032V2_SYSTM_ID_EEPROM_WP_BIT			         4
#  define DELTA_AG9032V2_SPI_WP_GBE_BIT				         3
#  define DELTA_AG9032V2_SPI_BIOS_WP_BIT			         2
#  define DELTA_AG9032V2_SPI_BAK_BIOS_WP_BIT			         1

#define DELTA_AG9032V2_SYS_IRQ_REG				         0x05
#  define DELTA_AG9032V2_LPC_CLK_FAIL_IRQ_EN_BIT		         7
#  define DELTA_AG9032V2_VRHOT_VCCP_IRQ_EN_BIT			         6
#  define DELTA_AG9032V2_CPU_THERMTRIP_IRQ_EN_BIT		         5
#  define DELTA_AG9032V2_LPC_CLK_FAIL_IRQ_BIT			         3
#  define DELTA_AG9032V2_VRHOT_VCCP_IRQ_BIT			         2
#  define DELTA_AG9032V2_CPU_THERMTRIP_IRQ_BIT			         1

#define DELTA_AG9032V2_SYS_WD_REG			                 0x06
#  define DELTA_AG9032V2_WD_TIMER_MSB			                 6
#  define DELTA_AG9032V2_WD_TIMER_LSB			                 4
#  define DELTA_AG9032V2_WD_EN_BIT			                 3
#  define DELTA_AG9032V2_WD_PUNCH_BIT			                 0

#define DELTA_AG9032V2_SYS_MB_RST_EN_REG		                 0x07
#  define DELTA_AG9032V2_MB_RST_EN_BIT			                 0

#define DELTA_AG9032V2_REBOOT_CAUSE_REG			                 0x08
#  define DELTA_AG9032V2_COLD_RESET_BIT			                 7
#  define DELTA_AG9032V2_WARM_RESET_BIT			                 6
#  define DELTA_AG9032V2_WD_FAIL_BIT			                 4
#  define DELTA_AG9032V2_BIOS_SWITCHOVER_BIT			         3
#  define DELTA_AG9032V2_BOOT_FAIL_BIT			                 2
#  define DELTA_AG9032V2_CPU_PWR_ERR_BIT			         0

#define DELTA_AG9032V2_CPU_PWR_EN_STATUS_REG		                 0x09
#  define DELTA_AG9032V2_V1P5_EN_BIT			                 7
#  define DELTA_AG9032V2_PWR_VDDR_EN_BIT		                 6
#  define DELTA_AG9032V2_PWR_CORE_EN_BIT		                 5
#  define DELTA_AG9032V2_V1P1_EN_BIT			                 4
#  define DELTA_AG9032V2_V1P0_EN_BIT			                 3
#  define DELTA_AG9032V2_V3P3_EN_BIT			                 2
#  define DELTA_AG9032V2_REG_1V8_EN_BIT			                 1
#  define DELTA_AG9032V2_REG_1V35_EN_BIT		                 0

#define DELTA_AG9032V2_CPU_PWR_STATUS_1_REG		                 0x0a
#  define DELTA_AG9032V2_PG_DDR_VTT_BIT			                 7
#  define DELTA_AG9032V2_PG_PVDDR_BIT			                 6
#  define DELTA_AG9032V2_PG_PWR_CORE_BIT		                 5
#  define DELTA_AG9032V2_PG_V1P1_BIT			                 4
#  define DELTA_AG9032V2_PG_V1P0_BIT			                 3
#  define DELTA_AG9032V2_PG_3V3_BIT			                 2
#  define DELTA_AG9032V2_PG_1V8_BIT			                 1
#  define DELTA_AG9032V2_PG_1V35_BIT			                 0

#define DELTA_AG9032V2_CPU_PWR_STATUS_2_REG		                 0x0b
#  define DELTA_AG9032V2_UPDATE_CPLD_BIT		                 0

/*------------------------------------------------------------------------------
 *
 *			       Master CPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AG9032V2_HW_REVISION_REG				         0x01
#  define DELTA_AG9032V2_PLATFORM_TYPE_MSB				 3
#  define DELTA_AG9032V2_PLATFORM_TYPE_LSB				 0

#define DELTA_AG9032V2_SOFTWARE_RESET_REG				 0x02
#  define DELTA_AG9032V2_MB_RST_N_P1_BIT			         7
#  define DELTA_AG9032V2_CPLD_LPCRST_P1_BIT			         6
#  define DELTA_AG9032V2_MB_RST_DONE_P1_BIT			         5
#  define DELTA_AG9032V2_PCIE_PERST_L_P1_BIT			         4
#  define DELTA_AG9032V2_RESET_BUTTON_N_P1_BIT			         3
#  define DELTA_AG9032V2_BMC_RSTN_P1_BIT			         2
#  define DELTA_AG9032V2_RESET_N_B56870_P1_BIT			         1
#  define DELTA_AG9032V2_RST_USB_HUBN_P1_BIT			         0

#define DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG			         0x03
#  define DELTA_AG9032V2_PS1_PRESENT_N_P1_BIT				 7
#  define DELTA_AG9032V2_PS1_PWR_FAN_OK_P1_BIT				 6
#  define DELTA_AG9032V2_PS1_PWR_INT_N_P1_BIT				 5
#  define DELTA_AG9032V2_PS1_PSON_N_P1_BIT				 4
#  define DELTA_AG9032V2_PS2_PRESENT_N_P1_BIT				 3
#  define DELTA_AG9032V2_PS2_PWR_FAN_OK_P1_BIT				 2
#  define DELTA_AG9032V2_PS2_PWR_INT_N_P1_BIT				 1
#  define DELTA_AG9032V2_PS2_PSON_N_P1_BIT				 0

#define DELTA_AG9032V2_POWER_ENABLE_REG				         0x04
#  define DELTA_AG9032V2_MAC_1V8_EN_P1_BIT				 4
#  define DELTA_AG9032V2_MAC_AVS_0P92V_EN_P1_BIT			 3
#  define DELTA_AG9032V2_MB_PWR_ENABLE_P1_BIT				 2
#  define DELTA_AG9032V2_VCC_3V3_EN_P1_BIT				 1
#  define DELTA_AG9032V2_VCC_MAC_1V25_EN_P1_BIT				 0

#define DELTA_AG9032V2_POWER_STATUS_REG				         0x05
#  define DELTA_AG9032V2_BMC_1V2_PG_BIT					 7
#  define DELTA_AG9032V2_BMC_1V15_PG_BIT				 6
#  define DELTA_AG9032V2_MAC_1V8_PG_P1_BIT				 5
#  define DELTA_AG9032V2_MAC_AVS_0P92V_PG_P1_BIT			 4
#  define DELTA_AG9032V2_MB_PWR_PGD_P1_BIT				 3
#  define DELTA_AG9032V2_VCC_3V3_PG_P1_BIT				 2
#  define DELTA_AG9032V2_VCC_MAC_1V25_PG_P1_BIT				 1
#  define DELTA_AG9032V2_BMC_DDR_2V5_PG_BIT				 0

#define DELTA_AG9032V2_INTERRUPT_1_REG				         0x06
#  define DELTA_AG9032V2_D_FAN_ALERTN_P1_BIT				 7
#  define DELTA_AG9032V2_THERMAL_OUTN_BIT				 6
#  define DELTA_AG9032V2_SMB_ALERT_P1_BIT				 5
#  define DELTA_AG9032V2_SMB_ALERT_1V_P1_BIT				 4
#  define DELTA_AG9032V2_AVS_1V_PHSFLTN_BIT				 3
#  define DELTA_AG9032V2_PCIE_INTR_L_BIT				 2
#  define DELTA_AG9032V2_5V_EN_BIT					 1

#define DELTA_AG9032V2_INTERRUPT_2_REG				         0x07
#  define DELTA_AG9032V2_PSU_FAN_EVENTN_BIT				 6
#  define DELTA_AG9032V2_OP_MODULE_EVENTN_BIT				 5
#  define DELTA_AG9032V2_MISC_INT_BIT					 4
#  define DELTA_AG9032V2_VCC_MAC_0P8V_EN_BIT				 3
#  define DELTA_AG9032V2_PG_VCC0P8V_BIT					 2
#  define DELTA_AG9032V2_SERIRQ_CPLD_P1_BIT				 1
#  define DELTA_AG9032V2_IRQ_PHY_P1_BIT					 0

#define DELTA_AG9032V2_INTERRUPT_MASK_1_REG			         0x08
#  define DELTA_AG9032V2_PSU_FAN_EVENTN_MASK_BIT			 6
#  define DELTA_AG9032V2_OP_MODULE_EVENTN_MASK_BIT			 5
#  define DELTA_AG9032V2_MISC_INT_MASK_BIT				 4
#  define DELTA_AG9032V2_SERIRQ_MASK_BIT				 1
#  define DELTA_AG9032V2_IRQ_PHY_P1_MASK_BIT				 0

#define NUM_QSFP_REGS							 4

#define DELTA_AG9032V2_QSFP28_MODULE_SELECT_1_REG		         0x0a
#define DELTA_AG9032V2_QSFP28_MODULE_SELECT_2_REG		         0x0b
#define DELTA_AG9032V2_QSFP28_MODULE_SELECT_3_REG		         0x0c
#define DELTA_AG9032V2_QSFP28_MODULE_SELECT_4_REG		         0x0d

#define DELTA_AG9032V2_QSFP28_LP_MODE_ENABLE_1_REG		         0x0e
#define DELTA_AG9032V2_QSFP28_LP_MODE_ENABLE_2_REG		         0x0f
#define DELTA_AG9032V2_QSFP28_LP_MODE_ENABLE_3_REG		         0x10
#define DELTA_AG9032V2_QSFP28_LP_MODE_ENABLE_4_REG		         0x11

#define DELTA_AG9032V2_QSFP28_PRESENCE_1_REG			         0x12
#define DELTA_AG9032V2_QSFP28_PRESENCE_2_REG			         0x13
#define DELTA_AG9032V2_QSFP28_PRESENCE_3_REG			         0x14
#define DELTA_AG9032V2_QSFP28_PRESENCE_4_REG			         0x15

#define DELTA_AG9032V2_QSFP28_RESET_1_REG			         0x16
#define DELTA_AG9032V2_QSFP28_RESET_2_REG			         0x17
#define DELTA_AG9032V2_QSFP28_RESET_3_REG			         0x18
#define DELTA_AG9032V2_QSFP28_RESET_4_REG			         0x19

#define DELTA_AG9032V2_QSFP28_INTERRUPT_1_REG			         0x16
#define DELTA_AG9032V2_QSFP28_INTERRUPT_2_REG			         0x17
#define DELTA_AG9032V2_QSFP28_INTERRUPT_3_REG			         0x18
#define DELTA_AG9032V2_QSFP28_INTERRUPT_4_REG			         0x19

#define DELTA_AG9032V2_PSU_AND_FAN_I2C_MUX_REG			         0x1e
#  define DELTA_AG9032V2_PSU_I2C_SEL_MSB				 5
#  define DELTA_AG9032V2_PSU_I2C_SEL_LSB				 4
#  define DELTA_AG9032V2_UART_SEL_BIT				         3
#  define DELTA_AG9032V2_FAN_I2C_SEL_MSB				 2
#  define DELTA_AG9032V2_FAN_I2C_SEL_LSB				 0

#define DELTA_AG9032V2_QSFP28_SFP_I2C_MUX_REG			         0x1f
#  define DELTA_AG9032V2_PORT_I2C_OE_MSB				 7
#  define DELTA_AG9032V2_PORT_I2C_OE_LSB				 6
#  define DELTA_AG9032V2_PORT_I2C_SEL_MSB				 5
#  define DELTA_AG9032V2_PORT_I2C_SEL_LSB				 0

#define DELTA_AG9032V2_FAN_I2C_DATA_CLK_REG			         0x20
#  define DELTA_AG9032V2_FAN_5_LED_MSB				         7
#  define DELTA_AG9032V2_FAN_5_LED_LSB				         6
#  define DELTA_AG9032V2_FAN_4_LED_MSB				         5
#  define DELTA_AG9032V2_FAN_4_LED_LSB				         4
#  define DELTA_AG9032V2_FAN_3_LED_MSB				         3
#  define DELTA_AG9032V2_FAN_3_LED_LSB				         2
#  define DELTA_AG9032V2_FAN_2_LED_MSB				         1
#  define DELTA_AG9032V2_FAN_2_LED_LSB				         0

#define DELTA_AG9032V2_SYSTEM_LED_1_REG			                 0x21
#  define DELTA_AG9032V2_PSU1_LED_MSB				         7
#  define DELTA_AG9032V2_PSU1_LED_LSB				         6
#  define DELTA_AG9032V2_PSU2_LED_MSB				         5
#  define DELTA_AG9032V2_PSU2_LED_LSB				         4
#  define DELTA_AG9032V2_SYS_LED_MSB				         3
#  define DELTA_AG9032V2_SYS_LED_LSB				         2
#  define DELTA_AG9032V2_FAN_LED_MSB				         1
#  define DELTA_AG9032V2_FAN_LED_LSB				         0

#define DELTA_AG9032V2_SYSTEM_LED_2_REG			                 0x22
#  define DELTA_AG9032V2_TLED1_B_P1_BIT				         7
#  define DELTA_AG9032V2_TLED1_G_P1_BIT				         6
#  define DELTA_AG9032V2_TLED1_R_P1_BIT				         5
#  define DELTA_AG9032V2_TLED3_P1_BIT				         4
#  define DELTA_AG9032V2_VRHOT_P1_BIT				         3

#define DELTA_AG9032V2_SYSTEM_LED_3_REG			                 0x23
#  define DELTA_AG9032V2_FAN_1_LED_MSB				         7
#  define DELTA_AG9032V2_FAN_1_LED_LSB				         6

#define DELTA_AG9032V2_GPIO_1_REG			                 0x24
#  define DELTA_AG9032V2_GPIO_P1_P2_A_BIT			         7
#  define DELTA_AG9032V2_GPIO_P1_P3_A_BIT			         6
#  define DELTA_AG9032V2_PLD1_PLD2_A_BIT			         5
#  define DELTA_AG9032V2_PLD1_PLD2_B_BIT			         4
#  define DELTA_AG9032V2_PLD1_PLD3_B_BIT			         3
#  define DELTA_AG9032V2_PLD1_PLD3_A_BIT			         2
#  define DELTA_AG9032V2_CPLD_A_CPLD_GPIO1_BIT			         1
#  define DELTA_AG9032V2_CPU_CPLD1_GPIO2_P1_BIT			         0

#define DELTA_AG9032V2_GPIO_2_REG			                 0x25
#  define DELTA_AG9032V2_CPLD_A_CPLD_GPIO3_P1_BIT		         7
#  define DELTA_AG9032V2_CPLD_B_CPLD_GPIO2_P1_BIT		         6
#  define DELTA_AG9032V2_CPLD_B_CPLD_GPIO3_P1_BIT		         5
#  define DELTA_AG9032V2_CPLD_A_CPLD_GPIO2_BIT			         4
#  define DELTA_AG9032V2_CPLD_B_CPLD_GPIO1_BIT			         3
#  define DELTA_AG9032V2_BMC_GPIOL1_BIT				         2
#  define DELTA_AG9032V2_BMC_GPIOL2_BIT				         1
#  define DELTA_AG9032V2_GPIOH3_BIT				         0

#define DELTA_AG9032V2_GPIO_3_REG			                 0x26
#  define DELTA_AG9032V2_MB_A_CPLD_RSTN_BIT			         7
#  define DELTA_AG9032V2_GPIO_PHYRST_BIT			         6
#  define DELTA_AG9032V2_P1_MPLD_1_BIT				         5
#  define DELTA_AG9032V2_EEPROM_WP_BIT				         4
#  define DELTA_AG9032V2_PCIE_WAKE_L_BIT			         3

#define DELTA_AG9032V2_CPLD_REVISION_REG			         0x27
#  define DELTA_AG9032V2_CPLD_REVISION_MSB				 7
#  define DELTA_AG9032V2_CPLD_REVISION_LSB				 0

/*------------------------------------------------------------------------------
 *
 *			       Slave1 CPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AG9032V2_SLAVE1_REVISION_REG			         0x01
#  define DELTA_AG9032V2_SLAVE1_REVISION_MSB				 7
#  define DELTA_AG9032V2_SLAVE1_REVISION_LSB				 0

#define DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG			         0x02
#  define DELTA_AG9032V2_SFP_P0_MOD_ABS_BIT				 7
#  define DELTA_AG9032V2_SFP_P0_RXLOS_BIT				 6
#  define DELTA_AG9032V2_SFP_P0_TX_DIS_BIT				 5
#  define DELTA_AG9032V2_SFP_P0_TXFAULT_BIT				 4
#  define DELTA_AG9032V2_SFP_P1_MOD_ABS_BIT				 3
#  define DELTA_AG9032V2_SFP_P1_RXLOS_BIT				 2
#  define DELTA_AG9032V2_SFP_P1_TX_DIS_BIT				 1
#  define DELTA_AG9032V2_SFP_P1_TXFAULT_BIT				 0

#define DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_MASK_REG		         0x03
#  define DELTA_AG9032V2_SFP_P0_MOD_ABS_MASK_BIT			 7
#  define DELTA_AG9032V2_SFP_P1_MOD_ABS_MASK_BIT			 6

#define DELTA_AG9032V2_SLAVE1_MAC_LED_REG			         0x04
#  define DELTA_AG9032V2_SLED_CLK0_B2A_P2_BIT				 7
#  define DELTA_AG9032V2_SLED_CLK1_B2A_P2_BIT				 6
#  define DELTA_AG9032V2_SLED_DAT0_B2A_P2_BIT				 5
#  define DELTA_AG9032V2_SLED_DAT1_B2A_P2_BIT				 4
#  define DELTA_AG9032V2_TLED4_B_P2_BIT					 3
#  define DELTA_AG9032V2_TLED4_G_P2_BIT					 2
#  define DELTA_AG9032V2_TLED4_R_P2_BIT					 1
#  define DELTA_AG9032V2_TLED5_P2_BIT					 0


#define DELTA_AG9032V2_SLAVE1_LED_REG				         0x05
#  define DELTA_AG9032V2_SFP_P0_ACT_LED_BIT				 7
#  define DELTA_AG9032V2_SFP_P0_LED_A_BIT				 6
#  define DELTA_AG9032V2_SFP_P0_LED_G_BIT				 5
#  define DELTA_AG9032V2_SFP_P1_ACT_LED_BIT				 4
#  define DELTA_AG9032V2_SFP_P1_LED_A_BIT				 3
#  define DELTA_AG9032V2_SFP_P1_LED_G_BIT				 2

#define DELTA_AG9032V2_SLAVE1_RESET_AND_HARDWARE_REVISION_REG	         0x06
#  define DELTA_AG9032V2_BOARD_STAGE_0_3_MSB				 7
#  define DELTA_AG9032V2_BOARD_STAGE_0_3_LSB				 4
#  define DELTA_AG9032V2_S1_CPLD_GPIO_RST_EN_BIT			 3
#  define DELTA_AG9032V2_GLOBAL_RESET_BIT				 2
#  define DELTA_AG9032V2_156P25M_CLK_SEL_BIT				 1

#define DELTA_AG9032V2_SLAVE1_GPIO1_REG				         0x07
#  define DELTA_AG9032V2_S1_CPLD_AB_GPIO4_BIT				 7
#  define DELTA_AG9032V2_S1_CPLD_AB_GPIO3_BIT				 6
#  define DELTA_AG9032V2_S1_PLD1_PLD2_A_BIT				 5
#  define DELTA_AG9032V2_S1_GPIO_P1_P2_A_BIT				 4
#  define DELTA_AG9032V2_S1_PLD1_PLD2_B_BIT				 3
#  define DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO2_BIT			 2
#  define DELTA_AG9032V2_S1_CPU_CPLD2_GPIO2_P2_BIT			 1
#  define DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO3_BIT			 0

#define DELTA_AG9032V2_SLAVE1_GPIO2_REG				         0x08
#  define DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO1_P2_BIT			 7
#  define DELTA_AG9032V2_S1_CPLD_AB_GPIO2_P2_BIT			 6
#  define DELTA_AG9032V2_S1_CPLD_AB_GPIO1_P2_BIT			 5
#  define DELTA_AG9032V2_S1_GPIOH4_BIT					 4
#  define DELTA_AG9032V2_S1_GPIOH5_BIT					 3
#  define DELTA_AG9032V2_S1_PLD2_PLD3_A_BIT				 2
#  define DELTA_AG9032V2_S1_PLD2_PLD3_B_BIT				 1
#  define DELTA_AG9032V2_S1_PLD2_PLD3_C_BIT				 0

/*------------------------------------------------------------------------------
 *
 *			       Slave2 CPLD Registers
 *
 *------------------------------------------------------------------------------
 */

#define DELTA_AG9032V2_SLAVE2_REVISION_REG			         0x01
#  define DELTA_AG9032V2_SLAVE2_REVISION_MSB				 7
#  define DELTA_AG9032V2_SLAVE2_REVISION_LSB				 0

#define DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG		         0x02
#  define DELTA_AG9032V2_54616S_LED1_BIT				 7
#  define DELTA_AG9032V2_54616S_LED2_BIT				 6
#  define DELTA_AG9032V2_54616S_LED3_BIT				 5
#  define DELTA_AG9032V2_TLED0_R_P3_BIT					 4
#  define DELTA_AG9032V2_TLED0_G_P3_BIT					 3
#  define DELTA_AG9032V2_TLED0_B_P3_BIT					 2
#  define DELTA_AG9032V2_TLED2_P3_BIT					 1

#define DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG			         0x03
#  define DELTA_AG9032V2_1PPS_OUT_BIT					 7
#  define DELTA_AG9032V2_82P33731_INT_BIT				 6
#  define DELTA_AG9032V2_BS_CLK_BIT					 5
#  define DELTA_AG9032V2_JER1_SYNC_E_RST_N_BIT				 4
#  define DELTA_AG9032V2_JA_CLK_SEL_BIT					 3
#  define DELTA_AG9032V2_SYNC_E_PRESENT_BIT				 2
#  define DELTA_AG9032V2_JER1_SYNC_E_EEPROM_WP_BIT			 1

#define DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG		         0x04
#  define DELTA_AG9032V2_T_I210_P1_ACTIVITY_N_BIT			 7
#  define DELTA_AG9032V2_T_I210_P1_LINK100_N_BIT			 6
#  define DELTA_AG9032V2_T_I210_P1_LINK1000_N_BIT			 5
#  define DELTA_AG9032V2_T_C_FM_CPLD_PCIE_RESET_N_BIT			 4
#  define DELTA_AG9032V2_T_C_I210_RSTN_BIT				 3
#  define DELTA_AG9032V2_T_C_LANWAKEB_BIT				 2
#  define DELTA_AG9032V2_CPU_RST_MB_OOB_BIT				 1
#  define DELTA_AG9032V2_U86_CLK_SEL_BIT				 0

#define DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG			         0x05
#  define DELTA_AG9032V2_MB_CPLD_UPGRADE_RST_BIT			 7
#  define DELTA_AG9032V2_S2_CPLD_UPGRADE_RST_BIT			 6
#  define DELTA_AG9032V2_CPLD_GPIO_RST_DONE_BIT				 5
#  define DELTA_AG9032V2_MB_B_CPLD_RSTN_P3_BIT				 4
#  define DELTA_AG9032V2_CPLD_MB_LDRQ0_N_BIT				 3
#  define DELTA_AG9032V2_CPLD_GPIO_PWR_EN_BIT				 2
#  define DELTA_AG9032V2_CPLD_GPIO_PWR_PG_BIT				 1
#  define DELTA_AG9032V2_S2_CPLD_GPIO_RST_EN_BIT			 0

#define DELTA_AG9032V2_SLAVE2_MAC_LED_REG			         0x06
#  define DELTA_AG9032V2_SLED_CLK0_B2A_P3_BIT				 7
#  define DELTA_AG9032V2_SLED_CLK1_B2A_P3_BIT				 6
#  define DELTA_AG9032V2_SLED_DAT0_B2A_P3_BIT				 5
#  define DELTA_AG9032V2_SLED_DAT1_B2A_P3_BIT				 4
#  define DELTA_AG9032V2_P3_MPLD_1_BIT					 3

#define DELTA_AG9032V2_SLAVE2_GPIO1_REG				         0x08
#  define DELTA_AG9032V2_S2_PLD1_PLD3_B_BIT				 7
#  define DELTA_AG9032V2_S2_PLD1_PLD3_A_BIT				 6
#  define DELTA_AG9032V2_S2_CPLD_AB_GPIO4_P3_BIT			 5
#  define DELTA_AG9032V2_S2_CPLD_AB_GPIO3_P3_BIT			 4
#  define DELTA_AG9032V2_S2_GPIO_P1_P3_A_BIT				 3
#  define DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO2_BIT			 2
#  define DELTA_AG9032V2_S2_CPU_CPLD3_GPIO1_BIT				 1
#  define DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO3_BIT			 0

#define DELTA_AG9032V2_SLAVE2_GPIO2_REG				         0x09
#  define DELTA_AG9032V2_S2_CPLD_AB_GPIO1_BIT				 7
#  define DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO1_P3_BIT			 6
#  define DELTA_AG9032V2_S2_CPLD_AB_GPIO2_BIT				 5
#  define DELTA_AG9032V2_S2_PLD2_PLD3_A_BIT				 4
#  define DELTA_AG9032V2_S2_PLD2_PLD3_B_BIT				 3
#  define DELTA_AG9032V2_S2_PLD2_PLD3_C_BIT				 2
#  define DELTA_AG9032V2_S2_GPIOH6_BIT					 1
#  define DELTA_AG9032V2_S2_GPIOH7_BIT					 0

#endif	/* DELTA_AG9032V2_CPLD_H__ */

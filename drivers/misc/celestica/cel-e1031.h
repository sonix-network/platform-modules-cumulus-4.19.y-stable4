/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Celestica Haliburton (E1031) CPLD Definitions
 *
 * Copyright (C) 2018, 2020 Cumulus Networks, Inc.  All Rights Reserved.
 * Authors: Puneet Shenoy <puneet@cumulusnetworks.com>
 *          David Yen <dhyen@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef CEL_E1031_CPLD_H__
#define CEL_E1031_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * This platform has two CPLD devices
 *
 *------------------------------------------------------------------------------
 */

#define E1031_PLATFORM_NAME "e1031_platform"

#define E1031_MMC_NAME      "e1031_mmc"
#define MMC_IO_BASE         0x100
#define MMC_IO_SIZE         0x0FF

#define E1031_SMC_NAME      "e1031_smc"
#define SMC_IO_BASE         0x200
#define SMC_IO_SIZE         0x0FF

#define CPLD_IO_BASE         0x100
#define CPLD_IO_SIZE         0x155

/*------------------------------------------------------------------------------
 *
 *                                 MMC Registers
 *
 *------------------------------------------------------------------------------
 */

#define MMC_VERSION_REG							 0x100
#  define MMC_MAJOR_VERSION_MSB						 7
#  define MMC_MAJOR_VERSION_LSB						 4
#  define MMC_MINOR_VERSION_MSB						 3
#  define MMC_MINOR_VERSION_LSB						 0

#define MMC_SW_SCRATCH_REG						 0x101
#  define MMC_SCRATCHPAD_MSB						 7
#  define MMC_SCRATCHPAD_LSB						 0

#define MMC_BOOT_OK_REG							 0x102
#  define MMC_CPU_BOOT_STA_BIT						 1
#  define MMC_CPU_BIOS_BIT						 0

#define MMC_EEPROM_WP_REG						 0x103
#  define MMC_SPD1_WP_BIT						 1
#  define MMC_SYSTEM_EEPROM_WP_BIT					 0

#define MMC_PRESENT_MB_REG						 0x104
#  define MMC_PRESENT_MB_BIT						 0

#define MMC_WD_WID_REG							 0x110
#  define MMC_WD_WID_MSB						 1
#  define MMC_WD_WID_LSB						 0

#define MMC_WD_MASK_REG							 0x111
#  define MMC_WD_MASK_BIT						 0

#define MMC_RST_SOURCE_REG						 0x112
#  define MMC_RST_SOURCE_MSB						 7
#  define MMC_RST_SOURCE_LSB						 0

#define MMC_RST_CTRL_REG						 0x113
#  define MMC_RST_CTRL_MSB						 7
#  define MMC_RST_CTRL_LSB						 0

#define MMC_SEP_RST_REG							 0x114
#  define MMC_SMC_RST_BIT						 1
#  define MMC_BCM54616_RST_BIT						 0

#define MMC_THERMAL_POWEROFF_CTRL_REG					 0x120
#  define MMC_CPU_POWEROFF_CTRL_BIT					 0

#define MMC_SUS0_TRIG_MOD_REG						 0x121
#  define MMC_THERMTRIP_TRIG_MSB					 5
#  define MMC_THERMTRIP_TRIG_LSB					 4
#  define MMC_BCM54616_TRIG_MSB						 3
#  define MMC_BCM54616_TRIG_LSB						 2
#  define MMC_SENSOR_TRIG_MSB						 1
#  define MMC_SENSOR_TRIG_LSB						 0

#define MMC_SUS0_COMBINE_REG						 0x122
#  define MMC_THERMALTRIP_COMBINE_BIT					 3
#  define MMC_BCM54616_COMBINE_BIT					 2
#  define MMC_SENSOR_PROCHOT_COMBINE_BIT				 1
#  define MMC_SENSOR_ALERT_COMBINE_BIT					 0

#define MMC_SUS0_STA_REG						 0x123
#  define MMC_THERMTRIP_STA_BIT						 3
#  define MMC_BCM54616_STA_BIT						 2
#  define MMC_TS_PROCHOT_STA_BIT					 1
#  define MMC_TS_ALERT_STA_BIT						 0

#define MMC_SUS0_INT_REG						 0x124
#  define MMC_THERMTRIP_INT_BIT						 3
#  define MMC_BCM54616_INT_BIT						 2
#  define MMC_TS_PROCHOT_INT_BIT					 1
#  define MMC_TS_ALERT_INT_BIT						 0

#define MMC_SUS0_MASK_REG						 0x125
#  define MMC_THERMTRIP_MASK_BIT					 3
#  define MMC_BCM54616_MASK_BIT						 2
#  define MMC_TS_PROCHOT_MASK_BIT					 1
#  define MMC_TS_ALERT_MASK_BIT						 0

/*------------------------------------------------------------------------------
 *
 *                                 SMC Registers
 *
 *------------------------------------------------------------------------------
 */

#define SMC_VERSION_REG							 0x200
#  define SMC_MAJOR_VERSION_MSB						 7
#  define SMC_MAJOR_VERSION_LSB						 4
#  define SMC_MINOR_VERSION_MSB						 3
#  define SMC_MINOR_VERSION_LSB						 0

#define SMC_SW_SCRATCH_REG						 0x201
#  define SMC_SCRATCHPAD_MSB						 7
#  define SMC_SCRATCHPAD_LSB						 0

#define SMC_BOARD_ID_REG						 0x202
#  define SMC_BOARD_ID_MSB						 3
#  define SMC_BOARD_ID_LSB						 0

#define SMC_PSU_CONTROL_REG						 0x203
#  define SMC_PSUR_CTRL_BIT						 1
#  define SMC_PSUL_CTRL_BIT						 0

#define SMC_PSU_STATUS_REG						 0x204
#  define SMC_PSUR_AC_BIT						 7
#  define SMC_PSUR_POWER_BIT						 6
#  define SMC_PSUR_ALERT_BIT						 5
#  define SMC_PSUR_PRESENT_BIT						 4
#  define SMC_PSUL_AC_BIT						 3
#  define SMC_PSUL_POWER_BIT						 2
#  define SMC_PSUL_ALERT_BIT						 1
#  define SMC_PSUL_PRESENT_BIT						 0

#define SMC_FAN1_LED_REG						 0x205
#  define SMC_FAN1_LED_MSB						 2
#  define SMC_FAN1_LED_LSB						 0

#define SMC_FAN2_LED_REG						 0x206
#  define SMC_FAN2_LED_MSB						 2
#  define SMC_FAN2_LED_LSB						 0

#define SMC_FAN3_LED_REG						 0x207
#  define SMC_FAN3_LED_MSB						 2
#  define SMC_FAN3_LED_LSB						 0

#define SMC_LED_OPMOD_REG						 0x208
#  define SMC_OP_MOD_BIT						 0

#define SMC_LED_TEST_REG						 0x209
#  define SMC_OP_CTRL_MSB						 1
#  define SMC_OP_CTRL_LSB						 0

#define SMC_FPS_LED_REG							 0x20a
#  define SMC_FPS_STA_LED_MSB						 3
#  define SMC_FPS_STA_LED_LSB						 2
#  define SMC_FPS_MASTER_LED_MSB					 1
#  define SMC_FPS_MASTER_LED_LSB					 0

#define SMC_DEV_STA_REG							 0x20c
#  define SMC_USB_HUB_STATE_BIT						 3
#  define SMC_FAN3_F2B_N_BIT						 2
#  define SMC_FAN2_F2B_N_BIT						 1
#  define SMC_FAN1_F2B_N_BIT						 0

#define SMC_FAN_EEPROM_WP_REG						 0x20d
#  define SMC_FAN_EEPROM_WP_BIT						 0

#define SMC_EEPROM_WC_REG						 0x20e
#  define SMC_EEPROM_WC_N_BIT						 0

#define SMC_POWER_EN_REG						 0x20f
#  define SMC_USB_RST_BIT						 2
#  define SMC_XP1R5V_EN_BIT						 1
#  define SMC_XP1R0V_EN_BIT						 0

#define SMC_WD_WID_REG							 0x220
#  define SMC_WD_WID_MSB						 1
#  define SMC_WD_WID_LSB						 0

#define SMC_WD_MASK_REG							 0x221
#  define SMC_WD_MASK_BIT						 0

#define SMC_SEP_RST_REG							 0x222
#  define SMC_PCIE_RST_N_BIT						 4
#  define SMC_USBHUB_RST_N_BIT						 3
#  define SMC_BCM50282_RST_BIT						 2
#  define SMC_PCA9548A_RST_BIT						 1
#  define SMC_BCM54616_RST_BIT						 0

#define SMC_SUS6_TRIG_MOD1_REG						 0x230
#  define SMC_TS_TRIG_MSB						 5
#  define SMC_TS_TRIG_LSB						 4
#  define SMC_54616S_TRIG_MSB						 3
#  define SMC_54616S_TRIG_LSB						 2
#  define SMC_B50282_TRIG_MSB						 1
#  define SMC_B50282_TRIG_LSB						 0

#define SMC_SUS6_TRIG_MOD2_REG						 0x231
#  define SMC_SWITCH_TRIG_MSB						 7
#  define SMC_SWITCH_TRIG_LSB						 6
#  define SMC_FAN_TRAY_TRIG_MSB						 5
#  define SMC_FAN_TRAY_TRIG_LSB						 4
#  define SMC_FAN_CTRL_TRIG_MSB						 3
#  define SMC_FAN_CTRL_TRIG_LSB						 2
#  define SMC_PSU_TRIG_MSB						 1
#  define SMC_PSU_TRIG_LSB						 0

#define SMC_SUS6_COMBINE_REG						 0x232
#  define SMC_SWITCH_COMBINE_BIT					 6
#  define SMC_FAN_TRAY_COMBINE_BIT					 5
#  define SMC_FAN_CTRL_COMBINE_BIT					 4
#  define SMC_PSU_COMBINE_BIT						 3
#  define SMC_TS_COMBINE_BIT						 2
#  define SMC_54616S_COMBINE_BIT					 1
#  define SMC_B50282_COMBINE_BIT					 0

#define SMC_SUS6_STA1_REG						 0x233
#  define SMC_THERMAL_INT_STA_BIT					 7
#  define SMC_54616S_INT_STA_BIT					 6
#  define SMC_B50282_6_STA_BIT						 5
#  define SMC_B50282_5_STA_BIT						 4
#  define SMC_B50282_4_STA_BIT						 3
#  define SMC_B50282_3_STA_BIT						 2
#  define SMC_B50282_2_STA_BIT						 1
#  define SMC_B50282_1_STA_BIT						 0

#define SMC_SUS6_STA2_REG						 0x234
#  define SMC_FAN3_PRESENT_STA_BIT					 7
#  define SMC_FAN2_PRESENT_STA_BIT					 6
#  define SMC_FAN1_PRESENT_STA_BIT					 5
#  define SMC_FAN_INT_STA_BIT						 4
#  define SMC_PSUR_ALERT_STA_BIT					 3
#  define SMC_PSUL_ALERT_STA_BIT					 2
#  define SMC_PSUR_PRES_STA_BIT						 1
#  define SMC_PSUL_PRES_STA_BIT						 0

#define SMC_SUS6_STA3_REG						 0x235
#  define SMC_SWITCH_STA_BIT						 0

#define SMC_SUS6_INT1_REG						 0x236
#  define SMC_TS_INT_BIT						 7
#  define SMC_54616S_INT_BIT						 6
#  define SMC_B50282_6_INT_BIT						 5
#  define SMC_B50282_5_INT_BIT						 4
#  define SMC_B50282_4_INT_BIT						 3
#  define SMC_B50282_3_INT_BIT						 2
#  define SMC_B50282_2_INT_BIT						 1
#  define SMC_B50282_1_INT_BIT						 0

#define SMC_SUS6_INT2_REG						 0x237
#  define SMC_FAN3_TRAY_INT_BIT						 7
#  define SMC_FAN2_TRAY_INT_BIT						 6
#  define SMC_FAN1_TRAY_INT_BIT						 5
#  define SMC_FAN_CTRL_INT_BIT						 4
#  define SMC_PSUR_ALERT_INT_BIT					 3
#  define SMC_PSUL_ALERT_INT_BIT					 2
#  define SMC_PSUR_PRES_INT_BIT						 1
#  define SMC_PSUL_PRES_INT_BIT						 0

#define SMC_SUS6_INT3_REG						 0x238
#  define SMC_SWITCH_INT_BIT						 0

#define SMC_SUS6_MASK1_REG						 0x239
#  define SMC_TS_MASK_BIT						 7
#  define SMC_54616S_MASK_BIT						 6
#  define SMC_B50282_6_MASK_BIT						 5
#  define SMC_B50282_5_MASK_BIT						 4
#  define SMC_B50282_4_MASK_BIT						 3
#  define SMC_B50282_3_MASK_BIT						 2
#  define SMC_B50282_2_MASK_BIT						 1
#  define SMC_B50282_1_MASK_BIT						 0

#define SMC_SUS6_MASK2_REG						 0x23a
#  define SMC_FAN3_TRAY_MASK_BIT					 7
#  define SMC_FAN2_TRAY_MASK_BIT					 6
#  define SMC_FAN1_TRAY_MASK_BIT					 5
#  define SMC_FAN_CTRL_MASK_BIT						 4
#  define SMC_PSUR_ALERT_MASK_BIT					 3
#  define SMC_PSUL_ALERT_MASK_BIT					 2
#  define SMC_PSUR_PRES_MASK_BIT					 1
#  define SMC_PSUL_PRES_MASK_BIT					 0

#define SMC_SUS6_MASK3_REG						 0x23b
#  define SMC_SWITCH_MASK_BIT						 0

#define SMC_SUS7_TRIG_MOD_REG						 0x240
#  define SMC_SFP_RXLOS_TRIG_MSB					 5
#  define SMC_SFP_RXLOS_TRIG_LSB					 4
#  define SMC_SFP_ABS_TRIG_MSB						 3
#  define SMC_SFP_ABS_TRIG_LSB						 2
#  define SMC_SFP_TXFLT_TRIG_MSB					 1
#  define SMC_SFP_TXFLT_TRIG_LSB					 0

#define SMC_SUS7_COMBINE_REG						 0x241
#  define SMC_SFP_RXLOS_COMBINE_BIT					 2
#  define SMC_SFP_ABS_COMBINE_BIT					 1
#  define SMC_SFP_TXFLT_COMBINE_BIT					 0

#define SMC_SUS7_STA1_REG						 0x242
#  define SMC_SFP4_TXFLT_STA_BIT					 3
#  define SMC_SFP3_TXFLT_STA_BIT					 2
#  define SMC_SFP2_TXFLT_STA_BIT					 1
#  define SMC_SFP1_TXFLT_STA_BIT					 0

#define SMC_SUS7_STA2_REG						 0x243
#  define SMC_SFP4_ABS_STA_BIT						 3
#  define SMC_SFP3_ABS_STA_BIT						 2
#  define SMC_SFP2_ABS_STA_BIT						 1
#  define SMC_SFP1_ABS_STA_BIT						 0

#define SMC_SUS7_STA3_REG						 0x244
#  define SMC_SFP4_RXLOS_STA_BIT					 3
#  define SMC_SFP3_RXLOS_STA_BIT					 2
#  define SMC_SFP2_RXLOS_STA_BIT					 1
#  define SMC_SFP1_RXLOS_STA_BIT					 0

#define SMC_SUS7_INT1_REG						 0x246
#  define SMC_SFP4_TXFLT_INT_BIT					 3
#  define SMC_SFP3_TXFLT_INT_BIT					 2
#  define SMC_SFP2_TXFLT_INT_BIT					 1
#  define SMC_SFP1_TXFLT_INT_BIT					 0

#define SMC_SUS7_INT2_REG						 0x247
#  define SMC_SFP4_ABS_INT_BIT						 3
#  define SMC_SFP3_ABS_INT_BIT						 2
#  define SMC_SFP2_ABS_INT_BIT						 1
#  define SMC_SFP1_ABS_INT_BIT						 0

#define SMC_SUS7_INT3_REG						 0x248
#  define SMC_SFP4_RXLOS_INT_BIT					 3
#  define SMC_SFP3_RXLOS_INT_BIT					 2
#  define SMC_SFP2_RXLOS_INT_BIT					 1
#  define SMC_SFP1_RXLOS_INT_BIT					 0

#define SMC_SUS7_MASK1_REG						 0x24a
#  define SMC_SFP4_TXFLT_MASK_BIT					 3
#  define SMC_SFP3_TXFLT_MASK_BIT					 2
#  define SMC_SFP2_TXFLT_MASK_BIT					 1
#  define SMC_SFP1_TXFLT_MASK_BIT					 0

#define SMC_SUS7_MASK2_REG						 0x24b
#  define SMC_SFP4_ABS_MASK_BIT						 3
#  define SMC_SFP3_ABS_MASK_BIT						 2
#  define SMC_SFP2_ABS_MASK_BIT						 1
#  define SMC_SFP1_ABS_MASK_BIT						 0

#define SMC_SUS7_MASK3_REG						 0x24c
#  define SMC_SFP4_RXLOS_MASK_BIT					 3
#  define SMC_SFP3_RXLOS_MASK_BIT					 2
#  define SMC_SFP2_RXLOS_MASK_BIT					 1
#  define SMC_SFP1_RXLOS_MASK_BIT					 0

#define SMC_SFPTX_CTRL_REG						 0x255
#  define SMC_SFP4_RATE_SEL_BIT						 7
#  define SMC_SFP3_RATE_SEL_BIT						 6
#  define SMC_SFP2_RATE_SEL_BIT						 5
#  define SMC_SFP1_RATE_SEL_BIT						 4
#  define SMC_SFP4_TXEN_BIT						 3
#  define SMC_SFP3_TXEN_BIT						 2
#  define SMC_SFP2_TXEN_BIT						 1
#  define SMC_SFP1_TXEN_BIT						 0

#endif /* CEL_E1031_CPLD_H__ */

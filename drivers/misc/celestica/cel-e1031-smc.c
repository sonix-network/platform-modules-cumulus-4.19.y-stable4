// SPDX-License-Identifier: GPL-2.0+
/*
 * Celestica Haliburton (E1031) SMC Driver
 *
 * Copyright (C) 2015, 2020 Cumulus Networks, Inc.  All rights reserved.
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

#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "cel-e1031.h"

#define DRIVER_NAME    E1031_SMC_NAME
#define DRIVER_VERSION "1.1"
#define IO_BASE        SMC_IO_BASE
#define IO_SIZE        SMC_IO_SIZE

/* bitfield accessor functions */

static u8 *cpld_regs;

static int lpc_read_reg(struct device *dev,
			int reg,
			int nregs,
			u32 *val)
{
	int nbits = nregs * 8;
	int bit;

	*val = 0;
	for (bit = 0; bit < nbits; bit += 8, reg++)
		*val |= ioread8(cpld_regs + reg - IO_BASE) << bit;
	return 0;
}

static int lpc_write_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val, cpld_regs + reg - IO_BASE);
	return 0;
}

#define cpld_read_reg  lpc_read_reg
#define cpld_write_reg lpc_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const led_fan_values[] = {
	PLATFORM_LED_GREEN,                /* 0 */
	PLATFORM_LED_GREEN_SLOW_BLINKING,  /* 1 */
	PLATFORM_LED_YELLOW,               /* 2 */
	PLATFORM_LED_YELLOW_SLOW_BLINKING, /* 3 */
	PLATFORM_LED_OFF,                  /* 4 */
	PLATFORM_LED_OFF,                  /* 5 */
	PLATFORM_LED_OFF,                  /* 6 */
	PLATFORM_LED_OFF,                  /* 7 */
};

static const char * const led_test_values[] = {
	PLATFORM_LED_OFF,                 /* 0 */
	PLATFORM_LED_GREEN_BLINKING,      /* 1 */
	PLATFORM_LED_GREEN,               /* 2 */
	PLATFORM_LED_GREEN_SLOW_BLINKING, /* 3 */
};

static const char * const led_status_values[] = {
	PLATFORM_LED_OFF,            /* 0 */
	PLATFORM_LED_GREEN,          /* 1 */
	PLATFORM_LED_GREEN_BLINKING, /* 2 */
	PLATFORM_LED_OFF,            /* 3 */
};

static const char * const led_master_values[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
	PLATFORM_LED_AMBER, /* 2 */
	PLATFORM_LED_OFF,   /* 3 */
};

static const char * const fan_direction_values[] = {
	"F2B", /* 0 */
	"B2F", /* 1 */
};

static const char * const wid_values[] = {
	"200ms", /* 0 */
	"30s",   /* 1 */
	"60s",   /* 2 */
	"180s",  /* 3 */
};

static const char * const trigger_values[] = {
	"falling edge", /* 0 */
	"rising edge",  /* 1 */
	"both edges",   /* 2 */
	"low level",    /* 3 */
};

/* CPLD registers */

cpld_bf_ro(major_version, SMC_VERSION_REG,
	   SMC_MAJOR_VERSION, NULL, 0);
cpld_bf_ro(minor_version, SMC_VERSION_REG,
	   SMC_MINOR_VERSION, NULL, 0);
cpld_bf_rw(scratchpad, SMC_SW_SCRATCH_REG,
	   SMC_SCRATCHPAD, NULL, 0);
cpld_bf_ro(board_id, SMC_BOARD_ID_REG,
	   SMC_BOARD_ID, NULL, 0);
cpld_bt_rw(psu_pwr1_en, SMC_PSU_CONTROL_REG,
	   SMC_PSUR_CTRL, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr2_en, SMC_PSU_CONTROL_REG,
	   SMC_PSUL_CTRL, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_ac_ok, SMC_PSU_STATUS_REG,
	   SMC_PSUR_AC, NULL, 0);
cpld_bt_ro(psu_pwr1_dc_ok, SMC_PSU_STATUS_REG,
	   SMC_PSUR_POWER, NULL, 0);
cpld_bt_ro(psu_pwr1_alarm, SMC_PSU_STATUS_REG,
	   SMC_PSUR_ALERT, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_present, SMC_PSU_STATUS_REG,
	   SMC_PSUR_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_ac_ok, SMC_PSU_STATUS_REG,
	   SMC_PSUL_AC, NULL, 0);
cpld_bt_ro(psu_pwr2_dc_ok, SMC_PSU_STATUS_REG,
	   SMC_PSUL_POWER, NULL, 0);
cpld_bt_ro(psu_pwr2_alarm, SMC_PSU_STATUS_REG,
	   SMC_PSUL_ALERT, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_present, SMC_PSU_STATUS_REG,
	   SMC_PSUL_PRESENT, NULL, BF_COMPLEMENT);
cpld_bf_rw(led_fan1, SMC_FAN1_LED_REG,
	   SMC_FAN1_LED, led_fan_values, 0);
cpld_bf_rw(led_fan2, SMC_FAN2_LED_REG,
	   SMC_FAN2_LED, led_fan_values, 0);
cpld_bf_rw(led_fan3, SMC_FAN3_LED_REG,
	   SMC_FAN3_LED, led_fan_values, 0);
cpld_bt_rw(led_test_en, SMC_LED_OPMOD_REG,
	   SMC_OP_MOD, NULL, 0);
cpld_bf_rw(led_test, SMC_LED_TEST_REG,
	   SMC_OP_CTRL, led_test_values, 0);
cpld_bf_rw(led_status, SMC_FPS_LED_REG,
	   SMC_FPS_STA_LED, led_status_values, 0);
cpld_bf_rw(led_master, SMC_FPS_LED_REG,
	   SMC_FPS_MASTER_LED, led_master_values, 0);
cpld_bt_ro(usb_hub_state, SMC_DEV_STA_REG,
	   SMC_USB_HUB_STATE, NULL, 0);
cpld_bt_ro(fan3_direction, SMC_DEV_STA_REG,
	   SMC_FAN3_F2B_N, fan_direction_values, 0);
cpld_bt_ro(fan2_direction, SMC_DEV_STA_REG,
	   SMC_FAN2_F2B_N, fan_direction_values, 0);
cpld_bt_ro(fan1_direction, SMC_DEV_STA_REG,
	   SMC_FAN1_F2B_N, fan_direction_values, 0);
cpld_bt_rw(fan_eeprom_wp, SMC_FAN_EEPROM_WP_REG,
	   SMC_FAN_EEPROM_WP, NULL, 0);
cpld_bt_rw(eeprom_wp, SMC_EEPROM_WC_REG,
	   SMC_EEPROM_WC_N, NULL, 0);
cpld_bt_rw(usb_power_en, SMC_POWER_EN_REG,
	   SMC_USB_RST, NULL, 0);
cpld_bt_rw(xp1r5v_en, SMC_POWER_EN_REG,
	   SMC_XP1R5V_EN, NULL, 0);
cpld_bt_rw(xp1r0v_en, SMC_POWER_EN_REG,
	   SMC_XP1R0V_EN, NULL, 0);
cpld_bf_rw(wd_width, SMC_WD_WID_REG,
	   SMC_WD_WID, wid_values, 0);
cpld_bt_rw(wd_en, SMC_WD_MASK_REG,
	   SMC_WD_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pcie, SMC_SEP_RST_REG,
	   SMC_PCIE_RST_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_usbhub, SMC_SEP_RST_REG,
	   SMC_USBHUB_RST_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm50282, SMC_SEP_RST_REG,
	   SMC_BCM50282_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548a, SMC_SEP_RST_REG,
	   SMC_PCA9548A_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm5464, SMC_SEP_RST_REG,
	   SMC_BCM54616_RST, NULL, BF_COMPLEMENT);
cpld_bf_rw(ts_trig, SMC_SUS6_TRIG_MOD1_REG,
	   SMC_TS_TRIG, trigger_values, 0);
cpld_bf_rw(54616s_trig, SMC_SUS6_TRIG_MOD1_REG,
	   SMC_54616S_TRIG, trigger_values, 0);
cpld_bf_rw(b50282_trig, SMC_SUS6_TRIG_MOD1_REG,
	   SMC_B50282_TRIG, trigger_values, 0);
cpld_bf_rw(switch_trig, SMC_SUS6_TRIG_MOD2_REG,
	   SMC_SWITCH_TRIG, trigger_values, 0);
cpld_bf_rw(fan_tray_trig, SMC_SUS6_TRIG_MOD2_REG,
	   SMC_FAN_TRAY_TRIG, trigger_values, 0);
cpld_bf_rw(fan_ctrl_trig, SMC_SUS6_TRIG_MOD2_REG,
	   SMC_FAN_CTRL_TRIG, trigger_values, 0);
cpld_bf_rw(psu_trig, SMC_SUS6_TRIG_MOD2_REG,
	   SMC_PSU_TRIG, trigger_values, 0);
cpld_bt_ro(switch_combine, SMC_SUS6_COMBINE_REG,
	   SMC_SWITCH_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_tray_combine, SMC_SUS6_COMBINE_REG,
	   SMC_FAN_TRAY_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_ctrl_combine, SMC_SUS6_COMBINE_REG,
	   SMC_FAN_CTRL_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_combine, SMC_SUS6_COMBINE_REG,
	   SMC_PSU_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(ts_combine, SMC_SUS6_COMBINE_REG,
	   SMC_TS_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(54616s_combine, SMC_SUS6_COMBINE_REG,
	   SMC_54616S_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_combine, SMC_SUS6_COMBINE_REG,
	   SMC_B50282_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(thermal_int_stat, SMC_SUS6_STA1_REG,
	   SMC_THERMAL_INT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(54616s_int_stat, SMC_SUS6_STA1_REG,
	   SMC_54616S_INT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_6_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_6_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_5_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_5_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_4_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_4_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_3_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_3_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_2_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_2_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(b50282_1_int_stat, SMC_SUS6_STA1_REG,
	   SMC_B50282_1_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan3_present, SMC_SUS6_STA2_REG,
	   SMC_FAN3_PRESENT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan2_present, SMC_SUS6_STA2_REG,
	   SMC_FAN2_PRESENT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan1_present, SMC_SUS6_STA2_REG,
	   SMC_FAN1_PRESENT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_int_stat, SMC_SUS6_STA2_REG,
	   SMC_FAN_INT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_int_stat, SMC_SUS6_STA2_REG,
	   SMC_PSUR_ALERT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_int_stat, SMC_SUS6_STA2_REG,
	   SMC_PSUL_ALERT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_present_stat, SMC_SUS6_STA2_REG,
	   SMC_PSUR_PRES_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_present_stat, SMC_SUS6_STA2_REG,
	   SMC_PSUL_PRES_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(switch_int_stat, SMC_SUS6_STA3_REG,
	   SMC_SWITCH_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(ts_interrupt, SMC_SUS6_INT1_REG,
	   SMC_TS_INT, NULL, 0);
cpld_bt_ro(54616s_interrupt, SMC_SUS6_INT1_REG,
	   SMC_54616S_INT, NULL, 0);
cpld_bt_ro(b50282_6_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_6_INT, NULL, 0);
cpld_bt_ro(b50282_5_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_5_INT, NULL, 0);
cpld_bt_ro(b50282_4_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_4_INT, NULL, 0);
cpld_bt_ro(b50282_3_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_3_INT, NULL, 0);
cpld_bt_ro(b50282_2_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_2_INT, NULL, 0);
cpld_bt_ro(b50282_1_interrupt, SMC_SUS6_INT1_REG,
	   SMC_B50282_1_INT, NULL, 0);
cpld_bt_ro(fan3_tray_interrupt, SMC_SUS6_INT2_REG,
	   SMC_FAN3_TRAY_INT, NULL, 0);
cpld_bt_ro(fan2_tray_interrupt, SMC_SUS6_INT2_REG,
	   SMC_FAN2_TRAY_INT, NULL, 0);
cpld_bt_ro(fan1_tray_interrupt, SMC_SUS6_INT2_REG,
	   SMC_FAN1_TRAY_INT, NULL, 0);
cpld_bt_ro(fan_ctrl_interrupt, SMC_SUS6_INT2_REG,
	   SMC_FAN_CTRL_INT, NULL, 0);
cpld_bt_ro(psu_pwr1_alert_interrupt, SMC_SUS6_INT2_REG,
	   SMC_PSUR_ALERT_INT, NULL, 0);
cpld_bt_ro(psu_pwr2_alert_interrupt, SMC_SUS6_INT2_REG,
	   SMC_PSUL_ALERT_INT, NULL, 0);
cpld_bt_ro(psu_pwr1_present_interrupt, SMC_SUS6_INT2_REG,
	   SMC_PSUR_PRES_INT, NULL, 0);
cpld_bt_ro(psu_pwr2_present_interrupt, SMC_SUS6_INT2_REG,
	   SMC_PSUL_PRES_INT, NULL, 0);
cpld_bt_ro(switch_interrupt, SMC_SUS6_INT3_REG,
	   SMC_SWITCH_INT, NULL, 0);
cpld_bt_rw(ts_mask, SMC_SUS6_MASK1_REG,
	   SMC_TS_MASK, NULL, 0);
cpld_bt_rw(54616s_mask, SMC_SUS6_MASK1_REG,
	   SMC_54616S_MASK, NULL, 0);
cpld_bt_rw(b50282_6_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_6_MASK, NULL, 0);
cpld_bt_rw(b50282_5_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_5_MASK, NULL, 0);
cpld_bt_rw(b50282_4_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_4_MASK, NULL, 0);
cpld_bt_rw(b50282_3_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_3_MASK, NULL, 0);
cpld_bt_rw(b50282_2_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_2_MASK, NULL, 0);
cpld_bt_rw(b50282_1_mask, SMC_SUS6_MASK1_REG,
	   SMC_B50282_1_MASK, NULL, 0);
cpld_bt_rw(fan3_tray_mask, SMC_SUS6_MASK2_REG,
	   SMC_FAN3_TRAY_MASK, NULL, 0);
cpld_bt_rw(fan2_tray_mask, SMC_SUS6_MASK2_REG,
	   SMC_FAN2_TRAY_MASK, NULL, 0);
cpld_bt_rw(fan1_tray_mask, SMC_SUS6_MASK2_REG,
	   SMC_FAN1_TRAY_MASK, NULL, 0);
cpld_bt_rw(fan_ctrl_mask, SMC_SUS6_MASK2_REG,
	   SMC_FAN_CTRL_MASK, NULL, 0);
cpld_bt_rw(psu_pwr1_alert_mask, SMC_SUS6_MASK2_REG,
	   SMC_PSUR_ALERT_MASK, NULL, 0);
cpld_bt_rw(psu_pwr2_alert_mask, SMC_SUS6_MASK2_REG,
	   SMC_PSUL_ALERT_MASK, NULL, 0);
cpld_bt_rw(psu_pwr1_present_mask, SMC_SUS6_MASK2_REG,
	   SMC_PSUR_PRES_MASK, NULL, 0);
cpld_bt_rw(psu_pwr2_present_mask, SMC_SUS6_MASK2_REG,
	   SMC_PSUL_PRES_MASK, NULL, 0);
cpld_bt_rw(switch_mask, SMC_SUS6_MASK3_REG,
	   SMC_SWITCH_MASK, NULL, 0);
cpld_bf_rw(sfp_rxlos_trig, SMC_SUS7_TRIG_MOD_REG,
	   SMC_SFP_RXLOS_TRIG, trigger_values, 0);
cpld_bf_rw(sfp_present_trig, SMC_SUS7_TRIG_MOD_REG,
	   SMC_SFP_ABS_TRIG, trigger_values, 0);
cpld_bf_rw(sfp_tx_fault_trig, SMC_SUS7_TRIG_MOD_REG,
	   SMC_SFP_TXFLT_TRIG, trigger_values, 0);
cpld_bt_ro(sfp_rxlos_combine, SMC_SUS7_COMBINE_REG,
	   SMC_SFP_RXLOS_COMBINE, NULL, 0);
cpld_bt_ro(sfp_present_combine, SMC_SUS7_COMBINE_REG,
	   SMC_SFP_ABS_COMBINE, NULL, 0);
cpld_bt_ro(sfp_tx_fault_combine, SMC_SUS7_COMBINE_REG,
	   SMC_SFP_TXFLT_COMBINE, NULL, 0);
cpld_bt_ro(sfp4_tx_fault, SMC_SUS7_STA1_REG,
	   SMC_SFP4_TXFLT_STA, NULL, 0);
cpld_bt_ro(sfp3_tx_fault, SMC_SUS7_STA1_REG,
	   SMC_SFP3_TXFLT_STA, NULL, 0);
cpld_bt_ro(sfp2_tx_fault, SMC_SUS7_STA1_REG,
	   SMC_SFP2_TXFLT_STA, NULL, 0);
cpld_bt_ro(sfp1_tx_fault, SMC_SUS7_STA1_REG,
	   SMC_SFP1_TXFLT_STA, NULL, 0);
cpld_bt_ro(sfp4_present, SMC_SUS7_STA2_REG,
	   SMC_SFP4_ABS_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp3_present, SMC_SUS7_STA2_REG,
	   SMC_SFP3_ABS_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp2_present, SMC_SUS7_STA2_REG,
	   SMC_SFP2_ABS_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp1_present, SMC_SUS7_STA2_REG,
	   SMC_SFP1_ABS_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp4_rxlos, SMC_SUS7_STA3_REG,
	   SMC_SFP4_RXLOS_STA, NULL, 0);
cpld_bt_ro(sfp3_rxlos, SMC_SUS7_STA3_REG,
	   SMC_SFP3_RXLOS_STA, NULL, 0);
cpld_bt_ro(sfp2_rxlos, SMC_SUS7_STA3_REG,
	   SMC_SFP2_RXLOS_STA, NULL, 0);
cpld_bt_ro(sfp1_rxlos, SMC_SUS7_STA3_REG,
	   SMC_SFP1_RXLOS_STA, NULL, 0);
cpld_bt_ro(sfp4_tx_fault_interrupt, SMC_SUS7_INT1_REG,
	   SMC_SFP4_TXFLT_INT, NULL, 0);
cpld_bt_ro(sfp3_tx_fault_interrupt, SMC_SUS7_INT1_REG,
	   SMC_SFP3_TXFLT_INT, NULL, 0);
cpld_bt_ro(sfp2_tx_fault_interrupt, SMC_SUS7_INT1_REG,
	   SMC_SFP2_TXFLT_INT, NULL, 0);
cpld_bt_ro(sfp1_tx_fault_interrupt, SMC_SUS7_INT1_REG,
	   SMC_SFP1_TXFLT_INT, NULL, 0);
cpld_bt_ro(sfp4_present_interrupt, SMC_SUS7_INT2_REG,
	   SMC_SFP4_ABS_INT, NULL, 0);
cpld_bt_ro(sfp3_present_interrupt, SMC_SUS7_INT2_REG,
	   SMC_SFP3_ABS_INT, NULL, 0);
cpld_bt_ro(sfp2_present_interrupt, SMC_SUS7_INT2_REG,
	   SMC_SFP2_ABS_INT, NULL, 0);
cpld_bt_ro(sfp1_present_interrupt, SMC_SUS7_INT2_REG,
	   SMC_SFP1_ABS_INT, NULL, 0);
cpld_bt_ro(sfp4_rxlos_interrupt, SMC_SUS7_INT3_REG,
	   SMC_SFP4_RXLOS_INT, NULL, 0);
cpld_bt_ro(sfp3_rxlos_interrupt, SMC_SUS7_INT3_REG,
	   SMC_SFP3_RXLOS_INT, NULL, 0);
cpld_bt_ro(sfp2_rxlos_interrupt, SMC_SUS7_INT3_REG,
	   SMC_SFP2_RXLOS_INT, NULL, 0);
cpld_bt_ro(sfp1_rxlos_interrupt, SMC_SUS7_INT3_REG,
	   SMC_SFP1_RXLOS_INT, NULL, 0);
cpld_bt_rw(sfp4_tx_fault_mask, SMC_SUS7_MASK1_REG,
	   SMC_SFP4_TXFLT_MASK, NULL, 0);
cpld_bt_rw(sfp3_tx_fault_mask, SMC_SUS7_MASK1_REG,
	   SMC_SFP3_TXFLT_MASK, NULL, 0);
cpld_bt_rw(sfp2_tx_fault_mask, SMC_SUS7_MASK1_REG,
	   SMC_SFP2_TXFLT_MASK, NULL, 0);
cpld_bt_rw(sfp1_tx_fault_mask, SMC_SUS7_MASK1_REG,
	   SMC_SFP1_TXFLT_MASK, NULL, 0);
cpld_bt_rw(sfp4_present_mask, SMC_SUS7_MASK2_REG,
	   SMC_SFP4_ABS_MASK, NULL, 0);
cpld_bt_rw(sfp3_present_mask, SMC_SUS7_MASK2_REG,
	   SMC_SFP3_ABS_MASK, NULL, 0);
cpld_bt_rw(sfp2_present_mask, SMC_SUS7_MASK2_REG,
	   SMC_SFP2_ABS_MASK, NULL, 0);
cpld_bt_rw(sfp1_present_mask, SMC_SUS7_MASK2_REG,
	   SMC_SFP1_ABS_MASK, NULL, 0);
cpld_bt_rw(sfp4_rxlos_mask, SMC_SUS7_MASK3_REG,
	   SMC_SFP4_RXLOS_MASK, NULL, 0);
cpld_bt_rw(sfp3_rxlos_mask, SMC_SUS7_MASK3_REG,
	   SMC_SFP3_RXLOS_MASK, NULL, 0);
cpld_bt_rw(sfp2_rxlos_mask, SMC_SUS7_MASK3_REG,
	   SMC_SFP2_RXLOS_MASK, NULL, 0);
cpld_bt_rw(sfp1_rxlos_mask, SMC_SUS7_MASK3_REG,
	   SMC_SFP1_RXLOS_MASK, NULL, 0);
cpld_bt_rw(sfp4_rate_sel, SMC_SFPTX_CTRL_REG,
	   SMC_SFP4_RATE_SEL, NULL, 0);
cpld_bt_rw(sfp3_rate_sel, SMC_SFPTX_CTRL_REG,
	   SMC_SFP3_RATE_SEL, NULL, 0);
cpld_bt_rw(sfp2_rate_sel, SMC_SFPTX_CTRL_REG,
	   SMC_SFP2_RATE_SEL, NULL, 0);
cpld_bt_rw(sfp1_rate_sel, SMC_SFPTX_CTRL_REG,
	   SMC_SFP1_RATE_SEL, NULL, 0);
cpld_bt_rw(sfp4_tx_disable, SMC_SFPTX_CTRL_REG,
	   SMC_SFP4_TXEN, NULL, 0);
cpld_bt_rw(sfp3_tx_disable, SMC_SFPTX_CTRL_REG,
	   SMC_SFP3_TXEN, NULL, 0);
cpld_bt_rw(sfp2_tx_disable, SMC_SFPTX_CTRL_REG,
	   SMC_SFP2_TXEN, NULL, 0);
cpld_bt_rw(sfp1_tx_disable, SMC_SFPTX_CTRL_REG,
	   SMC_SFP1_TXEN, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_major_version.attr,
	&cpld_minor_version.attr,
	&cpld_scratchpad.attr,
	&cpld_board_id.attr,
	&cpld_psu_pwr1_en.attr,
	&cpld_psu_pwr2_en.attr,
	&cpld_psu_pwr1_ac_ok.attr,
	&cpld_psu_pwr1_dc_ok.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr2_ac_ok.attr,
	&cpld_psu_pwr2_dc_ok.attr,
	&cpld_psu_pwr2_alarm.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_led_fan1.attr,
	&cpld_led_fan2.attr,
	&cpld_led_fan3.attr,
	&cpld_led_test_en.attr,
	&cpld_led_test.attr,
	&cpld_led_status.attr,
	&cpld_led_master.attr,
	&cpld_usb_hub_state.attr,
	&cpld_fan3_direction.attr,
	&cpld_fan2_direction.attr,
	&cpld_fan1_direction.attr,
	&cpld_fan_eeprom_wp.attr,
	&cpld_eeprom_wp.attr,
	&cpld_usb_power_en.attr,
	&cpld_xp1r5v_en.attr,
	&cpld_xp1r0v_en.attr,
	&cpld_wd_width.attr,
	&cpld_wd_en.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_usbhub.attr,
	&cpld_reset_bcm50282.attr,
	&cpld_reset_pca9548a.attr,
	&cpld_reset_bcm5464.attr,
	&cpld_ts_trig.attr,
	&cpld_54616s_trig.attr,
	&cpld_b50282_trig.attr,
	&cpld_switch_trig.attr,
	&cpld_fan_tray_trig.attr,
	&cpld_fan_ctrl_trig.attr,
	&cpld_psu_trig.attr,
	&cpld_switch_combine.attr,
	&cpld_fan_tray_combine.attr,
	&cpld_fan_ctrl_combine.attr,
	&cpld_psu_combine.attr,
	&cpld_ts_combine.attr,
	&cpld_54616s_combine.attr,
	&cpld_b50282_combine.attr,
	&cpld_thermal_int_stat.attr,
	&cpld_54616s_int_stat.attr,
	&cpld_b50282_6_int_stat.attr,
	&cpld_b50282_5_int_stat.attr,
	&cpld_b50282_4_int_stat.attr,
	&cpld_b50282_3_int_stat.attr,
	&cpld_b50282_2_int_stat.attr,
	&cpld_b50282_1_int_stat.attr,
	&cpld_fan3_present.attr,
	&cpld_fan2_present.attr,
	&cpld_fan1_present.attr,
	&cpld_fan_int_stat.attr,
	&cpld_psu_pwr1_int_stat.attr,
	&cpld_psu_pwr2_int_stat.attr,
	&cpld_psu_pwr1_present_stat.attr,
	&cpld_psu_pwr2_present_stat.attr,
	&cpld_switch_int_stat.attr,
	&cpld_ts_interrupt.attr,
	&cpld_54616s_interrupt.attr,
	&cpld_b50282_6_interrupt.attr,
	&cpld_b50282_5_interrupt.attr,
	&cpld_b50282_4_interrupt.attr,
	&cpld_b50282_3_interrupt.attr,
	&cpld_b50282_2_interrupt.attr,
	&cpld_b50282_1_interrupt.attr,
	&cpld_fan3_tray_interrupt.attr,
	&cpld_fan2_tray_interrupt.attr,
	&cpld_fan1_tray_interrupt.attr,
	&cpld_fan_ctrl_interrupt.attr,
	&cpld_psu_pwr1_alert_interrupt.attr,
	&cpld_psu_pwr2_alert_interrupt.attr,
	&cpld_psu_pwr1_present_interrupt.attr,
	&cpld_psu_pwr2_present_interrupt.attr,
	&cpld_switch_interrupt.attr,
	&cpld_ts_mask.attr,
	&cpld_54616s_mask.attr,
	&cpld_b50282_6_mask.attr,
	&cpld_b50282_5_mask.attr,
	&cpld_b50282_4_mask.attr,
	&cpld_b50282_3_mask.attr,
	&cpld_b50282_2_mask.attr,
	&cpld_b50282_1_mask.attr,
	&cpld_fan3_tray_mask.attr,
	&cpld_fan2_tray_mask.attr,
	&cpld_fan1_tray_mask.attr,
	&cpld_fan_ctrl_mask.attr,
	&cpld_psu_pwr1_alert_mask.attr,
	&cpld_psu_pwr2_alert_mask.attr,
	&cpld_psu_pwr1_present_mask.attr,
	&cpld_psu_pwr2_present_mask.attr,
	&cpld_switch_mask.attr,
	&cpld_sfp_rxlos_trig.attr,
	&cpld_sfp_present_trig.attr,
	&cpld_sfp_tx_fault_trig.attr,
	&cpld_sfp_rxlos_combine.attr,
	&cpld_sfp_present_combine.attr,
	&cpld_sfp_tx_fault_combine.attr,
	&cpld_sfp4_tx_fault.attr,
	&cpld_sfp3_tx_fault.attr,
	&cpld_sfp2_tx_fault.attr,
	&cpld_sfp1_tx_fault.attr,
	&cpld_sfp4_present.attr,
	&cpld_sfp3_present.attr,
	&cpld_sfp2_present.attr,
	&cpld_sfp1_present.attr,
	&cpld_sfp4_rxlos.attr,
	&cpld_sfp3_rxlos.attr,
	&cpld_sfp2_rxlos.attr,
	&cpld_sfp1_rxlos.attr,
	&cpld_sfp4_tx_fault_interrupt.attr,
	&cpld_sfp3_tx_fault_interrupt.attr,
	&cpld_sfp2_tx_fault_interrupt.attr,
	&cpld_sfp1_tx_fault_interrupt.attr,
	&cpld_sfp4_present_interrupt.attr,
	&cpld_sfp3_present_interrupt.attr,
	&cpld_sfp2_present_interrupt.attr,
	&cpld_sfp1_present_interrupt.attr,
	&cpld_sfp4_rxlos_interrupt.attr,
	&cpld_sfp3_rxlos_interrupt.attr,
	&cpld_sfp2_rxlos_interrupt.attr,
	&cpld_sfp1_rxlos_interrupt.attr,
	&cpld_sfp4_tx_fault_mask.attr,
	&cpld_sfp3_tx_fault_mask.attr,
	&cpld_sfp2_tx_fault_mask.attr,
	&cpld_sfp1_tx_fault_mask.attr,
	&cpld_sfp4_present_mask.attr,
	&cpld_sfp3_present_mask.attr,
	&cpld_sfp2_present_mask.attr,
	&cpld_sfp1_present_mask.attr,
	&cpld_sfp4_rxlos_mask.attr,
	&cpld_sfp3_rxlos_mask.attr,
	&cpld_sfp2_rxlos_mask.attr,
	&cpld_sfp1_rxlos_mask.attr,
	&cpld_sfp4_rate_sel.attr,
	&cpld_sfp3_rate_sel.attr,
	&cpld_sfp2_rate_sel.attr,
	&cpld_sfp1_rate_sel.attr,
	&cpld_sfp4_tx_disable.attr,
	&cpld_sfp3_tx_disable.attr,
	&cpld_sfp2_tx_disable.attr,
	&cpld_sfp1_tx_disable.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *cpld_device;

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	cpld_regs = ioport_map(IO_BASE, IO_SIZE);

	if (!cpld_regs) {
		pr_err("cpld: unable to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}

	return ret;

err_unmap:
	iounmap(cpld_regs);

err_exit:
	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	iounmap(cpld_regs);
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static int __init cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cpld_driver);
	if (rv)
		goto err_exit;

	cpld_device = platform_device_alloc(DRIVER_NAME, 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(cpld_device);
	if (rv) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dealloc;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;

err_dealloc:
	platform_device_unregister(cpld_device);

err_unregister:
	platform_driver_unregister(&cpld_driver);

err_exit:
	pr_err("%s platform_driver_register failed (%i)\n",
	       DRIVER_NAME, rv);
	return rv;
}

static void __exit cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("Celestica Haliburton (E1031) SMC Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


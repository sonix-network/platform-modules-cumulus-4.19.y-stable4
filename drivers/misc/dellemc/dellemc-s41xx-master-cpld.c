// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc-s41xx-master-cpld.c - Dell EMC S41xx Master CPLD Support.
 *
 *  Copyright (C) 2017, 2019 Cumulus Networks, Inc.  All Rights Reserved
 *  Author: David Yen (dhyen@cumulusnetworks.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 */

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-s41xx-cplds.h"

#define DRIVER_NAME     MASTER_CPLD_DRIVER_NAME
#define DRIVER_VERSION	"1.1"

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

static const char * const mb_board_type_values[] = {
	"S4148F Non-NEBS",       /*  0 */
	"S4148F NEBS",           /*  1 */
	"S4128F Non-NEBS",       /*  2 */
	"S4128F NEBS",           /*  3 */
	"S4148T Non-NEBS",       /*  4 */
	"S4148T NEBS",           /*  5 */
	"S4128T Non-NEBS",       /*  6 */
	"S4128T NEBS",           /*  7 */
	"S4148FE Non-NEBS",      /*  8 */
	"S4148FE NEBS",          /*  9 */
	"reserved (10)",         /* 10 */
	"reserved (11)",         /* 11 */
	"S4148U Non-NEBS",       /* 12 */
	"S4148U NEBS",           /* 13 */
	"reserved (14)",         /* 14 */
	"reserved (15)"          /* 15 */
};

static const char * const mb_board_rev_values[] = {
	"X00",            /*  0 */
	"X01",            /*  1 */
	"X02",            /*  2 */
	"X03",            /*  3 */
	"reserved (4)",   /*  4 */
	"reserved (5)",   /*  5 */
	"reserved (6)",   /*  6 */
	"reserved (7)",   /*  7 */
	"reserved (8)",   /*  8 */
	"reserved (9)",   /*  9 */
	"reserved (10)",  /* 10 */
	"reserved (11)",  /* 11 */
	"reserved (12)",  /* 12 */
	"reserved (13)",  /* 13 */
	"reserved (14)",  /* 14 */
	"reserved (15)"   /* 15 */
};

static const char * const fan_power_led_colors[] = {
	PLATFORM_LED_OFF,           /*  0 */
	PLATFORM_LED_AMBER,         /*  1 */
	PLATFORM_LED_GREEN,         /*  2 */
	PLATFORM_LED_AMBER_BLINKING /*  3 */
};

static const char * const fan_led_status[] = {
	PLATFORM_LED_OFF,        /*  0 */
	PLATFORM_NOT_INSTALLED,  /*  1 */
	PLATFORM_OK,             /*  2 */
	"fan failed"             /*  3 */
};

static const char * const power_led_status[] = {
	"no power",          /*  0 */
	"post in progress",  /*  1 */
	PLATFORM_OK,         /*  2 */
	"failed"             /*  3 */
};

static const char * const system_led_colors[] = {
	PLATFORM_LED_GREEN_BLINKING,/*  0 */
	PLATFORM_LED_GREEN,         /*  1 */
	PLATFORM_LED_AMBER,         /*  2 */
	PLATFORM_LED_AMBER_BLINKING /*  3 */
};

static const char * const system_led_status[] = {
	"booting",                  /*  0 */
	PLATFORM_OK,                /*  1 */
	"major fault w/traffic",    /*  2 */
	"major fault non-traffic"   /*  3 */
};

static const char * const beacon_led_colors[] = {
	PLATFORM_LED_OFF,           /*  0 */
	PLATFORM_LED_BLUE_BLINKING  /*  1 */
};

static const char * const stack_led_colors[] = {
	PLATFORM_LED_GREEN,         /*  0 */
	PLATFORM_LED_OFF            /*  1 */
};

static const char * const led_7_segment_values[] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "A", "B", "C", "D", "E", "F"
};

static const char * const fan_tray_led_colors[] = {
	PLATFORM_LED_OFF,       /*  0 */
	PLATFORM_LED_GREEN,     /*  1 */
	PLATFORM_LED_AMBER,     /*  2 */
	PLATFORM_LED_OFF        /*  3 */
};

static const char * const led_test_values[] = {
	PLATFORM_LED_OFF,       /*  0 */
	PLATFORM_LED_AMBER,     /*  1 */
	PLATFORM_LED_GREEN,     /*  2 */
	"green_activity"        /*  3 */
};

static const char * const rov_values[] = {
	"unknown",              /*  0 */
	"1.0V",                 /*  1 */
	"0.95V",                /*  2 */
	"unknown",              /*  3 */
	"0.90V or 1.025V"       /*  4 */
	"0.97V"                 /*  5 */
	"unknown",              /*  6 */
	"unknown",              /*  7 */
};

cpld_bf_ro(master_cpld_major_version, DELL_S41XX_MSTR_CPLD_REV_REG,
	   DELL_S41XX_MSTR_CPLD_REV_MJR_REV, NULL, BF_DECIMAL);
cpld_bf_ro(master_cpld_minor_version, DELL_S41XX_MSTR_CPLD_REV_REG,
	   DELL_S41XX_MSTR_CPLD_REV_MNR_REV, NULL, BF_DECIMAL);
mk_bf_rw(cpld, scratch, DELL_S41XX_MSTR_CPLD_GPR_REG, 0, 8, NULL, 0);
cpld_bf_ro(master_cpld_mb_board_revision, DELL_S41XX_MSTR_MB_BRD_REV_TYPE_REG,
	   DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_REV, mb_board_rev_values, 0);
cpld_bf_ro(master_cpld_mb_board_type, DELL_S41XX_MSTR_MB_BRD_REV_TYPE_REG,
	   DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_TYPE, mb_board_type_values, 0);
mk_bf_rw(cpld, port_led2_reset, DELL_S41XX_MSTR_SRR_REG,
	 DELL_S41XX_MSTR_SRR_PORT_LED_RESET_MSB, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, port_led1_reset, DELL_S41XX_MSTR_SRR_REG,
	 DELL_S41XX_MSTR_SRR_PORT_LED_RESET_LSB, 1, NULL, BF_COMPLEMENT);
cpld_bt_rw(main_board_reset, DELL_S41XX_MSTR_SRR_REG,
	   DELL_S41XX_MSTR_SRR_MB_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(micro_usb_reset, DELL_S41XX_MSTR_SRR_REG,
	   DELL_S41XX_MSTR_SRR_MICRO_USB_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(m1588_reset, DELL_S41XX_MSTR_SRR_REG,
	   DELL_S41XX_MSTR_SRR_1588_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(npu_reset, DELL_S41XX_MSTR_SRR_REG,
	   DELL_S41XX_MSTR_SRR_NPU_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(mgmt_phy_reset, DELL_S41XX_MSTR_SRR_REG,
	   DELL_S41XX_MSTR_SRR_MGMT_PHY_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(fan4_eeprom_wp, DELL_S41XX_MSTR_FAN_EEPROM_WP_REG,
	   DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN4_EEPROM_WP, NULL, 0);
cpld_bt_rw(fan3_eeprom_wp, DELL_S41XX_MSTR_FAN_EEPROM_WP_REG,
	   DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN3_EEPROM_WP, NULL, 0);
cpld_bt_rw(fan2_eeprom_wp, DELL_S41XX_MSTR_FAN_EEPROM_WP_REG,
	   DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN2_EEPROM_WP, NULL, 0);
cpld_bt_rw(fan1_eeprom_wp, DELL_S41XX_MSTR_FAN_EEPROM_WP_REG,
	   DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN1_EEPROM_WP, NULL, 0);
cpld_bf_rw(led_fan, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_FAN_LED, fan_power_led_colors, 0);
cpld_bf_rw(fan_led_status, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_FAN_LED, fan_led_status, 0);
cpld_bf_rw(led_system, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_SYSTEM, system_led_colors, 0);
cpld_bf_rw(system_led_status, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_SYSTEM, system_led_status, 0);
cpld_bt_rw(beacon_led_color, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_BEACON, beacon_led_colors, 0);
cpld_bf_rw(led_power, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_POWER, fan_power_led_colors, 0);
cpld_bf_rw(power_led_status, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_POWER, power_led_status, 0);
cpld_bt_rw(stack_led_color, DELL_S41XX_MSTR_SYSTEM_LED_REG,
	   DELL_S41XX_MSTR_SYSTEM_LED_STACK_LED, stack_led_colors, 0);
cpld_bt_rw(7_segment_blink, DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_REG,
	   DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_LED_BLNK, NULL, 0);
cpld_bt_rw(7_segment_on, DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_REG,
	   DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_LED_OFF, NULL, 0);
cpld_bt_rw(7_segment_dot_on, DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_REG,
	   DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_DOT, NULL, 0);
cpld_bf_rw(7_segment_value, DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_REG,
	   DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_DGT, led_7_segment_values, 0);
cpld_bf_rw(led_fan_tray4, DELL_S41XX_MSTR_FAN_TRAY_LED_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY4_LED, fan_tray_led_colors, 0);
cpld_bf_rw(led_fan_tray3, DELL_S41XX_MSTR_FAN_TRAY_LED_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY3_LED, fan_tray_led_colors, 0);
cpld_bf_rw(led_fan_tray2, DELL_S41XX_MSTR_FAN_TRAY_LED_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY2_LED, fan_tray_led_colors, 0);
cpld_bf_rw(led_fan_tray1, DELL_S41XX_MSTR_FAN_TRAY_LED_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY1_LED, fan_tray_led_colors, 0);
cpld_bt_ro(system_fan4_present, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY4_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan3_present, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY3_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan2_present, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY2_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan1_present, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY1_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan4_ok, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY4_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan3_ok, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY3_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan2_ok, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY2_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(system_fan1_ok, DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG,
	   DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY1_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(cpld_led_clk, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_LED_PT_REGEN, NULL, 0);
cpld_bf_rw(led_test, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_LED_TEST, led_test_values, 0);
cpld_bt_rw(cpld_spi_wp, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_CPLD_SPI_WP, NULL, BF_COMPLEMENT);
cpld_bt_ro(micro_usb_suspend, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_MICRO_USB_SUSPEND, NULL, 0);
cpld_bt_ro(micro_usb_present, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_MICRO_, NULL, 0);
cpld_bt_rw(npu_console, DELL_S41XX_MSTR_MISC_CTRL_REG,
	   DELL_S41XX_MSTR_MISC_CTRL_CNSL_SEL, NULL, 0);
cpld_bt_ro(psu_pwr1_present, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_PS, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_all_ok, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_PG, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_fault, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_INT, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr1_enable, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_ON, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_present, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_PS, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_all_ok, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_PG, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_fault, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_INT, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr2_enable, DELL_S41XX_MSTR_PSU_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_ON, NULL, BF_COMPLEMENT);
cpld_bt_rw(usb_vbus_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_USB1_VBUS_EN, NULL, 0);
cpld_bt_rw(vcc_1v25_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_VCC_1V25_EN, NULL, 0);
cpld_bt_rw(hot_swap1_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_HOT_SWAP1_EN, NULL, 0);
cpld_bt_rw(hot_swap2_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_HOT_SWAP2_EN, NULL, 0);
cpld_bt_rw(mac_avs1v_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_MAC_AVS1V_EN, NULL, 0);
cpld_bt_rw(mac_1v_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_MAC1V_EN, NULL, 0);
cpld_bt_rw(v3p3_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_V3P3_EN, NULL, 0);
cpld_bt_rw(v5p0_enable, DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_EN_STATUS_V5P0_EN, NULL, 0);
cpld_bt_ro(vcc_3p3_cpld_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_VCC_3P3_CPLD, NULL, 0);
cpld_bt_ro(vcc_1v25_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_VCC_1V25_PG, NULL, 0);
cpld_bt_ro(hot_swap1_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_HOT_SWAP_PG1, NULL, 0);
cpld_bt_ro(hot_swap2_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_HOT_SWAP_PG2, NULL, 0);
cpld_bt_ro(mac_avs1v_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_MAC_AVS1V_PG, NULL, 0);
cpld_bt_ro(mac_1v_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_MAC1V_PG, NULL, 0);
cpld_bt_ro(v3p3_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_VCC3V3_PG, NULL, 0);
cpld_bt_ro(v5p0_good, DELL_S41XX_MSTR_MB_PWR_STATUS_REG,
	   DELL_S41XX_MSTR_MB_PWR_STATUS_VCC5V_PG, NULL, 0);
cpld_bf_ro(rov, DELL_S41XX_MSTR_NPU_ROV_REG,
	   DELL_S41XX_MSTR_NPU__ROV, rov_values, 0);
mk_bf_ro(cpld, reboot_cause, DELL_S41XX_MSTR_MB_REBOOT_CAUSE_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp_30_25_present, DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_REG,
	 DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT25_PRESENT_BIT,
	 DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT30_PRESENT_BIT -
	 DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT25_PRESENT_BIT + 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp_30_25_reset, DELL_S41XX_MSTR_QSFP_RST_REG,
	 DELL_S41XX_MSTR_QSFP_RST_PORT25_RST_BIT,
	 DELL_S41XX_MSTR_QSFP_RST_PORT30_RST_BIT -
	 DELL_S41XX_MSTR_QSFP_RST_PORT25_RST_BIT + 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp_30_25_modsel, DELL_S41XX_MSTR_QSFP_MODSEL_REG,
	 DELL_S41XX_MSTR_QSFP_MODSEL_PORT25_MODSEL_BIT,
	 DELL_S41XX_MSTR_QSFP_MODSEL_PORT30_MODSEL_BIT -
	 DELL_S41XX_MSTR_QSFP_MODSEL_PORT25_MODSEL_BIT + 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp_30_25_lpmode, DELL_S41XX_MSTR_QSFP_LPMODE_REG,
	 DELL_S41XX_MSTR_QSFP_LPMODE_PORT25_LPMODE_BIT,
	 DELL_S41XX_MSTR_QSFP_LPMODE_PORT30_LPMODE_BIT -
	 DELL_S41XX_MSTR_QSFP_LPMODE_PORT25_LPMODE_BIT + 1,
	 NULL, 0);
mk_bf_ro(cpld, sfp_16_1_present,
	 DELL_S41XX_MSTR_PORT_8_1_PRESENT_STATUS_REG, 0, 16, NULL,
	 BF_COMPLEMENT);
mk_bf_rw(cpld, sfp_16_1_tx_disable,
	 DELL_S41XX_MSTR_PORT_8_1_TX_DISABLE_REG, 0, 16, NULL, 0);
mk_bf_ro(cpld, sfp_16_1_rx_los,
	 DELL_S41XX_MSTR_PORT_8_1_RX_LOS_STATUS_REG, 0, 16, NULL, 0);
mk_bf_ro(cpld, sfp_16_1_tx_fault,
	 DELL_S41XX_MSTR_PORT_8_1_TX_FAULT_STATUS_REG, 0, 16, NULL, 0);
mk_bf_rw(cpld, phy_reset, DELL_S41XX_MSTR_PHY_8_1_RST_REG, 0, 12, NULL,
	 BF_COMPLEMENT);
cpld_bt_rw(phy_vcc2v5_enable, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_VCC2V5_EN, NULL, 0);
cpld_bt_rw(phy_ct1v9_enable, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_CT1V9_EN, NULL, 0);
cpld_bt_rw(phy_1vr_enable, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_R_EN, NULL, 0);
cpld_bt_rw(phy_1vl_enable, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_L_EN, NULL, 0);
cpld_bt_ro(phy_vcc2v5_good, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_VCC2V5_PG, NULL, 0);
cpld_bt_ro(phy_ct1v9_good, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_CT1V9_PG, NULL, 0);
cpld_bt_ro(phy_1vr_good, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_R_PG, NULL, 0);
cpld_bt_ro(phy_1vl_good, DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG,
	   DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_L_PG, NULL, 0);
cpld_bt_ro(phy_pll_lock, DELL_S41XX_MSTR_PLL_STATUS_REG,
	   DELL_S41XX_MSTR_PLL_STATUS_PLL_STS, NULL, 0);
mk_bf_rw(cpld, broadsync_accuracy, DELL_S41XX_MSTR_ACCURACY_7_0_REG,
	 0, 8, NULL, BF_DECIMAL);
mk_bf_rw(cpld, broadsync_nanosec, DELL_S41XX_MSTR_NANOSEC_7_0_REG,
	 0, 30, NULL, BF_DECIMAL);
mk_bf_rw(cpld, broadsync_sec, DELL_S41XX_MSTR_SEC_7_0_REG,
	 0, 32, NULL, BF_DECIMAL);
mk_bf_rw(cpld, broadsync_epoch, DELL_S41XX_MSTR_EPOCH_7_0_REG,
	 0, 16, NULL, BF_DECIMAL);
cpld_bt_rw(broadsync_lock, DELL_S41XX_MSTR_LOCK_7_0_REG,
	   DELL_S41XX_MSTR_LOCK_7_0_LOCK, NULL, 0);

/*
 * SYSFS attributes
 */
static struct attribute *cpld_attrs[] = {
	&cpld_master_cpld_major_version.attr,
	&cpld_master_cpld_minor_version.attr,
	&cpld_scratch.attr,
	&cpld_master_cpld_mb_board_revision.attr,
	&cpld_master_cpld_mb_board_type.attr,
	&cpld_port_led2_reset.attr,
	&cpld_port_led1_reset.attr,
	&cpld_main_board_reset.attr,
	&cpld_micro_usb_reset.attr,
	&cpld_m1588_reset.attr,
	&cpld_npu_reset.attr,
	&cpld_mgmt_phy_reset.attr,
	&cpld_fan4_eeprom_wp.attr,
	&cpld_fan3_eeprom_wp.attr,
	&cpld_fan2_eeprom_wp.attr,
	&cpld_fan1_eeprom_wp.attr,
	&cpld_led_fan.attr,
	&cpld_fan_led_status.attr,
	&cpld_led_system.attr,
	&cpld_system_led_status.attr,
	&cpld_beacon_led_color.attr,
	&cpld_led_power.attr,
	&cpld_power_led_status.attr,
	&cpld_stack_led_color.attr,
	&cpld_7_segment_blink.attr,
	&cpld_7_segment_on.attr,
	&cpld_7_segment_dot_on.attr,
	&cpld_7_segment_value.attr,
	&cpld_led_fan_tray4.attr,
	&cpld_led_fan_tray3.attr,
	&cpld_led_fan_tray2.attr,
	&cpld_led_fan_tray1.attr,
	&cpld_system_fan4_present.attr,
	&cpld_system_fan3_present.attr,
	&cpld_system_fan2_present.attr,
	&cpld_system_fan1_present.attr,
	&cpld_system_fan4_ok.attr,
	&cpld_system_fan3_ok.attr,
	&cpld_system_fan2_ok.attr,
	&cpld_system_fan1_ok.attr,
	&cpld_cpld_led_clk.attr,
	&cpld_led_test.attr,
	&cpld_cpld_spi_wp.attr,
	&cpld_micro_usb_suspend.attr,
	&cpld_micro_usb_present.attr,
	&cpld_npu_console.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_fault.attr,
	&cpld_psu_pwr1_enable.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_fault.attr,
	&cpld_psu_pwr2_enable.attr,
	&cpld_usb_vbus_enable.attr,
	&cpld_vcc_1v25_enable.attr,
	&cpld_hot_swap1_enable.attr,
	&cpld_hot_swap2_enable.attr,
	&cpld_mac_avs1v_enable.attr,
	&cpld_mac_1v_enable.attr,
	&cpld_v3p3_enable.attr,
	&cpld_v5p0_enable.attr,
	&cpld_vcc_3p3_cpld_good.attr,
	&cpld_vcc_1v25_good.attr,
	&cpld_hot_swap1_good.attr,
	&cpld_hot_swap2_good.attr,
	&cpld_mac_avs1v_good.attr,
	&cpld_mac_1v_good.attr,
	&cpld_v3p3_good.attr,
	&cpld_v5p0_good.attr,
	&cpld_rov.attr,
	&cpld_reboot_cause.attr,
	&cpld_qsfp_30_25_present.attr,
	&cpld_qsfp_30_25_reset.attr,
	&cpld_qsfp_30_25_modsel.attr,
	&cpld_qsfp_30_25_lpmode.attr,
	&cpld_sfp_16_1_present.attr,
	&cpld_sfp_16_1_tx_disable.attr,
	&cpld_sfp_16_1_rx_los.attr,
	&cpld_sfp_16_1_tx_fault.attr,
	&cpld_phy_reset.attr,
	&cpld_phy_vcc2v5_enable.attr,
	&cpld_phy_ct1v9_enable.attr,
	&cpld_phy_1vr_enable.attr,
	&cpld_phy_1vl_enable.attr,
	&cpld_phy_vcc2v5_good.attr,
	&cpld_phy_ct1v9_good.attr,
	&cpld_phy_1vr_good.attr,
	&cpld_phy_1vl_good.attr,
	&cpld_phy_pll_lock.attr,
	&cpld_broadsync_accuracy.attr,
	&cpld_broadsync_nanosec.attr,
	&cpld_broadsync_sec.attr,
	&cpld_broadsync_epoch.attr,
	&cpld_broadsync_lock.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int boardrev;
	int cpldrev;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev,
			"adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Probe the hardware by reading the revision numbers.
	 */
	ret = i2c_smbus_read_byte_data(client, DELL_S41XX_MSTR_CPLD_REV_REG);
	if (ret < 0) {
		dev_err(dev, "read cpld revision register error %d\n", ret);
		goto err;
	}
	cpldrev = ret;
	ret = i2c_smbus_read_byte_data(client,
				       DELL_S41XX_MSTR_MB_BRD_REV_TYPE_REG);
	if (ret < 0) {
		dev_err(dev, "read board revision register error %d\n", ret);
		goto err;
	}
	boardrev = ret;

	/*
	 * Create sysfs nodes.
	 */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "sysfs_create_group failed\n");
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_info(dev,
		 "device created, board type %ld rev X%ld, CPLD rev %ld.%ld\n",
		 GET_FIELD(boardrev, DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_TYPE),
		 GET_FIELD(boardrev, DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_REV),
		 GET_FIELD(cpldrev, DELL_S41XX_MSTR_CPLD_REV_MJR_REV),
		 GET_FIELD(cpldrev, DELL_S41XX_MSTR_CPLD_REV_MNR_REV));

err:
	return ret;
}

static int cpld_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;

	sysfs_remove_group(&dev->kobj, &cpld_attr_group);
	return 0;
}

static const struct i2c_device_id cpld_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe_new = cpld_probe,
	.remove = cpld_remove,
	.id_table = cpld_id,
};

/*
 * Module init/exit
 */
static int __init cpld_init(void)
{
	int ret = 0;

	/*
	 * Do as little as possible in the module init function. Basically just
	 * register drivers. Those driver's probe functions will probe for
	 * hardware and create devices.
	 */
	pr_info(DRIVER_NAME
		": version " DRIVER_VERSION " initializing\n");

	/* Register the I2C CPLD driver for the CPLD */
	ret = i2c_add_driver(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": %s driver registration failed. (%d)\n",
		       cpld_driver.driver.name, ret);
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");
	return ret;
}

static void __exit cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S41xx master cpld support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

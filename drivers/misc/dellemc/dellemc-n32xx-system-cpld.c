// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc_n32xx-system-cpld.c - Dell EMC N32xx System CPLD support.
 *
 *  Copyright (C) 2017, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved
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
#include <linux/i2c-ismt.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-n32xx-n22xx-cplds.h"

#define DRIVER_NAME     SYSTEM_CPLD_DRIVER_NAME
#define DRIVER_VERSION	"1.2"

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* System CPLD register bitfields with enum-line values */

static const char * const led_fan_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_system_values[] = {
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_beacon_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_BLUE_BLINKING,
};

static const char * const led_power_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_stack_values[] = {
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static const char * const digit_values[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
};

static const char * const led_fan_tray_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_OFF,
};

static const char * const airflow_values[] = {
	"F2B",
	"B2F",
};

static const char * const wd_timer_values[] = {
	"15 sec",
	"20 sec",
	"30 sec",
	"40 sec",
	"50 sec",
	"60 sec",
	"65 sec",
	"70 sec",
};

/* System CPLD registers */

cpld_rg_ro(minor_revision, DELL_N32XX_SYS_CPLD_REV_REG0_REG, NULL, BF_DECIMAL);
cpld_rg_ro(major_revision, DELL_N32XX_SYS_CPLD_REV_REG1_REG, NULL, BF_DECIMAL);
cpld_rg_rw(scratch, DELL_N32XX_SYS_CPLD_GPR_REG, NULL, 0);
cpld_bf_ro(board_revision, DELL_N32XX_SYS_MB_BRD_REV_TYPE_REG,
	   DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_REV, NULL, 0);
cpld_bf_ro(board_type, DELL_N32XX_SYS_MB_BRD_REV_TYPE_REG,
	   DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_TYPE, NULL, 0);
cpld_bt_rw(reset_port_led, DELL_N32XX_SYS_SRR_REG,
	   DELL_N32XX_SYS_SRR_PORT_LED_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(poe_reset_disable, DELL_N32XX_SYS_SRR_REG,
	   DELL_N32XX_SYS_SRR_POE_RST_DSBL, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_npu, DELL_N32XX_SYS_SRR_REG, DELL_N32XX_SYS_SRR_NPU1_RST,
	   NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_mgmt_phy, DELL_N32XX_SYS_SRR_REG,
	   DELL_N32XX_SYS_SRR_MGMT_PHY_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(cpld_spi_wp, DELL_N32XX_SYS_EEPROM_WP_REG,
	   DELL_N32XX_SYS_EEPROM_WP_CPLD_SPI_WP, NULL, BF_COMPLEMENT);
cpld_bt_rw(sys_eeprom_wp, DELL_N32XX_SYS_EEPROM_WP_REG,
	   DELL_N32XX_SYS_EEPROM_WP_SYS_EEPROM_WP, NULL, BF_COMPLEMENT);
cpld_bt_rw(fan3_eeprom_wp, DELL_N32XX_SYS_EEPROM_WP_REG,
	   DELL_N32XX_SYS_EEPROM_WP_FAN3_EEPROM_WP, NULL, BF_COMPLEMENT);
cpld_bt_rw(fan2_eeprom_wp, DELL_N32XX_SYS_EEPROM_WP_REG,
	   DELL_N32XX_SYS_EEPROM_WP_FAN2_EEPROM_WP, NULL, BF_COMPLEMENT);
cpld_bt_rw(fan1_eeprom_wp, DELL_N32XX_SYS_EEPROM_WP_REG,
	   DELL_N32XX_SYS_EEPROM_WP_FAN1_EEPROM_WP, NULL, BF_COMPLEMENT);
cpld_bt_ro(tpm_int, DELL_N32XX_SYS_IRQ_STS_REG,
	   DELL_N32XX_SYS_IRQ_STS_TPM_INT, NULL, BF_COMPLEMENT);
cpld_bt_ro(usb_fault, DELL_N32XX_SYS_IRQ_STS_REG,
	   DELL_N32XX_SYS_IRQ_STS_USB_FAULT, NULL, BF_COMPLEMENT);
cpld_bt_ro(bcm54216s_int, DELL_N32XX_SYS_IRQ_STS_REG,
	   DELL_N32XX_SYS_IRQ_STS_BCM54216S_INT, NULL, BF_COMPLEMENT);
cpld_bt_ro(ucd_irq, DELL_N32XX_SYS_IRQ_STS_REG,
	   DELL_N32XX_SYS_IRQ_STS_UCD_IRQ, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_alert_int, DELL_N32XX_SYS_IRQ_STS_REG,
	   DELL_N32XX_SYS_IRQ_STS_FAN_ALERT_INT, NULL, BF_COMPLEMENT);
cpld_bf_rw(led_fan, DELL_N32XX_SYS_SYSTEM_LED_REG,
	   DELL_N32XX_SYS_SYSTEM_LED_FAN_LED, led_fan_values, 0);
cpld_bf_rw(led_system, DELL_N32XX_SYS_SYSTEM_LED_REG,
	   DELL_N32XX_SYS_SYSTEM_LED_SYSTEM, led_system_values, 0);
cpld_bt_rw(led_beacon, DELL_N32XX_SYS_SYSTEM_LED_REG,
	   DELL_N32XX_SYS_SYSTEM_LED_BEACON, led_beacon_values, 0);
cpld_bf_rw(led_power, DELL_N32XX_SYS_SYSTEM_LED_REG,
	   DELL_N32XX_SYS_SYSTEM_LED_POWER, led_power_values, 0);
cpld_bt_rw(led_stack, DELL_N32XX_SYS_SYSTEM_LED_REG,
	   DELL_N32XX_SYS_SYSTEM_LED_STACK_LED, led_stack_values, 0);
cpld_bt_rw(digit_blink, DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_REG,
	   DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_LED_BLNK, NULL, 0);
cpld_bt_rw(digit_on, DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_REG,
	   DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_LED_OFF, NULL, 0);
cpld_bt_rw(digit_dot, DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_REG,
	   DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_DOT, NULL, 0);
cpld_bf_rw(digit, DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_REG,
	   DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_DGT, digit_values, 0);
cpld_bf_rw(led_fan_tray3, DELL_N32XX_SYS_FAN_TRAY_LED_REG,
	   DELL_N32XX_SYS_FAN_TRAY_LED3_FAN, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray2, DELL_N32XX_SYS_FAN_TRAY_LED_REG,
	   DELL_N32XX_SYS_FAN_TRAY_LED2_FAN, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray1, DELL_N32XX_SYS_FAN_TRAY_LED_REG,
	   DELL_N32XX_SYS_FAN_TRAY_LED2_FAN, led_fan_tray_values, 0);
cpld_bt_ro(fan_tray3_type, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN3_TYPE, airflow_values, 0);
cpld_bt_ro(fan_tray2_type, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN2_TYPE, airflow_values, 0);
cpld_bt_ro(fan_tray1_type, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN1_TYPE, airflow_values, 0);
cpld_bt_ro(fan_tray3_present, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN3_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_tray2_present, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN2_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_tray1_present, DELL_N32XX_SYS_FAN_TRAY_STATUS_REG,
	   DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN1_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_rw(sys_irq_enable, DELL_N32XX_SYS_MISC_CTRL_REG,
	   DELL_N32XX_SYS_MISC_CTRL_SYS_IRQ_EN, NULL, 0);
cpld_bt_rw(fan_enable, DELL_N32XX_SYS_MISC_CTRL_REG,
	   DELL_N32XX_SYS_MISC_CTRL_FAN_EN, NULL, 0);
cpld_bt_ro(psu_pwr1_present, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS1_PS, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_all_ok, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS1_PG, NULL, 0);
cpld_bt_ro(psu_pwr1_alarm, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS1_INT, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr1_enable, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS1_ON, NULL, 0);
cpld_bt_ro(psu_pwr2_present, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS2_PS, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_all_ok, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS2_PG, NULL, 0);
cpld_bt_ro(psu_pwr2_alarm, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS2_INT, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr2_enable, DELL_N32XX_SYS_PSU_EN_STATUS_REG,
	   DELL_N32XX_SYS_PSU_EN_STATUS_PS1_ON, NULL, 0);
cpld_bt_rw(reset_poe, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_CTRL_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(poe_enable, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_PORT_DIS, NULL, 0);
cpld_bt_ro(poe_i2c_ready, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_I2C_RDY, NULL, BF_COMPLEMENT);
cpld_bt_ro(poe_int, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_INT, NULL, BF_COMPLEMENT);
cpld_bt_ro(poe_temp_alarm, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_TMP_ALRM, NULL, 0);
cpld_bt_ro(poe_system_ok, DELL_N32XX_SYS_POE_EN_STATUS_REG,
	   DELL_N32XX_SYS_POE_EN_STATUS_POE_SYSTEM_OK, NULL, BF_COMPLEMENT);
cpld_rg_rw(reboot_cause, DELL_N32XX_SYS_MB_REBOOT_CAUSE_REG, NULL, 0);
cpld_bf_rw(port_i2c_mux, DELL_N32XX_SYS_PORT_I2C_MUX_REG,
	   DELL_N32XX_SYS_PORT_I2C_MUX, NULL, 0);
cpld_bf_rw(fan_i2c_mux, DELL_N32XX_SYS_FAN_I2C_MUX_REG,
	   DELL_N32XX_SYS_FAN_I2C_MUX, NULL, 0);
cpld_bf_rw(psu_i2c_mux, DELL_N32XX_SYS_PSU_I2C_MUX_REG,
	   DELL_N32XX_SYS_PSU_I2C_MUX, NULL, 0);
cpld_bt_rw(usba_power_enable, DELL_N32XX_SYS_CTRL_REG,
	   DELL_N32XX_SYS_CTRL_USBA_PWR_EN, NULL, BF_COMPLEMENT);
cpld_bt_rw(power_down, DELL_N32XX_SYS_CTRL_REG,
	   DELL_N32XX_SYS_CTRL_PWR_DN, NULL, 0);
cpld_bt_rw(power_cycle, DELL_N32XX_SYS_CTRL_REG,
	   DELL_N32XX_SYS_CTRL_PWR_CYC_SYS, NULL, 0);
cpld_bf_rw(wd_timer, DELL_N32XX_SYS_WD_REG,
	   DELL_N32XX_SYS_WD_WD_TIMER, wd_timer_values, 0);
cpld_bt_rw(wd_enable, DELL_N32XX_SYS_WD_REG, DELL_N32XX_SYS_WD_WD_EN, NULL, 0);
cpld_bt_rw(wd_punch, DELL_N32XX_SYS_WD_REG,
	   DELL_N32XX_SYS_WD_WD_PUNCH, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_ac_ok, DELL_N32XX_SYS_MISC_STS_REG,
	   DELL_N32XX_SYS_MISC_STS_PSU2_AC_GOOD, NULL, 0);
cpld_bt_ro(psu_pwr1_ac_ok, DELL_N32XX_SYS_MISC_STS_REG,
	   DELL_N32XX_SYS_MISC_STS_PSU1_AC_GOOD, NULL, 0);
cpld_bf_ro(qsfp28_54_53_present, DELL_N32XX_SYS_QSFP_PRESENT_STATUS_REG,
	   DELL_N32XX_SYS_QSFP_PRESENT_STATUS_PORT_PRESENT, NULL,
	   BF_COMPLEMENT);
cpld_bf_rw(qsfp28_54_53_reset, DELL_N32XX_SYS_QSFP_RST_REG,
	   DELL_N32XX_SYS_QSFP_RST_PORT_RST, NULL, BF_COMPLEMENT);
cpld_bf_rw(qsfp28_54_53_lpmode, DELL_N32XX_SYS_QSFP_LPMODE_REG,
	   DELL_N32XX_SYS_QSFP_LPMODE_PORT_LPMODE, NULL, 0);
cpld_bf_ro(qsfp28_54_53_irq_status, DELL_N32XX_SYS_QSFP_IRQ_STATUS_REG,
	   DELL_N32XX_SYS_QSFP_IRQ_STATUS_PORT_IRQ_STATUS, NULL, BF_COMPLEMENT);
cpld_bf_rw(qsfp28_54_53_irq_mask, DELL_N32XX_SYS_QSFP_IRQ_MSK_REG,
	   DELL_N32XX_SYS_QSFP_IRQ_MSK_PORT_IRQ_MSK, NULL, BF_COMPLEMENT);
cpld_bf_ro(sfp28_52_49_present,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_PRESENT_STATUS_REG,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_PRESENT_STATUS_PRSNT_STS,
	   NULL, BF_COMPLEMENT);
cpld_bf_rw(sfp28_52_49_tx_disable,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_DISABLE_REG,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1__TX_DISABLE, NULL, 0);
cpld_bf_ro(sfp28_52_49_rx_los, DELL_N32XX_SYS_UPLINK_PORT_4_1_RX_LOS_STATUS_REG,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_RX_LOS_STATUS_RX_LOS, NULL, 0);
cpld_bf_ro(sfp28_52_49_tx_fault,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_FAULT_STATUS_REG,
	   DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_FAULT_STATUS_TX_FAULT, NULL, 0);
cpld_bf_rw(phy_reset, DELL_N32XX_SYS_PHY_7_1_RST_REG,
	   DELL_N32XX_SYS_PHY_7_1_RST_PHY_RST, NULL, BF_COMPLEMENT);
cpld_bf_ro(phy_irq_status, DELL_N32XX_SYS_PHY_7_1_IRQ_STATUS_REG,
	   DELL_N32XX_SYS_PHY_7_1_IRQ_STATUS_PHY_IRQ_STATUS, NULL,
	   BF_COMPLEMENT);
cpld_bf_ro(phy_irq_mask, DELL_N32XX_SYS_PHY_7_1_IRQ_MSK_REG,
	   DELL_N32XX_SYS_PHY_7_1_IRQ_MSK_PHY_IRQ_MSK, NULL, BF_COMPLEMENT);

struct attribute *cpld_attrs[] = {
	&cpld_minor_revision.attr,
	&cpld_major_revision.attr,
	&cpld_scratch.attr,
	&cpld_board_revision.attr,
	&cpld_board_type.attr,
	&cpld_reset_port_led.attr,
	&cpld_poe_reset_disable.attr,
	&cpld_reset_npu.attr,
	&cpld_reset_mgmt_phy.attr,
	&cpld_cpld_spi_wp.attr,
	&cpld_sys_eeprom_wp.attr,
	&cpld_fan3_eeprom_wp.attr,
	&cpld_fan2_eeprom_wp.attr,
	&cpld_fan1_eeprom_wp.attr,
	&cpld_tpm_int.attr,
	&cpld_usb_fault.attr,
	&cpld_bcm54216s_int.attr,
	&cpld_ucd_irq.attr,
	&cpld_fan_alert_int.attr,
	&cpld_led_fan.attr,
	&cpld_led_system.attr,
	&cpld_led_beacon.attr,
	&cpld_led_power.attr,
	&cpld_led_stack.attr,
	&cpld_digit_blink.attr,
	&cpld_digit_on.attr,
	&cpld_digit_dot.attr,
	&cpld_digit.attr,
	&cpld_led_fan_tray3.attr,
	&cpld_led_fan_tray2.attr,
	&cpld_led_fan_tray1.attr,
	&cpld_fan_tray3_type.attr,
	&cpld_fan_tray2_type.attr,
	&cpld_fan_tray1_type.attr,
	&cpld_fan_tray3_present.attr,
	&cpld_fan_tray2_present.attr,
	&cpld_fan_tray1_present.attr,
	&cpld_sys_irq_enable.attr,
	&cpld_fan_enable.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr1_enable.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_alarm.attr,
	&cpld_psu_pwr2_enable.attr,
	&cpld_reset_poe.attr,
	&cpld_poe_enable.attr,
	&cpld_poe_i2c_ready.attr,
	&cpld_poe_int.attr,
	&cpld_poe_temp_alarm.attr,
	&cpld_poe_system_ok.attr,
	&cpld_reboot_cause.attr,
	&cpld_port_i2c_mux.attr,
	&cpld_fan_i2c_mux.attr,
	&cpld_psu_i2c_mux.attr,
	&cpld_usba_power_enable.attr,
	&cpld_power_down.attr,
	&cpld_power_cycle.attr,
	&cpld_wd_timer.attr,
	&cpld_wd_enable.attr,
	&cpld_wd_punch.attr,
	&cpld_psu_pwr2_ac_ok.attr,
	&cpld_psu_pwr1_ac_ok.attr,
	&cpld_qsfp28_54_53_present.attr,
	&cpld_qsfp28_54_53_reset.attr,
	&cpld_qsfp28_54_53_lpmode.attr,
	&cpld_qsfp28_54_53_irq_status.attr,
	&cpld_qsfp28_54_53_irq_mask.attr,
	&cpld_sfp28_52_49_present.attr,
	&cpld_sfp28_52_49_tx_disable.attr,
	&cpld_sfp28_52_49_rx_los.attr,
	&cpld_sfp28_52_49_tx_fault.attr,
	&cpld_phy_reset.attr,
	&cpld_phy_irq_status.attr,
	&cpld_phy_irq_mask.attr,
	NULL
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret;
	int major_rev;
	int minor_rev;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "smbus read byte data not supported.\n");
		return -ENODEV;
	}

	/* probe the hardware by reading the revision numbers */
	ret = i2c_smbus_read_byte_data(client,
				       DELL_N32XX_SYS_CPLD_REV_REG1_REG);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLD major revision register error %d\n", ret);
		goto err;
	}
	major_rev = ret;
	ret = i2c_smbus_read_byte_data(client,
				       DELL_N32XX_SYS_CPLD_REV_REG0_REG);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLD minor revision register error %d\n", ret);
		goto err;
	}
	minor_rev = ret;

	/* create sysfs nodes */
	ret = sysfs_create_group(&client->dev.kobj, &cpld_attr_group);
	if (ret) {
		dev_err(&client->dev, " failed to create sysfs nodes\n");
		goto err;
	}

	/* all clear from this point on */
	dev_info(&client->dev,
		 "device created, System CPLD major rev %d, minor rev %d\n",
		 major_rev & 0xff,
		 minor_rev & 0xff);
	return 0;

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
		.name  = DRIVER_NAME,
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
MODULE_DESCRIPTION("Dell EMC N32xx system CPLD support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

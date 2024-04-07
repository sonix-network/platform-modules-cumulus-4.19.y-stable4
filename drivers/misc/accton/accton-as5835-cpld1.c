// SPDX-License-Identifier: GPL-2.0+
/*
 * Accton AS5835 CPLD1 Driver
 *
 * Copyright (c) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/hwmon.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as5835.h"

#define DRIVER_NAME    AS5835_CPLD1_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

static const char * const led_locator_values[] = {
	PLATFORM_LED_RED,
	PLATFORM_LED_OFF,
	PLATFORM_LED_RED_BLINKING,
	PLATFORM_LED_RED_BLINKING,
};

static const char * const led_diag_values[] = {
	PLATFORM_LED_AMBER,
	PLATFORM_LED_RED,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static const char * const led_fan_values[] = {
	"autodetect",
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static const char * const led_psu_values[] = {
	"autodetect",
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

/* CPLD registers */

cpld_bf_ro(board_id, ACCTON_AS5835_CPLD1_BOARD_ID_REG,
	   ACCTON_AS5835_CPLD1_BOARD_ID, NULL, 0);
cpld_bf_ro(cpld_version, ACCTON_AS5835_CPLD1_VERSION_REG,
	   ACCTON_AS5835_CPLD1_VERSION, NULL, 0);
cpld_bt_ro(psu_pwr2_ac_ok, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU2_AC_ALERT, NULL, 0);
cpld_bt_ro(psu_pwr2_dc_ok, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU2_12V_PG, NULL, 0);
cpld_bt_ro(psu_pwr2_present, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU2_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_ac_ok, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU1_AC_ALERT, NULL, 0);
cpld_bt_ro(psu_pwr1_dc_ok, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU1_12V_PG, NULL, 0);
cpld_bt_ro(psu_pwr1_present, ACCTON_AS5835_CPLD1_PSU_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU1_PRESENT, NULL, BF_COMPLEMENT);
cpld_bf_ro(svid_status, ACCTON_AS5835_CPLD1_VID_STATUS_REG,
	   ACCTON_AS5835_CPLD1_SVID_STATUS, NULL, 0);
cpld_bt_rw(reset_88e1512_phy, ACCTON_AS5835_CPLD1_RESET_CONTROL_REG,
	   ACCTON_AS5835_CPLD1_88E1512_PHY_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pcie, ACCTON_AS5835_CPLD1_RESET_CONTROL_REG,
	   ACCTON_AS5835_CPLD1_RESET_PCIE, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_mac, ACCTON_AS5835_CPLD1_RESET_CONTROL_REG,
	   ACCTON_AS5835_CPLD1_RESET_MAC, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i210_phy, ACCTON_AS5835_CPLD1_RESET_CONTROL_REG,
	   ACCTON_AS5835_CPLD1_I210_PHY_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548, ACCTON_AS5835_CPLD1_RESET_CONTROL_REG,
	   ACCTON_AS5835_CPLD1_I2C_SW_RESET_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy12, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_12_9_REG,
	   ACCTON_AS5835_CPLD1_PHY12_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy11, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_12_9_REG,
	   ACCTON_AS5835_CPLD1_PHY11_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy10, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_12_9_REG,
	   ACCTON_AS5835_CPLD1_PHY10_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy9, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_12_9_REG,
	   ACCTON_AS5835_CPLD1_PHY9_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy8, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY8_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy7, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY7_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy6, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY6_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy5, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY5_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy4, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY4_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy3, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY3_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy2, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY2_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_phy1, ACCTON_AS5835_CPLD1_RESET_CONTROL_FOR_XGPHY_8_1_REG,
	   ACCTON_AS5835_CPLD1_PHY1_RESET, NULL, BF_COMPLEMENT);
cpld_bt_ro(lm75_int_cpu, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_LM75_INT_CPU, NULL, BF_COMPLEMENT);
cpld_bt_ro(mac_int, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_MAC_INT, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpld23_int, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_CPLD23_INT_L, NULL, BF_COMPLEMENT);
cpld_bt_ro(88e1512_int, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_88E1512_INT, NULL, BF_COMPLEMENT);
cpld_bt_ro(lm75_int_ch3, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_LM75_INT_CH3, NULL, BF_COMPLEMENT);
cpld_bt_ro(lm75_int_ch2, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_LM75_INT_CH2, NULL, BF_COMPLEMENT);
cpld_bt_ro(lm75_int_ch1, ACCTON_AS5835_CPLD1_INTERRUPT_STATUS_REG,
	   ACCTON_AS5835_CPLD1_LM75_INT_CH1, NULL, BF_COMPLEMENT);
cpld_bf_rw(led_locator, ACCTON_AS5835_CPLD1_SYSTEM_LED_STATUS_REG,
	   ACCTON_AS5835_CPLD1_LOC_B, led_locator_values, 0);
cpld_bf_rw(led_diag, ACCTON_AS5835_CPLD1_SYSTEM_LED_STATUS_REG,
	   ACCTON_AS5835_CPLD1_DIAG, led_diag_values, 0);
cpld_bf_rw(led_fan, ACCTON_AS5835_CPLD1_SYSTEM_LED_STATUS_REG,
	   ACCTON_AS5835_CPLD1_FAN, led_fan_values, 0);
cpld_bf_rw(led_psu2, ACCTON_AS5835_CPLD1_PSU_LED_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU2, led_psu_values, 0);
cpld_bf_rw(led_psu1, ACCTON_AS5835_CPLD1_PSU_LED_STATUS_REG,
	   ACCTON_AS5835_CPLD1_PSU1, led_psu_values, 0);
cpld_bt_ro(mdc_mdio_sel1, ACCTON_AS5835_CPLD1_OTHER_1_REG,
	   ACCTON_AS5835_MDC_MDIO_SEL1, NULL, BF_COMPLEMENT);
cpld_bt_ro(mdc_mdio_sel0, ACCTON_AS5835_CPLD1_OTHER_1_REG,
	   ACCTON_AS5835_MDC_MDIO_SEL0, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpld_select, ACCTON_AS5835_CPLD1_OTHER_1_REG,
	   ACCTON_AS5835_CPLD_SELECT, NULL, BF_COMPLEMENT);
cpld_bt_ro(pcie_wake, ACCTON_AS5835_CPLD1_OTHER_2_REG,
	   ACCTON_AS5835_PCIE_WAKE_L, NULL, BF_COMPLEMENT);
cpld_bt_ro(pcie_intr, ACCTON_AS5835_CPLD1_OTHER_2_REG,
	   ACCTON_AS5835_PCIE_INTR_L, NULL, BF_COMPLEMENT);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_board_id.attr,
	&cpld_cpld_version.attr,
	&cpld_psu_pwr2_ac_ok.attr,
	&cpld_psu_pwr2_dc_ok.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr1_ac_ok.attr,
	&cpld_psu_pwr1_dc_ok.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_svid_status.attr,
	&cpld_reset_88e1512_phy.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_mac.attr,
	&cpld_reset_i210_phy.attr,
	&cpld_reset_pca9548.attr,
	&cpld_reset_phy12.attr,
	&cpld_reset_phy11.attr,
	&cpld_reset_phy10.attr,
	&cpld_reset_phy9.attr,
	&cpld_reset_phy8.attr,
	&cpld_reset_phy7.attr,
	&cpld_reset_phy6.attr,
	&cpld_reset_phy5.attr,
	&cpld_reset_phy4.attr,
	&cpld_reset_phy3.attr,
	&cpld_reset_phy2.attr,
	&cpld_reset_phy1.attr,
	&cpld_lm75_int_cpu.attr,
	&cpld_mac_int.attr,
	&cpld_cpld23_int.attr,
	&cpld_88e1512_int.attr,
	&cpld_lm75_int_ch3.attr,
	&cpld_lm75_int_ch2.attr,
	&cpld_lm75_int_ch1.attr,
	&cpld_led_locator.attr,
	&cpld_led_diag.attr,
	&cpld_led_fan.attr,
	&cpld_led_psu2.attr,
	&cpld_led_psu1.attr,
	&cpld_mdc_mdio_sel1.attr,
	&cpld_mdc_mdio_sel0.attr,
	&cpld_cpld_select.attr,
	&cpld_pcie_wake.attr,
	&cpld_pcie_intr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	s32 temp;
	int ret = 0;

	/* make sure the adpater supports i2c smbus reads */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/* probe the hardware by reading the version register */
	temp = i2c_smbus_read_byte_data(client,
					ACCTON_AS5835_CPLD1_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPLD1 version register error: %d\n",
			temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "failed to create sysfs group for cpld device\n");
		goto err;
	}

	/* all clear */
	dev_info(dev, "device probed, CPLD1 rev: %lu\n",
		 GET_FIELD(temp, ACCTON_AS5835_CPLD1_VERSION));

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
	{ },
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

/* module init/exit */

static int __init cpld_init(void)
{
	int ret;

	ret = i2c_add_driver(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": driver failed to load\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS5835 CPLD1 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

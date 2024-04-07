// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AGV848v1 SWPLD1 Driver
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-agv848v1.h"

#define DRIVER_NAME    AGV848V1_SWPLD1_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const led_values[] = {
	PLATFORM_LED_OFF,            /* 0 */
	PLATFORM_LED_GREEN,          /* 1 */
	PLATFORM_LED_AMBER,          /* 2 */
	PLATFORM_LED_OFF,            /* 3 */
	PLATFORM_LED_OFF,            /* 4 */
	PLATFORM_LED_GREEN_BLINKING, /* 5 */
	PLATFORM_LED_AMBER_BLINKING, /* 6 */
	PLATFORM_LED_OFF,            /* 7 */
};

static const char * const fan_led_values[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
	PLATFORM_LED_AMBER, /* 2 */
	PLATFORM_LED_OFF,   /* 3 */
};

/* CPLD registers */

cpld_bf_ro(board_id, DELTA_AGV848V1_SYSTEM_PRODUCT_ID_REG,
	   DELTA_AGV848V1_BOARD_ID, NULL, 0);
cpld_bf_ro(device_version, DELTA_AGV848V1_SYSTEM_DEVICE_VERSION_REG,
	   DELTA_AGV848V1_DEV_VER, NULL, 0);
cpld_bt_rw(reset_pcie, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_PCIE_PERST_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpld_lpc, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_CPLD_LPCRST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bmc, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_BMC_RSTN, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_mac, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_RST_MAC_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_uconsole, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_RST_UCONSOLE, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i210_phy, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_OOB_PHY_RSTN_I210_AT, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_usb_hub, DELTA_AGV848V1_HW_RESET_CONTROL_REG,
	   DELTA_AGV848V1_OOB_RST_USB_HUB, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpld2_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_CPLD_SWPLD2_INT, NULL, 0);
cpld_bt_ro(cpld3_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_CPLD_SWPLD3_INT, NULL, 0);
cpld_bt_ro(pcie_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_PCIE_INT, NULL, 0);
cpld_bt_ro(psu_pwr2_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_PS2_PWR_INT, NULL, 0);
cpld_bt_ro(psu_pwr1_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_PS1_PWR_INT, NULL, 0);
cpld_bt_ro(fanb_thermal_int, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_FANB_THERMAL_INT, NULL, 0);
cpld_bt_ro(fanb_alert, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_FANB_ALERT, NULL, 0);
cpld_bt_ro(smb_alert, DELTA_AGV848V1_HW_INTERRUPT_CHANGE_REG,
	   DELTA_AGV848V1_SMB_ALERT, NULL, 0);
cpld_bt_rw(cpld2_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_CPLD2_INT_MASK, NULL, 0);
cpld_bt_rw(cpld3_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_CPLD3_INT_MASK, NULL, 0);
cpld_bt_rw(pcie_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_PCIE_INTR_MASK, NULL, 0);
cpld_bt_rw(psu_pwr2_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_PS2_PWR_INT_MASK, NULL, 0);
cpld_bt_rw(psu_pwr1_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_PS1_PWR_INT_MASK, NULL, 0);
cpld_bt_rw(fanb_thermal_int_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_FANB_THERMAL_INTN_MASK, NULL, 0);
cpld_bt_rw(fanb_alert_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_FANB_ALERTN_MASK, NULL, 0);
cpld_bt_rw(smb_alert_mask, DELTA_AGV848V1_HW_INTERRUPT_MASK_REG,
	   DELTA_AGV848V1_SMB_ALERT_MASK, NULL, 0);
cpld_bt_rw(eeprom_wp, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_EEPROM_WP, NULL, 0);
cpld_bt_rw(oob_dev_off, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_OOB_DEV_OFF_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(pcie_wake, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_PCIE_WAKE_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(oob_pcie_wake, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_OOB_PCIE_WAKE_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(oob_rs232_invalid, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_OOB_RS232_INVALID_L, NULL, BF_COMPLEMENT);
cpld_bt_ro(usb_suspend, DELTA_AGV848V1_HW_MISC_CONTROL_REG,
	   DELTA_AGV848V1_USB_SUSPEND, NULL, 0);

/* Note: these bits control the LED labeled "PSU1" */
cpld_bf_rw(led_psu1, DELTA_AGV848V1_SYS_LED_CONTROL_1_REG,
	   DELTA_AGV848V1_STATUS_LED, led_values, 0);
cpld_bf_rw(led_fan, DELTA_AGV848V1_SYS_LED_CONTROL_1_REG,
	   DELTA_AGV848V1_FAN_LED, led_values, 0);

/* Note: these bits control the LED labeled "SYS" */
cpld_bf_rw(led_status, DELTA_AGV848V1_SYS_LED_CONTROL_2_REG,
	   DELTA_AGV848V1_PWR_LED, led_values, 0);

/* Note: these bits control the LED labeled "PSU2" */
cpld_bf_rw(led_psu2, DELTA_AGV848V1_SYS_LED_CONTROL_2_REG,
	   DELTA_AGV848V1_RPS_LED, led_values, 0);
cpld_bf_rw(led_fan1, DELTA_AGV848V1_FAN_TRAY_LED_CONTROL_REG,
	   DELTA_AGV848V1_FAN1_LED, fan_led_values, 0);
cpld_bf_rw(led_fan2, DELTA_AGV848V1_FAN_TRAY_LED_CONTROL_REG,
	   DELTA_AGV848V1_FAN2_LED, fan_led_values, 0);
cpld_bf_rw(led_fan3, DELTA_AGV848V1_FAN_TRAY_LED_CONTROL_REG,
	   DELTA_AGV848V1_FAN3_LED, fan_led_values, 0);
cpld_bf_rw(led_fan4, DELTA_AGV848V1_FAN_TRAY_LED_CONTROL_REG,
	   DELTA_AGV848V1_FAN4_LED, fan_led_values, 0);
cpld_bt_rw(pca_reset_fan, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_1_REG,
	   DELTA_AGV848V1_PCA_RST_FAN_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c1, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_1_REG,
	   DELTA_AGV848V1_PCA_RST_I2C1, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_6, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_6, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_5, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_5, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_4, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_4, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_3, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_3, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_2, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_2, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_1, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_1, NULL, BF_COMPLEMENT);
cpld_bt_rw(pca_reset_i2c2_0, DELTA_AGV848V1_I2C_MUX_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_PCA_RST_I2C2_0, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr2_en, DELTA_AGV848V1_POWER_SUPPLY_ENABLE_REG,
	   DELTA_AGV848V1_PS2_PSON_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr1_en, DELTA_AGV848V1_POWER_SUPPLY_ENABLE_REG,
	   DELTA_AGV848V1_PS1_PSON_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_present, DELTA_AGV848V1_POWER_SUPPLY_STATUS_REG,
	   DELTA_AGV848V1_PS1_PRESENT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_all_ok, DELTA_AGV848V1_POWER_SUPPLY_STATUS_REG,
	   DELTA_AGV848V1_PS1_PWOK, NULL, 0);
cpld_bt_ro(psu_pwr2_present, DELTA_AGV848V1_POWER_SUPPLY_STATUS_REG,
	   DELTA_AGV848V1_PS2_PRESENT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_all_ok, DELTA_AGV848V1_POWER_SUPPLY_STATUS_REG,
	   DELTA_AGV848V1_PS2_PWOK, NULL, 0);
cpld_bt_rw(oob_usb2_poe, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_OOB_USB2_POE_N, NULL, 0);
cpld_bt_rw(en_vcc_3v3, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_EN_VCC_3V3, NULL, 0);
cpld_bt_rw(en_vcc_mac_1v8, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_EN_VCC_MAC_1V8, NULL, 0);
cpld_bt_rw(en_vcc_mac_1v2, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_EN_VCC_MAC_1V2, NULL, 0);
cpld_bt_rw(en_vcc_avs_0v81, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_EN_VCC_AVS_0V81, NULL, 0);
cpld_bt_rw(en_vcc_mac_0v8, DELTA_AGV848V1_POWER_RAIL_ENABLE_REG,
	   DELTA_AGV848V1_EN_VCC_MAC_0V8, NULL, 0);
cpld_bt_ro(oob_usb_ovc, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_OOB_USB2_OVC, NULL, 0);
cpld_bt_ro(pg_vcc_stb_5v0, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_VCC_STB_5V0, NULL, 0);
cpld_bt_ro(pg_vcc_stb_3v3, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_VCC_STB_3V3, NULL, 0);
cpld_bt_ro(pg_vv_3v3, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_VCC_3V3, NULL, 0);
cpld_bt_ro(pg_bmc_1v15, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_BMC_1V15, NULL, 0);
cpld_bt_ro(pg_bmc_1v2, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_BMC_1V2, NULL, 0);
cpld_bt_ro(pg_bmc_2v5, DELTA_AGV848V1_POWER_RAIL_STATUS_1_REG,
	   DELTA_AGV848V1_PG_BMC_2V5, NULL, 0);
cpld_bt_ro(pg_vcc_avs_0v81, DELTA_AGV848V1_POWER_RAIL_STATUS_2_REG,
	   DELTA_AGV848V1_PG_VCC_AVS_0V81, NULL, 0);
cpld_bt_ro(pg_vcc_mac_0v8, DELTA_AGV848V1_POWER_RAIL_STATUS_2_REG,
	   DELTA_AGV848V1_PG_VCC_MAC_0V8, NULL, 0);
cpld_bt_ro(pg_vcc_mac_1v2, DELTA_AGV848V1_POWER_RAIL_STATUS_2_REG,
	   DELTA_AGV848V1_PG_VCC_MAC_1V2, NULL, 0);
cpld_bt_ro(pg_vcc_mac_1v8, DELTA_AGV848V1_POWER_RAIL_STATUS_2_REG,
	   DELTA_AGV848V1_PG_VCC_MAC_1V8, NULL, 0);
cpld_bt_rw(sys_pwr_1v_mac_avs, DELTA_AGV848V1_POWER_RAIL_MARGINS_CONTROL_REG,
	   DELTA_AGV848V1_SYS_PWR_1V_MAC_AVS, NULL, 0);
cpld_bf_rw(sys_pwr_1v_margin, DELTA_AGV848V1_POWER_RAIL_MARGINS_CONTROL_REG,
	   DELTA_AGV848V1_SYS_PWR_1V_MARGIN, NULL, 0);
cpld_bf_rw(sys_pwr_margin, DELTA_AGV848V1_POWER_RAIL_MARGINS_CONTROL_REG,
	   DELTA_AGV848V1_SYS_PWR_MARGIN, NULL, 0);
cpld_bt_rw(reset_swpld2, DELTA_AGV848V1_HW_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_RSTN_SWPLD2, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_swpld3, DELTA_AGV848V1_HW_RESET_CONTROL_2_REG,
	   DELTA_AGV848V1_RSTN_SWPLD3, NULL, BF_COMPLEMENT);
cpld_bf_rw(sfp28_8_1_tx_disable, DELTA_AGV848V1_SFP28_TX_DISABLE_1_REG,
	   DELTA_AGV848V1_SFP28_TX_DISABLE_8_1, NULL, 0);
cpld_bf_rw(sfp28_16_9_tx_disable, DELTA_AGV848V1_SFP28_TX_DISABLE_2_REG,
	   DELTA_AGV848V1_SFP28_TX_DISABLE_16_9, NULL, 0);
cpld_bf_rw(vid, DELTA_AGV848V1_BCM_AVS_VID_CONTROL_REG,
	   DELTA_AGV848V1_VID, NULL, 0);
cpld_bf_ro(cpld_mac_avs, DELTA_AGV848V1_CPLD_MAC_AVS_REG,
	   DELTA_AGV848V1_CPLD_MAC_AVS, NULL, 0);
cpld_bf_ro(hw_version, DELTA_AGV848V1_SYSTEM_HW_VERSION_REG,
	   DELTA_AGV848V1_HW_VER, NULL, 0);
cpld_bf_ro(cpld_version, DELTA_AGV848V1_SYSTEM_CPLD_VERSION_REG,
	   DELTA_AGV848V1_SWPLD1_VER, NULL, 0);

static struct attribute *cpld_attrs[] = {
	&cpld_board_id.attr,
	&cpld_device_version.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_cpld_lpc.attr,
	&cpld_reset_bmc.attr,
	&cpld_reset_mac.attr,
	&cpld_reset_uconsole.attr,
	&cpld_reset_i210_phy.attr,
	&cpld_reset_usb_hub.attr,
	&cpld_cpld2_int.attr,
	&cpld_cpld3_int.attr,
	&cpld_pcie_int.attr,
	&cpld_psu_pwr2_int.attr,
	&cpld_psu_pwr1_int.attr,
	&cpld_fanb_thermal_int.attr,
	&cpld_fanb_alert.attr,
	&cpld_smb_alert.attr,
	&cpld_cpld2_int_mask.attr,
	&cpld_cpld3_int_mask.attr,
	&cpld_pcie_int_mask.attr,
	&cpld_psu_pwr2_int_mask.attr,
	&cpld_psu_pwr1_int_mask.attr,
	&cpld_fanb_thermal_int_mask.attr,
	&cpld_fanb_alert_mask.attr,
	&cpld_smb_alert_mask.attr,
	&cpld_eeprom_wp.attr,
	&cpld_oob_dev_off.attr,
	&cpld_pcie_wake.attr,
	&cpld_oob_pcie_wake.attr,
	&cpld_oob_rs232_invalid.attr,
	&cpld_usb_suspend.attr,
	&cpld_led_status.attr,
	&cpld_led_fan.attr,
	&cpld_led_psu1.attr,
	&cpld_led_psu2.attr,
	&cpld_led_fan1.attr,
	&cpld_led_fan2.attr,
	&cpld_led_fan3.attr,
	&cpld_led_fan4.attr,
	&cpld_pca_reset_fan.attr,
	&cpld_pca_reset_i2c1.attr,
	&cpld_pca_reset_i2c2_6.attr,
	&cpld_pca_reset_i2c2_5.attr,
	&cpld_pca_reset_i2c2_4.attr,
	&cpld_pca_reset_i2c2_3.attr,
	&cpld_pca_reset_i2c2_2.attr,
	&cpld_pca_reset_i2c2_1.attr,
	&cpld_pca_reset_i2c2_0.attr,
	&cpld_psu_pwr2_en.attr,
	&cpld_psu_pwr1_en.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_oob_usb2_poe.attr,
	&cpld_en_vcc_3v3.attr,
	&cpld_en_vcc_mac_1v8.attr,
	&cpld_en_vcc_mac_1v2.attr,
	&cpld_en_vcc_avs_0v81.attr,
	&cpld_en_vcc_mac_0v8.attr,
	&cpld_oob_usb_ovc.attr,
	&cpld_pg_vcc_stb_5v0.attr,
	&cpld_pg_vcc_stb_3v3.attr,
	&cpld_pg_vv_3v3.attr,
	&cpld_pg_bmc_1v15.attr,
	&cpld_pg_bmc_1v2.attr,
	&cpld_pg_bmc_2v5.attr,
	&cpld_pg_vcc_avs_0v81.attr,
	&cpld_pg_vcc_mac_0v8.attr,
	&cpld_pg_vcc_mac_1v2.attr,
	&cpld_pg_vcc_mac_1v8.attr,
	&cpld_sys_pwr_1v_mac_avs.attr,
	&cpld_sys_pwr_1v_margin.attr,
	&cpld_sys_pwr_margin.attr,
	&cpld_reset_swpld2.attr,
	&cpld_reset_swpld3.attr,
	&cpld_sfp28_8_1_tx_disable.attr,
	&cpld_sfp28_16_9_tx_disable.attr,
	&cpld_vid.attr,
	&cpld_cpld_mac_avs.attr,
	&cpld_hw_version.attr,
	&cpld_cpld_version.attr,

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
					DELTA_AGV848V1_SYSTEM_CPLD_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read SWPLD1 version register error: %d\n",
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
	dev_info(dev, "device probed, SWPLD1 rev: %lu\n",
		 GET_FIELD(temp, DELTA_AGV848V1_SWPLD1_VER));

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
MODULE_DESCRIPTION("Delta AGV848v1 SWPLD1 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


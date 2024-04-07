// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AGV848v1 SWPLD2 Driver
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

#define DRIVER_NAME    AGV848V1_SWPLD2_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_rw(sfp28_24_17_tx_disable, DELTA_AGV848V1_SFP28_TX_DISABLE_3_REG,
	   DELTA_AGV848V1_SFP28_TX_DISABLE_24_17, NULL, 0);
cpld_bf_rw(sfp28_32_25_tx_disable, DELTA_AGV848V1_SFP28_TX_DISABLE_4_REG,
	   DELTA_AGV848V1_SFP28_TX_DISABLE_32_25, NULL, 0);
cpld_bf_rw(sfp28_40_33_tx_disable, DELTA_AGV848V1_SFP28_TX_DISABLE_5_REG,
	   DELTA_AGV848V1_SFP28_TX_DISABLE_40_33, NULL, 0);
cpld_bf_ro(sfp28_8_1_present, DELTA_AGV848V1_SFP28_ABSENT_STATUS_1_REG,
	   DELTA_AGV848V1_SFP28_MOD_ABS_8_1, NULL, BF_COMPLEMENT);
cpld_bf_ro(sfp28_16_9_present, DELTA_AGV848V1_SFP28_ABSENT_STATUS_2_REG,
	   DELTA_AGV848V1_SFP28_MOD_ABS_16_9, NULL, BF_COMPLEMENT);
cpld_bf_ro(sfp28_24_17_present, DELTA_AGV848V1_SFP28_ABSENT_STATUS_3_REG,
	   DELTA_AGV848V1_SFP28_MOD_ABS_24_17, NULL, BF_COMPLEMENT);
cpld_bf_ro(sfp28_32_25_present, DELTA_AGV848V1_SFP28_ABSENT_STATUS_4_REG,
	   DELTA_AGV848V1_SFP28_MOD_ABS_32_25, NULL, BF_COMPLEMENT);
cpld_bf_ro(sfp28_8_1_tx_fault, DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_1_REG,
	   DELTA_AGV848V1_SFP28_TX_FAULT_8_1, NULL, 0);
cpld_bf_ro(sfp28_16_9_tx_fault, DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_2_REG,
	   DELTA_AGV848V1_SFP28_TX_FAULT_16_9, NULL, 0);
cpld_bf_ro(sfp28_24_17_tx_fault, DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_3_REG,
	   DELTA_AGV848V1_SFP28_TX_FAULT_24_17, NULL, 0);
cpld_bf_ro(sfp28_32_25_tx_fault, DELTA_AGV848V1_SFP28_TX_FAULT_STATUS_4_REG,
	   DELTA_AGV848V1_SFP28_TX_FAULT_32_25, NULL, 0);
cpld_bf_ro(sfp28_8_1_rx_los, DELTA_AGV848V1_SFP28_RX_LOS_STATUS_1_REG,
	   DELTA_AGV848V1_SFP28_RX_LOS_8_1, NULL, 0);
cpld_bf_ro(sfp28_16_9_rx_los, DELTA_AGV848V1_SFP28_RX_LOS_STATUS_2_REG,
	   DELTA_AGV848V1_SFP28_RX_LOS_16_9, NULL, 0);
cpld_bf_ro(sfp28_24_17_rx_los, DELTA_AGV848V1_SFP28_RX_LOS_STATUS_3_REG,
	   DELTA_AGV848V1_SFP28_RX_LOS_24_17, NULL, 0);
cpld_bf_ro(sfp28_32_25_rx_los, DELTA_AGV848V1_SFP28_RX_LOS_STATUS_4_REG,
	   DELTA_AGV848V1_SFP28_RX_LOS_32_25, NULL, 0);
cpld_bf_ro(cpld_version, DELTA_AGV848V1_SWPLD2_VERSION_REG,
	   DELTA_AGV848V1_SWPLD2_VER, NULL, 0);
cpld_bf_rw(sfp28_ledctrl, DELTA_AGV848V1_SFP28_LED_TEST_CONTROL_REG,
	   DELTA_AGV848V1_SFP28_LEDCTRL, NULL, 0);

static struct attribute *cpld_attrs[] = {
	&cpld_sfp28_24_17_tx_disable.attr,
	&cpld_sfp28_32_25_tx_disable.attr,
	&cpld_sfp28_40_33_tx_disable.attr,
	&cpld_sfp28_8_1_present.attr,
	&cpld_sfp28_16_9_present.attr,
	&cpld_sfp28_24_17_present.attr,
	&cpld_sfp28_32_25_present.attr,
	&cpld_sfp28_8_1_tx_fault.attr,
	&cpld_sfp28_16_9_tx_fault.attr,
	&cpld_sfp28_24_17_tx_fault.attr,
	&cpld_sfp28_32_25_tx_fault.attr,
	&cpld_sfp28_8_1_rx_los.attr,
	&cpld_sfp28_16_9_rx_los.attr,
	&cpld_sfp28_24_17_rx_los.attr,
	&cpld_sfp28_32_25_rx_los.attr,
	&cpld_cpld_version.attr,
	&cpld_sfp28_ledctrl.attr,

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
					DELTA_AGV848V1_SWPLD2_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read SWPLD2 version register error: %d\n",
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
	dev_info(dev, "device probed, SWPLD2 rev: %lu\n",
		 GET_FIELD(temp, DELTA_AGV848V1_SWPLD2_VER));

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
MODULE_DESCRIPTION("Delta AGV848v1 SWPLD2 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


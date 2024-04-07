// SPDX-License-Identifier: GPL-2.0+
/*
 * Accton AS5835 CPLD2 Driver
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
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as5835.h"

#define DRIVER_NAME    AS5835_CPLD2_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_ro(cpld_version, ACCTON_AS5835_CPLD2_VERSION_REG,
	   ACCTON_AS5835_CPLD2_VERSION, NULL, 0);

/* special case for sfp registers */

struct sfp_info {
	int reg;
	int active_low;
};

static struct sfp_info sfp_reg[] = {
	{
		.reg = ACCTON_AS5835_CPLD2_SFP_PRESENT_REG,
		.active_low = 1,
	},
	{
		.reg = ACCTON_AS5835_CPLD2_SFP_TX_FAULT_REG,
		.active_low = 0,
	},
	{
		.reg = ACCTON_AS5835_CPLD2_SFP_TX_DISABLE_REG,
		.active_low = 0,
	},
	{
		.reg = ACCTON_AS5835_CPLD2_SFP_RX_LOS_REG,
		.active_low = 0,
	},
};

static ssize_t sfp_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = sfp_reg[idx].reg;
	int i;
	s32 tmp;
	u64 val = 0;

	for (i = 0; i < NUM_CPLD2_SFP_REGS; i++) {
		tmp = i2c_smbus_read_byte_data(client, (reg + i));
		if (tmp < 0) {
			pr_err(DRIVER_NAME ": CPLD2 read error - reg: 0x%02X\n",
			       reg);
			return -EINVAL;
		}
		if (sfp_reg[idx].active_low)
			tmp = ~tmp;
		val |= ((u64)tmp & 0xff) << i * 8;
	}
	return sprintf(buf, "0x%012llx\n", val & 0x3fffffffff);
}

static ssize_t sfp_store(struct device *dev, struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = sfp_reg[idx].reg;
	int ret;
	u64 val = 0;
	int i;
	u8 tmp;

	ret = kstrtou64(buf, 0, &val);
	if (ret < 0)
		return ret;

	for (i = 0; i < NUM_CPLD2_SFP_REGS; i++) {
		tmp = val >> (i * 8) & 0xff;
		if (sfp_reg[idx].active_low)
			tmp = ~tmp;
		ret = i2c_smbus_write_byte_data(client, (reg + i), tmp);
		if (ret) {
			pr_err(DRIVER_NAME ": CPLD2 write error - reg: 0x%02X\n",
			       reg);
			return -EINVAL;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp_38_1_present,    sfp_show, 0);
static SENSOR_DEVICE_ATTR_RO(sfp_38_1_tx_fault,   sfp_show, 1);
static SENSOR_DEVICE_ATTR_RW(sfp_38_1_tx_disable, sfp_show, sfp_store, 2);
static SENSOR_DEVICE_ATTR_RO(sfp_38_1_rx_los,     sfp_show, 3);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpld_version.attr,

	&sensor_dev_attr_sfp_38_1_present.dev_attr.attr,
	&sensor_dev_attr_sfp_38_1_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_38_1_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_38_1_rx_los.dev_attr.attr,

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
					ACCTON_AS5835_CPLD2_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPLD2 version register error: %d\n",
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
	dev_info(dev, "device probed, CPLD2 rev: %lu\n",
		 GET_FIELD(temp, ACCTON_AS5835_CPLD2_VERSION));

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
MODULE_DESCRIPTION("Accton AS5835 CPLD2 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

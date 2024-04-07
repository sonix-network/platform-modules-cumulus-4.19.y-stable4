// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc_n32xx-port-cpld.c - Dell EMC N32xx port CPLD support.
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

#define DRIVER_NAME     PORT_CPLD_DRIVER_NAME
#define DRIVER_VERSION	"1.2"

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* Port CPLD registers */

cpld_rg_ro(major_revision, DELL_N32XX_PORT_CPLD_REV_REG1_REG, NULL, 0);
cpld_rg_ro(minor_revision, DELL_N32XX_PORT_CPLD_REV_REG0_REG, NULL, 0);
cpld_rg_rw(scratch, DELL_N32XX_PORT_CPLD_GPR_REG, NULL, 0);
mk_bf_ro(cpld, sfp_24_1_present, DELL_N32XX_PORT_8_1_PRESENT_STATUS_REG, 0, 24,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, sfp_24_1_tx_disable, DELL_N32XX_PORT_8_1_TX_DISABLE_REG, 0, 24,
	 NULL, 0);
mk_bf_ro(cpld, sfp_24_1_rx_los, DELL_N32XX_PORT_8_1_RX_LOS_STATUS_REG, 0, 24,
	 NULL, 0);
mk_bf_ro(cpld, sfp_24_1_tx_fault, DELL_N32XX_PORT_8_1_TX_FAULT_STATUS_REG, 0,
	 24, NULL, 0);

struct attribute *cpld_attrs[] = {
	&cpld_minor_revision.attr,
	&cpld_major_revision.attr,
	&cpld_scratch.attr,
	&cpld_sfp_24_1_present.attr,
	&cpld_sfp_24_1_tx_disable.attr,
	&cpld_sfp_24_1_rx_los.attr,
	&cpld_sfp_24_1_tx_fault.attr,
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
				       DELL_N32XX_PORT_CPLD_REV_REG1_REG);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLD major revision register error %d\n", ret);
		goto err;
	}
	major_rev = ret;
	ret = i2c_smbus_read_byte_data(client,
				       DELL_N32XX_PORT_CPLD_REV_REG0_REG);
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
		 "device created, port CPLD major rev %d, minor rev %d\n",
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
MODULE_DESCRIPTION("Dell EMC N32xx port CPLD support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

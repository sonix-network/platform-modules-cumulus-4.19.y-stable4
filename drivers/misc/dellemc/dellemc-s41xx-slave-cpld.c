// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc-s41xx-slave-cpld.c - Dell EMC S41xx Slave CPLD Support.
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

#define DRIVER_NAME	SLAVE_CPLD_DRIVER_NAME
#define DRIVER_VERSION	"1.1"

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

cpld_bf_ro(slave_cpld_major_version, DELL_S41XX_SLV_CPLD_REV_REG,
	   DELL_S41XX_SLV_CPLD_REV_MJR_REV, NULL, BF_DECIMAL);
cpld_bf_ro(slave_cpld_minor_version, DELL_S41XX_SLV_CPLD_REV_REG,
	   DELL_S41XX_SLV_CPLD_REV_MNR_REV, NULL, BF_DECIMAL);
mk_bf_rw(cpld, scratch, DELL_S41XX_SLV_CPLD_GPR_REG, 0, 8, NULL, 0);

mk_bf_ro(cpld, sfp_24_17_present,
	 DELL_S41XX_SLV_CPLD_PORT_24_17_PRESENT_STATUS_REG, 0, 8, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_54_31_present,
	 DELL_S41XX_SLV_CPLD_PORT_38_31_PRESENT_STATUS_REG, 0, 24, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_30_27_present,
	 DELL_S41XX_SLV_CPLD_PORT_34_27_PRESENT_STATUS_REG, 0, 4, NULL,
	 BF_COMPLEMENT);

mk_bf_rw(cpld, sfp_24_17_tx_disable,
	 DELL_S41XX_SLV_CPLD_PORT_24_17_TX_DISABLE_REG, 0, 8, NULL, 0);
mk_bf_rw(cpld, sfp_54_31_tx_disable,
	 DELL_S41XX_SLV_CPLD_PORT_38_31_TX_DISABLE_REG, 0, 24, NULL, 0);
mk_bf_rw(cpld, sfp_30_27_tx_disable,
	 DELL_S41XX_SLV_CPLD_PORT_34_27_TX_DISABLE_REG, 0, 4, NULL, 0);

mk_bf_ro(cpld, sfp_24_17_rx_los,
	 DELL_S41XX_SLV_CPLD_PORT_24_17_RX_LOS_STATUS_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, sfp_54_31_rx_los,
	 DELL_S41XX_SLV_CPLD_PORT_38_31_RX_LOS_STATUS_REG, 0, 24, NULL, 0);
mk_bf_ro(cpld, sfp_30_27_rx_los,
	 DELL_S41XX_SLV_CPLD_PORT_34_27_RX_LOS_STATUS_REG, 0, 4, NULL, 0);

mk_bf_ro(cpld, sfp_24_17_tx_fault,
	 DELL_S41XX_SLV_CPLD_PORT_24_17_TX_FAULT_STATUS_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, sfp_54_31_tx_fault,
	 DELL_S41XX_SLV_CPLD_PORT_38_31_TX_FAULT_STATUS_REG, 0, 24, NULL, 0);
mk_bf_ro(cpld, sfp_30_27_tx_fault,
	 DELL_S41XX_SLV_CPLD_PORT_34_27_TX_FAULT_STATUS_REG, 0, 4, NULL, 0);

/*
 * SYSFS attributes
 */
static struct attribute *cpld_attrs[] = {
	&cpld_slave_cpld_major_version.attr,
	&cpld_slave_cpld_minor_version.attr,
	&cpld_scratch.attr,
	&cpld_sfp_24_17_present.attr,
	&cpld_sfp_54_31_present.attr,
	&cpld_sfp_30_27_present.attr,
	&cpld_sfp_24_17_tx_disable.attr,
	&cpld_sfp_54_31_tx_disable.attr,
	&cpld_sfp_30_27_tx_disable.attr,
	&cpld_sfp_24_17_rx_los.attr,
	&cpld_sfp_54_31_rx_los.attr,
	&cpld_sfp_30_27_rx_los.attr,
	&cpld_sfp_24_17_tx_fault.attr,
	&cpld_sfp_54_31_tx_fault.attr,
	&cpld_sfp_30_27_tx_fault.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
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
	ret = i2c_smbus_read_byte_data(client, DELL_S41XX_SLV_CPLD_REV_REG);
	if (ret < 0) {
		dev_err(dev, "read cpld revision register error %d\n", ret);
		goto err;
	}
	cpldrev = ret;

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
		 "device created, CPLD rev %ld.%ld\n",
		 GET_FIELD(cpldrev, DELL_S41XX_SLV_CPLD_REV_MJR_REV),
		 GET_FIELD(cpldrev, DELL_S41XX_SLV_CPLD_REV_MNR_REV));

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
MODULE_DESCRIPTION("Dell EMC S41xx slave cpld support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

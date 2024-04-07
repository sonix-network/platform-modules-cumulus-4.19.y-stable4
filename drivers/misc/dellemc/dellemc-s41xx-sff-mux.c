// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc-s41xx-sff-mux.c - Dell EMC S41xx SFF MUX Support.
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
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>

#include "dellemc-s41xx-cplds.h"

#define DRIVER_NAME             MUX_DRIVER_NAME
#define DRIVER_VERSION          "1.1"
#define MUX_INVALID_CHANNEL     0xFF
#define MUX_NUM_PORTS           54
#define MUX_SELECT_REG          DELL_S41XX_MSTR_PORT_I2C_MUX_REG
#define MUX_FIRST_BUS_NUM       DELL_S41XX_MUX_BUS_START

struct port_mux_info {
	int    prev_chan;
	struct i2c_client *cpld_client;
	struct i2c_mux_core *muxc;
};

static int mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	struct port_mux_info *data = i2c_mux_priv(muxc);
	int ret;

	if (data->prev_chan == chan)
		return 0;
	ret = i2c_smbus_write_byte_data(data->cpld_client,
					MUX_SELECT_REG,
					chan + 1);
	if (ret < 0) {
		data->prev_chan = MUX_INVALID_CHANNEL;
		return ret;
	}

	data->prev_chan = chan;
	return 0;
}

static int deselect_mux(struct i2c_mux_core *muxc, u32 chan)
{
	struct port_mux_info *data = i2c_mux_priv(muxc);

	data->prev_chan = MUX_INVALID_CHANNEL;
	return i2c_smbus_write_byte_data(data->cpld_client,
					 MUX_SELECT_REG,
					 MUX_INVALID_CHANNEL);
}

static int mux_probe(struct i2c_client *client)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct device *dev = &client->dev;
	struct i2c_client *cpld_client = dev_get_platdata(dev);
	struct i2c_mux_core *muxc;
	struct port_mux_info *data;
	int i;
	int ret;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	muxc = i2c_mux_alloc(adap, dev, MUX_NUM_PORTS, sizeof(*data),
			     I2C_MUX_LOCKED, mux_select_chan, deselect_mux);
	if (!muxc)
		return -ENOMEM;
	data = i2c_mux_priv(muxc);

	i2c_set_clientdata(client, muxc);
	data->cpld_client = cpld_client;

	/*
	 * Write the mux register at addr to verify that the mux is in fact
	 * present. This also initializes the mux to deselected state.
	 */
	if (i2c_smbus_write_byte_data(cpld_client,
				      MUX_SELECT_REG,
				      MUX_INVALID_CHANNEL)) {
		dev_warn(dev, "probe failed\n");
		return -ENODEV;
	}

	data->prev_chan = MUX_INVALID_CHANNEL;

	/* Now create an adapter for each channel */
	for (i = 0; i < MUX_NUM_PORTS; i++) {
		ret = i2c_mux_add_adapter(muxc, MUX_FIRST_BUS_NUM + i, i, 0);
		if (ret)
			goto fail_cleanup;
	}

	dev_info(dev, "registered %d multiplexed busses for I2C mux %s\n",
		 i, client->name);

	return 0;

fail_cleanup:
	i2c_mux_del_adapters(muxc);
	return ret;
}

static int mux_remove(struct i2c_client *client)
{
	struct i2c_mux_core *muxc = i2c_get_clientdata(client);

	if (muxc)
		i2c_mux_del_adapters(muxc);
	return 0;
}

static const struct i2c_device_id mux_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mux_id);

static struct i2c_driver mux_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe_new = mux_probe,
	.remove = mux_remove,
	.id_table = mux_id,
};

/*
 * Module init/exit
 */
static int __init mux_init(void)
{
	int ret = 0;

	/*
	 * Do as little as possible in the module init function. Basically just
	 * register drivers. Those driver's probe functions will probe for
	 * hardware and create devices.
	 */
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " initializing\n");

	/* Register the custom I2C MUX driver */
	ret = i2c_add_driver(&mux_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": %s driver registration failed. (%d)\n",
		       mux_driver.driver.name, ret);
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");
	return ret;
}

static void __exit mux_exit(void)
{
	i2c_del_driver(&mux_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(mux_init);
module_exit(mux_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S41xx SFF MUX support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

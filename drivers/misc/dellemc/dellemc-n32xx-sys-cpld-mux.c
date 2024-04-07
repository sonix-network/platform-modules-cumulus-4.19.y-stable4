// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc-n32xx-sys-cpld-mux.c - Dell EMC N32xx System CPLD MUX Support.
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
#include <linux/i2c-mux.h>

#include "dellemc-n32xx-n22xx-cplds.h"

#define DRIVER_NAME             SYS_CPLD_MUX_DRIVER_NAME
#define DRIVER_VERSION          "1.2"

struct cpld_mux_control {
	char *dev_name;
	int invalid_channel;
	int num_legs;
	u8 reg;
	int first_bus;
};

static struct cpld_mux_control cpld_muxes[] = {
	{
		.dev_name = PORT_MUX_DEVICE_NAME,
		.invalid_channel = 0,
		.num_legs = 31,
		.reg = DELL_N32XX_SYS_PORT_I2C_MUX_REG,
		.first_bus = DELL_N32XX_N22XX_PORT_MUX_BUS_START
	},
	{
		.dev_name = FAN_MUX_DEVICE_NAME,
		.invalid_channel = 0,
		.num_legs = 3,
		.reg = DELL_N32XX_SYS_FAN_I2C_MUX_REG,
		.first_bus = DELL_N32XX_N22XX_FAN_MUX_BUS_START
	},
	{
		.dev_name = PSU_MUX_DEVICE_NAME,
		.invalid_channel = 0,
		.num_legs = 3,
		.reg = DELL_N32XX_SYS_PSU_I2C_MUX_REG,
		.first_bus = DELL_N32XX_N22XX_PSU_MUX_BUS_START
	},
	{
		.dev_name = NULL
	}
};

struct cpld_mux_info {
	int    prev_chan;
	struct i2c_client *cpld_client;
	struct i2c_mux_core *muxc;
	struct cpld_mux_control *mux_cntrl;
};

static int mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	struct cpld_mux_info *data = i2c_mux_priv(muxc);
	int ret;

	if (data->prev_chan == chan + 1)
		return 0;
	ret = i2c_smbus_write_byte_data(data->cpld_client,
					data->mux_cntrl->reg,
					chan + 1);
	if (ret < 0) {
		data->prev_chan = data->mux_cntrl->invalid_channel;
		return ret;
	}

	data->prev_chan = chan + 1;
	return 0;
}

static int deselect_mux(struct i2c_mux_core *muxc, u32 chan)
{
	struct cpld_mux_info *data = i2c_mux_priv(muxc);

	data->prev_chan = data->mux_cntrl->invalid_channel;
	return i2c_smbus_write_byte_data(data->cpld_client,
					 data->mux_cntrl->reg,
					 data->mux_cntrl->invalid_channel);
}

static int mux_probe(struct i2c_client *client)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct device *dev = &client->dev;
	struct system_cpld_mux_platform_data *pd = dev_get_platdata(dev);
	struct i2c_mux_core *muxc;
	struct cpld_mux_info *data;
	struct cpld_mux_control *mux_cntrl;
	int i;
	int ret;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	/* Find the MUX type */
	for (mux_cntrl = cpld_muxes; mux_cntrl->dev_name; mux_cntrl++)
		if (!strcmp(mux_cntrl->dev_name, pd->dev_name))
			break;

	if (!mux_cntrl->dev_name)
		return -EINVAL;

	muxc = i2c_mux_alloc(adap, dev, mux_cntrl->num_legs, sizeof(*data),
			     I2C_MUX_LOCKED, mux_select_chan, deselect_mux);
	if (!muxc)
		return -ENOMEM;
	data = i2c_mux_priv(muxc);

	i2c_set_clientdata(client, muxc);
	data->cpld_client = pd->cpld;
	data->mux_cntrl = mux_cntrl;

	/*
	 * Write the mux register at addr to verify that the mux is in fact
	 * present. This also initializes the mux to deselected state.
	 */
	if (i2c_smbus_write_byte_data(data->cpld_client,
				      data->mux_cntrl->reg,
				      data->mux_cntrl->invalid_channel)) {
		dev_warn(dev, "probe failed\n");
		return -ENODEV;
	}

	data->prev_chan = mux_cntrl->invalid_channel;

	/* Now create an adapter for each channel */
	for (i = 0; i < mux_cntrl->num_legs; i++) {
		ret = i2c_mux_add_adapter(muxc, mux_cntrl->first_bus + i, i, 0);
		if (ret)
			goto fail_cleanup;
	}

	dev_info(dev, "registered %d multiplexed busses for I2C mux %s\n",
		 i, mux_cntrl->dev_name);

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
MODULE_DESCRIPTION("Dell EMC N32XX System CPLD MUX support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

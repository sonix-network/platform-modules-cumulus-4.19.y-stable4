// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v1 PSU Mux Driver
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/i2c-mux.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-ag9032v1.h"

#define DRIVER_NAME         AG9032V1_PSUMUX_NAME
#define DRIVER_VERSION      "1.1"
#define MUX_INVALID_CHANNEL 0x0
#define MUX_NUM_PORTS       4
#define MUX_SELECT_REG      DELTA_AG9032V1_SWCPLD_PSU_AND_FAN_I2C_MUX_REG
#define MUX_SELECT_MSB      DELTA_AG9032V1_PSU_I2C_SEL_MSB
#define MUX_SELECT_LSB      DELTA_AG9032V1_PSU_I2C_SEL_LSB
#define MUX_FIRST_BUS_NUM   CPLD_PSU_MUX_BUS0

struct port_mux_info {
	int    prev_chan;
	struct cpld_item *swcpld_item;
	struct i2c_mux_core *muxc;
};

/* mux select/deselect */

static int mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	struct port_mux_info *data = i2c_mux_priv(muxc);
	int ret;

	if (data->prev_chan == chan) {
		ret = 0;
		goto exit;
	}

	mutex_lock(&data->swcpld_item->cpld_mutex);
	ret = i2c_smbus_read_byte_data(data->swcpld_item->cpld_client,
				       MUX_SELECT_REG);
	if (ret < 0) {
		mutex_unlock(&data->swcpld_item->cpld_mutex);
		data->prev_chan = MUX_INVALID_CHANNEL;
		goto exit;
	}
	ret = PUT_FIELD(ret, MUX_SELECT, chan);
	ret = i2c_smbus_write_byte_data(data->swcpld_item->cpld_client,
					MUX_SELECT_REG,
					ret);
	mutex_unlock(&data->swcpld_item->cpld_mutex);
	if (ret < 0) {
		data->prev_chan = MUX_INVALID_CHANNEL;
		goto exit;
	}

	data->prev_chan = chan;

exit:
	return ret;
}

static int deselect_mux(struct i2c_mux_core *muxc, u32 chan)
{
	struct port_mux_info *data = i2c_mux_priv(muxc);
	int ret;

	data->prev_chan = MUX_INVALID_CHANNEL;

	mutex_lock(&data->swcpld_item->cpld_mutex);
	ret = i2c_smbus_read_byte_data(data->swcpld_item->cpld_client,
				       MUX_SELECT_REG);
	if (ret < 0) {
		mutex_unlock(&data->swcpld_item->cpld_mutex);
		goto exit;
	}
	ret = PUT_FIELD(ret, MUX_SELECT, MUX_INVALID_CHANNEL);
	ret = i2c_smbus_write_byte_data(data->swcpld_item->cpld_client,
					MUX_SELECT_REG,
					ret);
	mutex_unlock(&data->swcpld_item->cpld_mutex);

exit:
	return ret;
}

/* mux probe */

static int mux_probe(struct i2c_client *client)
{
	struct i2c_adapter *adap = to_i2c_adapter(client->dev.parent);
	struct device *dev = &client->dev;
	struct cpld_item *item = dev_get_platdata(dev);
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
	data->swcpld_item = item;

	/*
	 * Write the mux register at addr to verify that the mux is in fact
	 * present. This also initializes the mux to deselected state.
	 */
	mutex_lock(&data->swcpld_item->cpld_mutex);
	ret = i2c_smbus_read_byte_data(data->swcpld_item->cpld_client,
				       MUX_SELECT_REG);
	if (ret < 0) {
		mutex_unlock(&data->swcpld_item->cpld_mutex);
		goto err_exit;
	}
	ret = PUT_FIELD(ret, MUX_SELECT, MUX_INVALID_CHANNEL);
	ret = i2c_smbus_write_byte_data(data->swcpld_item->cpld_client,
					MUX_SELECT_REG,
					MUX_INVALID_CHANNEL);
	mutex_unlock(&data->swcpld_item->cpld_mutex);
	if (ret < 0)
		goto err_exit;

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

err_exit:
	dev_warn(dev, "probe failed\n");
	return -ENODEV;

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
	{ },
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

/* module init/exit */

static int __init mux_init(void)
{
	int ret;

	ret = i2c_add_driver(&mux_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": driver failed to load\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit mux_exit(void)
{
	i2c_del_driver(&mux_driver);
	pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(mux_init);
module_exit(mux_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Delta AG9032v1 PSU Mux Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/*
 * cel_smallstone_xp_b_muxpld.c - Celestica Smallstone XP-B Platform Support
 *
 * Copyright (C) 2017 Cumulus Networks, Inc. all rights reserved
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_device.h>

#include "cel-smallstone-xp-b-cpld.h"
#include "cel-xp-b-muxpld.h"

#define DRIVER_NAME	"cel_smallstone_xp_b_muxpld"
#define DRIVER_VERSION	"1.0"

struct cel_small_xpb_bus_mux_info {
	int bus;
	u32 io_base;
	int first_port_num;
	int num_ports;
};

static struct cel_small_xpb_bus_mux_info small_xp_bus_mux_info[] = {
	{
		.bus = CL_I2C_CPLD_MUX_1,
		.io_base = CPLD2_REG_I2C_FREQ_DIV,
		.first_port_num = 1,
		.num_ports = 16,
	},
	{
		.bus = CL_I2C_CPLD_MUX_2,
		.io_base = CPLD3_REG_I2C_FREQ_DIV,
		.first_port_num = 17,
		.num_ports = 16,
	},
};

struct cel_sxp_mux_data_struct {
	struct platform_device *bus_devices[ARRAY_SIZE(small_xp_bus_mux_info)];
	struct platform_device *mux_devices[ARRAY_SIZE(small_xp_bus_mux_info)];
	struct i2c_client *i2c_clients[CEL_SMALLSTONE_XP_B_NUM_PORTS];
	struct i2c_board_info *board_infos[CEL_SMALLSTONE_XP_B_NUM_PORTS];
};

static struct cel_sxp_mux_data_struct cel_sxp_mux_data;

static struct i2c_adapter *get_adapter(int bus)
{
	int bail = 20;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;
		msleep(100);
	}
	return NULL;
}

static void free_qsfp_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static void free_rxp_data(void)
{
	int i;

	for (i = 0; i < CEL_SMALLSTONE_XP_B_NUM_PORTS; i++) {
		if (cel_sxp_mux_data.i2c_clients[i])
			i2c_unregister_device(cel_sxp_mux_data.i2c_clients[i]);
		kfree(cel_sxp_mux_data.board_infos[i]);
	}

	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		if (cel_sxp_mux_data.mux_devices[i])
			platform_device_del(cel_sxp_mux_data.mux_devices[i]);
		if (cel_sxp_mux_data.bus_devices[i])
			platform_device_del(cel_sxp_mux_data.bus_devices[i]);
	}
}

#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port)
{
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct sff_8436_platform_data *sff8436_data;
	struct i2c_board_info *board_info;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_exit;
	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_exit_eeprom;
	sff8436_data = kzalloc(sizeof(*sff8436_data), GFP_KERNEL);
	if (!sff8436_data)
		goto err_exit_sff8436;
	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_exit_board;

	snprintf(label, QSFP_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data->byte_len = 512;
	sff8436_data->flags = SFF_8436_FLAG_IRUGO;
	sff8436_data->page_size = 1;
	sff8436_data->eeprom_data = eeprom_data;

	strcpy(board_info->type, "sff8436");
	board_info->addr = 0x50;
	board_info->platform_data = sff8436_data;

	return board_info;

err_exit_board:
	kfree(sff8436_data);
err_exit_sff8436:
	kfree(eeprom_data);
err_exit_eeprom:
	kfree(label);
err_exit:
	return NULL;
};

int create_i2c_bus(int index, int bus, u32 io_base)
{
	struct cpld_bus_data bus_data;
	struct platform_device *device;
	int ret = 0;

	bus_data.bus = bus;
	bus_data.io_base = io_base;
	bus_data.clock = CPLD_I2C_400KHZ;
	bus_data.timeout = 10000;

	device = platform_device_alloc("cel_xp_b_i2c_bus", index);
	if (!device) {
		pr_err("could not alloc bus device\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = platform_device_add_data(device, &bus_data,
				       sizeof(struct cpld_bus_data));
	if (ret) {
		pr_err("could not add platform device data\n");
		platform_device_put(device);
		goto exit;
	}

	ret = platform_device_add(device);
	if (ret) {
		pr_err("could not add bus device\n");
		platform_device_put(device);
		goto exit;
	}
	cel_sxp_mux_data.bus_devices[index] = device;

exit:
	return ret;
}

int create_i2c_mux(int index, int bus, int first_port_num, int num_ports)
{
	struct cpld_mux_data mux_data;
	struct platform_device *device;
	struct i2c_adapter *parent_adapter;
	int ret = 0;
	int i;

	mux_data.mux_base_id = bus;
	mux_data.mux_base_port_num = first_port_num;
	mux_data.mux_num_ports = num_ports;
	mux_data.mux_ports_base_bus = CL_I2C_CPLD_MUX_FIRST_PORT;

	parent_adapter = get_adapter(bus);
	if (!parent_adapter) {
		ret = -ENODEV;
		pr_err("could not get i2c adapter %u\n", bus);
		goto exit;
	}

	device = platform_device_alloc("cel_xp_b_cpld_mux", index);
	if (!device) {
		pr_err("could not alloc mux device\n");
		ret = -ENOMEM;
		goto exit;
	}

	device->dev.parent = &parent_adapter->dev;
	ret = platform_device_add_data(device, &mux_data,
				       sizeof(struct cpld_mux_data));
	if (ret) {
		pr_err("could not add mux platform device data\n");
		platform_device_put(device);
		goto exit;
	}

	ret = platform_device_add(device);
	if (ret) {
		pr_err("could not add mux device\n");
		platform_device_put(device);
		goto exit;
	}
	cel_sxp_mux_data.mux_devices[index] = device;

	for (i = 0; i < num_ports; i++) {
		struct i2c_board_info *board_info;
		struct i2c_adapter *adapter;
		struct i2c_client *client;
		int bus_num;
		int port_num;

		port_num = first_port_num + i;
		bus_num = CL_I2C_CPLD_MUX_FIRST_PORT + port_num;

		adapter = get_adapter(bus_num);
		if (!adapter) {
			pr_err("could not get adapter %u\n", bus_num);
			ret = -ENODEV;
		}
		board_info = alloc_qsfp_board_info(port_num);
		if (!board_info) {
			pr_err("could not allocate board info\n");
			ret = -ENOMEM;
			goto exit;
		}
		client = i2c_new_device(adapter, board_info);
		if (!client) {
			free_qsfp_board_info(board_info);
			pr_err("could not create i2c_client %s\n",
			       board_info->type);
			ret = -ENOMEM;
			goto exit;
		}
		cel_sxp_mux_data.i2c_clients[port_num] = client;
	}

exit:
	return ret;
}

static int __init cel_smallstone_xp_b_cpld_mux_init(void)
{
	int ret;
	int i;

	/*
	 * create the i2c busses
	 */
	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		ret = create_i2c_bus(i, small_xp_bus_mux_info[i].bus,
				     small_xp_bus_mux_info[i].io_base);
		if (ret)
			goto err_exit;
	}
	/*
	 * create the i2c muxes and EEPROM devices
	 */
	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		ret = create_i2c_mux(i, small_xp_bus_mux_info[i].bus,
				     small_xp_bus_mux_info[i].first_port_num,
				     small_xp_bus_mux_info[i].num_ports);
		if (ret)
			goto err_exit;
	}

	pr_info("%s: version %s successfully loaded\n",
		DRIVER_NAME, DRIVER_VERSION);
	return 0;

err_exit:
	free_rxp_data();
	return ret;
}

static void __exit cel_smallstone_xp_b_cpld_mux_exit(void)
{
	free_rxp_data();
	pr_info("%s: version %s unloaded\n",
		DRIVER_NAME, DRIVER_VERSION);
}

module_init(cel_smallstone_xp_b_cpld_mux_init);
module_exit(cel_smallstone_xp_b_cpld_mux_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Smallstone XP-B CPLD mux Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

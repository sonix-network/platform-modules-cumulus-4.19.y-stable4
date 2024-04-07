/*
 * cel_redstone_v_platform.c - Celestica Redstone-V platform support.
 *
 * Copyright (C) 2017, 2018, 2019 Cumulus Networks, Inc.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>

#include "platform-defs.h"
#include "cel-redstone-v.h"

#define DRIVER_NAME    "cel_redstone_v_platform"
#define DRIVER_VERSION "1.0"

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device,
 * using the method specific to the device type.  Then we put them
 * all together in a big table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_pca9548(mux1, I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, I2C_MUX2_BUS0, 1);

mk_eeprom(board, 56, 8192, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);

mk_eeprom(psu1,	 50,  256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,	 51,  256, AT24_FLAG_IRUGO);

/*
 * struct i2c_device_info -- i2c device container struct.  This
 * struct helps organize the i2c_board_info structures and the created
 * i2c_client objects.
 *
 */

struct i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info structure specifies the device type, address,
 * and platform data specific to the device type.
 */

static struct i2c_device_info i2c_devices[] = {
	/* devices on i801 bus */
	mk_i2cdev(I2C_ISMT_BUS,	 "24c64",	0x56, &board_56_at24),
	mk_i2cdev(I2C_ISMT_BUS,	 "pca9548",	0x76, &mux1_platform_data),
	mk_i2cdev(I2C_ISMT_BUS,	 "pca9548",	0x77, &mux2_platform_data),

	/* devices on mux1 */
	mk_i2cdev(I2C_MUX1_BUS0, "dps460",	0x58, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",	0x50, &psu1_50_at24),

	mk_i2cdev(I2C_MUX1_BUS1, "lm75b",	0x49, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "lm75b",	0x4a, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "lm75b",	0x4b, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "lm75b",	0x4c, NULL),

	mk_i2cdev(I2C_MUX1_BUS2, "dps460",	0x59, NULL),
	mk_i2cdev(I2C_MUX1_BUS2, "24c02",	0x51, &psu2_51_at24),

};

/*
 * Utility functions for i2c
 */

static struct i2c_client *clients_list[ARRAY_SIZE(i2c_devices)];

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

static void free_i2c_clients(struct i2c_client **clients_list, int num_clients)
{
	int i, idx;

	for (i = num_clients; i; i--) {
		idx = i - 1;
		if (clients_list[idx])
			i2c_unregister_device(clients_list[idx]);
	}
}

static void free_data(void)
{
	free_i2c_clients(clients_list, ARRAY_SIZE(i2c_devices));
}

static struct i2c_client *add_i2c_client(int bus,
					 struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err("could not get adapter %u\n", bus);
		return ERR_PTR(-ENODEV);
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err("could not add device\n");
		return ERR_PTR(-ENODEV);
	}
	return client;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < I2C_MUX1_BUS0; i++) {
		adapter = get_adapter(i);
		if (adapter &&
		    (strncmp(adapter->name, name, strlen(name)) == 0)) {
			return i;
		}
	}
	return -1;
}

static int populate_i2c_devices(struct i2c_device_info *devices,
				int num_devices,
				struct i2c_client **clients_list,
				int ismt_bus, int i801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == I2C_ISMT_BUS)
			devices[i].bus = ismt_bus;
		else if (devices[i].bus == I2C_I801_BUS)
			devices[i].bus = i801_bus;

		client = add_i2c_client(devices[i].bus,
					&devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
	}
	return 0;

err_exit:
	return ret;
}

static int __init platform_init(void)
{
	int ismt_bus;
	int i801_bus;
	int ret;

	ret = -1;

	ismt_bus = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err("could not find iSMT adapter bus\n");
		 ret = -ENODEV;
		 goto err_exit;
	}
	i801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err("could not find i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/* populate the i2c devices */
	ret = populate_i2c_devices(i2c_devices,
				   ARRAY_SIZE(i2c_devices),
				   clients_list,
				   ismt_bus,
				   i801_bus);
	if (ret)
		goto err_exit;

	pr_info("%s: version %s successfully loaded\n",
		DRIVER_NAME, DRIVER_VERSION);
	return 0;

err_exit:
	free_data();
	return ret;
}

static void __exit platform_exit(void)
{
	free_data();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Redstone-V platform support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

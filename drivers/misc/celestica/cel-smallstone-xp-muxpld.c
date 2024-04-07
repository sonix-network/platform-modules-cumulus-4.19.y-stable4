/*
 * cel_smallstone_cpld_muxpld.c - Celestica Redstone XP Platform Support
 *
 * Author: Alan Liebthal (alanl@cumulusnetworks.com)
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
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

#include "cel-xp-platform.h"
#include "cel-smallstone-xp-cpld.h"
#include "cel-xp-muxpld.h"

#define DRIVER_NAME	"cel_smallstone_xp_muxpld"
#define DRIVER_VERSION	"1.0"

static struct platform_driver cel_smallstone_xp_cpld_mux_driver;

static struct cel_xp_bus_mux_info small_xp_bus_mux_info[] = {
	{
		.bus = RXP_I2C_CPLD_MUX_1,
		.io_base = CPLD2_REG_I2C_PORT_ID_OFFSET,
		.first_port_num = 1,
		.num_ports = 16,
	},
	{
		.bus = RXP_I2C_CPLD_MUX_2,
		.io_base = CPLD3_REG_I2C_PORT_ID_OFFSET,
		.first_port_num = 17,
		.num_ports = 16,
	},
};

struct cel_smallxp_mux_data_struc {
	struct platform_device *bus_devices[ARRAY_SIZE(small_xp_bus_mux_info)];
	struct platform_device *mux_devices[ARRAY_SIZE(small_xp_bus_mux_info)];
	struct i2c_client *i2c_clients[CEL_SMALLSTONE_XP_NUM_PORTS];
	struct i2c_board_info *board_infos[CEL_SMALLSTONE_XP_NUM_PORTS];
};

static struct cel_smallxp_mux_data_struc cel_smallxp_mux_data;

static struct i2c_adapter *get_adapter(int bus)
{
	int bail=20;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter) {
			return adapter;
		}
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

	for (i = 0; i < CEL_SMALLSTONE_XP_NUM_PORTS; i++) {
		if (cel_smallxp_mux_data.i2c_clients[i]) {
			i2c_unregister_device(cel_smallxp_mux_data.i2c_clients[i]);
		}
		if (cel_smallxp_mux_data.board_infos[i]) {
			kfree(cel_smallxp_mux_data.board_infos[i]);
		}
	}

	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		if (cel_smallxp_mux_data.mux_devices[i]) {
			platform_device_del(cel_smallxp_mux_data.mux_devices[i]);
		}
		if (cel_smallxp_mux_data.bus_devices[i]) {
//			platform_device_del(cel_smallxp_mux_data.bus_devices[i]);  aplx
		}
	}
}

#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct sff_8436_platform_data *sff8436_data;
	struct i2c_board_info *board_info;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	sff8436_data = kzalloc(sizeof(*sff8436_data), GFP_KERNEL);
	if (!sff8436_data) {
		goto err_exit_sff8436;
	}
	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

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

static int cel_smallstone_xp_cpld_mux_probe(struct platform_device *dev)
{
	return 0;
}

static int cel_smallstone_xp_cpld_mux_remove(struct platform_device *dev)
{
	return 0;
}

int create_i2c_bus(int index, int bus, u32 io_base)
{
	struct cpld_bus_data bus_data;
	struct platform_device *device;
	int ret = 0;

	bus_data.bus = bus;
	bus_data.io_base = io_base;
	bus_data.clock = 100000;
	bus_data.timeout = 10000;

	device = platform_device_alloc("cel_xp_i2c_bus", index);
	if (!device) {
		pr_err("could not alloc bus device\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = platform_device_add_data(device, &bus_data, sizeof(struct cpld_bus_data));
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
	cel_smallxp_mux_data.bus_devices[index] = device;

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

	parent_adapter = get_adapter(bus);
	if (!parent_adapter) {
		ret = -ENODEV;
		pr_err("could not get i2c adapter %u\n", bus);
		goto exit;
	}

	device = platform_device_alloc("cel_xp_cpld_i2c_mux", index);
	if (!device) {
		pr_err("could not alloc mux device\n");
		ret = -ENOMEM;
		goto exit;
	}

	device->dev.parent = &parent_adapter->dev;
	ret = platform_device_add_data(device, &mux_data, sizeof(struct cpld_mux_data));
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
	cel_smallxp_mux_data.mux_devices[index] = device;

	for (i = 0; i < num_ports; i++) {
		struct i2c_board_info *board_info;
		struct i2c_adapter *adapter;
		struct i2c_client *client;
		int bus_num;
		int port_num;

		port_num = first_port_num + i;
		bus_num = RXP_I2C_CPLD_MUX_FIRST_PORT + port_num;

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
			pr_err("could not create i2c_client %s\n", board_info->type);
			ret = -ENOMEM;
			goto exit;
		}
		cel_smallxp_mux_data.i2c_clients[port_num - 1] = client;
	}

exit:
	return ret;
}

static int __init cel_smallstone_xp_cpld_mux_init(void)
{
	int ret;
	int i;

	ret = platform_driver_register(&cel_smallstone_xp_cpld_mux_driver);
	if (ret) {
		pr_err("could not register smallstone_xp_cpld_mux_driver\n");
		goto err_exit;
	}

	/*
	 * create the i2c busses
	 */
	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		ret = create_i2c_bus(i, small_xp_bus_mux_info[i].bus, small_xp_bus_mux_info[i].io_base);
		if (ret) {
			goto err_exit2;
		}
	}
	/*
	 * create the i2c muxes and EEPROM devices
	 */
	for (i = 0; i < ARRAY_SIZE(small_xp_bus_mux_info); i++) {
		ret = create_i2c_mux(i, small_xp_bus_mux_info[i].bus,
		                     small_xp_bus_mux_info[i].first_port_num,
		                     small_xp_bus_mux_info[i].num_ports);
		if (ret) {
			goto err_exit2;
		}
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;

err_exit2:
	free_rxp_data();
	platform_driver_unregister(&cel_smallstone_xp_cpld_mux_driver);

err_exit:
	return ret;
}

static void __exit cel_smallstone_xp_cpld_mux_exit(void)
{
	free_rxp_data();
	platform_driver_unregister(&cel_smallstone_xp_cpld_mux_driver);
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" unloaded\n");
}

static struct platform_driver cel_smallstone_xp_cpld_mux_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = cel_smallstone_xp_cpld_mux_probe,
	.remove = cel_smallstone_xp_cpld_mux_remove,
};


module_init(cel_smallstone_xp_cpld_mux_init);
module_exit(cel_smallstone_xp_cpld_mux_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Smallstone XP CPLD mux Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

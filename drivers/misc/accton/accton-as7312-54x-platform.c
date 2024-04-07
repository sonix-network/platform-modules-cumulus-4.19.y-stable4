/*
 * accton_as7312_54x_platform.c - Accton as7312-54x Platform Support.
 *
 * Copyright 2017 Cumulus Networks, Inc.
 * Author: Alan Liebthal (alanl@cumulusnetworks.com)
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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/pmbus.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "accton-as7312-54x-cpld.h"

#define DRIVER_NAME	"accton_as7312_54x_platform"
#define DRIVER_VERSION	"0.1"

/*---------------------------------------------------------------------
 *
 * Platform driver
 *
 *-------------------------------------------------------------------*/
static struct i2c_client *act_as7312_cpld_clients[NUM_CPLD_I2C_CLIENTS];

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).
 */

/* i2c bus adapter numbers for the i2c busses */
enum {
	CL_I2C_I801_BUS = 0,
	CL_I2C_ISMT_BUS,
	CL_I2C_I801_MUX_71_BUS0 = 10,
	CL_I2C_I801_MUX_76_BUS0 = 20,
	CL_I2C_ISMT_MUX_71_BUS0 = 30,
	CL_I2C_ISMT_MUX_72_BUS0 = 38,
	CL_I2C_ISMT_MUX_73_BUS0 = 46,
	CL_I2C_ISMT_MUX_74_BUS0 = 54,
	CL_I2C_ISMT_MUX_75_BUS0 = 62,
	CL_I2C_ISMT_MUX_76_BUS0 = 70,
	CL_I2C_ISMT_MUX_70_BUS0 = 86,
};

mk_eeprom(spd1,  52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2,  53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,  50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

enum {
	PORT_TYPE_NONE = 0,
	PORT_TYPE_SFP,
	PORT_TYPE_QSFP,
} port_type_t;

struct cl_platform_device_info {
	int bus;
	struct i2c_board_info board_info;
	int has_port;
	int port_base;
	int port_bus;
	int num_ports;
};

struct port_info {
	int type;
	struct i2c_board_info *b;
	struct i2c_client *c;
};

#define ACT_7312_PORT_COUNT    54
static struct port_info ports_info[ACT_7312_PORT_COUNT];

#define cl_i2c_pca9548(addr, bus, busno)                             \
	enum {                                                       \
		CL_I2C_##bus##_MUX_##addr##_0 = busno,               \
		CL_I2C_##bus##_MUX_##addr##_1,                       \
		CL_I2C_##bus##_MUX_##addr##_2,                       \
		CL_I2C_##bus##_MUX_##addr##_3,                       \
		CL_I2C_##bus##_MUX_##addr##_4,                       \
		CL_I2C_##bus##_MUX_##addr##_5,                       \
		CL_I2C_##bus##_MUX_##addr##_6,                       \
		CL_I2C_##bus##_MUX_##addr##_7,                       \
	};                                                           \
	static struct pca954x_platform_mode                          \
		cl_i2c_mode_pca9548_##bus##_##addr[] =               \
		{	{ .adap_id = CL_I2C_##bus##_MUX_##addr##_0,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_1,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_2,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_3,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_4,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_5,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_6,  \
			  .deselect_on_exit = 1,},                   \
			{ .adap_id = CL_I2C_##bus##_MUX_##addr##_7,  \
			  .deselect_on_exit = 1,},                   \
	};                                                           \
	static struct pca954x_platform_data                          \
		cl_i2c_data_pca9548_##bus##_##addr = {               \
			.modes = cl_i2c_mode_pca9548_##bus##_##addr, \
			.num_modes = ARRAY_SIZE(                     \
			cl_i2c_mode_pca9548_##bus##_##addr),         \
	}

cl_i2c_pca9548(76, I801, CL_I2C_I801_MUX_76_BUS0);
cl_i2c_pca9548(71, I801, CL_I2C_I801_MUX_71_BUS0);
cl_i2c_pca9548(70, ISMT, CL_I2C_ISMT_MUX_70_BUS0);
cl_i2c_pca9548(71, ISMT, CL_I2C_ISMT_MUX_71_BUS0);
cl_i2c_pca9548(72, ISMT, CL_I2C_ISMT_MUX_72_BUS0);
cl_i2c_pca9548(73, ISMT, CL_I2C_ISMT_MUX_73_BUS0);
cl_i2c_pca9548(74, ISMT, CL_I2C_ISMT_MUX_74_BUS0);
cl_i2c_pca9548(75, ISMT, CL_I2C_ISMT_MUX_75_BUS0);
cl_i2c_pca9548(76, ISMT, CL_I2C_ISMT_MUX_76_BUS0);

static struct cl_platform_device_info i2c_devices[] = {
	/* I2C_i801 Bus */
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x52),
			.platform_data = &spd1_52_at24,
		},
	},
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x53),
			.platform_data = &spd2_53_at24,
		},
	},
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x71),
			.platform_data = &cl_i2c_data_pca9548_I801_71,
		},
	},
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x76),
			.platform_data = &cl_i2c_data_pca9548_I801_76,
		},
	},
	{
		.bus = CL_I2C_I801_MUX_71_0,
		{
			I2C_BOARD_INFO("pmbus", 0x58),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_71_0,
		{
			I2C_BOARD_INFO("24c02", 0x50),
			.platform_data = &psu2_50_at24,
		},
	},
	{
		.bus = CL_I2C_I801_MUX_71_1,
		{
			I2C_BOARD_INFO("pmbus", 0x59),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_71_1,
		{
			I2C_BOARD_INFO("24c02", 0x51),
			.platform_data = &psu1_51_at24,
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_0,
		{
			I2C_BOARD_INFO("dummy", 0x66),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x48),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x49),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4a),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4b),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_2,
		{
			I2C_BOARD_INFO("dummy", 0x60),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_3,
		{
			I2C_BOARD_INFO("dummy", 0x62),
		},
	},
	{
		.bus = CL_I2C_I801_MUX_76_4,
		{
			I2C_BOARD_INFO("dummy", 0x64),
		},
	},
	{
		.bus = CL_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("24c02", 0x57),     /* board EEPROM */
			.platform_data = &board_57_at24,
		},
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* QSFP ports 49-54 */
		{
			I2C_BOARD_INFO("pca9548", 0x70),
			.platform_data = &cl_i2c_data_pca9548_ISMT_70,
		},
		.has_port = PORT_TYPE_QSFP,
		.num_ports = 6,
		.port_base = 49,
		.port_bus = CL_I2C_ISMT_MUX_70_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 41 - 48 */
		{
			I2C_BOARD_INFO("pca9548", 0x71),
			.platform_data = &cl_i2c_data_pca9548_ISMT_71,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 41,
		.port_bus = CL_I2C_ISMT_MUX_71_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 1 - 8 */
		{
			I2C_BOARD_INFO("pca9548", 0x72),
			.platform_data = &cl_i2c_data_pca9548_ISMT_72,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 1,
		.port_bus = CL_I2C_ISMT_MUX_72_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 9 - 16 */
		{
			I2C_BOARD_INFO("pca9548", 0x73),
			.platform_data = &cl_i2c_data_pca9548_ISMT_73,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 9,
		.port_bus = CL_I2C_ISMT_MUX_73_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 17 - 24 */
		{
			I2C_BOARD_INFO("pca9548", 0x74),
			.platform_data = &cl_i2c_data_pca9548_ISMT_74,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 17,
		.port_bus = CL_I2C_ISMT_MUX_74_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 25 - 32 */
		{
			I2C_BOARD_INFO("pca9548", 0x75),
			.platform_data = &cl_i2c_data_pca9548_ISMT_75,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 25,
		.port_bus = CL_I2C_ISMT_MUX_75_BUS0,
	},
	{
		.bus = CL_I2C_ISMT_BUS,                /* SFP ports 33 - 40 */
		{
			I2C_BOARD_INFO("pca9548", 0x76),
			.platform_data = &cl_i2c_data_pca9548_ISMT_76,
		},
		.has_port = PORT_TYPE_SFP,
		.num_ports = 8,
		.port_base = 33,
		.port_bus = CL_I2C_ISMT_MUX_76_BUS0,
	},
};

/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
struct i2c_client_info {
	struct i2c_client *i2c_client;
	struct cl_platform_device_info *platform_info;
};

static struct i2c_client_info i2c_clients[ARRAY_SIZE(i2c_devices)];
static int num_i2c_clients;
/**
 *
 * TODO:  This should be common code used by most platforms....
 *
 * Fetch i2c adapter by bus number.
 *
 * The retry / sleep logic is required as the dynamically allocated
 * adapters from the MUX devices can take "some time" to become
 * available.
 */
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
		client = ERR_PTR(-ENODEV);
	}

	i2c_put_adapter(adapter);
	return client;
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
	sff8436_data = kzalloc(sizeof(*sff8436_data),
			       GFP_KERNEL);
	if (!sff8436_data)
		goto err_exit_sff8436;
	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_exit_board;

	snprintf(label, QSFP_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data->byte_len = 256;
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

static int act_as7312_alloc_qsfp(int port, int bus, int num_ports)
{
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		int port_num = port + j;

		b_info = alloc_qsfp_board_info(port_num);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n",
			       port_num);
			return -1;
		}
		c = add_i2c_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n",
			       b_info->type, port_num);
			return -1;
		}
		ports_info[port_num - 1].type = 1;
		ports_info[port_num - 1].b = b_info;
		ports_info[port_num - 1].c = c;
	}
	return 0;
}

#define SFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_sfp_board_info(int port)
{
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct at24_platform_data *at24_data;
	struct i2c_board_info *board_info;

	label = kzalloc(SFP_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_exit;
	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_exit_eeprom;
	at24_data = kzalloc(sizeof(*at24_data), GFP_KERNEL);
	if (!at24_data)
		goto err_exit_at24;
	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_exit_board;

	snprintf(label, SFP_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	at24_data->byte_len = 512;
	at24_data->flags = AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG;
	at24_data->page_size = 1;
	at24_data->eeprom_data = eeprom_data;

	strcpy(board_info->type, "24c04");
	board_info->addr = 0x50;
	board_info->platform_data = at24_data;

	return board_info;

err_exit_board:
	kfree(at24_data);
err_exit_at24:
	kfree(eeprom_data);
err_exit_eeprom:
	kfree(label);
err_exit:
	return NULL;
}

static void free_sfp_board_info(struct i2c_board_info *board_info)
{
	struct at24_platform_data *at24_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = at24_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(at24_data);
	kfree(board_info);
}

static int act_as7312_alloc_sfp(int port, int bus, int num_ports)
{
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_sfp_board_info(port + j);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n",
			       port + j);
			return -1;
		}
		c = add_i2c_client(bus + j, b_info);
		if (!c) {
			free_sfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n",
			       b_info->type, port + j);
			return -1;
		}
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}

static int act_as7312_alloc_port(int type, int base, int bus,
				 int num_ports)
{
	int res;

	if (type == PORT_TYPE_QSFP)
		res = act_as7312_alloc_qsfp(base, bus, num_ports);
	else
		res = act_as7312_alloc_sfp(base, bus, num_ports);

	return res;
}

static void free_i2c_data(void)
{
	int i;
	/*
	 * Free the devices in reverse order so that child devices are
	 * freed before parent mux devices.
	 */
	for (i = num_i2c_clients - 1; i >= 0; i--)
		i2c_unregister_device(i2c_clients[i].i2c_client);
}

static int __init get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i <= CL_I2C_ISMT_BUS; i++) {
		adapter = get_adapter(i);
		if (adapter &&
		    (strncmp(adapter->name, name, strlen(name)) == 0)) {
			i2c_put_adapter(adapter);
			return i;
		}
		i2c_put_adapter(adapter);
	}
	return -1;
}

static int ismt_bus_num;
static int i801_bus_num;

/**
 * act_as7312_i2c_init -- Initialize I2C devices
 *
 */
static int __init act_as7312_i2c_init(void)
{
	struct i2c_client *client;
	int i;
	int ret = -1;
	int cpld_count = 0;

	ismt_bus_num = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (ismt_bus_num < 0) {
		pr_err("could not find ismt adapter bus\n");
		ret = -ENXIO;
		goto err_exit;
	}
	i801_bus_num = get_bus_by_name(I801_ADAPTER_NAME);
	if (i801_bus_num < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		/*
		 * Map logical buses CL_I2C_ISMT_BUS and
		 * CL_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case CL_I2C_ISMT_BUS:
			i2c_devices[i].bus = ismt_bus_num;
			break;
		case CL_I2C_I801_BUS:
			i2c_devices[i].bus = i801_bus_num;
			break;
		default:
			break;
			/* Fall through for PCA9548 buses */
		};
		client = add_i2c_client(i2c_devices[i].bus,
					&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_clients[num_i2c_clients].platform_info = &i2c_devices[i];
		i2c_clients[num_i2c_clients++].i2c_client = client;
		if (strcmp(i2c_devices[i].board_info.type, "dummy") == 0)
			act_as7312_cpld_clients[cpld_count++] = client;

		if (i2c_devices[i].has_port != PORT_TYPE_NONE) {
			act_as7312_alloc_port(i2c_devices[i].has_port,
					      i2c_devices[i].port_base,
					      i2c_devices[i].port_bus,
					      i2c_devices[i].num_ports);
		}
	}
	return 0;

err_exit:
	free_i2c_data();
	return ret;
}

static void __exit act_as7312_i2c_exit(void)
{
	free_i2c_data();
}

/*---------------------------------------------------------------------
 *
 * CPLD driver
 *
 *-------------------------------------------------------------------*/

static struct platform_device *act_as7312_cpld_device;

#define CPLD_NAME  "accton_as7312_54x_cpld"

static uint8_t cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	uint8_t val;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [%u:0x%02X]",
		       cpld_idx, reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	client = act_as7312_cpld_clients[cpld_idx];
	val = i2c_smbus_read_byte_data(client, reg);
	if (val < 0)
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
	return val;
}

static s32 cpld_reg_write(uint32_t reg, uint8_t write_val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("write to invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	client = act_as7312_cpld_clients[cpld_idx];
	res = i2c_smbus_write_byte_data(client, reg, write_val);
	if (res)
		pr_err("CPLD wr - addr: 0x%02X, reg: 0x%02X, val: 0x%02X",
		       client->addr, reg, write_val);
	return res;
}

/*
 * cpld_version
 */
static uint32_t cpld_ver_regs[] = { ACT7312_CPLD1_VERSION_REG,
				    ACT7312_CPLD2_VERSION_REG,
				    ACT7312_CPLD3_VERSION_REG,
				    ACT7312_CPLD4_VERSION_REG};
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	uint32_t reg = cpld_ver_regs[dattr->attr.name[4] - '1'];

	return sprintf(buf, "0x%02X\n", cpld_reg_read(reg));
}
static SYSFS_ATTR_RO(cpld1_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld2_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld3_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld4_version, cpld_version_show);

/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * psu_pwrX
 */
static ssize_t bulk_power_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	uint8_t read_val;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok;
	uint8_t error_l;

	read_val = cpld_reg_read(ACT7312_PSU_STATUS_REG);
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask      = CPLD_PSU1_MASK;
		present_l = CPLD_PSU1_PRESENT_L;
		pwr_ok    = CPLD_PSU1_POWER_GOOD;
		error_l   = CPLD_PSU1_ALERT_L;
	} else {
		mask      = CPLD_PSU2_MASK;
		present_l = CPLD_PSU2_PRESENT_L;
		pwr_ok    = CPLD_PSU2_POWER_GOOD;
		error_l   = CPLD_PSU2_ALERT_L;
	}
	read_val &= mask;

	if (~read_val & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(read_val & pwr_ok) || !(read_val & error_l))
			strcat(buf, ", " PLATFORM_PS_POWER_BAD);
		else
			strcat(buf, ", " PLATFORM_OK);
	} else {
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);

/*
 * Fan tachometers - each fan module contains two fans.  Map the sysfs
 * files to the Accton fan names as follows:
 *
 *   sysfs Name | Accton Specification
 *   ===========+=====================
 *   fan1	| FAN1
 *   fan2	| FANR1
 *   fan3	| FAN2
 *   fan4	| FANR2
 *   fan5	| FAN3
 *   fan6	| FANR3
 *   fan7	| FAN4
 *   fan8	| FANR4
 *   fan9	| FAN5
 *   fan10      | FANR5
 *   fan11	| FAN6
 *   fan12      | FANR6
 *
 */
static ssize_t fan_tach_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	int fan = 0;
	uint32_t reg;
	s32 val;
	int rpm;

	if ((sscanf(dattr->attr.name, "fan%d_", &fan) < 1) ||
	    (fan < 1) || (fan > 12))
		return -EINVAL;

	if (fan & 0x1)
		reg = ACT7312_FAN1_SPEED_REG + fan / 2;
	else
		reg = ACT7312_FANR1_SPEED_REG + (fan - 1) / 2;

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	/* as indicated in the AS7312-54X_CPLD_BT2_R0C_V0.C-2016-10-25.pdf spec:
	 *    1 ~ 255: the number of fan rotations in 600ms
	 *       => Fan speed (RPM) = Reg_value * 100
	 *       EX: register value : 80
	 *          The RPM value is 80*100 = 8000
	 */
	rpm = (uint8_t)val * FAN_SPEED_MULTIPLIER;

	return sprintf(buf, "%d\n", rpm);
}
static SYSFS_ATTR_RO(fan1_input, fan_tach_show);
static SYSFS_ATTR_RO(fan2_input, fan_tach_show);
static SYSFS_ATTR_RO(fan3_input, fan_tach_show);
static SYSFS_ATTR_RO(fan4_input, fan_tach_show);
static SYSFS_ATTR_RO(fan5_input, fan_tach_show);
static SYSFS_ATTR_RO(fan6_input, fan_tach_show);
static SYSFS_ATTR_RO(fan7_input, fan_tach_show);
static SYSFS_ATTR_RO(fan8_input, fan_tach_show);
static SYSFS_ATTR_RO(fan9_input, fan_tach_show);
static SYSFS_ATTR_RO(fan10_input, fan_tach_show);
static SYSFS_ATTR_RO(fan11_input, fan_tach_show);
static SYSFS_ATTR_RO(fan12_input, fan_tach_show);

/*
 * Fan present - see the discussion above for fan_tach_show() for the
 * mapping of sysfs file names to the Accton fan names.
 *
 */
static ssize_t fan_ok_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	int fan = 0;
	uint8_t fan_bit;
	s32 val;

	if ((sscanf(dattr->attr.name, "fan%d_", &fan) < 1) ||
	    (fan < 1) || (fan > 12))
		return -EINVAL;

	fan_bit = 1 << ((fan - 1) / 2);
	val = cpld_reg_read(ACT7312_FAN_TRAY_PRESENT_REG);
	if (val < 0)
		return val;

	return sprintf(buf, "%c\n", (val & fan_bit ? '0' : '1'));
}
static SYSFS_ATTR_RO(fan1_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan2_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan3_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan4_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan5_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan6_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan7_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan8_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan9_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan10_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan11_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan12_ok, fan_ok_show);

/*
 * fan pwm
 *  This is a 4 bit value. To get this to look correct
 *  for a 0 - 255 scale, we use 16 * (val + 1) - 1.
 *
 *  From the spec:
 *     0100 : 5 x 6.25% = 31.5% duty cycle or 16 * (4 + 1) - 1 = 79
 *     0101 : 6 x 6.25% = 37.5% duty cycle or 16 * (5 + 1) - 1 = 94
 *      ...
 *     1111: 16 x 6.25% = 100% duty cycle or 16 * (15 + 1) - 1 = 255
 */
static ssize_t pwm_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	return sprintf(buf, "%u\n",
		       ((cpld_reg_read(ACT7312_FAN_PWM_REG) + 1) *
			 FAN_PWM_MULTIPLIER) - 1);
}

static ssize_t pwm_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	unsigned int val;
	int ret;

	/* there is only one pwm but we make it look like
	 * there's 6 (one for each tray)
	 */
	ret = kstrtouint(buf, 0, &val);
	if (ret < 0 || val > 255)
		return ret;

	val = val / FAN_PWM_MULTIPLIER;
	cpld_reg_write(ACT7312_FAN_PWM_REG, (u8)val);
	return count;
}
SYSFS_ATTR_RW(pwm1, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm2, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm3, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm4, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm5, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm6, pwm_show, pwm_store);

/*
 * fan_watchdog_enable
 */
static ssize_t fan_wdog_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	return sprintf(buf, "%c\n",
		       ((cpld_reg_read(ACT7312_CPLD_FAN_WATCHDOG_REG) &
			 FAN_WATCHDOG_ENABLE) ? '1' : '0'));
}

static ssize_t fan_wdog_store(struct device *dev,
			      struct device_attribute *dattr,
			      const char *buf, size_t count)
{
	int val;
	int ret;
	uint8_t read_val;

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;

	read_val = cpld_reg_read(ACT7312_CPLD_FAN_WATCHDOG_REG);
	if (val)
		read_val |= FAN_WATCHDOG_ENABLE;
	else
		read_val &= ~FAN_WATCHDOG_ENABLE;
	cpld_reg_write(ACT7312_CPLD_FAN_WATCHDOG_REG, read_val);
	return count;
}
static SYSFS_ATTR_RW(fan_watchdog_enable, fan_wdog_show, fan_wdog_store);

/*------------------------------------------------------------------------------
 *
 * System LED definitions
 *
 */

struct led {
	char name[PLATFORM_LED_COLOR_NAME_SIZE];
	uint32_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[4];
};

static struct led cpld_leds[] = {
	{
		.name = "led_diag",
		.reg  = ACT7312_SYS_LED1_REG,
		.mask = ACT7312_DIAG_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, ACT7312_DIAG_LED_GREEN},
			{ PLATFORM_LED_YELLOW, ACT7312_DIAG_LED_AMBER},
			{ PLATFORM_LED_OFF, ACT7312_DIAG_LED_OFF},
			{ PLATFORM_LED_RED, ACT7312_DIAG_LED_RED},
		},
	},
	{
		.name = "led_loc",
		.reg  = ACT7312_SYS_LED1_REG,
		.mask = ACT7312_LOC_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_OFF, ACT7312_LOC_LED_OFF},
			{ PLATFORM_LED_AMBER, ACT7312_LOC_LED_AMBER},
		},
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * Front Panel Status LEDs
 */
static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	s32 val;
	uint8_t val8;
	int i;
	struct led *target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	/* read the register */
	val = cpld_reg_read(target->reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;

	/* find the color */
	val8 &= target->mask;
	for (i = 0; i < target->n_colors; i++)
		if (val8 == target->colors[i].value)
			break;

	if (i == target->n_colors)
		return sprintf(buf, "undefined color\n");
	else
		return sprintf(buf, "%s\n", target->colors[i].name);

	return strlen(buf);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	s32 val;
	uint8_t val8;
	int i, ret;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;

	for (i = 0; i < target->n_colors; i++)
		if (strcmp(raw, target->colors[i].name) == 0)
			break;

	if (i == target->n_colors)
		return -EINVAL;

	/* set the new value */
	val = cpld_reg_read(target->reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;
	val8 &= ~target->mask;
	val8 |= target->colors[i].value;
	ret = cpld_reg_write(target->reg, val8);
	if (ret < 0)
		return ret;

	return count;
}
static SYSFS_ATTR_RW(led_diag, led_show, led_store);
static SYSFS_ATTR_RW(led_loc, led_show, led_store);

/*
 * Port LEDs RGB
 *  we can set the LEDs for ports 49-54 to use
 * various colors depending what speed mode they
 * are in.
 */
static u32 set_red_regs[] = {
	ACT7312_100G_LED_RED1_REG, ACT7312_25G_LED_RED1_REG,
	ACT7312_40G_LED_RED1_REG,  ACT7312_10G_LED_RED1_REG,
	ACT7312_100G_LED_RED2_REG, ACT7312_25G_LED_RED2_REG,
	ACT7312_40G_LED_RED2_REG,  ACT7312_10G_LED_RED2_REG,
};

static u32 set_green_regs[] = {
	ACT7312_100G_LED_GREEN1_REG, ACT7312_25G_LED_GREEN1_REG,
	ACT7312_40G_LED_GREEN1_REG,  ACT7312_10G_LED_GREEN1_REG,
	ACT7312_100G_LED_GREEN2_REG, ACT7312_25G_LED_GREEN2_REG,
	ACT7312_40G_LED_GREEN2_REG,  ACT7312_10G_LED_GREEN2_REG,
};

static u32 set_blue_regs[] = {
	ACT7312_100G_LED_BLUE1_REG, ACT7312_25G_LED_BLUE1_REG,
	ACT7312_40G_LED_BLUE1_REG,  ACT7312_10G_LED_BLUE1_REG,
	ACT7312_100G_LED_BLUE2_REG, ACT7312_25G_LED_BLUE2_REG,
	ACT7312_40G_LED_BLUE2_REG,  ACT7312_10G_LED_BLUE2_REG,
};

#define NUM_COLOR_REGS  (sizeof(set_green_regs) / sizeof(u32))

static ssize_t port_leds_rgb_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	u32 val;

	val = cpld_reg_read(ACT7312_100G_LED_RED1_REG) << 16;
	val |= cpld_reg_read(ACT7312_100G_LED_GREEN1_REG) << 8;
	val |= cpld_reg_read(ACT7312_100G_LED_BLUE1_REG);

	return sprintf(buf, "0x%03X\n", val);
}

static ssize_t port_leds_rgb_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	uint val;
	int ret, i;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0 || val > 0xffffff)
		return ret;

	for (i = 0; i < NUM_COLOR_REGS; i++) {
		cpld_reg_write(set_red_regs[i], val >> 16);
		cpld_reg_write(set_green_regs[i], (val >> 8) & 0xff);
		cpld_reg_write(set_blue_regs[i], val & 0xff);
	}

	return count;
}
static SYSFS_ATTR_RW(port_leds_rgb, port_leds_rgb_show, port_leds_rgb_store);

/*------------------------------------------------------------------------------
 *
 * SFP status definitions
 *
 * All the definition use positive logic.
 */
struct sfp_status {
	char name[PLATFORM_LED_COLOR_NAME_SIZE];
	uint32_t regs[2];
	uint8_t active_low;
};

static struct sfp_status cpld_sfp_status[] = {
	{
		.name = "present",
		.active_low = 1,
		.regs = {
			ACT7312_SFP_1_8_PRESENT_REG,
			ACT7312_SFP_25_32_PRESENT_REG,
		},
	},
	{
		.name = "tx_fault",
		.regs = {
			ACT7312_SFP_1_8_TX_FAULT_REG,
			ACT7312_SFP_25_32_TX_FAULT_REG,
		},
	},
	{
		.name = "tx_disable",
		.regs = {
			ACT7312_SFP_1_8_TX_DISABLE_REG,
			ACT7312_SFP_25_32_TX_DISABLE_REG,
		},
	},
	{
		.name = "rx_loss",
		.regs = {
			ACT7312_SFP_1_8_RX_LOSS_REG,
			ACT7312_SFP_25_32_RX_LOSS_REG,
		},
	},
};

static int n_sfp_status = ARRAY_SIZE(cpld_sfp_status);

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int i, j;
	u64 val;
	s32 data;
	uint8_t name_len = 4; /* strlen("sfp_"); */
	struct sfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_sfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len,
			   cpld_sfp_status[i].name) == 0) {
			target = &cpld_sfp_status[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	val = 0;
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 3; j++) {
			data = cpld_reg_read(target->regs[i] + j);
			if (data < 0)
				return data;
			if (target->active_low)
				data = ~data;
			val |= (((u64)data & 0xff) << 8 * (i * 3 + j));
		}
	}
	return sprintf(buf, "0x%llx\n", val);
}

static ssize_t sfp_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int i, j, retval;
	u64 val, data;
	uint8_t name_len = 4; /* strlen("sfp_"); */
	struct sfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_sfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len,
			   cpld_sfp_status[i].name) == 0) {
			target = &cpld_sfp_status[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	retval = kstrtou64(buf, 0, &val);
	if (retval != 0)
		return retval;
	if (target->active_low)
		val = ~val;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 3; j++) {
			data = (val >> 8 * (i * 3 + j)) & 0xff;
			retval = cpld_reg_write(target->regs[i] + j, (u8)data);
			if (retval < 0)
				return retval;
		}
	}
	return count;
}
static SYSFS_ATTR_RO(sfp_present,     sfp_show);
static SYSFS_ATTR_RO(sfp_tx_fault,    sfp_show);
static SYSFS_ATTR_RW(sfp_tx_disable,  sfp_show, sfp_store);
static SYSFS_ATTR_RO(sfp_rx_loss,     sfp_show);

/*------------------------------------------------------------------------------
 *
 * QSFP status definitions
 *
 * All the definition use positive logic.
 */
#define QSFP_NUM_REGS 2
#define QSFP_49_52_MASK 0x0f
#define QSFP_53_54_MASK 0x03
#define QSFP_PRESENT_IDX 0
#define QSFP_RESET_IDX 1

struct reg_data {
	uint32_t reg;
	uint8_t mask;
};

struct qsfp_status {
	char name[PLATFORM_LED_COLOR_NAME_SIZE];
	struct reg_data regs[QSFP_NUM_REGS];
	uint8_t active_low;
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
		.name = "present",
		.active_low = 1,
		.regs = {
			  { .reg = ACT7312_QSFP_49_52_PRESENT_REG,
			    .mask = QSFP_49_52_MASK,
			  },
			  { .reg = ACT7312_QSFP_53_54_PRESENT_REG,
			    .mask = QSFP_53_54_MASK,
			  },
			}
	},
	{
		.name = "reset",
		.active_low = 1,
		.regs = {
			  { .reg = ACT7312_QSFP_49_52_RESET_REG,
			    .mask = QSFP_49_52_MASK,
			  },
			  { .reg = ACT7312_QSFP_53_54_RESET_REG,
			    .mask = QSFP_53_54_MASK,
			  },
			}
	},
};


static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	int i;
	s32 read_data, val = 0;
	struct qsfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < QSFP_NUM_REGS; i++) {
		read_data = cpld_reg_read(target->regs[i].reg);
		if (read_data < 0)
			return read_data;
		if (target->active_low)
			read_data = ~read_data;
		val |= ((read_data) & (target->regs[i].mask)) << (4 * i);
	}

	return sprintf(buf, "0x%x\n", (u8)val);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int i, retval;
	unsigned int val, write_val;
	struct qsfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	retval = kstrtouint(buf, 0, &val);
	if (retval != 0)
		return retval;

	for (i = 0; i < QSFP_NUM_REGS; i++) {
		write_val = ((val) & (target->regs[i].mask));
		if (target->active_low)
			write_val = ~write_val;
		retval = cpld_reg_write(target->regs[i].reg, (u8)write_val);
		if (retval < 0)
			return retval;
	}

	return count;
}
static SENSOR_DEVICE_ATTR_RO(qsfp_present, qsfp_show, QSFP_PRESENT_IDX);
static SENSOR_DEVICE_ATTR_RW(qsfp_reset, qsfp_show, qsfp_store, QSFP_RESET_IDX);

static struct attribute *act_as7312_cpld_attrs[] = {
	&dev_attr_cpld1_version.attr,
	&dev_attr_cpld2_version.attr,
	&dev_attr_cpld3_version.attr,
	&dev_attr_cpld4_version.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_fan1_input.attr,
	&dev_attr_fan2_input.attr,
	&dev_attr_fan3_input.attr,
	&dev_attr_fan4_input.attr,
	&dev_attr_fan5_input.attr,
	&dev_attr_fan6_input.attr,
	&dev_attr_fan7_input.attr,
	&dev_attr_fan8_input.attr,
	&dev_attr_fan9_input.attr,
	&dev_attr_fan10_input.attr,
	&dev_attr_fan11_input.attr,
	&dev_attr_fan12_input.attr,
	&dev_attr_fan1_ok.attr,
	&dev_attr_fan2_ok.attr,
	&dev_attr_fan3_ok.attr,
	&dev_attr_fan4_ok.attr,
	&dev_attr_fan5_ok.attr,
	&dev_attr_fan6_ok.attr,
	&dev_attr_fan7_ok.attr,
	&dev_attr_fan8_ok.attr,
	&dev_attr_fan9_ok.attr,
	&dev_attr_fan10_ok.attr,
	&dev_attr_fan11_ok.attr,
	&dev_attr_fan12_ok.attr,
	&dev_attr_pwm1.attr,
	&dev_attr_pwm2.attr,
	&dev_attr_pwm3.attr,
	&dev_attr_pwm4.attr,
	&dev_attr_pwm5.attr,
	&dev_attr_pwm6.attr,
	&dev_attr_fan_watchdog_enable.attr,
	&dev_attr_led_loc.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_port_leds_rgb.attr,
	&dev_attr_sfp_present.attr,
	&dev_attr_sfp_tx_fault.attr,
	&dev_attr_sfp_tx_disable.attr,
	&dev_attr_sfp_rx_loss.attr,
	&sensor_dev_attr_qsfp_present.dev_attr.attr,
	&sensor_dev_attr_qsfp_reset.dev_attr.attr,
	NULL,
};

static struct attribute_group act_as7312_cpld_attr_group = {
	.attrs = act_as7312_cpld_attrs,
};

static int act_as7312_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &act_as7312_cpld_attr_group);
	if (ret)
		pr_err("sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int act_as7312_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver act_as7312_cpld_driver = {
	.driver = {
		.name = CPLD_NAME,
		.owner = THIS_MODULE,
	},
	.probe = act_as7312_cpld_probe,
	.remove = act_as7312_cpld_remove,
};

static int __init act_as7312_cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&act_as7312_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for CPLD device");
		goto err_drvr;
	}

	act_as7312_cpld_device = platform_device_alloc(CPLD_NAME, 0);
	if (!act_as7312_cpld_device) {
		pr_err("platform_device_alloc() failed for CPLD device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(act_as7312_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for CPLD device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(act_as7312_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&act_as7312_cpld_driver);

err_drvr:
	return ret;
}

static void __exit act_as7312_cpld_exit(void)
{
}

static int __init
act_as7312_platform_init(void)
{
	int ret = 0;

	ret = act_as7312_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = act_as7312_cpld_init();
	if (ret) {
		pr_err("Initializing CPLD subsystem failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit
act_as7312_platform_exit(void)
{
	act_as7312_cpld_exit();
	act_as7312_i2c_exit();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(act_as7312_platform_init);
module_exit(act_as7312_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS7312-54X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/*
 * dell_s4000_platform.c - DELL S4000-C2338 Platform Support.
 *
 * copyright (C) 2014,2015,2016,2019 Cumulus Networks, Inc. All Rights Reserved
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
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>


#include "platform-defs.h"
#include "dell-s4000-cpld.h"

#define DRIVER_NAME        "dell_s4000"
#define DRIVER_VERSION     "1.0"


/*
 * This platform has two i2c busses:
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 *  SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 */

/* i2c bus adapter numbers for the down stream i2c busses */
enum {
	DELL_S4000_I2C_iSMT_BUS=0,
	DELL_S4000_I2C_I801_BUS,
	DELL_S4000_I2C_MUX1_BUS0=10,
	DELL_S4000_I2C_MUX1_BUS1,
	DELL_S4000_I2C_MUX1_BUS2,
	DELL_S4000_I2C_MUX1_BUS3,
	DELL_S4000_I2C_MUX1_BUS4,
	DELL_S4000_I2C_MUX1_BUS5,
	DELL_S4000_I2C_MUX1_BUS6,
	DELL_S4000_I2C_MUX1_BUS7,
	DELL_S4000_I2C_SFP_MUX_PORT_1 = 21,
	DELL_S4000_I2C_QSFP_MUX_PORT_49 = 69,
};

static struct i2c_client *dell_s4000_cpld_i2c_client_list[NUM_CPLD_I2C_CLIENTS];
static int num_cpld_i2c_devices;

mk_eeprom(spd,   52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,  50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan1,  51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,  52, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,  53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode mux1_platform_modes[] = {
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S4000_I2C_MUX1_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux1_platform_data = {
	.modes = mux1_platform_modes,
	.num_modes = ARRAY_SIZE(mux1_platform_modes),
};

struct dell_s4000_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct dell_s4000_i2c_device_info dell_s4000_i2c_devices[] = {
	{
		.bus = DELL_S4000_I2C_iSMT_BUS,
		{
			.type = "pca9547",
			.addr = 0x70,
			.platform_data = &mux1_platform_data,
		}
	},
	{
		.bus = DELL_S4000_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x52),         /* DIMM */
			.platform_data = &spd_52_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS0,
		{
			I2C_BOARD_INFO("24c02", 0x53),       /* 256-byte Board EEPROM */
			.platform_data = &board_53_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS0,
		{
			I2C_BOARD_INFO("dummy", 0x31),       /* System CPLD */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS0,
		{
			I2C_BOARD_INFO("dummy", 0x32),       /* Master CPLD */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS0,
		{
			I2C_BOARD_INFO("dummy", 0x33),       /* Slave CPLD */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS0,
		{
			I2C_BOARD_INFO("tmp75", 0x4d),       /* EMC1428 Temperature Sensor */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("tmp75", 0x4c),       /* TMP75 Temperature Sensor */
		},
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("tmp75", 0x4d),       /* TMP75 Temperature Sensor */
		},
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("tmp75", 0x4e),       /* TMP75 Temperature Sensor */
		},
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("max6620", 0x29),     /* MAX6620 Fan controller */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("max6620", 0x2a),     /* MAX6620 Fan controller */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("24c02", 0x51),       /* Fan 1 EEPROM */
			.platform_data = &fan1_51_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("24c02", 0x52),       /* Fan 2 EEPROM */
			.platform_data = &fan2_52_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("24c02", 0x53),       /* Fan 3 EEPROM */
			.platform_data = &fan3_53_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("ltc4215", 0x40),     /* LTC4215 PSU 2 monitor */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("ltc4215", 0x42),     /* LTC4215 PSU 1 monitor */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS4,
		{
			I2C_BOARD_INFO("dps460", 0x58),  /* PSU 2 PMBUS  */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS4,
		{
			I2C_BOARD_INFO("dps460", 0x59),  /* PSU 1 PMBUS  */
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS4,
		{
			I2C_BOARD_INFO("24c02", 0x51),       /* PSU 1 EEPROM */
			.platform_data = &psu1_51_at24,
		}
	},
	{
		.bus = DELL_S4000_I2C_MUX1_BUS4,
		{
			I2C_BOARD_INFO("24c02", 0x50),       /* PSU 2 EEPROM */
			.platform_data = &psu2_50_at24,
		}
	},
};


static int __init dell_s4000_cpld_init(void);
static void __exit dell_s4000_cpld_exit(void);

static int __init dell_s4000_port_mux_init(void);
static void __exit dell_s4000_port_mux_exit(void);

/**
 * dell_s4000_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
static struct i2c_client *dell_s4000_clients_list[ARRAY_SIZE(dell_s4000_i2c_devices)];

static void free_i2c_clients(struct i2c_client **clients_list, int num_clients)
{
	int i, idx;

	for (i = num_clients; i; i--) {
		idx = i - 1;
		if (clients_list[idx])
			i2c_unregister_device(clients_list[idx]);
	}
}

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

static void free_dell_s4000_i2c_data(void)
{
	free_i2c_clients(dell_s4000_clients_list, ARRAY_SIZE(dell_s4000_i2c_devices));
}

static struct i2c_client *add_i2c_client(int bus, struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err(DRIVER_NAME ": could not get adapter %u\n", bus);
		client = ERR_PTR(-ENODEV);
		goto exit;
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err(DRIVER_NAME ": could not add device\n");
		client = ERR_PTR(-ENODEV);
	}
	i2c_put_adapter(adapter);
exit:
	return client;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < DELL_S4000_I2C_MUX1_BUS0; i++) {
		adapter = get_adapter(i);
		if (adapter) {
			if (strncmp(adapter->name, name, strlen(name)) == 0) {
				i2c_put_adapter(adapter);
				return i;
			}
			i2c_put_adapter(adapter);
		}
	}
	return -1;
}

static int populate_i2c_devices(struct dell_s4000_i2c_device_info *devices,
                                int num_devices, struct i2c_client **clients_list,
                                int iSMT_bus, int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	num_cpld_i2c_devices = 0;
	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == DELL_S4000_I2C_iSMT_BUS) {
			devices[i].bus = iSMT_bus;
		} else if (devices[i].bus == DELL_S4000_I2C_I801_BUS) {
			devices[i].bus = I801_bus;
		}
		client = add_i2c_client(devices[i].bus, &devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
		if (strcmp(devices[i].board_info.type, "dummy") == 0 && num_cpld_i2c_devices < NUM_CPLD_I2C_CLIENTS) {
			dell_s4000_cpld_i2c_client_list[num_cpld_i2c_devices] = client;
			num_cpld_i2c_devices++;
		}
	}
	return 0;

err_exit:
	return ret;
}

static int __init dell_s4000_i2c_init(void)
{
	int iSMT_bus;
	int I801_bus;
	int ret;

	ret = -1;
	iSMT_bus = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (iSMT_bus < 0) {
		pr_err(DRIVER_NAME ": could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	I801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		pr_err(DRIVER_NAME ": could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/* populate the i2c devices
	*/
	ret = populate_i2c_devices(dell_s4000_i2c_devices, ARRAY_SIZE(dell_s4000_i2c_devices),
	                           &dell_s4000_clients_list[0], iSMT_bus, I801_bus);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_dell_s4000_i2c_data();
	return ret;
}

static void __exit dell_s4000_i2c_exit(void)
{
	free_dell_s4000_i2c_data();
}


/*---------------------------------------------------------------------
 *
 * CPLD SFP/QSFP mux
 *
 * Two muxes;
 *   'cpld-sfp-mux'  for sfp ports 1-48
 *   'cpld-qsfp-mux' for qsfp ports 49-55
 */
#define NUM_PORT_MUXES             (2)
#define FIRST_SFP_PORT_NUM         (1)
#define NUM_SFP_PORTS              (48)
#define FIRST_QSFP_PORT_NUM        (49)
#define NUM_QSFP_PORTS             (6)

struct port_mux_info_item {
	int num_items;
	struct platform_device *mux_dev;
	struct i2c_mux_core *muxc;
	struct i2c_client **mux_clients;
	struct i2c_board_info **mux_board_infos;
};

struct port_mux_info_struct {
	struct port_mux_info_item mux_item[NUM_PORT_MUXES];
	int num_port_muxes;
};

static struct port_mux_info_struct *port_mux_info;

/**
 * Invalid QSFP mux number
 *
 */
#define QSFP_MUX_INVALID_CHANNEL (0xDEADBEEF)

static void free_port_mux_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static void port_mux_info_free_data(void)
{
	int i, j;
	struct i2c_mux_core *muxc;

	if (port_mux_info) {
		for (i = 0; i < port_mux_info->num_port_muxes; i++) {
			for (j = 0; j < port_mux_info->mux_item[i].num_items; j++) {
				if (port_mux_info->mux_item[i].mux_clients[j])
					i2c_unregister_device(port_mux_info->mux_item[i].mux_clients[j]);
				if (port_mux_info->mux_item[i].mux_board_infos[j])
					free_port_mux_board_info(port_mux_info->mux_item[i].mux_board_infos[j]);
			}
			muxc = port_mux_info->mux_item[i].muxc;
			if (muxc)
				i2c_mux_del_adapters(muxc);
			kfree(port_mux_info->mux_item[i].mux_clients);
			kfree(port_mux_info->mux_item[i].mux_board_infos);
			platform_device_unregister(port_mux_info->mux_item[i].mux_dev);
		}
		kfree(port_mux_info);
		port_mux_info = NULL;
	}
}

static bool is_qsfp_port(int port) {
	if (port >= 49)
		return true;
	else
		return false;
}

#define PORT_LABEL_SIZE  8
static struct i2c_board_info *alloc_port_mux_board_info(int port) {
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;

	label = kzalloc(PORT_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, PORT_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	if (is_qsfp_port(port)) {
		struct sff_8436_platform_data *sff8436_data;
		sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
		if (!sff8436_data)
			goto err_kzalloc;

		sff8436_data->byte_len = 256;
		sff8436_data->flags = SFF_8436_FLAG_IRUGO;
		sff8436_data->page_size = 1;
		sff8436_data->eeprom_data = eeprom_data;
		board_info->platform_data = sff8436_data;
		strcpy(board_info->type, "sff8436");
	} else {
		struct at24_platform_data *at24_data;
		at24_data = kzalloc(sizeof(struct at24_platform_data), GFP_KERNEL);
		if (!at24_data)
			goto err_kzalloc;

		at24_data->byte_len = 512;
		at24_data->flags = AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG;
		at24_data->page_size = 1;
		at24_data->eeprom_data = eeprom_data;
		board_info->platform_data = at24_data;
		strcpy(board_info->type, "24c04");
	}

	board_info->addr = 0x50;

	return board_info;

err_kzalloc:
	kfree(board_info);
	kfree(eeprom_data);
	kfree(label);
	return NULL;
}

static struct i2c_adapter *mux1_bus0_adap;

static int qsfp_mux_i2c_reg_write(uint8_t reg, u8 val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;

	adap = mux1_bus0_adap;
	client = dell_s4000_cpld_i2c_client_list[GET_CPLD_IDX(reg)];
	reg = STRIP_CPLD_IDX(reg);

	if (adap->algo->master_xfer) {
		struct i2c_msg msg = { 0 };
		char buf[2];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2;
		buf[0] = reg;
		buf[1] = val;
		msg.buf = buf;
		rv = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

		data.byte = val;
		rv = adap->algo->smbus_xfer(adap, client->addr,
		                            client->flags,
		                            I2C_SMBUS_WRITE,
		                            reg, I2C_SMBUS_BYTE_DATA, &data);
	}
	return rv;
}

static int cpld_qsfp_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	static u32 prev_chan = QSFP_MUX_INVALID_CHANNEL;
	uint8_t val;
	int ret;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;
	val = ~(1 << (chan - DELL_S4000_I2C_QSFP_MUX_PORT_49));
	ret = qsfp_mux_i2c_reg_write(CPLD_QSFP_49_54_MUX_SELECT_REG, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int cpld_sfp_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	static u32 prev_chan = QSFP_MUX_INVALID_CHANNEL;
	uint8_t val;
	int ret;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;
	val = chan - DELL_S4000_I2C_SFP_MUX_PORT_1 + 1;
	ret = qsfp_mux_i2c_reg_write(CPLD_SFP_1_48_MUX_SELECT_REG, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int create_port_mux(int bus, int num_mux_clients, int first_port_num, int first_mux_num,
                           const char *mux_name,
			   int (select_function(struct i2c_mux_core *, u32)))
{
	struct i2c_adapter *adapter;
	struct port_mux_info_item *mux_item;
	int ret;
	int i;

	adapter = i2c_get_adapter(bus);
	if (!adapter) {
		pr_err(DRIVER_NAME
		       ": Could not find i2c adapter for %s bus %d.\n",
		       mux_name, bus);
		return -ENODEV;
	}
	mux_item = &port_mux_info->mux_item[port_mux_info->num_port_muxes];

	mux_item->mux_clients = kzalloc(num_mux_clients * sizeof(struct i2c_client *), GFP_KERNEL);
	if (!mux_item->mux_clients) {
		goto err_exit;
	}

	mux_item->mux_board_infos = kzalloc(num_mux_clients * sizeof(struct qsfp_board_info *), GFP_KERNEL);
	if (!mux_item->mux_board_infos) {
		goto err_exit_clients;
	}
	mux_item->num_items = num_mux_clients;

	mux_item->mux_dev = platform_device_alloc(mux_name, 0);
	if (!mux_item->mux_dev) {
		pr_err(DRIVER_NAME
		       ": platform_device_alloc() failed for %s.\n",
		       mux_name);
		goto err_exit_infos;
	}

	ret = platform_device_add(mux_item->mux_dev);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": platform_device_add() failed for %s.\n",
		       mux_name);
		goto err_exit_device;
	}
	mux_item->muxc = i2c_mux_alloc(adapter, &mux_item->mux_dev->dev,
				       num_mux_clients, 0, 0,
				       select_function, NULL);
	if (!mux_item->muxc) {
		pr_err(DRIVER_NAME ": i2c_mux_alloc() failed for port mux.\n");
		ret = -ENOMEM;
		goto err_exit;
	}
	port_mux_info->num_port_muxes++;

	for (i = 0; i < num_mux_clients; i++) {
		struct i2c_board_info *eeprom_info;
		struct i2c_client *client;
		int mux_num;

		mux_num = first_mux_num + i;
		ret = i2c_mux_add_adapter(mux_item->muxc, mux_num, mux_num, 0);
		if (ret) {
			pr_err(DRIVER_NAME
			       ": i2c_mux_add_adapter() failed for channel %u.\n",
			       mux_num);
			goto err_exit;
		}

		eeprom_info = alloc_port_mux_board_info(first_port_num + i);
		if (!eeprom_info) {
			goto err_exit;
		}

		mux_item->mux_board_infos[i] = eeprom_info;
		client = i2c_new_device(mux_item->muxc->adapter[i],
					eeprom_info);
		if (!client) {
			pr_err(DRIVER_NAME
			       ": i2c_new_device failed for %s device.\n",
			       mux_name);
			goto err_exit;
		}
		mux_item->mux_clients[i] = client;
	}

	i2c_put_adapter(adapter);
	return 0;

err_exit_device:
	platform_device_put(mux_item->mux_dev);

err_exit_infos:
	kfree(mux_item->mux_board_infos);

err_exit_clients:
	kfree(mux_item->mux_clients);
err_exit:
	port_mux_info_free_data();
	return -ENOMEM;
}

static int __init dell_s4000_port_mux_init(void)
{
	int ret;

	port_mux_info = kzalloc(sizeof(struct port_mux_info_struct), GFP_KERNEL);
	if (!port_mux_info) {
		return -ENOMEM;
	}

	mux1_bus0_adap = i2c_get_adapter(DELL_S4000_I2C_MUX1_BUS0);
	ret = create_port_mux(DELL_S4000_I2C_MUX1_BUS2, NUM_SFP_PORTS,
	                      FIRST_SFP_PORT_NUM, DELL_S4000_I2C_SFP_MUX_PORT_1,
	                      "cpld-sfp-mux",
	                      cpld_sfp_mux_select_chan);
	if (ret)
		goto exit;

	ret = create_port_mux(DELL_S4000_I2C_MUX1_BUS3, NUM_QSFP_PORTS,
	                      FIRST_QSFP_PORT_NUM, DELL_S4000_I2C_QSFP_MUX_PORT_49,
	                      "cpld-qsfp-mux",
	                      cpld_qsfp_mux_select_chan);
	if (ret)
		goto exit;

exit:
	i2c_put_adapter(mux1_bus0_adap);
	return ret;
}

static void __exit dell_s4000_port_mux_exit(void)
{
	port_mux_info_free_data();
}

/*---------------------------------------------------------------------
 *
 * CPLD
 */
static uint8_t cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	uint8_t val;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err(DRIVER_NAME
		       ": attempt to read invalid CPLD register [0x%02X]",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	val = i2c_smbus_read_byte_data(dell_s4000_cpld_i2c_client_list[cpld_idx], reg);
	if (val < 0) {
		pr_err(DRIVER_NAME
		       ": I2C read error - addr: 0x%02X, offset: 0x%02X",
		       dell_s4000_cpld_i2c_client_list[cpld_idx]->addr,
		       STRIP_CPLD_IDX(reg));
	}
	return val;
}

static int cpld_reg_write(uint32_t reg, uint8_t write_val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	struct i2c_client *cpld_client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err(DRIVER_NAME
		       ": attempt to write to invalid CPLD register [0x%02X]",
		       reg);
		return -EINVAL;
	}
	cpld_client = dell_s4000_cpld_i2c_client_list[cpld_idx];
	res = i2c_smbus_write_byte_data(cpld_client,
					STRIP_CPLD_IDX(reg),
					write_val);
	if (res) {
		pr_err(DRIVER_NAME
		       ": i2c write failed addr: 0x%02X, reg: 0x%02X, val: 0x%02X",
		       dell_s4000_cpld_i2c_client_list[cpld_idx]->addr,
		       STRIP_CPLD_IDX(reg), write_val);
	}
	return res;
}

static ssize_t bulk_power_show(struct device * dev,
                               struct device_attribute * dattr,
                               char * buf)
{
	uint8_t read_val;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok;
	uint8_t error;

	read_val = cpld_reg_read(CPLD_POWER_SUPPLY_STATUS_REG);
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask      = CPLD_PSU1_MASK;
		present_l = CPLD_PSU1_PRESENT_L;
		pwr_ok    = CPLD_PSU1_GOOD;
		error     = CPLD_PSU1_ERROR;
	} else {
		mask      = CPLD_PSU2_MASK;
		present_l = CPLD_PSU2_PRESENT_L;
		pwr_ok    = CPLD_PSU2_GOOD;
		error     = CPLD_PSU2_ERROR;
	}
	read_val &= mask;

	if (~read_val & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if ( !(read_val & pwr_ok) || ( read_val & error ) ) {
			strcat(buf, ", "PLATFORM_PS_POWER_BAD);
		} else {
			strcat(buf, ", "PLATFORM_OK);
		}
	} else {
		// Not present
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);

static ssize_t cpld_revision_show(struct device * dev,
                                  struct device_attribute * dattr,
                                  char * buf)
{
	return sprintf(buf, "System: 0x%02X, Master: 0x%02X, Slave: 0x%02X\n",
			cpld_reg_read(CPLD_HW_REV_REG),
			cpld_reg_read(CPLD_BOARD_REV_REG),
			cpld_reg_read(CPLD_SLAVE_REV_REG));
}
static SYSFS_ATTR_RO(cpld_revision, cpld_revision_show);

static ssize_t fan_show(struct device * dev,
                        struct device_attribute * dattr,
                        char *buf)
{
	uint8_t data;
	uint8_t present_l = CPLD_FAN_TRAY_0_PRESENT_L;
	int reg = CPLD_LED_CONTROL_2_REG;

	if (strcmp(dattr->attr.name, "fan_3") == 0) {
		/*
		 * we cannot safely read the register for fan_3
		 * status (CM-4426)
		 */
		present_l = CPLD_FAN_TRAY_2_PRESENT_L;
		reg = CPLD_FAN_STATUS_REG;
	}
	if (strcmp(dattr->attr.name, "fan_2") == 0) {
		present_l = CPLD_FAN_TRAY_1_PRESENT_L;
	}

	data = cpld_reg_read(reg);
	if (~data & present_l) {
		return sprintf(buf, PLATFORM_OK "\n");
	}
	return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
}
static SYSFS_ATTR_RO(fan_1, fan_show);
static SYSFS_ATTR_RO(fan_2, fan_show);
static SYSFS_ATTR_RO(fan_3, fan_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DELL_S4000_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_power",
		.offset = CPLD_LED_CONTROL_1_REG,
		.mask = CPLD_POWER_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_OFF, CPLD_POWER_LED_OFF },
			{ PLATFORM_LED_AMBER, CPLD_POWER_LED_AMBER },
			{ PLATFORM_LED_AMBER_BLINKING, CPLD_POWER_LED_AMBER_BLINK },
			{ PLATFORM_LED_GREEN, CPLD_POWER_LED_GREEN },
		},
	},
	{
		.name = "led_fan",
		.offset = CPLD_FAN_STATUS_REG,
		.mask = CPLD_FRONT_FAN_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FRONT_FAN_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FRONT_FAN_LED_YELLOW },
			{ PLATFORM_LED_YELLOW_BLINKING, CPLD_FRONT_FAN_LED_YELLOW_BLINK },
			{ PLATFORM_LED_OFF, CPLD_FRONT_FAN_LED_OFF },
		},
	},
	{
		.name = "led_health",
		.offset = CPLD_LED_CONTROL_1_REG,
		.mask = CPLD_HEALTH_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN_BLINKING, CPLD_HEALTH_LED_GREEN_BLINK },
			{ PLATFORM_LED_GREEN, CPLD_HEALTH_LED_GREEN },
			{ PLATFORM_LED_AMBER, CPLD_HEALTH_LED_AMBER },
			{ PLATFORM_LED_AMBER_BLINKING, CPLD_HEALTH_LED_AMBER_BLINK },
		},
	},
	{
		.name = "led_locator",
		.offset = CPLD_LED_CONTROL_1_REG,
		.mask = CPLD_LOCATED_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_BLUE, CPLD_LOCATED_LED_BLUE },
			{ PLATFORM_LED_BLUE_BLINKING, CPLD_LOCATED_LED_BLUE_BLINK },
			{ PLATFORM_LED_OFF, CPLD_LOCATED_LED_OFF },
		},
	},
	{
		.name = "led_master",
		.offset = CPLD_LED_CONTROL_1_REG,
		.mask = CPLD_MASTER_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_MASTER_LED_GREEN },
			{ PLATFORM_LED_OFF, CPLD_MASTER_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_1",
		.offset = CPLD_LED_CONTROL_2_REG,
		.mask = CPLD_FAN_TRAY_0_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_0_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_0_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_0_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_2",
		.offset = CPLD_LED_CONTROL_2_REG,
		.mask = CPLD_FAN_TRAY_1_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_1_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_1_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_1_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_3",
		.offset = CPLD_LED_CONTROL_2_REG,
		.mask = CPLD_FAN_TRAY_2_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_2_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_2_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_2_LED_OFF },
		},
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * LEDs
 */
static ssize_t led_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	tmp = cpld_reg_read(target->offset);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined\n");
	} else {
		return sprintf(buf, "%s\n", target->colors[i].name);
	}
}

static ssize_t led_store(struct device * dev,
                         struct device_attribute * dattr,
                         const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	int ret;
	struct led * target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}
	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0) {
			break;
		}
	}
	if (i == target->n_colors) {
		return -EINVAL;
	}

	/* set the new value */
	tmp = cpld_reg_read(target->offset);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;

	ret = cpld_reg_write(target->offset, tmp);
	if (ret < 0)
		return ret;

	return count;
}

static SYSFS_ATTR_RW(led_power, led_show, led_store);
static SYSFS_ATTR_RW(led_fan, led_show, led_store);
static SYSFS_ATTR_RW(led_health, led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_locator, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_1, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_2, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_3, led_show, led_store);

/*------------------------------------------------------------------------------
 *
 * SFP definitions
 *
 */
#define DELL_S4000_CPLD_SFP_REG_COUNT     (6)
struct sfp_status {
	char name[DELL_S4000_CPLD_STRING_NAME_SIZE];
	uint8_t regs[DELL_S4000_CPLD_SFP_REG_COUNT];
	uint8_t active_low;
};

static struct sfp_status cpld_sfp_status[] = {
	{
		.name = "present",
		.active_low = true,
		.regs = { CPLD_SFP_PORT_1_8_PRESENT_REG,
		          CPLD_SFP_PORT_9_16_PRESENT_REG,
		          CPLD_SFP_PORT_17_24_PRESENT_REG,
		          CPLD_SFP_PORT_25_32_PRESENT_REG,
		          CPLD_SFP_PORT_33_40_PRESENT_REG,
		          CPLD_SFP_PORT_41_48_PRESENT_REG
		},
	},
	{
		.name = "tx_enable",
		.active_low = true,
		.regs = { CPLD_SFP_PORT_1_8_TX_DISABLE_REG,
		          CPLD_SFP_PORT_9_16_TX_DISABLE_REG,
		          CPLD_SFP_PORT_17_24_TX_DISABLE_REG,
		          CPLD_SFP_PORT_25_32_TX_DISABLE_REG,
		          CPLD_SFP_PORT_33_40_TX_DISABLE_REG,
		          CPLD_SFP_PORT_41_48_TX_DISABLE_REG
		},
	},
	{
		.name = "rx_los",
		.regs = { CPLD_SFP_PORT_1_8_RX_LOS_REG,
		          CPLD_SFP_PORT_9_16_RX_LOS_REG,
		          CPLD_SFP_PORT_17_24_RX_LOS_REG,
		          CPLD_SFP_PORT_25_32_RX_LOS_REG,
		          CPLD_SFP_PORT_33_40_RX_LOS_REG,
		          CPLD_SFP_PORT_41_48_RX_LOS_REG
		},
	},
	{
		.name = "tx_fault",
		.regs = { CPLD_SFP_PORT_1_8_TX_FAULT_REG,
		          CPLD_SFP_PORT_9_16_TX_FAULT_REG,
		          CPLD_SFP_PORT_17_24_TX_FAULT_REG,
		          CPLD_SFP_PORT_25_32_TX_FAULT_REG,
		          CPLD_SFP_PORT_33_40_TX_FAULT_REG,
		          CPLD_SFP_PORT_41_48_TX_FAULT_REG
		},
	},
};
static int n_sfp_status = ARRAY_SIZE(cpld_sfp_status);

static ssize_t sfp_show(struct device *dev,
                        struct device_attribute *dattr,
                        char *buf)
{
	int i;
	uint8_t name_len = 4; /* strlen("sfp_"); */
	uint8_t data;
	struct sfp_status *target = NULL;

	for (i = 0; i < n_sfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_sfp_status[i].name) == 0) {
			target = &cpld_sfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	strcpy(buf, "0x");
	for (i = DELL_S4000_CPLD_SFP_REG_COUNT; i; i--) {
		data = cpld_reg_read(target->regs[i - 1]);
		if (target->active_low)
			data = ~data;
		sprintf(buf + strlen(buf), "%02X", data);
	}
	strcpy(buf + strlen(buf), "\n");

	return strlen(buf);
}

static int hexToInt64(const char *hex_str, uint64_t *val)
{
	char prefix[] = "0x";
	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0) {
		hex_str += strlen(prefix);
	}
	return sscanf(hex_str, "%llx", val) != 1;
}

static ssize_t sfp_tx_enable_store(struct device *dev,
                         struct device_attribute *dattr,
                         const char *buf, size_t count)
{
	int ret, i;
	uint64_t enable_mask;
	uint8_t val, reg;

	if (hexToInt64(buf, &enable_mask))
		return -EINVAL;

	/* 48-bit tx_enable register is active low */
	enable_mask = ~enable_mask & ((1LL << 48) - 1);

	/* Disassemble the 48-bit mask into 6 8-bit register values */
	for (i = 0; i < DELL_S4000_CPLD_SFP_REG_COUNT; i++) {
		reg = CPLD_SFP_PORT_1_8_TX_DISABLE_REG + i;
		val = (enable_mask >> (i * 8)) & 0xFF;
		ret = cpld_reg_write(reg, val);
		if (ret < 0)
			return ret;
	}

	return count;
}

SYSFS_ATTR_RO(sfp_present, sfp_show);
SYSFS_ATTR_RW(sfp_tx_enable, sfp_show, sfp_tx_enable_store);
SYSFS_ATTR_RO(sfp_rx_los, sfp_show);
SYSFS_ATTR_RO(sfp_tx_fault, sfp_show);

/*------------------------------------------------------------------------------
 *
 * QSFP definitions
 *
 */
struct qsfp_status {
	char name[DELL_S4000_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t active_low;
	uint8_t swap_bits;
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
		.name = "present",
		.reg = CPLD_QSFP_49_54_PRESENT_REG,
		.active_low = 1,
	},
	{
		.name = "interrupt",
		.reg = CPLD_QSFP_49_54_INTERRUPT_REG,
		.active_low = 1,
	},
	{
		.name = "lp_mode",
		.reg = CPLD_QSFP_49_54_LP_MODE_REG,
		// lp mode bits need to be swapped, ie. bits 0 - 5 are:
		// swp50, swp49, swp52, swp51, swp54, swp53
		.swap_bits = 1,
	},
	{
		.name = "reset",
		.reg = CPLD_QSFP_49_54_RESET_REG,
		.active_low = 1,
	},
	{
		.name = "i2c_enable",
		.reg = CPLD_QSFP_49_54_MUX_SELECT_REG,
		.active_low = 1,
	},
};
static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	int i;
	s32 val;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	struct qsfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	val = cpld_reg_read(target->reg);
	if (target->active_low)
		val = ~val;
	if (target->swap_bits)
		val = ((val & 0xaa) >> 1 | (val & 0x55) << 1);

	return sprintf(buf, "0x%02x\n", val & 0x3f);

}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int ret, i;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	uint64_t val64;
	uint8_t val;
	struct qsfp_status *target = NULL;

	if (hexToInt64(buf, &val64))
		return -EINVAL;

	/* Only the lower 6 bits are valid */
	if (val64 > 0x3f)
		return -EINVAL;

	val = val64 & 0x3f;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;
	if (target->swap_bits)
		val = ((val & 0xaa) >> 1 | (val & 0x55) << 1);

	if (target->active_low)
		val = ~val & 0x3f;

	ret = cpld_reg_write(target->reg, val);
	if (ret < 0)
		return ret;

	return count;
}

SYSFS_ATTR_RW(qsfp_reset, qsfp_show, qsfp_store);
SYSFS_ATTR_RW(qsfp_lp_mode, qsfp_show, qsfp_store);
SYSFS_ATTR_RO(qsfp_present, qsfp_show);
SYSFS_ATTR_RW(qsfp_i2c_enable, qsfp_show, qsfp_store);
SYSFS_ATTR_RO(qsfp_interrupt, qsfp_show);


static ssize_t shutdown_mode_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	s32 val = cpld_reg_read(CPLD_SYSTEM_FEATURE);

	if (val < 0)
		return val;
	val &= CPLD_SYSTEM_FEATURE_SHUTDOWN;
	return sprintf(buf, "%d\n", val);
}

static ssize_t shutdown_mode_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	int ret;
	long val;

	ret = kstrtol(buf, 0, &val);
	if (ret < 0)
		return ret;

	if ((val != 0) && (val != 1))
		return -EINVAL;

	ret = cpld_reg_read(CPLD_SYSTEM_FEATURE);
	if (ret < 0)
		return ret;

	if (val == 0)
		ret &= ~CPLD_SYSTEM_FEATURE_SHUTDOWN;
	else
		ret |= CPLD_SYSTEM_FEATURE_SHUTDOWN;

	ret = cpld_reg_write(CPLD_SYSTEM_FEATURE, ret);
	if (ret < 0)
		return ret;
	return count;
}
SYSFS_ATTR_RW(shutdown_mode, shutdown_mode_show, shutdown_mode_store);


static struct attribute *dell_s4000_cpld_attrs[] = {
	&dev_attr_cpld_revision.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_fan_1.attr,
	&dev_attr_fan_2.attr,
	&dev_attr_fan_3.attr,
	&dev_attr_led_power.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_health.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_locator.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_fan_tray_2.attr,
	&dev_attr_led_fan_tray_3.attr,
	&dev_attr_sfp_present.attr,
	&dev_attr_sfp_tx_enable.attr,
	&dev_attr_sfp_rx_los.attr,
	&dev_attr_sfp_tx_fault.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_interrupt.attr,
	&dev_attr_qsfp_i2c_enable.attr,
	&dev_attr_shutdown_mode.attr,
	NULL,
};

static struct attribute_group dell_s4000_cpld_attr_group = {
	.attrs = dell_s4000_cpld_attrs,
};

static int dell_s4000_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &dell_s4000_cpld_attr_group);
	if (ret)
		pr_err(DRIVER_NAME
		       ": sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int dell_s4000_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver dell_s4000_cpld_driver = {
	.driver = {
		.name = "dell_s4000_cpld",
		.owner = THIS_MODULE,
	},
	.probe = dell_s4000_cpld_probe,
	.remove = dell_s4000_cpld_remove,
};

static struct platform_device *dell_s4000_cpld_device;

static int __init dell_s4000_cpld_init(void)
{
	int ret;

	if (num_cpld_i2c_devices != NUM_CPLD_I2C_CLIENTS)
	{
		pr_err(DRIVER_NAME
		       ": cpld device unable to find i2c clients");
		ret = -ENODEV;
		goto err_drvr;
	}

	ret = platform_driver_register(&dell_s4000_cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	dell_s4000_cpld_device = platform_device_alloc("dell_s4000_cpld", 0);
	if (!dell_s4000_cpld_device) {
		pr_err(DRIVER_NAME
		       ": platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(dell_s4000_cpld_device);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}

	return 0;

err_dev_add:
	platform_device_put(dell_s4000_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&dell_s4000_cpld_driver);

err_drvr:
	return ret;
}

/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dell_s4000_init(void)
{
	int ret = 0;

	ret = dell_s4000_i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME ": Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = dell_s4000_cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME ": Registering CPLD driver failed.\n");
		return ret;
	}

	ret = dell_s4000_port_mux_init();
	if (ret) {
		pr_err(DRIVER_NAME ": Initializing SFP/QSFP mux failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version "
		DRIVER_VERSION " successfully loaded\n");
	return 0;
}

static void __exit dell_s4000_exit(void)
{
	dell_s4000_port_mux_exit();
	dell_s4000_cpld_exit();
	dell_s4000_i2c_exit();
	pr_err(DRIVER_NAME ": driver unloaded\n");
}

static void __exit dell_s4000_cpld_exit()
{
	platform_driver_unregister(&dell_s4000_cpld_driver);
	platform_device_unregister(dell_s4000_cpld_device);
}

module_init(dell_s4000_init);
module_exit(dell_s4000_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("DELL S4000 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

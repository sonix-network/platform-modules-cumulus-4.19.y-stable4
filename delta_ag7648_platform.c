/*
 *	delta_ag7648_platform.c - DNI AG7648 Platform Support.
 *
 *	Copyright (C) 2017 Cumulus Networks, Inc.
 *	Author: Nikhil Dhar (ndhar@cumulusnetworks.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *	General Public License for more details.
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
#include <linux/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/i2c/sff-8436.h>
#include <linux/i2c/pca954x.h>
#include <linux/hwmon-sysfs.h>

#include "platform_defs.h"
#include "delta_ag7648_cpld.h"

#define DRIVER_NAME		   "DNI_AG7648"
#define DRIVER_VERSION	   "0.1"

/*
 * This platform has two i2c busses:
 * SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 * SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 */

/* I2c bus adapter numbers for the down stream i2c busses */
enum {
	DNI_AG7648_I2C_ISMT_BUS = 0,
	DNI_AG7648_I2C_I801_BUS,
	DNI_AG7648_I2C_MUX1_BUS0 = 10,
	DNI_AG7648_I2C_MUX1_BUS1,
	DNI_AG7648_I2C_MUX1_BUS2,
	DNI_AG7648_I2C_MUX1_BUS3,
	DNI_AG7648_I2C_MUX1_BUS4,
	DNI_AG7648_I2C_MUX1_BUS5,
	DNI_AG7648_I2C_MUX1_BUS6,
	DNI_AG7648_I2C_MUX1_BUS7,
	DNI_AG7648_I2C_SFP_MUX_PORT = 21,
	DNI_AG7648_I2C_QSFP_MUX_PORT_49 = 69,
};

static struct i2c_client *ag7648_cpld_client_list[NO_CPLD_I2C_CLIENTS];
static int num_cpld_i2c_devices;

/* To-Do: Check eeprom sizes */
mk_eeprom(spd, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO |
		  AT24_FLAG_DISABLE_I2CBLOCK);
mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,  50, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1,  53, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan2,  52, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan3,  51, 256, AT24_FLAG_IRUGO);

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode mux1_platform_modes[] = {
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DNI_AG7648_I2C_MUX1_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux1_platform_data = {
	.modes = mux1_platform_modes,
	.num_modes = ARRAY_SIZE(mux1_platform_modes),
};

struct dni_ag7648_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct dni_ag7648_i2c_device_info ag7648_i2c_devices[] = {
	mk_i2cdev(DNI_AG7648_I2C_ISMT_BUS, "pca9547", 0x70,
		  &mux1_platform_data),
	mk_i2cdev(DNI_AG7648_I2C_I801_BUS, "spd", 0x52, &spd_52_at24),

	/* Mux1, Bus 0 */

	/* System eeprom */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS0, "24c02", 0x53, &board_53_at24),
	/* System CPLD */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS0, "dummy", 0x31, NULL),
	/* Master CPLD */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS0, "dummy", 0x32, NULL),
	/* Slave CPLD */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS0, "dummy", 0x33, NULL),
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS0, "tmp75", 0x4d, NULL),

	/* Mux1, Bus 1 */

	/* TMP75 Temp Sensor close to T2 */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "tmp75", 0x4c, NULL),
	/* TMP75 Temp Sensor close to sfp+ */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "tmp75", 0x4d, NULL),
	/* TMP75 Temp Sensor close to QSFP */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "tmp75", 0x4e, NULL),
	/* LTC4215 PSU 1 */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "ltc4215", 0x42, NULL),
	/* LTC4215 PSU 2 */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "ltc4215", 0x40, NULL),
	/* Fan 1 EEPROM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "24c02", 0x53, &fan1_53_at24),
	/* Fan 2 EEPROM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "24c02", 0x52, &fan2_52_at24),
	/* Fan 3 EEPROM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "24c02", 0x51, &fan3_51_at24),
	/* Fan controller 1 */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "max6620", 0x29, NULL),
	/* Fan controller 2 */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "max6620", 0x2a, NULL),
	/* AVS Power PWM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS1, "6764tr", 0x6f, NULL),

	/* Mux1, Bus 4 */

	/* PSU 1 PMBUS  */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS4, "dps460", 0x59, NULL),
	/* PSU 2 PMBUS  */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS4, "dps460", 0x58, NULL),
	/* PSU 1 EEPROM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS4, "24c02", 0x51, &psu1_51_at24),
	/* PSU 2 EEPROM */
	mk_i2cdev(DNI_AG7648_I2C_MUX1_BUS4, "24c02", 0x50, &psu2_50_at24),
};

static int __init dni_ag7648_cpld_init(void);
static void __exit dni_ag7648_cpld_exit(void);

static int __init dni_ag7648_port_mux_init(void);
static void __exit dni_ag7648_port_mux_exit(void);

/**
 * dni_ag7648_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
static struct i2c_client *ag7648_clients_list[ARRAY_SIZE(ag7648_i2c_devices)];

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
	int bail = 10;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;

		msleep(100);
	}
	return NULL;
}

static void free_dni_ag7648_i2c_data(void)
{
	free_i2c_clients(ag7648_clients_list,
			 ARRAY_SIZE(ag7648_i2c_devices));
}

static struct i2c_client *add_i2c_client(int bus,
					 struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err("could not get adapter %u\n", bus);
		client = ERR_PTR(-ENODEV);
		goto exit;
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err("could not add device\n");
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

	for (i = 0; i < DNI_AG7648_I2C_MUX1_BUS0; i++) {
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

static int populate_i2c_devices(struct dni_ag7648_i2c_device_info *devices,
				int num_devices,
				struct i2c_client **clients_list,
				int ISMT_bus, int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	num_cpld_i2c_devices = 0;
	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == DNI_AG7648_I2C_ISMT_BUS)
			devices[i].bus = ISMT_bus;
		else if (devices[i].bus == DNI_AG7648_I2C_I801_BUS)
			devices[i].bus = I801_bus;

		client = add_i2c_client(devices[i].bus, &devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
		if ((strcmp(devices[i].board_info.type, "dummy") == 0) &&
		    (num_cpld_i2c_devices < NO_CPLD_I2C_CLIENTS)) {
			ag7648_cpld_client_list[num_cpld_i2c_devices] = client;
			num_cpld_i2c_devices++;
		}
	}
	return 0;

err_exit:
	return ret;
}

static int __init dni_ag7648_i2c_init(void)
{
	int ISMT_bus;
	int I801_bus;
	int ret;

	ret = -1;
	ISMT_bus = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (ISMT_bus < 0) {
		pr_err("could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	I801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/* populate the i2c devices
	*/
	ret = populate_i2c_devices(ag7648_i2c_devices,
				   ARRAY_SIZE(ag7648_i2c_devices),
				   &ag7648_clients_list[0], ISMT_bus, I801_bus);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_dni_ag7648_i2c_data();
	return ret;
}

static void __exit dni_ag7648_i2c_exit(void)
{
	free_dni_ag7648_i2c_data();
}

/*---------------------------------------------------------------------
 *
 * CPLD SFP MUX
 *
 *	 'cpld-sfp-mux' for sfp ports 1-48
 */

#define NUM_PORT_MUXES			   (2)
#define FIRST_SFP_PORT_NUM		   (1)
#define NUM_SFP_PORTS			   (48)
#define FIRST_QSFP_PORT_NUM		   (49)
#define NUM_QSFP_PORTS			   (6)

struct port_mux_info_item {
	int num_items;
	struct platform_device *mux_dev;
	struct i2c_adapter **mux_adapters;
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
#define SFP_MUX_INVALID_CHANNEL (0xDEADBEEF)

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
	struct port_mux_info_struct *p = port_mux_info;

	if (p) {
		for (i = 0; i < p->num_port_muxes; i++) {
			for (j = 0; j < p->mux_item[i].num_items; j++) {
				if (p->mux_item[i].mux_clients[j])
					i2c_unregister_device(
						p->mux_item[i].mux_clients[j]);
			if (p->mux_item[i].mux_adapters[j])
				i2c_del_mux_adapter(
					p->mux_item[i].mux_adapters[j]);
			if (p->mux_item[i].mux_board_infos[j])
				free_port_mux_board_info(
					p->mux_item[i].mux_board_infos[j]);
			}
			kfree(p->mux_item[i].mux_clients);
			kfree(p->mux_item[i].mux_adapters);
			kfree(p->mux_item[i].mux_board_infos);
			platform_device_unregister(p->mux_item[i].mux_dev);
		}
		kfree(port_mux_info);
		port_mux_info = NULL;
	}
}

#define PORT_LABEL_SIZE  8
static struct i2c_board_info *alloc_port_mux_board_info(int port)
{
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;
	struct sff_8436_platform_data *sff8436_data;

	label = kzalloc(PORT_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, PORT_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data = kzalloc(sizeof(*sff8436_data),
			       GFP_KERNEL);
	if (!sff8436_data)
		goto err_kzalloc;

	sff8436_data->byte_len = 256;
	sff8436_data->flags = SFF_8436_FLAG_IRUGO;
	sff8436_data->page_size = 1;
	sff8436_data->eeprom_data = eeprom_data;
	board_info->platform_data = sff8436_data;
	strcpy(board_info->type, "sff8436");
	board_info->addr = 0x50;

	return board_info;

err_kzalloc:
	kfree(board_info);
	kfree(eeprom_data);
	kfree(label);
	return NULL;
}

static int sfp_mux_i2c_reg_write(uint8_t reg, u8 val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;

	adap = i2c_get_adapter(DNI_AG7648_I2C_MUX1_BUS0);
	client = ag7648_cpld_client_list[GET_CPLD_IDX(reg)];
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
						reg,
						I2C_SMBUS_BYTE_DATA, &data);
	}
	i2c_put_adapter(adap);
	return rv;
}

static int cpld_sfp_mux_select_chan(struct i2c_adapter *adapter, void *client,
				    u32 chan)
{
	static u32 prev_chan = SFP_MUX_INVALID_CHANNEL;
	uint8_t val;
	int ret = 0;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;
	val = (chan - DNI_AG7648_I2C_SFP_MUX_PORT) + 1;
	ret = sfp_mux_i2c_reg_write(CPLD_SFP_MUX_SELECT_REG, val);
	if (ret < 0)
		return ret;

	return ret;
}

static int cpld_qsfp_mux_select_chan(struct i2c_adapter *adapter, void *client,
				     u32 chan)
{
	static u32 prev_chan = SFP_MUX_INVALID_CHANNEL;
	uint8_t val;
	int ret;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;
	val = ~(1 << (chan - DNI_AG7648_I2C_QSFP_MUX_PORT_49));
	ret = sfp_mux_i2c_reg_write(CPLD_QSFP_MOD_SELECT_REG, val);
	if (ret < 0)
		return ret;

	return 0;
}

static int create_port_mux(int bus, int num_mux_clients, int first_port_num,
			   int first_mux_num, const char *mux_name,
			   int (select_function(struct i2c_adapter *,
						void *, u32)))
{
	struct i2c_adapter *adapter;
	struct port_mux_info_item *mux_item;
	int ret;
	int i;

	adapter = i2c_get_adapter(bus);
	if (!adapter) {
		pr_err("Could not find i2c adapter for %s bus %d.\n", mux_name,
		       bus);
		return -ENODEV;
	}

	mux_item = &port_mux_info->mux_item[port_mux_info->num_port_muxes];
	mux_item->mux_adapters = kzalloc(num_mux_clients *
					 sizeof(struct i2c_adapter *),
					 GFP_KERNEL);
	if (!mux_item->mux_adapters)
		goto err_exit;

	mux_item->mux_clients = kzalloc(num_mux_clients *
					sizeof(struct i2c_client *),
					GFP_KERNEL);
	if (!mux_item->mux_clients)
		goto err_exit_adapters;

	mux_item->mux_board_infos = kzalloc(num_mux_clients *
					    sizeof(struct qsfp_board_info *),
					    GFP_KERNEL);
	if (!mux_item->mux_board_infos)
		goto err_exit_clients;

	mux_item->num_items = num_mux_clients;

	mux_item->mux_dev = platform_device_alloc(mux_name, 0);
	if (!mux_item->mux_dev) {
		pr_err("platform_device_alloc() failed for %s.\n", mux_name);
		goto err_exit_infos;
	}

	ret = platform_device_add(mux_item->mux_dev);
	if (ret) {
		pr_err("platform_device_add() failed for %s.\n", mux_name);
		goto err_exit_device;
	}
	port_mux_info->num_port_muxes++;

	for (i = 0; i < num_mux_clients; i++) {
		struct i2c_adapter *virt_adap;
		struct i2c_board_info *eeprom_info;
		struct i2c_client *client;
		int mux_num;

		mux_num = first_mux_num + i;
		virt_adap = i2c_add_mux_adapter(adapter,
						&mux_item->mux_dev->dev, NULL,
						mux_num, mux_num, 0,
						select_function, NULL);
		if (!virt_adap) {
			pr_err("%s failed to add mux adapter for channel %u.\n",
			       mux_name, mux_num);
			goto err_exit;
		}

		mux_item->mux_adapters[i] = virt_adap;
		eeprom_info = alloc_port_mux_board_info(first_port_num + i);
		if (!eeprom_info)
			goto err_exit;

		mux_item->mux_board_infos[i] = eeprom_info;
		client = i2c_new_device(virt_adap, eeprom_info);
		if (!client) {
			pr_err("i2c_new_device failed for %s device.\n",
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

err_exit_adapters:
	kfree(mux_item->mux_adapters);

err_exit:
	port_mux_info_free_data();
	return -ENOMEM;
}

static int __init dni_ag7648_port_mux_init(void)
{
	int ret;

	port_mux_info = kzalloc(sizeof(*port_mux_info),
				GFP_KERNEL);
	if (!port_mux_info)
		return -ENOMEM;

	ret = create_port_mux(DNI_AG7648_I2C_MUX1_BUS2, NUM_SFP_PORTS,
			      FIRST_SFP_PORT_NUM, DNI_AG7648_I2C_SFP_MUX_PORT,
			      "cpld-sfp-mux", cpld_sfp_mux_select_chan);
	if (ret)
		goto exit;

	ret = create_port_mux(DNI_AG7648_I2C_MUX1_BUS3, NUM_QSFP_PORTS,
			      FIRST_QSFP_PORT_NUM,
			      DNI_AG7648_I2C_QSFP_MUX_PORT_49,
			      "cpld-qsfp-mux", cpld_qsfp_mux_select_chan);
	if (ret)
		goto exit;

exit:
	return ret;
}

static void __exit dni_ag7648_port_mux_exit(void)
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

	if (cpld_idx < 0 || cpld_idx >= NO_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	val = i2c_smbus_read_byte_data(ag7648_cpld_client_list[cpld_idx], reg);
	if (val < 0)
		pr_err("I2C read err - addr: 0x%02X, offset: 0x%02X",
		       ag7648_cpld_client_list[cpld_idx]->addr,
			STRIP_CPLD_IDX(reg));
	return val;
}

static int cpld_reg_write(uint32_t reg, uint8_t write_val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	int i;

	if (cpld_idx < 0 || cpld_idx >= NO_CPLD_I2C_CLIENTS) {
		pr_err("attempt to write invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	/* CM-9130: Sometimes I2C ISMT write byte operation fails with -EIO. */
	/* The following loop is a workaround till we can get the real fix.  */
	for (i = 0; i < 3; i++) {
		res =
		    i2c_smbus_write_byte_data(ag7648_cpld_client_list[cpld_idx],
					      STRIP_CPLD_IDX(reg), write_val);
		if (!res)
			return res;
	}
	pr_err("I2C write err - addr: 0x%02X, reg: 0x%02X, val: 0x%02X, Errno: %d",
	       ag7648_cpld_client_list[cpld_idx]->addr,
	       STRIP_CPLD_IDX(reg), write_val, res);

	return res;
}

static ssize_t bulk_power_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	uint8_t read_val;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok;
	uint8_t error;

	read_val = ~(cpld_reg_read(CPLD_MASTER_POWER_SUPPLY_STATUS_REG));
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask	  = CPLD_PSU1_MASK;
		present_l = CPLD_PSU1_PRESENT_L;
		pwr_ok	  = CPLD_PSU1_GOOD_L;
		error	  = CPLD_PSU1_ERROR;
	} else {
		mask	  = CPLD_PSU2_MASK;
		present_l = CPLD_PSU2_PRESENT_L;
		pwr_ok	  = CPLD_PSU2_GOOD_L;
		error	  = CPLD_PSU2_ERROR;
	}
	read_val &= mask;

	if (read_val & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(read_val & pwr_ok) || (read_val & error))
			strcat(buf, ", " PLATFORM_PS_POWER_BAD);
		else
			strcat(buf, ", " PLATFORM_OK);
	} else { /* Not Present */
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);

static ssize_t cpld_revision_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	return sprintf(buf, "System: 0x%02X\nMaster: 0x%02X\n"
		       "Slave: 0x%02X\n", cpld_reg_read(CPLD_SYSTEM_HW_REV_REG),
		       cpld_reg_read(CPLD_MASTER_HW_REV_REG),
		       cpld_reg_read(CPLD_SLAVE_REV_REG));
}

static SYSFS_ATTR_RO(cpld_revision, cpld_revision_show);

static ssize_t fan_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int     reg = CPLD_MASTER_LED_CONTROL_REG2;
	int     val = 0;
	uint8_t present_l = 0;

	if (strcmp(dattr->attr.name, "fan_1") == 0) {
		present_l = CPLD_FAN_TRAY_1_PRESENT_L;
	} else if (strcmp(dattr->attr.name, "fan_2") == 0) {
		present_l = CPLD_FAN_TRAY_2_PRESENT_L;
	} else if (strcmp(dattr->attr.name, "fan_3") == 0) {
		reg = CPLD_MASTER_FAN_STATUS_REG;
		present_l = CPLD_FAN_TRAY_3_PRESENT_L;
	}
	val = cpld_reg_read(reg);
	if (val < 0)
		return val;
	if (val & present_l)
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	return sprintf(buf, PLATFORM_OK "\n");
}

static SENSOR_DEVICE_ATTR(fan_1, S_IRUGO, fan_show, NULL, 0);
static SENSOR_DEVICE_ATTR(fan_2, S_IRUGO, fan_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan_3, S_IRUGO, fan_show, NULL, 2);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DNI_AG7648_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	/* LED control register 1 */
	{
		.name = "led_system",
		.offset = CPLD_MASTER_LED_CONTROL_REG1,
		.mask = CPLD_SYSTEM_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYSTEM_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_SYSTEM_LED_YELLOW },
			{ PLATFORM_LED_YELLOW_BLINKING,
			  CPLD_SYSTEM_LED_YELLOW_BLINK },
			{ PLATFORM_LED_GREEN_BLINKING,
			  CPLD_SYSTEM_LED_GREEN_BLINK },
		},
	},
	{
		.name = "led_locator",
		.offset = CPLD_MASTER_LED_CONTROL_REG1,
		.mask = CPLD_LOC_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYSTEM_LED_GREEN },
			{ PLATFORM_LED_GREEN_BLINKING,
			  CPLD_LOC_LED_GREEN_BLINK },
			{ PLATFORM_LED_OFF, CPLD_LOC_LED_OFF },
		},
	},
	{
		.name = "led_power",
		.offset = CPLD_MASTER_INT_REG,
		.mask = CPLD_POWER_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_POWER_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_POWER_LED_YELLOW },
			{ PLATFORM_LED_YELLOW_BLINKING,
			  CPLD_POWER_LED_YELLOW_BLINK },
			{ PLATFORM_LED_OFF, CPLD_POWER_LED_OFF },
		},
	},
	/* Fan status register 1 */
	{
		.name = "led_fan_tray_1",
		.offset = CPLD_MASTER_LED_CONTROL_REG2,
		.mask = CPLD_FAN_TRAY_1_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_1_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_1_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_1_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_2",
		.offset = CPLD_MASTER_LED_CONTROL_REG2,
		.mask = CPLD_FAN_TRAY_2_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_2_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_2_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_2_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_3",
		.offset = CPLD_MASTER_LED_CONTROL_REG2,
		.mask = CPLD_FAN_TRAY_3_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_3_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_3_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_3_LED_OFF },
		},
	},
	/* Fan status register 2 */
	{
		.name = "led_fan",
		.offset = CPLD_MASTER_FAN_STATUS_REG,
		.mask = CPLD_FRONT_FAN_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FRONT_FAN_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FRONT_FAN_LED_YELLOW },
			{ PLATFORM_LED_YELLOW_BLINKING,
			  CPLD_FRONT_FAN_LED_YELLOW_BLINK },
			{ PLATFORM_LED_OFF, CPLD_FRONT_FAN_LED_OFF },
		},
	},
};

/*
 * LEDs
 */
static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	uint8_t tmp;
	int i;
	struct led *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target led */
	target = &cpld_leds[sensor_dev_attr->index];
	if (!target)
		return sprintf(buf, "undefined\n");

	/* read the register */
	tmp = cpld_reg_read(target->offset);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++)
		if (tmp == target->colors[i].value)
			break;
	if (i == target->n_colors)
		return sprintf(buf, "undefined\n");
	return sprintf(buf, "%s\n", target->colors[i].name);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	uint8_t tmp;
	int i;
	int ret;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target led */
	target = &cpld_leds[sensor_dev_attr->index];
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
	tmp = cpld_reg_read(target->offset);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;

	ret = cpld_reg_write(target->offset, tmp);
	if (ret < 0)
		return ret;

	return count;
}

static SENSOR_DEVICE_ATTR_RW(led_system, led_show, led_store, 0);
static SENSOR_DEVICE_ATTR_RW(led_locator, led_show, led_store, 1);
static SENSOR_DEVICE_ATTR_RW(led_power, led_show, led_store, 2);
static SENSOR_DEVICE_ATTR_RW(led_fan_tray_1, led_show, led_store, 3);
static SENSOR_DEVICE_ATTR_RW(led_fan_tray_2, led_show, led_store, 4);
static SENSOR_DEVICE_ATTR_RW(led_fan_tray_3, led_show, led_store, 5);
static SENSOR_DEVICE_ATTR_RW(led_fan, led_show, led_store, 6);
/*------------------------------------------------------------------------------
 *
 * SFP status definitions
 *
 * All the definition use positive logic.
 */
#define DNI_AG7648_CPLD_STRING_NAME_SIZE 30
struct sfp_status {
	char name[DNI_AG7648_CPLD_STRING_NAME_SIZE];
	uint8_t reg[6];
	uint8_t active_low;
	uint8_t num_regs;
};

static struct sfp_status cpld_sfp_status[] = {
	{
		.name = "present",
		.num_regs = 6,
		.reg = { CPLD_SFP_1_8_MOD_PRESENT_L_REG,
			 CPLD_SFP_9_16_MOD_PRESENT_L_REG,
			 CPLD_SFP_17_24_MOD_PRESENT_L_REG,
			 CPLD_SFP_25_32_MOD_PRESENT_L_REG,
			 CPLD_SFP_33_40_MOD_PRESENT_L_REG,
			 CPLD_SFP_41_48_MOD_PRESENT_L_REG,},
		.active_low = 1,
	},
	{
		.name = "tx_fault",
		.num_regs = 6,
		.reg = { CPLD_SFP_1_8_MOD_TX_FAULT_REG,
			 CPLD_SFP_9_16_MOD_TX_FAULT_REG,
			 CPLD_SFP_17_24_MOD_TX_FAULT_REG,
			 CPLD_SFP_25_32_MOD_TX_FAULT_REG,
			 CPLD_SFP_33_40_MOD_TX_FAULT_REG,
			 CPLD_SFP_41_48_MOD_TX_FAULT_REG,},
	},
	{
		.name = "tx_enable",
		.num_regs = 6,
		.reg = { CPLD_SFP_1_8_MOD_TX_DISABLE_REG,
			 CPLD_SFP_9_16_MOD_TX_DISABLE_REG,
			 CPLD_SFP_17_24_MOD_TX_DISABLE_REG,
			 CPLD_SFP_25_32_MOD_TX_DISABLE_REG,
			 CPLD_SFP_33_40_MOD_TX_DISABLE_REG,
			 CPLD_SFP_41_48_MOD_TX_DISABLE_REG,},
		.active_low = 1,
	},
	{
		.name = "rx_los",
		.num_regs = 6,
		.reg = { CPLD_SFP_1_8_MOD_RX_LOS_REG,
			 CPLD_SFP_9_16_MOD_RX_LOS_REG,
			 CPLD_SFP_17_24_MOD_RX_LOS_REG,
			 CPLD_SFP_25_32_MOD_RX_LOS_REG,
			 CPLD_SFP_33_40_MOD_RX_LOS_REG,
			 CPLD_SFP_41_48_MOD_RX_LOS_REG,},
	},
	{
		/* I2C mux select register for sfp+ ports,
		 * 6 bits for 48 ports */
		.name = "i2c_enable",
		.num_regs = 1,
		.active_low = 0,
		.reg = {CPLD_SFP_MUX_SELECT_REG,},
	},
};

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int i;
	uint64_t val = 0;
	uint64_t temp_val = 0;
	struct sfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &cpld_sfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	/* Assemble the 6 8-bit status register values
	 * into a single 48-bit return value */
	for (i = 0; i < target->num_regs; i++) {
		temp_val = cpld_reg_read(target->reg[i]);
		val |= (temp_val << (i * 8));
	}

	if (target->active_low)
		val = ~val;

	if (strcmp(target->name, "i2c_enable") == 0)
		return sprintf(buf, "0x%x\n", ((uint8_t)val) & 0xff);

	return sprintf(buf, "0x%.12llx\n", (val & 0xffffffffffff));
}

static ssize_t sfp_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int ret = 0, i;
	uint64_t val;
	struct sfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);

	if (kstrtoll(buf, 0, &val) != 0)
		return -EINVAL;

	/* find the target register */
	target = &cpld_sfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		ret = cpld_reg_write(target->reg[i],
				     target->active_low ?
				     ~((val >> (i * 8)) & 0xff) :
				     (val >> (i * 8)) & 0xff);

		if (ret < 0) {
			pr_err("CPLD qsfp register (%s) write failed",
			       target->name);
			goto done;
		}
	}
done:
	if (ret < 0)
		return ret;

	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp_present, sfp_show, 0);
static SENSOR_DEVICE_ATTR_RO(sfp_tx_fault, sfp_show, 1);
static SENSOR_DEVICE_ATTR_RW(sfp_tx_enable, sfp_show, sfp_store, 2);
static SENSOR_DEVICE_ATTR_RO(sfp_rx_los, sfp_show, 3);
static SENSOR_DEVICE_ATTR_RW(sfp_i2c_enable, sfp_show, sfp_store, 4);

static struct sfp_status cpld_qsfp_status[] = {
	{
		.name = "modsel",
		.active_low = 1,
		.reg = { CPLD_QSFP_MOD_SELECT_REG,},
	},
	{
		.name = "lp_mode",
		.reg = { CPLD_QSFP_LP_MODE_REG,},
	},
	{
		.name = "present",
		.active_low = 1,
		.reg = { CPLD_QSFP_PRESENT_L_REG,},
	},
	{
		.name = "reset",
		.active_low = 1,
		.reg = { CPLD_QSFP_RESET_L_REG,},
	},
	{
		.name = "interrupt_ctrl",
		.active_low = 1,
		.reg = { CPLD_QSFP_INTERRUPT_L_REG,},
	},
};

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	uint8_t val = 0;
	struct sfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	val = cpld_reg_read(target->reg[0]);

	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%02x\n", val);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int ret = 0;
	uint8_t val;
	struct sfp_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	if (kstrtou8(buf, 0, &val) != 0)
		return -EINVAL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);

	/* find the target register */
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	ret = cpld_reg_write(target->reg[0],
			     target->active_low ?
			     ~val : val);
	if (ret < 0) {
		pr_err("CPLD qsfp register (%s) write failed",
		       target->name);
		goto done;
	}
done:
	if (ret < 0)
		return ret;

	return count;
}

static SENSOR_DEVICE_ATTR_RW(qsfp_modsel,         qsfp_show, qsfp_store, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp_lp_mode,        qsfp_show, qsfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp_present,        qsfp_show, 2);
static SENSOR_DEVICE_ATTR_RW(qsfp_reset,          qsfp_show, qsfp_store, 3);
static SENSOR_DEVICE_ATTR_RO(qsfp_interrupt_ctrl, qsfp_show, 4);

static struct attribute *dni_ag7648_cpld_attrs[] = {
	&dev_attr_cpld_revision.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&sensor_dev_attr_fan_1.dev_attr.attr,
	&sensor_dev_attr_fan_2.dev_attr.attr,
	&sensor_dev_attr_fan_3.dev_attr.attr,
	&sensor_dev_attr_led_system.dev_attr.attr,
	&sensor_dev_attr_led_power.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&sensor_dev_attr_led_locator.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_1.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_2.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_3.dev_attr.attr,
	&sensor_dev_attr_qsfp_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp_modsel.dev_attr.attr,
	&sensor_dev_attr_qsfp_lp_mode.dev_attr.attr,
	&sensor_dev_attr_qsfp_present.dev_attr.attr,
	&sensor_dev_attr_qsfp_interrupt_ctrl.dev_attr.attr,
	&sensor_dev_attr_sfp_present.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_enable.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp_i2c_enable.dev_attr.attr,
	NULL,
};

static struct attribute_group dni_ag7648_cpld_attr_group = {
	.attrs = dni_ag7648_cpld_attrs,
};

static int dni_ag7648_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &dni_ag7648_cpld_attr_group);
	if (ret)
		pr_err("sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int dni_ag7648_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver dni_ag7648_cpld_driver = {
	.driver = {
		.name = "delta_ag7648_cpld",
		.owner = THIS_MODULE,
	},
	.probe = dni_ag7648_cpld_probe,
	.remove = dni_ag7648_cpld_remove,
};

static struct platform_device *dni_ag7648_cpld_device;

static struct platform_device_info cpld_pltf_dev_info = {
	.name = "delta_ag7648_cpld",
	.id = 0,
};

static int __init dni_ag7648_cpld_init(void)
{
	int ret;

	if (num_cpld_i2c_devices != NO_CPLD_I2C_CLIENTS) {
		pr_err("cpld device unable to find i2c clients");
		ret = -ENODEV;
		goto err_drvr;
	}

	ret = platform_driver_register(&dni_ag7648_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	dni_ag7648_cpld_device =
			     platform_device_register_full(&cpld_pltf_dev_info);
	if (!dni_ag7648_cpld_device) {
		pr_err("platform_device_register_full() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	return 0;

err_dev_alloc:
	platform_driver_unregister(&dni_ag7648_cpld_driver);

err_drvr:
	return ret;
}

/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dni_ag7648_init(void)
{
	int ret = 0;

	ret = dni_ag7648_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = dni_ag7648_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		return ret;
	}

	ret = dni_ag7648_port_mux_init();
	if (ret) {
		pr_err("Initializing QSFP mux failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit dni_ag7648_exit(void)
{
	dni_ag7648_port_mux_exit();
	dni_ag7648_cpld_exit();
	dni_ag7648_i2c_exit();
	pr_err(DRIVER_NAME " driver unloaded\n");
}

static void __exit dni_ag7648_cpld_exit(void)
{
	platform_driver_unregister(&dni_ag7648_cpld_driver);
	platform_device_unregister(dni_ag7648_cpld_device);
}

module_init(dni_ag7648_init);
module_exit(dni_ag7648_exit);

MODULE_AUTHOR("Nikhil Dhar(ndhar@cumulusnetworks.com)");
MODULE_DESCRIPTION("DNI AG7648 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


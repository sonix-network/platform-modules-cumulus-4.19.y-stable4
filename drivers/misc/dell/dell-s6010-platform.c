/*
 *	dell_s6010_platform.c - DELL S6010-C2538 Platform Support.
 *
 *	Copyright (C) 2016 Cumulus Networks, Inc.
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
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *	02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>

#include "platform-defs.h"
#include "dell-s6010-cpld.h"


#define DRIVER_NAME		   "dell_s6010"
#define DRIVER_VERSION	   "0.2"

/*
 * This platform has two i2c busses:
 * SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 * SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 */

/* I2c bus adapter numbers for the down stream i2c busses */
enum {
	DELL_S6010_I2C_iSMT_BUS = 0,
	DELL_S6010_I2C_I801_BUS,
	DELL_S6010_I2C_MUX1_BUS0 = 10,
	DELL_S6010_I2C_MUX1_BUS1,
	DELL_S6010_I2C_MUX1_BUS2,
	DELL_S6010_I2C_MUX1_BUS3,
	DELL_S6010_I2C_MUX1_BUS4,
	DELL_S6010_I2C_MUX1_BUS5,
	DELL_S6010_I2C_MUX1_BUS6,
	DELL_S6010_I2C_MUX1_BUS7,
	DELL_S6010_I2C_QSFP_MUX_PORT = 21,
};


static struct i2c_client *s6010_cpld_client_list[NO_CPLD_I2C_CLIENTS];
static int num_cpld_i2c_devices;


mk_eeprom(spd, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  50, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1,  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan2,  52, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan3,  53, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan4,  54, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan5,  55, 256, AT24_FLAG_IRUGO);


/*
 * the iSMT bus has a single PCA9548 switch that connects the devices
 */
static struct pca954x_platform_mode mux1_platform_modes[] = {
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S6010_I2C_MUX1_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux1_platform_data = {
	.modes = mux1_platform_modes,
	.num_modes = ARRAY_SIZE(mux1_platform_modes),
};

struct dell_s6010_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct dell_s6010_i2c_device_info s6010_i2c_devices[] = {
	{
		.bus = DELL_S6010_I2C_iSMT_BUS,
		{
			.type = "pca9548",
			.addr = 0x70,
			.platform_data = &mux1_platform_data,
		}
	},
	{
		.bus = DELL_S6010_I2C_I801_BUS,
		{
			/* DIMM */
			I2C_BOARD_INFO("spd", 0x52),
			.platform_data = &spd_52_at24,
		}
	},
	/* Bus 0 */
	{
		.bus = DELL_S6010_I2C_MUX1_BUS0,
		{
			/*  Board EEPROM */
			I2C_BOARD_INFO("24c02", 0x53),
			.platform_data = &board_53_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS0,
		{
			/* System CPLD */
			I2C_BOARD_INFO("dummy", 0x31), 
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS0,
		{
			/* Master CPLD */
			I2C_BOARD_INFO("dummy", 0x32),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS0,
		{
			/* Slave CPLD */
			I2C_BOARD_INFO("dummy", 0x33),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS0,
		{
			/* TMP75 Temp Sensor */
			I2C_BOARD_INFO("tmp75", 0x4d),
		}
	},
	/* Bus 1 */
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/*TMP75 Temp Sensor close to T2+*/
			I2C_BOARD_INFO("tmp75", 0x4a),
		},
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* TMP75 Temp Sensor */
			I2C_BOARD_INFO("tmp75", 0x4b),
		},
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* TMP75 Temp Sensor */
			I2C_BOARD_INFO("tmp75", 0x4c),
		},
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/*TMP75 Temp Sensor close to QSFP.*/
			I2C_BOARD_INFO("tmp75", 0x4e),
		},
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* TMP75 Temp Sensor close to
			 * fan tray */
			I2C_BOARD_INFO("tmp75", 0x4f),
		},
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* LTC4215 PSU 1 */
			I2C_BOARD_INFO("ltc4215", 0x40),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* LTC4215 PSU 2 */
			I2C_BOARD_INFO("ltc4215", 0x42),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan 1 EEPROM */
			I2C_BOARD_INFO("24c02", 0x51),
			.platform_data = &fan1_51_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan 2 EEPROM */
			I2C_BOARD_INFO("24c02", 0x52),
			.platform_data = &fan2_52_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan 3 EEPROM */
			I2C_BOARD_INFO("24c02", 0x53),
			.platform_data = &fan3_53_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan 4 EEPROM */
			I2C_BOARD_INFO("24c02", 0x54),
			.platform_data = &fan4_54_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan 5 EEPROM */
			I2C_BOARD_INFO("24c02", 0x55),
			.platform_data = &fan5_55_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* Fan controller */
			I2C_BOARD_INFO("emc2305", 0x4d),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS1,
		{
			/* AVS Power PWM */
			I2C_BOARD_INFO("6764tr", 0x6f),
		}
	},
	/* Bus 4 */
	{
		.bus = DELL_S6010_I2C_MUX1_BUS4,
		{
			/* PSU 1 PMBUS  */
			I2C_BOARD_INFO("dps460", 0x58),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS4,
		{
			/* PSU 2 PMBUS  */
			I2C_BOARD_INFO("dps460", 0x59),
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS4,
		{
			/* PSU 1 EEPROM */
			I2C_BOARD_INFO("24c02", 0x50),
			.platform_data = &psu1_50_at24,
		}
	},
	{
		.bus = DELL_S6010_I2C_MUX1_BUS4,
		{
			/* PSU 2 EEPROM */
			I2C_BOARD_INFO("24c02", 0x51),
			.platform_data = &psu2_51_at24,
		}
	},
};

static int __init dell_s6010_cpld_init(void);
static void __exit dell_s6010_cpld_exit(void);

static int __init dell_s6010_port_mux_init(void);
static void __exit dell_s6010_port_mux_exit(void);

/**
 * dell_s6010_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
static struct i2c_client *s6010_clients_list[ARRAY_SIZE(s6010_i2c_devices)];

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

static void free_dell_s6010_i2c_data(void)
{
	free_i2c_clients(s6010_clients_list,
			 ARRAY_SIZE(s6010_i2c_devices));
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

	for (i = 0; i < DELL_S6010_I2C_MUX1_BUS0; i++) {
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

static int populate_i2c_devices(struct dell_s6010_i2c_device_info *devices,
				int num_devices,
				struct i2c_client **clients_list,
				int iSMT_bus, int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	num_cpld_i2c_devices = 0;
	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == DELL_S6010_I2C_iSMT_BUS) {
			devices[i].bus = iSMT_bus;
		} else if (devices[i].bus == DELL_S6010_I2C_I801_BUS) {
			devices[i].bus = I801_bus;
		}
		client = add_i2c_client(devices[i].bus, &devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
		if ((strcmp(devices[i].board_info.type, "dummy") == 0) &&
			(num_cpld_i2c_devices < NO_CPLD_I2C_CLIENTS)) {
			s6010_cpld_client_list[num_cpld_i2c_devices] = client;
			num_cpld_i2c_devices++;
		}
	}
	return 0;

err_exit:
	return ret;
}

static int __init dell_s6010_i2c_init(void)
{
	int iSMT_bus;
	int I801_bus;
	int ret;

	ret = -1;
	iSMT_bus = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (iSMT_bus < 0) {
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
	ret = populate_i2c_devices(s6010_i2c_devices,
				   ARRAY_SIZE(s6010_i2c_devices),
				   &s6010_clients_list[0], iSMT_bus, I801_bus);
	if (ret) {
		goto err_exit;
	}

	return 0;

err_exit:
	free_dell_s6010_i2c_data();
	return ret;
}

static void __exit dell_s6010_i2c_exit(void)
{
	free_dell_s6010_i2c_data();
}

/*---------------------------------------------------------------------
 *
 * CPLD SFP/QSFP mux
 *
 *	 'cpld-qsfp-mux' for qsfp ports 1-32
 */

#define NUM_PORT_MUXES			   (1)
#define FIRST_QSFP_PORT_NUM		   (1)
#define NUM_QSFP_PORTS			   (32)

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
	struct port_mux_info_struct *p = port_mux_info;


	if (p) {
		for (i = 0; i < p->num_port_muxes; i++) {
			for (j = 0; j < p->mux_item[i].num_items; j++) {
				if (p->mux_item[i].mux_clients[j])
					i2c_unregister_device(
						p->mux_item[i].mux_clients[j]);
			if (p->mux_item[i].mux_board_infos[j])
				free_port_mux_board_info(
					p->mux_item[i].mux_board_infos[j]);
			}
			if (p->mux_item[i].muxc)
				i2c_mux_del_adapters(p->mux_item[i].muxc);
			kfree(p->mux_item[i].mux_clients);
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

	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, PORT_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data),
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

static int qsfp_mux_i2c_reg_write(uint8_t reg, u8 val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;

	adap = i2c_get_adapter(DELL_S6010_I2C_MUX1_BUS0);
	client = s6010_cpld_client_list[GET_CPLD_IDX(reg)];
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

static uint8_t modsel_regs[] = {
				CPLD_QSFP_1_8_MOD_SELECT_REG,
				CPLD_QSFP_9_16_MOD_SELECT_REG,
				CPLD_QSFP_17_24_MOD_SELECT_REG,
				CPLD_QSFP_25_32_MOD_SELECT_REG
				};

static int cpld_set_modsel(uint8_t val)
{
	int ret;
	uint8_t modsel_reg;
	uint8_t modsel_val;

	modsel_reg = modsel_regs[(val - 1) / 8];
	modsel_val = ~(1 << ((val - 1) % 8));
	ret = qsfp_mux_i2c_reg_write(modsel_reg, modsel_val);
	if (ret < 0) {
		pr_err("CPLD reg write failed for modsel reg (%x)\n",
		       modsel_reg);
		return ret;
	}
	usleep_range(1000,1200);
	return 0;
}

static int cpld_clear_modsel(uint8_t val)
{
	int ret;
	uint8_t modsel_reg;

	modsel_reg = modsel_regs[(val - 1) / 8];
	ret = qsfp_mux_i2c_reg_write(modsel_reg, 0xFF);
	if (ret < 0) {
		pr_err("CPLD reg write failed for modsel reg (%x)\n",
		       modsel_reg);
		return ret;
	}
	return 0;
}

static int cpld_reg_write(uint32_t reg, uint8_t write_val);
static int cpld_qsfp_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	uint8_t val;
	int ret;

	val = (chan - DELL_S6010_I2C_QSFP_MUX_PORT) + 1;
	ret = qsfp_mux_i2c_reg_write(CPLD_QSFP_1_32_MUX_SELECT_REG, val);
	if (ret < 0)
		return ret;
	return cpld_set_modsel(val);
}

static int cpld_qsfp_mux_deselect_chan(struct i2c_mux_core *muxc, u32 chan)
{
	uint8_t val;
	int ret;

	val = (chan - DELL_S6010_I2C_QSFP_MUX_PORT) + 1;
	ret = cpld_clear_modsel(val);
	if (ret < 0)
		return ret;
	return qsfp_mux_i2c_reg_write(CPLD_QSFP_1_32_MUX_SELECT_REG, 0);
}

static int create_port_mux(int bus, int num_mux_clients, int first_port_num,
			   int first_mux_num, const char *mux_name,
			   int (select_function(struct i2c_mux_core *, u32)),
			   int (deselect_function(struct i2c_mux_core *, u32)))
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

	mux_item->mux_clients = kzalloc(num_mux_clients *
					sizeof(struct i2c_client *),
					GFP_KERNEL);
	if (!mux_item->mux_clients)
		goto err_exit;

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

	mux_item->muxc = i2c_mux_alloc(adapter, &mux_item->mux_dev->dev,
				       num_mux_clients, 0, 0,
				       cpld_qsfp_mux_select_chan,
				       cpld_qsfp_mux_deselect_chan);
	if (!mux_item->muxc) {
		pr_err("i2c_mux_alloc() failed for mux %s.\n", mux_name);
		ret = -ENOMEM;
		goto err_exit_device;
	}

	port_mux_info->num_port_muxes++;

	for (i = 0; i < num_mux_clients; i++) {
		struct i2c_board_info *eeprom_info;
		struct i2c_client *client;
		int mux_num;

		mux_num = first_mux_num + i;
		ret = i2c_mux_add_adapter(mux_item->muxc, mux_num, mux_num, 0);
		if (ret) {
			pr_err("i2c_mux_add_adapter() failed for channel %u.\n", mux_num);
			goto err_exit;
		}

		eeprom_info = alloc_port_mux_board_info(first_port_num + i);
		if (!eeprom_info) {
			ret = -ENOMEM;
			goto err_exit;
		}

		mux_item->mux_board_infos[i] = eeprom_info;
		client = i2c_new_device(mux_item->muxc->adapter[i], eeprom_info);
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

err_exit:
	port_mux_info_free_data();
	return -ENOMEM;
}

static int __init dell_s6010_port_mux_init(void)
{
	int ret;

	port_mux_info = kzalloc(sizeof(struct port_mux_info_struct),
				GFP_KERNEL);
	if (!port_mux_info)
		return -ENOMEM;

	ret = create_port_mux(DELL_S6010_I2C_MUX1_BUS2, NUM_QSFP_PORTS,
				  FIRST_QSFP_PORT_NUM,
				  DELL_S6010_I2C_QSFP_MUX_PORT,
				  "cpld-qsfp-mux",
				  cpld_qsfp_mux_select_chan,
				  cpld_qsfp_mux_deselect_chan);
	if (ret)
		goto exit;

exit:
	return ret;
}

static void __exit dell_s6010_port_mux_exit(void)
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
	val = i2c_smbus_read_byte_data(s6010_cpld_client_list[cpld_idx], reg);
	if (val < 0)
		pr_err("I2C read err - addr: 0x%02X, offset: 0x%02X",
			   s6010_cpld_client_list[cpld_idx]->addr,
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
		res = i2c_smbus_write_byte_data (s6010_cpld_client_list[cpld_idx],
						 STRIP_CPLD_IDX(reg), write_val);
		if (!res)
			return res;
    }
	pr_err("I2C write err - addr: 0x%02X, reg: 0x%02X, val: 0x%02X, Errno:\
			%d", s6010_cpld_client_list[cpld_idx]->addr,
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

	read_val = ~(cpld_reg_read(CPLD_POWER_SUPPLY_STATUS_REG));
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
			strcat(buf, ", "PLATFORM_PS_POWER_BAD);
		else
			strcat(buf, ", "PLATFORM_OK);
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
			"Slave: 0x%02X\n",
			   cpld_reg_read(CPLD_SYSTEM_REV_REG),
			   cpld_reg_read(CPLD_MASTER_REV_REG),
			   cpld_reg_read(CPLD_SLAVE_REV_REG));

}
static SYSFS_ATTR_RO(cpld_revision, cpld_revision_show);

static ssize_t hw_revision_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	return sprintf(buf, "System: 0x%02X, Master: 0x%02X \n",
			   cpld_reg_read(CPLD_SYSTEM_HW_REV_REG),
			   cpld_reg_read(CPLD_MASTER_HW_REV_REG));

}
static SYSFS_ATTR_RO(hw_revision, hw_revision_show);

static ssize_t fan_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int     reg = CPLD_FAN_STATUS_1_REG;
    int     val = 0;
    uint8_t present_l = 0;

	if (strcmp(dattr->attr.name, "fan_1") == 0) {
		present_l = CPLD_FAN_TRAY_1_PRESENT_L;
	} else if (strcmp(dattr->attr.name, "fan_2") == 0) {
		present_l = CPLD_FAN_TRAY_2_PRESENT_L;
	} else if (strcmp(dattr->attr.name, "fan_3") == 0) {
		reg = CPLD_FAN_STATUS_2_2_REG;
		present_l = CPLD_FAN_TRAY_3_PRESENT_L;
	} else if (strcmp(dattr->attr.name, "fan_4") == 0) {
		reg = CPLD_FAN_STATUS_2_2_REG;
		present_l = CPLD_FAN_TRAY_4_PRESENT_L;
    } else if (strcmp(dattr->attr.name, "fan_5") == 0) {
		reg = CPLD_FAN_STATUS_2_2_REG;
		present_l = CPLD_FAN_TRAY_5_PRESENT_L;
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
static SENSOR_DEVICE_ATTR(fan_4, S_IRUGO, fan_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan_5, S_IRUGO, fan_show, NULL, 4);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DELL_S6010_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	/* LED control register 1 */
	{
		.name = "led_system",
		.offset = CPLD_LED_CONTROL_1_REG,
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
		.offset = CPLD_LED_CONTROL_1_REG,
		.mask = CPLD_BEACON_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_BLUE_BLINKING,
			  CPLD_BEACON_LED_BLUE_BLINK },
			{ PLATFORM_LED_OFF, CPLD_BEACON_LED_OFF },
		},
	},
	{
		.name = "led_power",
		.offset = CPLD_LED_CONTROL_1_REG,
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
	/* Fan status register 1 */
	{
		.name = "led_fan_tray_1",
		.offset = CPLD_FAN_STATUS_1_REG,
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
		.offset = CPLD_FAN_STATUS_1_REG,
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
		.offset = CPLD_FAN_STATUS_1_REG,
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
		.offset = CPLD_FAN_STATUS_2_REG,
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
	{
		.name = "led_port",
		.offset = CPLD_FAN_STATUS_2_REG,
		.mask = CPLD_PORT_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_ON,	CPLD_PORT_LED_ON },
			{ PLATFORM_LED_OFF, CPLD_PORT_LED_OFF },
		},
	},
	/* Fan status register 2/2 */
	{
		.name = "led_fan_tray_4",
		.offset = CPLD_FAN_STATUS_2_2_REG,
		.mask = CPLD_FAN_TRAY_4_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_4_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_4_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_4_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_5",
		.offset = CPLD_FAN_STATUS_2_2_REG,
		.mask = CPLD_FAN_TRAY_5_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_TRAY_5_LED_GREEN },
			{ PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_5_LED_YELLOW },
			{ PLATFORM_LED_OFF, CPLD_FAN_TRAY_5_LED_OFF },
		},
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

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

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL)
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

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL)
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

static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_power, led_show, led_store);
static SYSFS_ATTR_RW(led_fan, led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_locator, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_1, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_2, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_3, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_4, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_5, led_show, led_store);
static SYSFS_ATTR_RW(led_port, led_show, led_store);

/*------------------------------------------------------------------------------
 *
 * QSFP definitions
 *
 */
struct qsfp_status {
	char name[DELL_S6010_CPLD_STRING_NAME_SIZE];
	uint8_t active_low;
	uint8_t num_regs;
	uint8_t	reg[4];
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
		.name = "modsel",
		.active_low = 1,
		.num_regs = 4,
		.reg = { CPLD_QSFP_1_8_MOD_SELECT_REG,
			 CPLD_QSFP_9_16_MOD_SELECT_REG,
			 CPLD_QSFP_17_24_MOD_SELECT_REG,
			 CPLD_QSFP_25_32_MOD_SELECT_REG,
			},
	},
	{
		.name = "lp_mode",
		.num_regs = 4,
		.reg = { CPLD_QSFP_1_8_LP_MODE_REG,
			 CPLD_QSFP_9_16_LP_MODE_REG,
			 CPLD_QSFP_17_24_LP_MODE_REG,
			 CPLD_QSFP_25_32_LP_MODE_REG,
		       },
	},
	{
		.name = "present",
		.active_low = 1,
		.num_regs = 4,
		.reg = { CPLD_QSFP_1_8_PRESENT_REG,
			 CPLD_QSFP_9_16_PRESENT_REG,
			 CPLD_QSFP_17_24_PRESENT_REG,
			 CPLD_QSFP_25_32_PRESENT_REG,
		       },
	},
	{
		.name = "reset",
		.active_low = 1,
		.num_regs = 4,
		.reg = { CPLD_QSFP_1_8_RESET_REG,
			 CPLD_QSFP_9_16_RESET_REG,
			 CPLD_QSFP_17_24_RESET_REG,
			 CPLD_QSFP_25_32_RESET_REG,
		       },
	},
	{
		.name = "interrupt_ctrl",
		.active_low = 1,
		.num_regs = 4,
		.reg = { CPLD_QSFP_1_8_INTERRUPT_REG,
			 CPLD_QSFP_9_16_INTERRUPT_REG,
			 CPLD_QSFP_17_24_INTERRUPT_REG,
			 CPLD_QSFP_25_32_INTERRUPT_REG,
		       },
	},
	{
		/* I2C mux select register for qsfp+ ports, 
		 * 6 bits for 32 ports */
		.name = "i2c_enable",
		.active_low = 0,
		.num_regs = 1,
		.reg = {CPLD_QSFP_1_32_MUX_SELECT_REG,},
	},
};

static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	int i;
	s32 val = 0;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	struct qsfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len,
			   cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}

	if (target == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < target->num_regs; i++) {
		val |= (cpld_reg_read(target->reg[i]) << (i * 8));
	}

	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%08x\n", val);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int ret = 0, i;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	uint32_t val;
	struct qsfp_status *target = NULL;

	if (kstrtouint(buf, 0, &val) != 0)
		return -EINVAL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len,
			   cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}

	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		if ((ret = cpld_reg_write(target->reg[i], 
					  target->active_low ?
					  ~((val >> (i * 8)) & 0xff) :
					  (val >> (i * 8)) & 0xff)) < 0) {

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

SYSFS_ATTR_RW(qsfp_modsel,		qsfp_show, qsfp_store);
SYSFS_ATTR_RW(qsfp_reset,		qsfp_show, qsfp_store);
SYSFS_ATTR_RW(qsfp_lp_mode,		qsfp_show, qsfp_store);
SYSFS_ATTR_RO(qsfp_present,		qsfp_show);
SYSFS_ATTR_RW(qsfp_i2c_enable,		qsfp_show, qsfp_store);
SYSFS_ATTR_RO(qsfp_interrupt_ctrl,	qsfp_show);

static struct attribute *dell_s6010_cpld_attrs[] = {
	&dev_attr_cpld_revision.attr,
	&dev_attr_hw_revision.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&sensor_dev_attr_fan_1.dev_attr.attr,
	&sensor_dev_attr_fan_2.dev_attr.attr,
	&sensor_dev_attr_fan_3.dev_attr.attr,
	&sensor_dev_attr_fan_4.dev_attr.attr,
	&sensor_dev_attr_fan_5.dev_attr.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_power.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_port.attr,
	&dev_attr_led_locator.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_fan_tray_2.attr,
	&dev_attr_led_fan_tray_3.attr,
	&dev_attr_led_fan_tray_4.attr,
	&dev_attr_led_fan_tray_5.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_modsel.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_interrupt_ctrl.attr,
	&dev_attr_qsfp_i2c_enable.attr,
	NULL,
};

static struct attribute_group dell_s6010_cpld_attr_group = {
	.attrs = dell_s6010_cpld_attrs,
};

static int dell_s6010_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &dell_s6010_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_cpld_driver_group failed for cpld driver");
	}

	return ret;
}

static int dell_s6010_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver dell_s6010_cpld_driver = {
	.driver = {
		.name = "dell_s6010_cpld",
		.owner = THIS_MODULE,
	},
	.probe = dell_s6010_cpld_probe,
	.remove = dell_s6010_cpld_remove,
};

static struct platform_device *dell_s6010_cpld_device;

static int __init dell_s6010_cpld_init(void)
{
	int ret;

	if (num_cpld_i2c_devices != NO_CPLD_I2C_CLIENTS) {
		pr_err("cpld device unable to find i2c clients");
		ret = -ENODEV;
		goto err_drvr;
	}

	ret = platform_driver_register(&dell_s6010_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	dell_s6010_cpld_device = platform_device_alloc("dell_s6010_cpld", 0);
	if (!dell_s6010_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(dell_s6010_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}

	return 0;

err_dev_add:
	platform_device_put(dell_s6010_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&dell_s6010_cpld_driver);

err_drvr:
	return ret;
}



/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dell_s6010_init(void)
{
	int ret = 0;

	ret = dell_s6010_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = dell_s6010_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		return ret;
	}

	ret = dell_s6010_port_mux_init();
	if (ret) {
		pr_err("Initializing QSFP mux failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit dell_s6010_exit(void)
{
	dell_s6010_port_mux_exit();
	dell_s6010_cpld_exit();
	dell_s6010_i2c_exit();
	pr_err(DRIVER_NAME" driver unloaded\n");
}

static void __exit dell_s6010_cpld_exit(void)
{
	platform_driver_unregister(&dell_s6010_cpld_driver);
	platform_device_unregister(dell_s6010_cpld_device);
}

module_init(dell_s6010_init);
module_exit(dell_s6010_exit);

MODULE_AUTHOR("Nikhil (ndhar@cumulusnetworks.com)");
MODULE_DESCRIPTION("DELL S6010 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/*
 * dell_s3000_platform.c - DELL S3000-C2338 Platform Support.
 *
 * Author: Vidya Ravipati (vidya@cumulusnetworks.com)
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
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/sff-8436.h>

#include "platform-defs.h"

#define DRIVER_NAME        "dell_s3000"
#define DRIVER_VERSION     "0.1"

/*
 * This platform has two i2c busses:
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 *  SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 */

/* i2c bus adapter numbers for the down stream i2c busses */
enum {
	DELL_S3000_I2C_iSMT_BUS=0,
	DELL_S3000_I2C_I801_BUS,
	DELL_S3000_I2C_MASTER_SWITCH_BUS=9,
	DELL_S3000_I2C_MUX1_BUS0=10,
	DELL_S3000_I2C_MUX1_BUS1,
	DELL_S3000_I2C_MUX1_BUS2,
	DELL_S3000_I2C_MUX1_BUS3,
	DELL_S3000_I2C_MUX1_BUS4,
	DELL_S3000_I2C_MUX1_BUS5,
	DELL_S3000_I2C_MUX1_BUS6,
	DELL_S3000_I2C_MUX1_BUS7,
	DELL_S3000_I2C_MUX2_BUS0=20,
	DELL_S3000_I2C_MUX2_BUS1,
	DELL_S3000_I2C_MUX2_BUS2,
	DELL_S3000_I2C_MUX2_BUS3,
	DELL_S3000_I2C_MUX2_BUS4,
	DELL_S3000_I2C_MUX2_BUS5,
	DELL_S3000_I2C_MUX2_BUS6,
	DELL_S3000_I2C_MUX2_BUS7,
	DELL_S3000_I2C_MUX3_BUS0=30,
	DELL_S3000_I2C_MUX3_BUS1,
	DELL_S3000_I2C_MUX3_BUS2,
	DELL_S3000_I2C_MUX3_BUS3,
	DELL_S3000_I2C_MUX3_BUS4,
	DELL_S3000_I2C_MUX3_BUS5,
	DELL_S3000_I2C_MUX3_BUS6,
	DELL_S3000_I2C_MUX3_BUS7,
};
#define NUM_CPLD_I2C_CLIENTS (0)

static struct i2c_client *dell_s3000_cpld_i2c_client_list[NUM_CPLD_I2C_CLIENTS];
static int num_cpld_i2c_devices;

mk_eeprom(spd1,  50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board2, 50, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(board, 53, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_port_eeprom(port49,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_eeprom(fan1,  54, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(fan2,  54, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(fan3,  54, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(psu1,  53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,  52, 256, AT24_FLAG_IRUGO);

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode mux1_platform_modes[] = {
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX1_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux1_platform_data = {
	.modes = mux1_platform_modes,
	.num_modes = ARRAY_SIZE(mux1_platform_modes),
};

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode mux2_platform_modes[] = {
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX2_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux2_platform_data = {
	.modes = mux2_platform_modes,
	.num_modes = ARRAY_SIZE(mux2_platform_modes),
};

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode mux3_platform_modes[] = {
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS0, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS1, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS2, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS3, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS4, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS5, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS6, .deselect_on_exit = 1,
	},
	{
		.adap_id = DELL_S3000_I2C_MUX3_BUS7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data mux3_platform_data = {
	.modes = mux3_platform_modes,
	.num_modes = ARRAY_SIZE(mux3_platform_modes),
};

struct dell_s3000_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct dell_s3000_i2c_device_info dell_s3000_i2c_devices[] = {
	{
		.bus = DELL_S3000_I2C_iSMT_BUS,
		{
			.type = "pca9548",
			.addr = 0x73,
			.platform_data = &mux1_platform_data,
		}
	},
	{
		.bus = DELL_S3000_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x50),         /* DIMM */
			.platform_data = &spd1_50_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX1_BUS0,
		{
			.type = "24c64",                    /* 64 Kbit Board EEPROM */
			.addr = 0x50,
			.platform_data = &board2_50_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX1_BUS1,
		{
			I2C_BOARD_INFO("max6697", 0x1A),     /* MAX6699 Temperature Monitor */
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX1_BUS6,
		{
			.type = "pca9548",                 /* PCA-9548 Switch-1 */
			.addr = 0x71,
			.platform_data = &mux2_platform_data,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX1_BUS6,
		{
			.type = "pca9548",                /* PCA9548 Switch-2 */
			.addr = 0x72,
			.platform_data = &mux3_platform_data,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX1_BUS7,
		{
			I2C_BOARD_INFO("dummy", 0x31),       /* System Management CPLD */
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS0,
		{
			.type = "24c64",                    /* 64 Kbit EEPROM */
			.addr = 0x53,
			.platform_data = &board_53_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS1,
		{
			I2C_BOARD_INFO("max6697", 0x1A),     /* MAX6699 Temperature Monitor */
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS2,
		{
			.type = "24c02",                    /* PSU2 - 2 Kbit EEPROM */
			.addr = 0x52,
			.platform_data = &psu2_52_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS2,
		{
			I2C_BOARD_INFO("dps200", 0x5a),  /* PSU2 PMBUS */
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS3,
		{
			.type = "24c02",                    /* PSU1 - 2 Kbit EEPROM */
			.addr = 0x53,
			.platform_data = &psu1_53_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS3,
		{
			I2C_BOARD_INFO("dps200", 0x5b),  /* PSU1 PMBUS */
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS4,
		{
			.type = "24c04",                    /* SFP Port 50 EEPROM */
			.addr = 0x50,
			.platform_data = &port50_50_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS5,
		{
			.type = "24c04",                    /* SFP Port 49 EEPROM */
			.addr = 0x50,
			.platform_data = &port49_50_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS6,
		{
			.type = "24c04",                    /* SFP Port 52 EEPROM */
			.addr = 0x50,
			.platform_data = &port52_50_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX2_BUS7,
		{
			.type = "24c04",                    /* SFP Port 51 EEPROM */
			.addr = 0x50,
			.platform_data = &port51_50_at24,
		}
	},
	/* TODO: Need to visit fan1, fan2, fan3 devices */
	{
		.bus = DELL_S3000_I2C_MUX3_BUS2,
		{
			I2C_BOARD_INFO("24c64", 0x54),       /* 64 Kbit EEPROM Fan 1 */
			.platform_data = &fan1_54_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX3_BUS3,
		{
			I2C_BOARD_INFO("24c64", 0x54),       /* 64 Kbit EEPROM Fan 2 */
			.platform_data = &fan2_54_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX3_BUS4,
		{
			I2C_BOARD_INFO("24c64", 0x54),       /* 64 Kbit EEPROM Fan 3 */
			.platform_data = &fan3_54_at24,
		}
	},
	{
		.bus = DELL_S3000_I2C_MUX3_BUS5,
		{
			.type = "emc2305",
			.addr = 0x4d,
		}
	},
};

/**
 * dell_s3000_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
static struct i2c_client *dell_s3000_clients_list[ARRAY_SIZE(dell_s3000_i2c_devices)];

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

static void free_dell_s3000_i2c_data(void)
{
	free_i2c_clients(dell_s3000_clients_list, ARRAY_SIZE(dell_s3000_i2c_devices));
}

static struct i2c_client *add_i2c_client(int bus, struct i2c_board_info *board_info)
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

	for (i = 0; i < DELL_S3000_I2C_MUX1_BUS0; i++) {
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

static int populate_i2c_devices(struct dell_s3000_i2c_device_info *devices,
                                int num_devices, struct i2c_client **clients_list,
                                int iSMT_bus, int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	num_cpld_i2c_devices = 0;
	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == DELL_S3000_I2C_iSMT_BUS) {
			devices[i].bus = iSMT_bus;
		} else if (devices[i].bus == DELL_S3000_I2C_I801_BUS) {
			devices[i].bus = I801_bus;
		}
		client = add_i2c_client(devices[i].bus, &devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
		if (strcmp(devices[i].board_info.type, "dummy") == 0 && num_cpld_i2c_devices < NUM_CPLD_I2C_CLIENTS) {
			dell_s3000_cpld_i2c_client_list[num_cpld_i2c_devices] = client;
			num_cpld_i2c_devices++;
		}
	}
	return 0;

err_exit:
	return ret;
}

static int __init dell_s3000_i2c_init(void)
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
	ret = populate_i2c_devices(dell_s3000_i2c_devices, ARRAY_SIZE(dell_s3000_i2c_devices),
	                           &dell_s3000_clients_list[0], iSMT_bus, I801_bus);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_dell_s3000_i2c_data();
	return ret;
}

static void __exit dell_s3000_i2c_exit(void)
{
	free_dell_s3000_i2c_data();
}


/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dell_s3000_init(void)
{
	int ret = 0;

	ret = dell_s3000_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit dell_s3000_exit(void)
{
	dell_s3000_i2c_exit();
	pr_err(DRIVER_NAME" driver unloaded\n");
}

module_init(dell_s3000_init);
module_exit(dell_s3000_exit);

MODULE_AUTHOR("Vidya Ravipati (vidya@cumulusnetworks.com)");
MODULE_DESCRIPTION("DELL S3000 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

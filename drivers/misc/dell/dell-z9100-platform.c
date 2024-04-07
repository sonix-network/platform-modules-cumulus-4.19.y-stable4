/*
 * dell_z9100_platform.c - Dell Z9100 Platform Support
 *
 * Copyright (C) 2015, 2020 Cumulus Networks, Inc.
 * Author: Puneet Shenoy (puneet@cumulusnetworks.com)
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
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/sysfs.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/sff-8436.h>

#include "platform-defs.h"
#include "dell-z9100-cpld.h"

#define DRIVER_NAME	"dell_z9100_platform"
#define DRIVER_VERSION	"1.1"
#define Z9100_I2C_I801_BUS  0
#define Z9100_I2C_ISMT_BUS  1
#define Z9100_PORT_COUNT    34

/*
 * Dell has indicated that the devices on the PCA9541 are debug only.
 * The devices behind this mux will be managed by a SmartFusion Chip.
 * In case we need to enable these devices, define PCA9541_DEBUG 1.
 */
#define PCA9541_DEBUG 0
static struct platform_driver z9100_platform_driver;


enum {
	PORT_TYPE_NONE = 0,
	PORT_TYPE_SFP,
	PORT_TYPE_QSFP,
};

struct port_info {
	int type;  // PORT_TYPE_XXXX
	struct i2c_board_info *b;
	struct i2c_client *c;
};
static struct port_info ports_info[Z9100_PORT_COUNT];

struct z9100_device_info {
	int bus;
	struct i2c_board_info info;
	int has_port;  /* 1 for SFP, 2 for QSFP */
	int port_base;
	int port_bus;
	int num_ports;
};

#define z9100_pca9547(addr, busno)						\
	enum {								\
		Z9100_PCA9547_##addr##_0 = busno,		 \
		Z9100_PCA9547_##addr##_1,					\
		Z9100_PCA9547_##addr##_2,					\
		Z9100_PCA9547_##addr##_3,					\
		Z9100_PCA9547_##addr##_4,					\
		Z9100_PCA9547_##addr##_5,					\
		Z9100_PCA9547_##addr##_6,					\
		Z9100_PCA9547_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode z9100_mode_pca9547_##addr [] = { \
		{ .adap_id = Z9100_PCA9547_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9547_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data z9100_data_pca9547_##addr = { \
		.modes = z9100_mode_pca9547_##addr,			\
		.num_modes = ARRAY_SIZE(z9100_mode_pca9547_##addr),	\
	};

#define z9100_pca9541(addr, busno)						\
	enum {								\
		Z9100_PCA9541_##addr##_0 = busno,			\
	};								\
	static struct pca954x_platform_mode z9100_mode_pca9541_##addr [] = { \
		{ .adap_id = Z9100_PCA9541_##addr##_0, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data z9100_data_pca9541_##addr = { \
		.modes = z9100_mode_pca9541_##addr,			\
		.num_modes = ARRAY_SIZE(z9100_mode_pca9541_##addr),	\
	};

#define z9100_pca9548(addr, idx, busno)					\
	enum {								\
		Z9100_PCA9548_##idx##_##addr##_0 = busno,				\
		Z9100_PCA9548_##idx##_##addr##_1,					\
		Z9100_PCA9548_##idx##_##addr##_2,					\
		Z9100_PCA9548_##idx##_##addr##_3,					\
		Z9100_PCA9548_##idx##_##addr##_4,					\
		Z9100_PCA9548_##idx##_##addr##_5,					\
		Z9100_PCA9548_##idx##_##addr##_6,					\
		Z9100_PCA9548_##idx##_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode z9100_mode_pca9548_##idx##_##addr [] = { \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = Z9100_PCA9548_##idx##_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data z9100_data_pca9548_##idx##_##addr = { \
		.modes = z9100_mode_pca9548_##idx##_##addr,			\
		.num_modes = ARRAY_SIZE(z9100_mode_pca9548_##idx##_##addr),	\
	};

mk_eeprom(spd1, 50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 50, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan5, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(port33, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(port34, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/* I2C1 (ISMT) -> PCA9547 (0x70) */
z9100_pca9547(70, 10)   // 10-17
/* I2C1 (ISMT) -> PCA9547 (0x70) S1 -> PCA9541 (0x74) */

#if PCA9541_DEBUG
z9100_pca9541(74, 18)   // 18
/* I2C1 (ISMT) -> PCA9547 (0x70) S1 -> PCA9541 (0x74) -> PCA9548_1 (0x71) */
/* Temp Sensors and CPLD 1*/
z9100_pca9548(71, 1, 19) // 19-26
/* I2C1 (ISMT) -> PCA9547 (0x70) S1 -> PCA9541 (0x74) -> PCA9548_3 (0x72) */
/* PSU and Fan Controllers and Fan/PSU GPIOs*/
z9100_pca9548(72, 3, 35) // 35-42
#endif // PCA9541_DEBUG

/* I2C1 (ISMT) -> PCA9547 (0x70) S2 -> PCA9548_1 (0x71) */
/* CPLD 2/3/4, SFP+ EEPROM and Board EEPROM */
z9100_pca9548(71, 2, 27) // 27-34
/* I2C1 (ISMT) -> PCA9547 (0x70) S4 -> PCA9548_1 (0x71) */
/* QSFP EEPROM 1-8 */
z9100_pca9548(71, 4, 43) // 43-50
/* I2C1 (ISMT) -> PCA9547 (0x70) S5 -> PCA9548_1 (0x71) */
/* QSFP EEPROM 9-16 */
z9100_pca9548(71, 5, 51) // 51-58
/* I2C1 (ISMT) -> PCA9547 (0x70) S6 -> PCA9548_1 (0x71) */
/* QSFP EEPROM 17-24 */
z9100_pca9548(71, 6, 59) // 59-66
/* I2C1 (ISMT) -> PCA9547 (0x70) S7 -> PCA9548_1 (0x71) */
/* QSFP EEPROM 25-32 */
z9100_pca9548(71, 7, 67) // 67-74

/* I2C Device Map */
static struct z9100_device_info i801_devices[] = {
	{
		.bus = Z9100_I2C_I801_BUS,       /* SPD DIMM1 EEPROM */
		{I2C_BOARD_INFO("spd", 0x50),
		 .platform_data = &spd1_50_at24,},
	},
};

static struct z9100_device_info ismt_devices[] = {
	{
		.bus = Z9100_I2C_ISMT_BUS,
		{I2C_BOARD_INFO("pca9547", 0x70),
		 .platform_data = &z9100_data_pca9547_70,},
	},
};

static struct z9100_device_info pca9547_devices[] = {
        /* Devices on I2C1 (ISMT) -> PCA9547 (0x70) */
	{
		.bus = Z9100_PCA9547_70_0,
		{I2C_BOARD_INFO("24c02", 0x50),     /* Board EEPROM */
		 .platform_data = &board_50_at24,},
	},
#if PCA9541_DEBUG
	{
		.bus = Z9100_PCA9547_70_1,
		{I2C_BOARD_INFO("pca9541", 0x74),
		 .platform_data = &z9100_data_pca9541_74,},
	},
#endif // PCA9541_DEBUG
	{
		.bus = Z9100_PCA9547_70_2,
		{I2C_BOARD_INFO("pca9548", 0x71),
		 .platform_data = &z9100_data_pca9548_2_71,},
	},
	{
		.bus = Z9100_PCA9547_70_4,        /* QSFP28 EEPROM  25-32 */
		{I2C_BOARD_INFO("pca9548", 0x71),
		 .platform_data = &z9100_data_pca9548_4_71,},
		 .has_port = PORT_TYPE_QSFP,
		 .num_ports = 8, .port_base = 25,
		 .port_bus = Z9100_PCA9548_4_71_0,
	},
	{
		.bus = Z9100_PCA9547_70_5,      /* QSFP28 EEPROM  17-24 */
		{I2C_BOARD_INFO("pca9548", 0x71),
		 .platform_data = &z9100_data_pca9548_5_71,},
		 .has_port = PORT_TYPE_QSFP,
		 .num_ports = 8, .port_base = 17,
		 .port_bus = Z9100_PCA9548_5_71_0,
	},
	{
		.bus = Z9100_PCA9547_70_6,   /* QSFP28 EEPROM  9-16 */
		{I2C_BOARD_INFO("pca9548", 0x71),
		 .platform_data = &z9100_data_pca9548_6_71,},
		 .has_port = PORT_TYPE_QSFP,
		 .num_ports = 8, .port_base = 9,
		 .port_bus = Z9100_PCA9548_6_71_0,
	},
	{
		.bus = Z9100_PCA9547_70_7,  /* QSFP28 EEPROM  1-8 */
		{I2C_BOARD_INFO("pca9548", 0x71),
		 .platform_data = &z9100_data_pca9548_7_71,},
		 .has_port = PORT_TYPE_QSFP,
		 .num_ports = 8, .port_base = 1,
		 .port_bus = Z9100_PCA9548_7_71_0,
	},
};

#if PCA9541_DEBUG
static struct z9100_device_info pca9541_devices[] = {
        /* I2C1 (ISMT) -> PCA9547 (0x70) S1 -> PCA9541 (0x74) */
	{
		.bus = Z9100_PCA9541_74_0,
		{I2C_BOARD_INFO("lm75", 0x48),},  /* Board Temp Sensor */
	},
	{
		.bus = Z9100_PCA9541_74_0,
		{I2C_BOARD_INFO("pca9548", 0x71),
		.platform_data = &z9100_data_pca9548_1_71,},
	},
	{
		.bus = Z9100_PCA9541_74_0,
		{I2C_BOARD_INFO("pca9548", 0x72),
		.platform_data = &z9100_data_pca9548_3_72,},
	},
};

static struct z9100_device_info pca9548_3_devices[] = {
	// FIXME: Could be 0x50 for Fans.
	{
		.bus = Z9100_PCA9548_3_72_0,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &fan1_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_1,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &fan2_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_2,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &fan3_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_3,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &fan4_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_4,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &fan5_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_5,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &psu1_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_6,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &psu2_50_at24,},
	},
	{
		.bus = Z9100_PCA9548_3_72_7,
		{I2C_BOARD_INFO("emc2305", 0x2c),},
	},
	{
		.bus = Z9100_PCA9548_3_72_7,
		{I2C_BOARD_INFO("emc2305", 0x2e),},
	},
	{
		.bus = Z9100_PCA9548_3_72_7,
		{I2C_BOARD_INFO("lm75", 0x49),},
	},
};

static struct z9100_device_info pca9548_1_devices[] = {
	{
		.bus = Z9100_PCA9548_1_71_1,      /* BCM56960 Onboard 2 */
		{I2C_BOARD_INFO("lm75", 0x4a),},
	},
	{
		.bus = Z9100_PCA9548_1_71_1,      /* System Inlet 1 */
		{I2C_BOARD_INFO("lm75", 0x4b),},
	},
	{
		.bus = Z9100_PCA9548_1_71_1,      /* System Inlet 2 */
		{I2C_BOARD_INFO("lm75", 0x4c),},
	},
	{
		.bus = Z9100_PCA9548_1_71_2,
		{I2C_BOARD_INFO("lm75", 0x4a),},
	},
	{
		.bus = Z9100_PCA9548_1_71_3,      /* BCM56960 Onboard 2 */
		{I2C_BOARD_INFO("lm75", 0x4a),},
	},
	{
		.bus = Z9100_PCA9548_1_71_4,
		{I2C_BOARD_INFO("lm75", 0x4a),},
	},
	{
		.bus = Z9100_PCA9548_1_71_5,
		{I2C_BOARD_INFO("lm75", 0x4a),},
	},
};
#endif // PCA9541_DEBUG

static struct z9100_device_info pca9548_2_devices[] = {

	/* PCA9547A (0x70) -> S2 PCA9548_2 (0x71) MUX devices  */
	{
		.bus = Z9100_PCA9548_2_71_1,
		.port_bus = Z9100_PCA9548_2_71_1,
		.has_port = PORT_TYPE_SFP,
		.num_ports = 1, .port_base = 33,
	},
	{
		.bus = Z9100_PCA9548_2_71_2,
		.port_bus = Z9100_PCA9548_2_71_2,
		.has_port = PORT_TYPE_SFP,
		.num_ports = 1, .port_base = 34,
	},
	{
		.bus = Z9100_PCA9548_2_71_4, /* CPLD 2 */
		{I2C_BOARD_INFO("dummy", 0x3E),},   
	},
	{
		.bus = Z9100_PCA9548_2_71_5, /* CPLD 3 */
		{I2C_BOARD_INFO("dummy", 0x3E),},   
	},
	{
		.bus = Z9100_PCA9548_2_71_6, /* CPLD 4 */
		{I2C_BOARD_INFO("dummy", 0x3E),},   
	},
	{
		.bus = Z9100_PCA9548_2_71_7, /* CPLD 1,SmartFusion Controlled */
		{I2C_BOARD_INFO("dummy", 0x3E),},
	},

};


/* Level 1 */
static struct i2c_client *i801_clients[ARRAY_SIZE(i801_devices)];
static struct i2c_client *ismt_clients[ARRAY_SIZE(ismt_devices)];

/* Level 2 */
static struct i2c_client *pca9547_clients[ARRAY_SIZE(pca9547_devices)];
#if PCA9541_DEBUG
static struct i2c_client *pca9541_clients[ARRAY_SIZE(pca9541_devices)];
#endif // PCA9541_DEBUG
static struct i2c_client *pca9548_2_clients[ARRAY_SIZE(pca9548_2_devices)];

#if PCA9541_DEBUG
/* Level 3 */
static struct i2c_client *pca9548_1_clients[ARRAY_SIZE(pca9548_1_devices)];
static struct i2c_client *pca9548_3_clients[ARRAY_SIZE(pca9548_3_devices)];
#endif // PCA9541_DEBUG

static struct i2c_client *add_i2c_client(int bus, struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(bus);
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
static struct i2c_board_info *alloc_qsfp_board_info(int port) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct sff_8436_platform_data *sff8436_data;
	struct i2c_board_info *board_info;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
	if (!sff8436_data) {
		goto err_exit_sff8436;
	}
	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

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

static int z9100_alloc_qsfp(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		/* Port 13 - 14 and 15 - 16 are swapped */
		int port_num = port + j;
		switch (port_num) {
		case 13:
			port_num = 14;
			break;
		case 14:
			port_num = 13;
			break;
		case 15:
			port_num = 16;
			break;
		case 16:
			port_num = 15;
			break;
		default:
			break;
		}
		b_info = alloc_qsfp_board_info(port_num);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port_num);
			return -1;
		}
		c = add_i2c_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port_num);
			return -1;
		}
		ports_info[port_num - 1].type = 1;
		ports_info[port_num - 1].b = b_info;
		ports_info[port_num - 1].c = c;
	}
	return 0;
}

#define SFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_sfp_board_info(int port) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct at24_platform_data *at24_data;
	struct i2c_board_info *board_info;

	label = kzalloc(SFP_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	at24_data = kzalloc(sizeof(struct at24_platform_data), GFP_KERNEL);
	if (!at24_data) {
		goto err_exit_at24;
	}
	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

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


static int z9100_alloc_sfp(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_sfp_board_info(port + j);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port + j);
			return -1;
		}
		c = add_i2c_client(bus + j, b_info);
		if (!c) {
			free_sfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port + j);
			return -1;
		}
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}


static void z9100_free_data(void)
{
	int i;

	for(i = 0; i < Z9100_PORT_COUNT; i++) {
		if (ports_info[i].b) {
			if (ports_info[i].type == PORT_TYPE_QSFP) {
				free_qsfp_board_info(ports_info[i].b);
			} else if (ports_info[i].type == PORT_TYPE_SFP) {
				free_sfp_board_info(ports_info[i].b);
			}
		}				
		if (ports_info[i].c) {
			i2c_unregister_device(ports_info[i].c);
		}
	}
#if PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9548_3_clients); i++)
		if (pca9548_3_clients[i])
			i2c_unregister_device(pca9548_3_clients[i]);

	for (i = 0; i < ARRAY_SIZE(pca9548_1_clients); i++)
		if (pca9548_1_clients[i])
			i2c_unregister_device(pca9548_1_clients[i]);
#endif // PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9548_2_clients); i++)
		if (pca9548_2_clients[i])
			i2c_unregister_device(pca9548_2_clients[i]);
#if PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9541_clients); i++)
		if (pca9541_clients[i])
			i2c_unregister_device(pca9541_clients[i]);
#endif // PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9547_clients); i++)
		if (pca9547_clients[i])
			i2c_unregister_device(pca9547_clients[i]);

	for (i = 0; i < ARRAY_SIZE(ismt_clients); i++)
		if (ismt_clients[i])
			i2c_unregister_device(ismt_clients[i]);

	for (i = 0; i < ARRAY_SIZE(i801_clients); i++)
		if (i801_clients[i])
			i2c_unregister_device(i801_clients[i]);
}

/*************************************************/
/* BEGIN CPLD Platform Driver                    */
/*                                               */
/* This driver is responsible for the sysfs intf */
/*************************************************/

/**
 * Array of the CPLD i2c devices, used by z9100_read() /
 * z9100_write().
 */
static struct i2c_client *cpld_i2c_clients[NUM_CPLD_I2C_CLIENTS];

static int z9100_read(struct i2c_client *client, u8 addr)
{
	int ret;

        /* Dell Z9100 CPLD uses 16-bit addressing */
	ret = i2c_smbus_write_byte_data(client, 0x0, addr);
	if (ret < 0) {
		dev_err(&client->dev, "16-bit addr write failed for addr: 0x%x\n", addr);
		return ret;
	}
	ret = i2c_smbus_read_byte(client);
	if (ret < 0) {
		dev_err(&client->dev, "Read failed for addr: 0x%x\n", addr);
	}
	return ret;
}

static int z9100_write(struct i2c_client *client, u8 addr, u8 val)
{
	int ret;

        /* Dell Z9100 CPLD uses 16-bit addressing */
	ret = i2c_smbus_write_word_data(client, 0x0, ((val << 8) | addr));
	if (ret < 0) {
		dev_err(&client->dev, "Write failed for addr: 0x%x\n", addr);
	}
	return ret;
}

static ssize_t cpld_version_show(struct device * dev,
                                 struct device_attribute * dattr,
                                 char * buf)
{
	int type, rev;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_i2c_clients[attr->index];
	int ret = z9100_read(client, CPLD_ID);

	ret = z9100_read(client, CPLD_VERSION);
	if (ret < 0) {
		return ret;
	}	
	type = (ret & CPLD_VERSION_H_MASK) >> CPLD_VERSION_H_SHIFT;
	rev  = (ret & CPLD_VERSION_L_MASK)  >> CPLD_VERSION_L_SHIFT;
	return sprintf(buf, "%d.%d\n", type, rev);
}

static ssize_t cpld_id_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_i2c_clients[attr->index];
	int ret = z9100_read(client, CPLD_ID);

	if (ret < 0) {
		return ret;
	}	
	return sprintf(buf, "%d\n", ret);
}

static ssize_t cpld_board_type_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_i2c_clients[attr->index];
	int ret = z9100_read(client, CPLD_BOARD_TYPE);

	if (ret < 0) {
		return ret;
	}	
	if (ret == CPLD_BOARD_TYPE_Z9100) {
		return sprintf(buf, "Z9100\n");
	} else if (ret == CPLD_BOARD_TYPE_Z6100) {
		return sprintf(buf, "Z6100\n");
	}
	return sprintf(buf, "UNKNOWN\n");			
}

static ssize_t cpld_sw_scratch_show(struct device * dev,
				  struct device_attribute * dattr,
				  char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_i2c_clients[attr->index];
	int ret = z9100_read(client, CPLD_SW_SCRATCH);

	if (ret < 0) {
		return ret;
	}
	return sprintf(buf, "0x%x\n", (ret & CPLD_PORT_LED_OPMOD_MASK));
}
static ssize_t cpld_sw_scratch_store(struct device * dev,
				   struct device_attribute * dattr,
				   const char * buf, size_t count)
{
	int tmp, ret;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_i2c_clients[attr->index];

	if (sscanf(buf, "0x%x", &tmp) < 0) {
		return -EINVAL;
	}
	if (tmp > 0xff)  {
		return -EINVAL;
	}		       
	ret = z9100_write(client, CPLD_SW_SCRATCH, (u8)tmp);
	if (ret < 0) {
		return ret;
	}
	return count;
}

struct z9100_cpld_regs {
	u8 mask;
	u8 off;
};

static struct z9100_cpld_regs cpld_regs[] = {
	{
		.mask = CPLD2_ZQSFP_1_MASK,
		.off   = CPLD2_ZQSFP_1_OFF,
	},
	{
		.mask = CPLD2_ZQSFP_2_MASK,
		.off   = CPLD2_ZQSFP_2_OFF,
	},
	{
		.mask = CPLD3_ZQSFP_1_MASK,
		.off   = CPLD3_ZQSFP_1_OFF,
	},
	{
		.mask = CPLD3_ZQSFP_2_MASK,
		.off  = CPLD3_ZQSFP_2_OFF,
	},
	{
		.mask = CPLD4_ZQSFP_1_MASK,
		.off   = CPLD4_ZQSFP_1_OFF,
	},
	{
		.mask = CPLD4_ZQSFP_2_MASK,
		.off   = CPLD4_ZQSFP_2_OFF,
	},

};
static ssize_t show_zqsfp(struct device *dev, struct device_attribute *dattr,
			  char *buf)
{
	int ret, i;
	u32 final = 0;
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int reg = attr->index;
	int active_low = attr->nr;

	for (i = 0; i < ARRAY_SIZE(cpld_regs); i++) {
		ret = z9100_read(cpld_i2c_clients[i/2], reg + (i%2));
		if (ret < 0) {
			return ret;
		}
		final |= (((u8)ret & cpld_regs[i].mask) << cpld_regs[i].off);
	}
	if (active_low) {
		final = ~final;
	}
	return sprintf(buf, "0x%x\n", final);
}

static ssize_t store_zqsfp(struct device *dev, struct device_attribute *dattr,
			   const char *buf, size_t count)
{
	int ret, i;
	u32 final;
	u8 tmp;
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int active_low = attr->nr;
	int reg = attr->index;

	if (sscanf(buf, "%x", &final) < 0) {
		return -EINVAL;
	}
	if (final > 0xffffffff) {
		return -EINVAL;
	}
	if (active_low) {
		final = ~final;
	}
	for (i = 0; i < ARRAY_SIZE(cpld_regs); i++) {
		ret = z9100_read(cpld_i2c_clients[i/2], reg + (i%2));
		if (ret < 0) {
			return ret;
		}
		tmp = (u8)ret & ~(cpld_regs[i].mask);
		tmp |= (u8)((final >> cpld_regs[i].off) & cpld_regs[i].mask);
		ret = z9100_write(cpld_i2c_clients[i/2], reg + (i%2), tmp);
		if (ret < 0) {
			return ret;
		}
	}
	return count;
}

static ssize_t show_sfpp(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int active_low = attr->nr;
	u8 reg = attr->index;
	struct i2c_client *client = cpld_i2c_clients[CPLD_CPLD4_IDX];
	int ret = z9100_read(client, reg);

	if (ret < 0) {
		return ret;
	}
	if (active_low) {
		ret = ~ret;
	}
	return sprintf(buf, "0x%x\n", (ret & CPLD_SFPP_MASK));
}
static ssize_t store_sfpp(struct device * dev,
			 struct device_attribute * dattr,
			 const char * buf, size_t count)
{
	int ret, tmp;
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int active_low = attr->nr;
	u8 reg = attr->index;
	struct i2c_client *client = cpld_i2c_clients[CPLD_CPLD4_IDX];


	if (sscanf(buf, "%x", &tmp) < 0) {
		return -EINVAL;
	}
	if (tmp > CPLD_SFPP_MASK)  {
		return -EINVAL;
	}		       
	if (active_low) {
		tmp = ~tmp;
	}

	ret = z9100_read(client, reg);
	if (ret < 0) {
		return ret;
	}
	ret &= ~CPLD_SFPP_MASK;
	ret |= tmp;

	ret = z9100_write(client, reg, (u8)ret);
	if (ret < 0) {
		return ret;
	}
	return count;
}

static SENSOR_DEVICE_ATTR(cpld2_version, S_IRUGO, cpld_version_show, NULL, CPLD_CPLD2_IDX);
static SENSOR_DEVICE_ATTR(cpld3_version, S_IRUGO, cpld_version_show, NULL, CPLD_CPLD3_IDX);			  
static SENSOR_DEVICE_ATTR(cpld4_version, S_IRUGO, cpld_version_show, NULL, CPLD_CPLD4_IDX);

static SENSOR_DEVICE_ATTR(cpld2_id, S_IRUGO, cpld_id_show, NULL, CPLD_CPLD2_IDX);
static SENSOR_DEVICE_ATTR(cpld3_id, S_IRUGO, cpld_id_show, NULL, CPLD_CPLD3_IDX);			  
static SENSOR_DEVICE_ATTR(cpld4_id, S_IRUGO, cpld_id_show, NULL, CPLD_CPLD4_IDX);

static SENSOR_DEVICE_ATTR(cpld2_board_type, S_IRUGO, cpld_board_type_show, NULL, CPLD_CPLD2_IDX);
static SENSOR_DEVICE_ATTR(cpld3_board_type, S_IRUGO, cpld_board_type_show, NULL, CPLD_CPLD3_IDX);			  
static SENSOR_DEVICE_ATTR(cpld4_board_type, S_IRUGO, cpld_board_type_show, NULL, CPLD_CPLD4_IDX);

static SENSOR_DEVICE_ATTR(cpld2_sw_scratch, S_IRUGO, cpld_sw_scratch_show, cpld_sw_scratch_store, CPLD_CPLD2_IDX);
static SENSOR_DEVICE_ATTR(cpld3_sw_scratch, S_IRUGO, cpld_sw_scratch_show, cpld_sw_scratch_store, CPLD_CPLD3_IDX);
static SENSOR_DEVICE_ATTR(cpld4_sw_scratch, S_IRUGO, cpld_sw_scratch_show, cpld_sw_scratch_store, CPLD_CPLD4_IDX);

static SENSOR_DEVICE_ATTR_2(zqsfp_present,      S_IRUGO, show_zqsfp, NULL,        1, CPLD_ZQSFP_PRESENT_STA0);
static SENSOR_DEVICE_ATTR_2(zqsfp_reset,        S_IRUGO | S_IWUSR, show_zqsfp, store_zqsfp, 1, CPLD_ZQSFP_RESET_CTRL0);
static SENSOR_DEVICE_ATTR_2(zqsfp_lpmod,        S_IRUGO | S_IWUSR, show_zqsfp, store_zqsfp, 0, CPLD_ZQSFP_LPMOD_CTRL0);
static SENSOR_DEVICE_ATTR_2(zqsfp_int,          S_IRUGO, show_zqsfp, NULL,        1, CPLD_ZQSFP_INT_STA0);
static SENSOR_DEVICE_ATTR_2(zqsfp_int_int,      S_IRUGO, show_zqsfp, NULL,        0, CPLD_ZQSFP_INT_INT0);
static SENSOR_DEVICE_ATTR_2(zqsfp_present_int,  S_IRUGO, show_zqsfp, NULL,        0, CPLD_ZQSFP_PRESENT_INT0);
static SENSOR_DEVICE_ATTR_2(zqsfp_int_mask,     S_IRUGO | S_IWUSR, show_zqsfp, store_zqsfp, 0, CPLD_ZQSFP_INT_MASK0);
static SENSOR_DEVICE_ATTR_2(zqsfp_present_mask, S_IRUGO | S_IWUSR, show_zqsfp, store_zqsfp, 0, CPLD_ZQSFP_PRESENT_MASK0);

static SENSOR_DEVICE_ATTR_2(sfp_tx_disable,  S_IRUGO | S_IWUSR, show_sfpp, store_sfpp, 0, CPLD_SFPP_TXDISABLE_CTRL);
static SENSOR_DEVICE_ATTR_2(sfp_rs_ctrl,     S_IRUGO | S_IWUSR, show_sfpp, store_sfpp, 0, CPLD_SFPP_RS_CTRL);
static SENSOR_DEVICE_ATTR_2(sfp_rxlos,       S_IRUGO          , show_sfpp, NULL,       0, CPLD_SFPP_RXLOS_STA);
static SENSOR_DEVICE_ATTR_2(sfp_tx_fault,    S_IRUGO          , show_sfpp, NULL,       1, CPLD_SFPP_TXFAULT_STA);
static SENSOR_DEVICE_ATTR_2(sfp_present,     S_IRUGO          , show_sfpp, NULL,       1, CPLD_SFPP_PRESENT_STA);
static SENSOR_DEVICE_ATTR_2(sfp_rxlos_int,   S_IRUGO          , show_sfpp, NULL,       0, CPLD_SFPP_RXLOS_INT);
static SENSOR_DEVICE_ATTR_2(sfp_present_int, S_IRUGO          , show_sfpp, NULL,       0, CPLD_SFPP_PRESENT_INT);
static SENSOR_DEVICE_ATTR_2(sfp_rxlos_mask,  S_IRUGO | S_IWUSR, show_sfpp, store_sfpp, 0, CPLD_SFPP_RXLOS_MASK);
static SENSOR_DEVICE_ATTR_2(sfp_present_mask,S_IRUGO | S_IWUSR, show_sfpp, store_sfpp, 0, CPLD_SFPP_PRESENT_MASK);


static struct attribute *z9100_cpld_attrs[] = {
	&sensor_dev_attr_cpld2_version.dev_attr.attr,
	&sensor_dev_attr_cpld3_version.dev_attr.attr,
	&sensor_dev_attr_cpld4_version.dev_attr.attr,

	&sensor_dev_attr_cpld2_id.dev_attr.attr,
	&sensor_dev_attr_cpld3_id.dev_attr.attr,
	&sensor_dev_attr_cpld4_id.dev_attr.attr,

	&sensor_dev_attr_cpld2_board_type.dev_attr.attr,
	&sensor_dev_attr_cpld3_board_type.dev_attr.attr,
	&sensor_dev_attr_cpld4_board_type.dev_attr.attr,

	&sensor_dev_attr_cpld2_sw_scratch.dev_attr.attr,
	&sensor_dev_attr_cpld3_sw_scratch.dev_attr.attr,
	&sensor_dev_attr_cpld4_sw_scratch.dev_attr.attr,

	&sensor_dev_attr_zqsfp_present.dev_attr.attr,
	&sensor_dev_attr_zqsfp_reset.dev_attr.attr,
	&sensor_dev_attr_zqsfp_lpmod.dev_attr.attr,
	&sensor_dev_attr_zqsfp_int.dev_attr.attr,
	&sensor_dev_attr_zqsfp_int_int.dev_attr.attr,
	&sensor_dev_attr_zqsfp_present_int.dev_attr.attr,
	&sensor_dev_attr_zqsfp_int_mask.dev_attr.attr,
	&sensor_dev_attr_zqsfp_present_mask.dev_attr.attr,

	&sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_rs_ctrl.dev_attr.attr,
	&sensor_dev_attr_sfp_rxlos.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_present.dev_attr.attr,
	&sensor_dev_attr_sfp_rxlos_int.dev_attr.attr,
	&sensor_dev_attr_sfp_present_int.dev_attr.attr,
	&sensor_dev_attr_sfp_rxlos_mask.dev_attr.attr,
	&sensor_dev_attr_sfp_present_mask.dev_attr.attr,

	NULL,
};

static struct attribute_group z9100_cpld_attr_group = {
	.attrs = z9100_cpld_attrs,
};

static int z9100_cpld_probe(struct platform_device *dev)
{
	s32 ret = 0;

	ret = sysfs_create_group(&dev->dev.kobj, &z9100_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
		return ret;
	}
	return 0;
}

static int z9100_cpld_remove(struct platform_device *dev)
{
	sysfs_remove_group(&dev->dev.kobj, &z9100_cpld_attr_group);
	return 0;
}

static struct platform_driver z9100_cpld_driver = {
	.driver = {
		.name = "z9100_cpld",
		.owner = THIS_MODULE,
	},
	.probe = z9100_cpld_probe,
	.remove = z9100_cpld_remove,
};

static struct platform_device *z9100_cpld_device;

/**
 * z9100_i2c_init -- CPLD I2C devices
 *
 * Create a device that provides generic access to the CPLD registers.
 */
static int __init z9100_cpld_init(void)
{
	int i;
	int ret;

	/* Verify we found them all */
	for (i = 0; i < ARRAY_SIZE(cpld_i2c_clients); i++) {
		if (cpld_i2c_clients[i] == NULL) {
			pr_err("Unable to find all CPLD I2C devices.  "
			       "Missing cpld_i2c_clients[%d]\n", i);
			return -ENODEV;
		}
	}

	ret = platform_driver_register(&z9100_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	z9100_cpld_device = platform_device_alloc("z9100_cpld", 0);
	if (!z9100_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(z9100_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(z9100_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&z9100_cpld_driver);

err_drvr:
	return ret;
}

static int __init z9100_i2c_init(void)
{
	struct i2c_adapter *adapter;
	int ret = -1;
	int i801_bus = -1;
	int ismt_bus = -1;
	int i;


	/* Find the I801 bus */
	for (i = 0; i < 5; i++) {
		adapter = i2c_get_adapter(i);
		if (adapter) {
			if (!strncmp(adapter->name, I801_ADAPTER_NAME, strlen(I801_ADAPTER_NAME)))
				i801_bus = i;
			i2c_put_adapter(adapter);
		}
		if (i801_bus >= 0)
			break;
	}
	if (i801_bus < 0) {
		pr_err("Unable to find %s\n", I801_ADAPTER_NAME);
		return -ENXIO;
	}

	/* Find the iSMT bus */
	for (i = 0; i < 5; i++) {
		adapter = i2c_get_adapter(i);
		if (adapter) {
			if (!strncmp(adapter->name, ISMT_ADAPTER_NAME, strlen(ISMT_ADAPTER_NAME)))
				ismt_bus = i;
			i2c_put_adapter(adapter);
		}
		if (ismt_bus >= 0)
			break;
	}
	if (ismt_bus < 0) {
		pr_err("Unable to find %s\n", ISMT_ADAPTER_NAME);
		return -ENXIO;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i801_devices); i++) {
		i801_devices[i].bus = i801_bus;
		i801_clients[i] = add_i2c_client(i801_devices[i].bus,
						 &i801_devices[i].info);
		if (IS_ERR(i801_clients[i])) {
			ret = PTR_ERR(i801_clients[i]);
			goto err_exit;
		}
	}
	for (i = 0; i < ARRAY_SIZE(ismt_devices); i++) {
		ismt_devices[i].bus = ismt_bus;
		ismt_clients[i] = add_i2c_client(ismt_devices[i].bus,
						 &ismt_devices[i].info);
		if (IS_ERR(ismt_clients[i])) {
			ret = PTR_ERR(ismt_clients[i]);
			goto err_exit;
		}
	}
	for (i = 0; i < ARRAY_SIZE(pca9547_devices); i++) {
		pca9547_clients[i] = add_i2c_client(pca9547_devices[i].bus,
						 &pca9547_devices[i].info);
		if (IS_ERR(pca9547_clients[i])) {
			ret = PTR_ERR(pca9547_clients[i]);
			goto err_exit;
		}
		if (pca9547_devices[i].has_port == PORT_TYPE_QSFP) {
			z9100_alloc_qsfp(pca9547_devices[i].port_base,
					 pca9547_devices[i].port_bus,
					 pca9547_devices[i].num_ports);
		}

	}
#if PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9541_devices); i++) {
		pca9541_clients[i] = add_i2c_client(pca9541_devices[i].bus,
						 &pca9541_devices[i].info);
		if (IS_ERR(pca9541_clients[i])) {
			ret = PTR_ERR(pca9541_clients[i]);
			goto err_exit;
		}
	}
#endif // PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9548_2_devices); i++) {
		if (pca9548_2_devices[i].has_port == PORT_TYPE_SFP) {
			z9100_alloc_sfp(pca9548_2_devices[i].port_base,
					pca9548_2_devices[i].port_bus,
					pca9548_2_devices[i].num_ports);
		} else {		
			pca9548_2_clients[i] = add_i2c_client(pca9548_2_devices[i].bus,
							      &pca9548_2_devices[i].info);

			if (IS_ERR(pca9548_2_clients[i])) {
				ret = PTR_ERR(pca9548_2_clients[i]);
				goto err_exit;
			}

			switch (pca9548_2_devices[i].bus) {
			case Z9100_PCA9548_2_71_4:
				cpld_i2c_clients[CPLD_CPLD2_IDX] = pca9548_2_clients[i];
				break;			
			case Z9100_PCA9548_2_71_5:
				cpld_i2c_clients[CPLD_CPLD3_IDX] = pca9548_2_clients[i];
				break;
			
			case Z9100_PCA9548_2_71_6:
				cpld_i2c_clients[CPLD_CPLD4_IDX] = pca9548_2_clients[i];
				break;
			default:
				continue;
			}
		}
	}
#if PCA9541_DEBUG
	for (i = 0; i < ARRAY_SIZE(pca9548_1_devices); i++) {
		pca9548_1_clients[i] = add_i2c_client(pca9548_1_devices[i].bus,
						 &pca9548_1_devices[i].info);
		if (IS_ERR(pca9548_1_clients[i])) {
			ret = PTR_ERR(pca9548_1_clients[i]);
			goto err_exit;
		}
	}
	for (i = 0; i < ARRAY_SIZE(pca9548_3_devices); i++) {
		pca9548_3_clients[i] = add_i2c_client(pca9548_3_devices[i].bus,
						 &pca9548_3_devices[i].info);
		if (IS_ERR(pca9548_3_clients[i])) {
			ret = PTR_ERR(pca9548_3_clients[i]);
			goto err_exit;
		}
	}
#endif // PCA9541_DEBUG
	return 0;
err_exit:
	z9100_free_data();
	return ret;
}

static int __init z9100_platform_init(void)
{
	int ret = 0;
	ret = z9100_i2c_init();
	if (ret) {
		pr_info(DRIVER_NAME": version "DRIVER_VERSION" i2c init failed\n");
		return ret;
	}

	ret = z9100_cpld_init();
	if (ret) {		
		pr_info(DRIVER_NAME": version "DRIVER_VERSION" cpld init failed\n");
		z9100_free_data();
		return ret;
	}


	ret = platform_driver_register(&z9100_platform_driver);
	if (ret) {
		pr_info(DRIVER_NAME": version "DRIVER_VERSION" driver registration failed\n");
		platform_driver_unregister(&z9100_cpld_driver);
		platform_device_unregister(z9100_cpld_device);
		z9100_free_data();
		return ret;
	}
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit z9100_platform_exit(void)
{
	platform_driver_unregister(&z9100_cpld_driver);
	platform_device_unregister(z9100_cpld_device);	
	z9100_free_data();
	platform_driver_unregister(&z9100_platform_driver);
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" unloaded\n");
}

static int z9100_platform_probe(struct platform_device *dev)
{
	return 0;
}

static int z9100_platform_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver z9100_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = z9100_platform_probe,
	.remove = z9100_platform_remove,
};

module_init(z9100_platform_init);
module_exit(z9100_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell Z9100 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

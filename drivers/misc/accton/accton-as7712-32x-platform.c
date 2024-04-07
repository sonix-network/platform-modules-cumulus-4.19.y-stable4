/*
 * accton_as7712_32x_platform.c - Accton as7712-32x Platform Support.
 *
 * Copyright 2016 Cumulus Networks, Inc.
 * Author: Puneet Shenoy (puneet@cumulusnetworks.com)
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
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "accton-as7712-32x-cpld.h"

#define DRIVER_NAME	"accton_as7712_32x_platform"
#define DRIVER_VERSION	"0.1"

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).  ISMT has a
 * PCA9548 mux.
 */

/* i2c bus adapter numbers for the 8 down stream PCA9548 i2c busses */
enum {
	AS7712_I2C_ISMT_BUS = 0,
	AS7712_I2C_I801_BUS,
	AS7712_I2C_MUX_BUS_0,
};

mk_eeprom(spd1, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2, 53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1, 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

#define AS7712_PORT_COUNT 32
#define PORT_EEPROMS_PER_MUX 8

static struct i2c_client *as7712_cpld_client_list[NUM_CPLD_I2C_CLIENTS];

struct as7712_device_info {
	int bus;
	struct i2c_board_info board_info;
	int has_port_eeprom;
	int port_bus;
	int port_order[PORT_EEPROMS_PER_MUX];
};

struct port_info {
	struct i2c_board_info *b;
	struct i2c_client *c;
};

static struct port_info ports_info[AS7712_PORT_COUNT];

#define as7712_pca9548(addr, busno)					\
	enum {								\
		AS7712_MUX_##addr##_0 = busno,				\
		AS7712_MUX_##addr##_1,					\
		AS7712_MUX_##addr##_2,					\
		AS7712_MUX_##addr##_3,					\
		AS7712_MUX_##addr##_4,					\
		AS7712_MUX_##addr##_5,					\
		AS7712_MUX_##addr##_6,					\
		AS7712_MUX_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode as7712_mode_pca9548_##addr[] = { \
		{ .adap_id = AS7712_MUX_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = AS7712_MUX_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data as7712_data_pca9548_##addr = { \
		.modes = as7712_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(as7712_mode_pca9548_##addr),	\
	}								\

/* I2C ISMT -> PCA9548(0x71) */
as7712_pca9548(71, 10);
/* I2C ISMT -> PCA9548(0x72) */
as7712_pca9548(72, 18);
/* I2C ISMT -> PCA9548(0x73) */
as7712_pca9548(73, 26);
/* I2C ISMT -> PCA9548(0x74) */
as7712_pca9548(74, 34);
/* I2C ISMT -> PCA9548(0x75) */
as7712_pca9548(75, 42);
/* I2C i801 -> PCA9548(0x76) */
as7712_pca9548(76, 50);

static struct as7712_device_info i2c_devices[] = {
	/* Begin I2C_ISMT Bus */
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("24c02", 0x57),	/* Board EEPROM */
			.platform_data = &board_57_at24,
		},
	},
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x71),
			.platform_data = &as7712_data_pca9548_71,
		},
	},
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x72),
			.platform_data = &as7712_data_pca9548_72,
		},
		.has_port_eeprom = 1,
		.port_bus = AS7712_MUX_72_0,
		.port_order = {9, 10, 11, 12, 1, 2, 3, 4},
	},
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x73),
			.platform_data = &as7712_data_pca9548_73,
		},
		.has_port_eeprom = 1,
		.port_bus = AS7712_MUX_73_0,
		.port_order = {6, 5, 8, 7, 13, 14, 15, 16},
	},
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x74),
			.platform_data = &as7712_data_pca9548_74,
		},
		.has_port_eeprom = 1,
		.port_bus = AS7712_MUX_74_0,
		.port_order = {17, 18, 19, 20, 25, 26, 27, 28},
	},
	{
		.bus = AS7712_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x75),
			.platform_data = &as7712_data_pca9548_75,
		},
		.has_port_eeprom = 1,
		.port_bus = AS7712_MUX_75_0,
		.port_order = {29, 30, 31, 32, 21, 22, 23, 24},
	},
	{
		.bus = AS7712_MUX_71_1,
		{
			I2C_BOARD_INFO("24c02", 0x53),
			.platform_data = &psu1_53_at24,
		},
	},
	{
		.bus = AS7712_MUX_71_0,
		{
			I2C_BOARD_INFO("cpr4011", 0x58),
		},
	},
	{
		.bus = AS7712_MUX_71_0,
		{
			I2C_BOARD_INFO("24c02", 0x50),
			.platform_data = &psu2_50_at24,
		},
	},
	{
		.bus = AS7712_MUX_71_1,
		{
			I2C_BOARD_INFO("cpr4011", 0x5b),
		},
	},
	/* Begin I2C_I801 Bus */
	{
		.bus = AS7712_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x52),  /* DIMM */
			.platform_data = &spd1_52_at24,
		},
	},
	{
		.bus = AS7712_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x53),  /* DIMM */
			.platform_data = &spd2_53_at24,
		},
	},
	{
		.bus = AS7712_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x76),
			.platform_data = &as7712_data_pca9548_76,
		},
	},
	{
		.bus = AS7712_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x48),
		},
	},
	{
		.bus = AS7712_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x49),
		},
	},
	{
		.bus = AS7712_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4a),
		},
	},
	{
		.bus = AS7712_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4b),
		},
	},
	{
		.bus = AS7712_MUX_76_0,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x66), /* FAN CPLD */
		},
	},
	{
		.bus = AS7712_MUX_76_2,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x60), /* CPLD1 */
		},
	},
	{
		.bus = AS7712_MUX_76_3,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x62), /* CPLD2 */
		},
	},
	{
		.bus = AS7712_MUX_76_4,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x64), /* CPLD3 */
		},
	},
};

/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
static struct i2c_client *i2c_clients[ARRAY_SIZE(i2c_devices)];

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

	eeprom_data = kzalloc(sizeof(*eeprom_data),
			      GFP_KERNEL);
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

static int accton_as7712_32_alloc_qsfp(int port_bus, int *port_order)
{
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < PORT_EEPROMS_PER_MUX; j++) {
		int port = port_order[j];

		b_info = alloc_qsfp_board_info(port);
		if (!b_info) {
			pr_err("could not allocate board info: %d\n", port);
			return -1;
		}
		c = add_i2c_client(port_bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n",
			       b_info->type, port);
			return -1;
		}
		ports_info[port].b = b_info;
		ports_info[port].c = c;
	}
	return 0;
}

static void free_i2c_data(void)
{
	int i;

	for (i = 0; i < AS7712_PORT_COUNT; i++) {
		if (ports_info[i].b)
			free_qsfp_board_info(ports_info[i].b);
		if (ports_info[i].c)
			i2c_unregister_device(ports_info[i].c);
	}
	/*
	 * Free the devices in reverse order so that child devices are
	 * freed before parent mux devices.
	 */
	for (i = ARRAY_SIZE(i2c_devices) - 1; i >= 0; i--)
		i2c_unregister_device(i2c_clients[i]);
}

static int __init get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < AS7712_I2C_MUX_BUS_0; i++) {
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
 * accton_as7712_32x_i2c_init -- Initialize I2C devices
 *
 */
static int __init accton_as7712_32x_i2c_init(void)
{
	struct i2c_client *client;
	int ret;
	int i;

	ret = -1;

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
		 * Map logical buses S7712_I2C_ISMT_BUS and
		 * AS7712_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case AS7712_I2C_ISMT_BUS:
			i2c_devices[i].bus = ismt_bus_num;
			break;
		case AS7712_I2C_I801_BUS:
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
		i2c_clients[i] = client;
		if (i2c_devices[i].has_port_eeprom)
			accton_as7712_32_alloc_qsfp(i2c_devices[i].port_bus,
						    i2c_devices[i].port_order);
	}

	return 0;

err_exit:
	free_i2c_data();
	return ret;
}

static void __exit accton_as7712_32x_i2c_exit(void)
{
	free_i2c_data();
}

/*******************************************************
 *
 *                     CPLD Driver
 *
 ********************************************************/

/**
 * cpld_reg_read - Read an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte
 * received from the device.
 */
static s32
cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int val;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	val = i2c_smbus_read_byte_data(as7712_cpld_client_list[cpld_idx],
				       STRIP_CPLD_IDX(reg));
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       as7712_cpld_client_list[cpld_idx]->addr,
		       STRIP_CPLD_IDX(reg));
	}
	return val;
}

/**
 * cpld_reg_write - Writes an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */
static s32
cpld_reg_write(uint32_t reg, uint8_t val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to write to invalid CPLD register [0x%02X]",
		       reg);
		return -EINVAL;
	}

	res = i2c_smbus_write_byte_data(as7712_cpld_client_list[cpld_idx],
					STRIP_CPLD_IDX(reg), val);
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       as7712_cpld_client_list[cpld_idx]->addr,
		       reg, val);
	}
	return res;
}

/*
 * Board Info for CPLD1 and Fan CPLD
 */
static ssize_t
board_info_show(struct device *dev,
		struct device_attribute *dattr,
		char *buf)
{
	s32  val_cpld1, val_fan_cpld;

	buf[0]       = '\0';
	val_cpld1    = cpld_reg_read(CPLD_BOARD_INFO_REG);
	val_fan_cpld = cpld_reg_read(FAN_CPLD_BOARD_INFO_REG);
	if (val_cpld1 >= 0) {
		if ((val_cpld1 & CPLD_BOARD_INFO_PCB_ID_MASK) == 0)
			sprintf(buf, "CPLD1_PCB_ID: ES7632BT\n");
		switch (val_cpld1 & CPLD_BOARD_INFO_PCB_VER_MASK) {
		case 0:
			strcat(buf, "CPLD1_PCB_VER: R0A\n");
			break;
		case 1:
			strcat(buf, "CPLD1_PCB_VER: R0B\n");
			break;
		case 2:
			strcat(buf, "CPLD1_PCB_VER: R0C\n");
			break;
		}
	}
	if (val_fan_cpld >= 0) {
		switch (val_fan_cpld & FAN_CPLD_BOARD_INFO_VER_ID_MASK) {
		case 0:
			strcat(buf, "FAN_CPLD_VER_ID: R0A\n");
			break;
		case 32:
			strcat(buf, "FAN_CPLD_VER_ID: R0B\n");
			break;
		case 128:
			strcat(buf, "FAN_CPLD_VER_ID: R01\n");
			break;
		}
		switch (val_fan_cpld & FAN_CPLD_BOARD_INFO_BOARD_ID_MASK) {
		case 0:
			strcat(buf, "FAN_CPLD_BOARD_ID: 5 fan board\n");
			break;
		case 3:
			strcat(buf, "FAN_CPLD_BOARD_ID: 6 fan board\n");
			break;
		}
	}
	return strlen(buf);
}

static SYSFS_ATTR_RO(board_info, board_info_show);

/*
 * CPLD Version
 */
static ssize_t
cpld_version_show(struct device *dev,
		  struct device_attribute *dattr,
		  char *buf)
{
	s32 val_cpld1, val_cpld2, val_cpld3, val_fan_cpld;
	s32 num_bytes;

	val_cpld1	 = cpld_reg_read(CPLD_CPLD1_VERSION_REG);
	val_cpld2	 = cpld_reg_read(CPLD_CPLD2_VERSION_REG);
	val_cpld3	 = cpld_reg_read(CPLD_CPLD3_VERSION_REG);
	val_fan_cpld = cpld_reg_read(FAN_CPLD_VERSION_REG);

	num_bytes  = sprintf(buf, "CPLD1 Ver: %d\n", val_cpld1);
	num_bytes += sprintf(buf + num_bytes, "CPLD2 Ver: %d\n", val_cpld2);
	num_bytes += sprintf(buf + num_bytes, "CPLD3 Ver: %d\n", val_cpld3);
	num_bytes += sprintf(buf + num_bytes, "Fan CPLD Ver: %d\n",
				val_fan_cpld);

	return num_bytes;
}
static SYSFS_ATTR_RO(cpld_version, cpld_version_show);

static ssize_t
bulk_power_show(struct device *dev,
		struct device_attribute *dattr,
		char *buf)
{
	uint8_t read_val;
	uint8_t psu_present;
	uint8_t pwr_ok;

	read_val = (cpld_reg_read(CPLD_CPLD1_PSU_STATUS_REG));
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		psu_present = CPLD_CPLD1_PSU1_STATUS_PRESENT_MASK;
		pwr_ok	    = (CPLD_CPLD1_PSU1_STATUS_AC_ALERT_MASK |
				CPLD_CPLD1_PSU1_STATUS_12V_GOOD_MASK);
	} else {
		psu_present = CPLD_CPLD1_PSU2_STATUS_PRESENT_MASK;
		pwr_ok	    = (CPLD_CPLD1_PSU2_STATUS_AC_ALERT_MASK |
				CPLD_CPLD1_PSU2_STATUS_12V_GOOD_MASK);
	}

	if (!(read_val & psu_present)) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(read_val & pwr_ok))
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

static ssize_t
psu_led_show(struct device *dev,
	     struct device_attribute *dattr,
	     char *buf)
{
	uint8_t read_val;
	uint8_t psu_present;
	uint8_t pwr_ok;

	read_val = (cpld_reg_read(CPLD_CPLD1_PSU_STATUS_REG));
	if (strcmp(dattr->attr.name, "led_psu1") == 0) {
		psu_present = CPLD_CPLD1_PSU1_STATUS_PRESENT_MASK;
		pwr_ok	    = (CPLD_CPLD1_PSU1_STATUS_AC_ALERT_MASK |
				CPLD_CPLD1_PSU1_STATUS_12V_GOOD_MASK);
	} else {
		psu_present = CPLD_CPLD1_PSU2_STATUS_PRESENT_MASK;
		pwr_ok	    = (CPLD_CPLD1_PSU2_STATUS_AC_ALERT_MASK |
				CPLD_CPLD1_PSU2_STATUS_12V_GOOD_MASK);
	}

	if (!(read_val & psu_present)) {
		if (!(read_val & pwr_ok))
			strcat(buf, PLATFORM_LED_RED);
		else
			strcat(buf, PLATFORM_LED_GREEN);
	} else { /* Not Present */
		sprintf(buf, PLATFORM_LED_OFF);
	}
	strcat(buf, "\n");

	return strlen(buf);
}

static SYSFS_ATTR_RO(led_psu1, psu_led_show);
static SYSFS_ATTR_RO(led_psu2, psu_led_show);

/*****************************************************
 *
 *		QSFP status definitions
 *
 *****************************************************/
struct qsfp_status {
	char name[ACCTON_AS7712_CPLD_STRING_NAME_SIZE];
	uint8_t active_low;
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
		.name = "present",
		.active_low = 1,
	},
	{
		.name = "reset",
		.active_low = 1,
	},
	{
		.name = "interrupt_ctrl",
		.active_low = 1,
	},
	{
		.name = "interrupt_mask",
		.active_low = 0,
	},
};

static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

static ssize_t
qsfp_show(struct device *dev,
	  struct device_attribute *dattr,
	  char *buf)
{
	int i;
	s32 val = 0;
	s32 reg_1 = 0, reg_2 = 0, reg_3 = 0, reg_4 = 0;
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
	if (!target)
		return -EINVAL;

	/* If modsel register, read all of them
	 * and present as a 32 bit bitmap */
	 if (strcmp(target->name, "present") == 0) {
		reg_1 = cpld_reg_read(CPLD_CPLD1_QSFP_1_8_PRESENT_REG);
		reg_2 = cpld_reg_read(CPLD_CPLD1_QSFP_9_16_PRESENT_REG);
		reg_3 = cpld_reg_read(CPLD_CPLD1_QSFP_17_24_PRESENT_REG);
		reg_4 = cpld_reg_read(CPLD_CPLD1_QSFP_25_32_PRESENT_REG);
	 } else if (strcmp(target->name, "reset") == 0) {
		reg_1 = cpld_reg_read(CPLD_CPLD1_QSFP_1_8_RESET_REG);
		reg_2 = cpld_reg_read(CPLD_CPLD1_QSFP_9_16_RESET_REG);
		reg_3 = cpld_reg_read(CPLD_CPLD1_QSFP_17_24_RESET_REG);
		reg_4 = cpld_reg_read(CPLD_CPLD1_QSFP_25_32_RESET_REG);
	 } else if (strcmp(target->name, "interrupt_ctrl") == 0) {
		reg_1 = cpld_reg_read(
				CPLD_CPLD1_QSFP_1_8_INTERRUPT_STATUS_REG);
		reg_2 = cpld_reg_read(
				CPLD_CPLD1_QSFP_9_16_INTERRUPT_STATUS_REG);
		reg_3 = cpld_reg_read(
				CPLD_CPLD1_QSFP_17_24_INTERRUPT_STATUS_REG);
		reg_4 = cpld_reg_read(
				CPLD_CPLD1_QSFP_25_32_INTERRUPT_STATUS_REG);
	 } else if (strcmp(target->name, "interrupt_mask") == 0) {
		reg_1 = cpld_reg_read(CPLD_CPLD1_QSFP_1_8_INTERRUPT_MASK_REG);
		reg_2 = cpld_reg_read(CPLD_CPLD1_QSFP_9_16_INTERRUPT_MASK_REG);
		reg_3 = cpld_reg_read(CPLD_CPLD1_QSFP_17_24_INTERRUPT_MASK_REG);
		reg_4 = cpld_reg_read(CPLD_CPLD1_QSFP_25_32_INTERRUPT_MASK_REG);
	} else {
		pr_err("Invalid CPLD register");
	}

	val = reg_1 | (reg_2 << 8) | (reg_3 << 16) | (reg_4 << 24);

	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%08x\n", val);
}

static int
hex_to_int64(const char *hex_str, uint64_t *val)
{
	return (kstrtoull(hex_str, 0, val) != 0);
}

static ssize_t
qsfp_store(struct device *dev,
	   struct device_attribute *dattr,
	   const char *buf, size_t count)
{
	int i, ret = -1;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	uint64_t val64;
	uint8_t val_1_8, val_9_16, val_17_24, val_25_32;
	struct qsfp_status *target = NULL;

	if (hex_to_int64(buf, &val64))
		return -EINVAL;

	val_1_8   = val64 & 0xff;
	val_9_16  = (val64 >> 8) & 0xff;
	val_17_24 = (val64 >> 16) & 0xff;
	val_25_32 = (val64 >> 24) & 0xff;

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

	if (target->active_low) {
		val_1_8   = ~val_1_8;
		val_9_16  = ~val_9_16;
		val_17_24 = ~val_17_24;
		val_25_32 = ~val_25_32;
	}

	if (strcmp(target->name, "reset") == 0) {
		if ((cpld_reg_write(CPLD_CPLD1_QSFP_1_8_RESET_REG,
				    val_1_8)) < 0 ||
			(cpld_reg_write(CPLD_CPLD1_QSFP_9_16_RESET_REG,
					val_9_16)) < 0 ||
			(cpld_reg_write(CPLD_CPLD1_QSFP_17_24_RESET_REG,
					val_17_24)) < 0 ||
			(cpld_reg_write(CPLD_CPLD1_QSFP_25_32_RESET_REG,
					val_25_32)) < 0) {
			pr_err("CPLD reset register write failed");
			goto done;
		}
	} else {
		pr_err("Invalid CPLD register");
		goto done;
	}
	ret = 0;

done:
	if (ret < 0)
		return ret;
	return count;
}

SYSFS_ATTR_RW(qsfp_reset,		qsfp_show, qsfp_store);
SYSFS_ATTR_RO(qsfp_present,		qsfp_show);
SYSFS_ATTR_RO(qsfp_interrupt_ctrl,	qsfp_show);
SYSFS_ATTR_RO(qsfp_interrupt_mask,	qsfp_show);

/*****************************************************
 *
 *		Fan CPLD definitions
 *
 *****************************************************/
static ssize_t
pwm1_show(struct device *dev,
	  struct device_attribute *dattr,
	  char *buf)
{
	s32 val;
	s32 ret_val;

	val = cpld_reg_read(FAN_CPLD_FAN_MOD_PWM_REG);
	if (val < 0)
		return val;
	/* isolate least significant nibble */
	val = val & FAN_CPLD_FAN_MOD_PWM_MASK;

	/* The PWM register contains a value between 4 and 15
	 * inclusive, representing the fan duty cycle in 6.25%
	 * increments.	A value of 0xf is 100% duty cycle.
	 * For hwmon devices map the pwm value into the range 0 to
	 * 255.
	 *
	 * 255 / 15 = 17, so with integer multiply by 17
	 *
	 */
	ret_val = (val * 17);
	return sprintf(buf, "%d\n", ret_val);
}

static ssize_t
pwm1_store(struct device *dev,
	   struct device_attribute *dattr,
	   const char *buf, size_t count)
{
	int ret;
	uint32_t pwm = 0;

	if (kstrtouint(buf, 0, &pwm) < 0)
		return -EINVAL;

	pwm = clamp_val(pwm, 0, 255);
	/* Convert to a value b/w 4 and 15
	 * for writing to the register.
	 * For conversion, see above */
	pwm = pwm / 17;
	/* Minimum value of 4 is enforced */
	pwm = (pwm < 4) ? 4 : pwm;
	ret = cpld_reg_write(FAN_CPLD_FAN_MOD_PWM_REG, pwm);
	if (ret < 0)
		return ret;
	return count;
}
static SYSFS_ATTR_RW(pwm1, pwm1_show, pwm1_store);
/*
 * Fan tachometers - each fan module contains two fans - Front and Rear.
 * Map the sysfs files to the Accton fan names as follows:
 *
 *	 sysfs Name | Accton Specification
 *	 ===========+=====================
 *	 fan01	| FAN1
 *	 fan02	| REAR FAN1
 *	 fan03	| FAN2
 *	 fan04	| REAR FAN2
 *	 fan05	| FAN3
 *	 fan06	| REAR FAN3
 *	 fan07	| FAN4
 *	 fan08	| REAR FAN4
 *	 fan09	| FAN5
 *	 fan10	| REAR FAN5
 *	 fan11	| FAN6
 *	 fan12	| REAR FAN6
 */
static ssize_t
fan_tach_show(struct device *dev,
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
		reg = FAN_CPLD_FAN_1_SPEED_REG + (((fan + 1) / 2) - 1);
	else
		reg = FAN_CPLD_REAR_FAN_1_SPEED_REG + ((fan / 2) - 1);

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	rpm = (uint8_t)val * 100;

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

static ssize_t
fan_show(struct device *dev,
	 struct device_attribute *dattr,
	 char *buf)
{
	int		reg = FAN_CPLD_FAN_MOD_PRESENT_REG;
	int		val = 0;
	uint8_t fan_present = 0;

	if (strcmp(dattr->attr.name, "fan_1") == 0) {
		fan_present = FAN_CPLD_FAN_1_PRESENT;
	} else if (strcmp(dattr->attr.name, "fan_2") == 0) {
		fan_present = FAN_CPLD_FAN_2_PRESENT;
	} else if (strcmp(dattr->attr.name, "fan_3") == 0) {
		fan_present = FAN_CPLD_FAN_3_PRESENT;
	} else if (strcmp(dattr->attr.name, "fan_4") == 0) {
		fan_present = FAN_CPLD_FAN_4_PRESENT;
	} else if (strcmp(dattr->attr.name, "fan_5") == 0) {
		fan_present = FAN_CPLD_FAN_5_PRESENT;
	} else if (strcmp(dattr->attr.name, "fan_6") == 0) {
		fan_present = FAN_CPLD_FAN_6_PRESENT;
	} else {
		pr_err("Invalid fan number");
		return 0;
	}
	val = cpld_reg_read(reg);
	if (val < 0)
		return val;
	if (val & fan_present)
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	return sprintf(buf, PLATFORM_OK "\n");
}

static SENSOR_DEVICE_ATTR(fan_1, S_IRUGO, fan_show, NULL, 0);
static SENSOR_DEVICE_ATTR(fan_2, S_IRUGO, fan_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan_3, S_IRUGO, fan_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan_4, S_IRUGO, fan_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan_5, S_IRUGO, fan_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan_6, S_IRUGO, fan_show, NULL, 4);

/*
 * Fan tray LEDs
 *
 */
static char *fan_led_color[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_RED,
};

static ssize_t
led_fan_tray_show(struct device *dev,
		  struct device_attribute *dattr,
		  char *buf)
{
	int tray = 0;
	uint8_t val8;
	uint32_t reg;
	s32 val;
	uint8_t shift = 0, mask = 0x3;

	if ((sscanf(dattr->attr.name, "led_fan_tray_%d", &tray) < 1) ||
	    (tray < 1) || (tray > 6)) {
		return -EINVAL;
	}

	if (tray >= 1 && tray <= 4) {
		reg = FAN_CPLD_FAN_1_4_LED_DISPLAY_REG;
		shift = (4 - tray) * 2;
	} else {
		reg = FAN_CPLD_FAN_5_6_LED_DISPLAY_REG;
		shift = (6 - tray) * 2;
	}

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;
	val8 = (val8 >> shift) & mask;
	return sprintf(buf, "%s\n", fan_led_color[val8]);
}

static SYSFS_ATTR_RO(led_fan_tray_1, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_2, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_3, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_4, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_5, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_6, led_fan_tray_show);

/******************************************************
 *
 *		System LED definitions
 *
 ******************************************************/
struct led {
	char name[ACCTON_AS7712_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_diag",
		.reg  = CPLD_CPLD1_SYSTEM_LED_1_REG,
		.mask = CPLD_CPLD1_DIAG_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_CPLD1_DIAG_GREEN_LED_ON},
			{ PLATFORM_LED_RED,   CPLD_CPLD1_DIAG_RED_LED_ON},
			{ PLATFORM_LED_OFF,   CPLD_CPLD1_DIAG_LED_OFF},
		},
	},
	{
		.name = "led_loc",
		.reg  = CPLD_CPLD1_SYSTEM_LED_1_REG,
		.mask = CPLD_CPLD1_LOC_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_OFF,  CPLD_CPLD1_LOC_BLUE_LED_OFF},
			{ PLATFORM_LED_BLUE, CPLD_CPLD1_LOC_BLUE_LED_ON},
		},
	},
	{
		.name = "led_fan",
		.reg  = CPLD_CPLD1_INTERRUPT_STATUS_5_REG,
		.mask = CPLD_CPLD1_INTERRUPT_STATUS_FAN,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN,  CPLD_CPLD1_INTERRUPT_STATUS_FAN},
			{ PLATFORM_LED_RED, 0x00},
		},
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * Front Panel Status LEDs
 */
static ssize_t
led_show(struct device *dev,
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
}

static ssize_t
led_store(struct device *dev,
	  struct device_attribute *dattr,
	  const char *buf, size_t count)
{
	s32 val;
	uint8_t val8;
	int i, ret;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	char fmt[10];

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
	snprintf(fmt, sizeof(fmt), "%%%ds", PLATFORM_LED_COLOR_NAME_SIZE - 1);
	if (sscanf(buf, fmt, raw) <= 0)
		return -EINVAL;

	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0)
			break;
	}
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
static SYSFS_ATTR_RW(led_loc,	led_show, led_store);
static SYSFS_ATTR_RO(led_fan,	led_show);

/* Fan CPLD Watchdog timer control */
static ssize_t
fan_wd_show(struct device *dev,
	    struct device_attribute *dattr,
	    char *buf)
{
	s32      val;
	uint8_t  val8;

	/* read the register */
	val = cpld_reg_read(FAN_CPLD_WD_DISABLE_REG);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;

	return sprintf(buf, "0x%x\n", val8);
}

static ssize_t
fan_wd_store(struct device *dev,
	     struct device_attribute *dattr,
	     const char *buf, size_t count)
{
	int ret;
	uint32_t val;

	if (kstrtouint(buf, 0, &val) < 0)
		return -EINVAL;

	if (val > 1)
		return -EINVAL;

	ret = cpld_reg_write(FAN_CPLD_WD_DISABLE_REG, val);

	if (ret < 0)
		return ret;
	return count;
}

static SYSFS_ATTR_RW(fan_cpld_wd_enable, fan_wd_show, fan_wd_store);

static struct attribute *accton_as7712_32x_cpld_attrs[] = {
	&dev_attr_board_info.attr,
	&dev_attr_cpld_version.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_interrupt_ctrl.attr,
	&dev_attr_qsfp_interrupt_mask.attr,
	&sensor_dev_attr_fan_1.dev_attr.attr,
	&sensor_dev_attr_fan_2.dev_attr.attr,
	&sensor_dev_attr_fan_3.dev_attr.attr,
	&sensor_dev_attr_fan_4.dev_attr.attr,
	&sensor_dev_attr_fan_5.dev_attr.attr,
	&sensor_dev_attr_fan_6.dev_attr.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_fan_tray_2.attr,
	&dev_attr_led_fan_tray_3.attr,
	&dev_attr_led_fan_tray_4.attr,
	&dev_attr_led_fan_tray_5.attr,
	&dev_attr_led_fan_tray_6.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_led_loc.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_fan_cpld_wd_enable.attr,
	NULL,
};

static struct attribute_group accton_as7712_32x_cpld_attr_group = {
	.attrs = accton_as7712_32x_cpld_attrs,
};

static struct attribute *accton_as7712_32x_cpld_sensor_attrs[] = {
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
	&dev_attr_pwm1.attr,
	NULL,
};

ATTRIBUTE_GROUPS(accton_as7712_32x_cpld_sensor);

struct accton_as7712_32x_cpld_data {
	struct device *hwmon_dev;
};

static struct accton_as7712_32x_cpld_data cpld_data;

static int
accton_as7712_32x_cpld_probe(struct platform_device *dev)
{
	s32 ret = 0;

	ret = sysfs_create_group(&dev->dev.kobj,
				 &accton_as7712_32x_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
		return ret;
	}

	cpld_data.hwmon_dev = hwmon_device_register_with_groups(&dev->dev,
						dev->name,
						NULL,
						accton_as7712_32x_cpld_sensor_groups);
	if (IS_ERR(cpld_data.hwmon_dev)) {
		ret = PTR_ERR(cpld_data.hwmon_dev);
		dev_err(&dev->dev, "hwmon registration failed");
		goto err_hwmon_device;
	}

	return 0;

err_hwmon_device:
	sysfs_remove_group(&dev->dev.kobj,
			   &accton_as7712_32x_cpld_attr_group);
	return ret;
}

static int
accton_as7712_32x_cpld_remove(struct platform_device *dev)
{
	if (cpld_data.hwmon_dev)
		hwmon_device_unregister(cpld_data.hwmon_dev);

	sysfs_remove_group(&dev->dev.kobj, &accton_as7712_32x_cpld_attr_group);
	return 0;
}

static struct platform_driver accton_as7712_32x_cpld_driver = {
	.driver = {
		.name = "accton_as7712_32x_cpld",
		.owner = THIS_MODULE,
	},
	.probe = accton_as7712_32x_cpld_probe,
	.remove = accton_as7712_32x_cpld_remove,
};

static struct platform_device *accton_as7712_32x_cpld_device;

/**
 * accton_as7712_32x_cpld_init -- CPLD I2C devices
 *
 * Create a device that provides generic access to the CPLD registers.
 */
static int __init
accton_as7712_32x_cpld_init(void)
{
	int i;
	int ret;

	/* Find the 4 CPLD I2C devices -- their I2C addresses are unique */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		if (i2c_clients[i]) {
			switch (i2c_clients[i]->addr) {
			case 0x60:
				as7712_cpld_client_list[CPLD1_ID - 1] =
								i2c_clients[i];
				break;
			case 0x62:
				as7712_cpld_client_list[CPLD2_ID - 1] =
								i2c_clients[i];
				break;
			case 0x64:
				as7712_cpld_client_list[CPLD3_ID - 1] =
								i2c_clients[i];
				break;
			case 0x66:
				as7712_cpld_client_list[FAN_CPLD_ID - 1] =
								i2c_clients[i];
				break;
			default:
				continue;
			}
		}
	}
	/* Verify we found them all */
	for (i = 0; i < ARRAY_SIZE(as7712_cpld_client_list); i++) {
		if (!as7712_cpld_client_list[i]) {
			pr_err("Unable to find all CPLD I2C devices. Missing as7712_cpld_client_list[%d]\n",
			       i);
			return -ENODEV;
		}
	}
	ret = platform_driver_register(&accton_as7712_32x_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	accton_as7712_32x_cpld_device = platform_device_alloc(
						"accton_as7712_32x_cpld", 0);
	if (!accton_as7712_32x_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(accton_as7712_32x_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(accton_as7712_32x_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&accton_as7712_32x_cpld_driver);

err_drvr:
	return ret;
}

static int __init
accton_as7712_32x_platform_init(void)
{
	int ret = 0;

	ret = accton_as7712_32x_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = accton_as7712_32x_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit
accton_as7712_32x_platform_exit(void)
{
	accton_as7712_32x_i2c_exit();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(accton_as7712_32x_platform_init);
module_exit(accton_as7712_32x_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS7712-32X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

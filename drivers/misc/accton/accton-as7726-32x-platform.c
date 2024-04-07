/*
 * accton_as7726_32x_platform.c - Accton as7726-32x Platform Support.
 *
 * Copyright (c) 2018, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: Nikhil Dhar (ndhar@cumulusnetworks.com)
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
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/at24.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "accton-as7726-32x-cpld.h"

#define DRIVER_NAME	"accton_as7726_32x_platform"
#define DRIVER_VERSION	"0.2"

/* defines for port EEPROM */
#define AS7726_PORT_COUNT 32
#define PORT_EEPROMS_PER_MUX 8

static struct i2c_client *cpld_client_list[NUM_CPLD_I2C_CLIENTS];
/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).
*/

/* i2c bus adapter numbers */
enum {
	AS7726_I2C_I801_BUS = 0,

	AS7726_I2C_MUX0_BUS0 = 10,
	AS7726_I2C_MUX0_BUS1,

	AS7726_I2C_MUX1_BUS0 = 20,
	AS7726_I2C_MUX1_BUS1,
	AS7726_I2C_MUX1_BUS2,
	AS7726_I2C_MUX1_BUS3,
	AS7726_I2C_MUX1_BUS4,
	AS7726_I2C_MUX1_BUS5,
	AS7726_I2C_MUX1_BUS6,
	AS7726_I2C_MUX1_BUS7,

	AS7726_I2C_MUX2_BUS0 = 30,
	AS7726_I2C_MUX2_BUS1,
	AS7726_I2C_MUX2_BUS2,
	AS7726_I2C_MUX2_BUS3,
	AS7726_I2C_MUX2_BUS4,
	AS7726_I2C_MUX2_BUS5,
	AS7726_I2C_MUX2_BUS6,
	AS7726_I2C_MUX2_BUS7,

	AS7726_I2C_MUX3_BUS0 = 40,
	AS7726_I2C_MUX3_BUS1,
	AS7726_I2C_MUX3_BUS2,
	AS7726_I2C_MUX3_BUS3,
	AS7726_I2C_MUX3_BUS4,
	AS7726_I2C_MUX3_BUS5,
	AS7726_I2C_MUX3_BUS6,
	AS7726_I2C_MUX3_BUS7,

	AS7726_I2C_MUX4_BUS0 = 50,
	AS7726_I2C_MUX4_BUS1,
	AS7726_I2C_MUX4_BUS2,
	AS7726_I2C_MUX4_BUS3,
	AS7726_I2C_MUX4_BUS4,
	AS7726_I2C_MUX4_BUS5,
	AS7726_I2C_MUX4_BUS6,
	AS7726_I2C_MUX4_BUS7,

	AS7726_I2C_MUX5_BUS0 = 60,
	AS7726_I2C_MUX5_BUS1,
	AS7726_I2C_MUX5_BUS2,
	AS7726_I2C_MUX5_BUS3,
	AS7726_I2C_MUX5_BUS4,
	AS7726_I2C_MUX5_BUS5,
	AS7726_I2C_MUX5_BUS6,
	AS7726_I2C_MUX5_BUS7,

	AS7726_I2C_MUX6_BUS0 = 70,
	AS7726_I2C_MUX6_BUS1,
	AS7726_I2C_MUX6_BUS2,
	AS7726_I2C_MUX6_BUS3,
	AS7726_I2C_MUX6_BUS4,
	AS7726_I2C_MUX6_BUS5,
	AS7726_I2C_MUX6_BUS6,
	AS7726_I2C_MUX6_BUS7,
};

/* EEPROM devices */

/*
 * Edgecore designed a new version of the switch in early 2020.  It fixed a
 * problem with the board eeprom at address 0x56 colliding with PHYs also at
 * address 0x56 on SFF modules.  The fix they implemented was to move the
 * board eeprom to address 0x57.
 *
 * To accomodate this new version, we need to probe for eeproms at both 0x56
 * and 0x57.  Once the eeprom has been found and it responds to reads, remove
 * any other eeproms so that they don't show up in sysfs.
 */

mk_eeprom(board, 56, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);

mk_eeprom(psu1, 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(spd1, 50, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
/* EEPROM devices for 32 ports */
mk_qsfp_port_eeprom(port1, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port10, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port11, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port12, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port13, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port14, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port15, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port16, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port17, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port18, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port19, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port20, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port21, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port22, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port23, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port24, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port25, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port31, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port32, 50, 256, SFF_8436_FLAG_IRUGO);
/* I2C Muxes */
mk_pca9548(mux0, AS7726_I2C_MUX0_BUS0, 1);
mk_pca9548(mux1, AS7726_I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, AS7726_I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, AS7726_I2C_MUX3_BUS0, 1);
mk_pca9548(mux4, AS7726_I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, AS7726_I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, AS7726_I2C_MUX6_BUS0, 1);

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(AS7726_I2C_I801_BUS, "pca9548", 0x77, &mux0_platform_data),

	/* Probe for both board eeproms.  Only one should be present. */
	mk_i2cdev(AS7726_I2C_I801_BUS, "24c02", 0x56, &board_56_at24),
	mk_i2cdev(AS7726_I2C_I801_BUS, "24c02", 0x57, &board_57_at24),

	mk_i2cdev(AS7726_I2C_MUX0_BUS1, "pca9548", 0x71, &mux1_platform_data),
	mk_i2cdev(AS7726_I2C_MUX0_BUS0, "pca9548", 0x72, &mux2_platform_data),
	mk_i2cdev(AS7726_I2C_MUX0_BUS0, "pca9548", 0x73, &mux3_platform_data),
	mk_i2cdev(AS7726_I2C_MUX0_BUS0, "pca9548", 0x74, &mux4_platform_data),
	mk_i2cdev(AS7726_I2C_MUX0_BUS0, "pca9548", 0x75, &mux5_platform_data),
	mk_i2cdev(AS7726_I2C_MUX0_BUS0, "pca9548", 0x76, &mux6_platform_data),
	mk_i2cdev(AS7726_I2C_MUX1_BUS0, "24c02", 0x50, &psu2_50_at24),
	mk_i2cdev(AS7726_I2C_MUX1_BUS0, "cpr4011", 0x58, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS1, "24c02", 0x53, &psu1_53_at24),
	mk_i2cdev(AS7726_I2C_MUX1_BUS1, "cpr4011", 0x5b, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS6, "lm75", 0x48, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS6, "lm75", 0x49, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS6, "lm75", 0x4a, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS6, "lm75", 0x4b, NULL),
	mk_i2cdev(AS7726_I2C_MUX2_BUS0, "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS1, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS2, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS3, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS4, "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS5, "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS6, "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX2_BUS7, "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS0, "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS1, "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS2, "sff8436", 0x50, &port8_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS3, "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS4, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS5, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS6, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX3_BUS7, "sff8436", 0x50, &port16_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS0, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS1, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS2, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS3, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS4, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS5, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS6, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX4_BUS7, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS0, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS1, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS2, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS3, "sff8436", 0x50, &port32_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS4, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS5, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS6, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX5_BUS7, "sff8436", 0x50, &port24_50_sff8436),
	mk_i2cdev(AS7726_I2C_MUX6_BUS2, "cpld-1", 0x60, NULL),
	mk_i2cdev(AS7726_I2C_MUX6_BUS3, "cpld-2", 0x62, NULL),
	mk_i2cdev(AS7726_I2C_MUX6_BUS4, "cpld-3", 0x64, NULL),
	mk_i2cdev(AS7726_I2C_MUX1_BUS5, "fan-cpld", 0x66, NULL),
};

/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
static struct i2c_client *i2c_clients[ARRAY_SIZE(i2c_devices)];

/* I2C initialization */
static int num_cpld_devices;

static void i2c_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		struct i2c_client *c = i2c_devices[i].client;

		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}
}

static int __init i2c_init(void)
{
	int i;
	int ret = -1;

	int i801_bus_num = cumulus_i2c_find_adapter(SMB_I801_NAME);
	struct at24_platform_data *plat_data;
	int board_eeprom = 0;

	if (i801_bus_num < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	num_cpld_devices = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		switch (bus) {
		case AS7726_I2C_I801_BUS:
			i2c_devices[i].bus = i801_bus_num;
			break;
		default:
			break;
		};
		client =
		    cumulus_i2c_add_client(bus, &i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;

		/* find the real board eeprom, remove all others */
		if (strstr(i2c_devices[i].board_info.type, "24c02")) {
			plat_data = i2c_devices[i].board_info.platform_data;
			if (strstr(plat_data->eeprom_data->label, "board_eeprom")) {
				if (board_eeprom || i2c_smbus_read_byte_data(client, 0) < 0) {
					/* already found the board eeprom or this one is bogus,
					   so delete this one */
					i2c_unregister_device(i2c_devices[i].client);
                                        i2c_devices[i].client = NULL;
                                } else {
					pr_err("Board EEPROM discovered at: 0x%02x\n", i2c_devices[i].board_info.addr);
					board_eeprom = i2c_devices[i].board_info.addr;
				}
			}
		}

		if (strstr(i2c_devices[i].board_info.type, "cpld")) {
			pr_err("ADDED CPLD: %s",
			       i2c_devices[i].board_info.type);
			cpld_client_list[num_cpld_devices] = client;
			num_cpld_devices++;
		}
	}
	return 0;

 err_exit:
	i2c_exit();
	return ret;
}

/* CPLD related helper functions */

/**
 * cpld_reg_read - Read an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte
 * received from the device.
 */
static s32 cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int val;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	val = i2c_smbus_read_byte_data(cpld_client_list[cpld_idx],
				       STRIP_CPLD_IDX(reg));
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       cpld_client_list[cpld_idx]->addr, STRIP_CPLD_IDX(reg));
	}
	return val;
}

/**
 * cpld_reg_write - Writes an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */
static s32 cpld_reg_write(uint32_t reg, uint8_t val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to write to invalid CPLD register [0x%02X]",
		       reg);
		return -EINVAL;
	}
	res = i2c_smbus_write_byte_data(cpld_client_list[cpld_idx],
					STRIP_CPLD_IDX(reg), val);

	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       cpld_client_list[cpld_idx]->addr, reg, val);
	}
	return res;
}

/*
 * CPLD Version
 */
static ssize_t
cpld_version_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	s32 cpld_version;
	s32 cpld_reg;
	s32 num_bytes = 0;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	switch (sensor_dev_attr->index) {
	case 0:
	cpld_reg = CPLD1_VERSION_REG;
	break;
	case 1:
	cpld_reg = CPLD2_VERSION_REG;
	break;
	case 2:
	cpld_reg = CPLD3_VERSION_REG;
	break;
	case 3:
	cpld_reg = FAN_CPLD_VERSION_REG;
	break;
	default:
	return num_bytes;
	}

	cpld_version = cpld_reg_read(cpld_reg);

	num_bytes = sprintf(buf, "%d\n", cpld_version);

	return num_bytes;
}

static ssize_t
bulk_power_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	uint8_t read_val;
	uint8_t psu_present;
	uint8_t pwr_ok;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	read_val = (cpld_reg_read(CPLD1_PSU_STATUS_REG));
	if (sensor_dev_attr->index == 0) {
		psu_present = CPLD1_PSU1_STATUS_PRESENT_MASK;
		pwr_ok = (CPLD1_PSU1_STATUS_AC_ALERT_MASK |
			  CPLD1_PSU1_STATUS_12V_GOOD_MASK);
	} else {
		psu_present = CPLD1_PSU2_STATUS_PRESENT_MASK;
		pwr_ok = (CPLD1_PSU2_STATUS_AC_ALERT_MASK |
			  CPLD1_PSU2_STATUS_12V_GOOD_MASK);
	}
	if (!(read_val & psu_present)) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(read_val & pwr_ok))
			strcat(buf, ", " PLATFORM_PS_POWER_BAD);
		else
			strcat(buf, ", " PLATFORM_OK);
	} else {		/* Not Present */
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}

static ssize_t
psu_led_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	uint8_t read_val;
	uint8_t psu_present;
	uint8_t pwr_ok;

	read_val = (cpld_reg_read(CPLD1_PSU_STATUS_REG));
	if (strcmp(dattr->attr.name, "led_psu1") == 0) {
		psu_present = CPLD1_PSU1_STATUS_PRESENT_MASK;
		pwr_ok = (CPLD1_PSU1_STATUS_AC_ALERT_MASK |
			  CPLD1_PSU1_STATUS_12V_GOOD_MASK);
	} else {
		psu_present = CPLD1_PSU2_STATUS_PRESENT_MASK;
		pwr_ok = (CPLD1_PSU2_STATUS_AC_ALERT_MASK |
			  CPLD1_PSU2_STATUS_12V_GOOD_MASK);
	}

	if (!(read_val & psu_present)) {
		if (!(read_val & pwr_ok))
			strcat(buf, PLATFORM_LED_RED);
		else
			strcat(buf, PLATFORM_LED_GREEN);
	} else {		/* Not Present */
		sprintf(buf, PLATFORM_LED_OFF);
	}
	strcat(buf, "\n");

	return strlen(buf);
}

/*****************************************************
 *
 *		QSFP status definitions
 *
 *****************************************************/
struct qsfp_status {
	char name[ACCTON_AS7726_CPLD_STRING_NAME_SIZE];
	uint8_t active_low;
	uint8_t start_reg;
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
	 .name = "present",
	 .active_low = 1,
	 .start_reg = CPLD1_QSFP_1_8_PRESENT_REG,
	 },
	{
	 .name = "reset",
	 .active_low = 1,
	 .start_reg = CPLD1_QSFP_1_8_RESET_REG,
	 },
	{
	 .name = "interrupt_ctrl",
	 .active_low = 1,
	 .start_reg = CPLD1_QSFP_1_8_INTERRUPT_STATUS_REG,
	 },
	{
	 .name = "interrupt_mask",
	 .active_low = 0,
	 .start_reg = CPLD1_QSFP_1_8_INTERRUPT_MASK_REG,
	 },
};

static ssize_t
qsfp_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	int i;
	s32 val = 0;
	struct sensor_device_attribute *sensor_dev_attr = NULL;
	struct qsfp_status *target = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	for (i = 0; i < MAX_NUM_QSFP_REGS; i++) {
		s32 tmp_val = cpld_reg_read((target->start_reg) + i);

		val = val | (tmp_val << (8 * i));
	}
	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%x\n", val);
}

static int hex_to_int64(const char *hex_str, uint64_t *val)
{
	return (kstrtoull(hex_str, 0, val) != 0);
}

static ssize_t
qsfp_store(struct device *dev,
	   struct device_attribute *dattr, const char *buf, size_t count)
{
	int i;
	uint64_t val64;
	uint8_t val;
	struct sensor_device_attribute *sensor_dev_attr = NULL;
	struct qsfp_status *target = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	target = &cpld_qsfp_status[sensor_dev_attr->index];
	if (hex_to_int64(buf, &val64))
		return -EINVAL;

	if (sensor_dev_attr->index != 1)
		return -EINVAL;

	for (i = 0; i < MAX_NUM_QSFP_REGS; i++) {
		val = (val64 >> (8 * i)) & 0xff;
		if (target->active_low)
			val = ~val;
		if (cpld_reg_write(target->start_reg + i, val) < 0) {
			pr_err("CPLD reset register write failed");
			return -EINVAL;
		}
	}

	return count;
}

/*****************************************************
 *
 *		Fan CPLD definitions
 *
 *****************************************************/
static ssize_t
pwm1_show(struct device *dev, struct device_attribute *dattr, char *buf)
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
	 * increments.  A value of 0xf is 100% duty cycle.
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
	   struct device_attribute *dattr, const char *buf, size_t count)
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
fan_tach_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	int fan = 0;
	uint32_t reg;
	s32 val;
	int rpm;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	fan = sensor_dev_attr->index;
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

static ssize_t
fan_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	int reg = FAN_CPLD_FAN_MOD_PRESENT_REG;
	int val = 0;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	val = cpld_reg_read(reg);
	if (val < 0)
		return val;
	if (val & (FAN_CPLD_FAN_1_PRESENT << sensor_dev_attr->index))
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	return sprintf(buf, PLATFORM_OK "\n");
}

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
led_fan_tray_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	int tray = 0;
	uint8_t val8;
	uint32_t reg;
	s32 val;
	uint8_t shift = 0, mask = 0x3;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	tray = sensor_dev_attr->index;
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

/******************************************************
 *
 *		System LED definitions
 *
 ******************************************************/
struct led {
	char name[ACCTON_AS7726_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
	 .name = "led_diag",
	 .reg = CPLD1_SYSTEM_LED_1_REG,
	 .mask = CPLD1_DIAG_LED_MASK,
	 .n_colors = 3,
	 .colors = {
		    {PLATFORM_LED_GREEN, CPLD1_DIAG_GREEN_LED_ON},
		    {PLATFORM_LED_RED, CPLD1_DIAG_RED_LED_ON},
		    {PLATFORM_LED_OFF, CPLD1_DIAG_LED_OFF},
		    },
	 },
	{
	 .name = "led_loc",
	 .reg = CPLD1_SYSTEM_LED_1_REG,
	 .mask = CPLD1_LOC_LED_MASK,
	 .n_colors = 2,
	 .colors = {
		    {PLATFORM_LED_OFF, CPLD1_LOC_BLUE_LED_OFF},
		    {PLATFORM_LED_BLUE, CPLD1_LOC_BLUE_LED_ON},
		    },
	 },
	{
	 .name = "led_fan",
	 .reg = CPLD1_INTERRUPT_STATUS_5_REG,
	 .mask = CPLD1_INTERRUPT_STATUS_FAN,
	 .n_colors = 2,
	 .colors = {
		    {PLATFORM_LED_GREEN, CPLD1_INTERRUPT_STATUS_FAN},
		    {PLATFORM_LED_RED, 0x00},
		    },
	 },
};

/*
 * Front Panel Status LEDs
 */
static ssize_t
led_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	s32 val;
	uint8_t val8;
	int i;
	struct led *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	target = &cpld_leds[sensor_dev_attr->index];
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
	  struct device_attribute *dattr, const char *buf, size_t count)
{
	s32 val;
	uint8_t val8;
	int i, ret;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	char fmt[10];
	struct led *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	target = &cpld_leds[sensor_dev_attr->index];
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

/* Fan CPLD Watchdog timer control */
static ssize_t
fan_wd_show(struct device *dev, struct device_attribute *dattr, char *buf)
{
	s32 val;
	uint8_t val8;

	/* read the register */
	val = cpld_reg_read(FAN_CPLD_WD_DISABLE_REG);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;

	return sprintf(buf, "0x%x\n", val8);
}

static ssize_t
fan_wd_store(struct device *dev,
	     struct device_attribute *dattr, const char *buf, size_t count)
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

/* sysfs files corresponding to CPLD1 registers */
static SENSOR_DEVICE_ATTR_RO(qsfp_present, qsfp_show, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp_reset, qsfp_show, qsfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp_interrupt_ctrl, qsfp_show, 2);
static SENSOR_DEVICE_ATTR_RO(qsfp_interrupt_mask, qsfp_show, 3);
static SENSOR_DEVICE_ATTR_RO(cpld1_version, cpld_version_show, 0);
static SENSOR_DEVICE_ATTR_RO(cpld2_version, cpld_version_show, 1);
static SENSOR_DEVICE_ATTR_RO(cpld3_version, cpld_version_show, 2);
static SENSOR_DEVICE_ATTR_RO(fan_cpld_version, cpld_version_show, 3);
static SENSOR_DEVICE_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show, 0);
static SENSOR_DEVICE_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show, 1);
static SYSFS_ATTR_RO(led_psu1, psu_led_show);
static SYSFS_ATTR_RO(led_psu2, psu_led_show);
/* sysfs files corresponding to FAN CPLD registers */
static SYSFS_ATTR_RW(pwm1, pwm1_show, pwm1_store);
static SENSOR_DEVICE_ATTR_RO(fan1_input, fan_tach_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan2_input, fan_tach_show, 2);
static SENSOR_DEVICE_ATTR_RO(fan3_input, fan_tach_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan4_input, fan_tach_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan5_input, fan_tach_show, 5);
static SENSOR_DEVICE_ATTR_RO(fan6_input, fan_tach_show, 6);
static SENSOR_DEVICE_ATTR_RO(fan7_input, fan_tach_show, 7);
static SENSOR_DEVICE_ATTR_RO(fan8_input, fan_tach_show, 8);
static SENSOR_DEVICE_ATTR_RO(fan9_input, fan_tach_show, 9);
static SENSOR_DEVICE_ATTR_RO(fan10_input, fan_tach_show, 10);
static SENSOR_DEVICE_ATTR_RO(fan11_input, fan_tach_show, 11);
static SENSOR_DEVICE_ATTR_RO(fan12_input, fan_tach_show, 12);
static SENSOR_DEVICE_ATTR_RO(fan_1, fan_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan_2, fan_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan_3, fan_show, 2);
static SENSOR_DEVICE_ATTR_RO(fan_4, fan_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan_5, fan_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan_6, fan_show, 5);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_1, led_fan_tray_show, 1);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_2, led_fan_tray_show, 2);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_3, led_fan_tray_show, 3);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_4, led_fan_tray_show, 4);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_5, led_fan_tray_show, 5);
static SENSOR_DEVICE_ATTR_RO(led_fan_tray_6, led_fan_tray_show, 6);
static SYSFS_ATTR_RW(fan_cpld_wd_enable, fan_wd_show, fan_wd_store);
static SENSOR_DEVICE_ATTR_RW(led_diag, led_show, led_store, 0);
static SENSOR_DEVICE_ATTR_RW(led_loc, led_show, led_store, 1);
static SENSOR_DEVICE_ATTR_RO(led_fan, led_show, 2);

static struct attribute *cpld_attrs[] = {
	&sensor_dev_attr_cpld1_version.dev_attr.attr,
	&sensor_dev_attr_cpld2_version.dev_attr.attr,
	&sensor_dev_attr_cpld3_version.dev_attr.attr,
	&sensor_dev_attr_fan_cpld_version.dev_attr.attr,
	&sensor_dev_attr_qsfp_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp_present.dev_attr.attr,
	&sensor_dev_attr_qsfp_interrupt_ctrl.dev_attr.attr,
	&sensor_dev_attr_qsfp_interrupt_mask.dev_attr.attr,
	&sensor_dev_attr_fan_1.dev_attr.attr,
	&sensor_dev_attr_fan_2.dev_attr.attr,
	&sensor_dev_attr_fan_3.dev_attr.attr,
	&sensor_dev_attr_fan_4.dev_attr.attr,
	&sensor_dev_attr_fan_5.dev_attr.attr,
	&sensor_dev_attr_fan_6.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_1.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_2.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_3.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_4.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_5.dev_attr.attr,
	&sensor_dev_attr_led_fan_tray_6.dev_attr.attr,
	&sensor_dev_attr_psu_pwr1.dev_attr.attr,
	&sensor_dev_attr_psu_pwr2.dev_attr.attr,
	&sensor_dev_attr_led_diag.dev_attr.attr,
	&sensor_dev_attr_led_loc.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_fan_cpld_wd_enable.attr,
	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static struct attribute *cpld_sensor_attrs[] = {
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,
	&sensor_dev_attr_fan11_input.dev_attr.attr,
	&sensor_dev_attr_fan12_input.dev_attr.attr,
	&dev_attr_pwm1.attr,
	NULL,
};

ATTRIBUTE_GROUPS(cpld_sensor);

struct cpld_data {
	struct device *hwmon_dev;
};

static struct cpld_data act7726_cpld_data;

static int cpld_probe(struct platform_device *dev)
{
	s32 ret = 0;

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
		return ret;
	}

	act7726_cpld_data.hwmon_dev = hwmon_device_register_with_groups(&dev->dev,
																dev->name,
																NULL,
																cpld_sensor_groups);
	if (IS_ERR(act7726_cpld_data.hwmon_dev)) {
		ret = PTR_ERR(act7726_cpld_data.hwmon_dev);
		dev_err(&dev->dev, "hwmon registration failed");
		goto err_hwmon_device;
	}
	return 0;

 err_hwmon_device:
	sysfs_remove_group(&dev->dev.kobj, &cpld_attr_group);
	return ret;
}

static int __exit cpld_remove(struct platform_device *dev)
{
	if (act7726_cpld_data.hwmon_dev)
		hwmon_device_unregister(act7726_cpld_data.hwmon_dev);

	sysfs_remove_group(&dev->dev.kobj, &cpld_attr_group);
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		   .name = "accton_as7726_32x_cpld",
		   .owner = THIS_MODULE,
		   },
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static struct platform_device *cpld_device;
static struct platform_device_info cpld_pltf_dev_info = {
	.name = "accton_as7726_32x_cpld",
	.id = 0,
};

/**
 * cpld_init -- CPLD I2C devices
 *
 * Create a device that provides generic access to the CPLD registers.
 */
static int __init cpld_init(void)
{
	int i;
	int ret;

	/* Find the 4 CPLD I2C devices -- their I2C addresses are unique */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		if (i2c_clients[i]) {
			switch (i2c_clients[i]->addr) {
			case 0x60:
				cpld_client_list[CPLD1_ID - 1] = i2c_clients[i];
				break;
			case 0x62:
				cpld_client_list[CPLD2_ID - 1] = i2c_clients[i];
				break;
			case 0x64:
				cpld_client_list[CPLD3_ID - 1] = i2c_clients[i];
				break;
			case 0x66:
				cpld_client_list[FAN_CPLD_ID - 1] =
				    i2c_clients[i];
				break;
			default:
				continue;
			}
		}
	}
	/* Verify we found them all */
	for (i = 0; i < ARRAY_SIZE(cpld_client_list); i++) {
		if (!cpld_client_list[i]) {
			pr_err
			    ("Missing cpld_client_list[%d]\n",
			     i);
			return -ENODEV;
		}
	}
	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	cpld_device = platform_device_register_full(&cpld_pltf_dev_info);
	if (!cpld_device) {
		pr_err
		    ("platform_device_register_full() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}
	return 0;

 err_dev_alloc:
	platform_driver_unregister(&cpld_driver);
 err_drvr:
	return ret;
}

static int __init platform_init(void)
{
	int ret = 0;

	ret = i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit platform_exit(void)
{
	i2c_exit();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("Nikhil Dhar (ndhar@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS7726-32X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

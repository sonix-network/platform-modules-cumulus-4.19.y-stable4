/*
 * Smartfusion mfd driver definitions and declarations.
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

#ifndef S6100_SMF_H__
#define S6100_SMF_H__

#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include "dell-s6100-smf-reg.h"

#define S6100_SMF_DRIVER_NAME	"dell-s6100-smf"

/**
 * gen_device_attr() -- helper function
 *
 * Helper function that creates a 'struct attribute' from a template
 * 'struct sensor_device_attribute' object.
 *
 * Returns a pointer to the 'struct attribute' from inside the 'struct
 * sensor_device_attribute' or -ENOMEM.
 */
static inline struct attribute *
gen_device_attr(struct platform_device *pdev,
		struct sensor_device_attribute *pattr)
{
	struct sensor_device_attribute *sensor_attr;

	sensor_attr = devm_kzalloc(&pdev->dev,
				   sizeof(*sensor_attr), GFP_KERNEL);
	if (!sensor_attr)
		return ERR_PTR(-ENOMEM);

	sysfs_attr_init(&sensor_attr->dev_attr.attr);
	sensor_attr->dev_attr.attr.name = pattr->dev_attr.attr.name;
	sensor_attr->dev_attr.attr.mode = pattr->dev_attr.attr.mode;
	sensor_attr->dev_attr.show      = pattr->dev_attr.show;
	sensor_attr->dev_attr.store     = pattr->dev_attr.store;
	sensor_attr->index              = pattr->index;

	return &sensor_attr->dev_attr.attr;
}

#define S6100_MODULE_DRIVER_NAME "dell-s6100-module"
#define S6100_MODULE_CPLD_ADDR 0x3e

/**
 * struct s6100_module_platform_data
 *
 * Platform data common to all IO modules
 */
struct s6100_module_platform_data {
	struct i2c_client *cpld_client;
	int optical_i2c_bus;
};

/**
 * S6100_MODULE_ID_XXX
 *
 * These IDs identify the different types of S6100 IO modules.  These
 * values match the contents of the module CPLD register CARD_ID
 * (0x3).
 */
#define S6100_MODULE_ID_16X40G_QSFP   0x1
#define S6100_MODULE_ID_8X100G_QSFP28 0x2
#define S6100_MODULE_ID_8X100G_CXP    0x3

/**
 * S6100_MODULE_GPIO_CTRL_BASE
 *
 * Base GPIO number to use for IO module GPIOs.
 */
#define S6100_MODULE_GPIO_CTRL_BASE 1200

/**
 * S6100_GPIO_PER_MODULE
 *
 * Number of GPIOs to assign to each IO module GPIOs.
 *
 * The base of each IO module GPIO range (zero-based) is given by:
 *
 *   module_base_gpio = S6100_MODULE_GPIO_CTRL_BASE +
 *       (mod_number * S6100_GPIO_PER_MODULE)
 */
#define S6100_GPIO_PER_MODULE       100

/**
 * S6100_MODULE_I2C_BUS_BASE
 *
 * Base i2c adapter number to use for IO module i2c muxes.
 */
#define S6100_MODULE_I2C_BUS_BASE 100

/**
 * S6100_I2C_BUS_PER_MODULE
 *
 * Number of i2c adapters to assign to each IO module.
 *
 * The base i2c adapter to use for each IO module (zero-based) is
 * given by:
 *
 *    base_adapter = S6100_MODULE_I2C_BUS_BASE + (mod_number * S6100_I2C_BUS_PER_MODULE)
 */
#define S6100_I2C_BUS_PER_MODULE  20

/**
 * S6100_MAX_EEPROM_NAME_LEN
 *
 * Maximum length in bytes for the module port EEPROM labels.
 */
#define S6100_MAX_EEPROM_NAME_LEN 20

/**
 * module_cpld_raw_reg_rd()
 * @client: i2c client object for CPLD device
 * @reg:    register offset
 * @val:    register read value
 *
 * Reads a module i2c CPLD register directly using the i2c
 * infrastructure.
 *
 * Providing raw access (non-regmap) function for early use by the
 * driver probe() routine.
 */
static inline int
module_cpld_raw_reg_rd(struct i2c_client *client, uint8_t reg, uint8_t *val)
{
	int rc = 0;
	*val = 0;

	/* Module CPLD uses 16-bit i2c addressing */
	rc = i2c_smbus_write_byte_data(client, 0x0, reg);
	if (rc) {
		dev_err(&client->dev,
			"CPLD i2c addr write failed, addr: 0x%02x, register: 0x%02x (%d)\n",
			client->addr, reg, rc);
		return rc;
	}
	rc = i2c_smbus_read_byte(client);
	if (rc < 0) {
		dev_err(&client->dev,
			"CPLD i2c read failed, addr: 0x%02x, register: 0x%02x (%d)\n",
			client->addr, reg, rc);
	} else {
		*val = rc & 0xFF;
		rc = 0;
	}

	return rc;
}

/**
 * Common module i2c CPLD registers
 *
 */
#define S6100_MOD_CPLD_VERSION	0x00
#define S6100_MOD_BOARD_TYPE	0x01
#define S6100_MOD_SW_SCRATCH	0x02
#define S6100_MOD_MODULE_ID	0x03

#endif /* S6100_SMF_H__ */

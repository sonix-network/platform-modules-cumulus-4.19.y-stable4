/*
 * Celestica Seastone2 and Questone2 Platform Support
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
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
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/interrupt.h>
#include <linux/stddef.h>
#include <linux/acpi.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"

#define DRIVER_NAME	"cel_sea2que2_platform"
#define DRIVER_VERSION	"1.0"

#define ISMT_ADAPTER_NAME	  "SMBus iSMT adapter"
#define I801_ADAPTER_NAME	  "SMBus I801 adapter"

/*
 * This platform has both an iSMT adapter and an i801 adapter.
 *
 * The iSMT adapter is connected to the following devices:
 *
 *    board eeprom (0x56)
 *
 */

enum {
	I2C_ISMT_BUS = -1,
	I2C_I801_BUS,
};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device,
 * using the method specific to the device type.  Then we put them
 * all together in a big table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_eeprom(board, 56, 256, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);

/*
 * i2c device tables
 *
 * We use the magic mk_i2cdev() macro to construct the entries.  Each entry is
 * a bus number and an i2c_board_info.  The i2c_board_info structure specifies
 * the device type, address, and platform data specific to the device type.
 *
 * The i2c_devices[] table contains all the devices that we expose on the
 * Denverton i2c busses.
 *
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS, "24c02",   0x56, &board_56_at24),
};

/* I2C Initialization */

static void i2c_exit(void)
{
	int i;
	struct i2c_client *c;

	/* unregister  the Denverton i2c clients */
	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		c = i2c_devices[i].client;
		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}

	pr_info("I2C driver unloaded\n");
}

static int i2c_init(void)
{
	struct i2c_client *client;
	int ismt_bus;
	int i801_bus;
	int ret;
	int i;

	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err("Could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err("Could not find the i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus;

		bus = i2c_devices[i].bus;
		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("Add client failed for bus %d: %d\n",
			       bus, ret);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	pr_info("I2C driver loaded\n");
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* Module init and exit */

static int __init cel_sea2que2_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C initialization failed\n");
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit cel_sea2que2_exit(void)
{
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(cel_sea2que2_init);
module_exit(cel_sea2que2_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Seastone2 and Questone2 Platform Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

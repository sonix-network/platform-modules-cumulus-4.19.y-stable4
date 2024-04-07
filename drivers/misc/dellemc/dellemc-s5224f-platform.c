// SPDX-Licnese-Identifier: GPL-2.0+
/*
 * Dell EMC S5224F Platform Support
 * This driver used the Dell EMC S5296F as its model/template.  
 * That being said, this driver is virtually identical to the 5296f 
 * driver (except for a few very minor things that came up in code review).
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
 * Author: Andy Rao <arao@cumulusnetworks.com>
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

#define DRIVER_NAME	"dellemc_s5224f_platform"
#define DRIVER_VERSION	"1.0"

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
 * For eeproms,  we specify the label, i2c address, size, and some flags, all
 * done in mk*_eeprom() macros.  The label is the string that ends up in
 * /sys/class/eeprom_dev/eepromN/label, which we use to identify them at user
 * level.
 *
 */

mk_eeprom(board, 50, 256, AT24_FLAG_IRUGO);

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
	mk_i2cdev(I2C_ISMT_BUS, "24c02", 0x50, &board_50_at24),
};

/* I2C Initialization */

static void i2c_exit(void)
{
	int i;
	struct i2c_client *c;

	/* unregister the Denverton i2c clients */
	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		c = i2c_devices[i].client;
		if (c) {
			i2c_unregister_device(c);
			i2c_devices[i].client = NULL;
		}
	}

	pr_info(DRIVER_NAME ": I2C driver unloaded\n");
}

static int i2c_init(void)
{
	int ismt_bus;
	int i801_bus;
	int ret;
	int i;

	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err(DRIVER_NAME ": could not find iSMT adapter bus\n");
		return -ENODEV;
	}

	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err(DRIVER_NAME ": could not find the i801 adapter bus\n");
		return -ENODEV;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;

		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err(DRIVER_NAME ": add client failed for bus %d: %d\n",
			       bus, ret);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	pr_info(DRIVER_NAME ": I2C driver loaded\n");
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* Module init and exit */

static int __init dellemc_s5224f_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit dellemc_s5224f_exit(void)
{
	i2c_exit();
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(dellemc_s5224f_init);
module_exit(dellemc_s5224f_exit);

MODULE_AUTHOR("Andy Rao (arao@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5224F Platform Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

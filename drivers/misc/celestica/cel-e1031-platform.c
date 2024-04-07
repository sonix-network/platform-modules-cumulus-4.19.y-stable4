// SPDX-License-Identifier: GPL-2.0+
/*
 * Celestica Haliburton (E1031) Platform Driver
 *
 * Copyright (c) 2015, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Authors: Puneet Shenoy <puneet@cumulusnetworks.com>
 *          David Yen <dhyen@cumulusnetworks.com>
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
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_device.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "cel-e1031.h"

#define DRIVER_NAME    E1031_PLATFORM_NAME
#define DRIVER_VERSION "1.2"

/*
 * This platform has two i2c busses:
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3
 *  SMBus_1: SMBus iSMT adapter at PCIe address 0000:00:13.0
 */

/* i2c bus adapter numbers for the down stream i2c busses */
enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS,
	I2C_MASTER_SWITCH_BUS = 9,
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,
	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,
	I2C_MUX3_BUS0 = 30,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,
};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device, using the
 * method specific to the device type.  Then we put them all together in a big
 * table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports, using
 * the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the sff transceivers, we specify the label,
 * i2c address, size, and some flags, all done in mk*_eeprom() macros.  The
 * label is the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_pca9547(mux1, I2C_MUX1_BUS0, 1);
mk_pca9547(mux2, I2C_MUX2_BUS0, 1);
mk_pca9547(mux3, I2C_MUX3_BUS0, 1);

mk_eeprom(spd1,   50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board,  50, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(board2, 53, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);

mk_eeprom(fan1, 54, 8192,
	  AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2, 54, 8192,
	  AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3, 54, 8192,
	  AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1, 53, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 52, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_port_eeprom(port49, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.  Each entry is a bus
 * number and a i2c_board_info.  The i2c_board_info structure specifies the
 * device type, address, and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS,  "pca9548", 0x73, &mux1_platform_data),
	mk_i2cdev(I2C_I801_BUS,  "spd",     0x50, &spd1_50_at24),
	mk_i2cdev(I2C_MUX1_BUS0, "24c64",   0x50, &board_50_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "max6697", 0x1a, NULL), /* temp sensor */
	mk_i2cdev(I2C_MUX1_BUS6, "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(I2C_MUX1_BUS6, "pca9548", 0x72, &mux3_platform_data),
	mk_i2cdev(I2C_MUX2_BUS0, "24c64",   0x53, &board2_53_at24),
	mk_i2cdev(I2C_MUX2_BUS1, "max6697", 0x1a, NULL), /* temp sensor */
	mk_i2cdev(I2C_MUX2_BUS2, "24c02",   0x52, &psu2_52_at24),
	mk_i2cdev(I2C_MUX2_BUS2, "dps200",  0x5a, NULL), /* PSU2 PMBus */
	mk_i2cdev(I2C_MUX2_BUS3, "24c02",   0x53, &psu1_53_at24),
	mk_i2cdev(I2C_MUX2_BUS3, "dps200",  0x5b, NULL), /* PSU1 PMBus */
	mk_i2cdev(I2C_MUX2_BUS4, "24c04",   0x50, &port50_50_at24),
	mk_i2cdev(I2C_MUX2_BUS5, "24c04",   0x50, &port49_50_at24),
	mk_i2cdev(I2C_MUX2_BUS6, "24c04",   0x50, &port52_50_at24),
	mk_i2cdev(I2C_MUX2_BUS7, "24c04",   0x50, &port51_50_at24),
	mk_i2cdev(I2C_MUX3_BUS2, "24c64",   0x54, &fan1_54_at24),
	mk_i2cdev(I2C_MUX3_BUS3, "24c64",   0x54, &fan2_54_at24),
	mk_i2cdev(I2C_MUX3_BUS4, "24c64",   0x54, &fan3_54_at24),
	mk_i2cdev(I2C_MUX3_BUS5, "emc2305", 0x4d, NULL),
};

#define NUM_I2C_DEVICES ARRAY_SIZE(i2c_devices)

/* i2c init */

static void del_i2c_clients(void)
{
	int i;

	for (i = NUM_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(i2c_devices[i].client);
		i2c_devices[i].client = NULL;
	}
}

static int add_i2c_clients(struct platform_i2c_device_info *pdi, int num_dev,
			   int ismt_bus, int i801_bus)
{
	int ret;

	while (num_dev--) {
		if (pdi->bus == I2C_ISMT_BUS)
			pdi->bus = ismt_bus;
		else if (pdi->bus == I2C_I801_BUS)
			pdi->bus = i801_bus;

		if (!pdi->client) {
			pdi->client = cumulus_i2c_add_client(pdi->bus,
							     &pdi->board_info);
			if (IS_ERR(pdi->client)) {
				pr_info(DRIVER_NAME ": failed to add client on bus %d\n",
					pdi->bus);
				ret = PTR_ERR(pdi->client);
				pdi->client = NULL;
				return ret;
			}
		}

		pdi++;
	}
	return 0;
}

static int platform_probe(struct platform_device *dev)
{
	int ismt_bus;
	int i801_bus;
	int ret;

	/* identify the adapter buses */
	ret = -ENODEV;
	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		dev_err(&dev->dev, "Could not find the iSMT adapter bus\n");
		goto err_exit;
	}
	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		dev_err(&dev->dev, "Could not find the i801 adapter bus\n");
		goto err_exit;
	}

	/* add the i2c devices */
	ret = add_i2c_clients(&i2c_devices[0], NUM_I2C_DEVICES,
			      ismt_bus, i801_bus);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	if (ret != -EPROBE_DEFER) {
		dev_info(&dev->dev, "error during probe, deleting clients\n");
		del_i2c_clients();
	}
	return ret;
}

static int platform_remove(struct platform_device *dev)
{
	del_i2c_clients();
	return 0;
}

static const struct platform_device_id platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(platform, platform_id);

static struct platform_driver plat_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = platform_probe,
	.remove = platform_remove,
	.id_table = platform_id
};

static struct platform_device *plat_device;

/* module init/exit */

static int __init platform_init(void)
{
	int ret = 0;

	/* register the platform driver */
	ret = platform_driver_register(&plat_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": %s driver registration failed (%d)\n",
		       plat_driver.driver.name, ret);
		goto err_plat_driver;
	}

	/* create the platform device */
	plat_device = platform_device_register_simple(DRIVER_NAME, -1,
						      NULL, 0);
	if (IS_ERR(plat_device)) {
		ret = PTR_ERR(plat_device);
		pr_err(DRIVER_NAME ": platform device registration failed (%d)\n",
		       ret);
		goto err_plat_device;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " registered\n");
	return ret;

err_plat_device:
	platform_driver_unregister(&plat_driver);
err_plat_driver:
	return ret;
}

static void __exit platform_exit(void)
{
	platform_device_unregister(plat_device);
	platform_driver_unregister(&plat_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("Celestica Haliburton (E1031) Platform Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

// SPDX-License-Identifier: GPL-2.0+
/*
 * Accton AS5835-54T Platform Support
 *
 * Copyright (c) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
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
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_device.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "accton-as5835.h"

#define DRIVER_NAME    AS5835_54T_PLATFORM_NAME
#define DRIVER_VERSION "1.0"

/*
 * The platform has 2 types of I2C SMBUSes:
 *   i2c-0 = SMBus iSMT adapter
 *   i2c-1 = SMBus I801 adapter
 */

enum {
	I2C_I801_BUS = -1,
	I2C_ISMT_BUS,

	/* mux at 0x77 */
	I2C_MUX1_BUS0 = 10, /* Main Board CH1 */
	I2C_MUX1_BUS1,      /* Main Board CH2 */
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	/* mux at 0x77.0 -> 0x70 */
	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	/* mux at 0x77.0 -> 0x72 */
	I2C_MUX3_BUS0,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	/* mux at 0x77.0 -> 0x71 */
	I2C_MUX4_BUS0,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
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
 */

mk_pca9548(mux1,  I2C_MUX1_BUS0,  1);
mk_pca9548(mux2,  I2C_MUX2_BUS0,  1);
mk_pca9548(mux3,  I2C_MUX3_BUS0,  1);
mk_pca9545(mux4,  I2C_MUX4_BUS0,  1);

mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);

mk_qsfp_port_eeprom(port49, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54, 50, 256, SFF_8436_FLAG_IRUGO);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info structure specifies the device type, address,
 * and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS,  "24c02",    0x57, &board_57_at24),
	mk_i2cdev(I2C_ISMT_BUS,  AS5835_CPUCPLD_NAME, 0x65, NULL),
	mk_i2cdev(I2C_ISMT_BUS,  "pca9548",  0x77, &mux1_platform_data),

	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x70, &mux2_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x72, &mux3_platform_data),
	mk_i2cdev(I2C_MUX3_BUS2, "sff8436",  0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS3, "sff8436",  0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS0, "sff8436",  0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS4, "sff8436",  0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS5, "sff8436",  0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS1, "sff8436",  0x50, &port54_50_sff8436),

	mk_i2cdev(I2C_MUX1_BUS0, "pca9545",  0x71, &mux4_platform_data),
	mk_i2cdev(I2C_MUX4_BUS0, "tmp75",    0x4b, NULL),
	mk_i2cdev(I2C_MUX4_BUS1, "tmp75",    0x4c, NULL),
	mk_i2cdev(I2C_MUX4_BUS2, "tmp75",    0x49, NULL),
	mk_i2cdev(I2C_MUX4_BUS3, "tmp75",    0x4a, NULL),

	mk_i2cdev(I2C_MUX1_BUS1, AS5835_CPLD1_NAME, 0x60, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, AS5835_CPLD2_NAME, 0x61, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, AS5835_CPLD3_NAME, 0x62, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, AS5835_CPLD4_NAME, 0x63, NULL),
};

#define NUM_I2C_DEVICES ARRAY_SIZE(i2c_devices)

/* I2C init */

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
	return 0;
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

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS5835-54T Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

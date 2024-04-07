// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v2 Platform Support.
 *
 * Copyright (C) 2018, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/hwmon-sysfs.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_device.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "delta-ag9032v2.h"

#define DRIVER_NAME    AG9032V2_PLATFORM_NAME
#define DRIVER_VERSION "1.1"

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device, using the
 * method specific to the device type.  Then we put them all together in a big
 * table (see i2c_devices below).
 *
 * For eeproms, we specify the label, i2c address, size, and some flags, all
 * done in mk*_eeprom() macros.  The label is the string that ends up in
 * /sys/class/eeprom_dev/eepromN/label, which we use to identify them at user
 * level.
 *
 */

mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);

/* all of the devices that exist on one of the CPLD muxes */
mk_qsfp_port_eeprom(port1,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port10, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port11, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port12, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port13, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port14, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port15, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port16, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port17, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port18, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port19, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port20, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port21, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port22, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port23, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port24, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port25, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port31, 50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port32, 50, 256,  SFF_8436_FLAG_IRUGO);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.  Each entry is a bus
 * number and a i2c_board_info.  The i2c_board_info structure specifies the
 * device type, address, and platform data specific to the device type.
 */

static struct cpld_item mastercpld_item;

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on the iSMT bus */
	mk_i2cdev(I2C_ISMT_BUS, "24c02",         0x53, &board_53_at24),
	mk_i2cdev(I2C_ISMT_BUS, AG9032V2_SYSTEMCPLD_NAME, 0x31, NULL),
	mk_i2cdev(I2C_ISMT_BUS, AG9032V2_MASTERCPLD_NAME, 0x6a, NULL),
	mk_i2cdev(I2C_ISMT_BUS, AG9032V2_SLAVE1CPLD_NAME, 0x73, NULL),
	mk_i2cdev(I2C_ISMT_BUS, AG9032V2_SLAVE2CPLD_NAME, 0x75, NULL),

	/*
	 * This driver makes use of registers in MASTERCPLD so it must
	 * come *after* the MASTERCPLD instantiation in this table.
	 */
	mk_i2cdev(I2C_ISMT_BUS, AG9032V2_SFFMUX_NAME, 0x7f, &mastercpld_item),

	/* sff mux devices */
	mk_i2cdev(CPLD_QSFP_MUX_BUS1,  "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS2,  "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS3,  "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS4,  "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS5,  "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS6,  "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS7,  "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS8,  "sff8436", 0x50, &port8_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS9,  "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS10, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS11, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS12, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS13, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS14, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS15, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS16, "sff8436", 0x50, &port16_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS17, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS18, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS19, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS20, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS21, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS22, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS23, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS24, "sff8436", 0x50, &port24_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS25, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS26, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS27, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS28, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS29, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS30, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS31, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS32, "sff8436", 0x50, &port32_50_sff8436),
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

	mutex_init(&mastercpld_item.cpld_mutex);

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

		/* Save the client of the MASTERCPLD. */
		if (!strcmp(pdi->board_info.type, AG9032V2_MASTERCPLD_NAME) &&
		    pdi->board_info.addr == 0x6a)
			mastercpld_item.cpld_client = pdi->client;
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

	/* register the platform_driver */
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

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Delta AG9032v2 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

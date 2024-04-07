// SPDX-License-Identifier: GPL-2.0+
/*
 * Dell EMC S5048F Platform Support
 *
 * Copyright (c) 2018, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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
#include "dellemc-s5048f.h"
#include "dellemc-s5048f-smf.h"

#define DRIVER_NAME    S5048F_PLATFORM_NAME
#define DRIVER_VERSION "1.0"

/*
 * This platform has two i2c busses:
 *  SMBus_1: SMBus I801 adapter at f000
 *  SMBus_0: SMBus iSMT adapter at dfff0000
 *
 * SMBus_0 is connected to the following devices:
 *    pca9547 8-channel switch (0x70)
 *	 0 eeprom (0x50-0x57)
 *	 1 bus master selector (0x74)
 *	 2 pca9548 8-channel mux (0x71)
 *	     0 eeprom (0x50)
 *	     4 cpld#2 (0x3e)
 *	     5 cpld#3 (0x3e)
 *	     6 cpld#4 (0x3e)
 *	     7 cpld#1 (0x3e)
 *	 4 pca9548 8-channel mux for qsfp28 49-54 (0x71)
 *	 5 pca9548 8-channel mux for  sfp28 41-48 (0x71)
 *	 5 pca9548 8-channel mux for  sfp28 33-40 (0x72)
 *	 6 pca9548 8-channel mux for  sfp28 25-32 (0x71)
 *	 6 pca9548 8-channel mux for  sfp28 17-24 (0x72)
 *	 7 pca9548 8-channel mux for  sfp28  9-16 (0x71)
 *	 7 pca9548 8-channel mux for  sfp28  1- 8 (0x72)
 *
 * SMBus_1 is connected to the following devices:
 *    i2c host (0x08)
 *    so-dimm temp sensor (0x18)
 *    isl90727 ddr3 vref tuning (0x2e)
 *    so-dimm eeprom (0x50)
 *    clock gen (0x69)
 */

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device, using the
 * method specific to the device type.  Then we put them all together in a big
 * table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the sff transceivers, we specify the label,
 * i2c address, size, and some flags, all done in mk*_eeprom() macros.  The
 * label is the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 */

mk_pca9547(switch1, I2C_SWITCH1_BUS0, 1);
mk_pca9548(mux2,    I2C_MUX2_BUS0,    1);
mk_pca9548(mux4,    I2C_MUX4_BUS0,    1);
mk_pca9548(mux5,    I2C_MUX5_BUS0,    1);
mk_pca9548(mux6,    I2C_MUX6_BUS0,    1);
mk_pca9548(mux7,    I2C_MUX7_BUS0,    1);
mk_pca9548(mux8,    I2C_MUX8_BUS0,    1);
mk_pca9548(mux9,    I2C_MUX9_BUS0,    1);
mk_pca9548(mux10,   I2C_MUX10_BUS0,   1);

mk_eeprom(spd1,	 50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);

mk_eeprom(eeprom0, 50, 256, AT24_FLAG_IRUGO);
mk_eeprom(board,   50, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom2, 51, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom3, 52, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom4, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom5, 54, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom6, 55, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom7, 56, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom8, 57, 256, AT24_FLAG_IRUGO);

mk_port_eeprom(port1,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port2,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port3,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port4,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port5,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port6,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port7,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port8,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port9,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port10, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port11, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port12, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port13, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port14, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port15, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port16, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port17, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port18, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port19, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port20, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port21, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port22, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port23, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port24, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port25, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port26, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port27, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port28, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port29, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port30, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port31, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port32, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port33, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port34, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port35, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port36, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port37, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port38, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port39, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port40, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port41, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port42, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port43, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port44, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port45, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port46, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port47, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port48, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_qsfp_port_eeprom(port49, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54, 50, 256, SFF_8436_FLAG_IRUGO);

static struct platform_i2c_device_info i2c_devices[] = {
	/* SMBus_0 */
	mk_i2cdev(I2C_I801_BUS,	    "spd",     0x50, &spd1_50_at24),

	/* SMBus_1 */
	mk_i2cdev(I2C_ISMT_BUS,	    "pca9547", 0x70, &switch1_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x50, &board_50_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x51, &eeprom2_51_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x52, &eeprom3_52_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x53, &eeprom4_53_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x54, &eeprom5_54_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x55, &eeprom6_55_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x56, &eeprom7_56_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x57, &eeprom8_57_at24),

	mk_i2cdev(I2C_SWITCH1_BUS2, "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS4, "pca9548", 0x71, &mux4_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS5, "pca9548", 0x71, &mux5_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS5, "pca9548", 0x72, &mux8_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS6, "pca9548", 0x71, &mux6_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS6, "pca9548", 0x72, &mux9_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS7, "pca9548", 0x71, &mux7_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS7, "pca9548", 0x72, &mux10_platform_data),

	mk_i2cdev(I2C_MUX2_BUS0,    "24c02",   0x50, &eeprom0_50_at24),
	mk_i2cdev(I2C_MUX2_BUS4,    S5048F_CPLD2_NAME, 0x3e, NULL),
	mk_i2cdev(I2C_MUX2_BUS5,    S5048F_CPLD3_NAME, 0x3e, NULL),
	mk_i2cdev(I2C_MUX2_BUS6,    S5048F_CPLD4_NAME, 0x3e, NULL),

	mk_i2cdev(I2C_MUX4_BUS0,    "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS1,    "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS2,    "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS3,    "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS4,    "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS5,    "sff8436", 0x50, &port51_50_sff8436),

	mk_i2cdev(I2C_MUX5_BUS0,    "24c04",   0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1,    "24c04",   0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2,    "24c04",   0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3,    "24c04",   0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4,    "24c04",   0x50, &port16_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5,    "24c04",   0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6,    "24c04",   0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7,    "24c04",   0x50, &port13_50_at24),

	mk_i2cdev(I2C_MUX6_BUS0,    "24c04",   0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1,    "24c04",   0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2,    "24c04",   0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3,    "24c04",   0x50, &port8_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4,    "24c04",   0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5,    "24c04",   0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6,    "24c04",   0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7,    "24c04",   0x50, &port3_50_at24),

	mk_i2cdev(I2C_MUX7_BUS0,    "24c04",   0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1,    "24c04",   0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2,    "24c04",   0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3,    "24c04",   0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4,    "24c04",   0x50, &port24_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5,    "24c04",   0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6,    "24c04",   0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7,    "24c04",   0x50, &port21_50_at24),

	mk_i2cdev(I2C_MUX8_BUS0,    "24c04",   0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1,    "24c04",   0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2,    "24c04",   0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3,    "24c04",   0x50, &port48_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4,    "24c04",   0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5,    "24c04",   0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6,    "24c04",   0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7,    "24c04",   0x50, &port44_50_at24),

	mk_i2cdev(I2C_MUX9_BUS0,    "24c04",   0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1,    "24c04",   0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2,    "24c04",   0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3,    "24c04",   0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4,    "24c04",   0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5,    "24c04",   0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6,    "24c04",   0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7,    "24c04",   0x50, &port40_50_at24),

	mk_i2cdev(I2C_MUX10_BUS0,   "24c04",   0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX10_BUS1,   "24c04",   0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX10_BUS2,   "24c04",   0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX10_BUS3,   "24c04",   0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX10_BUS4,   "24c04",   0x50, &port32_50_at24),
	mk_i2cdev(I2C_MUX10_BUS5,   "24c04",   0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX10_BUS6,   "24c04",   0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX10_BUS7,   "24c04",   0x50, &port29_50_at24),
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
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5048F Platform Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

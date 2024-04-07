/*
 * Platform device declarations and instantiations for dell_s6100
 * platforms.
 *
 * Copyright (C) 2017, 2020 Cumulus Networks, Inc.
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

/**
 * This module is a "device object container" that creates kernel
 * device objects specific to the platform.
 *
 * This module is not a device driver.
 *
 * When a device is created, an appropriate driver module is loaded by
 * the udev system.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/i2c.h>
#include <linux/i2c/pca954x.h>
#include <linux/gpio.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dell-s6100-platform.h"
#include "dell-s6100-cpld.h"
#include "dell-s6100-smf.h"

#define PLATFORM_MODULE_VERSION "0.2"

/**
 * DELL S6100 Device Hierarchy
 *
 * This platform contains the following root device objects:
 *
 * - Smart Fusion System Management Controller:
 *   bus: LPC
 *
 * - Switch Board CPLDSmart Fusion System Management Controller:
 *   bus: LPC
 *
 * - SMBus iSMT adapter:
 *   bus: i2c
 *   devices: top level PCA9547 i2c switch, board EEPROM, I/O Module i2c
 *
 * - SMBus I801 adapter
 *   bus: i2c
 *   devices: DDR DIMM, clock generator, DDR VREF tuning
 */

/**
 * DOC: I2C adapter definitions
 *
 * Define the I2C bus hierarchy, including EEPROMs, temp sensors, i2c
 * muxes and i2c switches.
 */


mk_eeprom(spd1, 50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 50, 2048, AT24_FLAG_IRUGO);

/* EEPROMs for two fixed front panel SFP+ ports.  On the face plate
 * silkscreen these ports reside in module "5" with labels "1" and
 * "2".
 */
mk_port_eeprom(portm5p1, 50, 512, AT24_FLAG_IRUGO);
mk_port_eeprom(portm5p2, 50, 512, AT24_FLAG_IRUGO);

enum {
	I2C_I801_BUS = -10,
	I2C_ISMT_BUS,

	/*
	 * top-level CPU board i2c mux, attached to ISMT adapter
	 */
	I2C_PCA9547_BUS0 = 10,
	I2C_PCA9547_BUS1,
	I2C_PCA9547_BUS2,
	I2C_PCA9547_BUS3,
	I2C_PCA9547_BUS4, /* IO Module 1 Optical SCL/SDA EEPROM MUXES */
	I2C_PCA9547_BUS5, /* IO Module 3 Optical SCL/SDA EEPROM MUXES */
	I2C_PCA9547_BUS6, /* IO Module 2 Optical SCL/SDA EEPROM MUXES */
	I2C_PCA9547_BUS7, /* IO Module 4 Optical SCL/SDA EEPROM MUXES */

	/*
	 * top-level switch board i2c switch, attached to
	 * I2C_PCA9547_BUS2 adapter
	 */
	I2C_PCA9548_2_BUS0,
	I2C_PCA9548_2_BUS1,
	I2C_PCA9548_2_BUS2,
	I2C_PCA9548_2_BUS3,
	I2C_PCA9548_2_BUS4, /* IO Module 1 CPLD, EEPROM */
	I2C_PCA9548_2_BUS5, /* IO Module 3 CPLD, EEPROM */
	I2C_PCA9548_2_BUS6, /* IO Module 2 CPLD, EEPROM */
	I2C_PCA9548_2_BUS7, /* IO Module 4 CPLD, EEPROM */
};

#define MODULE_OPTICAL_I2C_BUS_BASE I2C_PCA9547_BUS4
#define MODULE_CPLD_I2C_BUS_BASE    I2C_PCA9548_2_BUS4

mk_pca9548(pca9547, I2C_PCA9547_BUS0, 1);
mk_pca9548(pca9548_2, I2C_PCA9548_2_BUS0, 1);

/**
 * i2c_devices[]
 *
 * Array of i2c device to create
 */
static struct platform_i2c_device_info i2c_devices[] = {
	/* i801 bus devices */
	mk_i2cdev(I2C_I801_BUS, "spd",  0x50, &spd1_50_at24),

	/* iSMT bus devices */
	mk_i2cdev(I2C_ISMT_BUS, "pca9547", 0x70, &pca9547_platform_data),

	/* PCA9547 bus 0 devices */
	mk_i2cdev(I2C_PCA9547_BUS0, "24c16", 0x50, &board_50_at24),

	/* PCA9547 bus 1 is empty */

	/* PCA9547 bus 2 devices */
	mk_i2cdev(I2C_PCA9547_BUS2, "pca9548", 0x71, &pca9548_2_platform_data),

	/* PCA9547 bus 3 is empty */

	/* PCA9547 buses 4 - 7 go to the IO module optical GPIOs */

	/* PCA9548_2 bus 0 has devices, but we don't need any of them */

	/* PCA9548_2 bus 1 devices */
	mk_i2cdev(I2C_PCA9548_2_BUS1, "24c04", 0x50, &portm5p1_50_at24),

	/* PCA9548_2 bus 2 devices */
	mk_i2cdev(I2C_PCA9548_2_BUS2, "24c04", 0x50, &portm5p2_50_at24),

	/* PCA9548_2 bus 3 is empty */

	/* PCA9548_2 buses 4 - 7 go to the IO module CPLD and EEPROM */
};

#define DEFINE_RES_I2C_BUS(_bus) \
	DEFINE_RES_NAMED((_bus), 0, NULL, IORESOURCE_BUS)

/**
 * s6100_smf_res - device resources
 *
 * Defines the resources consumed by this device.
 */
static struct resource s6100_smf_res[] = {
	DEFINE_RES_IO(S6100_SMF_IO_BASE, S6100_SMF_IO_SIZE),
	DEFINE_RES_I2C_BUS(MODULE_OPTICAL_I2C_BUS_BASE),
	DEFINE_RES_I2C_BUS(MODULE_CPLD_I2C_BUS_BASE),
};

/**
 * s6100_smf_dev_info - device info
 *
 * Bundles all the device info together for passing to
 * platform_device_register_full().
 */
static struct platform_device_info s6100_smf_dev_info __initdata = {
	.name    = S6100_SMF_DRIVER_NAME,
	.id      = PLATFORM_DEVID_NONE,
	.res     = s6100_smf_res,
	.num_res = ARRAY_SIZE(s6100_smf_res),
};

/**
 * s6100_cpld_res - device resources
 *
 * Defines the resources consumed by this device.
 */
static struct resource s6100_cpld_res[] = {
	DEFINE_RES_IO(S6100_CPLD_IO_BASE, S6100_CPLD_IO_SIZE),
};

/**
 * s6100_cpld_dev_info - device info
 *
 * Bundles all the device info together for passing to
 * platform_device_register_full().
 */
static struct platform_device_info s6100_cpld_dev_info __initdata = {
	.name    = S6100_CPLD_DRIVER_NAME,
	.id      = PLATFORM_DEVID_NONE,
	.res     = s6100_cpld_res,
	.num_res = ARRAY_SIZE(s6100_cpld_res),
};

/**
 * struct s6100_platform_priv -- private platform data
 * @smf_dev:   SMF FPGA device object
 * @cpld_dev:  Switch board CPLD device object
 *
 * Structure containing private data for the s6100 platform.
 */
struct s6100_platform_priv {
	struct platform_device *smf_dev;
	struct platform_device *cpld_dev;
};

static struct s6100_platform_priv s6100_platform;

/**
 * s6100_i2c_init()
 * @priv - private data structure
 *
 * Locate the top-level i2c controllers (Intel i801 and Intel iSMT)
 * and attach the various i2c devices.
 *
 * Returns 0 on success and negative errno on failure.
 */
static int __init s6100_i2c_init(struct s6100_platform_priv *priv)
{
	int rc = 0;
	int i;
	int i801_adapter;
	int ismt_adapter;
	struct platform_i2c_device_info *info;

	i801_adapter = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_adapter < 0) {
		rc = i801_adapter;
		pr_err("%s: Unable to find %s\n",
		       THIS_MODULE->name, I801_ADAPTER_NAME);
		return rc;
	}

	ismt_adapter = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_adapter < 0) {
		rc = ismt_adapter;
		pr_err("%s: Unable to find %s\n",
		       THIS_MODULE->name, ISMT_ADAPTER_NAME);
		return rc;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		info = i2c_devices + i;
		if (info->bus == I2C_I801_BUS)
			info->bus = i801_adapter;
		else if (info->bus == I2C_ISMT_BUS)
			info->bus = ismt_adapter;
		info->client = cumulus_i2c_add_client(info->bus,
						      &info->board_info);
		if (IS_ERR(info->client)) {
			rc = PTR_ERR(info->client);
			pr_err("%s: Problems adding i2c device, bus: %d, type: %s, addr: 0x%02x (%d)\n",
			       THIS_MODULE->name,
			       info->bus, info->board_info.type,
			       info->board_info.addr, rc);
			while (--i >= 0)
				i2c_unregister_device(i2c_devices[i].client);
			return rc;
		}
	}

	return rc;
}

static int __init dell_s6100_platform_init(void)
{
	int rc = 0;

	/* Instantiate i2c adapters and related devices */
	rc = s6100_i2c_init(&s6100_platform);
	if (rc) {
		pr_err("%s: s6100_i2c_init() failed: %d\n", THIS_MODULE->name, rc);
		goto err_exit;
	}

	/* Create smart fusion device object */
	s6100_platform.smf_dev = platform_device_register_full(&s6100_smf_dev_info);
	if (IS_ERR(s6100_platform.smf_dev)) {
		rc = PTR_ERR(s6100_platform.smf_dev);
		pr_err("%s: platform_device_register_full() failed for device: %s (%d)\n",
			THIS_MODULE->name, s6100_smf_dev_info.name, rc);
		s6100_platform.smf_dev = NULL;
		goto err_exit;
	}

	/* Create switch board CPLD device object */
	s6100_platform.cpld_dev = platform_device_register_full(&s6100_cpld_dev_info);
	if (IS_ERR(s6100_platform.cpld_dev)) {
		rc = PTR_ERR(s6100_platform.cpld_dev);
		pr_err("%s: platform_device_register_full() failed for device: %s (%d)\n",
			THIS_MODULE->name, s6100_cpld_dev_info.name, rc);
		s6100_platform.cpld_dev = NULL;
		goto err_exit;
	}

	pr_info("%s: module version "PLATFORM_MODULE_VERSION" loaded ok\n",
		THIS_MODULE->name);
	return rc;

err_exit:
	if (s6100_platform.smf_dev)
		platform_device_unregister(s6100_platform.smf_dev);
	if (s6100_platform.cpld_dev)
		platform_device_unregister(s6100_platform.cpld_dev);
	pr_err("%s: unable to load module\n", THIS_MODULE->name);
	return rc;
}

static void __exit dell_s6100_platform_exit(void)
{
	int i;

	if (s6100_platform.smf_dev)
		platform_device_unregister(s6100_platform.smf_dev);
	if (s6100_platform.cpld_dev)
		platform_device_unregister(s6100_platform.cpld_dev);

	for (i = ARRAY_SIZE(i2c_devices) - 1; i >= 0; i--)
		i2c_unregister_device(i2c_devices[i].client);

	pr_info("%s: driver exit\n", THIS_MODULE->name);
}

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("DELL S6100 Platform Device Module");
MODULE_LICENSE("GPL");
MODULE_VERSION(PLATFORM_MODULE_VERSION);

module_init(dell_s6100_platform_init);
module_exit(dell_s6100_platform_exit);

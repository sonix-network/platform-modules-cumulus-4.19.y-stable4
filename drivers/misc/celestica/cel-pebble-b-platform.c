// SPDX-License-Identifier: GPL-2.0+
/*
 * cel_pebble_platform.c - Celestica Pebble E1052 with BMC Platform Support.
 * Substantially based on Celestica Pebble driver.
 *
 * Copyright 2015, 2016, 2017, 2019 Cumulus Networks, Inc.  All Rights Reserved.
 *
 * E1050 Author: Vidya Ravipati (vidya@cumulusnetworks.com)
 * E1052 Author: Dave Olson <olson@cumulusnetworks.com)
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
 * You should have received a copy of the GNU General Public License along with
 * this program.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/io.h>
#include <linux/dmi.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "cel-pebble-b-platform.h"

#define DRIVER_NAME        "cel_pebble_b"
#define DRIVER_VERSION     "1.0"

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 55, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_port_eeprom(eth0,    50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port49,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/*
 * GPIO definitions
 *
 * The GPIO infrastructure has an unfortunate interface for
 * initializing pins.  The pin names are specified in platform_data. So we
 * consolidate all the information in one place (struct cel_pebble_b_gpio_pin),
 * and try to make the best of it.  The pin name array is filled in at runtime
 * by cel_pebble_i2c_init(), before creating the I2C device.  Then the pins are
 * created by init_gpio_pins().
 */
struct cel_pebble_b_gpio_pin {
	int pin;
	const char *label;
	unsigned long flags;
	bool claimed;
};

#define mk_gpio_pin(_num, _name, _flags) \
	{ \
		.pin = (_num), \
		.label = #_name, \
		.flags = (GPIOF_EXPORT_DIR_FIXED | (_flags)) \
	}

#define mk_gpio_platform_data(_name, _base, _numpins) \
	static char const *_name##_pinnames[_numpins]; \
	static struct pca953x_platform_data _name##_platform_data = { \
		.gpio_base = (_base), \
		.names = _name##_pinnames, \
	}

static struct cel_pebble_b_gpio_pin cel_pebble_b_gpio_pins[] = {
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_1_RS,         sfp1_rs,
		    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_1_RX_LOS,     sfp1_rx_los,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_1_PRESENT_L,  sfp1_present,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_1_TX_DISABLE, sfp1_tx_enable,
		    GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_1_TX_FAULT,   sfp1_tx_fault,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_2_RS,         sfp2_rs,
		    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_2_RX_LOS,     sfp2_rx_los,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_2_PRESENT_L,  sfp2_present,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_2_TX_DISABLE, sfp2_tx_enable,
		    GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_2_TX_FAULT,   sfp2_tx_fault,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_3_RS,         sfp3_rs,
		    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_3_RX_LOS,     sfp3_rx_los,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_3_PRESENT_L,  sfp3_present,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_3_TX_DISABLE, sfp3_tx_enable,
		    GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_3_TX_FAULT,   sfp3_tx_fault,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_4_RS,         sfp4_rs,
		    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_4_RX_LOS,     sfp4_rx_los,
		    GPIOF_IN),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_4_PRESENT_L,  sfp4_present,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_4_TX_DISABLE, sfp4_tx_enable,
		    GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_SFP_4_TX_FAULT,   sfp4_tx_fault,
		    GPIOF_IN),
#ifdef USE_GPIO_INTR_EXT /* not used on E1050, nor on E1052 */
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_RTC_INT_L,        rtc_int,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_LM75B1_OS,        lm75b1_os,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_LM75B2_OS,        lm75b2_os,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_B50282_1_INT_L,   B50282_1_INT,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_B50282_2_INT_L,   B50282_2_INT,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(CEL_PEBBLE_B_GPIO_PCIE_INT_L,       pcie_int,
		    GPIOF_IN | GPIOF_ACTIVE_LOW),
#endif /* USE_GPIO_INTR_EXT */
};

mk_gpio_platform_data(cel_pebble_b_gpio, CEL_PEBBLE_B_GPIO_1_BASE,
		      CEL_PEBBLE_B_GPIO_1_PIN_COUNT);

/*
 * the iSMT bus has a single PCA9546 switch connected to the CPLD.
 * that connects the devices
 * with the switch-related i2c connected to a 9548 on port 2 of the 9546
 */
mk_pca9545(pca9546_mux, CEL_PEBBLE_B_I2C_PCA9546_BUS_0, 1);
mk_pca9548(pca9548_mux, CEL_PEBBLE_B_I2C_PCA9548_BUS_0, 1);

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct platform_i2c_device_info cel_peb_b_i2c_devices[] = {
	/* Begin I2C_I801_BUS */
	mk_i2cdev(CEL_PEBBLE_B_I2C_I801_BUS, "spd", 0x52, &spd1_52_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_I801_BUS, "pca9546", 0x74,
		  &pca9546_mux_platform_data),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9546_BUS_0, "24c04", 0x50,
		  &eth0_50_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9546_BUS_1, "24c64", 0x55,
		  &board_55_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9546_BUS_2, "pca9548", 0x71,
		  &pca9548_mux_platform_data),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9546_BUS_3, "lm75", 0x4A, NULL),
	/* PCA9548 BUS 0 is RTC, but it's working without this */
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_1, "pca9505", 0x20,
		  &cel_pebble_b_gpio_platform_data),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_2, "24c04", 0x50,
		  &port49_50_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_3, "24c04", 0x50,
		  &port50_50_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_4, "24c04", 0x50,
		  &port51_50_at24),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_5, "24c04", 0x50,
		  &port52_50_at24),
#ifdef EXPOSE_DS_RETIMER /*  these are TI DS110DF111 for port 51, 52 */
	/* See page 87 of HW Design Spec and diagram on page 28.
	 * Not exposing at this time.
	 */
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_6, "retimer", 0x18, NULL),
	mk_i2cdev(CEL_PEBBLE_B_I2C_PCA9548_BUS_6, "retimer", 0x19, NULL),
#endif /* EXPOSE_DS_RETIMER these are TI DS110DF111 */
};

/**
 * cel_pebble_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
#if defined CONFIG_DMI
static const struct dmi_system_id plat_dmi_table[] = {
	{ /* Celestica 1052 */
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "E1052"),
		},
	},
};
#endif

static int init_gpio_pins(struct platform_device *dev,
			  struct cel_pebble_b_gpio_pin *gpio_pin,
			  int num_pins, int gpio_base)
{
	int i;
	int ret;

	for (i = 0; i < num_pins; i++, gpio_pin++) {
		if (gpio_pin->claimed)
			continue;
		ret = gpio_request_one(gpio_base + gpio_pin->pin,
				       gpio_pin->flags, gpio_pin->label);
		if (ret) {
			if (ret == -EPROBE_DEFER)
				return ret;
			dev_err(&dev->dev,
				": Failed to request %d GPIO pin %s, err %d\n",
				gpio_base + gpio_pin->pin, gpio_pin->label,
				ret);
			return ret;
		}
		gpio_pin->claimed = true;
	}
	return 0;
}

static void free_gpio_pins(struct cel_pebble_b_gpio_pin *gpio_pin,
			   int num_pins, int gpio_base)
{
	int i;

	for (i = 0; i < num_pins; i++, gpio_pin++) {
		if (gpio_pin->claimed) {
			gpio_free(gpio_base + gpio_pin->pin);
			gpio_pin->claimed = false;
		}
	}
}

static int populate_i2c_devices(struct platform_i2c_device_info *devices,
				int num_devices, int I801_bus)
{
	int i;
	int ret = 0;
	struct i2c_client *client;

	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == CEL_PEBBLE_B_I2C_I801_BUS)
			devices[i].bus = I801_bus;
		client = cumulus_i2c_add_client(devices[i].bus,
						&devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			break;
		}
		devices[i].client = client;
	}
	return ret;
}

static void free_cel_pebble_i2c_data(void)
{
	int i = ARRAY_SIZE(cel_peb_b_i2c_devices);

	while (--i >= 0) {
		if (cel_peb_b_i2c_devices[i].client) {
			i2c_unregister_device(cel_peb_b_i2c_devices[i].client);
			cel_peb_b_i2c_devices[i].client = NULL;
		}
	}
}

/*---------------------------------------------------------------------
 *
 * Platform driver probe/remove
 */
static int platform_probe(struct platform_device *dev)
{
	int I801_bus;
	int ret = -1, i;

	I801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		dev_err(&dev->dev, "could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/*
	 * populate the platform_data structure for the gpio with
	 * our pin names
	 */
	for (i = 0; i < ARRAY_SIZE(cel_pebble_b_gpio_pins); i++) {
		cel_pebble_b_gpio_pinnames[cel_pebble_b_gpio_pins[i].pin] =
				cel_pebble_b_gpio_pins[i].label;
	}

	/*
	 * populate the i2c devices
	 */
	ret = populate_i2c_devices(cel_peb_b_i2c_devices,
				   ARRAY_SIZE(cel_peb_b_i2c_devices), I801_bus);
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(dev, cel_pebble_b_gpio_pins,
			     ARRAY_SIZE(cel_pebble_b_gpio_pins),
			     CEL_PEBBLE_B_GPIO_1_BASE);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_cel_pebble_i2c_data();
	return ret;
}

static int platform_remove(struct platform_device *dev)
{
	free_gpio_pins(cel_pebble_b_gpio_pins,
		       ARRAY_SIZE(cel_pebble_b_gpio_pins),
		       CEL_PEBBLE_B_GPIO_1_BASE);
	free_cel_pebble_i2c_data();
	return 0;
}

/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static const struct platform_device_id cel_pebble_b_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, cel_pebble_b_platform_id);

static struct platform_driver platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = platform_probe,
	.remove = platform_remove,
	.id_table = cel_pebble_b_platform_id
};

static struct platform_device *plat_device;

static int __init cel_pebble_b_init(void)
{
	int ret = 0;

#if defined CONFIG_DMI
	if (!dmi_check_system(plat_dmi_table)) {
		pr_info(DRIVER_NAME
			": Error, platform is not Pebble BMC E1052\n");
	return -ENODEV;
	}
#endif
	/* Register the platform driver */
	if (!driver_find(platform_driver.driver.name, &platform_bus_type)) {
		ret = platform_driver_register(&platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME
			       ": %s driver registration failed.(%d)\n",
			       platform_driver.driver.name, ret);
			return ret;
		}
	}

	/* Create the platform device */
	if (!plat_device) {
		plat_device = platform_device_register_simple(DRIVER_NAME, -1,
							      NULL, 0);
		if (IS_ERR(plat_device)) {
			ret = PTR_ERR(plat_device);
			plat_device = NULL;
			pr_err(DRIVER_NAME
			       ": Platform device registration failed. (%d)\n",
			       ret);
			platform_driver_unregister(&platform_driver);
			return ret;
		}
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");
	return 0;
}

static void __exit cel_pebble_b_exit(void)
{
	platform_device_unregister(plat_device);
	plat_device = NULL;
	platform_driver_unregister(&platform_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(cel_pebble_b_init);
module_exit(cel_pebble_b_exit);

MODULE_AUTHOR("Dave Olson <olson@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Pebble BMC E1052 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

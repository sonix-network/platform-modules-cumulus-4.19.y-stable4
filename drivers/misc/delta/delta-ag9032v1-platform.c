// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v1 Platform Support.
 *
 * Copyright (C) 2016, 2017, 2019 Cumulus Networks, Inc.  All Rights Reserved.
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
#include "delta-ag9032v1.h"

#define DRIVER_NAME    AG9032V1_PLATFORM_NAME
#define DRIVER_VERSION "1.1"

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

mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);

mk_eeprom(fan1,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,	 52, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,	 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,	 54, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan5,	 55, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_eeprom(psu1,	 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,	 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

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
 * GPIO definitions
 *
 * The GPIO infrastructure has an unfortunate interface for initializing pins.
 * The pin names are specified in platform_data, but everthing else has to be
 * done by calls after pin creation.  So we consolidate all the information in
 * one place (struct gpio_pin), and try to make the best of it.	 The pin name
 * array is filled in at runtime by init_gpio_platform_data(), before creating
 * the I2C device.  Then the pins are created by init_gpios() which is a
 * callback from end of the gpio controller's probe function.
 */

struct gpio_pin {
	int pin;
	const char *label;
	unsigned long flags;
};

#define mk_gpio_pins(_name) \
	static struct gpio_pin _name##_pins[]
#define mk_gpio_pin(_num, _name, _flags) \
	{ \
		.pin = (_num), \
		.label = #_name, \
		.flags = (GPIOF_EXPORT_DIR_FIXED | (_flags)) \
	}

#define mk_gpio_platform_data(_name, _base, _numpins, _pins, _setup, \
			      _teardown)			     \
	static char const *_name##_pinnames[_numpins]; \
	static struct pca953x_platform_data _name##_platform_data = { \
		.gpio_base = (_base), \
		.names = _name##_pinnames, \
		.context = _pins, \
		.setup = _setup, \
		.teardown = _teardown, \
	}

mk_gpio_pins(gpio) = {
	mk_gpio_pin(0,  fan_tray5_absent, GPIOF_IN),
	mk_gpio_pin(1,  fan_tray4_absent, GPIOF_IN),
	mk_gpio_pin(2,  fan_tray3_absent, GPIOF_IN),
	mk_gpio_pin(3,  fan_tray2_absent, GPIOF_IN),
	mk_gpio_pin(4,  fan_tray1_absent, GPIOF_IN),
	mk_gpio_pin(5,  unused_pin5,      GPIOF_IN),
	mk_gpio_pin(6,  unused_pin6,      GPIOF_IN),
	mk_gpio_pin(7,  unused_pin7,      GPIOF_IN),
	mk_gpio_pin(8,  unused_pin8,      GPIOF_IN),
	mk_gpio_pin(9,  unused_pin9,      GPIOF_IN),
	mk_gpio_pin(10, unused_pin10,     GPIOF_IN),
	mk_gpio_pin(11, unused_pin11,     GPIOF_IN),
	mk_gpio_pin(12, unused_pin12,     GPIOF_IN),
	mk_gpio_pin(13, unused_pin13,     GPIOF_IN),
	mk_gpio_pin(14, unused_pin14,     GPIOF_IN),
	mk_gpio_pin(15, unused_pin15,     GPIOF_IN),
};

static int free_gpios(struct i2c_client *client, unsigned int gpio_base,
		      unsigned int num_pins, void *c)

{
	struct gpio_pin *pins = c;

	while (num_pins--)
		gpio_free(gpio_base + (pins++)->pin);
	return 0;
}

static int init_gpios(struct i2c_client *client, unsigned int gpio_base,
		      unsigned int num_pins, void *c)
{
	struct device *dev = &client->dev;
	int i;
	int ret;
	struct gpio_pin *pins = c;

	dev_info(dev, "Registering %d GPIO pins\n", num_pins);
	for (i = 0; i < num_pins; i++, pins++) {
		ret = gpio_request_one(gpio_base + pins->pin, pins->flags,
				       pins->label);
		if (ret) {
			dev_err(dev, "failed to request GPIO pin %s (%d), err %d\n",
				pins->label, gpio_base + pins->pin, ret);
			goto err_exit;
		}
	}
	return 0;

err_exit:
	while (i--)
		gpio_free(gpio_base + (--pins)->pin);
	return ret;
}

mk_gpio_platform_data(gpio, 430, 16, &gpio_pins, init_gpios, free_gpios);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.  Each entry is a bus
 * number and a i2c_board_info.  The i2c_board_info structure specifies the
 * device type, address, and platform data specific to the device type.
 */

static struct cpld_item swcpld_item;

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS,	 "pca9547",   0x71, &mux1_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",     0x53, &board_53_at24),
	mk_i2cdev(I2C_MUX1_BUS0, AG9032V1_CPUPLD_NAME, 0x31, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "tmp75",     0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS4, AG9032V1_SWCPLD_NAME, 0x31, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",     0x4c, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",     0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",     0x4e, NULL),

	/*
	 * These three drivers make use of registers in SWCPLD so they must
	 * come *after* the SWCPLD instantiation in this table.
	 */
	mk_i2cdev(I2C_MUX1_BUS1, AG9032V1_FANMUX_NAME, 0x7f, &swcpld_item),
	mk_i2cdev(I2C_MUX1_BUS2, AG9032V1_PSUMUX_NAME, 0x7f, &swcpld_item),
	mk_i2cdev(I2C_MUX1_BUS3, AG9032V1_SFFMUX_NAME, 0x7f, &swcpld_item),

	/* fan mux devices */
	mk_i2cdev(CPLD_FAN_MUX_BUS0, "24c02",	0x51, &fan1_51_at24),
	mk_i2cdev(CPLD_FAN_MUX_BUS1, "24c02",	0x52, &fan2_52_at24),
	mk_i2cdev(CPLD_FAN_MUX_BUS2, "24c02",	0x53, &fan3_53_at24),
	mk_i2cdev(CPLD_FAN_MUX_BUS3, "24c02",	0x54, &fan4_54_at24),
	mk_i2cdev(CPLD_FAN_MUX_BUS4, "24c02",	0x55, &fan5_55_at24),
	mk_i2cdev(CPLD_FAN_MUX_BUS5, "emc2305", 0x2c, NULL),
	mk_i2cdev(CPLD_FAN_MUX_BUS5, "emc2305", 0x2d, NULL),
	mk_i2cdev(CPLD_FAN_MUX_BUS6, "lm75a",	0x4f, NULL),
	mk_i2cdev(CPLD_FAN_MUX_BUS7, "pca9555", 0x27, &gpio_platform_data),

	/* psu mux devices */
	mk_i2cdev(CPLD_PSU_MUX_BUS0, "24c02",	0x50, &psu1_50_at24),
	mk_i2cdev(CPLD_PSU_MUX_BUS0, "dps460",	0x58, NULL),
	mk_i2cdev(CPLD_PSU_MUX_BUS1, "ltc4215", 0x40, NULL),
	mk_i2cdev(CPLD_PSU_MUX_BUS2, "24c02",	0x50, &psu2_50_at24),
	mk_i2cdev(CPLD_PSU_MUX_BUS2, "dps460",	0x58, NULL),
	mk_i2cdev(CPLD_PSU_MUX_BUS3, "ltc4215", 0x40, NULL),

	/* sff mux devices */
	mk_i2cdev(CPLD_QSFP_MUX_BUS0,  "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS1,  "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS2,  "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS3,  "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS4,  "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS5,  "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS6,  "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS7,  "sff8436", 0x50, &port8_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS8,  "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS9,  "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS10, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS11, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS12, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS13, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS14, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS15, "sff8436", 0x50, &port16_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS16, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS17, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS18, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS19, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS20, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS21, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS22, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS23, "sff8436", 0x50, &port24_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS24, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS25, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS26, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS27, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS28, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS29, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS30, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(CPLD_QSFP_MUX_BUS31, "sff8436", 0x50, &port32_50_sff8436),
};

#define NUM_I2C_DEVICES ARRAY_SIZE(i2c_devices)

/* i2c init */

static void init_gpio_platform_data(struct gpio_pin *pins,
				    int num_pins,
				    struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < num_pins; i++)
		names[pins[i].pin] = pins[i].label;
}

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

	mutex_init(&swcpld_item.cpld_mutex);

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

		/* Save the client of the SWCPLD. */
		if (!strcmp(pdi->board_info.type, AG9032V1_SWCPLD_NAME) &&
		    pdi->board_info.addr == 0x31)
			swcpld_item.cpld_client = pdi->client;
		pdi++;
	}
	return 0;
}

static int platform_probe(struct platform_device *dev)
{
	int ismt_bus;
	int i801_bus;
	int ret;

	/* apply the gpio names */
	init_gpio_platform_data(gpio_pins, ARRAY_SIZE(gpio_pins),
				&gpio_platform_data);

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

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Delta AG9032v1 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

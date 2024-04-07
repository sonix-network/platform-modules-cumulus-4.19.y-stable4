 /*
 * qct_ly4r_platform.c - Quanta LY4R Platform Support.
 *
 * Copyright (C) 2017, 2020 Cumulus Networks, Inc. all rights reserved
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/pmbus.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/cumulus-platform.h>
#include "platform-defs.h"

#define DRIVER_NAME	"qct_ly4r_platform"
#define DRIVER_VERSION	"0.1"

/*---------------------------------------------------------------------
 *
 * Platform driver
 *
 *-------------------------------------------------------------------*/

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport). 
 */

/* i2c bus adapter numbers for the i2c busses */
enum {
	CL_I2C_I801_BUS = 0,
	CL_I2C_ISMT_BUS,
	CL_I2C_MUX1_BUS0=10,
	CL_I2C_MUX1_BUS1=11,
	CL_I2C_MUX1_BUS2=12,
	CL_I2C_MUX1_BUS3=13,
	CL_I2C_MUX1_BUS4=14,
	CL_I2C_MUX1_BUS5=15,
	CL_I2C_MUX1_BUS6=16,
	CL_I2C_MUX1_BUS7=17,
	CL_I2C_MUX2=100,
};

mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

struct cl_platform_device_info {
	int bus;
	struct i2c_board_info board_info;
};

struct port_info {
	struct i2c_board_info *b;
	struct i2c_client *c;
};

/*
 * platform data for the gpio chip
 */
#define QCT_LY4R_GPIO_PIN_COUNT          19
#define QCT_LY4R_GPIO_BASE               412

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

static int free_gpios(struct i2c_client *client, unsigned int gpio_base,
		      unsigned int num_pins, void *c)

{
	struct gpio_pin *pins = c;
	unsigned int used_pins = QCT_LY4R_GPIO_PIN_COUNT;

	while (used_pins--)
		gpio_free(gpio_base + (pins++)->pin);
	return 0;
}

static int init_gpios(struct i2c_client *client, unsigned int gpio_base,
		      unsigned int num_pins, void *c)
{
	struct device *dev = &client->dev;
	int i, retries, ret;
	struct gpio_pin *pins = c;

	dev_info(dev, "Registering %d GPIO pins\n", QCT_LY4R_GPIO_PIN_COUNT);
	for (i = 0; i < QCT_LY4R_GPIO_PIN_COUNT; i++, pins++) {
		for (retries = 0; retries < 3; retries++) {
			ret = gpio_request_one(gpio_base + pins->pin,
					       pins->flags,
					       pins->label);
			if (!ret) {
				break;
			}
			msleep(100);
		}
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

static void init_gpio_platform_data(struct gpio_pin *pins,
				    int num_pins,
				    struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < QCT_LY4R_GPIO_PIN_COUNT; i++)
		names[pins[i].pin] = pins[i].label;
}

mk_gpio_pins(qct_ly4r_gpio) = {
	mk_gpio_pin(0, sfp49_tx_fault,
		    GPIOF_DIR_IN),

	mk_gpio_pin(1, sfp49_tx_disable,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(2, sfp49_present,
		    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(3, sfp49_rx_los,
		    GPIOF_DIR_IN),

	mk_gpio_pin(4, sfp50_tx_fault,
		    GPIOF_DIR_IN),

	mk_gpio_pin(5, sfp50_tx_disable,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(6, sfp50_present,
		    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(7, sfp50_rx_los,
		    GPIOF_DIR_IN),

	mk_gpio_pin(8, sfp51_tx_fault,
		    GPIOF_DIR_IN),

	mk_gpio_pin(9, sfp51_tx_disable,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(10, sfp51_present,
		    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(11, sfp51_rx_los,
		    GPIOF_DIR_IN),

	mk_gpio_pin(12, sfp52_tx_fault,
		    GPIOF_DIR_IN),

	mk_gpio_pin(13, sfp52_tx_disable,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(14, sfp52_present,
		    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(15, sfp52_rx_los,
		    GPIOF_DIR_IN),

	mk_gpio_pin(20, led_boot,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(21, led_status,
		    (GPIOF_DIR_OUT | GPIOF_OUT_INIT_LOW)),

	mk_gpio_pin(22, sfp_power_enable,
		    (GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW | GPIOF_OUT_INIT_LOW)),
};

mk_gpio_platform_data(qct_ly4r_gpio, QCT_LY4R_GPIO_BASE,
		      QCT_LY4R_GPIO_PIN_COUNT,
		      &qct_ly4r_gpio_pins, init_gpios, free_gpios);

mk_port_eeprom(port49,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/*
 * PCA9548 at iSMT 0x73
 */
mk_pca9548(mux1, CL_I2C_MUX1_BUS0, 1);

static struct cl_platform_device_info i2c_devices[] = {
	mk_i2cdev(CL_I2C_ISMT_BUS,  "24c02",   0x54, &board_54_at24),
	mk_i2cdev(CL_I2C_ISMT_BUS,  "pca9548", 0x73, &mux1_platform_data),
	mk_i2cdev(CL_I2C_ISMT_BUS,  "pca9698", 0x20,
		  &qct_ly4r_gpio_platform_data),
	mk_i2cdev(CL_I2C_MUX1_BUS0, "24c02",   0x50, &port49_50_at24),
	mk_i2cdev(CL_I2C_MUX1_BUS1, "24c02",   0x50, &port50_50_at24),
	mk_i2cdev(CL_I2C_MUX1_BUS2, "24c02",   0x50, &port51_50_at24),
	mk_i2cdev(CL_I2C_MUX1_BUS3, "24c02",   0x50, &port52_50_at24),
};

/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
struct i2c_client_info {
	struct i2c_client *i2c_client;
	struct cl_platform_device_info *platform_info;
};

static struct i2c_client_info i2c_clients[ARRAY_SIZE(i2c_devices)];
static int num_i2c_clients;

static void free_i2c_data(void)
{
	int i;
	/*
	 * Free the devices in reverse order so that child devices are
	 * freed before parent mux devices.
	 */
	for (i = num_i2c_clients - 1; i >= 0; i--)
		i2c_unregister_device(i2c_clients[i].i2c_client);
}

static int ismt_bus_num;
static int i801_bus_num;

/*
 * Initialize I2C devices
 *
 */
static int init_i2c_devices(void)
{
	struct i2c_client *client;
	int i;
	int ret = -1;

	ismt_bus_num = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus_num < 0) {
		pr_err("could not find ismt adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	i801_bus_num = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus_num < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		/*
		 * Map logical buses CL_I2C_ISMT_BUS and
		 * CL_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case CL_I2C_ISMT_BUS:
			i2c_devices[i].bus = ismt_bus_num;
			break;
		case CL_I2C_I801_BUS:
			i2c_devices[i].bus = i801_bus_num;
			break;
		default:
			break;
			/* Fall through for PCA9548 buses */
		};
		client = cumulus_i2c_add_client(i2c_devices[i].bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_clients[num_i2c_clients].platform_info = &i2c_devices[i];
		i2c_clients[num_i2c_clients++].i2c_client = client;
	}
	return 0;
err_exit:
	free_i2c_data();
	return ret;
}

static void qct_ly4r_i2c_exit(void)
{
	free_i2c_data();
}

/*
 *  Platform driver
 */
static int platform_probe(struct platform_device *dev)
{
	int ret;

	init_gpio_platform_data(qct_ly4r_gpio_pins,
				ARRAY_SIZE(qct_ly4r_gpio_pins),
				&qct_ly4r_gpio_platform_data);
	ret = init_i2c_devices();
	if (ret) {
		dev_err(&dev->dev, "I2C initialization failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

static int platform_remove(struct platform_device *dev)
{
	qct_ly4r_i2c_exit();
	return 0;
}

static const struct platform_device_id qct_ly4r_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, qct_ly4r_platform_id);

static struct platform_driver platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = platform_probe,
	.remove = platform_remove,
	.id_table = qct_ly4r_platform_id
};

static struct platform_device *plat_device;

static int __init
qct_ly4r_platform_init(void)
{
	int ret = 0;

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
	return ret;

}

static void __exit
qct_ly4r_platform_exit(void)
{
	platform_device_unregister(plat_device);
	plat_device = NULL;
	platform_driver_unregister(&platform_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(qct_ly4r_platform_init);
module_exit(qct_ly4r_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta LY4R Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

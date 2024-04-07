/*
 * quanta-utils.c - Quanta Utility APIs.
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved
 * Author: Pradeep Srinivasan (pradeeps@cumulusnetworks.com)
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
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/gpio.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-utils.h"

#define QUANTA_UTILS_PLATFORM_MODULE_VERSION "1.1"

/*
 * Utility functions for GPIO
 */

void init_gpio_platform_data(struct gpio_pin *pins,
			     int num_pins,
			     struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < num_pins; i++) {
		names[pins[i].num] = pins[i].name;
	}
}
EXPORT_SYMBOL_GPL(init_gpio_platform_data);

int init_gpio_pins(struct i2c_client *client,
		    unsigned int gpio_base,
		    unsigned int num_pins,
		    void *c)
{
	struct device *dev = &client->dev;
	int i;
	int ret;
	struct gpio_pin *pins = c;

	dev_info(dev, "Registering %d GPIO pins\n", num_pins);
	for (i = 0; i < num_pins; i++, pins++) {
		dev_info(dev, "QCT: gpio pin base = %x pinnum = %d name = %s \n", 
			 gpio_base,
			 pins->num,
			 pins->name);
		ret = gpio_request_one(gpio_base + pins->num,
				       pins->flags,
				       pins->name);
		if (ret) {
			dev_err(dev, "Failed to request %x GPIO pin"
					" %s, err %d\n", gpio_base + pins->num,
					pins->name, ret);
			goto err_exit;
		}
	}

	dev_info(dev, "Succeeeded in gpio pins create \n");
	return 0;

err_exit:
	while(i--) {
		gpio_free(gpio_base + (--pins)->num);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(init_gpio_pins);

int free_gpio_pins(struct i2c_client *client,
		    unsigned int gpio_base,
		    unsigned int num_pins,
		    void *c)
{
	struct gpio_pin *pins = c;

	while (num_pins--) {
		gpio_free(gpio_base + (pins++)->num);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(free_gpio_pins);

MODULE_AUTHOR("Pradeep Srinivasan <pradeeps@cumulusnetworks.com");
MODULE_DESCRIPTION("Quanta Platform Utils Library");
MODULE_LICENSE("GPL");
MODULE_VERSION(QUANTA_UTILS_PLATFORM_MODULE_VERSION);

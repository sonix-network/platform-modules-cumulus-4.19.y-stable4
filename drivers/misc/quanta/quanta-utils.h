/*
 * Quanta Platform Definitions
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved
 * Pradeep Srinivasan <pradeeps@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */
#ifndef _QUANTA_UTILS_H__
#define _QUANTA_UTILS_H__

#define SMB_I801_NAME       "SMBus I801 adapter"
#define PCA_9555_GPIO_COUNT 16
#define PCA_9698_GPIO_COUNT 40

/*
 * GPIO definitions
 *
 * The GPIO infrastructure has an unfortunate interface for
 * initializing pins.  The pin names are specified in platform_data,
 * but everthing else has to be done by calls after pin creation.
 * So we consolidate all the information in one place (struct gpio_pin),
 * and try to make the best of it.  The pin name array is filled in
 * at runtime by init_gpio_platform_data(), before creating the I2C
 * device.  Then the pins are created by init_gpio_pins().
 */

struct gpio_pin {
	int num;
	const char *name;
	unsigned long flags;
};

#define mk_gpio_pins(_name) \
	static struct gpio_pin _name##_pins[]
#define mk_gpio_pin(_num, _name, _flags) \
	{ \
		.num = (_num), \
		.name = #_name, \
		.flags = (GPIOF_EXPORT_DIR_FIXED | (_flags)) \
	}
#define mk_gpio_platform_data(_name, _base, _numpins, _pins, _setup, \
			      _teardown) \
	static char const *_name##_pinnames[_numpins]; \
	static struct pca953x_platform_data _name##_platform_data = { \
		.gpio_base = (_base), \
		.names = _name##_pinnames, \
		.context = _pins, \
		.setup = _setup, \
		.teardown = _teardown, \
	}

/*
 * Utility functions for GPIO
 */
void init_gpio_platform_data(struct gpio_pin *pins,
			     int num_pins,
			     struct pca953x_platform_data *pdata);

int init_gpio_pins(struct i2c_client *client,
		   unsigned int gpio_base,
		   unsigned int num_pins,
		   void* c);
int free_gpio_pins(struct i2c_client *client,
		    unsigned int gpio_base,
		    unsigned int num_pins,
		    void* c);

#endif

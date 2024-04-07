// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AGV848v1 Platform Driver
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_device.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "delta-agv848v1.h"

#define DRIVER_NAME    AGV848V1_PLATFORM_NAME
#define DRIVER_VERSION "1.0"

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
 * For eeproms, we specify the label, i2c address, size, and some flags, all
 * done in mk*_eeprom() macros.  The label is the string that ends up in
 * /sys/class/eeprom_dev/eepromN/label, which we use to identify them at user
 * level.
 *
 */

mk_pca9548(mux1,  I2C_MUX1_BUS0,  1);
mk_pca9548(mux2,  I2C_MUX2_BUS0,  1);
mk_pca9548(mux3,  I2C_MUX3_BUS0,  1);
mk_pca9548(mux4,  I2C_MUX4_BUS0,  1);
mk_pca9548(mux5,  I2C_MUX5_BUS0,  1);
mk_pca9548(mux6,  I2C_MUX6_BUS0,  1);
mk_pca9548(mux7,  I2C_MUX7_BUS0,  1);
mk_pca9548(mux8,  I2C_MUX8_BUS0,  1);
mk_pca9548(mux9,  I2C_MUX9_BUS0,  1);
mk_pca9548(mux10, I2C_MUX10_BUS0, 1);

mk_eeprom(board,   54,  256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom1, 50, 1024, AT24_FLAG_IRUGO);
mk_eeprom(psu1,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan1,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,    50,  256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

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
mk_qsfp_port_eeprom(port55, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port56, 50, 256, SFF_8436_FLAG_IRUGO);

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
	mk_gpio_pin(0,  unused_pin0,       GPIOF_IN),
	mk_gpio_pin(1,  fan_tray4_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,  fan_tray3_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,  fan_tray2_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,  fan_tray1_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,  fan_eeprom_wp,     GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,  d_fan_alert,       GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,  thermal_fan_int,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,  unused_pin8,       GPIOF_IN),
	mk_gpio_pin(9,  fan_tray4_b2f,     GPIOF_IN),
	mk_gpio_pin(10, fan_tray3_b2f,     GPIOF_IN),
	mk_gpio_pin(11, fan_tray2_b2f,     GPIOF_IN),
	mk_gpio_pin(12, fan_tray1_b2f,     GPIOF_IN),
	mk_gpio_pin(13, unused_pin13,      GPIOF_IN),
	mk_gpio_pin(14, unused_pin14,      GPIOF_IN),
	mk_gpio_pin(15, unused_pin15,      GPIOF_IN),
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

mk_gpio_platform_data(gpio, 1, 16, &gpio_pins, init_gpios, free_gpios);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.  Each entry is a bus
 * number and a i2c_board_info.  The i2c_board_info structure specifies the
 * device type, address, and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on the iSMT bus */
	mk_i2cdev(I2C_ISMT_BUS,  AGV848V1_CPUPLD_NAME, 0x31, NULL),
	mk_i2cdev(I2C_ISMT_BUS,  "24c02",    0x54, &board_54_at24),
	mk_i2cdev(I2C_ISMT_BUS,  "pca9548",  0x70, &mux1_platform_data),

	/* mux1 */
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x71, &mux2_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "24c08",    0x50, &eeprom1_50_at24),
	mk_i2cdev(I2C_MUX1_BUS2, "tmp75",    0x4d, NULL), /* U41 MAC right */
	mk_i2cdev(I2C_MUX1_BUS2, "tmp75",    0x4e, NULL), /* U42 MAC left */
	mk_i2cdev(I2C_MUX1_BUS2, "tmp75",    0x4f, NULL), /* U12 CPU board */
	mk_i2cdev(I2C_MUX1_BUS3, "pca9548",  0x71, &mux3_platform_data),
	mk_i2cdev(I2C_MUX1_BUS4, "24c02",    0x50, &psu1_50_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "pmbus",    0x58, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "24c02",    0x50, &psu2_50_at24),
	mk_i2cdev(I2C_MUX1_BUS5, "pmbus",    0x58, NULL),
	mk_i2cdev(I2C_MUX1_BUS7, AGV848V1_SWPLD1_NAME, 0x40, NULL),
	mk_i2cdev(I2C_MUX1_BUS7, AGV848V1_SWPLD2_NAME, 0x41, NULL),
	mk_i2cdev(I2C_MUX1_BUS7, AGV848V1_SWPLD3_NAME, 0x42, NULL),
	mk_i2cdev(I2C_MUX1_BUS7, AGV848V1_SWPLD4_NAME, 0x43, NULL),

	/* mux2 */
	mk_i2cdev(I2C_MUX2_BUS1, "24c02",    0x50, &fan1_50_at24),
	mk_i2cdev(I2C_MUX2_BUS2, "24c02",    0x50, &fan2_50_at24),
	mk_i2cdev(I2C_MUX2_BUS3, "24c02",    0x50, &fan3_50_at24),
	mk_i2cdev(I2C_MUX2_BUS4, "24c02",    0x50, &fan4_50_at24),
	mk_i2cdev(I2C_MUX2_BUS5, "emc2305",  0x2c, NULL),
	mk_i2cdev(I2C_MUX2_BUS5, "emc2305",  0x2d, NULL),
	mk_i2cdev(I2C_MUX2_BUS6, "tmp75",    0x4f, NULL), /* U334 fan board */
	mk_i2cdev(I2C_MUX2_BUS7, "pca9555",  0x27, &gpio_platform_data),

	/* mux3 */
	mk_i2cdev(I2C_MUX3_BUS0, "pca9548",  0x72, &mux4_platform_data),
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548",  0x72, &mux5_platform_data),
	mk_i2cdev(I2C_MUX3_BUS2, "pca9548",  0x72, &mux6_platform_data),
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548",  0x72, &mux7_platform_data),
	mk_i2cdev(I2C_MUX3_BUS4, "pca9548",  0x72, &mux8_platform_data),
	mk_i2cdev(I2C_MUX3_BUS5, "pca9548",  0x72, &mux9_platform_data),
	mk_i2cdev(I2C_MUX3_BUS6, "pca9548",  0x72, &mux10_platform_data),

	/* mux4: swp1 - swp8 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c04",    0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c04",    0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c04",    0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c04",    0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c04",    0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c04",    0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX4_BUS6, "24c04",    0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX4_BUS7, "24c04",    0x50, &port8_50_at24),

	/* mux5: swp9 - swp16 */
	mk_i2cdev(I2C_MUX5_BUS0, "24c04",    0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1, "24c04",    0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2, "24c04",    0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3, "24c04",    0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4, "24c04",    0x50, &port13_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5, "24c04",    0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6, "24c04",    0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7, "24c04",    0x50, &port16_50_at24),

	/* mux6: swp17 - swp24 */
	mk_i2cdev(I2C_MUX6_BUS0, "24c04",    0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1, "24c04",    0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2, "24c04",    0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3, "24c04",    0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4, "24c04",    0x50, &port21_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5, "24c04",    0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6, "24c04",    0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7, "24c04",    0x50, &port24_50_at24),

	/* mux7: swp25 - swp32 */
	mk_i2cdev(I2C_MUX7_BUS0, "24c04",    0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1, "24c04",    0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2, "24c04",    0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3, "24c04",    0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4, "24c04",    0x50, &port29_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5, "24c04",    0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6, "24c04",    0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7, "24c04",    0x50, &port32_50_at24),

	/* mux8: swp33 - swp40 */
	mk_i2cdev(I2C_MUX8_BUS0, "24c04",    0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1, "24c04",    0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2, "24c04",    0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3, "24c04",    0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4, "24c04",    0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5, "24c04",    0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6, "24c04",    0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7, "24c04",    0x50, &port40_50_at24),

	/* mux9: swp41 - swp48 */
	mk_i2cdev(I2C_MUX9_BUS0, "24c04",    0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1, "24c04",    0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2, "24c04",    0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3, "24c04",    0x50, &port44_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4, "24c04",    0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5, "24c04",    0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6, "24c04",    0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7, "24c04",    0x50, &port48_50_at24),

	/* mux10: swp49 - swp56 */
	mk_i2cdev(I2C_MUX10_BUS0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS3, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS5, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS6, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS7, "sff8436", 0x50, &port56_50_sff8436),
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
MODULE_DESCRIPTION("Delta AGV848v1 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

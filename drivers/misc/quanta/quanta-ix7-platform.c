/*
 * quanta_ix7_platform.c - Quanta IX7 Platform Support.
 *
 * Copyright (C) 2018, 2019 Cumulus Networks, Inc.  All Rights Reserved
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
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/gpio.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-ix7.h"

#define DRIVER_NAME	"quanta_ix7_platform"
#define DRIVER_VERSION	"1.0"

/*
 * The ismt bus is connected to the following devices on the CPU board:
 *
 *    pca9546 4-channel mux (0x71)
 *	 0 pca9555 io expander (0x20)
 *	 2 pca9554 io expander (0x21)
 *	 3 pca9555 io expander (0x22)
 *    so-dimm spd eeprom (0x52)
 *
 * The i801 bus is connected to the following devices on the main board:
 *    pca9546 4-channel mux (0x72)
 *	 0 cpld#2 io expander qsfp28 1-16 (0x38)
 *	   cpld#1 port leds qsfp28 1-16 (0x39)
 *	 1 cpld#3 io expander qsfp28 17-32 (0x38)
 *	   cpld#4 port leds qsfp28 17-32 (0x39)
 *	 2 board eeprom (0x54)
 *	 3 pca9555 8-bit io expander for board id (0x23)
 *    pca9548 8-channel mux (0x77)
 *	 0 pca9548 8 channel mux qsfp28 1-8 (0x73)
 *	 1 pca9548 8 channel mux qsfp28 9-16 (0x73)
 *	 2 pca9548 8 channel mux qsfp28 17-24 (0x73)
 *	 3 pca9548 8 channel mux qsfp28 25-32 (0x73)
 */

enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS = 1,

	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,

	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,

	I2C_MUX3_BUS0,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	I2C_MUX4_BUS0 = 31,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	I2C_MUX5_BUS0,
	I2C_MUX5_BUS1,
	I2C_MUX5_BUS2,
	I2C_MUX5_BUS3,
	I2C_MUX5_BUS4,
	I2C_MUX5_BUS5,
	I2C_MUX5_BUS6,
	I2C_MUX5_BUS7,

	I2C_MUX6_BUS0,
	I2C_MUX6_BUS1,
	I2C_MUX6_BUS2,
	I2C_MUX6_BUS3,
	I2C_MUX6_BUS4,
	I2C_MUX6_BUS5,
	I2C_MUX6_BUS6,
	I2C_MUX6_BUS7,

	I2C_MUX7_BUS0,
	I2C_MUX7_BUS1,
	I2C_MUX7_BUS2,
	I2C_MUX7_BUS3,
	I2C_MUX7_BUS4,
	I2C_MUX7_BUS5,
	I2C_MUX7_BUS6,
	I2C_MUX7_BUS7,
};

/*
 * The list of I2C devices and their bus connections for this platform.
 *
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info specifies the device type, address,
 * and platform data depending on the device type.
 *
 * For muxes, we specify the bus number for each port,
 * and set the deselect_on_exit but (see comment above).
 *
 * For EEPROMs, including ones in the QSFP28 transceivers,
 * we specify the label, I2C address, size, and some flags.
 * All done in the magic mk*_eeprom() macros.  The label is
 * the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 */

mk_pca9545(mux1, I2C_MUX1_BUS0, 1);
mk_pca9545(mux2, I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, I2C_MUX3_BUS0, 1);

mk_pca9548(mux4, I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, I2C_MUX6_BUS0, 1);
mk_pca9548(mux7, I2C_MUX7_BUS0, 1);

mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

mk_qsfp_port_eeprom(port1,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port10, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port11, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port12, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port13, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port14, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port15, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port16, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port17, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port18, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port19, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port20, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port21, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port22, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port23, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port24, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port25, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port31, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port32, 50, 256, SFF_8436_FLAG_IRUGO);

static struct quanta_ix7_platform_data cpld1 = {
	.idx = IX7_LED_QSFP28_1_16_CPLD_ID,
};

static struct quanta_ix7_platform_data cpld2 = {
	.idx = IX7_IO_QSFP28_1_16_CPLD_ID,
};

static struct quanta_ix7_platform_data cpld3 = {
	.idx = IX7_IO_QSFP28_17_32_CPLD_ID,
};

static struct quanta_ix7_platform_data cpld4 = {
	.idx = IX7_LED_QSFP28_17_32_CPLD_ID,
};

/*
 * GPIO definitions
 * num, name, flags
 */

mk_gpio_pins(cpu_gpio_20) = {
	mk_gpio_pin(0,	mb_i2c_rst,    (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,	mb_led_rst,    (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,	mb_eth_rst,    (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,	mb_sw_rst,     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,	usb_rst,       (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(9,	i2c_switch,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10, boot_led,      (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)), 
	mk_gpio_pin(11, sys_led,       (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(12, board_pwr_led, GPIOF_OUT_INIT_HIGH),
};

mk_gpio_pins(cpu_gpio_21) = {
	mk_gpio_pin(0,	sb_id0,	       GPIOF_DIR_IN),
	mk_gpio_pin(1,	sb_id1,	       GPIOF_DIR_IN),
	mk_gpio_pin(2,	sb_id2,	       GPIOF_DIR_IN), 
	mk_gpio_pin(3,	sb_id3,	       GPIOF_DIR_IN),
	mk_gpio_pin(4,	sb_id4,        GPIOF_DIR_IN),
	mk_gpio_pin(5,	sb_id5,        GPIOF_DIR_IN),
};

mk_gpio_pins(cpu_gpio_22) = {
	mk_gpio_pin(3,	sb_present,	 (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,	usb_overcurrent, (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,	mb_pca9555_int,	 (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, psoc_irq,	 (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(12, psu_9555_irq,	 (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, bios_wp,	 (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
};

mk_gpio_pins(mb_gpio_23) = {
	mk_gpio_pin(0,	cpld23_int,	    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,	cpld14_int,	    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,	qsfp28_power_good,  GPIOF_DIR_IN),
	mk_gpio_pin(5,	qsfp28_power_enable,GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,	mac_reset,	    (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,	usb_reset,	    (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(8,	mgmt_present,	    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(10, board_id_0,	    GPIOF_DIR_IN),
	mk_gpio_pin(11, board_id_1,	    GPIOF_DIR_IN),
	mk_gpio_pin(12, board_id_2,	    GPIOF_DIR_IN),
	mk_gpio_pin(13, board_id_3,	    GPIOF_DIR_IN),
	mk_gpio_pin(14, board_id_4,	    GPIOF_DIR_IN),
	mk_gpio_pin(15, board_id_5,	    GPIOF_DIR_IN),
};

mk_gpio_platform_data(cpu_gpio_20, IX7_CPU_GPIO_20_BASE, PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_21, IX7_CPU_GPIO_21_BASE, PCA_9554_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_22, IX7_CPU_GPIO_22_BASE, PCA_9555_GPIO_COUNT);

mk_gpio_platform_data(mb_gpio_23,  IX7_MB_GPIO_23_BASE, PCA_9555_GPIO_COUNT);

static struct platform_i2c_device_info i2c_devices[] = {
	/* cpu board */
	mk_i2cdev(I2C_I801_BUS,	 "pca9546",  0x71, &mux1_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9555",  0x20, &cpu_gpio_20_platform_data),
	mk_i2cdev(I2C_MUX1_BUS2, "pca9554",  0x21, &cpu_gpio_21_platform_data),
	mk_i2cdev(I2C_MUX1_BUS3, "pca9555",  0x22, &cpu_gpio_22_platform_data),

	/* main board */
	mk_i2cdev(I2C_I801_BUS,	 "pca9546",  0x72, &mux2_platform_data),
	mk_i2cdev(I2C_I801_BUS,	 "pca9548",  0x77, &mux3_platform_data),

	/* mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "quanta_ix7_cpld", 0x38, &cpld2),
	mk_i2cdev(I2C_MUX2_BUS0, "quanta_ix7_cpld", 0x39, &cpld1),
	mk_i2cdev(I2C_MUX2_BUS1, "quanta_ix7_cpld", 0x38, &cpld3),
	mk_i2cdev(I2C_MUX2_BUS1, "quanta_ix7_cpld", 0x39, &cpld4),
	mk_i2cdev(I2C_MUX2_BUS2, "24c02",    0x54, &board_54_at24),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9555",  0x23, &mb_gpio_23_platform_data),

	/* mux3 */
	mk_i2cdev(I2C_MUX3_BUS0, "pca9548",  0x73, &mux4_platform_data),
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548",  0x73, &mux5_platform_data),
	mk_i2cdev(I2C_MUX3_BUS2, "pca9548",  0x73, &mux6_platform_data),
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548",  0x73, &mux7_platform_data),

	/* mux4 */
	mk_i2cdev(I2C_MUX4_BUS0, "sff8436",  0x50, &port1_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS1, "sff8436",  0x50, &port2_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS2, "sff8436",  0x50, &port3_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS3, "sff8436",  0x50, &port4_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS4, "sff8436",  0x50, &port5_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS5, "sff8436",  0x50, &port6_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS6, "sff8436",  0x50, &port7_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS7, "sff8436",  0x50, &port8_50_sff8436),

	/* mux5 */
	mk_i2cdev(I2C_MUX5_BUS0, "sff8436",  0x50, &port9_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS1, "sff8436",  0x50, &port10_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS2, "sff8436",  0x50, &port11_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS3, "sff8436",  0x50, &port12_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS4, "sff8436",  0x50, &port13_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS5, "sff8436",  0x50, &port14_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS6, "sff8436",  0x50, &port15_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS7, "sff8436",  0x50, &port16_50_sff8436),

	/* mux6 */
	mk_i2cdev(I2C_MUX6_BUS0, "sff8436",  0x50, &port17_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS1, "sff8436",  0x50, &port18_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS2, "sff8436",  0x50, &port19_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS3, "sff8436",  0x50, &port20_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS4, "sff8436",  0x50, &port21_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS5, "sff8436",  0x50, &port22_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS6, "sff8436",  0x50, &port23_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS7, "sff8436",  0x50, &port24_50_sff8436),

	/* mux7 */
	mk_i2cdev(I2C_MUX7_BUS0, "sff8436",  0x50, &port25_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS1, "sff8436",  0x50, &port26_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS2, "sff8436",  0x50, &port27_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS3, "sff8436",  0x50, &port28_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS4, "sff8436",  0x50, &port29_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS5, "sff8436",  0x50, &port30_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS6, "sff8436",  0x50, &port31_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS7, "sff8436",  0x50, &port32_50_sff8436),
};

/*
 * Utility functions for I2C
 */

static void free_i2c_devices(void)
{
	int i;

	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		struct i2c_client *c = i2c_devices[i].client;

		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}
}

static int i2c_init(void)
{
	int ismt_bus = -1;
	int i801_bus = -1;
	int i;
	int ret;

	ret = -1;
	ismt_bus = cumulus_i2c_find_adapter(SMBUS_ISMT_NAME);
	if (ismt_bus < 0) {
		pr_err("could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	i801_bus = cumulus_i2c_find_adapter(SMBUS_I801_NAME);
	if (i801_bus < 0) {
		pr_err("could not find i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	return 0;

err_exit:
	return ret;
}

/*
 * Utility functions for GPIO
 */

static void init_gpio_platform_data(struct gpio_pin *pins,
				    int num_pins,
				    struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < num_pins; i++)
		names[pins[i].num] = pins[i].name;
}

static int init_gpio_pins(struct gpio_pin *pins,
			  int num_pins,
			  int gpio_base)
{
	int i;
	int ret;

	pr_debug("Registering %d GPIO pins\n", num_pins);
	for (i = 0; i < num_pins; i++, pins++) {
		ret = gpio_request_one(gpio_base + pins->num,
				       pins->flags,
				       pins->name);
		if (ret) {
			if (ret !=  -EPROBE_DEFER) {
				pr_err(DRIVER_NAME
					"Failed to request %d GPIO pin"
					" %s, err %d\n", gpio_base + pins->num,
					pins->name, ret);
			}
			goto err_exit;
		}
	}
	pr_debug(DRIVER_NAME "Succeeeded in gpio pins create \n");
	return 0;

err_exit:
	while (i--) {
		gpio_free(gpio_base + (--pins)->num);
	}

	return ret;
}

static void free_gpio_pins(struct gpio_pin *pins,
			   int num_pins,
			   int gpio_base)
{
	while (num_pins--) {
		gpio_free(gpio_base + (pins++)->num);
	}
}

/*
 * Module init and exit for all I2C devices, including GPIO
 */

static void gpio_free_all(void)
{
	free_gpio_pins(mb_gpio_23_pins, ARRAY_SIZE(mb_gpio_23_pins),
		       IX7_MB_GPIO_23_BASE);
	free_gpio_pins(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
		       IX7_CPU_GPIO_22_BASE);
	free_gpio_pins(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
		       IX7_CPU_GPIO_21_BASE);
	free_gpio_pins(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
		       IX7_CPU_GPIO_20_BASE);
}

static int ix7_i2c_init(void)
{
	int ret;

	pr_debug(DRIVER_NAME " ix7 I2C init \n");

	init_gpio_platform_data(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
				&cpu_gpio_20_platform_data);
	init_gpio_platform_data(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
				&cpu_gpio_21_platform_data);
	init_gpio_platform_data(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
				&cpu_gpio_22_platform_data);
	init_gpio_platform_data(mb_gpio_23_pins, ARRAY_SIZE(mb_gpio_23_pins),
				&mb_gpio_23_platform_data);

	ret = i2c_init();
	if (ret)
		goto err_exit;

	pr_debug(DRIVER_NAME "ix7 I2C Init succeeded \n");
	return 0;

err_exit:
	pr_err(DRIVER_NAME "ix7 I2C init failed \n");
	free_i2c_devices();
	return ret;

}

static void ix7_i2c_exit(void)
{
	free_i2c_devices();
}

static void ix7_gpio_exit(void)
{
	gpio_free_all();
}

static int ix7_gpio_init(void)
{
	int ret;

	pr_debug(DRIVER_NAME "ix7 GPIO init \n");

	ret = init_gpio_pins(cpu_gpio_20_pins,
			     ARRAY_SIZE(cpu_gpio_20_pins),
			     IX7_CPU_GPIO_20_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(cpu_gpio_21_pins, 
			     ARRAY_SIZE(cpu_gpio_21_pins),
			     IX7_CPU_GPIO_21_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(cpu_gpio_22_pins,
			     ARRAY_SIZE(cpu_gpio_22_pins),
			     IX7_CPU_GPIO_22_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(mb_gpio_23_pins,
			     ARRAY_SIZE(mb_gpio_23_pins),
			     IX7_MB_GPIO_23_BASE);
	if (ret)
		goto err_exit;

	pr_debug(DRIVER_NAME "Succeeded in gpio init \n");
	return 0;

err_exit:
	gpio_free_all();
	return ret;
}
static int ix7_platform_probe(struct platform_device *dev)
{
	int ret;

	dev_info(&dev->dev, "ix7 probe begin \n");

	ret = ix7_gpio_init();
	if (ret) {
		if (ret != -EPROBE_DEFER) {
			ix7_i2c_exit();
			dev_err(&dev->dev, "GPIO initialization failed (%d)\n",
				ret);
		} else {
			dev_info(&dev->dev, "GPIO initialization deferred\n");
		}

		return ret;
	}

	dev_info(&dev->dev, "ix7 probe succeeded \n");
	return 0;
}

static int ix7_platform_remove(struct platform_device *dev)
{
    ix7_gpio_exit();
    ix7_i2c_exit();
    return 0;
}
static const struct platform_device_id ix7_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ix7_platform_id);

static struct platform_driver ix7_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ix7_platform_probe,
	.remove = ix7_platform_remove,

};

static struct platform_device *ix7_plat_device = NULL;

static int __init ix7_platform_init(void)
{
	int ret;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ix7_platform_driver.driver.name, &platform_bus_type)) {
		ret = platform_driver_register(&ix7_platform_driver);
		if (ret) {
		    pr_err(DRIVER_NAME ": %s driver registration failed."
		       "(%d)\n", ix7_platform_driver.driver.name, ret);
		    return ret;
		}
	}

	if (ix7_plat_device == NULL) {
		ix7_plat_device = platform_device_register_simple(DRIVER_NAME, -1,
								  NULL, 0);
		if (IS_ERR(ix7_plat_device)) {
			ret = PTR_ERR(ix7_plat_device);
			ix7_plat_device = NULL;
			pr_err(DRIVER_NAME": Platform device registration"
					"failed. (%d)\n", ret);
			platform_driver_unregister(&ix7_platform_driver);
			return ret;
		}
	}

	ret = ix7_i2c_init();
	if (ret) {
		pr_err("I2C initialization failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded \n");

	return 0;
}

static void __exit ix7_platform_exit(void)
{
	platform_device_unregister(ix7_plat_device);
	ix7_plat_device = NULL;
	platform_driver_unregister(&ix7_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}

module_init(ix7_platform_init);
module_exit(ix7_platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta IX7 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

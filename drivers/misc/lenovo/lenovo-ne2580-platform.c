// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE2580 platform driver
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.  All rights reserved.
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
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "lenovo-ne2580.h"

#define DRIVER_NAME	NE2580_PLATFORM_NAME
#define DRIVER_VERSION	"1.1"

/*
 * The platform has one i801 adapter.
 *
 * The i801 is connected to the following devices on the CPU board:
 *
 *    bios backup eeprom (0x56)
 *    onie eeprom (0x57)
 *    tmp75 (0x48)
 *
 * The i801 is connected to the following devices on the switch board:
 *
 *    cpld (0x77)
 *    vpd eeprom (0x55)
 *    swconfig eeprom (0x54)
 *    pca9543 2-channel switch (0x73)
 *	 0 psu1 (0x5a)
 *	   psu2 (0x5b)
 *	   ucd90160 (0x34)
 *	 1 tmp75 (0x4a)
 *	   tmp75 (0x4d)
 *	   tmp75 (0x4e)
 *    pca9548 8-channel mux (0x70)
 *	 0 pca9548 8-channel mux (0x72): swp49-swp56
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 1 pca9548 8-channel mux (0x72): swp01-swp08
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 2 pca9548 8-channel mux (0x72): swp09-swp16
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 3 pca9548 8-channel mux (0x72): swp17-swp24
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 4 pca9548 8-channel mux (0x72): swp25-swp32
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 5 pca9548 8-channel mux (0x72): swp33-swp40
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *	 6 pca9548 8-channel mux (0x72): swp41-swp48
 *	   pca9555 #1 16-channel gpio expander (0x20)
 *	   pca9555 #2 16-channel gpio expander (0x20)
 *	   pca9555 #3 16-channel gpio expander (0x20)
 *
 */

enum {
	I2C_I801_BUS = -1,

	/* 0x73 pca9543 switch */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,

	/* 0x70 pca9548 mux */
	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX3_BUS0 = 69, /* ports 49-56 */
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX4_BUS0 = 21, /* ports 1-8... */
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX5_BUS0,
	I2C_MUX5_BUS1,
	I2C_MUX5_BUS2,
	I2C_MUX5_BUS3,
	I2C_MUX5_BUS4,
	I2C_MUX5_BUS5,
	I2C_MUX5_BUS6,
	I2C_MUX5_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX6_BUS0,
	I2C_MUX6_BUS1,
	I2C_MUX6_BUS2,
	I2C_MUX6_BUS3,
	I2C_MUX6_BUS4,
	I2C_MUX6_BUS5,
	I2C_MUX6_BUS6,
	I2C_MUX6_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX7_BUS0,
	I2C_MUX7_BUS1,
	I2C_MUX7_BUS2,
	I2C_MUX7_BUS3,
	I2C_MUX7_BUS4,
	I2C_MUX7_BUS5,
	I2C_MUX7_BUS6,
	I2C_MUX7_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX8_BUS0,
	I2C_MUX8_BUS1,
	I2C_MUX8_BUS2,
	I2C_MUX8_BUS3,
	I2C_MUX8_BUS4,
	I2C_MUX8_BUS5,
	I2C_MUX8_BUS6,
	I2C_MUX8_BUS7,

	/* 0x72 pca9548 mux */
	I2C_MUX9_BUS0,
	I2C_MUX9_BUS1,
	I2C_MUX9_BUS2,
	I2C_MUX9_BUS3,
	I2C_MUX9_BUS4,
	I2C_MUX9_BUS5,
	I2C_MUX9_BUS6,
	I2C_MUX9_BUS7,
};

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

mk_pca9543(mux1, I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, I2C_MUX3_BUS0, 1);
mk_pca9548(mux4, I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, I2C_MUX6_BUS0, 1);
mk_pca9548(mux7, I2C_MUX7_BUS0, 1);
mk_pca9548(mux8, I2C_MUX8_BUS0, 1);
mk_pca9548(mux9, I2C_MUX9_BUS0, 1);

mk_eeprom(swconfig, 54, 512, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(vpd,	    55, 512, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(bios,	    56, 512, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(board,    57, 512, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);

mk_eeprom(psu1,	    52, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,	    53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

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
 * the I2C device.  Then the pins are created by init_gpio_pins().
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

/* --- mux channel 0 --- */
mk_gpio_pins(gpio1) = {
	mk_gpio_pin(0,	port49_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(1,	port50_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,	port51_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,	port52_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,	port53_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port54_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port55_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port56_modsel, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port49_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,	port50_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10,	port51_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11,	port52_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12,	port53_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13,	port54_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14,	port55_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15,	port56_reset,  GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW),
};

mk_gpio_pins(gpio2) = {
	mk_gpio_pin(0,	port49_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port50_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port51_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port52_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port53_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port54_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port55_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port56_lpmode, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	unused_pin8,   GPIOF_IN),
	mk_gpio_pin(9,	unused_pin9,   GPIOF_IN),
	mk_gpio_pin(10,	unused_pin10,  GPIOF_IN),
	mk_gpio_pin(11,	unused_pin11,  GPIOF_IN),
	mk_gpio_pin(12,	unused_pin12,  GPIOF_IN),
	mk_gpio_pin(13,	unused_pin13,  GPIOF_IN),
	mk_gpio_pin(14,	unused_pin14,  GPIOF_IN),
	mk_gpio_pin(15,	unused_pin15,  GPIOF_IN),
};

mk_gpio_pins(gpio3) = {
	mk_gpio_pin(0,	port49_rxlos,	GPIOF_IN),
	mk_gpio_pin(1,	port50_rxlos,	GPIOF_IN),
	mk_gpio_pin(2,	port51_rxlos,	GPIOF_IN),
	mk_gpio_pin(3,	port52_rxlos,	GPIOF_IN),
	mk_gpio_pin(4,	port53_rxlos,	GPIOF_IN),
	mk_gpio_pin(5,	port54_rxlos,	GPIOF_IN),
	mk_gpio_pin(6,	port55_rxlos,	GPIOF_IN),
	mk_gpio_pin(7,	port56_rxlos,	GPIOF_IN),
	mk_gpio_pin(8,	port49_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,	port50_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10, port51_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11, port52_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12, port53_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13, port54_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14, port55_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15, port56_present, GPIOF_IN | GPIOF_ACTIVE_LOW),
};

/* --- mux channel 1 --- */
mk_gpio_pins(gpio4) = {
	mk_gpio_pin(0,	port1_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(1,	port2_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(2,	port3_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(3,	port4_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(4,	port1_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port2_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port3_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port4_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port1_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port2_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port3_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port4_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port1_rx_los,	  GPIOF_IN),
	mk_gpio_pin(13, port2_rx_los,	  GPIOF_IN),
	mk_gpio_pin(14, port3_rx_los,	  GPIOF_IN),
	mk_gpio_pin(15, port4_rx_los,	  GPIOF_IN),
};

mk_gpio_pins(gpio5) = {
	mk_gpio_pin(0,	port5_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(1,	port6_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(2,	port7_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(3,	port8_tx_fault,	  GPIOF_IN),
	mk_gpio_pin(4,	port5_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port6_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port7_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port8_present,	  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port5_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port6_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port7_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port8_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port5_rx_los,	  GPIOF_IN),
	mk_gpio_pin(13, port6_rx_los,	  GPIOF_IN),
	mk_gpio_pin(14, port7_rx_los,	  GPIOF_IN),
	mk_gpio_pin(15, port8_rx_los,	  GPIOF_IN),
};

mk_gpio_pins(gpio6) = {
	mk_gpio_pin(0,	port1_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port1_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port2_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port2_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port3_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port3_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port4_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port4_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port5_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port5_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port6_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port6_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port7_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port7_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port8_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port8_rs1, GPIOF_OUT_INIT_LOW),
};

/* --- mux channel 2 --- */
mk_gpio_pins(gpio7) = {
	mk_gpio_pin(0,	port9_tx_fault,	   GPIOF_IN),
	mk_gpio_pin(1,	port10_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port11_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port12_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port9_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port10_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port11_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port12_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port9_tx_disable,  GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port10_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port11_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port12_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port9_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port10_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port11_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port12_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio8) = {
	mk_gpio_pin(0,	port13_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port14_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port15_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port16_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port13_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port14_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port15_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port16_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port13_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port14_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port15_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port16_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port13_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port14_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port15_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port16_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio9) = {
	mk_gpio_pin(0,	port9_rs0,  GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port9_rs1,  GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port10_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port10_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port11_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port11_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port12_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port12_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port13_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port13_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port14_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port14_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port15_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port15_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port16_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port16_rs1, GPIOF_OUT_INIT_LOW),
};

/* --- mux channel 3 --- */
mk_gpio_pins(gpio10) = {
	mk_gpio_pin(0,	port17_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port18_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port19_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port20_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port17_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port18_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port19_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port20_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port17_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port18_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port19_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port20_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port17_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port18_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port19_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port20_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio11) = {
	mk_gpio_pin(0,	port21_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port22_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port23_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port24_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port21_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port22_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port23_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port24_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port21_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port22_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port23_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port24_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port21_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port22_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port23_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port24_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio12) = {
	mk_gpio_pin(0,	port17_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port17_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port18_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port18_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port19_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port19_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port20_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port20_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port21_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port21_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port22_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port22_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port23_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port23_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port24_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port24_rs1, GPIOF_OUT_INIT_LOW),
};

/* --- mux channel 4 --- */
mk_gpio_pins(gpio13) = {
	mk_gpio_pin(0,	port25_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port26_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port27_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port28_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port25_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port26_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port27_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port28_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port25_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port26_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port27_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port28_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port25_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port26_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port27_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port28_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio14) = {
	mk_gpio_pin(0,	port29_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port30_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port31_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port32_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port29_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port30_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port31_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port32_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port29_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port30_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port31_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port32_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port29_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port30_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port31_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port32_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio15) = {
	mk_gpio_pin(0,	port25_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port25_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port26_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port26_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port27_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port27_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port28_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port28_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port29_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port29_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port30_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port30_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port31_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port31_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port32_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port32_rs1, GPIOF_OUT_INIT_LOW),
};

/* --- mux channel 5 --- */
mk_gpio_pins(gpio16) = {
	mk_gpio_pin(0,	port33_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port34_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port35_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port36_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port33_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port34_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port35_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port36_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port33_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port34_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port35_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port36_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port33_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port34_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port35_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port36_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio17) = {
	mk_gpio_pin(0,	port37_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port38_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port39_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port40_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port37_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port38_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port39_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port40_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port37_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port38_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port39_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port40_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port37_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port38_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port39_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port40_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio18) = {
	mk_gpio_pin(0,	port33_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port33_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port34_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port34_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port35_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port35_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port36_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port36_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port37_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port37_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port38_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port38_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port39_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port39_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port40_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port40_rs1, GPIOF_OUT_INIT_LOW),
};

/* --- mux channel 6 --- */
mk_gpio_pins(gpio19) = {
	mk_gpio_pin(0,	port41_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port42_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port43_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port44_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port41_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port42_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port43_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port44_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port41_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port42_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port43_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port44_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port41_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port42_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port43_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port44_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio20) = {
	mk_gpio_pin(0,	port45_tx_fault,   GPIOF_IN),
	mk_gpio_pin(1,	port46_tx_fault,   GPIOF_IN),
	mk_gpio_pin(2,	port47_tx_fault,   GPIOF_IN),
	mk_gpio_pin(3,	port48_tx_fault,   GPIOF_IN),
	mk_gpio_pin(4,	port45_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,	port46_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,	port47_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,	port48_present,	   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,	port45_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port46_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port47_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port48_tx_disable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port45_rx_los,	   GPIOF_IN),
	mk_gpio_pin(13, port46_rx_los,	   GPIOF_IN),
	mk_gpio_pin(14, port47_rx_los,	   GPIOF_IN),
	mk_gpio_pin(15, port48_rx_los,	   GPIOF_IN),
};

mk_gpio_pins(gpio21) = {
	mk_gpio_pin(0,	port41_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,	port41_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,	port42_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,	port42_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	port43_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,	port43_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,	port44_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,	port44_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	port45_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,	port45_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, port46_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, port46_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, port47_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, port47_rs1, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, port48_rs0, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, port48_rs1, GPIOF_OUT_INIT_LOW),
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
	int i;
	int ret;
	struct gpio_pin *pins = c;

	pr_info("Registering %d GPIO pins\n", num_pins);
	for (i = 0; i < num_pins; i++, pins++) {
		ret = gpio_request_one(gpio_base + pins->pin, pins->flags,
				       pins->label);
		if (ret) {
			pr_err(DRIVER_NAME ": failed to request GPIO pin %s (%d), err %d\n",
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

mk_gpio_platform_data(gpio1,	1, 16, &gpio1_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio2,   17, 16, &gpio2_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio3,   33, 16, &gpio3_pins,  init_gpios, free_gpios);

mk_gpio_platform_data(gpio4,   49, 16, &gpio4_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio5,   65, 16, &gpio5_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio6,   81, 16, &gpio6_pins,  init_gpios, free_gpios);

mk_gpio_platform_data(gpio7,   97, 16, &gpio7_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio8,  113, 16, &gpio8_pins,  init_gpios, free_gpios);
mk_gpio_platform_data(gpio9,  129, 16, &gpio9_pins,  init_gpios, free_gpios);

mk_gpio_platform_data(gpio10, 145, 16, &gpio10_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio11, 161, 16, &gpio11_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio12, 177, 16, &gpio12_pins, init_gpios, free_gpios);

mk_gpio_platform_data(gpio13, 193, 16, &gpio13_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio14, 209, 16, &gpio14_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio15, 225, 16, &gpio15_pins, init_gpios, free_gpios);

mk_gpio_platform_data(gpio16, 241, 16, &gpio16_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio17, 257, 16, &gpio17_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio18, 273, 16, &gpio18_pins, init_gpios, free_gpios);

mk_gpio_platform_data(gpio19, 289, 16, &gpio19_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio20, 305, 16, &gpio20_pins, init_gpios, free_gpios);
mk_gpio_platform_data(gpio21, 321, 16, &gpio21_pins, init_gpios, free_gpios);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.  Each entry is a bus
 * number and a i2c_board_info.  The i2c_board_info structure specifies the
 * device type, address, and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_I801_BUS,	 NE2580_CPLD1_NAME, 0x77, NULL),
	mk_i2cdev(I2C_I801_BUS,	 NE2580_CPLD2_NAME, 0x33, NULL),

	mk_i2cdev(I2C_I801_BUS,	 "pca9543",	 0x73, &mux1_platform_data),
	mk_i2cdev(I2C_I801_BUS,	 "pca9548",	 0x70, &mux2_platform_data),

	mk_i2cdev(I2C_MUX1_BUS0, "pmbus",	 0x5a, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "pmbus",	 0x5b, NULL),

	mk_i2cdev(I2C_MUX1_BUS1, "tmp75",	 0x4a, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75",	 0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75",	 0x4e, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75",	 0x48, NULL),

	mk_i2cdev(I2C_MUX2_BUS0, "pca9548",	 0x72, &mux3_platform_data),
	mk_i2cdev(I2C_MUX2_BUS1, "pca9548",	 0x72, &mux4_platform_data),
	mk_i2cdev(I2C_MUX2_BUS2, "pca9548",	 0x72, &mux5_platform_data),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9548",	 0x72, &mux6_platform_data),
	mk_i2cdev(I2C_MUX2_BUS4, "pca9548",	 0x72, &mux7_platform_data),
	mk_i2cdev(I2C_MUX2_BUS5, "pca9548",	 0x72, &mux8_platform_data),
	mk_i2cdev(I2C_MUX2_BUS6, "pca9548",	 0x72, &mux9_platform_data),

	/*
	 * Add the board EEPROM first and immediately afterwards, add the
	 * port EEPROMs so that the port EEPROMs are numbered exactly like
	 * the ports.  Then add all the remaining EEPROMs at the end.  Not
	 * required, just nice.
	 */
	mk_i2cdev(I2C_I801_BUS,	 "24c02",	 0x57, &board_57_at24),

	/* QSFP28 ports 49-56 */
	mk_i2cdev(I2C_MUX3_BUS0, "sff8436",	 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS1, "sff8436",	 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS2, "sff8436",	 0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS3, "sff8436",	 0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS4, "sff8436",	 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS5, "sff8436",	 0x50, &port54_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS6, "sff8436",	 0x50, &port55_50_sff8436),
	mk_i2cdev(I2C_MUX3_BUS7, "sff8436",	 0x50, &port56_50_sff8436),
	mk_i2cdev(I2C_MUX2_BUS0, "pca9555",	 0x20, &gpio1_platform_data),
	mk_i2cdev(I2C_MUX2_BUS0, "pca9555",	 0x21, &gpio2_platform_data),
	mk_i2cdev(I2C_MUX2_BUS0, "pca9555",	 0x22, &gpio3_platform_data),

	/* SFP28 ports 1-8 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c04",	 0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c04",	 0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c04",	 0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c04",	 0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c04",	 0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c04",	 0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX4_BUS6, "24c04",	 0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX4_BUS7, "24c04",	 0x50, &port8_50_at24),
	mk_i2cdev(I2C_MUX2_BUS1, "pca9555",	 0x20, &gpio4_platform_data),
	mk_i2cdev(I2C_MUX2_BUS1, "pca9555",	 0x21, &gpio5_platform_data),
	mk_i2cdev(I2C_MUX2_BUS1, "pca9555",	 0x22, &gpio6_platform_data),

	/* SFP28 ports 9-16 */
	mk_i2cdev(I2C_MUX5_BUS0, "24c04",	 0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1, "24c04",	 0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2, "24c04",	 0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3, "24c04",	 0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4, "24c04",	 0x50, &port13_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5, "24c04",	 0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6, "24c04",	 0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7, "24c04",	 0x50, &port16_50_at24),
	mk_i2cdev(I2C_MUX2_BUS2, "pca9555",	 0x20, &gpio7_platform_data),
	mk_i2cdev(I2C_MUX2_BUS2, "pca9555",	 0x21, &gpio8_platform_data),
	mk_i2cdev(I2C_MUX2_BUS2, "pca9555",	 0x22, &gpio9_platform_data),

	/* SFP28 ports 17-24 */
	mk_i2cdev(I2C_MUX6_BUS0, "24c04",	 0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1, "24c04",	 0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2, "24c04",	 0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3, "24c04",	 0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4, "24c04",	 0x50, &port21_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5, "24c04",	 0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6, "24c04",	 0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7, "24c04",	 0x50, &port24_50_at24),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9555",	 0x20, &gpio10_platform_data),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9555",	 0x21, &gpio11_platform_data),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9555",	 0x22, &gpio12_platform_data),

	/* SFP28 ports 25-32 */
	mk_i2cdev(I2C_MUX7_BUS0, "24c04",	 0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1, "24c04",	 0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2, "24c04",	 0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3, "24c04",	 0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4, "24c04",	 0x50, &port29_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5, "24c04",	 0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6, "24c04",	 0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7, "24c04",	 0x50, &port32_50_at24),
	mk_i2cdev(I2C_MUX2_BUS4, "pca9555",	 0x20, &gpio13_platform_data),
	mk_i2cdev(I2C_MUX2_BUS4, "pca9555",	 0x21, &gpio14_platform_data),
	mk_i2cdev(I2C_MUX2_BUS4, "pca9555",	 0x22, &gpio15_platform_data),

	/* SFP28 ports 33-40 */
	mk_i2cdev(I2C_MUX8_BUS0, "24c04",	 0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1, "24c04",	 0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2, "24c04",	 0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3, "24c04",	 0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4, "24c04",	 0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5, "24c04",	 0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6, "24c04",	 0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7, "24c04",	 0x50, &port40_50_at24),
	mk_i2cdev(I2C_MUX2_BUS5, "pca9555",	 0x20, &gpio16_platform_data),
	mk_i2cdev(I2C_MUX2_BUS5, "pca9555",	 0x21, &gpio17_platform_data),
	mk_i2cdev(I2C_MUX2_BUS5, "pca9555",	 0x22, &gpio18_platform_data),

	/* SFP28 ports 41-48 */
	mk_i2cdev(I2C_MUX9_BUS0, "24c04",	 0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1, "24c04",	 0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2, "24c04",	 0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3, "24c04",	 0x50, &port44_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4, "24c04",	 0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5, "24c04",	 0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6, "24c04",	 0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7, "24c04",	 0x50, &port48_50_at24),
	mk_i2cdev(I2C_MUX2_BUS6, "pca9555",	 0x20, &gpio19_platform_data),
	mk_i2cdev(I2C_MUX2_BUS6, "pca9555",	 0x21, &gpio20_platform_data),
	mk_i2cdev(I2C_MUX2_BUS6, "pca9555",	 0x22, &gpio21_platform_data),

	/*
	 * Add these EEPROMs after the port EEPROMs so that the port EEPROMs
	 * are numbered exactly like the ports.	 Not required, just nice.
	 */
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",	 0x52, &psu1_52_at24),
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",	 0x53, &psu2_53_at24),
	mk_i2cdev(I2C_I801_BUS,	 "24c02",	 0x54, &swconfig_54_at24),
	mk_i2cdev(I2C_I801_BUS,	 "24c02",	 0x55, &vpd_55_at24),
	mk_i2cdev(I2C_I801_BUS,	 "24c02",	 0x56, &bios_56_at24),

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
		if (i2c_devices[i].client) {
			i2c_unregister_device(i2c_devices[i].client);
			i2c_devices[i].client = NULL;
		}
	}
}

static int add_i2c_clients(struct platform_i2c_device_info *pdi, int num_dev,
			   int i801_bus)
{
	int ret;

	while (num_dev--) {
		if (pdi->bus == I2C_I801_BUS)
			pdi->bus = i801_bus;

		if (!pdi->client) {
			pdi->client = cumulus_i2c_add_client(pdi->bus,
							     &pdi->board_info);
			if (IS_ERR(pdi->client)) {
				ret = PTR_ERR(pdi->client);
				pdi->client = NULL;
				return ret;
			}
		}

		pdi++;
	}
	return 0;
}

/* platform driver */

static int platform_probe(struct platform_device *dev)
{
	int i801_bus;
	int ret;

	/* apply the gpio names */
	init_gpio_platform_data(gpio1_pins, ARRAY_SIZE(gpio1_pins),
				&gpio1_platform_data);
	init_gpio_platform_data(gpio2_pins, ARRAY_SIZE(gpio2_pins),
				&gpio2_platform_data);
	init_gpio_platform_data(gpio3_pins, ARRAY_SIZE(gpio3_pins),
				&gpio3_platform_data);

	init_gpio_platform_data(gpio4_pins, ARRAY_SIZE(gpio4_pins),
				&gpio4_platform_data);
	init_gpio_platform_data(gpio5_pins, ARRAY_SIZE(gpio5_pins),
				&gpio5_platform_data);
	init_gpio_platform_data(gpio6_pins, ARRAY_SIZE(gpio6_pins),
				&gpio6_platform_data);

	init_gpio_platform_data(gpio7_pins, ARRAY_SIZE(gpio7_pins),
				&gpio7_platform_data);
	init_gpio_platform_data(gpio8_pins, ARRAY_SIZE(gpio8_pins),
				&gpio8_platform_data);
	init_gpio_platform_data(gpio9_pins, ARRAY_SIZE(gpio9_pins),
				&gpio9_platform_data);

	init_gpio_platform_data(gpio10_pins, ARRAY_SIZE(gpio10_pins),
				&gpio10_platform_data);
	init_gpio_platform_data(gpio11_pins, ARRAY_SIZE(gpio11_pins),
				&gpio11_platform_data);
	init_gpio_platform_data(gpio12_pins, ARRAY_SIZE(gpio12_pins),
				&gpio12_platform_data);

	init_gpio_platform_data(gpio13_pins, ARRAY_SIZE(gpio13_pins),
				&gpio13_platform_data);
	init_gpio_platform_data(gpio14_pins, ARRAY_SIZE(gpio14_pins),
				&gpio14_platform_data);
	init_gpio_platform_data(gpio15_pins, ARRAY_SIZE(gpio15_pins),
				&gpio15_platform_data);

	init_gpio_platform_data(gpio16_pins, ARRAY_SIZE(gpio16_pins),
				&gpio16_platform_data);
	init_gpio_platform_data(gpio17_pins, ARRAY_SIZE(gpio17_pins),
				&gpio17_platform_data);
	init_gpio_platform_data(gpio18_pins, ARRAY_SIZE(gpio18_pins),
				&gpio18_platform_data);

	init_gpio_platform_data(gpio19_pins, ARRAY_SIZE(gpio19_pins),
				&gpio19_platform_data);
	init_gpio_platform_data(gpio20_pins, ARRAY_SIZE(gpio20_pins),
				&gpio20_platform_data);
	init_gpio_platform_data(gpio21_pins, ARRAY_SIZE(gpio21_pins),
				&gpio21_platform_data);

	/* identify the adapter buses */
	ret = -ENODEV;
	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		dev_err(&dev->dev, "Could not find the i801 adapter bus\n");
		goto err_exit;
	}

	/* add the i2c devices */
	ret = add_i2c_clients(&i2c_devices[0], NUM_I2C_DEVICES,
			      i801_bus);

	if (ret)
		goto err_exit;

	return 0;

err_exit:
	if (ret != -EPROBE_DEFER)
		del_i2c_clients();
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

static struct platform_driver platform_driver = {
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
	if (!driver_find(platform_driver.driver.name, &platform_bus_type)) {
		ret = platform_driver_register(&platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed (%d)\n",
			       platform_driver.driver.name, ret);
			return ret;
		}
	}

	/* create the platform device */
	if (!plat_device) {
		plat_device = platform_device_register_simple(DRIVER_NAME, -1,
							      NULL, 0);
		if (IS_ERR(plat_device)) {
			ret = PTR_ERR(plat_device);
			plat_device = NULL;
			pr_err(DRIVER_NAME ": platform device registration failed (%d)\n",
			       ret);
			platform_driver_unregister(&platform_driver);
			return ret;
		}
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " registered\n");
	return ret;
}

static void __exit platform_exit(void)
{
	platform_device_unregister(plat_device);
	plat_device = NULL;
	platform_driver_unregister(&platform_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Lenovo NE2580 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
